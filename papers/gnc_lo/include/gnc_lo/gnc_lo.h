#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace gnc_lo {

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

struct GncLoParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double max_correspondence_dist = 2.0;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  // --- Graduated Non-Convexity (TLS) ---
  /// インライア残差の切断しきい値 c̄ [m] (point-to-plane 残差)。これを超えると外れ値。
  double gnc_truncation = 0.3;
  /// 非凸性の継続係数 (μ ← gnc_factor * μ, 凸→TLS へ段階的に絞る)。
  double gnc_factor = 1.4;
  /// μ の初期値 (<=0 なら最大残差から自動初期化し凸スタートにする)。
  double gnc_mu_init = 0.0;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct GncLoResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  /// 最終反復のインライア率 (平均 GNC 重み, 1 に近いほど外れ値少)。
  double inlier_ratio = 0.0;
  /// 最終反復の継続パラメータ μ。
  double mu_final = 0.0;
};

/// GNC-LO: Graduated Non-Convexity ロバスト point-to-plane LiDAR オドメトリ。
///
///  "Graduated Non-Convexity for Robust Spatial Perception" (Yang, Antonante,
///  Tzoumas, Carlone, IEEE RA-L 2020, arXiv:1909.08605) の GNC-TLS ロバスト推定
///  機構を LiDAR オドメトリに適用した from-paper 再構成。著者の参照実装は
///  認識系ライブラリ内にあり、本リポジトリ向けの scan-to-map オドメトリとしての
///  公開実装は無い。
///
///  M 推定子 (Cauchy/Student-t の固定カーネル) や EM の σ-annealing と異なり、GNC は
///  **代理関数の非凸性そのものを連続変化させる継続法**:
///   - Truncated Least Squares (TLS) を Black-Rangarajan 双対で重み付き最小二乗化。
///   - 制御変数 μ を凸に近い値から始め (広い収束域)、μ ← gnc_factor·μ で徐々に
///     非凸 TLS へ近づける。各 μ で GNC-TLS 重み
///       w = 1                           (r² ≤ μ/(μ+1) c̄²,  インライア)
///       w = 0                           (r² ≥ (μ+1)/μ c̄²,  アウトライア)
///       w = (c̄/|r|)√(μ(μ+1)) − μ        (その間, ソフト)
///     を用い point-to-plane を解く。最終的に外れ値は重み 0 で hard 棄却される。
class GncLoPipeline {
public:
  explicit GncLoPipeline(const GncLoParams& params = GncLoParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  GncLoResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

  /// GNC-TLS 重み (テスト用に公開)。
  static double gncWeight(double residual, double truncation, double mu);

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& base,
                                  GncLoResult* result);

  GncLoParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace gnc_lo
}  // namespace localization_zoo
