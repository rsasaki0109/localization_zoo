#include "voxel_gicp/voxel_gicp_registration.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

#include <cmath>
#include <iostream>
#include <limits>

namespace localization_zoo {
namespace voxel_gicp {

namespace {

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0.0, -v.z(), v.y(), v.z(), 0.0, -v.x(), -v.y(), v.x(), 0.0;
  return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& omega) {
  const double theta = omega.norm();
  if (theta < 1e-10) {
    return Eigen::Matrix3d::Identity() + skew(omega);
  }

  const Eigen::Matrix3d K = skew(omega / theta);
  return Eigen::Matrix3d::Identity() + std::sin(theta) * K +
         (1.0 - std::cos(theta)) * K * K;
}

std::vector<Eigen::Vector3d> transformPoints(
    const std::vector<Eigen::Vector3d>& points, const Eigen::Matrix3d& R,
    const Eigen::Vector3d& t) {
  std::vector<Eigen::Vector3d> transformed(points.size());
  for (size_t i = 0; i < points.size(); i++) transformed[i] = R * points[i] + t;
  return transformed;
}

pcl::PointCloud<pcl::PointXYZ>::Ptr toPclCloud(
    const std::vector<Eigen::Vector3d>& points) {
  auto cloud = pcl::PointCloud<pcl::PointXYZ>::Ptr(
      new pcl::PointCloud<pcl::PointXYZ>);
  cloud->reserve(points.size());
  for (const auto& p : points) {
    cloud->push_back(
        pcl::PointXYZ(static_cast<float>(p.x()), static_cast<float>(p.y()),
                      static_cast<float>(p.z())));
  }
  return cloud;
}

}  // namespace

VoxelGICPMap::VoxelGICPMap(double resolution, int min_points_per_voxel,
                           double covariance_regularization)
    : resolution_(resolution),
      min_points_per_voxel_(min_points_per_voxel),
      covariance_regularization_(covariance_regularization),
      cloud_(new pcl::PointCloud<pcl::PointXYZ>) {}

Eigen::Vector3i VoxelGICPMap::toVoxel(const Eigen::Vector3d& point) const {
  return Eigen::Vector3i(static_cast<int>(std::floor(point.x() / resolution_)),
                         static_cast<int>(std::floor(point.y() / resolution_)),
                         static_cast<int>(std::floor(point.z() / resolution_)));
}

void VoxelGICPMap::setInputCloud(const std::vector<Eigen::Vector3d>& points) {
  centroids_.clear();
  covariances_.clear();
  cloud_->clear();

  std::unordered_map<Eigen::Vector3i, std::vector<Eigen::Vector3d>, VoxelHash>
      voxels;
  for (const auto& point : points) {
    voxels[toVoxel(point)].push_back(point);
  }

  centroids_.reserve(voxels.size());
  covariances_.reserve(voxels.size());

  for (const auto& [voxel, voxel_points] : voxels) {
    (void)voxel;
    if (static_cast<int>(voxel_points.size()) < min_points_per_voxel_) continue;

    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
    for (const auto& p : voxel_points) centroid += p;
    centroid /= static_cast<double>(voxel_points.size());

    Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
    for (const auto& p : voxel_points) {
      const Eigen::Vector3d diff = p - centroid;
      covariance += diff * diff.transpose();
    }
    covariance /= static_cast<double>(voxel_points.size());

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(covariance);
    if (solver.info() != Eigen::Success) continue;

    Eigen::Vector3d lambda = solver.eigenvalues();
    lambda(0) = covariance_regularization_;
    lambda(1) = std::max(lambda(1), covariance_regularization_ * 10.0);
    lambda(2) = std::max(lambda(2), covariance_regularization_ * 10.0);

    centroids_.push_back(centroid);
    covariances_.push_back(
        solver.eigenvectors() * lambda.asDiagonal() *
        solver.eigenvectors().transpose());
  }

  cloud_ = toPclCloud(centroids_);
  if (!centroids_.empty()) kdtree_.setInputCloud(cloud_);
}

bool VoxelGICPMap::findNearestNeighbor(const Eigen::Vector3d& point,
                                       double max_distance, int* index,
                                       double* distance_sq) const {
  if (centroids_.empty()) return false;

  std::vector<int> indices(1);
  std::vector<float> distances(1);
  pcl::PointXYZ query(static_cast<float>(point.x()), static_cast<float>(point.y()),
                      static_cast<float>(point.z()));
  if (kdtree_.nearestKSearch(query, 1, indices, distances) == 0) return false;

  const double max_distance_sq = max_distance * max_distance;
  if (distances[0] > max_distance_sq) return false;

  *index = indices[0];
  *distance_sq = distances[0];
  return true;
}

void VoxelGICPRegistration::setTarget(
    const std::vector<Eigen::Vector3d>& target_points) {
  target_map_.setInputCloud(target_points);
}

std::vector<std::pair<int, int>> VoxelGICPRegistration::findCorrespondences(
    const std::vector<Eigen::Vector3d>& transformed_source,
    const VoxelGICPMap& source_map) const {
  std::vector<std::pair<int, int>> correspondences;
  correspondences.reserve(source_map.size());

  for (size_t i = 0; i < transformed_source.size(); i++) {
    int target_index = -1;
    double distance_sq = 0.0;
    if (!target_map_.findNearestNeighbor(
            transformed_source[i], params_.max_correspondence_distance,
            &target_index, &distance_sq)) {
      continue;
    }
    correspondences.emplace_back(static_cast<int>(i), target_index);
  }

  return correspondences;
}

VoxelGICPResult VoxelGICPRegistration::align(
    const std::vector<Eigen::Vector3d>& source_points,
    const Eigen::Matrix4d& initial_guess) {
  VoxelGICPResult result;
  result.transformation = initial_guess;

  if (target_map_.size() == 0) {
    std::cerr << "[Voxel-GICP] Target not set." << std::endl;
    return result;
  }
  if (source_points.empty()) {
    std::cerr << "[Voxel-GICP] Source is empty." << std::endl;
    return result;
  }

  VoxelGICPMap source_map(params_.voxel_resolution, params_.min_points_per_voxel,
                          params_.covariance_regularization);
  source_map.setInputCloud(source_points);
  if (source_map.size() == 0) {
    std::cerr << "[Voxel-GICP] Source voxel map is empty." << std::endl;
    return result;
  }

  Eigen::Matrix3d R = initial_guess.block<3, 3>(0, 0);
  Eigen::Vector3d t = initial_guess.block<3, 1>(0, 3);
  double last_fitness = std::numeric_limits<double>::infinity();

  for (int iter = 0; iter < params_.max_iterations; iter++) {
    const auto transformed_source =
        transformPoints(source_map.centroids(), R, t);
    const auto correspondences =
        findCorrespondences(transformed_source, source_map);
    if (correspondences.size() < 5) {
      std::cerr << "[Voxel-GICP] Too few correspondences at iteration " << iter
                << std::endl;
      break;
    }

    Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> b = Eigen::Matrix<double, 6, 1>::Zero();
    double total_error = 0.0;

    for (const auto& [source_index, target_index] : correspondences) {
      const Eigen::Vector3d& x = transformed_source[source_index];
      const Eigen::Vector3d& q = target_map_.centroids()[target_index];

      const Eigen::Matrix3d metric =
          (target_map_.covariances()[target_index] +
           R * source_map.covariances()[source_index] * R.transpose() +
           params_.covariance_regularization * Eigen::Matrix3d::Identity())
              .inverse()
              .eval();

      const Eigen::Vector3d residual = x - q;
      total_error += residual.dot(metric * residual);

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(x);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      H += J.transpose() * metric * J;
      b += J.transpose() * metric * residual;
    }

    H += 1e-6 * Eigen::Matrix<double, 6, 6>::Identity();
    const Eigen::Matrix<double, 6, 1> delta = -H.ldlt().solve(b);
    if (!delta.allFinite()) break;

    const Eigen::Matrix3d dR = expSO3(delta.head<3>());
    R = dR * R;
    t = dR * t + delta.tail<3>();

    result.num_iterations = iter + 1;
    result.num_correspondences = static_cast<int>(correspondences.size());
    result.fitness = total_error / static_cast<double>(correspondences.size());

    const double rotation_delta = delta.head<3>().norm();
    const double translation_delta = delta.tail<3>().norm();
    const double fitness_delta = std::abs(last_fitness - result.fitness);
    if (rotation_delta < params_.rotation_epsilon &&
        translation_delta < params_.translation_epsilon) {
      result.converged = true;
      break;
    }
    if (fitness_delta < params_.fitness_epsilon) {
      result.converged = true;
      break;
    }

    last_fitness = result.fitness;
  }

  result.transformation.setIdentity();
  result.transformation.block<3, 3>(0, 0) = R;
  result.transformation.block<3, 1>(0, 3) = t;
  return result;
}

}  // namespace voxel_gicp
}  // namespace localization_zoo
