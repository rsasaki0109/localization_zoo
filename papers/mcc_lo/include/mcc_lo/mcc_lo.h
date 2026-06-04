#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace mcc_lo {

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

struct MccLoParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double max_correspondence_dist = 2.0;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  // --- Maximum Correntropy Criterion (MCC) ---
  /// 相関エントロピーのカーネルバンド幅 σ [m] の初期値 (固定モード時はこの値)。
  double mcc_sigma = 0.3;
  /// true なら毎反復 Silverman の経験則で σ を残差分布から適応推定する。
  bool mcc_adaptive_sigma = true;
  /// 適応 σ の下限 [m]。カーネルが収束信号 (未整合点の残差) まで棄却する σ→0 の
  /// 自己崩壊を防ぐ。annealing 無し correntropy-ICP は狭すぎる σ で過小収束する。
  double mcc_sigma_floor = 0.3;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct MccLoResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  /// 最終反復で用いたカーネルバンド幅 σ。
  double sigma_used = 0.0;
  /// 最終反復の平均相関エントロピー重み (1 に近いほど外れ値少)。
  double mean_weight = 0.0;
};

/// MCC-LO: Maximum Correntropy Criterion ロバスト point-to-plane LiDAR オドメトリ。
///
///  "Robust 3D point cloud registration based on bidirectional Maximum
///  Correntropy Criterion" (He et al., PLOS ONE 2018, doi:10.1371/
///  journal.pone.0197542) および "Correntropy based scale ICP algorithm for
///  robust point set registration" (Pattern Recognition 2019) の相関エントロピー
///  ロバスト推定機構を scan-to-map LiDAR オドメトリへ適用した from-paper 再構成。
///  原論文は対点群レジストレーションのみで、本リポジトリ向けオドメトリパイプライン
///  としての公開実装は無い。
///
///  二乗誤差 (MSE) を最小化する代わりに **相関エントロピー** V = Σ Gσ(r) =
///  Σ exp(−r²/2σ²) を最大化する (MCC)。半二次最適化で IRLS 重み
///    w = exp(−r²/2σ²)   (Welsch/Gauss カーネル)
///  となり、外れ値は指数的に減衰する。M 推定子 (Cauchy/Student-t) の重尾、GNC の
///  継続 TLS、GMM の soft assignment と異なり、情報理論的な相関尺度に基づく点と、
///  カーネルバンド幅 σ を Silverman の経験則で残差分布から **適応推定** する点が
///  本機構の特徴。
class MccLoPipeline {
public:
  explicit MccLoPipeline(const MccLoParams& params = MccLoParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  MccLoResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

  /// 相関エントロピー (Welsch/Gauss カーネル) 重み (テスト用に公開)。
  static double correntropyWeight(double residual, double sigma);

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& base,
                                  MccLoResult* result);

  MccLoParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace mcc_lo
}  // namespace localization_zoo
