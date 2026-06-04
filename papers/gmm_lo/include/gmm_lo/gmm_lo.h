#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace gmm_lo {

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

/// GMM ソフト対応つきボクセルハッシュマップ。各クエリ点に対し近傍モデル点を
/// Gaussian responsibility で重み付けしたソフト重心と局所法線を返す。
class VoxelHashMap {
public:
  struct SoftCorrespondence {
    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();  // ソフト重心 (Σγq/Σγ)
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double weight = 0.0;       // インライア信頼度 W/(W+outlier)
    double planarity = 0.0;
    bool found = false;
    bool has_normal = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);
  /// sigma を分散とする Gaussian responsibility でソフト対応を計算する。
  std::vector<SoftCorrespondence> getSoftCorrespondences(
      const std::vector<Eigen::Vector3d>& points, double max_dist, double sigma,
      double outlier_weight, int normal_min_neighbors) const;
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

struct GmmLoParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  // --- GMM / EM ソフト割当 ---
  /// deterministic annealing の初期標準偏差 σ [m] (大=広い収束域, ソフト割当)。
  double sigma_init = 1.2;
  /// annealing 最終 σ [m] (小=シャープ割当, ほぼ hard NN)。
  double sigma_final = 0.25;
  /// 一様アウトライア項 (responsibility 正規化に加算, 外れ値の自動棄却)。
  double outlier_weight = 0.1;
  /// 近傍探索半径 = neighbor_radius_scale * σ (ボクセル環で上限)。
  double neighbor_radius_scale = 3.0;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct GmmLoResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  /// 最終反復の平均インライア信頼度 (Σγ/(Σγ+outlier))。
  double mean_weight = 0.0;
};

/// GMM-LO: Gaussian-Mixture / EM ソフト割当 LiDAR オドメトリ。
///
///  "Fast and Accurate Point Cloud Registration using Trees of Gaussian
///  Mixtures" (HGMR, Eckart et al., 2018, arXiv:1807.02587) の GMM-EM 登録機構を
///  論文記述から compact 再構成したもの。著者公開コードは無い。
///
///  hard nearest-neighbor (ICP) や hard voxel 割当 (NDT) と異なり、各ソース点を
///  近傍モデル点群が成す GMM に **ソフトに対応付ける**:
///   E-step: responsibility γ_ij ∝ exp(-‖Tp_i - q_j‖² / 2σ²)、一様アウトライア項で
///           正規化。ソフト重心 m_i = Σ_j γ_ij q_j / Σ_j γ_ij、信頼度 w_i。
///   M-step: Σ_i w_i [n_i·(Tp_i - m_i)]² を Gauss-Newton で最小化 (ソフト重心への
///           point-to-plane)。
///  σ を反復で sigma_init→sigma_final に **deterministic annealing** し、粗から密へ
///  (広い収束域→シャープな割当) と段階的に絞る。完全な GMM tree は範囲外で、
///  単一スケールのボクセル近傍 GMM に圧縮している。
class GmmLoPipeline {
public:
  explicit GmmLoPipeline(const GmmLoParams& params = GmmLoParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  GmmLoResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

  /// Gaussian responsibility ソフト重心 (テスト用に公開)。
  static Eigen::Vector3d softCentroid(const Eigen::Vector3d& query,
                                      const std::vector<Eigen::Vector3d>& neighbors,
                                      double sigma);

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& base,
                                  GmmLoResult* result);

  GmmLoParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace gmm_lo
}  // namespace localization_zoo
