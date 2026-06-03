#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace intensity_flow {

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

/// 強度つき点。
struct PointI {
  Eigen::Vector3d p = Eigen::Vector3d::Zero();
  double intensity = 0.0;
};

/// 法線 + 平均強度つきボクセルハッシュマップ。
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double planarity = 0.0;
    double mean_intensity = 0.0;
    bool found = false;
    bool has_normal = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<PointI>& points);
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
    std::vector<double> intensity;
  };
  std::unordered_map<Eigen::Vector3i, VoxelBlock, VoxelHash> map_;
};

/// RMS 幾何 gradient-flow サンプリング (Petráček et al., RA-L 2024)。
/// 各点 p で近傍重心への変位 ∆p = mean(neighbor - p) を計算 (∆p が大 → 面の縁/
/// コーナー = 局所化に寄与する点)。∆̄p を K ビンの histogram にし、ビンを巡回しつつ
/// 各ビンから高 ∆p 点を取り出すことで冗長を抑えつつ空間分布 (エントロピー) を保つ。
/// 選択点インデックスを返す。
std::vector<int> gradientFlowSample(const std::vector<PointI>& points,
                                    double radius, int num_bins,
                                    double keep_ratio);

struct IntensityFlowParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double initial_threshold = 2.0;

  // --- RMS gradient-flow サンプリング ---
  bool enable_gradient_flow = true;
  double gf_radius = 1.0;     ///< 近傍半径 λp (簡略: 固定値)。
  int gf_num_bins = 10;       ///< histogram ビン数 K (RMS 推奨 10)。
  double gf_keep_ratio = 0.5; ///< 採用率 (エントロピー率しきい値の簡略代替)。

  // --- intensity-geometry fused matching ---
  bool enable_intensity = true;
  double intensity_sigma = 0.2;  ///< 強度整合性の σ_I (w=exp(-ΔI²/2σ²))。

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct IntensityFlowResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_input = 0;     ///< RMS 前の点数
  int num_sampled = 0;   ///< RMS で採用した点数
};

/// Intensity-enhanced LiDAR(-inertial) odometry with gradient-flow sampling
/// (Measurement, 2026, PII S0263224126007219) の from-paper 再実装。著者コード
/// 未公開、出版社 wall のため数式は構成要素論文 (RMS gradient-flow + COIN-LIO/
/// IGE-LIO intensity 整合) から再構成。
///
///  (1) RMS 幾何 gradient-flow サンプリングで局所化に寄与する点を冗長最小に選ぶ。
///  (2) intensity-geometry fused matching: point-to-plane 対応を強度整合性
///      w_int = exp(-ΔI²/2σ_I²) で重み付け (強度の食い違う対応を抑制)。
///
/// 純 LiDAR scope (KITTI、IMU 無)。予測は等速。KITTI .bin は PointXYZI を持つ。
class IntensityFlowPipeline {
public:
  explicit IntensityFlowPipeline(
      const IntensityFlowParams& params = IntensityFlowParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  IntensityFlowResult registerFrame(const std::vector<PointI>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<PointI> voxelDownsample(const std::vector<PointI>& points,
                                      double voxel_size) const;
  std::vector<PointI> rangeFilter(const std::vector<PointI>& points) const;
  Eigen::Matrix4d runRegistration(const std::vector<PointI>& source,
                                  const Eigen::Matrix4d& initial_guess,
                                  IntensityFlowResult* result);
  Eigen::Matrix4d predict() const;

  IntensityFlowParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace intensity_flow
}  // namespace localization_zoo
