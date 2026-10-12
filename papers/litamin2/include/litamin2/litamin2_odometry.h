#pragma once

// LiTAMIN2 odometry as described in Sec. III of the paper (Yokozuka et al.,
// ICRA 2021), separate from the harness-oriented scan-to-map path:
//
// - each raw scan is voted into voxels of `voxel_size` (3 m on KITTI) and every
//   voxel becomes one normal distribution (about 0.5 % of the raw points);
// - the map is a voxel map of normal distributions in the world frame;
// - each source distribution is paired with the nearest map distribution
//   (the paper uses a kd-tree; here the nearest mean among the 27 map voxels
//   around the transformed source mean, which finds the same partner whenever
//   it is closer than one voxel, i.e. wherever the robust weight is not ~0);
// - the cost is w_ICP E_ICP + w_Cov (tr(X) + tr(X^-1) - 6) (Table I, weights
//   from eqs. 3-6) and is minimised by Newton steps without damping; the
//   shape term's Hessian is projected to positive semi-definite per pair;
// - the next pose is predicted with constant velocity.

#include "litamin2/gaussian_voxel_map.h"

#include <Eigen/Core>

#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace litamin2 {

struct LiTAMIN2OdometryParams {
  double voxel_size = 3.0;
  int min_points_per_voxel = 3;
  double min_cov_eigenvalue = 1e-3;
  double lambda = 1e-6;
  double sigma_icp = 0.5;
  double sigma_cov = 3.0;
  bool use_cov_cost = true;  // false: the paper's "ICP" variant
  int max_iterations = 30;
  double rotation_epsilon = 1e-5;     // rad
  double translation_epsilon = 1e-4;  // m

  // Map. kAggregate votes every registered raw point into the map voxels, so a
  // map distribution is the running mean / covariance of all points that fell
  // into it; kFirst keeps the first scan distribution that landed in a voxel;
  // kLatest replaces it with the newest one.
  enum class MapMode { kAggregate, kFirst, kLatest };
  MapMode map_mode = MapMode::kAggregate;
  double map_voxel_size = 3.0;
  double map_radius = 100.0;  // map voxels farther than this from the sensor are dropped
  int num_threads = 1;
};

struct LiTAMIN2OdometryResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();  // world <- sensor
  int iterations = 0;
  int source_distributions = 0;
  int correspondences = 0;
  size_t map_distributions = 0;
};

class LiTAMIN2Odometry {
public:
  explicit LiTAMIN2Odometry(const LiTAMIN2OdometryParams& params = LiTAMIN2OdometryParams());

  // points: one raw scan in the sensor frame.
  LiTAMIN2OdometryResult process(const std::vector<Eigen::Vector3d>& points);

  const Eigen::Matrix4d& pose() const { return pose_; }

  // Distributions of one scan (exposed for tests).
  std::vector<GaussianVoxel> voxelize(const std::vector<Eigen::Vector3d>& points) const;

  // One registration of `source` (sensor frame) against the current map.
  Eigen::Matrix4d align(const std::vector<GaussianVoxel>& source, const Eigen::Matrix4d& guess,
                        int* iterations, int* correspondences) const;

private:
  struct MapVoxel {
    int n = 0;
    Eigen::Vector3d sum = Eigen::Vector3d::Zero();
    Eigen::Matrix3d sum_sq = Eigen::Matrix3d::Zero();
    GaussianVoxel dist;  // finalized distribution
    bool valid = false;
  };

  Eigen::Vector3i mapKey(const Eigen::Vector3d& p) const;
  void insert(const std::vector<Eigen::Vector3d>& points, const std::vector<GaussianVoxel>& source,
              const Eigen::Matrix4d& pose);
  const GaussianVoxel* nearest(const Eigen::Vector3d& q) const;

  LiTAMIN2OdometryParams params_;
  std::unordered_map<Eigen::Vector3i, MapVoxel, Vector3iHash> map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d velocity_ = Eigen::Matrix4d::Identity();
  int frame_ = 0;
};

}  // namespace litamin2
}  // namespace localization_zoo
