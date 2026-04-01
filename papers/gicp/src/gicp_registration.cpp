#include "gicp/gicp_registration.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

#include <cmath>
#include <iostream>
#include <limits>

namespace localization_zoo {
namespace gicp {

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

void GICPRegistration::setTarget(
    const std::vector<Eigen::Vector3d>& target_points) {
  target_points_ = target_points;
  target_cloud_ = toPclCloud(target_points_);
  target_kdtree_.setInputCloud(target_cloud_);
  target_covariances_ = computeCovariances(target_points_);
}

std::vector<Eigen::Matrix3d> GICPRegistration::computeCovariances(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Matrix3d> covariances(
      points.size(), params_.covariance_regularization * Eigen::Matrix3d::Identity());
  if (points.size() < 4) return covariances;

  auto cloud = toPclCloud(points);
  pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;
  kdtree.setInputCloud(cloud);

  const int k =
      std::min(static_cast<int>(points.size()), params_.k_neighbors + 1);
  std::vector<int> neighbor_indices(k);
  std::vector<float> neighbor_distances(k);

  for (size_t i = 0; i < points.size(); i++) {
    const auto& p = points[i];
    pcl::PointXYZ query(static_cast<float>(p.x()), static_cast<float>(p.y()),
                        static_cast<float>(p.z()));
    const int found = kdtree.nearestKSearch(query, k, neighbor_indices,
                                            neighbor_distances);
    if (found < 4) continue;

    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    int used = 0;
    for (int j = 0; j < found; j++) {
      if (neighbor_indices[j] == static_cast<int>(i)) continue;
      mean += points[neighbor_indices[j]];
      used++;
    }
    if (used < 3) continue;
    mean /= static_cast<double>(used);

    Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
    for (int j = 0; j < found; j++) {
      if (neighbor_indices[j] == static_cast<int>(i)) continue;
      const Eigen::Vector3d diff = points[neighbor_indices[j]] - mean;
      covariance += diff * diff.transpose();
    }
    covariance /= static_cast<double>(used);

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(covariance);
    if (solver.info() != Eigen::Success) continue;

    const Eigen::Matrix3d U = solver.eigenvectors();
    Eigen::Vector3d lambda(1.0, 1.0, 1.0);
    lambda(0) = params_.covariance_regularization;
    covariances[i] = U * lambda.asDiagonal() * U.transpose();
  }

  return covariances;
}

std::vector<std::pair<int, int>> GICPRegistration::findCorrespondences(
    const std::vector<Eigen::Vector3d>& transformed_source) const {
  std::vector<std::pair<int, int>> correspondences;
  correspondences.reserve(transformed_source.size());
  if (target_points_.empty()) return correspondences;

  const double max_dist_sq =
      params_.max_correspondence_distance * params_.max_correspondence_distance;
  std::vector<int> index(1);
  std::vector<float> distance_sq(1);

  for (size_t i = 0; i < transformed_source.size(); i++) {
    const auto& p = transformed_source[i];
    pcl::PointXYZ query(static_cast<float>(p.x()), static_cast<float>(p.y()),
                        static_cast<float>(p.z()));
    if (target_kdtree_.nearestKSearch(query, 1, index, distance_sq) == 0) {
      continue;
    }
    if (distance_sq[0] > max_dist_sq) continue;
    correspondences.emplace_back(static_cast<int>(i), index[0]);
  }

  return correspondences;
}

GICPResult GICPRegistration::align(
    const std::vector<Eigen::Vector3d>& source_points,
    const Eigen::Matrix4d& initial_guess) {
  GICPResult result;
  result.transformation = initial_guess;

  if (target_points_.empty()) {
    std::cerr << "[GICP] Target not set." << std::endl;
    return result;
  }
  if (source_points.empty()) {
    std::cerr << "[GICP] Source is empty." << std::endl;
    return result;
  }

  const auto source_covariances = computeCovariances(source_points);
  Eigen::Matrix3d R = initial_guess.block<3, 3>(0, 0);
  Eigen::Vector3d t = initial_guess.block<3, 1>(0, 3);
  double last_fitness = std::numeric_limits<double>::infinity();

  for (int iter = 0; iter < params_.max_iterations; iter++) {
    const auto transformed_source = transformPoints(source_points, R, t);
    const auto correspondences = findCorrespondences(transformed_source);
    if (correspondences.size() < 10) {
      std::cerr << "[GICP] Too few correspondences at iteration " << iter
                << std::endl;
      break;
    }

    Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> b = Eigen::Matrix<double, 6, 1>::Zero();
    double total_error = 0.0;

    for (const auto& [source_index, target_index] : correspondences) {
      const Eigen::Vector3d& x = transformed_source[source_index];
      const Eigen::Vector3d& q = target_points_[target_index];

      const Eigen::Matrix3d metric =
          (target_covariances_[target_index] +
           R * source_covariances[source_index] * R.transpose() +
           params_.covariance_regularization * Eigen::Matrix3d::Identity())
              .inverse()
              .eval();

      const Eigen::Vector3d residual = x - q;
      const double mahalanobis = residual.dot(metric * residual);

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(x);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      H += J.transpose() * metric * J;
      b += J.transpose() * metric * residual;
      total_error += mahalanobis;
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

}  // namespace gicp
}  // namespace localization_zoo
