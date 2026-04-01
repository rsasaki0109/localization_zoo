#pragma once

#include "ct_icp/types.h"
#include "imu_preintegration/imu_preintegration.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <deque>
#include <vector>

namespace localization_zoo {
namespace fast_lio2 {

struct FastLio2Params {
  int max_iterations = 8;
  int ceres_max_iterations = 6;
  double voxel_resolution = 1.0;
  double keypoint_voxel_size = 0.5;
  int knn = 5;
  double max_correspondence_distance = 3.0;
  double planarity_threshold = 0.1;
  double convergence_threshold = 0.001;
  double cauchy_loss_param = 0.3;

  double imu_rotation_weight = 4.0;
  double imu_translation_weight = 0.5;
  double imu_velocity_weight = 0.5;
  double velocity_prior_weight = 0.2;

  int max_frames_in_map = 30;
  Eigen::Vector3d gravity = Eigen::Vector3d(0.0, 0.0, -9.81);
  imu_preintegration::ImuNoiseParams imu_noise;
};

struct FastLio2State {
  Eigen::Quaterniond quat = Eigen::Quaterniond::Identity();
  Eigen::Vector3d trans = Eigen::Vector3d::Zero();
  Eigen::Vector3d velocity = Eigen::Vector3d::Zero();
  Eigen::Vector3d gyro_bias = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel_bias = Eigen::Vector3d::Zero();
};

struct FastLio2Result {
  FastLio2State state;
  imu_preintegration::PreintegrationResult preintegration;
  int num_iterations = 0;
  int num_correspondences = 0;
  bool converged = false;
  bool valid = false;
  bool imu_used = false;
  bool initialized = false;
  size_t map_size = 0;
};

class FastLio2 {
public:
  explicit FastLio2(const FastLio2Params& params = FastLio2Params())
      : params_(params) {}

  FastLio2Result process(
      const std::vector<Eigen::Vector3d>& raw_points,
      const std::vector<imu_preintegration::ImuSample>& imu_samples = {});

  void clear();

  const FastLio2State& state() const { return state_; }
  FastLio2State& state() { return state_; }
  const ct_icp::VoxelHashMap& map() const { return voxel_map_; }
  size_t mapSize() const { return voxel_map_.size(); }
  size_t numFramesInMap() const { return frame_maps_.size(); }
  bool initialized() const { return initialized_; }

private:
  struct Correspondence {
    int point_idx = 0;
    Eigen::Vector3d reference = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double weight = 0.0;
  };

  static ct_icp::Voxel pointToVoxel(const Eigen::Vector3d& point,
                                    double resolution);
  static ct_icp::VoxelHashMap buildFrameMap(
      const std::vector<Eigen::Vector3d>& world_points,
      double resolution);
  static void mergeFrameMap(const ct_icp::VoxelHashMap& frame_map,
                            ct_icp::VoxelHashMap* voxel_map);
  void rebuildMapFromWindow();
  void addPointsToMap(const std::vector<Eigen::Vector3d>& world_points);

  std::vector<Eigen::Vector3d> subsamplePoints(
      const std::vector<Eigen::Vector3d>& raw_points) const;
  std::vector<Correspondence> findCorrespondences(
      const std::vector<Eigen::Vector3d>& keypoints,
      const Eigen::Quaterniond& quat,
      const Eigen::Vector3d& trans) const;
  void predictFromImu(
      const FastLio2State& previous_state,
      const imu_preintegration::PreintegrationResult& preintegration,
      FastLio2State* predicted_state) const;
  static std::vector<Eigen::Vector3d> transformPoints(
      const std::vector<Eigen::Vector3d>& raw_points,
      const Eigen::Quaterniond& quat,
      const Eigen::Vector3d& trans);

  FastLio2Params params_;
  ct_icp::VoxelHashMap voxel_map_;
  std::deque<ct_icp::VoxelHashMap> frame_maps_;
  FastLio2State state_;
  bool initialized_ = false;
};

}  // namespace fast_lio2
}  // namespace localization_zoo
