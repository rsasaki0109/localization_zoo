#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

#include "imu_preintegration/imu_preintegration.h"

namespace localization_zoo {
namespace ua_lio {

struct VoxelHash {
  std::size_t operator()(const Eigen::Vector3i& v) const {
    std::size_t seed = 0;
    auto hc = [&seed](int val) {
      seed ^= std::hash<int>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    hc(v.x()); hc(v.y()); hc(v.z());
    return seed;
  }
};

/// Gaussian ボクセルマップ。各ボクセルが点群分布 (μ, Σ) を逐次蓄積する
/// (UA-LIO の distribution 表現)。
class GaussianVoxelMap {
public:
  struct Feature {
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double plane_variance = 0.0;
    bool found = false;
    bool has_normal = false;
  };

  GaussianVoxelMap(double voxel_size, int min_points)
      : voxel_size_(voxel_size), min_points_(min_points) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);
  Feature query(const Eigen::Vector3d& world_point) const;
  void pruneFarVoxels(const Eigen::Vector3d& center, double max_distance);
  void clear() { voxels_.clear(); }
  size_t size() const { return voxels_.size(); }

private:
  Eigen::Vector3i toVoxel(const Eigen::Vector3d& p) const {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / voxel_size_)),
                           static_cast<int>(std::floor(p.y() / voxel_size_)),
                           static_cast<int>(std::floor(p.z() / voxel_size_)));
  }
  struct Voxel {
    int count = 0;
    Eigen::Vector3d sum = Eigen::Vector3d::Zero();
    Eigen::Matrix3d sum_outer = Eigen::Matrix3d::Zero();
    Feature feat;
  };
  void refresh(Voxel& v) const;

  double voxel_size_;
  int min_points_;
  std::unordered_map<Eigen::Vector3i, Voxel, VoxelHash> voxels_;
};

struct UALIOParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int voxel_min_points = 6;
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double max_correspondence_dist = 2.0;

  // 点不確かさ (range/bearing)
  double sigma_depth = 0.02;
  double sigma_bearing = 0.0015;

  // 地面拘束
  bool ground_constraint = true;
  double ground_normal_z_min = 0.85;  ///< 法線 z 成分がこれ以上なら地面平面
  double ground_weight = 0.5;         ///< 地面 z 拘束の相対重み

  double gyro_bias_gain = 0.3;
  double max_gyro_bias = 0.5;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct UALIOResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  bool used_imu = false;
  int ground_cells = 0;  ///< 地面拘束に寄与したセル数
};

/// UA-LIO (IEEE TIM 2025): Uncertainty-Aware LiDAR-Inertial Odometry。
///
///  (1) 各点を局所平面上の Gaussian とみなし、range/bearing 由来の点共分散を
///      姿勢で伝播する distribution-to-distribution (D2D) 更新。
///  (2) 残差情報行列 = (R Σ_p Rᵀ + Σ_voxel)⁻¹ で Mahalanobis 重み付け。
///  (3) 地面平面 (法線≈z) を検出し z 方向ドリフトを抑える地面拘束。
class UALIOPipeline {
public:
  explicit UALIOPipeline(const UALIOParams& params = UALIOParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }

  UALIOResult registerFrame(
      const std::vector<Eigen::Vector3d>& frame,
      const std::vector<localization_zoo::imu_preintegration::ImuSample>& imu);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix3d pointCovariance(const Eigen::Vector3d& p_sensor) const;
  Eigen::Matrix4d runICP(const std::vector<Eigen::Vector3d>& source,
                         const Eigen::Matrix4d& initial_guess, int* ground_cells);

  UALIOParams params_;
  GaussianVoxelMap map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  Eigen::Vector3d gyro_bias_ = Eigen::Vector3d::Zero();
  int frame_count_ = 0;
};

}  // namespace ua_lio
}  // namespace localization_zoo
