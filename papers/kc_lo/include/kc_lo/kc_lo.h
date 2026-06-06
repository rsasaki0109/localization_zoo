#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace kc_lo {

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

/// 点を格納するボクセルハッシュマップ。カーネル相関では対応点を取らず、各クエリ点に
/// 対する近傍モデル点の **親和度重み平均** (mean-shift 的ターゲット) を返す。
class VoxelHashMap {
public:
  /// クエリ点に対するカーネル相関ターゲット。
  struct KcTarget {
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();  // 親和度重み平均 ȳ
    double weight = 0.0;                              // 総親和度 Σ w_j (局所密度)
    bool found = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);
  /// 各クエリ点について半径 radius 内のモデル点の親和度 w=exp(−d²/2σ²) 重み平均を返す。
  std::vector<KcTarget> kernelTargets(const std::vector<Eigen::Vector3d>& points,
                                      double sigma, double radius) const;
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

struct KcLoParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  // --- Kernel Correlation (Renyi 二次エントロピー) ---
  /// 最終 (最小) カーネルスケール σ_min [m]。
  double kc_sigma = 0.5;
  /// 初期 (粗) カーネルスケール σ_init [m] (アニーリング開始値)。
  double kc_sigma_init = 2.0;
  /// 毎反復 σ ← max(σ*anneal, σ_min) で縮める (coarse-to-fine)。
  double kc_anneal = 0.7;
  /// 探索半径 = radius_factor * σ。
  double kc_radius_factor = 3.0;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct KcLoResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;  // ターゲットが見つかった点数 (最終反復)
  /// 最終反復で用いたカーネルスケール σ。
  double sigma_used = 0.0;
  /// 最終反復の平均総親和度 (局所密度の指標)。
  double mean_weight = 0.0;
};

/// KC-LO: Kernel Correlation (対応点を取らない / correspondence-free) LiDAR
/// オドメトリ。
///
///   "A Correlation-Based Approach to Robust Point Set Registration"
///   (Y. Tsin & T. Kanade, ECCV 2004) のカーネル相関機構を scan-to-map LiDAR
///   オドメトリへ適用した from-paper 再構成。原論文は対点群レジストレーション
///   のみで、本リポジトリ向けオドメトリパイプラインとしての公開実装は無い。
///
/// 変換後スキャン X とモデル Y の総親和度 (カーネル相関)
///   C(θ) = Σ_i Σ_j exp(−||T(θ)x_i − y_j||² / 2σ²)
/// を最大化する (= 結合点群密度の Renyi 二次エントロピー最小化)。ICP の最近傍
/// 対応も、GMM-EM の潜在割当も、NDT のボクセル離散化も用いない点が本機構の特徴。
/// 各反復で各スキャン点を近傍モデル点の親和度重み平均へ引き寄せる soft な
/// point-to-point 更新を行い、σ を粗→細にアニーリングして広い収束域を得る。
class KcLoPipeline {
public:
  explicit KcLoPipeline(const KcLoParams& params = KcLoParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  KcLoResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

  // --- 機構ヘルパ (ユニットテスト用に公開) ---

  /// ガウスカーネル親和度 w = exp(−dist² / 2σ²)。
  static double kernelWeight(double dist_sq, double sigma);
  /// 近傍点集合の親和度重み平均 ȳ = Σ w_j y_j / Σ w_j (w_j=kernelWeight)。
  /// weight_out に総親和度 Σ w_j を返す。
  static Eigen::Vector3d weightedMean(const std::vector<Eigen::Vector3d>& pts,
                                      const Eigen::Vector3d& query, double sigma,
                                      double* weight_out);

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& base,
                                  KcLoResult* result);

  KcLoParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace kc_lo
}  // namespace localization_zoo
