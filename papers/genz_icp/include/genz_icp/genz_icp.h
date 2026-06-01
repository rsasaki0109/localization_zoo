#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <functional>
#include <vector>
#include <unordered_map>

namespace localization_zoo {
namespace genz_icp {

/// ボクセル座標ハッシュ (KISS-ICP と同じ方式)
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

/// GenZ-ICP 用ボクセルハッシュマップ。
/// 最近傍点に加えて、近傍点群から推定した局所平面法線と planarity を返す。
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();   ///< 最近傍点
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();  ///< 近傍重心 (平面アンカー)
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();  ///< 局所平面法線
    double planarity = 0.0;  ///< 平面度 [0,1] (大きいほど平面的)
    bool found = false;
    bool has_normal = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);

  /// 各クエリ点に対し最近傍点・平面法線・planarity を推定して返す。
  /// 法線は近傍 (27 ボクセル内) の点の共分散行列を固有値分解して求める。
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

struct GenZICPParams {
  double voxel_size = 0.5;          ///< マップボクセルサイズ [m]
  double max_range = 100.0;         ///< 最大距離 [m]
  double min_range = 3.0;           ///< 最小距離 [m]
  int max_points_per_voxel = 20;
  double initial_threshold = 2.0;   ///< 初期対応距離閾値 [m]
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double planarity_threshold = 0.55;  ///< これ以上を平面 (point-to-plane) とみなす
  int normal_min_neighbors = 5;       ///< 法線推定に必要な最小近傍点数
  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct GenZICPResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  double planar_ratio = 0.0;  ///< 直近フレームの平面対応比率
  double alpha = 0.0;         ///< 直近フレームの adaptive weight (point-to-point 側)
};

/// GenZ-ICP パイプライン。
/// KISS-ICP と同じ前処理・等速モデル・適応的閾値を用いつつ、
/// ICP solve を point-to-plane / point-to-point の adaptive hybrid に置き換える。
class GenZICPPipeline {
public:
  explicit GenZICPPipeline(const GenZICPParams& params = GenZICPParams());

  GenZICPResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;

  /// Adaptive hybrid ICP. 平面/非平面の残差を適応的重みで結合する。
  Eigen::Matrix4d runICP(const std::vector<Eigen::Vector3d>& source,
                         const Eigen::Matrix4d& initial_guess,
                         double max_correspondence_dist,
                         double* planar_ratio_out, double* alpha_out);

  double computeAdaptiveThreshold();

  GenZICPParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();

  std::vector<double> model_errors_;
  int frame_count_ = 0;
};

}  // namespace genz_icp
}  // namespace localization_zoo
