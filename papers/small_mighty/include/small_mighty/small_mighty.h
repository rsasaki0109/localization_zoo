#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace small_mighty {

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

/// 平面法線・線方向つきボクセルハッシュマップ。局所 PCA から平面性/線形性を返し、
/// point-to-plane / point-to-line 双方の対応に使う。ソース走査の局所 smoothness
/// 解析にも同じ構造を再利用する。
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d nearest = Eigen::Vector3d::Zero();
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();    // 最小固有ベクトル (面法線)
    Eigen::Vector3d line_dir = Eigen::Vector3d::Zero();  // 最大固有ベクトル (線方向)
    double planarity = 0.0;
    double linearity = 0.0;
    int neighbor_count = 0;
    bool found = false;
    bool has_pca = false;
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

/// 選択されたソース特徴 (stability-aware selection + contribution weight 付き)。
struct SourceFeature {
  Eigen::Vector3d point = Eigen::Vector3d::Zero();
  bool is_edge = false;  // true=線特徴, false=面特徴
  double weight = 1.0;   // contribution weight (安定特徴ほど大)
};

struct SmallMightyParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double initial_threshold = 2.0;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  // --- Small-but-Mighty: stability-aware selection + contribution weighting ---
  /// ソース走査の局所 PCA で面特徴とみなす平面性下限。
  double planar_min = 0.4;
  /// 線特徴とみなす線形性下限。
  double edge_min = 0.5;
  /// contribution weight: 平面性/線形性が分布平均から (val-μ)/σ だけ上の安定特徴を
  /// w = 1 + gain·clamp((val-μ)/σ, 0, cap) で強化する。
  double contribution_gain = 1.0;
  double contribution_cap = 3.0;
  /// 面特徴 / 線特徴の全体相対重み。
  double plane_weight = 1.0;
  double edge_weight = 0.5;
  /// Cauchy ロバスト核スケール (<=0 で無効)。
  double robust_scale = 1.0;
  /// 等速予測 (IMU 無し KITTI 代替) への prior 精度。
  double prior_precision = 0.0;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct SmallMightyResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  int num_planar = 0;
  int num_edge = 0;
};

/// Small-but-Mighty: A Lightweight Feature Enhancement Strategy for LiDAR
/// Odometry in Challenging Environments (Remote Sensing 2025, 17(15):2656) の
/// from-paper 再実装。著者コード未公開。
///
///  非地面特徴に対し局所 smoothness (局所 PCA の平面性/線形性) の統計分布から
///  安定な特徴を選び、その寄与を point-to-plane / point-to-line 拘束で強化する:
///   - stability-aware selection: 平面性/線形性の分布から面/線特徴を分類。
///   - contribution weighting: 分布平均より上の安定特徴ほど大きな重みを与え、
///     低品質残差の影響を抑制する (w = 1 + gain·(val-μ)/σ)。
///  軽量 (kd-tree 不使用、ボクセル局所 PCA のみ) を意図。
///
/// 純 LiDAR scope。IMU 緩結合は範囲外 (KITTI は IMU 無し→等速予測)。
class SmallMightyPipeline {
public:
  explicit SmallMightyPipeline(const SmallMightyParams& params = SmallMightyParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  SmallMightyResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

  /// stability-aware selection + contribution weighting (テスト用に公開)。
  std::vector<SourceFeature> selectFeatures(
      const std::vector<Eigen::Vector3d>& source) const;

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(const std::vector<SourceFeature>& features,
                                  const Eigen::Matrix4d& base,
                                  SmallMightyResult* result);

  SmallMightyParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace small_mighty
}  // namespace localization_zoo
