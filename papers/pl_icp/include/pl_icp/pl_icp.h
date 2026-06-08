#pragma once

#include <Eigen/Core>

#include <cstddef>
#include <vector>

namespace localization_zoo {
namespace pl_icp {

/// Planar laser scan (ROS LaserScan-compatible layout).
struct LaserScan {
  double angle_min = -3.141592653589793;
  double angle_max = 3.141592653589793;
  double angle_increment = 0.0;
  double range_min = 0.1;
  double range_max = 100.0;
  double timestamp = 0.0;
  std::vector<double> ranges;

  size_t size() const { return ranges.size(); }
};

struct PLICPParams {
  int max_iterations = 30;
  double max_correspondence_distance = 0.5;
  double convergence_translation = 1e-4;
  double convergence_rotation = 1e-4;
  double min_range = 0.1;
  double max_range = 30.0;
  double max_neighbor_gap = 2.0;
  int min_correspondences = 20;
  bool use_motion_prior = true;
};

struct PLICPResult {
  Eigen::Matrix3d increment = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d pose = Eigen::Matrix3d::Identity();
  bool valid = false;
  int iterations = 0;
};

/// PL-ICP 2D scan matching (Censi, IROS 2008, simplified scan-to-scan odometry).
class PLICPEstimator {
 public:
  explicit PLICPEstimator(const PLICPParams& params = PLICPParams());

  void reset();
  void setInitialPose(const Eigen::Matrix3d& pose);

  PLICPResult registerScan(const LaserScan& scan);

  const Eigen::Matrix3d& pose() const { return pose_; }

 private:
  struct RefPoint {
    Eigen::Vector2d point = Eigen::Vector2d::Zero();
    Eigen::Vector2d normal = Eigen::Vector2d::UnitX();
    bool valid_line = false;
    bool valid_point = false;
  };

  std::vector<Eigen::Vector2d> scanToPoints(const LaserScan& scan) const;
  std::vector<RefPoint> buildReferenceModel(const LaserScan& scan) const;
  bool solveIncrement(const std::vector<Eigen::Vector2d>& current,
                      const Eigen::Matrix3d& transform, Eigen::Matrix3d* increment) const;
  static Eigen::Vector2d transformPoint(const Eigen::Matrix3d& T, const Eigen::Vector2d& p);

  PLICPParams params_;
  bool initialized_ = false;
  std::vector<RefPoint> ref_model_;
  Eigen::Matrix3d pose_ = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d last_increment_ = Eigen::Matrix3d::Identity();
};

Eigen::Matrix3d pose2D(double x, double y, double yaw);

}  // namespace pl_icp
}  // namespace localization_zoo
