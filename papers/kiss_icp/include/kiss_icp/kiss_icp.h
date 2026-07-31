#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstdint>
#include <functional>
#include <limits>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace kiss_icp {

double modelDeviationError(const Eigen::Matrix4d& deviation,
                           double max_range);
void correctElevationAngle(std::vector<Eigen::Vector3d>& points,
                           double angle_rad);
std::vector<Eigen::Vector3d> deskewScanToEnd(
    const std::vector<Eigen::Vector3d>& points,
    const std::vector<double>& relative_times,
    const Eigen::Matrix4d& relative_motion);
bool motionWithinLimits(const Eigen::Matrix4d& relative_motion,
                        double max_translation_m,
                        double max_rotation_rad);
bool motionWithinAdaptiveLimits(
    const Eigen::Matrix4d& relative_motion,
    const Eigen::Matrix4d& previous_motion,
    double max_translation_m,
    double max_rotation_rad,
    double max_translation_multiplier,
    double translation_consistency_m,
    double rotation_consistency_rad);

/// ボクセル座標ハッシュ
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

/// ボクセルハッシュマップ (KISS-ICPのマップ表現)
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    bool found = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20,
                        bool update_full_voxels = false)
      : voxel_size_(voxel_size),
        max_points_(max_points_per_voxel),
        update_full_voxels_(update_full_voxels) {}

  /// 点群をマップに追加
  void addPoints(const std::vector<Eigen::Vector3d>& points);

  /// 最近傍探索 (各クエリ点に対して最近傍1点を返す)
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
  bool update_full_voxels_;

  struct VoxelBlock {
    std::vector<Eigen::Vector3d> points;
    std::uint64_t observations = 0;
  };
  std::unordered_map<Eigen::Vector3i, VoxelBlock, VoxelHash> map_;
};

struct KISSICPParams {
  double voxel_size = 0.5;          // マップボクセルサイズ [m]
  double max_range = 100.0;         // 最大距離 [m]
  double min_range = 3.0;           // 最小距離 [m]
  int max_points_per_voxel = 20;
  double initial_threshold = 2.0;   // 初期対応距離閾値 [m]
  int max_icp_iterations = 500;
  double convergence_criterion = 0.001;
  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
  bool update_full_voxels = false;
  bool use_model_deviation_threshold = false;
  double model_deviation_correspondence_multiplier = 3.0;
  bool enable_motion_guard = false;
  double max_step_translation_m = 2.0;
  double max_step_rotation_rad = 0.3490658503988659;
  bool enable_adaptive_motion_guard = false;
  std::size_t adaptive_motion_guard_min_trusted_steps = 3;
  double adaptive_motion_guard_max_translation_multiplier = 2.0;
  double adaptive_motion_guard_translation_consistency_m = 0.75;
  double adaptive_motion_guard_rotation_consistency_rad = 0.08726646259971647;
};

struct KISSICPResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  bool motion_guard_rejected = false;
  bool motion_guard_adaptive_accepted = false;
};

struct KISSMatcherParams {
  double target_voxel_size = 0.75;
  double source_voxel_size = 0.75;
  double min_range = 2.0;
  double max_range = 70.0;
  int max_points_per_voxel = 20;
  double max_correspondence_distance = 1.5;
  int max_icp_iterations = 30;
  int min_correspondences = 80;
  double convergence_criterion = 0.001;
};

struct KISSMatcherResult {
  Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  double rmse = std::numeric_limits<double>::infinity();
  double information_min_eigenvalue = 0.0;
  double information_max_eigenvalue = 0.0;
  double information_condition = std::numeric_limits<double>::infinity();
};

/// Pair-wise KISS-style matcher. `align()` estimates source-to-target transform.
class KISSMatcher {
public:
  explicit KISSMatcher(const KISSMatcherParams& params = KISSMatcherParams());

  void setTarget(const std::vector<Eigen::Vector3d>& target);
  KISSMatcherResult align(const std::vector<Eigen::Vector3d>& source,
                          const Eigen::Matrix4d& initial_guess =
                              Eigen::Matrix4d::Identity()) const;

private:
  std::vector<Eigen::Vector3d> preprocess(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;

  KISSMatcherParams params_;
  VoxelHashMap target_map_;
};

/// KISS-ICP パイプライン
class KISSICPPipeline {
public:
  explicit KISSICPPipeline(const KISSICPParams& params = KISSICPParams());

  /// 新しいスキャンを処理
  KISSICPResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  const Eigen::Matrix4d& lastDelta() const { return last_delta_; }
  size_t mapSize() const { return local_map_.size(); }
  double adaptiveThreshold() const;
  std::size_t motionGuardRejections() const {
    return motion_guard_rejections_;
  }
  std::size_t adaptiveMotionGuardAcceptances() const {
    return adaptive_motion_guard_acceptances_;
  }

private:
  /// ボクセルサブサンプリング
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;

  /// 距離フィルタ
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;

  /// Point-to-Point ICP (ロバストカーネル付き)
  Eigen::Matrix4d runICP(const std::vector<Eigen::Vector3d>& source,
                         const std::vector<Eigen::Vector3d>& target,
                         const Eigen::Matrix4d& initial_guess,
                         double max_correspondence_dist,
                         double kernel_threshold);

  /// 適応的閾値の計算
  double computeAdaptiveThreshold();

  KISSICPParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();

  // 適応的閾値のための履歴
  std::vector<double> model_errors_;
  double model_error_squared_sum_ = 0.0;
  std::size_t model_error_samples_ = 0;
  int frame_count_ = 0;
  std::size_t motion_guard_rejections_ = 0;
  std::size_t adaptive_motion_guard_acceptances_ = 0;
  std::size_t trusted_motion_steps_ = 0;
};

}  // namespace kiss_icp
}  // namespace localization_zoo
