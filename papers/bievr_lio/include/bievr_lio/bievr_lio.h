#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace bievr_lio {

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

/// Bump-image (voxel-wise oriented height image) ボクセルマップ。
///
/// 各ボクセルで支配平面 (重心 μ, 法線 n) を推定し、平面に整列した局所枠
/// (z=n, x/y=面内基底) を張る。面内 KxK グリッドの各セルに平面からの
/// 高さ偏差 (bump) を蓄積する。登録時は法線方向の point-to-plane に加え、
/// 高さ画像の勾配から面内 2 方向の拘束を得る (BIEVR-LIO の核)。
class BumpVoxelMap {
public:
  struct Feature {
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    Eigen::Vector3d ex = Eigen::Vector3d::Zero();  ///< 面内基底 x
    Eigen::Vector3d ey = Eigen::Vector3d::Zero();  ///< 面内基底 y
    double height = 0.0;     ///< クエリ点セルの bump 高さ I(u,v)
    double grad_u = 0.0;     ///< 高さ画像勾配 ∂I/∂u
    double grad_v = 0.0;     ///< 高さ画像勾配 ∂I/∂v
    double plane_variance = 0.0;
    bool found = false;
    bool has_plane = false;
  };

  BumpVoxelMap(double voxel_size, int min_points, double planarity_ratio,
               double pixel_res)
      : voxel_size_(voxel_size),
        min_points_(min_points),
        planarity_ratio_(planarity_ratio),
        pixel_res_(pixel_res),
        grid_(std::max(1, static_cast<int>(std::round(voxel_size / pixel_res)))) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);
  Feature query(const Eigen::Vector3d& world_point) const;
  void pruneFarVoxels(const Eigen::Vector3d& center, double max_distance);
  void clear() { voxels_.clear(); }
  size_t size() const { return voxels_.size(); }

private:
  Eigen::Vector3i toVoxel(const Eigen::Vector3d& p) const {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / voxel_size_)),
                           static_cast<int>(std::floor(p.y() / voxel_size_)),
                           static_cast<int>(std::floor(p.z() / voxel_size_)));
  }

  struct Voxel {
    int count = 0;
    Eigen::Vector3d sum = Eigen::Vector3d::Zero();
    Eigen::Matrix3d sum_outer = Eigen::Matrix3d::Zero();
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    Eigen::Vector3d ex = Eigen::Vector3d::Zero();
    Eigen::Vector3d ey = Eigen::Vector3d::Zero();
    double plane_variance = 0.0;
    bool has_plane = false;
    std::vector<double> img;  ///< grid_*grid_ の高さ
    std::vector<double> wimg; ///< grid_*grid_ の重み
    std::vector<Eigen::Vector3d> recent;  ///< bump 再構築用の最近点
  };

  void rebuild(Voxel& v) const;
  bool pixelOf(const Voxel& v, const Eigen::Vector3d& p, int& iu, int& iv,
               double& fu, double& fv, double& h) const;

  double voxel_size_;
  int min_points_;
  double planarity_ratio_;
  double pixel_res_;
  int grid_;
  std::unordered_map<Eigen::Vector3i, Voxel, VoxelHash> voxels_;
};

struct BievrLIOParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int voxel_min_points = 8;
  double planarity_ratio = 0.15;
  double pixel_res = 0.2;       ///< bump 画像の画素解像度 [m]
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double max_correspondence_dist = 2.0;
  double huber_delta = 0.2;     ///< Huber ロバストしきい [m]
  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct BievrLIOResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  double bump_constraint_ratio = 0.0;  ///< 画像勾配拘束が効いた点の割合
};

/// BIEVR-LIO (arXiv:2604.14421) の bump-image-enhanced voxel map odometry。
class BievrLIOPipeline {
public:
  explicit BievrLIOPipeline(const BievrLIOParams& params = BievrLIOParams());

  BievrLIOResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d runICP(const std::vector<Eigen::Vector3d>& source,
                         const Eigen::Matrix4d& initial_guess,
                         double* bump_ratio_out);

  BievrLIOParams params_;
  BumpVoxelMap map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  int frame_count_ = 0;
};

}  // namespace bievr_lio
}  // namespace localization_zoo
