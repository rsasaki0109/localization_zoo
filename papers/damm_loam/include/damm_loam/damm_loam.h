#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace damm_loam {

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

/// 点の幾何クラス (range-image 法線コンセンサスに基づく多メトリック分類)。
enum class PointClass {
  kUnknown = 0,
  kGround,   ///< 平面 + 法線がほぼ鉛直 + センサより低い
  kRoof,     ///< 平面 + 法線がほぼ鉛直 + センサより高い
  kWall,     ///< 平面 + 法線がほぼ水平
  kEdge,     ///< 低平面度 (高曲率)、線特徴
  kNonPlanar ///< どれにも当てはまらない
};

/// 平面法線つきボクセルハッシュマップ。
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
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

struct DammLoamParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;  ///< これ以上で平面候補 (ground/wall/roof)
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double initial_threshold = 2.0;

  // --- range-image 法線コンセンサス分類 ---
  double vertical_cos = 0.966;  ///< |n_z| >= これで鉛直法線 (~15deg)
  double horizontal_cos = 0.34; ///< |n_z| <= これで水平法線 (wall)
  double ground_z = 0.0;        ///< センサ系で z < ground_z を地面候補に

  // --- 退化対応多メトリック重み ---
  double edge_weight = 0.5;        ///< 非退化時の edge (点対点) 基本重み α0
  double degeneracy_ratio = 0.05;  ///< λ_k < ratio·λ_max で退化方向と判定
  double degeneracy_boost = 4.0;   ///< 退化方向に整列する点の重み増幅上限

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct ClassCounts {
  int ground = 0;
  int roof = 0;
  int wall = 0;
  int edge = 0;
  int non_planar = 0;
  int unknown = 0;
};

struct DammLoamResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  double degeneracy_factor_trans = 1.0;  ///< S_t = λ_max/λ_min (並進ブロック)
  bool degenerate = false;               ///< 並進に退化方向ありと判定
  ClassCounts counts;                    ///< 直近フレームの点クラス内訳
};

/// DAMM-LOAM: Degeneracy-Aware Multi-Metric LiDAR Odometry (arXiv:2510.13287)。
///
///  (1) 各点を局所法線コンセンサスで ground/roof/wall/edge/non-planar に分類
///      (range-image の法線方向支配を近似)。
///  (2) クラス別の多メトリック残差: 平面クラスは point-to-plane、edge は
///      point-to-point。基本重み α でバランス。
///  (3) 並進 Hessian ブロック H_tt を固有分解し、相対基準で退化方向を検出。
///      退化方向に整列する点と edge 点の重みを適応的に増幅して補償する。
class DammLoamPipeline {
public:
  explicit DammLoamPipeline(const DammLoamParams& params = DammLoamParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }

  DammLoamResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  /// スキャン (センサ系) を局所法線で分類する。テスト/可視化用に公開。
  std::vector<PointClass> classifyScan(
      const std::vector<Eigen::Vector3d>& points) const;

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  /// 退化対応多メトリック point-to-plane / point-to-point ICP。
  Eigen::Matrix4d runICP(const std::vector<Eigen::Vector3d>& source,
                         const Eigen::Matrix4d& initial_guess,
                         Eigen::Matrix<double, 6, 6>* H_out,
                         double* S_trans_out, bool* degenerate_out);

  DammLoamParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  int frame_count_ = 0;
};

}  // namespace damm_loam
}  // namespace localization_zoo
