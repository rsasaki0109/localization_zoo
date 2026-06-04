#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace lidar_iba {

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

// ---------------------------------------------------------------------------
// ステレオ投影による平面法線パラメータ化 (論文 Eq.9, 20)。
// 単位法線 n∈S² を北極を除く 2 パラメータ u=[ux,uy] で表す。‖n(u)‖=1 が自動的に
// 保証され、制約なし最適化ができる (over-parameterization と特異点を回避)。
// ---------------------------------------------------------------------------

/// u → 単位法線 n (Eq.9)。denom=1+ux²+uy²、n=[2ux,2uy,ux²+uy²-1]/denom。
Eigen::Vector3d stereoToNormal(const Eigen::Vector2d& u);

/// 単位法線 n → ステレオ座標 u (逆写像)。n_z=1 (北極) 近傍では n を反転して回避。
Eigen::Vector2d normalToStereo(const Eigen::Vector3d& n_in);

/// ∂n/∂u (3x2 ヤコビアン, Eq.20)。
Eigen::Matrix<double, 3, 2> stereoJacobian(const Eigen::Vector2d& u);

// ---------------------------------------------------------------------------

/// 平面法線つきボクセルハッシュマップ (front-end scan-to-map 用)。
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

struct LidarIbaParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double initial_threshold = 2.0;

  // --- sliding-window plane bundle adjustment ---
  /// BA を有効化 (off なら front-end scan-to-map のみ)。
  bool enable_ba = true;
  /// キーフレーム間隔 (このフレーム数ごとにキーフレーム化、論文は <=3)。
  int keyframe_interval = 2;
  /// スライディングウィンドウのキーフレーム数 W。
  int window_size = 5;
  /// BA のガウス・ニュートン反復回数。
  int ba_iterations = 4;
  /// 平面ランドマーク抽出ボクセルサイズ。
  double plane_voxel = 2.0;
  /// 平面ランドマーク採用の最小点数。
  int plane_min_points = 8;
  /// 平面ランドマークの最小 planarity。
  double plane_min_planarity = 0.6;
  /// 採用するランドマーク最大数 (dense 解の規模制限)。
  int max_landmarks = 150;
  /// 1 ランドマークあたり最大観測数。
  int max_obs_per_landmark = 24;
  /// ステレオ座標の半径制限 δ (‖u‖≤1+δ、Eq.9-10 直後の数値安定化)。
  double stereo_radius_margin = 0.2;
  /// LiDAR 点ノイズ σ (point-to-plane 残差の重み 1/σ)。
  double lidar_sigma = 0.05;
  /// 各キーフレーム pose を front-end (odometry) 推定へ柔らかく繋ぐ prior の重み。
  /// IMU/marginalization prior (r_I) の純 LiDAR 代替で、自己相似シーンでの BA の
  /// gauge 崩壊を防ぐ。0 で prior 無効。
  double pose_prior_weight = 20.0;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct LidarIbaResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  bool ba_ran = false;          ///< このフレームで BA が走ったか
  int ba_landmarks = 0;         ///< BA で使った平面ランドマーク数
  int ba_observations = 0;      ///< BA で使った観測数
  double ba_cost_before = 0.0;  ///< BA 前のコスト
  double ba_cost_after = 0.0;   ///< BA 後のコスト
};

/// A Consistency-Improved LiDAR(-Inertial) Bundle Adjustment
/// (arXiv:2602.06380, Li et al. 2026) の from-paper 再実装。著者コード未公開。
///
///  (1) 平面法線をステレオ投影 u=[ux,uy] でパラメータ化 (Eq.9, 20)。
///      ‖n(u)‖=1 が自動保証され、地面平面 (原点近傍を通り従来 closest-point 表現が
///      発散する) でも Jacobian が有界で over-parameterization を回避する。
///  (2) キーフレームの滑動窓で pose と平面ランドマークを同時最適化する LiDAR BA。
///  (3) FEJ (First-Estimates Jacobian): 観測不能方向 (純 LiDAR では world 6-DoF) へ
///      偽情報が入らないよう、marginalize される最古キーフレームの線形化点を固定する。
///
/// 純 LiDAR scope。IMU 予測は等速 (constant-velocity) で代替。FEJ は最古キーフレーム
/// pose の gauge 固定 + 線形化点凍結として実現する (完全な Schur complement prior の
/// 持ち越しは範囲外、README 参照)。
class LidarIbaPipeline {
public:
  explicit LidarIbaPipeline(const LidarIbaParams& params = LidarIbaParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  LidarIbaResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& initial_guess,
                                  LidarIbaResult* result);
  Eigen::Matrix4d predict() const;
  void runWindowBA(LidarIbaResult* result);

  LidarIbaParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;

  struct Keyframe {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    Eigen::Matrix4d fe_pose = Eigen::Matrix4d::Identity();  // front-end 推定 (prior 用、不変)
    std::vector<Eigen::Vector3d> points;  // body 系 (downsampled)
  };
  std::deque<Keyframe> keyframes_;
  int frame_count_ = 0;
  int frames_since_kf_ = 0;
};

}  // namespace lidar_iba
}  // namespace localization_zoo
