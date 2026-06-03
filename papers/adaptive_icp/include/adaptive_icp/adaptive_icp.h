#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace adaptive_icp {

/// ボクセル座標ハッシュ (KISS-ICP / GenZ-ICP と同じ方式)。
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

/// 局所平面法線つきボクセルハッシュマップ。
/// クエリ点ごとに最近傍点・近傍重心・局所平面法線・planarity を返す。
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();   ///< 最近傍点
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();  ///< 近傍重心 (平面アンカー)
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();  ///< 局所平面法線
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

struct AdaptiveICPParams {
  double voxel_size = 0.5;        ///< マップボクセルサイズ [m]
  double max_range = 100.0;       ///< 最大距離 [m]
  double min_range = 3.0;         ///< 最小距離 [m]
  int max_points_per_voxel = 20;
  double initial_threshold = 2.0; ///< σ_th 初期値 / 上限の基準 [m]
  int normal_min_neighbors = 5;

  // --- 粗位置合わせ (density filtering) ---
  /// 密度パーセンタイル α [%]。これ未満の低密度点は粗位置合わせから除外。
  double density_percentile = 5.0;
  /// 密度カウント半径 (voxel_size の倍率) [-]。
  double density_radius_scale = 1.5;
  int coarse_max_iterations = 8;

  // --- 信頼初期姿勢選択 ---
  /// 粗解 T_align と動き予測 T_pred の並進差がこの閾値 [m] を超えたら T_pred を採用。
  double reliable_translation_tau = 1.5;

  // --- 適応しきい値 σ_th (motion-state weighted) ---
  double motion_decay = 1.5;     ///< 加速度変化に対する減衰係数 σ_decay
  double rot_scale = 2.0;        ///< 重み付き誤差の回転スケール σ_max
  double rot_beta = 1.0;         ///< 回転 tanh ゲイン β

  // --- 精位置合わせ ---
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double planarity_threshold = 0.5;  ///< これ以上を平面 (point-to-plane) とみなす

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct AdaptiveICPResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  double sigma_th = 0.0;       ///< 直近フレームの適応しきい値
  bool used_coarse_pose = false; ///< 粗解 T_align を初期姿勢に採用したか
};

/// Adaptive ICP LiDAR Odometry (arXiv:2509.22058)。
///
/// 信頼できる初期姿勢に基づく適応 ICP。各フレームで
///  (1) 密度フィルタした点で粗位置合わせ T_align を求め、
///  (2) 動き予測 T_pred と並進差で比較して信頼初期姿勢 T_init を選択、
///  (3) 現在/過去の誤差から適応しきい値 σ_th を更新し、
///  (4) ロバスト重み β=σ_th²/(σ_th²+e²) の point-to-plane ICP を local map に対し実行する。
class AdaptiveICPPipeline {
public:
  explicit AdaptiveICPPipeline(const AdaptiveICPParams& params = AdaptiveICPParams());

  AdaptiveICPResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;

  /// 密度フィルタ: 近傍点数が α-percentile 以上の点だけ残す。
  std::vector<Eigen::Vector3d> densityFilter(
      const std::vector<Eigen::Vector3d>& points) const;

  /// 動き予測姿勢 T_pred を直近の相対姿勢から推定する。
  Eigen::Matrix4d predictMotion() const;

  /// ロバスト重み付き point-to-plane ICP。max_iters / sigma_th で挙動を変える。
  Eigen::Matrix4d runICP(const std::vector<Eigen::Vector3d>& source,
                         const Eigen::Matrix4d& initial_guess,
                         double sigma_th, int max_iters) const;

  /// 適応しきい値 σ_th を motion-state 重み付き誤差履歴から計算する。
  double computeSigmaTh() const;

  AdaptiveICPParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();

  std::deque<Eigen::Matrix4d> recent_deltas_;  ///< 直近の相対姿勢 (動き予測用)
  std::deque<double> weighted_errors_;         ///< σ_th 用の重み付き誤差履歴
  std::deque<double> motion_weights_;          ///< 対応する γ_i
  Eigen::Vector3d last_velocity_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d last_accel_ = Eigen::Vector3d::Zero();
  int frame_count_ = 0;
};

}  // namespace adaptive_icp
}  // namespace localization_zoo
