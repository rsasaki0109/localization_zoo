#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <vector>

namespace localization_zoo {
namespace small_gicp {

struct SmallGICPParams {
  double voxel_resolution = 1.0;
  int k_neighbors = 12;
  double max_correspondence_distance = 4.0;
  int max_correspondences = 256;
  int max_iterations = 20;
  double rotation_epsilon = 1e-4;
  double translation_epsilon = 1e-4;
  double covariance_regularization = 1e-3;
  double fitness_epsilon = 1e-6;
};

struct SmallGICPResult {
  Eigen::Matrix4d transformation = Eigen::Matrix4d::Identity();
  double fitness = 0.0;
  int num_iterations = 0;
  int num_correspondences = 0;
  bool converged = false;
};

class SmallGICPRegistration {
public:
  explicit SmallGICPRegistration(const SmallGICPParams& params = SmallGICPParams())
      : params_(params),
        target_cloud_(new pcl::PointCloud<pcl::PointXYZ>) {}

  void setTarget(const std::vector<Eigen::Vector3d>& target_points);

  SmallGICPResult align(
      const std::vector<Eigen::Vector3d>& source_points,
      const Eigen::Matrix4d& initial_guess = Eigen::Matrix4d::Identity());

  size_t targetSize() const { return target_points_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points) const;
  std::vector<Eigen::Matrix3d> computeCovariances(
      const std::vector<Eigen::Vector3d>& points) const;
  std::vector<std::pair<int, int>> findCorrespondences(
      const std::vector<Eigen::Vector3d>& transformed_source) const;

  SmallGICPParams params_;
  std::vector<Eigen::Vector3d> target_points_;
  std::vector<Eigen::Matrix3d> target_covariances_;
  pcl::PointCloud<pcl::PointXYZ>::Ptr target_cloud_;
  pcl::KdTreeFLANN<pcl::PointXYZ> target_kdtree_;
};

}  // namespace small_gicp
}  // namespace localization_zoo
