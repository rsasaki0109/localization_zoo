#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "imu_preintegration/imu_preintegration.h"

namespace localization_zoo {
namespace fr_lio {

/// Robocentric two-layer voxel map (FR-LIO RC-Vox, simplified from-paper).
class RCVoxelMap {
 public:
  struct PlaneMatch {
    Eigen::Vector3d normal = Eigen::Vector3d::UnitZ();
    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
    bool valid = false;
  };

  explicit RCVoxelMap(double lidar_range = 40.0, double grid_size = 2.0,
                      double voxel_size = 0.5, double lambda = 1.0,
                      int max_points_per_voxel = 12);

  void initialize(const Eigen::Vector3d& robot_position);
  void updateRobot(const Eigen::Vector3d& robot_position);
  void addPoints(const std::vector<Eigen::Vector3d>& world_points);
  PlaneMatch matchPointToPlane(const Eigen::Vector3d& query,
                               double max_dist, int k_neighbors = 5) const;

  size_t activeGrids() const { return grids_.size(); }
  void clear();

 private:
  struct VoxelCell {
    std::vector<Eigen::Vector3d> points;
    std::vector<Eigen::Vector3d> neighbor_cache;
  };

  struct BottomLevel {
    int dim = 0;
    std::unordered_map<int, VoxelCell> voxels;
  };

  int wrapIndex(int v) const;
  Eigen::Vector3i wrapVec(const Eigen::Vector3i& v) const;
  int64_t gridKey(const Eigen::Vector3i& it) const;
  int voxelKey(const Eigen::Vector3i& ib) const;
  Eigen::Vector3i tlaIndexForPoint(const Eigen::Vector3d& p) const;
  Eigen::Vector3i blaIndexForPoint(const Eigen::Vector3d& p,
                                   const Eigen::Vector3i& it) const;
  BottomLevel& gridAt(const Eigen::Vector3i& it);
  const VoxelCell* voxelAt(const Eigen::Vector3i& it,
                           const Eigen::Vector3i& ib) const;
  void insertIntoVoxel(BottomLevel& bl, const Eigen::Vector3i& ib,
                       const Eigen::Vector3d& p, bool cache_neighbors);

  double lidar_range_;
  double grid_size_;
  double voxel_size_;
  double lambda_;
  int max_points_per_voxel_;
  int tla_dim_;
  int bla_dim_;

  Eigen::Vector3d t_init_ = Eigen::Vector3d::Zero();
  Eigen::Vector3i map_origin_tla_ = Eigen::Vector3i::Zero();
  std::unordered_map<int64_t, BottomLevel> grids_;
};

struct FRLIOParams {
  double source_voxel_size = 0.5;
  double voxel_size = 0.5;
  double grid_size = 2.0;
  double lidar_range = 40.0;
  double map_lambda = 1.0;
  double min_range = 1.0;
  double max_range = 80.0;
  int max_icp_iterations = 20;
  double max_correspondence_dist = 1.5;
  int min_plane_points = 5;
  int max_subframes = 4;
  double sigma_acc_max = 2.0;
  double sigma_gyr_max = 1.0;
  double min_covariance_eig = 1e-4;
  int min_valid_matches = 30;
  double esks_gain = 0.5;
  int max_points_per_voxel = 12;
};

struct FRLIOResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int subframes = 1;
  int smoothed_subframes = 0;
  bool used_imu = false;
  bool map_updated = true;
};

/// FR-LIO: adaptive sub-frames + simplified ESKS + RC-Vox scan-to-map LIO.
class FRLIOPipeline {
 public:
  explicit FRLIOPipeline(const FRLIOParams& params = FRLIOParams());

  void setInitialPose(const Eigen::Matrix4d& pose);

  FRLIOResult registerFrame(
      const std::vector<Eigen::Vector3d>& frame,
      const std::vector<imu_preintegration::ImuSample>& imu);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapGrids() const { return map_.activeGrids(); }

  static int computeSubframeCount(
      const std::vector<imu_preintegration::ImuSample>& imu,
      const FRLIOParams& params);

 private:
  std::vector<Eigen::Vector3d> filterRange(
      const std::vector<Eigen::Vector3d>& points) const;
  std::vector<Eigen::Vector3d> downsample(
      const std::vector<Eigen::Vector3d>& points, double leaf) const;
  std::vector<std::vector<Eigen::Vector3d>> splitSubframes(
      const std::vector<Eigen::Vector3d>& points, int n_sub) const;
  std::vector<std::vector<imu_preintegration::ImuSample>> splitImu(
      const std::vector<imu_preintegration::ImuSample>& imu, int n_sub) const;
  Eigen::Matrix4d predictMotion(
      const std::vector<imu_preintegration::ImuSample>& imu,
      bool* used_imu) const;
  Eigen::Matrix4d alignSubframe(
      const std::vector<Eigen::Vector3d>& points,
      const Eigen::Matrix4d& initial_guess, int* valid_matches) const;
  void backwardSmooth(std::vector<Eigen::Matrix4d>& poses,
                      const std::vector<Eigen::Matrix4d>& predictions) const;
  bool constraintOk(int valid_matches) const;

  FRLIOParams params_;
  RCVoxelMap map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  int frame_count_ = 0;
};

}  // namespace fr_lio
}  // namespace localization_zoo
