#pragma once

#include <Eigen/Core>

#include <cstddef>
#include <unordered_map>
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
  double score_sigma = 0.15;
  double min_range = 0.1;
  double max_range = 30.0;
  bool use_motion_prior = true;
  bool use_local_map = false;
  double local_map_radius = 15.0;
  double local_map_voxel_size = 0.15;
  bool use_branch_and_bound = true;
  double bnb_min_xy = 0.04;
  double bnb_min_yaw = 0.02;
  int leaf_xy_steps = 3;
  int leaf_yaw_steps = 3;
  int bnb_max_nodes = 128;
  /// Brute-force fallback (when use_branch_and_bound = false).
  int coarse_xy_steps = 13;
  int coarse_yaw_steps = 13;
  int fine_xy_steps = 7;
  int fine_yaw_steps = 7;
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
  size_t mapSize() const { return map_points_robot_.size(); }

 private:
  struct Grid {
    double resolution = 0.05;
    int width = 0;
    int height = 0;
    double origin_x = 0.0;
    double origin_y = 0.0;
    std::vector<float> dist_m;
    std::vector<float> score;
    std::vector<float> bound;
  };

  struct SearchNode {
    double dx = 0.0;
    double dy = 0.0;
    double dt = 0.0;
    double wx = 0.0;
    double wy = 0.0;
    double wt = 0.0;
    int level = 0;
    double bound = 0.0;
  };

  static int64_t voxelKey(double x, double y, double voxel_size);
  std::vector<Eigen::Vector2d> scanToPoints(const LaserScan& scan) const;
  std::vector<Eigen::Vector2d> localMapPoints() const;
  void rebuildPointVoxels();
  void transformRobotMap(const Eigen::Matrix3d& inv_increment);
  void addScanToMap(const std::vector<Eigen::Vector2d>& points);
  void pruneMap();
  Grid buildGrid(const std::vector<Eigen::Vector2d>& points, double resolution) const;
  void computeDistanceTransform(Grid* grid) const;
  void computeScoreGrid(Grid* grid) const;
  void downsampleGrid(const Grid& fine, Grid* coarse) const;
  void computeBoundFromFiner(const Grid& fine, Grid* coarse) const;
  std::vector<Grid> buildPyramid(const std::vector<Eigen::Vector2d>& points) const;
  double scorePose(const Grid& grid, const std::vector<Eigen::Vector2d>& points,
                   const Eigen::Matrix3d& increment) const;
  double lookupDistanceM(const Grid& grid, const Eigen::Vector2d& p) const;
  double lookupBound(const Grid& grid, int x, int y) const;
  double nodeUpperBound(const Grid& grid, const SearchNode& node,
                        const std::vector<Eigen::Vector2d>& points,
                        const Eigen::Matrix3d& base_increment) const;
  Eigen::Matrix3d searchBranchAndBound(const std::vector<Grid>& pyramid,
                                       const std::vector<Eigen::Vector2d>& points,
                                       const Eigen::Matrix3d& prior, double xy_range,
                                       double yaw_range) const;
  Eigen::Matrix3d searchBranchAndBoundAtLevel(const Grid& grid, int level,
                                              const std::vector<Eigen::Vector2d>& points,
                                              const Eigen::Matrix3d& center_increment,
                                              double xy_range, double yaw_range) const;
  Eigen::Matrix3d refineAtLevel(const Grid& grid, const std::vector<Eigen::Vector2d>& points,
                                const Eigen::Matrix3d& center_increment, int level, int finest,
                                double xy_range, double yaw_range) const;
  Eigen::Matrix3d searchBestTransformBruteForce(const std::vector<Grid>& pyramid,
                                                const std::vector<Eigen::Vector2d>& points,
                                                const Eigen::Matrix3d& prior, double xy_range,
                                                double yaw_range) const;

  CSMParams params_;
  bool initialized_ = false;
  std::vector<Eigen::Vector2d> ref_points_;
  std::vector<Grid> ref_pyramid_;
  std::vector<Eigen::Vector2d> map_points_robot_;
  std::unordered_map<int64_t, size_t> point_voxels_;
  Eigen::Matrix3d pose_ = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d last_increment_ = Eigen::Matrix3d::Identity();
};

Eigen::Matrix3d pose2D(double x, double y, double yaw);

}  // namespace csm
}  // namespace localization_zoo
