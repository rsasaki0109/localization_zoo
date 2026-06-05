#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

#include "imu_preintegration/imu_preintegration.h"

namespace localization_zoo {
namespace degen_sense {

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

struct DegenSenseParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double initial_threshold = 2.0;

  // --- 適応的退化検出 (DBSCAN 近似: ロバスト外れ値検出) ---
  int warmup_frames = 10;        ///< これだけ貯めてから外れ値判定 (論文の DBSCAN 起動条件に対応)
  int buffer_size = 100;         ///< 退化ファクタ履歴長
  double mad_k = 3.0;            ///< median + k·MAD を超えたら退化と判定
  double min_fusion_weight = 0.05;  ///< 1/S のクランプ下限 (LiDAR 重みの最小)

  double gyro_bias_gain = 0.3;
  double max_gyro_bias = 0.5;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct DegenSenseResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  bool used_imu = false;
  double degeneracy_factor_rot = 1.0;    ///< S_R = λ_max/λ_min (回転)
  double degeneracy_factor_trans = 1.0;  ///< S_t = λ_max/λ_min (並進)
  bool degenerate = false;               ///< このフレームが退化と判定されたか
};

/// A Real-time Degeneracy Sensing and Compensation Method (arXiv:2412.07513)。
///
///  (1) 退化ファクタ S = λ_max/λ_min (Hessian の回転/並進サブブロックの条件数)。
///  (2) 退化ファクタ時系列を適応的外れ値検出 (論文の DBSCAN を median+MAD で近似)
///      で判定し、手動しきい値なしで退化スパイクを検出。
///  (3) 退化時は LiDAR 解と IMU 予測を 1/S で重み付け融合して補償する。
class DegenSensePipeline {
public:
  explicit DegenSensePipeline(const DegenSenseParams& params = DegenSenseParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }

  DegenSenseResult registerFrame(
      const std::vector<Eigen::Vector3d>& frame,
      const std::vector<localization_zoo::imu_preintegration::ImuSample>& imu);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  /// point-to-plane ICP。収束時の Hessian を H_out に返す。
  Eigen::Matrix4d runICP(const std::vector<Eigen::Vector3d>& source,
                         const Eigen::Matrix4d& initial_guess,
                         Eigen::Matrix<double, 6, 6>* H_out);
  /// 退化ファクタ履歴から適応的に外れ値 (退化) かを判定する。
  bool isDegenerate(double factor, const std::deque<double>& history) const;

  DegenSenseParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  Eigen::Vector3d gyro_bias_ = Eigen::Vector3d::Zero();
  std::deque<double> rot_history_;
  std::deque<double> trans_history_;
  int frame_count_ = 0;
};

}  // namespace degen_sense
}  // namespace localization_zoo
