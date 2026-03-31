#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace kiss_icp {

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
  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  /// 点群をマップに追加
  void addPoints(const std::vector<Eigen::Vector3d>& points);

  /// 最近傍探索 (各クエリ点に対して最近傍1点を返す)
  std::vector<Eigen::Vector3d> getCorrespondences(
      const std::vector<Eigen::Vector3d>& points, double max_dist) const;

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

struct KISSICPParams {
  double voxel_size = 0.5;          // マップボクセルサイズ [m]
  double max_range = 100.0;         // 最大距離 [m]
  double min_range = 3.0;           // 最小距離 [m]
  int max_points_per_voxel = 20;
  double initial_threshold = 2.0;   // 初期対応距離閾値 [m]
  int max_icp_iterations = 500;
  double convergence_criterion = 0.001;
};

struct KISSICPResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
};

/// KISS-ICP パイプライン
class KISSICPPipeline {
public:
  explicit KISSICPPipeline(const KISSICPParams& params = KISSICPParams());

  /// 新しいスキャンを処理
  KISSICPResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

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
                         double max_correspondence_dist);

  /// 適応的閾値の計算
  double computeAdaptiveThreshold();

  KISSICPParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();

  // 適応的閾値のための履歴
  std::vector<double> model_errors_;
  int frame_count_ = 0;
};

}  // namespace kiss_icp
}  // namespace localization_zoo
