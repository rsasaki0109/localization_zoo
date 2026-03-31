#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace voxel_gicp {

struct VoxelHash {
  std::size_t operator()(const Eigen::Vector3i& v) const {
    std::size_t seed = 0;
    auto hc = [&seed](int val) {
      seed ^= std::hash<int>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    hc(v.x());
    hc(v.y());
    hc(v.z());
    return seed;
  }
};

struct VoxelGICPParams {
  double voxel_resolution = 1.0;
  int min_points_per_voxel = 5;
  double max_correspondence_distance = 4.0;
  int max_iterations = 30;
  double rotation_epsilon = 1e-4;
  double translation_epsilon = 1e-4;
  double covariance_regularization = 1e-3;
  double fitness_epsilon = 1e-6;
};

struct VoxelGICPResult {
  Eigen::Matrix4d transformation = Eigen::Matrix4d::Identity();
  double fitness = 0.0;
  int num_iterations = 0;
  int num_correspondences = 0;
  bool converged = false;
};

class VoxelGICPMap {
public:
  VoxelGICPMap(double resolution, int min_points_per_voxel,
               double covariance_regularization);

  void setInputCloud(const std::vector<Eigen::Vector3d>& points);

  size_t size() const { return centroids_.size(); }
  const std::vector<Eigen::Vector3d>& centroids() const { return centroids_; }
  const std::vector<Eigen::Matrix3d>& covariances() const { return covariances_; }

  bool findNearestNeighbor(const Eigen::Vector3d& point, double max_distance,
                           int* index, double* distance_sq) const;

private:
  Eigen::Vector3i toVoxel(const Eigen::Vector3d& point) const;

  double resolution_;
  int min_points_per_voxel_;
  double covariance_regularization_;
  std::vector<Eigen::Vector3d> centroids_;
  std::vector<Eigen::Matrix3d> covariances_;
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_;
  pcl::KdTreeFLANN<pcl::PointXYZ> kdtree_;
};

class VoxelGICPRegistration {
public:
  explicit VoxelGICPRegistration(
      const VoxelGICPParams& params = VoxelGICPParams())
      : params_(params),
        target_map_(params.voxel_resolution, params.min_points_per_voxel,
                    params.covariance_regularization) {}

  void setTarget(const std::vector<Eigen::Vector3d>& target_points);

  VoxelGICPResult align(
      const std::vector<Eigen::Vector3d>& source_points,
      const Eigen::Matrix4d& initial_guess = Eigen::Matrix4d::Identity());

  size_t targetSize() const { return target_map_.size(); }

private:
  std::vector<std::pair<int, int>> findCorrespondences(
      const std::vector<Eigen::Vector3d>& transformed_source,
      const VoxelGICPMap& source_map) const;

  VoxelGICPParams params_;
  VoxelGICPMap target_map_;
};

}  // namespace voxel_gicp
}  // namespace localization_zoo
