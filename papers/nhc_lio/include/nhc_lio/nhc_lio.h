#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace nhc_lio {

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

/// 平面法線つきボクセルハッシュマップ (point-to-plane 用)。
class VoxelHashMap {
public:
  struct Correspondence {
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

struct NhcLioParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double initial_threshold = 2.0;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  // --- 非ホロノミック拘束 (NHC) ファクタ ---
  /// NHC を有効化する。
  bool enable_nhc = true;
  /// NHC 基本情報重み (body 系 横/上下 並進速度 ≈ 0 への soft 拘束)。
  double nhc_weight = 5.0;
  /// 旋回時の adaptive 減衰スケール (frame あたり yaw 角 rad)。論文の CNN/IMU 予測
  /// による reliable NHC を、純 LiDAR では運動状態 (yaw rate) ベースで近似する:
  /// w = nhc_weight / (1 + (yaw_rate/yaw_ref)²) で急旋回時に減衰。
  double nhc_yaw_ref = 0.03;
  /// 車両の前進軸 (KITTI Velodyne は x)。拘束する横/上下軸はこの軸以外。
  int forward_axis = 0;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct NhcLioResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  /// このフレームで実際に使われた NHC 重み (adaptive)。
  double nhc_weight_used = 0.0;
};

/// NHC-LIO: A Novel Vehicle Lidar-Inertial Odometry with Reliable Nonholonomic
/// Constraint (NHC) Factor (Chen et al., IEEE Sensors Journal 2023,
/// doi:10.1109/JSEN.2023.3320150) の from-paper 再実装。著者コード未公開。
///
///  車両は body 系で横滑り (lateral) と上下動 (vertical) の並進速度がほぼ 0 という
///  非ホロノミック拘束 (NHC) を持つ。これを scan-to-map point-to-plane オドメトリの
///  最適化に soft factor として加え、横方向 drift を抑える。論文は 9 軸 IMU から
///  CNN で reliable な NHC 擬似観測を予測し情報行列を adaptive にするが、KITTI は
///  IMU 無しのため、運動状態 (yaw rate) ベースの adaptive 重みで近似する (急旋回時に
///  NHC を減衰: 旋回中は横滑りが増えるため)。
///
/// 純 LiDAR scope。IMU 緩結合・CNN 予測は範囲外 (KITTI は IMU 無し→等速予測)。
class NhcLioPipeline {
public:
  explicit NhcLioPipeline(const NhcLioParams& params = NhcLioParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  NhcLioResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& base, double nhc_w,
                                  NhcLioResult* result);

  NhcLioParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace nhc_lio
}  // namespace localization_zoo
