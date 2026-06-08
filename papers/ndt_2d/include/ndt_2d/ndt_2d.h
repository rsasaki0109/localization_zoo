#pragma once

#include <Eigen/Core>

#include <cstddef>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace ndt_2d {

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

struct NDT2DParams {
  double cell_size = 0.4;
  double cov_regularization = 0.1;
  int min_points_per_cell = 1;
  int max_iterations = 30;
  double convergence_translation = 1e-4;
  double convergence_rotation = 1e-4;
  double min_range = 0.1;
  double max_range = 30.0;
  int min_points = 20;
  bool use_motion_prior = true;
};

struct NDT2DResult {
  Eigen::Matrix3d increment = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d pose = Eigen::Matrix3d::Identity();
  bool valid = false;
  int iterations = 0;
  double score = 0.0;
};

/// 2D Normal Distributions Transform scan matching (Biber & Straßer, IROS 2003).
class NDT2DEstimator {
 public:
  explicit NDT2DEstimator(const NDT2DParams& params = NDT2DParams());

  void reset();
  void setInitialPose(const Eigen::Matrix3d& pose);

  NDT2DResult registerScan(const LaserScan& scan);

  const Eigen::Matrix3d& pose() const { return pose_; }

 private:
  struct Cell {
    Eigen::Vector2d mean = Eigen::Vector2d::Zero();
    Eigen::Matrix2d inv_cov = Eigen::Matrix2d::Identity();
    int count = 0;
    bool valid = false;
  };

  using CellMap = std::unordered_map<int64_t, Cell>;

  int64_t cellKey(int ix, int iy) const;
  void cellIndex(const Eigen::Vector2d& p, int* ix, int* iy) const;

  std::vector<Eigen::Vector2d> scanToPoints(const LaserScan& scan) const;
  CellMap buildNDTMap(const std::vector<Eigen::Vector2d>& points);
  const Cell* lookupCell(const CellMap& map, const Eigen::Vector2d& p) const;
  bool solveIncrement(const std::vector<Eigen::Vector2d>& current,
                      const CellMap& map, const Eigen::Matrix3d& transform,
                      Eigen::Matrix3d* increment, double* score) const;
  static Eigen::Vector2d transformPoint(const Eigen::Matrix3d& T,
                                        const Eigen::Vector2d& p);

  NDT2DParams params_;
  bool initialized_ = false;
  CellMap ref_map_;
  Eigen::Vector2d map_origin_ = Eigen::Vector2d::Zero();
  Eigen::Matrix3d pose_ = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d last_increment_ = Eigen::Matrix3d::Identity();
};

Eigen::Matrix3d pose2D(double x, double y, double yaw);

}  // namespace ndt_2d
}  // namespace localization_zoo
