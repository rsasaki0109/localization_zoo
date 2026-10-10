// Place-recognition benchmark: runs loop-closure descriptors over a KITTI
// Velodyne sequence and writes each query's top-1 match and score.
//
// Ground truth is deliberately not read here. Labels (revisit / correct match)
// are assigned afterwards by evaluation/scripts/evaluate_place_recognition.py,
// so a descriptor never sees poses.

#include "dtd/dtd.h"
#include "isc_loam/isc_loam.h"
#include "scan_context/scan_context.h"

#include <Eigen/Core>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
namespace sc = localization_zoo::scan_context;
namespace isc = localization_zoo::isc_loam;
namespace dtd = localization_zoo::dtd;

namespace {

using Clock = std::chrono::steady_clock;

struct Scan {
  std::vector<Eigen::Vector3d> xyz;
  localization_zoo::aloam::PointCloudPtr xyzi;
};

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

Scan loadKittiScan(const fs::path& path) {
  std::ifstream stream(path, std::ios::binary | std::ios::ate);
  if (!stream) throw std::runtime_error("cannot open scan: " + path.string());
  const std::streamsize bytes = stream.tellg();
  if (bytes < 0 ||
      bytes % static_cast<std::streamsize>(4 * sizeof(float)) != 0) {
    throw std::runtime_error("invalid KITTI scan size: " + path.string());
  }
  stream.seekg(0);
  std::vector<float> values(static_cast<std::size_t>(bytes) / sizeof(float));
  stream.read(reinterpret_cast<char*>(values.data()), bytes);
  if (!stream) throw std::runtime_error("short KITTI scan read: " + path.string());

  Scan scan;
  scan.xyzi.reset(new localization_zoo::aloam::PointCloud);
  scan.xyz.reserve(values.size() / 4);
  scan.xyzi->points.reserve(values.size() / 4);
  for (std::size_t i = 0; i < values.size(); i += 4) {
    const Eigen::Vector3d p(values[i], values[i + 1], values[i + 2]);
    if (!p.array().isFinite().all() || !std::isfinite(values[i + 3])) continue;
    scan.xyz.push_back(p);
    localization_zoo::aloam::PointT point;
    point.x = values[i];
    point.y = values[i + 1];
    point.z = values[i + 2];
    point.intensity = values[i + 3];
    scan.xyzi->points.push_back(point);
  }
  scan.xyzi->width = static_cast<std::uint32_t>(scan.xyzi->points.size());
  scan.xyzi->height = 1;
  return scan;
}

// Top-1 answer for one query. score is "higher is more confident" for every
// method so the evaluator can sweep one threshold direction.
struct Query {
  int match = -1;
  double score = -std::numeric_limits<double>::infinity();
};

class Method {
 public:
  virtual ~Method() = default;
  virtual std::string name() const = 0;
  virtual std::string scoreMeaning() const = 0;
  virtual Query queryThenAdd(const Scan& scan, int frame) = 0;
  double seconds = 0.0;
};

class ScanContextMethod : public Method {
 public:
  explicit ScanContextMethod(int exclude_frames) {
    sc::ScanContextParams params;
    params.exclude_recent_frames = exclude_frames;
    params.distance_threshold = 0.0;  // keep every candidate; evaluator thresholds
    manager_ = std::make_unique<sc::ScanContextManager>(params);
  }
  std::string name() const override { return "scan_context"; }
  std::string scoreMeaning() const override {
    return "negative Scan Context column-cosine distance";
  }
  Query queryThenAdd(const Scan& scan, int) override {
    Query query;
    const auto candidate = manager_->detectLoop(scan.xyz);
    if (candidate.valid) {
      query.match = candidate.index;
      query.score = -candidate.distance;
    }
    manager_->addScan(scan.xyz);
    return query;
  }

 private:
  std::unique_ptr<sc::ScanContextManager> manager_;
};

class IntensityScanContextMethod : public Method {
 public:
  explicit IntensityScanContextMethod(int exclude_frames) {
    isc::IntensityScanContextParams params;
    params.exclude_recent_frames = exclude_frames;
    params.distance_threshold = std::numeric_limits<double>::infinity();
    manager_ = std::make_unique<isc::IntensityScanContextManager>(params);
  }
  std::string name() const override { return "isc"; }
  std::string scoreMeaning() const override {
    return "negative intensity scan context column-cosine distance";
  }
  Query queryThenAdd(const Scan& scan, int) override {
    Query query;
    const auto candidate = manager_->detectLoop(scan.xyzi);
    if (candidate.valid) {
      query.match = candidate.index;
      query.score = -candidate.distance;
    }
    manager_->addScan(scan.xyzi);
    return query;
  }

 private:
  std::unique_ptr<isc::IntensityScanContextManager> manager_;
};

class DtdMethod : public Method {
 public:
  explicit DtdMethod(int exclude_frames) : exclude_frames_(exclude_frames) {
    dtd::DTDParams params;
    params.min_inliers = 0;  // report the verified best frame; evaluator thresholds
    detector_ = std::make_unique<dtd::DTDDetector>(params);
  }
  std::string name() const override { return "dtd"; }
  std::string scoreMeaning() const override {
    return "geometric-verification inlier count";
  }
  Query queryThenAdd(const Scan& scan, int frame) override {
    Query query;
    const auto descriptors = detector_->describe(scan.xyz, frame);
    const auto result =
        detector_->detectLoop(descriptors, frame, exclude_frames_);
    if (result.detected) {
      query.match = result.matched_frame;
      query.score = static_cast<double>(result.inliers);
    }
    detector_->addToDatabase(descriptors);
    return query;
  }

