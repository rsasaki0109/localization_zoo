#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace lodestar {

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

/// 平面法線つきボクセルハッシュマップ。
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

struct LodestarParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double initial_threshold = 2.0;

  // --- DA-ASKF (退化対応適応 Schmidt-Kalman フィルタ) ---
  /// 条件数しきい値 T_χ。χ(H)=λmax/λmin ≥ T_χ の方向を退化と判定。
  /// 論文値は DA-DE 正規化後 Jacobian に対する 1.5。raw Hessian なので
  /// 各方向 (λ_k < λmax / T_χ) に適用し、対称シーンでは χ≈1 で非退化。
  double t_chi = 1.5;
  /// 退化方向に注入する anchor 情報の強さ係数 (fixed 状態の共分散逆を近似)。
  double anchor_strength = 1.0;
  /// 等速予測を平均する active 過去状態数 s_a。
  int active_window = 2;

  // --- DA-DE (退化対応データ活用) ---
  bool enable_data_exploitation = true;
  /// 局所化寄与しきい値 T_loc = cos(35°)。退化フレームで剪定に使用。
  double t_loc = 0.819;
  /// 剪定後の最小残存率 (これを下回る剪定は破棄して全点使用)。
  double min_keep_ratio = 0.3;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct LodestarResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  double condition_number = 1.0;        ///< χ(H) 全 6x6 の条件数
  double condition_number_trans = 1.0;  ///< χ(H_tt) 並進ブロックの条件数
  bool degenerate = false;              ///< このフレームが退化判定されたか
  int num_degenerate_dirs = 0;          ///< 退化方向 (anchor 注入) の数
  int num_pruned = 0;                   ///< DA-DE で剪定された測定数
  int num_active = 0;                   ///< 活用された (active) 測定数
};

/// LODESTAR: Degeneracy-Aware LiDAR-Inertial Odometry with Adaptive
/// Schmidt-Kalman Filter and Data Exploitation (arXiv:2511.09142, KAIST,
/// IEEE RA-L 2025)。著者コード未公開の from-paper 再実装。
///
///  (1) DA-ASKF: registration Hessian H の固有分解で条件数 χ(H)=λmax/λmin を
///      計算。λ_k < λmax/T_χ の方向を退化とみなし、滑動窓の fixed 過去状態を
///      anchor (lodestar) として、その方向の解を等速予測へ Schmidt-Kalman 的に
///      固定する (fixed 状態は更新せず共分散で active 状態を拘束)。
///  (2) DA-DE: 退化フレームでは局所化寄与 Ω=|n·v_ℓ| が T_loc 未満の曖昧な測定を
///      剪定し、退化方向に寄与する測定で χ を補償する。
///
/// 純 LiDAR scope。KITTI は IMU 無のため IMU 予測は等速 (constant-velocity) で
/// 代替し、fixed 状態 anchor は等速予測として表現する (README 参照)。
class LodestarPipeline {
public:
  explicit LodestarPipeline(const LodestarParams& params = LodestarParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }

  LodestarResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  /// DA-ASKF + DA-DE つき point-to-plane registration。
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& initial_guess,
                                  const Eigen::Matrix4d& prediction,
                                  LodestarResult* result);
  Eigen::Matrix4d predict() const;

  LodestarParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;  ///< 滑動窓 (active 状態の相対運動)
  int frame_count_ = 0;
};

}  // namespace lodestar
}  // namespace localization_zoo
