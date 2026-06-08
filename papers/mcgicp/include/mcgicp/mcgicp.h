#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace mcgicp {

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

struct PointI {
  Eigen::Vector3d p = Eigen::Vector3d::Zero();
  double intensity = 0.0;
};

using Vec4 = Eigen::Matrix<double, 4, 1>;
using Mat4 = Eigen::Matrix<double, 4, 4>;

/// Multi-Channel GICP (Servos & Waslander, RAS 2017) の 4D 共分散推定と
/// Mahalanobis 距離 (テスト用に公開)。
class McGicpCore {
 public:
  static Mat4 computeCovariance(const std::vector<PointI>& neighbors,
                                double intensity_scale,
                                double normal_epsilon,
                                double intensity_epsilon);

  static double mahalanobisSquared(const Vec4& a, const Vec4& b, const Mat4& cov_a,
                                   const Mat4& cov_b);

  static Vec4 toVec4(const PointI& pt, double intensity_scale);

  /// 4D 共分散の強度方向分散から point-to-plane 重みを得る。
  static double intensityWeight(double intensity_residual, double intensity_epsilon);
};

class VoxelHashMap {
 public:
  struct Correspondence {
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double mean_intensity = 0.0;
    double planarity = 0.0;
    Mat4 local_cov = Mat4::Identity();
    bool found = false;
    bool has_normal = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<PointI>& points);
  std::vector<Correspondence> getCorrespondences(
      const std::vector<PointI>& queries, double max_dist, int normal_min_neighbors,
      double intensity_scale, double normal_epsilon,
      double intensity_epsilon) const;
  void pruneFarVoxels(const Eigen::Vector3d& center, double max_distance);
  void clear();
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

struct McGicpLoParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double initial_threshold = 2.0;
  double max_correspondence_dist = 2.0;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  double intensity_scale = 2.0;
  double normal_epsilon = 1e-3;
  double intensity_epsilon = 0.05;
  bool enable_intensity = true;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct McGicpLoResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  double mean_intensity_residual = 0.0;
};

class McGicpLoPipeline {
 public:
  explicit McGicpLoPipeline(const McGicpLoParams& params = McGicpLoParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  McGicpLoResult registerFrame(const std::vector<PointI>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

 private:
  std::vector<PointI> voxelDownsample(const std::vector<PointI>& points,
                                      double voxel_size) const;
  std::vector<PointI> rangeFilter(const std::vector<PointI>& points) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(const std::vector<PointI>& source,
                                  const Eigen::Matrix4d& base,
                                  McGicpLoResult* result);

  McGicpLoParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace mcgicp
}  // namespace localization_zoo
