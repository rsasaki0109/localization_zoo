#include "hdl_graph_slam/hdl_graph_slam.h"
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

std::vector<Eigen::Matrix4d> loadPoses(const fs::path& path) {
  std::ifstream stream(path);
  if (!stream) throw std::runtime_error("cannot open raw poses");
  std::vector<Eigen::Matrix4d> poses;
  while (stream) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    for (int row = 0; row < 3; ++row) {
      for (int column = 0; column < 4; ++column) {
        if (!(stream >> pose(row, column))) return poses;
      }
    }
    poses.push_back(pose);
  }
  return poses;
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
  if (!stream) throw std::runtime_error("short scan read: " + path.string());
  std::vector<Eigen::Vector3d> points;
  points.reserve(values.size() / 4);
  for (std::size_t index = 0; index < values.size(); index += 4) {
    const Eigen::Vector3d point(values[index], values[index + 1],
                                values[index + 2]);
    if (point.array().isFinite().all()) points.push_back(point);
  }
  return points;
}

std::vector<Eigen::Vector3d> preprocess(
    const std::vector<Eigen::Vector3d>& points) {
  std::map<VoxelKey, VoxelAccumulator> voxels;
  for (const auto& point : points) {
    const VoxelKey key{static_cast<int>(std::floor(point.z() / 0.8)),
                       static_cast<int>(std::floor(point.y() / 0.8)),
                       static_cast<int>(std::floor(point.x() / 0.8))};
    auto& accumulator = voxels[key];
    accumulator.sum += point;
    ++accumulator.count;
  }
  std::vector<Eigen::Vector3d> filtered;
  filtered.reserve(voxels.size());
  for (const auto& [key, accumulator] : voxels) {
    (void)key;
    const Eigen::Vector3d point =
        accumulator.sum / static_cast<double>(accumulator.count);
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
  localization_zoo::kiss_icp::correctElevationAngle(
      filtered, 0.205 * std::acos(-1.0) / 180.0);
  return filtered;
}

localization_zoo::aloam::PointCloudPtr makeCloud(
    const std::vector<Eigen::Vector3d>& points) {
  auto cloud = localization_zoo::aloam::PointCloudPtr(
      new localization_zoo::aloam::PointCloud);
  cloud->reserve(points.size());
  for (const auto& point : points) {
    localization_zoo::aloam::PointT pcl_point;
    pcl_point.x = static_cast<float>(point.x());
    pcl_point.y = static_cast<float>(point.y());
    pcl_point.z = static_cast<float>(point.z());
    pcl_point.intensity = 0.0f;
    cloud->push_back(pcl_point);
  }
  return cloud;
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
  if (argc != 5 && argc != 6) {
    std::cerr << "usage: kiss_bin_pose_graph SCAN_DIR RAW_POSES "
                 "OUTPUT_POSES MANIFEST_JSON [MAX_FRAMES]\n";
    return 2;
  }
  auto scans = listScans(argv[1]);
  const auto raw_poses = loadPoses(argv[2]);
  if (argc == 6) {
    const std::size_t max_frames = static_cast<std::size_t>(std::stoull(argv[5]));
    scans.resize(std::min(scans.size(), max_frames));
  }
  if (scans.empty() || raw_poses.size() < scans.size()) {
    throw std::runtime_error("raw pose count must cover selected scans");
  }

  localization_zoo::hdl_graph_slam::HdlGraphSlamParams params;
  params.registration_voxel_size = 0.8;
  params.map_voxel_size = 0.8;
  params.keyframe_stride = 10;
  params.keyframe_translation_threshold =
      std::numeric_limits<double>::infinity();
  params.keyframe_rotation_threshold_rad =
      std::numeric_limits<double>::infinity();
  params.min_loop_index_gap = 30;
  params.scan_context.exclude_recent_frames = 30;
  params.scan_context.distance_threshold = 0.18;
  params.scan_context.num_candidates = 5;
  params.loop_gicp.max_correspondence_distance = 5.0;
  params.loop_gicp.max_iterations = 60;
  params.loop_fitness_threshold = std::numeric_limits<double>::infinity();
  params.loop_rmse_threshold = 1.0;
  params.min_loop_correspondences = 80;
  params.loop_submap_half_window = 2;
  params.min_loop_clusters_for_correction = 2;
  params.loop_cluster_keyframe_radius = 5;
  params.descriptor_stride = 1;
  params.loop_stride = 2;
  params.enable_floor_constraint = false;
  params.optimize_every_n_keyframes = 0;
  params.odom_rotation_weight = 40.0;
  params.odom_translation_weight = 20.0;
  params.loop_rotation_weight = 200.0;
  params.loop_translation_weight = 100.0;
  localization_zoo::hdl_graph_slam::HdlGraphSlam backend(params);

  std::ofstream output(argv[3]);
  if (!output) throw std::runtime_error("cannot open output poses");
  double algorithm_seconds = 0.0;
  std::size_t scans_loaded = 0;
  localization_zoo::aloam::PointCloudPtr cloud;
  const auto total_start = std::chrono::steady_clock::now();
  for (std::size_t index = 0; index < scans.size(); ++index) {
    const bool keyframe_scan_required =
        !cloud || params.keyframe_stride <= 0 ||
        (index + 1) % static_cast<std::size_t>(params.keyframe_stride) == 0;
    if (keyframe_scan_required) {
      cloud = makeCloud(preprocess(loadKittiScan(scans[index])));
      ++scans_loaded;
    }
    const auto algorithm_start = std::chrono::steady_clock::now();
    const auto result = backend.processExternalOdometry(cloud, raw_poses[index]);
    algorithm_seconds += std::chrono::duration<double>(
        std::chrono::steady_clock::now() - algorithm_start).count();
    writePose(output, result.pose);
    if (index % 100 == 0) {
      std::cerr << "\r[KISS-bin-PG] " << index << '/' << scans.size()
                << " keyframes=" << backend.numKeyframes()
                << " loops=" << backend.numLoopEdges()
                << " clusters=" << backend.numLoopClusters();
    }
  }
  output.close();
  const double total_seconds = std::chrono::duration<double>(
      std::chrono::steady_clock::now() - total_start).count();

  std::ofstream manifest(argv[4]);
  manifest << std::setprecision(15)
           << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"method\": \"dynamic_radius_causal_pose_graph\",\n"
           << "  \"ground_truth_used\": false,\n"
           << "  \"causality\": \"output i uses scans and poses only through i\",\n"
           << "  \"frames\": " << scans.size() << ",\n"
           << "  \"scans_loaded\": " << scans_loaded << ",\n"
           << "  \"keyframes\": " << backend.numKeyframes() << ",\n"
           << "  \"loop_edges\": " << backend.numLoopEdges() << ",\n"
           << "  \"loop_clusters\": " << backend.numLoopClusters() << ",\n"
           << "  \"loop_candidates\": " << backend.loopCandidateDetections()
           << ",\n"
           << "  \"loop_registration_rejections\": "
           << backend.loopRegistrationRejections() << ",\n"
           << "  \"loop_gicp_max_iterations\": "
           << params.loop_gicp.max_iterations << ",\n"
           << "  \"correction_enabled\": "
           << (backend.loopCorrectionEnabled() ? "true" : "false") << ",\n"
           << "  \"algorithm_seconds\": " << algorithm_seconds << ",\n"
           << "  \"total_seconds\": " << total_seconds << ",\n"
           << "  \"algorithm_fps\": " << scans.size() / algorithm_seconds
           << ",\n"
           << "  \"total_fps\": " << scans.size() / total_seconds << ",\n"
           << "  \"loop_attempts\": [\n";
  const auto& loop_attempts = backend.loopAttempts();
  for (std::size_t index = 0; index < loop_attempts.size(); ++index) {
    const auto& attempt = loop_attempts[index];
    const auto scalar = [](double value) {
      return std::isfinite(value) ? std::to_string(value) : "null";
    };
    manifest << "    {\"from\": " << attempt.from
             << ", \"to\": " << attempt.to
             << ", \"descriptor_distance\": "
             << scalar(attempt.descriptor_distance)
             << ", \"converged\": "
             << (attempt.converged ? "true" : "false")
             << ", \"fitness\": " << scalar(attempt.fitness)
             << ", \"rmse\": " << scalar(attempt.rmse)
             << ", \"correspondences\": " << attempt.correspondences
             << ", \"accepted\": "
             << (attempt.accepted ? "true" : "false") << "}"
             << (index + 1 < loop_attempts.size() ? "," : "") << '\n';
  }
  manifest << "  ]\n}\n";
  std::cerr << "\r[KISS-bin-PG] " << scans.size() << '/' << scans.size()
            << " loops=" << backend.numLoopEdges()
            << " clusters=" << backend.numLoopClusters() << '\n';
  return 0;
}
