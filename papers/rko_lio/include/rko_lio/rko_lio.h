#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <functional>
#include <vector>
#include <unordered_map>

#include "imu_preintegration/imu_preintegration.h"

namespace localization_zoo {
namespace rko_lio {

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

/// scan-to-map 用ボクセルハッシュマップ (最近傍 1 点を返す)。
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    bool found = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);
  std::vector<Correspondence> getCorrespondences(
      const std::vector<Eigen::Vector3d>& points, double max_dist) const;
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

struct RKOLIOParams {
  double voxel_size = 1.0;          ///< マップボクセルサイズ [m]
  double max_range = 100.0;         ///< 最大距離 [m]
  double min_range = 1.0;           ///< 最小距離 [m]
  int max_points_per_voxel = 20;
  double initial_threshold = 2.0;   ///< 初期対応距離閾値 [m]
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
  /// gyro バイアスのオンライン補正ゲイン (0 で補正なし)。
  /// IMU 先験回転と ICP 解の差分から gyro バイアスを推定し、生 gyro の
  /// バイアスが誤った回転先験を生むのを抑える。
  double gyro_bias_gain = 0.3;
  double max_gyro_bias = 0.5;  ///< 推定 gyro バイアスのノルム上限 [rad/s]
};

struct RKOLIOResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  bool used_imu = false;  ///< このフレームで IMU 回転先験を使ったか
};

/// RKO-LIO パイプライン。
/// KISS 系 voxel-hash scan-to-map robust ICP に IMU 回転先験を組み合わせる。
/// IMU (gyro) を事前積分して得た相対回転を ICP 初期推定の回転として使い、
/// 並進は等速モデルで予測する。IMU が無いフレームでは等速モデルに退化する。
class RKOLIOPipeline {
public:
  explicit RKOLIOPipeline(const RKOLIOParams& params = RKOLIOParams());

  /// 初期ポーズ (GT シード等) を設定する。最初のフレーム前に呼ぶこと。
  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }

  /// 1 スキャンを処理する。
  /// @param frame センサ座標の点群
  /// @param imu  直前フレームから現フレームまでの IMU サンプル (空可)
  RKOLIOResult registerFrame(
      const std::vector<Eigen::Vector3d>& frame,
      const std::vector<localization_zoo::imu_preintegration::ImuSample>& imu);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d runICP(const std::vector<Eigen::Vector3d>& source,
                         const Eigen::Matrix4d& initial_guess,
                         double max_correspondence_dist);
  double computeAdaptiveThreshold();

  RKOLIOParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  Eigen::Vector3d gyro_bias_ = Eigen::Vector3d::Zero();  ///< 推定 gyro バイアス [rad/s]

  std::vector<double> model_errors_;
  int frame_count_ = 0;
};

}  // namespace rko_lio
}  // namespace localization_zoo
