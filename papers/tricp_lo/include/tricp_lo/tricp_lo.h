#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace tricp_lo {

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

struct TricpLoParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double max_correspondence_dist = 2.0;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  // --- Trimmed / Fractional ICP (Least Trimmed Squares) ---
  /// true なら毎反復 FRMSD を最小化してオーバーラップ率 ξ を自動推定する (FICP)。
  bool auto_overlap = true;
  /// 固定モード (auto_overlap=false) で用いる ξ (残差最小の ξ 割合のみ採用)。
  double overlap_ratio = 0.9;
  /// 自動推定の下限 ξ_min。連続 LiDAR スキャンのオーバーラップは ~85-95% と高く、
  /// FRMSD は残差連続分布だと ξ を過剰に縮めるため、現実的な下限で抑える。
  double min_overlap = 0.8;
  /// FRMSD のペナルティ指数 λ。FRMSD(ξ)=sqrt(e(ξ))/ξ^λ。
  double frmsd_lambda = 1.5;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct TricpLoResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  /// トリミング後に採用された inlier 対応数 (最終反復)。
  int num_inliers = 0;
  /// 推定されたオーバーラップ率 ξ (最終反復)。
  double overlap = 0.0;
};

/// TrICP-LO: Trimmed / Fractional ICP (Least Trimmed Squares) point-to-plane
/// LiDAR オドメトリ。
///
///   "The Trimmed Iterative Closest Point Algorithm" (D. Chetverikov et al.,
///   ICPR 2002) / "Robust Euclidean alignment of 3D point sets: the trimmed
///   ICP algorithm" (Image Vis. Comput. 2005) + "Outlier Robust ICP for
///   Minimizing Fractional RMSD" (J. Phillips et al., 3DIM 2007) の最小トリム
///   二乗 (LTS) ロバスト推定機構を scan-to-map LiDAR オドメトリへ適用した
///   from-paper 再構成。原論文は対点群レジストレーションのみで、本リポジトリ
///   向けオドメトリパイプラインとしての公開実装は無い。
///
/// M 推定子 (Student-T/Cauchy/MCC) や GNC が残差の **大きさ**で連続的に重みを
/// 下げるのに対し、TrICP は残差を**順位**でソートし最良 ξ 割合のみを最小二乗に
/// 使う高破壊点ロバスト推定 (LTS)。さらに ξ を FRMSD(ξ)=sqrt(e(ξ))/ξ^λ の最小化で
/// **自動推定**し、連続スキャン間の非オーバーラップ領域 (新出/消失/動物体) を
/// 明示的に棄却する点が本機構の特徴。
class TricpLoPipeline {
public:
  explicit TricpLoPipeline(const TricpLoParams& params = TricpLoParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  TricpLoResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

  // --- 機構ヘルパ (ユニットテスト用に公開) ---

  /// 昇順ソート済み二乗残差の、最小 ⌊ξN⌋ 個の平均 e(ξ)。
  static double trimmedMeanSqResidual(const std::vector<double>& sorted_sq,
                                      double xi);
  /// FRMSD(ξ) = sqrt(e(ξ)) / ξ^λ (昇順ソート済み二乗残差から計算)。
  static double frmsd(const std::vector<double>& sorted_sq, double xi,
                      double lambda);
  /// FRMSD を最小化するオーバーラップ率 ξ* を返す ([min_overlap, 1] を走査)。
  /// 入力は未ソートでよい (内部でソート)。
  static double estimateOverlap(std::vector<double> sq_residuals,
                                double min_overlap, double lambda);

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& base,
                                  TricpLoResult* result);

  TricpLoParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace tricp_lo
}  // namespace localization_zoo
