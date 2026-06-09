#pragma once

#include <Eigen/Core>

#include <cstddef>
#include <cstdint>
#include <unordered_map>
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
  bool use_local_map = false;
  double local_map_radius = 15.0;
  double local_map_voxel_size = 0.15;
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
  size_t mapSize() const { return local_refs_.size(); }

 private:
  struct RefPoint {
    Eigen::Vector2d point = Eigen::Vector2d::Zero();
    double range = 0.0;
    double bearing = 0.0;
    int beam = -1;
    bool valid = false;
  };

  struct Sample {
    Eigen::Vector2d point;
    double range = 0.0;
    double bearing = 0.0;
    int beam = 0;
  };

  struct LocalMapIndex {
    double cell_size = 1.0;
    double query_radius = 1.0;
    std::unordered_map<int64_t, std::vector<size_t>> bins;

    static LocalMapIndex build(const std::vector<RefPoint>& refs, double cell_size,
                               double query_radius);
    void queryCandidates(const Eigen::Vector2d& p, const std::vector<uint32_t>& stamp,
                         uint32_t generation, std::vector<size_t>* candidates) const;
  };

  static int64_t voxelKey(double x, double y, double voxel_size);
  double angleIncrement(const LaserScan& scan) const;
  std::vector<Sample> scanToSamples(const LaserScan& scan) const;
  std::vector<RefPoint> buildReferenceModel(const LaserScan& scan) const;
  void rebuildPointVoxels();
  void addScanToLocalMap(const LaserScan& scan);
  void transformRobotMap(const Eigen::Matrix3d& inv_increment);
  void pruneLocalMap();
  void rebuildLocalMapIndex();
  bool solveCP(const std::vector<Sample>& current, const std::vector<RefPoint>& refs,
               const Eigen::Matrix3d& transform, double* tx, double* ty) const;
  bool solveCPIndexed(const std::vector<Sample>& current, const std::vector<RefPoint>& refs,
                      const LocalMapIndex& index, const Eigen::Matrix3d& transform, double* tx,
                      double* ty) const;
  bool solveRRScanToScan(const std::vector<Sample>& current, const std::vector<RefPoint>& refs,
                         const Eigen::Matrix3d& transform, double* yaw) const;
  bool solveRRLocalMap(const std::vector<Sample>& current, const std::vector<RefPoint>& refs,
                       const LaserScan& scan, const Eigen::Matrix3d& transform,
                       double* yaw) const;
  static Eigen::Vector2d transformPoint(const Eigen::Matrix3d& T, const Eigen::Vector2d& p);

  IDCParams params_;
  bool initialized_ = false;
  std::vector<RefPoint> ref_model_;
  std::vector<RefPoint> local_refs_;
  std::unordered_map<int64_t, size_t> point_voxels_;
  LocalMapIndex local_map_index_;
  bool local_map_index_valid_ = false;
  mutable std::vector<uint32_t> query_stamp_;
  mutable uint32_t query_generation_ = 1;
  Eigen::Matrix3d pose_ = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d last_increment_ = Eigen::Matrix3d::Identity();
};

Eigen::Matrix3d pose2D(double x, double y, double yaw);

}  // namespace idc
}  // namespace localization_zoo
