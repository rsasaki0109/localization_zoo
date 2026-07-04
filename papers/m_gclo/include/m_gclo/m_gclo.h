#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace m_gclo {

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

/// 平面法線・分布つきボクセルハッシュマップ。地面 point-to-plane と
/// 非地面 point-to-distribution(NDT) 双方の対応に使う局所統計を返す。
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d nearest = Eigen::Vector3d::Zero();
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    Eigen::Matrix3d cov = Eigen::Matrix3d::Identity();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();  // 最小固有ベクトル
    double planarity = 0.0;
    int neighbor_count = 0;
    bool found = false;
    bool has_pca = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);
  std::vector<Correspondence> getCorrespondences(
      const std::vector<Eigen::Vector3d>& points, double max_dist,
      int min_neighbors) const;
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

struct MGcloParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double initial_threshold = 2.0;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  // --- M-GCLO: 地面/非地面分離 + 複数地面平面拘束 + NDT + 不確かさ重み ---
  /// 対応平面の法線鉛直成分 |n_z| がこれ以上なら地面平面とみなす。
  double ground_normal_threshold = 0.85;
  /// 地面候補の高さ条件: 対応点平均 z がセンサ高 + offset より下なら地面。
  double ground_height_offset = -0.5;
  /// 地面 point-to-plane 拘束の相対重み (M-GCLO は鉛直精度向上のため地面を重視)。
  double ground_weight = 2.0;
  /// 非地面 point-to-distribution(NDT) 拘束の相対重み。
  double nonground_weight = 1.0;
  /// 地面分類に要求する平面性下限。
  double ground_planarity = 0.3;
  /// NDT 共分散の正則化 (最小固有値を λmax のこの比に下限化)。
  double distribution_regularization = 0.01;
  /// 各点の不確かさ重み: w_unc = ref²/(ref² + range²) (遠点ほど低信頼)。
  double uncertainty_range_ref = 40.0;
  /// Cauchy ロバスト核スケール (<=0 で無効)。
  double robust_scale = 1.0;
  /// 等速予測 (IMU 無し KITTI 代替) への prior 精度。
  double prior_precision = 0.0;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct MGcloResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  /// 地面平面拘束に使われた対応数。
  int num_ground = 0;
  /// 非地面 NDT 拘束に使われた対応数。
  int num_nonground = 0;
};

/// M-GCLO: Multiple Ground Constrained LiDAR Odometry
/// (ISPRS Annals X-1-2024, 283-290, Yang et al. 2024) の from-paper 再実装。
/// 著者コード未公開。
///
///  点群を地面 / 非地面に分類し、地面はボクセル化して複数の地面平面を抽出・
///  パラメータ化し point-to-plane 拘束で姿勢誤差 (特に鉛直成分) を抑える。非地面は
///  NDT ボクセルマップへの point-to-distribution マッチングを行う。各点の不確かさ
///  (距離依存ノイズ) を考慮した残差重み付けを併用する。3 要素:
///   1. 地面/非地面分類 (対応平面の法線鉛直性 + 高さ)
///   2. 複数地面平面 point-to-plane (鉛直精度向上)
///   3. 非地面 point-to-distribution (NDT Mahalanobis) + 距離依存不確かさ重み
///
/// 純 LiDAR scope。IMU 緩結合は範囲外 (KITTI は IMU 無し→等速予測)。
class MGcloPipeline {
public:
  explicit MGcloPipeline(const MGcloParams& params = MGcloParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  MGcloResult registerFrame(const std::vector<Eigen::Vector3d>& frame,
                            const Eigen::Matrix4d* init_guess = nullptr);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& base,
                                  MGcloResult* result);

  MGcloParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace m_gclo
}  // namespace localization_zoo
