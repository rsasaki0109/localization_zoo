#include "lins/lins.h"

#include <Eigen/Eigenvalues>
#include <pcl/kdtree/kdtree_flann.h>

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace localization_zoo {
namespace lins {

namespace {

struct VoxelHash {
  std::size_t operator()(const Eigen::Vector3i& key) const {
    std::size_t seed = 0;
    auto mix = [&seed](int value) {
      seed ^= std::hash<int>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    mix(key.x());
    mix(key.y());
    mix(key.z());
    return seed;
  }
};

aloam::PointT toPoint(const Eigen::Vector3d& point) {
  aloam::PointT pcl_point;
  pcl_point.x = static_cast<float>(point.x());
  pcl_point.y = static_cast<float>(point.y());
  pcl_point.z = static_cast<float>(point.z());
  pcl_point.intensity = 0.0f;
  return pcl_point;
}

}  // namespace

LINS::LINS(const LINSParams& params)
    : params_(params), filter_(params.filter), map_cloud_(new aloam::PointCloud) {}

std::vector<Eigen::Vector3d> LINS::preprocess(
    const aloam::PointCloudConstPtr& cloud, double voxel_size) const {
  std::vector<Eigen::Vector3d> filtered;
  if (cloud == nullptr) {
    return filtered;
  }

  filtered.reserve(cloud->size());
  for (const auto& point : cloud->points) {
    if (!std::isfinite(point.x) || !std::isfinite(point.y) ||
        !std::isfinite(point.z)) {
      continue;
    }

    const Eigen::Vector3d eigen_point(point.x, point.y, point.z);
    const double range = eigen_point.norm();
    if (range < params_.min_range || range > params_.max_range) {
      continue;
    }
    filtered.push_back(eigen_point);
  }

  std::unordered_map<Eigen::Vector3i, Eigen::Vector3d, VoxelHash> voxels;
  voxels.reserve(filtered.size());
  for (const auto& point : filtered) {
    const Eigen::Vector3i key(
        static_cast<int>(std::floor(point.x() / voxel_size)),
        static_cast<int>(std::floor(point.y() / voxel_size)),
        static_cast<int>(std::floor(point.z() / voxel_size)));
    voxels.emplace(key, point);
  }

  std::vector<Eigen::Vector3d> downsampled;
  downsampled.reserve(voxels.size());
  for (const auto& [key, point] : voxels) {
    (void)key;
    downsampled.push_back(point);
  }
  return downsampled;
}

void LINS::propagateImu(
    const std::vector<imu_preintegration::ImuSample>& imu_samples) {
  if (imu_samples.size() < 2) {
    return;
  }

  for (size_t i = 1; i < imu_samples.size(); ++i) {
    const double dt = imu_samples[i].timestamp - imu_samples[i - 1].timestamp;
    if (dt <= 0.0 || dt > 0.1) {
      continue;
    }

    relead::ImuMeasurement imu;
    imu.timestamp = imu_samples[i].timestamp;
    imu.angular_velocity = 0.5 * (imu_samples[i - 1].gyro + imu_samples[i].gyro);
    imu.linear_acceleration =
        0.5 * (imu_samples[i - 1].accel + imu_samples[i].accel);
    filter_.predict(imu, dt);
  }
}

std::vector<Eigen::Vector3d> LINS::transformPoints(
    const std::vector<Eigen::Vector3d>& points, const relead::State& state) {
  std::vector<Eigen::Vector3d> transformed(points.size());
  for (size_t i = 0; i < points.size(); ++i) {
    transformed[i] = state.rotation * points[i] + state.position;
  }
  return transformed;
}

double LINS::rotationAngle(const Eigen::Matrix3d& rotation) {
  const Eigen::Quaterniond quat(rotation);
  const double clamped = std::clamp(std::abs(quat.normalized().w()), 0.0, 1.0);
  return 2.0 * std::acos(clamped);
}

bool LINS::shouldAddKeyframe(const relead::State& candidate_state) const {
  const double translation =
      (candidate_state.position - last_keyframe_state_.position).norm();
  const double rotation =
      rotationAngle(last_keyframe_state_.rotation.transpose() *
                    candidate_state.rotation);
  return translation >= params_.keyframe_translation_threshold ||
         rotation >= params_.keyframe_rotation_threshold_rad;
}

void LINS::rebuildMap() {
  map_points_.clear();
  size_t total_points = 0;
  for (const auto& keyframe : keyframes_) {
    total_points += keyframe.size();
  }

  map_points_.reserve(total_points);
  for (const auto& keyframe : keyframes_) {
    map_points_.insert(map_points_.end(), keyframe.begin(), keyframe.end());
  }

  map_cloud_->clear();
  map_cloud_->reserve(map_points_.size());
  for (const auto& point : map_points_) {
    map_cloud_->push_back(toPoint(point));
  }
}

void LINS::addKeyframe(const std::vector<Eigen::Vector3d>& points_world,
                       const relead::State& state) {
  keyframes_.push_back(points_world);
  while (static_cast<int>(keyframes_.size()) > params_.max_keyframes_in_map) {
    keyframes_.pop_front();
  }
  last_keyframe_state_ = state;
  rebuildMap();
}

std::vector<relead::PointWithNormal> LINS::buildCorrespondences(
    const std::vector<Eigen::Vector3d>& points_world) const {
  std::vector<relead::PointWithNormal> correspondences;
  if (map_cloud_->size() < static_cast<size_t>(params_.knn)) {
    return correspondences;
  }

  pcl::KdTreeFLANN<aloam::PointT> kdtree;
  kdtree.setInputCloud(map_cloud_);

  const double max_distance_sq =
      params_.max_correspondence_distance * params_.max_correspondence_distance;

  std::vector<int> point_search_indices;
  std::vector<float> point_search_distances;

  correspondences.reserve(points_world.size());
  const size_t stride = std::max<size_t>(
      1, points_world.size() / static_cast<size_t>(std::max(1, params_.max_correspondences)));
  for (size_t point_idx = 0; point_idx < points_world.size(); point_idx += stride) {
    const auto& point_world = points_world[point_idx];
    const aloam::PointT query = toPoint(point_world);
    point_search_indices.clear();
    point_search_distances.clear();

    kdtree.nearestKSearch(query, params_.knn, point_search_indices,
                          point_search_distances);
    if (point_search_distances.size() < static_cast<size_t>(params_.knn) ||
        point_search_distances.back() > max_distance_sq) {
      continue;
    }

    Eigen::Vector3d center = Eigen::Vector3d::Zero();
    for (int idx : point_search_indices) {
      center += map_points_[static_cast<size_t>(idx)];
    }
    center /= static_cast<double>(point_search_indices.size());

    Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
    for (int idx : point_search_indices) {
      const Eigen::Vector3d diff = map_points_[static_cast<size_t>(idx)] - center;
      covariance += diff * diff.transpose();
    }
    covariance /= static_cast<double>(point_search_indices.size());

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(covariance);
    if (solver.info() != Eigen::Success) {
      continue;
    }

    const Eigen::Vector3d normal = solver.eigenvectors().col(0);
    bool plane_valid = true;
    for (int idx : point_search_indices) {
      const double residual =
          std::abs((map_points_[static_cast<size_t>(idx)] - center).dot(normal));
      if (residual > params_.plane_threshold) {
        plane_valid = false;
        break;
      }
    }
    if (!plane_valid) {
      continue;
    }

    relead::PointWithNormal corr;
    corr.point = point_world;
    corr.normal = normal.normalized();
    corr.closest = center;
    correspondences.push_back(corr);
    if (static_cast<int>(correspondences.size()) >= params_.max_correspondences) {
      break;
    }
  }

  return correspondences;
}

LINSResult LINS::process(
    const aloam::PointCloudConstPtr& cloud,
    const std::vector<imu_preintegration::ImuSample>& imu_samples) {
  LINSResult result;
  result.state = filter_.state();
  result.initialized = initialized_;
  result.num_keyframes = numKeyframes();
  result.map_points = mapSize();

  const std::vector<Eigen::Vector3d> registration_points =
      preprocess(cloud, params_.registration_voxel_size);
  if (registration_points.empty()) {
    return result;
  }

  const std::vector<Eigen::Vector3d> map_points =
      preprocess(cloud, params_.map_voxel_size);
  if (map_points.empty()) {
    return result;
  }

  propagateImu(imu_samples);

  if (!initialized_) {
    addKeyframe(transformPoints(map_points, filter_.state()), filter_.state());
    initialized_ = true;
    result.state = filter_.state();
    result.initialized = true;
    result.num_keyframes = numKeyframes();
    result.map_points = mapSize();
    return result;
  }

  const std::vector<Eigen::Vector3d> points_world =
      transformPoints(registration_points, filter_.state());
  const std::vector<relead::PointWithNormal> correspondences =
      buildCorrespondences(points_world);

  result.num_correspondences = static_cast<int>(correspondences.size());
  if (correspondences.empty()) {
    result.state = filter_.state();
    result.initialized = true;
    return result;
  }

  const relead::CESIKFResult update_result = filter_.update(correspondences);
  result.state = update_result.state;
  result.degeneracy_info = update_result.degeneracy_info;
  result.initialized = true;
  result.valid = true;
  result.converged = update_result.converged;
  result.num_iterations = update_result.iterations;

  if (shouldAddKeyframe(update_result.state)) {
    addKeyframe(transformPoints(map_points, update_result.state), update_result.state);
    result.keyframe_added = true;
  }

  result.num_keyframes = numKeyframes();
  result.map_points = mapSize();
  return result;
}

void LINS::clear() {
  filter_ = relead::CESIKF(params_.filter);
  keyframes_.clear();
  map_points_.clear();
  map_cloud_->clear();
  last_keyframe_state_ = relead::State();
  initialized_ = false;
}

}  // namespace lins
}  // namespace localization_zoo
