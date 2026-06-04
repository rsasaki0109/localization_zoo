#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace pcr_dat {

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

/// ボクセルハッシュマップ。各クエリ点に対し近傍の点数・分布(平均/共分散)・法線を
/// 返し、PCR-DAT の距離/分布ファクタ切替に必要な統計量を供給する。
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d nearest = Eigen::Vector3d::Zero();  // 最近傍点 (距離ファクタ用)
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();     // 近傍分布の平均
    Eigen::Matrix3d cov = Eigen::Matrix3d::Identity();  // 近傍分布の共分散
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();   // 局所平面法線
    double planarity = 0.0;
    int neighbor_count = 0;
    bool found = false;
    bool has_distribution = false;  // 共分散/法線が有効か
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);
  std::vector<Correspondence> getCorrespondences(
      const std::vector<Eigen::Vector3d>& points, double max_dist,
      int min_neighbors) const;
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

struct PcrDatParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double initial_threshold = 2.0;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  // --- PCR-DAT: 距離ファクタ / 分布ファクタの per-point 切替 ---
  /// 近傍点数がこの値以上なら「特徴豊富(rich)」とみなし分布(Gauss)ファクタを使う。
  /// 未満なら「疎(sparse)」とみなし距離(point-to-plane)ファクタを使う。
  int distribution_min_points = 8;
  /// rich 判定の追加条件: 局所平面性がこの値以上 (分布が面として有効か)。
  double distribution_planarity = 0.1;
  /// 距離ファクタの point-to-plane に要求する平面性。未満は point-to-point に退避。
  double distance_planarity = 0.4;
  /// 距離ファクタのノイズ σ (情報重み 1/σ²)。
  double distance_sigma = 0.1;
  /// 分布ファクタの共分散正則化 (最小固有値を λmax のこの比に下限化、NDT 同様)。
  double distribution_regularization = 0.01;
  /// 距離ファクタ全体の相対重み (分布ファクタを 1 とした比)。
  double distance_weight = 1.0;
  /// 分布ファクタ全体の相対重み。
  double distribution_weight = 1.0;
  /// Cauchy ロバスト核スケール (残差 / scale)。<=0 で無効。
  double robust_scale = 1.0;
  /// 予測 (等速、IMU 事前積分ファクタの KITTI 代替) への prior 精度。
  double prior_precision = 0.0;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct PcrDatResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  /// 分布(rich)ファクタを使った対応数。
  int num_distribution = 0;
  /// 距離(sparse)ファクタを使った対応数。
  int num_distance = 0;
};

/// PCR-DAT: 距離(distance)ファクタと Gauss 分布(distribution)ファクタを点ごとに
/// 切り替える点群レジストレーション (Intelligent Service Robotics 2024,
/// DOI:10.1007/s11370-024-00517-6) の from-paper 再実装。著者コード未公開。
///
///  伝統的 ICP は rich/sparse が混在する領域で局所最適に陥りやすい。PCR-DAT は
///  局所点密度に応じてファクタを使い分ける:
///   - 疎な特徴 (近傍点が少ない) → 距離ファクタ (point-to-plane 残差 n·(p-μ))
///   - 豊富な特徴 (近傍点が多く面分布が立つ) → 分布ファクタ (Mahalanobis 残差
///     (p-μ)ᵀ Σ⁻¹ (p-μ)、GICP/NDT 系の point-to-distribution)
///  両ファクタを 1 つの Gauss-Newton 系に統合し、IMU 事前積分ファクタ (KITTI では
///  等速予測 prior で代替) を加える。
///
/// 純 LiDAR scope。IMU 緩結合は範囲外 (KITTI は IMU 無し→CV 予測)。
class PcrDatPipeline {
public:
  explicit PcrDatPipeline(const PcrDatParams& params = PcrDatParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  PcrDatResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& base,
                                  PcrDatResult* result);

  PcrDatParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace pcr_dat
}  // namespace localization_zoo
