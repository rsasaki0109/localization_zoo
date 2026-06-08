#pragma once

#include <Eigen/Core>

#include <cstddef>
#include <optional>
#include <vector>

namespace localization_zoo {
namespace kinematic_icp {

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

/// Wheel odometry increment (forward arc length + yaw) between consecutive scans.
struct WheelOdom {
  double forward_m = 0.0;
  double yaw_rad = 0.0;
};

struct KinematicICPParams {
  int max_iterations = 30;
  double max_correspondence_distance = 1.5;
  double convergence_translation = 1e-4;
  double convergence_rotation = 1e-4;
  double min_range = 0.1;
  double max_range = 30.0;
  double max_neighbor_gap = 2.0;
  int min_correspondences = 20;
  double wheel_odom_weight = 5.0;
  bool use_last_increment_as_wheel_odom = true;
};

struct KinematicICPResult {
  Eigen::Matrix3d increment = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d pose = Eigen::Matrix3d::Identity();
  bool valid = false;
  int iterations = 0;
  double forward_m = 0.0;
  double yaw_rad = 0.0;
};

/// Kinematic-ICP 2D scan odometry (Guadagnino et al., ICRA 2025, simplified 2D port).
class KinematicICPEstimator {
 public:
  explicit KinematicICPEstimator(const KinematicICPParams& params = KinematicICPParams());

  void reset();
  void setInitialPose(const Eigen::Matrix3d& pose);

  KinematicICPResult registerScan(const LaserScan& scan,
                                  const std::optional<WheelOdom>& wheel_odom = std::nullopt);

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
  static WheelOdom incrementToWheelOdom(const Eigen::Matrix3d& T);
  static Eigen::Matrix3d wheelOdomToIncrement(double forward_m, double yaw_rad);
  static Eigen::Vector2d transformPoint(const Eigen::Matrix3d& T, const Eigen::Vector2d& p);
  bool solveKinematicIncrement(const std::vector<Eigen::Vector2d>& current,
                             const Eigen::Matrix3d& transform, const WheelOdom& wheel_prior,
                             Eigen::Matrix3d* increment) const;

  static Eigen::Matrix3d projectToUnicycle(const Eigen::Matrix3d& T);

  KinematicICPParams params_;
  bool initialized_ = false;
  std::vector<RefPoint> ref_model_;
  Eigen::Matrix3d pose_ = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d last_increment_ = Eigen::Matrix3d::Identity();
};

Eigen::Matrix3d pose2D(double x, double y, double yaw);

}  // namespace kinematic_icp
}  // namespace localization_zoo
