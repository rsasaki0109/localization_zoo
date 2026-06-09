#pragma once

#include <Eigen/Core>

#include <cstddef>
#include <vector>

namespace localization_zoo {
namespace karto_matcher {

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

struct KartoMatcherParams {
  double grid_resolution = 0.05;
  int pyramid_levels = 3;
  double search_xy_range = 0.6;
  double search_yaw_range = 0.15;
  double score_sigma = 0.15;
  double min_range = 0.1;
  double max_range = 30.0;
  bool use_motion_prior = true;
  /// Rolling local map radius around the robot [m].
  double local_map_radius = 12.0;
  int coarse_xy_steps = 11;
  int coarse_yaw_steps = 11;
  int fine_xy_steps = 5;
  int fine_yaw_steps = 5;
};

struct KartoMatcherResult {
  Eigen::Matrix3d increment = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d pose = Eigen::Matrix3d::Identity();
  bool valid = false;
  double score = 0.0;
};

/// Karto-style correlative scan matching against a rolling occupancy map with
/// multi-resolution coarse-to-fine search (Olson ICRA 2009, simplified port).
class KartoMatcherEstimator {
 public:
  explicit KartoMatcherEstimator(const KartoMatcherParams& params = KartoMatcherParams());

  void reset();
  void setInitialPose(const Eigen::Matrix3d& pose);

  KartoMatcherResult registerScan(const LaserScan& scan);

  const Eigen::Matrix3d& pose() const { return pose_; }
  size_t mapSize() const { return map_points_world_.size(); }

 private:
  struct Grid {
    double resolution = 0.05;
    int width = 0;
    int height = 0;
    double origin_x = 0.0;
    double origin_y = 0.0;
    std::vector<float> dist_m;
  };

  std::vector<Eigen::Vector2d> scanToPoints(const LaserScan& scan) const;
  std::vector<Eigen::Vector2d> localMapPoints() const;
  Grid buildGrid(const std::vector<Eigen::Vector2d>& points, double resolution) const;
  void computeDistanceTransform(Grid* grid) const;
  std::vector<Grid> buildPyramid(const std::vector<Eigen::Vector2d>& points) const;
  double scorePose(const Grid& grid, const std::vector<Eigen::Vector2d>& points,
                   const Eigen::Matrix3d& world_transform) const;
  double lookupDistanceM(const Grid& grid, const Eigen::Vector2d& p) const;
  Eigen::Matrix3d searchBestTransform(const std::vector<Grid>& pyramid,
                                      const std::vector<Eigen::Vector2d>& points,
                                      const Eigen::Matrix3d& prior) const;
  void addScanToMap(const std::vector<Eigen::Vector2d>& points);
  void pruneMap();

  KartoMatcherParams params_;
  bool initialized_ = false;
  std::vector<Eigen::Vector2d> map_points_world_;
  Eigen::Matrix3d pose_ = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d last_increment_ = Eigen::Matrix3d::Identity();
};

Eigen::Matrix3d pose2D(double x, double y, double yaw);

}  // namespace karto_matcher
}  // namespace localization_zoo
