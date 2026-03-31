#pragma once

#include "aloam/types.h"
#include "imu_preintegration/imu_preintegration.h"
#include "relead/cesikf.h"

#include <deque>
#include <vector>

namespace localization_zoo {
namespace lins {

struct LINSParams {
  relead::CESIKFParams filter;

  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.5;
  double map_voxel_size = 0.8;
  int knn = 5;
  double max_correspondence_distance = 3.0;
  double plane_threshold = 0.2;
  int max_correspondences = 200;

  double keyframe_translation_threshold = 0.6;
  double keyframe_rotation_threshold_rad = 8.0 * 3.14159265358979323846 / 180.0;
  int max_keyframes_in_map = 30;
};

struct LINSResult {
  relead::State state;
  relead::DegeneracyInfo degeneracy_info;
  bool initialized = false;
  bool valid = false;
  bool converged = false;
  bool keyframe_added = false;
  int num_keyframes = 0;
  int num_correspondences = 0;
  int num_iterations = 0;
  size_t map_points = 0;
};

class LINS {
public:
  explicit LINS(const LINSParams& params = LINSParams());

  LINSResult process(
      const aloam::PointCloudConstPtr& cloud,
      const std::vector<imu_preintegration::ImuSample>& imu_samples = {});
  void clear();

  const relead::State& state() const { return filter_.state(); }
  int numKeyframes() const { return static_cast<int>(keyframes_.size()); }
  size_t mapSize() const { return map_points_.size(); }

private:
  std::vector<Eigen::Vector3d> preprocess(
      const aloam::PointCloudConstPtr& cloud, double voxel_size) const;
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

  LINSParams params_;
  relead::CESIKF filter_;
  std::deque<std::vector<Eigen::Vector3d>> keyframes_;
  std::vector<Eigen::Vector3d> map_points_;
  aloam::PointCloudPtr map_cloud_;
  relead::State last_keyframe_state_;
  bool initialized_ = false;
};

}  // namespace lins
}  // namespace localization_zoo
