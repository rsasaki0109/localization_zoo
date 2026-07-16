#include <bievr_lio/config_loader.h>
#include <bievr_lio/synchronizer.h>
#include <pcl/PCLPointCloud2.h>
#include <pcl/io/pcd_io.h>
#include <tbb/global_control.h>
#include <tbb/task_arena.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct FrameInfo {
  size_t index = 0;
  double timestamp_s = 0.0;
};

uint64_t secondsToNs(double seconds) {
  if (!std::isfinite(seconds) || seconds < 0.0) {
    throw std::runtime_error("Invalid non-finite or negative timestamp.");
  }
  return static_cast<uint64_t>(std::llround(seconds * 1e9));
}

std::vector<std::string> splitCsv(const std::string& line) {
  std::vector<std::string> values;
  std::stringstream stream(line);
  std::string value;
  while (std::getline(stream, value, ',')) values.push_back(value);
  return values;
}

std::vector<FrameInfo> loadFrames(const fs::path& path, int max_frames) {
  std::ifstream input(path);
  if (!input) throw std::runtime_error("Failed to open " + path.string());
  std::string line;
  std::getline(input, line);
  std::vector<FrameInfo> frames;
  while (std::getline(input, line)) {
    const auto values = splitCsv(line);
    if (values.size() < 2) continue;
    frames.push_back({static_cast<size_t>(std::stoull(values[0])), std::stod(values[1])});
    if (max_frames >= 0 && static_cast<int>(frames.size()) >= max_frames) break;
  }
  if (frames.empty()) throw std::runtime_error("No frame timestamps in " + path.string());
  return frames;
}

std::vector<bievr::ImuMeasurement> loadImu(const fs::path& path) {
  std::ifstream input(path);
  if (!input) throw std::runtime_error("Failed to open " + path.string());
  std::string line;
  std::getline(input, line);
  std::vector<bievr::ImuMeasurement> measurements;
  while (std::getline(input, line)) {
    const auto values = splitCsv(line);
    if (values.size() < 7) continue;
    bievr::ImuMeasurement measurement;
    measurement.stamp = secondsToNs(std::stod(values[0]));
    measurement.gyro =
        bievr::V3(std::stod(values[1]), std::stod(values[2]), std::stod(values[3]));
    measurement.acc =
        bievr::V3(std::stod(values[4]), std::stod(values[5]), std::stod(values[6]));
    measurements.push_back(measurement);
  }
  if (measurements.empty()) throw std::runtime_error("No IMU measurements in " + path.string());
  return measurements;
}

const pcl::PCLPointField* findField(const pcl::PCLPointCloud2& cloud, const std::string& name) {
  for (const auto& field : cloud.fields) {
    if (field.name == name) return &field;
  }
  return nullptr;
}

double readField(const uint8_t* point, const pcl::PCLPointField* field, double fallback = 0.0) {
  if (!field) return fallback;
  const uint8_t* value = point + field->offset;
  switch (field->datatype) {
    case pcl::PCLPointField::FLOAT32: {
      float result;
      std::memcpy(&result, value, sizeof(result));
      return result;
    }
    case pcl::PCLPointField::FLOAT64: {
      double result;
      std::memcpy(&result, value, sizeof(result));
      return result;
    }
    case pcl::PCLPointField::UINT32: {
      uint32_t result;
      std::memcpy(&result, value, sizeof(result));
      return result;
    }
    default:
      throw std::runtime_error("Unsupported PCD field datatype.");
  }
}

