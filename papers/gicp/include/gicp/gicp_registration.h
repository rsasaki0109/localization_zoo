#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <vector>

namespace localization_zoo {
namespace gicp {

struct GICPParams {
  int k_neighbors = 20;                    // 共分散推定の近傍点数
  double max_correspondence_distance = 3.0;
  int max_iterations = 30;
  double rotation_epsilon = 1e-4;
  double translation_epsilon = 1e-4;
  double covariance_regularization = 1e-3;
  double fitness_epsilon = 1e-6;
};

struct GICPResult {
  Eigen::Matrix4d transformation = Eigen::Matrix4d::Identity();
  double fitness = 0.0;
  int num_iterations = 0;
  int num_correspondences = 0;
  bool converged = false;
};

class GICPRegistration {
public:
  explicit GICPRegistration(const GICPParams& params = GICPParams())
      : params_(params),
        target_cloud_(new pcl::PointCloud<pcl::PointXYZ>) {}

  void setTarget(const std::vector<Eigen::Vector3d>& target_points);

  GICPResult align(const std::vector<Eigen::Vector3d>& source_points,
                   const Eigen::Matrix4d& initial_guess =
                       Eigen::Matrix4d::Identity());

  size_t targetSize() const { return target_points_.size(); }

private:
  std::vector<Eigen::Matrix3d> computeCovariances(
      const std::vector<Eigen::Vector3d>& points) const;

  std::vector<std::pair<int, int>> findCorrespondences(
      const std::vector<Eigen::Vector3d>& transformed_source) const;

  GICPParams params_;
  std::vector<Eigen::Vector3d> target_points_;
  std::vector<Eigen::Matrix3d> target_covariances_;
  pcl::PointCloud<pcl::PointXYZ>::Ptr target_cloud_;
  pcl::KdTreeFLANN<pcl::PointXYZ> target_kdtree_;
};

}  // namespace gicp
}  // namespace localization_zoo
