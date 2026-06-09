#pragma once

#include <Eigen/Core>

#include <cstddef>
#include <vector>

namespace localization_zoo {
namespace mb_icp {

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

struct MbICPParams {
  int max_iterations = 30;
  double max_metric_distance = 1.0;
  double metric_radius = 3.0;
  double convergence_translation = 1e-4;
  double convergence_rotation = 1e-4;
  double min_range = 0.1;
  double max_range = 30.0;
  double max_neighbor_gap = 2.0;
  double trim_fraction = 0.9;
  int min_correspondences = 20;
  bool use_motion_prior = true;
  bool use_local_map = false;
  double local_map_radius = 15.0;
  double local_map_voxel_size = 0.15;
};

struct MbICPResult {
  Eigen::Matrix3d increment = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d pose = Eigen::Matrix3d::Identity();
  bool valid = false;
  int iterations = 0;
};

/// Metric-based ICP 2D scan matching (Minguez, Lamiraux, Montesano, ICRA 2005).
class MbICPEstimator {
 public:
  explicit MbICPEstimator(const MbICPParams& params = MbICPParams());

  void reset();
  void setInitialPose(const Eigen::Matrix3d& pose);

  MbICPResult registerScan(const LaserScan& scan);

  const Eigen::Matrix3d& pose() const { return pose_; }
  size_t mapSize() const { return local_points_.size() + local_segments_.size(); }

 private:
  struct RefPoint {
    Eigen::Vector2d point = Eigen::Vector2d::Zero();
    bool valid = false;
  };

  struct Segment {
    Eigen::Vector2d a = Eigen::Vector2d::Zero();
    Eigen::Vector2d b = Eigen::Vector2d::Zero();
  };

  std::vector<Eigen::Vector2d> scanToPoints(const LaserScan& scan) const;
  void buildReferenceModel(const LaserScan& scan);
  std::vector<RefPoint> localPointsInFrame(const Eigen::Matrix3d& frame) const;
  std::vector<Segment> localSegmentsInFrame(const Eigen::Matrix3d& frame) const;
  void addScanToLocalMap(const LaserScan& scan);
  void pruneLocalMap();
  bool solveIncrement(const std::vector<Eigen::Vector2d>& current,
                      const std::vector<RefPoint>& points,
                      const std::vector<Segment>& segments,
                      const Eigen::Matrix3d& transform, Eigen::Matrix3d* increment) const;
  double metricDistanceSquared(const Eigen::Vector2d& delta,
                               const Eigen::Vector2d& reference) const;
  Eigen::Matrix2d metricMatrix(const Eigen::Vector2d& reference) const;
  static Eigen::Vector2d transformPoint(const Eigen::Matrix3d& T, const Eigen::Vector2d& p);

  MbICPParams params_;
  bool initialized_ = false;
  std::vector<RefPoint> ref_points_;
  std::vector<Segment> ref_segments_;
  std::vector<RefPoint> local_points_;
  std::vector<Segment> local_segments_;
  Eigen::Matrix3d pose_ = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d last_increment_ = Eigen::Matrix3d::Identity();
};

Eigen::Matrix3d pose2D(double x, double y, double yaw);

}  // namespace mb_icp
}  // namespace localization_zoo
