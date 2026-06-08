#pragma once

#include <Eigen/Core>

#include <cstddef>
#include <vector>

namespace localization_zoo {
namespace csm {

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

struct CSMParams {
  double grid_resolution = 0.05;
  int pyramid_levels = 3;
  double search_xy_range = 0.6;
  double search_yaw_range = 0.15;
  int coarse_xy_steps = 13;
  int coarse_yaw_steps = 13;
  int fine_xy_steps = 7;
  int fine_yaw_steps = 7;
  double score_sigma = 0.15;
  double min_range = 0.1;
  double max_range = 30.0;
  bool use_motion_prior = true;
};

struct CSMResult {
  Eigen::Matrix3d increment = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d pose = Eigen::Matrix3d::Identity();
  bool valid = false;
  double score = 0.0;
};

/// Multi-resolution correlative scan matching (Olson-style, simplified 2D odometry).
class CSMEstimator {
 public:
  explicit CSMEstimator(const CSMParams& params = CSMParams());

  void reset();
  void setInitialPose(const Eigen::Matrix3d& pose);

  CSMResult registerScan(const LaserScan& scan);

  const Eigen::Matrix3d& pose() const { return pose_; }

 private:
  struct Grid {
    double resolution = 0.05;
    int width = 0;
    int height = 0;
    double origin_x = 0.0;
    double origin_y = 0.0;
    /// Distance to nearest occupied cell [m] after distance transform.
    std::vector<float> dist_m;
  };

  std::vector<Eigen::Vector2d> scanToPoints(const LaserScan& scan) const;
  Grid buildGrid(const std::vector<Eigen::Vector2d>& points, double resolution) const;
  void computeDistanceTransform(Grid* grid) const;
  std::vector<Grid> buildPyramid(const std::vector<Eigen::Vector2d>& points) const;
  double scorePose(const Grid& grid, const std::vector<Eigen::Vector2d>& points,
                   const Eigen::Matrix3d& transform) const;
  Eigen::Matrix3d searchBestTransform(const std::vector<Grid>& pyramid,
                                      const std::vector<Eigen::Vector2d>& points,
                                      const Eigen::Matrix3d& prior) const;
  double lookupDistanceM(const Grid& grid, const Eigen::Vector2d& p) const;

  CSMParams params_;
  bool initialized_ = false;
  std::vector<Eigen::Vector2d> ref_points_;
  std::vector<Grid> ref_pyramid_;
  Eigen::Matrix3d pose_ = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d last_increment_ = Eigen::Matrix3d::Identity();
};

Eigen::Matrix3d pose2D(double x, double y, double yaw);

}  // namespace csm
}  // namespace localization_zoo
