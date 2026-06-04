#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

#include "imu_preintegration/imu_preintegration.h"

namespace localization_zoo {
namespace d2lio {

/// ボクセル座標ハッシュ (KISS-ICP と同じ方式)。
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

/// 局所平面法線つきボクセルハッシュマップ (scan-to-submap の submap 役)。
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double planarity = 0.0;
    bool found = false;
    bool has_normal = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);
  std::vector<Correspondence> getCorrespondences(
      const std::vector<Eigen::Vector3d>& points, double max_dist,
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

struct D2LIOParams {
  double voxel_size = 1.0;        ///< submap ボクセルサイズ [m]
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;

  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;

  // --- 適応的外れ値除去しきい値 ε_j ---
  /// マップ離散化に対する基礎しきい値 [m] (motion 項に加算)。
  double base_threshold = 1.0;
  /// ε_j の上限 [m] (誤対応の暴走防止)。
  double max_threshold = 3.0;

  // --- robust kernel ---
  double cauchy_scale = 1.0;      ///< Cauchy カーネルスケール [m]

  // --- 退化対応ハイブリッド重み (IMU 正則化) ---
  /// 相対退化比 r。ブロック内で λ_k < r·λ_max の方向を退化とみなし正則化する。
  /// スケール不変なので場面規模に依らず発火する (条件数ベース)。
  double degeneracy_ratio = 0.05;
  /// IMU prior の基本強度 β_prior (退化方向の最大固有値に対する倍率)。
  double imu_prior_weight = 1.0;

  // --- gyro バイアスオンライン補正 ---
  double gyro_bias_gain = 0.3;
  double max_gyro_bias = 0.5;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct D2LIOResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  bool used_imu = false;
  int degenerate_rot_dirs = 0;    ///< 直近フレームの退化回転方向数 (0..3)
  int degenerate_trans_dirs = 0;  ///< 直近フレームの退化並進方向数 (0..3)
};

/// D²-LIO (arXiv:2508.14355): Directional Degeneracy を考慮した LiDAR-IMU odometry。
///
/// 3 つの貢献を実装する:
///  (1) 適応的外れ値除去しきい値 ε_j = base + ‖t_rel‖ + 2‖p_j‖·sin(½‖θ_rel‖)。
///      点ごとに予測フレーム間運動と range に応じて対応許容距離を変える。
///  (2) scan-to-submap 登録 + IMU 回転先験 (gyro 事前積分)。
///  (3) ハイブリッド重み: 点群 Hessian の回転/並進ブロックを固有値分解し、
///      退化 (小固有値) 方向に IMU 予測へ引き戻す prior を加える W=(AᵀA·P)⁻¹ 近似。
class D2LIOPipeline {
public:
  explicit D2LIOPipeline(const D2LIOParams& params = D2LIOParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }

  D2LIOResult registerFrame(
      const std::vector<Eigen::Vector3d>& frame,
      const std::vector<localization_zoo::imu_preintegration::ImuSample>& imu);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;

  /// 退化対応 point-to-plane ICP。prediction は IMU prior の引き戻し先。
  Eigen::Matrix4d runICP(const std::vector<Eigen::Vector3d>& source,
                         const Eigen::Matrix4d& initial_guess,
                         const Eigen::Matrix4d& prediction, double t_rel,
                         double theta_rel,
                         const Eigen::Matrix3d& imu_rot_info,
                         D2LIOResult* result);

  D2LIOParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  Eigen::Vector3d gyro_bias_ = Eigen::Vector3d::Zero();
  int frame_count_ = 0;
};

}  // namespace d2lio
}  // namespace localization_zoo
