#include "dlo/dlo.h"

#include <Eigen/Geometry>

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace localization_zoo {
namespace dlo {

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

}  // namespace

DLO::DLO(const DLOParams& params)
    : params_(params), registration_(params.gicp) {}

std::vector<Eigen::Vector3d> DLO::preprocess(
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

std::vector<Eigen::Vector3d> DLO::transformPoints(
    const std::vector<Eigen::Vector3d>& points, const Eigen::Matrix4d& pose) {
  const Eigen::Matrix3d rotation = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d translation = pose.block<3, 1>(0, 3);

  std::vector<Eigen::Vector3d> transformed(points.size());
  for (size_t i = 0; i < points.size(); ++i) {
    transformed[i] = rotation * points[i] + translation;
  }
  return transformed;
}

double DLO::rotationAngle(const Eigen::Matrix4d& transform) {
  const Eigen::Quaterniond quat(transform.block<3, 3>(0, 0));
  const double clamped = std::clamp(std::abs(quat.normalized().w()), 0.0, 1.0);
  return 2.0 * std::acos(clamped);
}

bool DLO::shouldAddKeyframe(const Eigen::Matrix4d& candidate_pose) const {
  const Eigen::Matrix4d relative = last_keyframe_pose_.inverse() * candidate_pose;
  const double translation =
      relative.block<3, 1>(0, 3).norm();
  const double rotation = rotationAngle(relative);
  return translation >= params_.keyframe_translation_threshold ||
         rotation >= params_.keyframe_rotation_threshold_rad;
}

void DLO::rebuildMap() {
  map_points_.clear();
  size_t total_points = 0;
  for (const auto& keyframe : keyframes_) {
    total_points += keyframe.size();
  }

  map_points_.reserve(total_points);
  for (const auto& keyframe : keyframes_) {
    map_points_.insert(map_points_.end(), keyframe.begin(), keyframe.end());
  }

  registration_.setTarget(map_points_);
}

void DLO::addKeyframe(const std::vector<Eigen::Vector3d>& map_points,
                      const Eigen::Matrix4d& pose) {
  keyframes_.push_back(map_points);
  while (static_cast<int>(keyframes_.size()) > params_.max_keyframes_in_map) {
    keyframes_.pop_front();
  }
  last_keyframe_pose_ = pose;
  rebuildMap();
}

DLOResult DLO::process(const aloam::PointCloudConstPtr& cloud) {
  DLOResult result;
  result.pose = pose_;
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

  if (!initialized_) {
    addKeyframe(transformPoints(map_points, pose_), pose_);
    initialized_ = true;
    result.pose = pose_;
    result.initialized = true;
    result.num_keyframes = numKeyframes();
    result.map_points = mapSize();
    return result;
  }

  const Eigen::Matrix4d prediction = pose_ * last_delta_;
  const Eigen::Matrix4d previous_pose = pose_;
  const gicp::GICPResult alignment =
      registration_.align(registration_points, prediction);

  pose_ = alignment.transformation;
  last_delta_ = previous_pose.inverse() * pose_;

  result.pose = pose_;
  result.initialized = true;
  result.valid = alignment.num_correspondences > 0;
  result.converged = alignment.converged;
  result.num_correspondences = alignment.num_correspondences;
  result.num_iterations = alignment.num_iterations;
  result.fitness = alignment.fitness;

  if (shouldAddKeyframe(pose_)) {
    addKeyframe(transformPoints(map_points, pose_), pose_);
    result.keyframe_added = true;
  }

  result.num_keyframes = numKeyframes();
  result.map_points = mapSize();
  return result;
}

void DLO::clear() {
  registration_ = gicp::GICPRegistration(params_.gicp);
  keyframes_.clear();
  map_points_.clear();
  pose_.setIdentity();
  last_delta_.setIdentity();
  last_keyframe_pose_.setIdentity();
  initialized_ = false;
}

}  // namespace dlo
}  // namespace localization_zoo
