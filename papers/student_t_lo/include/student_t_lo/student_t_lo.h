#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace student_t_lo {

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

struct StudentTLoParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double initial_threshold = 2.0;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  // --- Student's-t ロバスト重み ---
  /// Student's-t 重み付けを有効化する (false なら従来 Gaussian カーネルに退化)。
  bool enable_student_t = true;
  /// 自由度 ν。小さいほど重尾でアウトライアに頑健 (ν→∞ で Gaussian, ν=1 で Cauchy)。
  double student_t_dof = 5.0;
  /// 残差スケール σ の初期値 [m] (point-to-plane 残差の標準偏差)。
  double scale_init = 0.5;
  /// 反復ごとに σ を EM で更新する (σ² = Σ wᵢ rᵢ² / N)。
  bool estimate_scale = true;
  /// σ の下限 [m]。0 への崩壊を防ぐ。
  double scale_floor = 0.05;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct StudentTLoResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  /// 最終反復で使われた残差スケール σ [m] (adaptive)。
  double scale_used = 0.0;
  /// 最終反復の平均 Student's-t 重み (1=全 inlier 相当, <1=ロバスト減衰)。
  double mean_weight = 0.0;
};

/// Student-T-LO: heavy-tail ロバスト point-to-plane LiDAR オドメトリ。
///
///  "Research on Student's T-Distribution Point Cloud Registration Algorithm
///  Based on Local Features" (2024, PMC11314997) の機構を from-paper で再実装。
///  著者コード未公開。
///
///  LiDAR の残差は laser speckle・データ欠損・動的物体により重尾ノイズを含むため、
///  Gaussian 仮定 (NDT/GICP/標準 ICP) は外れ値で系統誤差を生む。本手法は残差を
///  Student's-t 分布でモデル化し、EM の E-step で各対応に重み
///    wᵢ = (ν + 1) / (ν + (rᵢ/σ)²)
///  を与える IRLS を解く。重尾のため大残差 (外れ値) を自動減衰しつつ、Gaussian
///  カーネルのような hard クリップ無しで滑らかに頑健化する。M-step でスケール σ を
///    σ² = Σ wᵢ rᵢ² / N
///  で更新し、フレームの残差分布に追従させる (adaptive scale)。
///
///  Cauchy カーネル (ν=1) の一般化にあたるが、可変自由度 ν と EM による adaptive
///  scale 推定が固定カーネルとの差別化点。
class StudentTLoPipeline {
public:
  explicit StudentTLoPipeline(const StudentTLoParams& params = StudentTLoParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  StudentTLoResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

  /// 単一残差に対する Student's-t 重み (テスト用に公開)。
  static double studentTWeight(double residual, double scale, double dof);

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& base,
                                  StudentTLoResult* result);

  StudentTLoParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace student_t_lo
}  // namespace localization_zoo
