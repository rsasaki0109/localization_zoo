#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace r_voxelmap {

/// ボクセル座標ハッシュ。
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

/// 平面特徴。
struct Plane {
  Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
  Eigen::Vector3d normal = Eigen::Vector3d::Zero();
  double plane_variance = 0.0;  ///< 平面厚み (= λ_min)
  bool valid = false;
};

/// 再帰的平面フィッティングを行う octree ボクセルマップ (R-VoxelMap)。
///
/// 各ルートボクセルで平面を最小二乗フィットし外れ値を検出、
/// 外れ値を子オクタントへ伝播して再帰的に平面フィットする
/// (outlier detect-and-reuse)。点分布による妥当性チェックで
/// 異なる物理平面の誤マージを防ぐ。
class RecursiveVoxelMap {
public:
  RecursiveVoxelMap(double voxel_size, int min_points, double planarity_ratio,
                    double inlier_dist, int max_depth)
      : voxel_size_(voxel_size),
        min_points_(min_points),
        planarity_ratio_(planarity_ratio),
        inlier_dist_(inlier_dist),
        max_depth_(max_depth) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);

  /// クエリ点に対し、octree を降りて最も細かい有効平面を返す。
  Plane query(const Eigen::Vector3d& world_point) const;

  void pruneFarVoxels(const Eigen::Vector3d& center, double max_distance);
  void clear() { roots_.clear(); }
  size_t size() const { return roots_.size(); }

private:
  Eigen::Vector3i toVoxel(const Eigen::Vector3d& p) const {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / voxel_size_)),
                           static_cast<int>(std::floor(p.y() / voxel_size_)),
                           static_cast<int>(std::floor(p.z() / voxel_size_)));
  }

  struct Node {
    std::vector<Eigen::Vector3d> points;  ///< このレベルの (再フィット用) 点
    Plane plane;
    Eigen::Vector3d center = Eigen::Vector3d::Zero();  ///< ノード中心
    double half = 0.0;                                 ///< 半辺長
    int depth = 0;
    std::array<std::unique_ptr<Node>, 8> children;
    bool has_children = false;
    bool dirty = true;
  };

  /// ノードの平面を (再)フィットし、外れ値を子へ伝播する。
  void refit(Node& node) const;
  /// クエリ点を含む子オクタント index。
  int childIndex(const Node& node, const Eigen::Vector3d& p) const;
  const Plane* queryNode(const Node& node, const Eigen::Vector3d& p) const;

  double voxel_size_;
  int min_points_;
  double planarity_ratio_;
  double inlier_dist_;
  int max_depth_;
  std::unordered_map<Eigen::Vector3i, std::unique_ptr<Node>, VoxelHash> roots_;
};

struct RVoxelMapParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int voxel_min_points = 6;
  double planarity_ratio = 0.1;     ///< λ0 < ratio·λ2 で平面とみなす
  double inlier_dist = 0.1;         ///< 平面インライア距離 [m]
  int max_depth = 2;                ///< octree 再帰の最大深さ

  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double max_correspondence_dist = 2.0;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct RVoxelMapResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  double matched_ratio = 0.0;  ///< 平面対応が取れた点の割合
};

/// R-VoxelMap (arXiv:2601.12377) の再帰的平面ボクセルマップ odometry。
class RVoxelMapPipeline {
public:
  explicit RVoxelMapPipeline(const RVoxelMapParams& params = RVoxelMapParams());

  RVoxelMapResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d runICP(const std::vector<Eigen::Vector3d>& source,
                         const Eigen::Matrix4d& initial_guess,
                         double* matched_ratio_out);

  RVoxelMapParams params_;
  RecursiveVoxelMap map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  int frame_count_ = 0;
};

}  // namespace r_voxelmap
}  // namespace localization_zoo
