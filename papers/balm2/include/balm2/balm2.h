#pragma once

#include "aloam/laser_odometry.h"
#include "aloam/scan_registration.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <vector>

namespace localization_zoo {
namespace balm2 {

struct Balm2Params {
  aloam::ScanRegistrationParams scan_registration;
  aloam::LaserOdometryParams odometry;

  double corner_resolution = 0.3;
  double surface_resolution = 0.6;
  int corner_stride = 2;
  int surface_stride = 3;

  int window_size = 5;
  int num_outer_iterations = 2;
  int ceres_max_iterations = 6;
  int knn = 5;

  double knn_max_dist_sq = 4.0;
  double edge_eigenvalue_ratio = 3.0;
  double plane_threshold = 0.2;
  double huber_loss_s = 0.1;

  double odom_rotation_weight = 40.0;
  double odom_translation_weight = 20.0;
};

struct Balm2Result {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  int num_keyframes = 0;
  int num_window_keyframes = 0;
  int num_line_constraints = 0;
  int num_plane_constraints = 0;
  bool valid = false;
};

class BALM2 {
public:
  explicit BALM2(const Balm2Params& params = Balm2Params());

  Balm2Result process(const aloam::PointCloudConstPtr& cloud);
  void clear();

  int numKeyframes() const { return static_cast<int>(keyframes_.size()); }
  int currentWindowSize() const;

private:
  struct PoseState {
    Eigen::Quaterniond q = Eigen::Quaterniond::Identity();
    Eigen::Vector3d t = Eigen::Vector3d::Zero();
  };

  struct Keyframe {
    PoseState raw_pose;
    PoseState optimized_pose;
    aloam::PointCloudPtr corner_points;
    aloam::PointCloudPtr surface_points;
  };

  static PoseState relativePose(const PoseState& from, const PoseState& to);
  static PoseState composePose(const PoseState& a, const PoseState& b);
  static PoseState invertPose(const PoseState& pose);

  aloam::PointCloudPtr prepareCornerCloud(
      const aloam::FeatureCloud& features) const;
  aloam::PointCloudPtr prepareSurfaceCloud(
      const aloam::FeatureCloud& features) const;

  void optimizeWindow(Balm2Result* result);

  Balm2Params params_;
  aloam::ScanRegistration scan_registration_;
  aloam::LaserOdometry odometry_;
  std::vector<Keyframe> keyframes_;
};

}  // namespace balm2
}  // namespace localization_zoo
