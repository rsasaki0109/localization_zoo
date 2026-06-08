#pragma once

#include <Eigen/Core>

#include <cstddef>
#include <vector>

namespace localization_zoo {
namespace psm {

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

struct PSMParams {
  double search_xy_range = 0.6;
  double search_yaw_range = 0.15;
  int coarse_xy_steps = 13;
  int coarse_yaw_steps = 13;
  int fine_xy_steps = 7;
  int fine_yaw_steps = 7;
  double min_range = 0.1;
  double max_range = 30.0;
  double range_sigma = 0.08;
  int min_valid_bins = 30;
  int refine_iterations = 12;
  bool use_motion_prior = true;
};

struct PSMResult {
  Eigen::Matrix3d increment = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d pose = Eigen::Matrix3d::Identity();
  bool valid = false;
  double score = 0.0;
};

/// Polar scan matching 2D odometry (Hähnel-style polar range correlation).
class PSMEstimator {
 public:
  explicit PSMEstimator(const PSMParams& params = PSMParams());

  void reset();
  void setInitialPose(const Eigen::Matrix3d& pose);

  PSMResult registerScan(const LaserScan& scan);

  const Eigen::Matrix3d& pose() const { return pose_; }

 private:
  std::vector<Eigen::Vector2d> scanToPoints(const LaserScan& scan) const;
  std::vector<double> polarProfileFromPoints(const std::vector<Eigen::Vector2d>& points,
                                             const LaserScan& grid) const;
  double scorePolarProfiles(const std::vector<double>& ref,
                            const std::vector<double>& cur) const;
  double scoreTransform(const std::vector<Eigen::Vector2d>& points,
                        const Eigen::Matrix3d& transform) const;
  Eigen::Matrix3d searchBestTransform(const std::vector<Eigen::Vector2d>& points,
                                      const Eigen::Matrix3d& prior) const;
  Eigen::Matrix3d refineTransform(const std::vector<Eigen::Vector2d>& points,
                                  const Eigen::Matrix3d& initial) const;
  static Eigen::Vector2d transformPoint(const Eigen::Matrix3d& T, const Eigen::Vector2d& p);

  PSMParams params_;
  bool initialized_ = false;
  LaserScan ref_scan_;
  std::vector<double> ref_polar_;
  Eigen::Matrix3d pose_ = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d last_increment_ = Eigen::Matrix3d::Identity();
};

Eigen::Matrix3d pose2D(double x, double y, double yaw);

}  // namespace psm
}  // namespace localization_zoo
