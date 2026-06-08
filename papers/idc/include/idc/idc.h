#pragma once

#include <Eigen/Core>

#include <cstddef>
#include <vector>

namespace localization_zoo {
namespace idc {

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

struct IDCParams {
  int max_iterations = 30;
  double max_cp_distance = 0.5;
  double max_range_diff = 0.3;
  int range_match_beam_window = 5;
  double convergence_translation = 1e-4;
  double convergence_rotation = 1e-4;
  double min_range = 0.1;
  double max_range = 30.0;
  int min_correspondences = 20;
  bool use_motion_prior = true;
};

struct IDCResult {
  Eigen::Matrix3d increment = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d pose = Eigen::Matrix3d::Identity();
  bool valid = false;
  int iterations = 0;
};

/// Iterative Dual Correspondence 2D scan matching (Lu & Milios, 1997).
class IDCEstimator {
 public:
  explicit IDCEstimator(const IDCParams& params = IDCParams());

  void reset();
  void setInitialPose(const Eigen::Matrix3d& pose);

  IDCResult registerScan(const LaserScan& scan);

  const Eigen::Matrix3d& pose() const { return pose_; }

 private:
  struct RefPoint {
    Eigen::Vector2d point = Eigen::Vector2d::Zero();
    double range = 0.0;
    bool valid = false;
  };

  struct Sample {
    Eigen::Vector2d point;
    double range = 0.0;
    int beam = 0;
  };

  std::vector<Sample> scanToSamples(const LaserScan& scan) const;
  std::vector<RefPoint> buildReferenceModel(const LaserScan& scan) const;
  bool solveCP(const std::vector<Sample>& current, const Eigen::Matrix3d& transform,
               double* tx, double* ty) const;
  bool solveRR(const std::vector<Sample>& current, const Eigen::Matrix3d& transform,
               double* yaw) const;
  static Eigen::Vector2d transformPoint(const Eigen::Matrix3d& T, const Eigen::Vector2d& p);

  IDCParams params_;
  bool initialized_ = false;
  std::vector<RefPoint> ref_model_;
  Eigen::Matrix3d pose_ = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d last_increment_ = Eigen::Matrix3d::Identity();
};

Eigen::Matrix3d pose2D(double x, double y, double yaw);

}  // namespace idc
}  // namespace localization_zoo
