#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace quadric_lo {

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

using Vec10 = Eigen::Matrix<double, 10, 1>;

/// 局所近傍に二次曲面 (quadric) implicit function を当てはめたボクセルハッシュ
/// マップ。各クエリ点に対し中心化座標での quadric 係数 θ (10) と平面フォールバック
/// 用の局所 PCA を返す。
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d nearest = Eigen::Vector3d::Zero();
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();   // 近傍中心 (中心化原点)
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();  // 平面フォールバック用法線
    Vec10 quadric = Vec10::Zero();  // [A00,A11,A22,A01,A02,A12,b0,b1,b2,c] (中心化座標)
    double planarity = 0.0;
    int neighbor_count = 0;
    bool found = false;
    bool has_plane = false;
    bool has_quadric = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);
  std::vector<Correspondence> getCorrespondences(
      const std::vector<Eigen::Vector3d>& points, double max_dist,
      int plane_min_neighbors, int quadric_min_neighbors) const;
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

/// 中心化座標 y に対する quadric 単項式ベクトル m(y) (q(y)=m(y)·θ)。
Vec10 quadricMonomials(const Eigen::Vector3d& y);
/// quadric 係数 θ から対称行列 A を復元 (q=yᵀAy+bᵀy+c)。
Eigen::Matrix3d quadricMatrixA(const Vec10& theta);

struct QuadricLoParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int plane_min_neighbors = 5;
  double initial_threshold = 2.0;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  // --- Quadric Representations ---
  /// quadric 当てはめに要する近傍点数 (10 パラメータの安定 fit 用)。未満は平面に退避。
  int quadric_min_neighbors = 14;
  /// 平面フォールバックに要求する平面性下限。
  double planarity_threshold = 0.4;
  /// quadric が立たない局所近傍を point-to-plane に退避する。
  bool allow_plane_fallback = true;
  /// quadric 拘束の相対重み (平面フォールバックを 1 とした比)。
  double quadric_weight = 1.0;
  /// Taubin 距離の勾配ノルム下限 (退化 quadric の弾き)。
  double min_grad_norm = 1e-3;
  /// Cauchy ロバスト核スケール (<=0 で無効)。
  double robust_scale = 1.0;
  /// 等速予測 (IMU 無し KITTI 代替) への prior 精度。
  double prior_precision = 0.0;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct QuadricLoResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  /// quadric (二次曲面) 拘束に使われた対応数。
  int num_quadric = 0;
  /// 平面フォールバックに使われた対応数。
  int num_plane = 0;
};

/// Quadric-LO: Quadric Representations for LiDAR Odometry, Mapping and
/// Localization (Xia et al., IEEE RA-L 2023, arXiv:2304.14190) の from-paper
/// 再実装。公開実装は確認できず、本実装は論文記述からの独自再構成。
///
///  シーンを点や平面でなく二次曲面 (quadric: 円柱/球/楕円体/平面はその退化) の
///  implicit function q(x)=xᵀAx+bᵀx+c で表現する。局所近傍に quadric を代数当てはめ
///  (M=Σ m mᵀ の最小固有ベクトル) し、ソース点の point-to-quadric Taubin 距離
///  d=q(p)/‖∇q(p)‖ を最小化する。曲面構造 (木の幹=円柱 等) を平面より忠実に表す。
///  近傍が少なく quadric が立たない場合は point-to-plane に退避する。
///
/// 純 LiDAR scope。IMU 緩結合は範囲外 (KITTI は IMU 無し→等速予測)。
class QuadricLoPipeline {
public:
  explicit QuadricLoPipeline(const QuadricLoParams& params = QuadricLoParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  QuadricLoResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

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
                                  QuadricLoResult* result);

  QuadricLoParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace quadric_lo
}  // namespace localization_zoo
