#pragma once

#include "ct_icp/types.h"
#include "imu_preintegration/imu_preintegration.h"

#include <deque>
#include <vector>

namespace localization_zoo {
namespace ct_lio {

struct CTLIOParams {
  int max_iterations = 20;
  int ceres_max_iterations = 5;
  double voxel_resolution = 1.0;
  int knn = 5;
  double max_correspondence_dist = 100.0;
  double planarity_threshold = 0.3;
  double convergence_threshold = 0.001;
  double cauchy_loss_param = 0.5;
  double keypoint_voxel_size = 0.5;

  double imu_rotation_weight = 2.0;
  double imu_translation_weight = 2.0;
  double velocity_prior_weight = 0.2;
  double gyro_bias_prior_weight = 0.02;
  double accel_bias_prior_weight = 0.02;
  bool estimate_imu_bias = false;
  int fixed_lag_state_window = 1;
  double fixed_lag_velocity_prior_weight = 0.0;
  double fixed_lag_gyro_bias_weight_scale = 0.0;
  double fixed_lag_accel_bias_weight_scale = 0.0;
  double fixed_lag_history_decay = 1.0;
  int fixed_lag_outer_iterations = 3;
  bool fixed_lag_optimize_history = false;

  int max_frames_in_map = 30;
  Eigen::Vector3d gravity = Eigen::Vector3d(0.0, 0.0, -9.81);
};

struct CTLIOState {
  ct_icp::TrajectoryFrame frame;
  Eigen::Vector3d begin_velocity = Eigen::Vector3d::Zero();
  Eigen::Vector3d end_velocity = Eigen::Vector3d::Zero();
  Eigen::Vector3d gyro_bias = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel_bias = Eigen::Vector3d::Zero();
};

struct CTLIOResult {
  CTLIOState state;
  imu_preintegration::PreintegrationResult preintegration;
  int num_iterations = 0;
  int num_correspondences = 0;
  bool converged = false;
};

class CTLIORegistration {
public:
  explicit CTLIORegistration(const CTLIOParams& params = CTLIOParams())
      : params_(params) {}

  void addPointsToMap(const std::vector<Eigen::Vector3d>& world_points);
  void clearMap();

  const ct_icp::VoxelHashMap& map() const { return voxel_map_; }
  size_t mapSize() const { return voxel_map_.size(); }
  size_t numFramesInMap() const { return frame_maps_.size(); }
  size_t stateHistorySize() const { return state_history_.size(); }

  CTLIOResult registerFrame(
      const std::vector<ct_icp::TimedPoint>& timed_points,
      const CTLIOState& initial_state,
      const std::vector<imu_preintegration::ImuSample>& imu_samples);

private:
  struct Correspondence {
    int point_idx = 0;
    Eigen::Vector3d reference = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double weight = 0.0;
  };
  struct LaggedStatePrior {
    bool valid = false;
    Eigen::Vector3d velocity = Eigen::Vector3d::Zero();
    Eigen::Vector3d gyro_bias = Eigen::Vector3d::Zero();
    Eigen::Vector3d accel_bias = Eigen::Vector3d::Zero();
  };
  struct LaggedStateEntry {
    CTLIOState state;
    imu_preintegration::PreintegrationResult preintegration;
    Eigen::Vector3d reference_gyro_bias = Eigen::Vector3d::Zero();
    Eigen::Vector3d reference_accel_bias = Eigen::Vector3d::Zero();
    std::vector<ct_icp::TimedPoint> keypoints;
  };

  std::vector<ct_icp::TimedPoint> subsampleKeypoints(
      const std::vector<ct_icp::TimedPoint>& points) const;
  std::vector<Correspondence> findCorrespondences(
      const std::vector<ct_icp::TimedPoint>& keypoints,
      const ct_icp::TrajectoryFrame& frame) const;
  ct_icp::VoxelHashMap buildFrameMap(
      const std::vector<Eigen::Vector3d>& world_points) const;
  void rebuildMapFromWindow();
  void initializeFromImu(
      const imu_preintegration::PreintegrationResult& preintegration,
      CTLIOState* state) const;
  LaggedStatePrior computeLaggedStatePrior() const;
  void rememberState(const CTLIOState& state,
                     const imu_preintegration::PreintegrationResult& preintegration,
                     const Eigen::Vector3d& reference_gyro_bias,
                     const Eigen::Vector3d& reference_accel_bias,
                     std::vector<ct_icp::TimedPoint> keypoints);
  void optimizeLaggedHistory();

  CTLIOParams params_;
  ct_icp::VoxelHashMap voxel_map_;
  std::deque<ct_icp::VoxelHashMap> frame_maps_;
  std::deque<LaggedStateEntry> state_history_;
};

}  // namespace ct_lio
}  // namespace localization_zoo