bievr::StampedIntensityPointcloud loadCloud(const fs::path& path, uint64_t stamp_ns) {
  pcl::PCLPointCloud2 raw;
  if (pcl::io::loadPCDFile(path.string(), raw) != 0) {
    throw std::runtime_error("Failed to load " + path.string());
  }
  const auto* x_field = findField(raw, "x");
  const auto* y_field = findField(raw, "y");
  const auto* z_field = findField(raw, "z");
  const auto* intensity_field = findField(raw, "intensity");
  const auto* time_field = findField(raw, "time");
  if (!x_field || !y_field || !z_field) {
    throw std::runtime_error(path.string() + " has no x/y/z fields.");
  }

  const size_t raw_points = static_cast<size_t>(raw.width) * raw.height;
  bievr::StampedIntensityPointcloud cloud;
  cloud.resize(raw_points);
  size_t valid = 0;
  double max_time_s = 0.0;
  for (size_t i = 0; i < raw_points; ++i) {
    const uint8_t* point = raw.data.data() + i * raw.point_step;
    const double x = readField(point, x_field);
    const double y = readField(point, y_field);
    const double z = readField(point, z_field);
    const double intensity = readField(point, intensity_field);
    const double time_s = readField(point, time_field);
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
        !std::isfinite(intensity) || !std::isfinite(time_s) || time_s < 0.0) {
      continue;
    }
    cloud.data().col(valid) << x, y, z, time_s, intensity;
    max_time_s = std::max(max_time_s, time_s);
    ++valid;
  }
  cloud.resize(valid);
  cloud.stamp = stamp_ns;
  cloud.end_stamp = stamp_ns + secondsToNs(max_time_s);
  return cloud;
}

fs::path framePath(const fs::path& pcd_dir, size_t index) {
  std::ostringstream name;
  name << std::setw(8) << std::setfill('0') << index;
  return pcd_dir / name.str() / "cloud.pcd";
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 6) {
    std::cerr << "usage: " << argv[0]
              << " PCD_DIR PARAMS_YAML SENSOR_YAML OUTPUT_TUM MAX_FRAMES\n";
    return 2;
  }
  try {
    const fs::path pcd_dir = argv[1];
    const fs::path params_yaml = argv[2];
    const fs::path sensor_yaml = argv[3];
    const fs::path output_tum = argv[4];
    const int max_frames = std::stoi(argv[5]);

    bievr::Config config;
    if (!bievr::loadConfigFromYaml({params_yaml.string(), sensor_yaml.string()}, config)) {
      throw std::runtime_error("Failed to load BIEVR-LIO configuration.");
    }
    fs::create_directories(output_tum.parent_path());
    config.pipeline_config.log_path = output_tum.string();
    config.pipeline_config.print_dashboard = false;
    const int thread_count = config.max_num_threads > 0
                                 ? config.max_num_threads
                                 : tbb::this_task_arena::max_concurrency();
    tbb::global_control tbb_control(
        tbb::global_control::max_allowed_parallelism, thread_count);

    auto pipeline = std::make_shared<bievr::Pipeline>(config.pipeline_config);
    pipeline->registerPublisher<bievr::Odometry>(
        [](const auto&, const auto&, const auto&, const auto&) {});
    pipeline->registerPublisher<bievr::V3>(
        [](const auto&, const auto&, const auto&, const auto&) {});
    pipeline->registerPublisher<bievr::IntensityPointcloud>(
        [](const auto&, const auto&, const auto&, const auto&) {});
    pipeline->registerPublisher<bievr::Pointcloud>(
        [](const auto&, const auto&, const auto&, const auto&) {});
    bievr::Synchronizer synchronizer(pipeline);

    const auto frames = loadFrames(pcd_dir / "frame_timestamps.csv", max_frames);
    const auto imu = loadImu(pcd_dir / "imu.csv");
    size_t imu_index = 0;
    for (size_t i = 0; i < frames.size(); ++i) {
      const uint64_t cloud_stamp = secondsToNs(frames[i].timestamp_s);
      while (imu_index < imu.size() && imu[imu_index].stamp < cloud_stamp) {
        synchronizer.addImu(imu[imu_index++]);
      }
      auto cloud = loadCloud(framePath(pcd_dir, frames[i].index), cloud_stamp);
      synchronizer.addPointcloud(cloud);
      if (imu_index < imu.size()) synchronizer.addImu(imu[imu_index++]);
      if ((i + 1) % 10 == 0 || i + 1 == frames.size()) {
        std::cerr << "\r[BIEVR-LIO upstream] " << (i + 1) << "/" << frames.size()
                  << std::flush;
      }
    }
    while (imu_index < imu.size()) synchronizer.addImu(imu[imu_index++]);
    std::cerr << "\n[done] trajectory=" << output_tum << "\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "[error] " << error.what() << "\n";
    return 1;
  }
}
