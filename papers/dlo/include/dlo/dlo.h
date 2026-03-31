#pragma once

#include "aloam/types.h"
#include "gicp/gicp_registration.h"

#include <Eigen/Core>

#include <deque>
#include <vector>

namespace localization_zoo {
namespace dlo {

struct DLOParams {
  gicp::GICPParams gicp;

  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.5;
  double map_voxel_size = 0.8;

  double keyframe_translation_threshold = 0.6;
  double keyframe_rotation_threshold_rad = 8.0 * 3.14159265358979323846 / 180.0;
  int max_keyframes_in_map = 30;
};

struct DLOResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool initialized = false;
  bool valid = false;
  bool converged = false;
  bool keyframe_added = false;
  int num_keyframes = 0;
  int num_correspondences = 0;
  int num_iterations = 0;
  double fitness = 0.0;
  size_t map_points = 0;
};

class DLO {
public:
  explicit DLO(const DLOParams& params = DLOParams());

  DLOResult process(const aloam::PointCloudConstPtr& cloud);
  void clear();

  const Eigen::Matrix4d& pose() const { return pose_; }
  int numKeyframes() const { return static_cast<int>(keyframes_.size()); }
  size_t mapSize() const { return map_points_.size(); }

private:
  std::vector<Eigen::Vector3d> preprocess(
      const aloam::PointCloudConstPtr& cloud, double voxel_size) const;
  void rebuildMap();
  bool shouldAddKeyframe(const Eigen::Matrix4d& candidate_pose) const;
  void addKeyframe(const std::vector<Eigen::Vector3d>& map_points,
                   const Eigen::Matrix4d& pose);

  static std::vector<Eigen::Vector3d> transformPoints(
      const std::vector<Eigen::Vector3d>& points, const Eigen::Matrix4d& pose);
  static double rotationAngle(const Eigen::Matrix4d& transform);

  DLOParams params_;
  gicp::GICPRegistration registration_;
  std::deque<std::vector<Eigen::Vector3d>> keyframes_;
  std::vector<Eigen::Vector3d> map_points_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_keyframe_pose_ = Eigen::Matrix4d::Identity();
  bool initialized_ = false;
};

}  // namespace dlo
}  // namespace localization_zoo
