#pragma once

#include "imu_preintegration/imu_preintegration.h"
#include "relead/cesikf.h"

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <deque>
#include <vector>

namespace localization_zoo {
namespace point_lio {

struct PointLioParams {
  relead::CESIKFParams filter;

  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.4;
  double map_voxel_size = 0.7;
  int knn = 5;
  double max_correspondence_distance = 3.0;
  double planarity_threshold = 0.08;
  int max_correspondences = 250;

  double keyframe_translation_threshold = 0.6;
  double keyframe_rotation_threshold_rad = 8.0 * 3.14159265358979323846 / 180.0;
  int max_keyframes_in_map = 30;
};

struct PointLioResult {
  relead::State state;
  relead::DegeneracyInfo degeneracy_info;
  bool initialized = false;
  bool valid = false;
  bool converged = false;
  bool keyframe_added = false;
  bool imu_used = false;
  int num_keyframes = 0;
  int num_correspondences = 0;
  int num_iterations = 0;
  size_t map_points = 0;
};

class PointLio {
public:
  explicit PointLio(const PointLioParams& params = PointLioParams());

  PointLioResult process(
      const std::vector<Eigen::Vector3d>& raw_points,
      const std::vector<imu_preintegration::ImuSample>& imu_samples = {});
  void clear();

  const relead::State& state() const { return filter_.state(); }
  int numKeyframes() const { return static_cast<int>(keyframes_.size()); }
  size_t mapSize() const { return map_points_.size(); }

private:
  std::vector<Eigen::Vector3d> preprocess(
      const std::vector<Eigen::Vector3d>& raw_points, double voxel_size) const;
  void propagateImu(const std::vector<imu_preintegration::ImuSample>& imu_samples);
  std::vector<relead::PointWithNormal> buildCorrespondences(
      const std::vector<Eigen::Vector3d>& points_world) const;
  void rebuildMap();
  bool shouldAddKeyframe(const relead::State& candidate_state) const;
  void addKeyframe(const std::vector<Eigen::Vector3d>& points_world,
                   const relead::State& state);

  static std::vector<Eigen::Vector3d> transformPoints(
      const std::vector<Eigen::Vector3d>& points, const relead::State& state);
  static double rotationAngle(const Eigen::Matrix3d& rotation);

  PointLioParams params_;
  relead::CESIKF filter_;
  std::deque<std::vector<Eigen::Vector3d>> keyframes_;
  std::vector<Eigen::Vector3d> map_points_;
  pcl::PointCloud<pcl::PointXYZ>::Ptr map_cloud_;
  relead::State last_keyframe_state_;
  bool initialized_ = false;
};

}  // namespace point_lio
}  // namespace localization_zoo
