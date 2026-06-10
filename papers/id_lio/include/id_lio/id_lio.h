#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <functional>
#include <unordered_map>
#include <vector>

#include "imu_preintegration/imu_preintegration.h"

namespace localization_zoo {
namespace id_lio {

struct VoxelHash {
  std::size_t operator()(const Eigen::Vector3i& v) const {
    std::size_t seed = 0;
    auto hc = [&seed](int val) {
      seed ^= std::hash<int>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    hc(v.x());
    hc(v.y());
    hc(v.z());
    return seed;
  }
};

class IndexedVoxelMap {
public:
  struct IndexedPoint {
    std::uint64_t id = 0;
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    int observations = 0;
    int last_seen_frame = 0;
    int missing_age = 0;
    int dynamic_age = 0;
    double confidence = 1.0;
    bool removed = false;
  };

  struct Correspondence {
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double weight = 0.0;
    bool found = false;
    bool has_normal = false;
  };

  explicit IndexedVoxelMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addOrUpdatePoints(const std::vector<Eigen::Vector3d>& points,
                         const std::vector<double>& weights, int frame_index,
                         double update_radius);
  Correspondence query(const Eigen::Vector3d& world_point, double max_dist,
                       int normal_min_neighbors, double dynamic_weight) const;
  void pruneFarVoxels(const Eigen::Vector3d& center, double max_distance);
  void pruneDynamic(int delayed_removal_frames);
  void clear() {
    points_.clear();
    voxels_.clear();
  }

  size_t size() const;
  size_t totalIndexedPoints() const { return points_.size(); }
  size_t dynamicCount() const;
  size_t removedCount() const;

  std::vector<IndexedPoint>& points() { return points_; }
  const std::vector<IndexedPoint>& points() const { return points_; }

private:
  Eigen::Vector3i toVoxel(const Eigen::Vector3d& p) const {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / voxel_size_)),
                           static_cast<int>(std::floor(p.y() / voxel_size_)),
                           static_cast<int>(std::floor(p.z() / voxel_size_)));
  }
  double staticWeight(const IndexedPoint& p, double dynamic_weight) const;

  double voxel_size_;
  int max_points_;
  std::uint64_t next_id_ = 1;
  std::vector<IndexedPoint> points_;
  std::unordered_map<Eigen::Vector3i, std::vector<size_t>, VoxelHash> voxels_;
};

struct IDLIOParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  int max_icp_iterations = 30;
  double convergence_criterion = 0.001;
  double max_correspondence_dist = 2.0;

  int range_image_width = 1024;
  int range_image_height = 64;
  double fov_up_deg = 3.0;
  double fov_down_deg = -25.0;
  double new_object_margin = 1.0;
  double disappearance_margin = 1.0;
  double dynamic_weight = 0.15;
  int delayed_removal_frames = 3;
  double map_update_radius = 0.5;

  double gyro_bias_gain = 0.2;
  double max_gyro_bias = 0.5;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct IDLIOResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  int num_dynamic_points = 0;
  int num_removed_points = 0;
  bool used_imu = false;
};

/// Compact ID-LIO-style odometry.
///
/// ID-LIO (Sensors 2023) builds on LIO-SAM and targets dynamic environments with
/// pseudo-occupancy dynamic-point detection, indexed points propagated through
/// keyframes, and delayed removal/dynamic weights in the LiDAR optimization.
/// This port keeps those front-end/map-management mechanisms in a scan-to-map
/// odometry loop. IMU gyro preintegration is used as a rotation prior when a
/// synchronized imu.csv is available; otherwise constant velocity is used.
class IDLIOPipeline {
public:
  explicit IDLIOPipeline(const IDLIOParams& params = IDLIOParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }

  IDLIOResult registerFrame(
      const std::vector<Eigen::Vector3d>& frame,
      const std::vector<localization_zoo::imu_preintegration::ImuSample>& imu);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return map_.size(); }
  size_t indexedPoints() const { return map_.totalIndexedPoints(); }
  size_t dynamicMapPoints() const { return map_.dynamicCount(); }

private:
  struct RangeImage {
    std::vector<double> ranges;
    std::vector<bool> valid;
  };

  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  bool projectToRangeImage(const Eigen::Vector3d& p_sensor, int* idx,
                           double* range = nullptr) const;
  RangeImage buildCurrentRangeImage(
      const std::vector<Eigen::Vector3d>& source) const;
  RangeImage buildMapRangeImage(const Eigen::Matrix4d& sensor_to_world) const;
  std::vector<double> classifySourceWeights(
      const std::vector<Eigen::Vector3d>& source,
      const RangeImage& map_ranges, int* dynamic_points) const;
  void markMissingMapPoints(const RangeImage& current_ranges,
                            const Eigen::Matrix4d& sensor_to_world);
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const std::vector<double>& source_weights,
                                  const Eigen::Matrix4d& initial_guess,
                                  IDLIOResult* result) const;

  IDLIOParams params_;
  IndexedVoxelMap map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  Eigen::Vector3d gyro_bias_ = Eigen::Vector3d::Zero();
  int frame_count_ = 0;
};

}  // namespace id_lio
}  // namespace localization_zoo
