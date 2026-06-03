#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

#include "imu_preintegration/imu_preintegration.h"

namespace localization_zoo {
namespace vibration_lio {

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

/// 平面 (法線 + 平面分散) を保持する確率的ボクセルマップ。
class VoxelHashMap {
public:
  struct PlaneFeature {
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double plane_variance = 0.0;
    bool found = false;
    bool has_normal = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);
  PlaneFeature query(const Eigen::Vector3d& world_point, double max_dist,
                     int normal_min_neighbors) const;
  void pruneFarVoxels(const Eigen::Vector3d& center, double max_distance);
  void clear() { map_.clear(); }
  size_t size() const { return map_.size(); }

private:
  Eigen::Vector3i toVoxel(const Eigen::Vector3d& p) const {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / voxel_size_)),
                           static_cast<int>(std::floor(p.y() / voxel_size_)),
                           static_cast<int>(std::floor(p.z() / voxel_size_)));
  }
  double voxel_size_;
  int max_points_;
  struct VoxelBlock {
    std::vector<Eigen::Vector3d> points;
  };
  std::unordered_map<Eigen::Vector3i, VoxelBlock, VoxelHash> map_;
};

struct VibrationLIOParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double max_correspondence_dist = 2.0;

  // --- 測定不確かさ (range/bearing) ---
  double sigma_depth = 0.02;
  double sigma_bearing = 0.0015;

  // --- 振動由来 post-undistortion 不確かさ ---
  double gamma = 0.1;            ///< 振動強度→標準偏差スケール (100-200Hz IMU で 0.1)
  double scan_period = 0.1;      ///< スキャン周期 [s] (δt の代表値)
  double min_vibration = 1e-3;   ///< 振動強度の下限 (IMU 無しフォールバック)

  double gyro_bias_gain = 0.3;
  double max_gyro_bias = 0.5;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct VibrationLIOResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  bool used_imu = false;
  double vibration_omega = 0.0;  ///< 直近の角振動強度 k_ω
  double vibration_accel = 0.0;  ///< 直近の線振動強度 k_v
};

/// Vibration-aware LIO (arXiv:2507.04311)。
///
/// 高周波振動によるスキャン歪み補正後の残差不確かさを点ごとにモデル化:
///   Σ_p = ⌊p⌋× Σr ⌊p⌋×ᵀ + Σ_T + R Σ_meas Rᵀ
/// ここで Σr=σ_r²I (角振動), Σ_T=σ_T²I (線振動), σ_{r,T}=γ·δt·k_{ω,v}、
/// k_{ω,v} は IMU 角速度/加速度の MAD (mean absolute deviation) で振動強度を推定。
/// この点共分散で point-to-plane 残差を Mahalanobis 重み付け
/// (R_j = uᵀΣ_p u) し、不確かな点を down-weight する。
class VibrationLIOPipeline {
public:
  explicit VibrationLIOPipeline(
      const VibrationLIOParams& params = VibrationLIOParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }

  VibrationLIOResult registerFrame(
      const std::vector<Eigen::Vector3d>& frame,
      const std::vector<localization_zoo::imu_preintegration::ImuSample>& imu);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix3d measurementCovariance(const Eigen::Vector3d& p_sensor) const;
  /// 振動由来 + 測定の合成点共分散 (センサ系)。
  Eigen::Matrix3d pointCovariance(const Eigen::Vector3d& p_sensor, double sigma_r,
                                  double sigma_T) const;
  Eigen::Matrix4d runICP(const std::vector<Eigen::Vector3d>& source,
                         const Eigen::Matrix4d& initial_guess, double sigma_r,
                         double sigma_T);

  VibrationLIOParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  Eigen::Vector3d gyro_bias_ = Eigen::Vector3d::Zero();
  int frame_count_ = 0;
};

}  // namespace vibration_lio
}  // namespace localization_zoo
