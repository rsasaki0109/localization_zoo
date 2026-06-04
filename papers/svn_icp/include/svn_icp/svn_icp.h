#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace svn_icp {

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

/// 平面法線つきボクセルハッシュマップ (point-to-plane 用)。
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

struct SvnIcpParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double initial_threshold = 2.0;
  /// MAP 点推定 (GN point-to-plane) の最大反復と収束しきい値。
  int max_gn_iterations = 20;
  double convergence_criterion = 0.001;

  // --- Stein Variational Newton ICP ---
  /// 粒子数 M (姿勢事後分布をこの数の粒子で近似)。
  int num_particles = 12;
  /// 1 フレームあたりの SVN 反復回数。
  int svn_iterations = 4;
  /// SVN Newton ステップのステップ幅 α。
  double step_size = 1.0;
  /// LiDAR 点ノイズ σ (point-to-plane 尤度 1/σ²)。
  double lidar_sigma = 0.1;
  /// 予測 (等速) への prior 精度 (θ=0 へ引く弱い正則化)。
  double prior_precision = 1.0;
  /// 粒子初期散布 std (回転 rad / 並進 m、予測まわり)。
  double init_spread_rot = 0.01;
  double init_spread_trans = 0.05;
  /// RBF 核バンド幅の下限 (median heuristic が小さすぎる時の床)。
  double min_bandwidth = 1e-4;
  int rng_seed = 12345;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct SvnIcpResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  /// 粒子から推定した姿勢共分散のトレース (不確かさ指標)。
  double covariance_trace = 0.0;
  /// 並進成分の不確かさ (共分散並進ブロックのトレース sqrt, m)。
  double trans_std = 0.0;
  /// 回転成分の不確かさ (rad)。
  double rot_std = 0.0;
  int num_correspondences = 0;
};

/// SVN-ICP: Uncertainty Estimation of ICP-based LiDAR Odometry using Stein
/// Variational Newton (arXiv:2509.08069, Ma/Zhang/Toussaint 2025) の from-paper
/// 再実装。著者コード未公開。
///
///  姿勢事後分布を M 個の粒子で近似し、Stein Variational Newton (SVN) で更新する。
///  各粒子 θ_i (予測 T_base まわりの SE3 左接ベクトル) について point-to-plane 尤度の
///  勾配 g_i と Gauss-Newton Hessian H_i を計算し、RBF 核 (バンド幅は median
///  heuristic) で粒子間に引力 (勾配) と斥力 (核勾配) を働かせる:
///    φ(θ_i) = (1/M) Σ_j [ k(θ_j,θ_i)·∇log p(θ_j) + ∇_{θ_j} k(θ_j,θ_i) ]
///    H_svn(θ_i) = (1/M) Σ_j [ k(θ_j,θ_i)²·H_j + ∇k ∇kᵀ ]
///    θ_i ← θ_i + α · H_svn(θ_i)⁻¹ φ(θ_i)
///  論文題目どおり「ICP の点推定 + 不確かさ推定」: 出力姿勢は point-to-plane の
///  MAP (GN 収束解、長系列で安定)、姿勢共分散は MAP 周りの粒子分散から推定する
///  (明示的ノイズモデル不要)。素の粒子平均は repulsion でモードからずれ長系列
///  (KITTI seq00) で発散するため、点推定は GN-MAP に分離した (README 参照)。
///
/// 純 LiDAR scope。IMU 予測は等速 (constant-velocity)、IMU との緩結合 ESKF は範囲外。
class SvnIcpPipeline {
public:
  explicit SvnIcpPipeline(const SvnIcpParams& params = SvnIcpParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  SvnIcpResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d predict() const;
  /// SVN 推論を走らせ、粒子平均姿勢と共分散を result に書く。
  Eigen::Matrix4d runSvn(const std::vector<Eigen::Vector3d>& source,
                         const Eigen::Matrix4d& base, SvnIcpResult* result);

  SvnIcpParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace svn_icp
}  // namespace localization_zoo