 private:
  int exclude_frames_;
  std::unique_ptr<dtd::DTDDetector> detector_;
};

std::unique_ptr<Method> makeMethod(const std::string& name, int exclude_frames) {
  if (name == "scan_context") return std::make_unique<ScanContextMethod>(exclude_frames);
  if (name == "isc") return std::make_unique<IntensityScanContextMethod>(exclude_frames);
  if (name == "dtd") return std::make_unique<DtdMethod>(exclude_frames);
  throw std::runtime_error("unknown method: " + name +
                           " (expected scan_context, isc, dtd)");
}

std::vector<std::string> splitComma(const std::string& text) {
  std::vector<std::string> parts;
  std::stringstream stream(text);
  std::string part;
  while (std::getline(stream, part, ',')) {
    if (!part.empty()) parts.push_back(part);
  }
  return parts;
}

void usage(const char* program) {
  std::cerr << "Usage: " << program
            << " --velodyne-dir DIR --output-json PATH"
               " [--methods scan_context,isc,dtd] [--exclude-frames 50]"
               " [--max-frames N]\n";
}

}  // namespace

int main(int argc, char** argv) {
  std::string velodyne_dir;
  std::string output_json;
  std::string methods_arg = "scan_context,isc,dtd";
  int exclude_frames = 50;
  int max_frames = -1;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto value = [&]() -> std::string {
      if (i + 1 >= argc) throw std::runtime_error(arg + " requires a value");
      return argv[++i];
    };
    try {
      if (arg == "--velodyne-dir") velodyne_dir = value();
      else if (arg == "--output-json") output_json = value();
      else if (arg == "--methods") methods_arg = value();
      else if (arg == "--exclude-frames") exclude_frames = std::stoi(value());
      else if (arg == "--max-frames") max_frames = std::stoi(value());
      else if (arg == "--help" || arg == "-h") {
        usage(argv[0]);
        return 0;
      } else {
        throw std::runtime_error("unknown argument: " + arg);
      }
    } catch (const std::exception& error) {
      std::cerr << error.what() << "\n";
      usage(argv[0]);
      return 1;
    }
  }
  if (velodyne_dir.empty() || output_json.empty()) {
    usage(argv[0]);
    return 1;
  }

  std::vector<std::unique_ptr<Method>> methods;
  try {
    for (const auto& name : splitComma(methods_arg)) {
      methods.push_back(makeMethod(name, exclude_frames));
    }
  } catch (const std::exception& error) {
    std::cerr << error.what() << "\n";
    return 1;
  }

  auto scans = listScans(velodyne_dir);
  if (max_frames >= 0 && static_cast<int>(scans.size()) > max_frames) {
    scans.resize(static_cast<std::size_t>(max_frames));
  }
  if (scans.empty()) {
    std::cerr << "no .bin scans in " << velodyne_dir << "\n";
    return 1;
  }

  std::vector<std::vector<Query>> results(methods.size());
  for (auto& per_method : results) per_method.reserve(scans.size());
  for (std::size_t frame = 0; frame < scans.size(); ++frame) {
    const Scan scan = loadKittiScan(scans[frame]);
    for (std::size_t m = 0; m < methods.size(); ++m) {
      const auto start = Clock::now();
      results[m].push_back(
          methods[m]->queryThenAdd(scan, static_cast<int>(frame)));
      methods[m]->seconds +=
          std::chrono::duration<double>(Clock::now() - start).count();
    }
    if (frame % 500 == 0) {
      std::cerr << "[place_recognition] " << frame << "/" << scans.size() << "\n";
    }
  }

  std::ofstream out(output_json);
  if (!out) {
    std::cerr << "cannot write " << output_json << "\n";
    return 1;
  }
  out << std::setprecision(9);
  out << "{\n  \"schema_version\": 1,\n";
  out << "  \"velodyne_dir\": \"" << fs::absolute(velodyne_dir).string() << "\",\n";
  out << "  \"frames\": " << scans.size() << ",\n";
  out << "  \"exclude_frames\": " << exclude_frames << ",\n";
  out << "  \"methods\": {\n";
  for (std::size_t m = 0; m < methods.size(); ++m) {
    out << "    \"" << methods[m]->name() << "\": {\n";
    out << "      \"score\": \"" << methods[m]->scoreMeaning() << "\",\n";
    out << "      \"seconds\": " << methods[m]->seconds << ",\n";
    out << "      \"queries\": [";
    for (std::size_t q = 0; q < results[m].size(); ++q) {
      const auto& query = results[m][q];
      if (q != 0) out << ",";
      out << (q % 8 == 0 ? "\n        " : " ");
      if (query.match < 0) {
        out << "[-1, null]";
      } else {
        out << "[" << query.match << ", " << query.score << "]";
      }
    }
    out << "\n      ]\n    }" << (m + 1 < methods.size() ? "," : "") << "\n";
  }
  out << "  }\n}\n";
  std::cerr << "[place_recognition] wrote " << output_json << "\n";
  return 0;
}
