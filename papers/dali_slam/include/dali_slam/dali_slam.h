#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace dali_slam {

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

struct DaliSlamParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double initial_threshold = 2.0;

  // --- DS-MDC (dual-spline motion distortion correction) ---
  /// 連続時間スプライン deskew を有効化。
  bool enable_deskew = true;
  /// スプラインを 2 次 (加速度考慮) で外挿。off なら等速 (1 次)。実 KITTI では
  /// 2 次外挿はノイズの多い delta を増幅し deskew が発散源になるため既定 off
  /// (等速 deskew = KISS-ICP/CT-ICP と同等の安定動作)。
  bool spline_quadratic = false;

  // --- degeneracy-aware update (solution remapping) ---
  /// 退化判定の固有値しきい値 (絶対値)。これ未満の方向は更新から除去。
  /// 論文 (DALI-SLAM) デフォルト degeneracy=4.48 相当。
  double degeneracy_threshold = 4.48;
  bool enable_degeneracy = true;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct DaliSlamResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  bool deskew_applied = false;
  bool degenerate = false;        ///< 退化方向を検出・除去したフレームか
  int num_degenerate_dirs = 0;    ///< 除去した退化方向数
  double min_eigenvalue = 0.0;    ///< Hessian 最小固有値 (退化指標)
};

/// DALI-SLAM (Wang et al., ISPRS J. P&RS 221, 2025) の from-paper 再実装。
/// 著者コードは公開されているが (FAST-LIO2 + basalt spline ベース)、本実装は
/// 既存 module 流の自前 point-to-plane パイプラインへ DALI の 2 つの核を移植する。
///
///  (1) DS-MDC: 連続時間スプラインで scan の motion distortion を補正する。原実装は
///      LiDAR レジストレーション姿勢列の lf_spline と IMU 積分姿勢列の lf_imu_spline
///      の dual SE3 cubic B-spline を用い、T_correct = T_lf·T_lf_imu⁻¹·T_imu で
///      離散 IMU 姿勢を補正してから deskew する。
///  (2) degeneracy-aware ESKF update: 有効点 Jacobian の 6x6 係数行列を固有分解し、
///      しきい値未満の特異方向を解からゼロremap (solution remapping)。
///
/// 純 LiDAR scope (KITTI、IMU 無)。lf_imu_spline は等速外挿軌跡で代替する。等速では
/// LiDAR 軌跡と運動モデル軌跡が一致に近づき DS-MDC 補正量は小さくなる (論文の数値は
/// MEMS IMU 前提で、KITTI では退化対応 update が主寄与、README 参照)。点ごと時刻は
/// KITTI 回転 LiDAR の方位角から復元する。
class DaliSlamPipeline {
public:
  explicit DaliSlamPipeline(const DaliSlamParams& params = DaliSlamParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  DaliSlamResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  /// スプライン外挿でフレーム内相対運動 (twist) を推定。
  Eigen::Matrix<double, 6, 1> scanMotionTwist() const;
  /// 方位角ベースの per-point 時刻で deskew (scan 末端フレームへ)。
  std::vector<Eigen::Vector3d> deskew(const std::vector<Eigen::Vector3d>& points,
                                      const Eigen::Matrix<double, 6, 1>& twist) const;
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& initial_guess,
                                  DaliSlamResult* result);
  Eigen::Matrix4d predict() const;

  DaliSlamParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;  ///< lf_spline 用の相対運動列
  int frame_count_ = 0;
};

}  // namespace dali_slam
}  // namespace localization_zoo
