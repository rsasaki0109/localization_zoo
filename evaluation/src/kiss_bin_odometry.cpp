#include "kiss_icp/kiss_icp.h"

#include <Eigen/Core>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;
using localization_zoo::kiss_icp::KISSICPParams;
using localization_zoo::kiss_icp::KISSICPPipeline;
using localization_zoo::kiss_icp::correctElevationAngle;

namespace {

struct VoxelAccumulator {
  Eigen::Vector3d sum = Eigen::Vector3d::Zero();
  std::size_t count = 0;
};

using VoxelKey = std::tuple<int, int, int>;

std::vector<fs::path> listScans(const fs::path& directory) {
  std::vector<fs::path> scans;
  for (const auto& entry : fs::directory_iterator(directory)) {
    if (entry.is_regular_file() && entry.path().extension() == ".bin") {
      scans.push_back(entry.path());
    }
  }
  std::sort(scans.begin(), scans.end());
  return scans;
}

std::vector<Eigen::Vector3d> loadKittiScan(const fs::path& path) {
  std::ifstream stream(path, std::ios::binary | std::ios::ate);
  if (!stream) throw std::runtime_error("cannot open scan: " + path.string());
  const std::streamsize bytes = stream.tellg();
  if (bytes < 0 || bytes % static_cast<std::streamsize>(4 * sizeof(float)) != 0) {
    throw std::runtime_error("invalid KITTI scan size: " + path.string());
  }
  stream.seekg(0);
  std::vector<float> values(static_cast<std::size_t>(bytes) / sizeof(float));
  stream.read(reinterpret_cast<char*>(values.data()), bytes);
  if (!stream) throw std::runtime_error("short KITTI scan read: " + path.string());

  std::vector<Eigen::Vector3d> points;
  points.reserve(values.size() / 4);
  for (std::size_t index = 0; index < values.size(); index += 4) {
    const Eigen::Vector3d point(values[index], values[index + 1],
                                values[index + 2]);
    if (point.array().isFinite().all()) points.push_back(point);
  }
  return points;
}

std::vector<Eigen::Vector3d> voxelCentroids(
    const std::vector<Eigen::Vector3d>& points, double leaf) {
  std::map<VoxelKey, VoxelAccumulator> voxels;
  for (const auto& point : points) {
    const VoxelKey key{static_cast<int>(std::floor(point.z() / leaf)),
                       static_cast<int>(std::floor(point.y() / leaf)),
                       static_cast<int>(std::floor(point.x() / leaf))};
    auto& accumulator = voxels[key];
    accumulator.sum += point;
    ++accumulator.count;
  }
  std::vector<Eigen::Vector3d> result;
  result.reserve(voxels.size());
  for (const auto& [key, accumulator] : voxels) {
    (void)key;
    result.push_back(accumulator.sum / static_cast<double>(accumulator.count));
  }
  return result;
}

std::vector<Eigen::Vector3d> preprocess(
    const std::vector<Eigen::Vector3d>& points) {
  std::vector<Eigen::Vector3d> filtered;
  for (const auto& point : voxelCentroids(points, 0.8)) {
    const double range = point.norm();
    if (range > 1.0 && range < 80.0) filtered.push_back(point);
  }
  constexpr std::size_t max_points = 2000;
  if (filtered.size() > max_points) {
    std::vector<Eigen::Vector3d> limited;
    limited.reserve(max_points);
    const double step = static_cast<double>(filtered.size()) / max_points;
    for (std::size_t index = 0; index < max_points; ++index) {
      limited.push_back(filtered[std::min(
          static_cast<std::size_t>(index * step), filtered.size() - 1)]);
    }
    filtered = std::move(limited);
  }
  correctElevationAngle(filtered, 0.205 * std::acos(-1.0) / 180.0);
  return filtered;
}

void writePose(std::ostream& stream, const Eigen::Matrix4d& pose) {
  stream << std::setprecision(15);
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 4; ++column) {
      if (row != 0 || column != 0) stream << ' ';
      stream << pose(row, column);
    }
  }
  stream << '\n';
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "usage: kiss_bin_odometry SCAN_DIR OUTPUT_POSES MANIFEST_JSON\n";
    return 2;
  }
  const fs::path scan_directory(argv[1]);
  const fs::path output_path(argv[2]);
  const fs::path manifest_path(argv[3]);
  const auto scans = listScans(scan_directory);
  if (scans.empty()) throw std::runtime_error("no KITTI .bin scans found");

  KISSICPParams params;
  params.voxel_size = 1.2;
  params.initial_threshold = 1.75;
  params.max_points_per_voxel = 6;
  params.max_icp_iterations = 15;
  params.local_map_radius = 45.0;
  params.map_cleanup_interval = 2;
  params.update_full_voxels = false;
  params.use_model_deviation_threshold = true;
  params.model_deviation_correspondence_multiplier = 2.0;
  params.enable_motion_guard = true;
  params.enable_adaptive_motion_guard = true;
  KISSICPPipeline pipeline(params);

  std::ofstream output(output_path);
  if (!output) throw std::runtime_error("cannot open output pose file");
  double algorithm_seconds = 0.0;
  double sigma_sum = 0.0;
  double sigma_min = std::numeric_limits<double>::infinity();
  double sigma_max = 0.0;
  const auto total_start = std::chrono::steady_clock::now();
  for (std::size_t index = 0; index < scans.size(); ++index) {
    auto points = preprocess(loadKittiScan(scans[index]));
    const auto algorithm_start = std::chrono::steady_clock::now();
    const auto result = pipeline.registerFrame(points);
    algorithm_seconds += std::chrono::duration<double>(
        std::chrono::steady_clock::now() - algorithm_start).count();
    writePose(output, result.pose);
    const double sigma = pipeline.adaptiveThreshold();
    sigma_sum += sigma;
    sigma_min = std::min(sigma_min, sigma);
    sigma_max = std::max(sigma_max, sigma);
    if (index % 100 == 0) {
      std::cerr << "\r[KISS-bin] " << index << '/' << scans.size();
    }
  }
  output.close();
  const double total_seconds = std::chrono::duration<double>(
      std::chrono::steady_clock::now() - total_start).count();

  std::ofstream manifest(manifest_path);
  manifest << std::setprecision(15)
           << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"method\": \"kiss_modeldev_dynamic_voxel_radius\",\n"
           << "  \"ground_truth_used\": false,\n"
           << "  \"frames\": " << scans.size() << ",\n"
           << "  \"source_voxel_size\": 0.8,\n"
           << "  \"map_voxel_size\": 1.2,\n"
           << "  \"max_source_points\": 2000,\n"
           << "  \"vertical_angle_correction_deg\": 0.205,\n"
           << "  \"algorithm_seconds\": " << algorithm_seconds << ",\n"
           << "  \"total_seconds\": " << total_seconds << ",\n"
           << "  \"algorithm_fps\": " << scans.size() / algorithm_seconds << ",\n"
           << "  \"total_fps\": " << scans.size() / total_seconds << ",\n"
           << "  \"adaptive_sigma_min\": " << sigma_min << ",\n"
           << "  \"adaptive_sigma_mean\": " << sigma_sum / scans.size() << ",\n"
           << "  \"adaptive_sigma_max\": " << sigma_max << ",\n"
           << "  \"motion_guard_rejections\": "
           << pipeline.motionGuardRejections() << ",\n"
           << "  \"adaptive_motion_guard_acceptances\": "
           << pipeline.adaptiveMotionGuardAcceptances() << "\n"
           << "}\n";
  std::cerr << "\r[KISS-bin] " << scans.size() << '/' << scans.size()
            << " total_fps=" << scans.size() / total_seconds << '\n';
  return 0;
}
