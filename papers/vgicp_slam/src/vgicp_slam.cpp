#include "vgicp_slam/vgicp_slam.h"

#include <ceres/ceres.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <unordered_map>
#include <common/ceres_compat.h>

namespace localization_zoo {
namespace vgicp_slam {

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

struct PoseGraphEdgeCost {
  PoseGraphEdgeCost(const Eigen::Quaterniond& q_ij, const Eigen::Vector3d& t_ij,
                    double rotation_weight, double translation_weight)
      : q_ij_(q_ij),
        t_ij_(t_ij),
        rotation_weight_(rotation_weight),
        translation_weight_(translation_weight) {}

  template <typename T>
  bool operator()(const T* const q_i_ptr, const T* const t_i_ptr,
                  const T* const q_j_ptr, const T* const t_j_ptr,
                  T* residuals_ptr) const {
    Eigen::Map<const Eigen::Quaternion<T>> q_i(q_i_ptr);
    Eigen::Map<const Eigen::Matrix<T, 3, 1>> t_i(t_i_ptr);
    Eigen::Map<const Eigen::Quaternion<T>> q_j(q_j_ptr);
    Eigen::Map<const Eigen::Matrix<T, 3, 1>> t_j(t_j_ptr);

    const Eigen::Quaternion<T> q_meas(
        T(q_ij_.w()), T(q_ij_.x()), T(q_ij_.y()), T(q_ij_.z()));
    const Eigen::Matrix<T, 3, 1> t_meas = t_ij_.template cast<T>();

    const Eigen::Quaternion<T> q_est = q_i.conjugate() * q_j;
    const Eigen::Quaternion<T> q_err = q_meas.conjugate() * q_est;
    const Eigen::Matrix<T, 3, 1> t_err = q_i.conjugate() * (t_j - t_i) - t_meas;

    Eigen::Map<Eigen::Matrix<T, 6, 1>> residuals(residuals_ptr);
    residuals.template head<3>() =
        T(rotation_weight_) * T(2.0) * q_err.vec();
    residuals.template tail<3>() = T(translation_weight_) * t_err;
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Quaterniond& q_ij,
                                     const Eigen::Vector3d& t_ij,
                                     double rotation_weight,
                                     double translation_weight) {
    return new ceres::AutoDiffCostFunction<PoseGraphEdgeCost, 6, 4, 3, 4, 3>(
        new PoseGraphEdgeCost(q_ij, t_ij, rotation_weight, translation_weight));
  }

  Eigen::Quaterniond q_ij_;
  Eigen::Vector3d t_ij_;
  double rotation_weight_;
  double translation_weight_;
};

}  // namespace

VgicpSlamParams::VgicpSlamParams() {
  front_end.voxel_resolution = 0.8;
  front_end.min_points_per_voxel = 1;
  front_end.max_correspondence_distance = 4.0;
  front_end.max_iterations = 20;
  front_end.rotation_epsilon = 1e-3;
  front_end.translation_epsilon = 1e-3;
  front_end.fitness_epsilon = 1e-4;
  front_end.covariance_regularization = 1e-3;

  loop_vgicp = front_end;
  loop_vgicp.voxel_resolution = 1.0;
  loop_vgicp.max_iterations = 25;
  loop_vgicp.max_correspondence_distance = 5.0;

  scan_context.exclude_recent_frames = 5;
  scan_context.num_candidates = 5;
  scan_context.distance_threshold = 0.18;
  scan_context.search_ratio = 0.2;
}

VgicpSlam::VgicpSlam(const VgicpSlamParams& params)
    : params_(params),
      registration_(params.front_end),
      scan_context_(params.scan_context) {}

std::vector<Eigen::Vector3d> VgicpSlam::preprocess(
    const aloam::PointCloudConstPtr& cloud, double voxel_size) const {
  std::vector<Eigen::Vector3d> filtered;
  if (cloud == nullptr || voxel_size <= 0.0) {
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

std::vector<Eigen::Vector3d> VgicpSlam::transformPoints(
    const std::vector<Eigen::Vector3d>& points, const Eigen::Matrix4d& pose) {
  const Eigen::Matrix3d rotation = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d translation = pose.block<3, 1>(0, 3);

  std::vector<Eigen::Vector3d> transformed(points.size());
  for (size_t i = 0; i < points.size(); ++i) {
    transformed[i] = rotation * points[i] + translation;
  }
  return transformed;
}

double VgicpSlam::rotationAngle(const Eigen::Matrix4d& transform) {
  const Eigen::Quaterniond quat(transform.block<3, 3>(0, 0));
  const double clamped = std::clamp(std::abs(quat.normalized().w()), 0.0, 1.0);
  return 2.0 * std::acos(clamped);
}

bool VgicpSlam::shouldCreateKeyframe(const Eigen::Matrix4d& candidate_pose) const {
  if (keyframes_.empty()) {
    return true;
  }

  const Eigen::Matrix4d relative = last_keyframe_pose_.inverse() * candidate_pose;
  const double translation = relative.block<3, 1>(0, 3).norm();
  const double rotation = rotationAngle(relative);

  if (params_.keyframe_stride > 0 &&
      valid_frame_count_ % params_.keyframe_stride == 0) {
    return true;
  }
  if (translation >= params_.keyframe_translation_threshold) {
    return true;
  }
  if (rotation >= params_.keyframe_rotation_threshold_rad) {
    return true;
  }
  return false;
}

void VgicpSlam::rebuildSubmap() {
  local_map_points_.clear();
  size_t total_points = 0;
  for (int index : submap_indices_) {
    total_points += keyframes_[static_cast<size_t>(index)].world_points.size();
  }

  local_map_points_.reserve(total_points);
  for (int index : submap_indices_) {
    const auto& world_points = keyframes_[static_cast<size_t>(index)].world_points;
    local_map_points_.insert(local_map_points_.end(), world_points.begin(),
                             world_points.end());
  }

  registration_.setTarget(local_map_points_);
}

void VgicpSlam::addKeyframe(
    const Eigen::Matrix4d& pose, const std::vector<Eigen::Vector3d>& map_points,
    const std::vector<Eigen::Vector3d>& descriptor_points,
    const std::vector<Eigen::Vector3d>& loop_points) {
  Keyframe keyframe;
  keyframe.raw_pose = matrixToPose(pose);
  keyframe.optimized_pose = keyframe.raw_pose;
  keyframe.world_points = transformPoints(map_points, pose);
  keyframe.descriptor_points = descriptor_points;
  keyframe.loop_points = loop_points;

  keyframes_.push_back(std::move(keyframe));
  submap_indices_.push_back(static_cast<int>(keyframes_.size()) - 1);
  while (static_cast<int>(submap_indices_.size()) > params_.max_keyframes_in_submap) {
    submap_indices_.pop_front();
  }
  last_keyframe_pose_ = pose;
  rebuildSubmap();
}

VgicpSlamResult VgicpSlam::process(const aloam::PointCloudConstPtr& cloud) {
  VgicpSlamResult result;
  result.pose = pose_;
  result.q_w_curr = Eigen::Quaterniond(pose_.block<3, 3>(0, 0)).normalized();
  result.t_w_curr = pose_.block<3, 1>(0, 3);
  result.initialized = initialized_;
  result.num_keyframes = numKeyframes();
  result.num_loop_edges = numLoopEdges();
  result.submap_points = submapSize();

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

  valid_frame_count_++;
  if (!initialized_) {
    pose_.setIdentity();
    last_delta_.setIdentity();
    last_keyframe_pose_.setIdentity();

    std::vector<Eigen::Vector3d> descriptor_points =
        stridePoints(registration_points, params_.descriptor_stride);
    std::vector<Eigen::Vector3d> loop_points =
        stridePoints(map_points, params_.loop_stride);
    if (descriptor_points.empty()) {
      descriptor_points = registration_points;
    }
    if (loop_points.size() < 20) {
      loop_points = descriptor_points;
    }

    addKeyframe(pose_, map_points, descriptor_points, loop_points);
    scan_context_.addScan(descriptor_points);
    initialized_ = true;

    result.pose = pose_;
    result.q_w_curr = Eigen::Quaterniond::Identity();
    result.t_w_curr = Eigen::Vector3d::Zero();
    result.initialized = true;
    result.valid = true;
    result.converged = true;
    result.keyframe_added = true;
    result.num_keyframes = numKeyframes();
    result.num_loop_edges = numLoopEdges();
    result.submap_points = submapSize();
    return result;
  }

  const Eigen::Matrix4d prediction = pose_ * last_delta_;
  const Eigen::Matrix4d previous_pose = pose_;
  const voxel_gicp::VoxelGICPResult alignment =
      registration_.align(registration_points, prediction);
  result.front_end_result = alignment;
  pose_ = alignment.transformation;
  if (alignment.num_iterations > 0) {
    last_delta_ = previous_pose.inverse() * pose_;
  }

  result.pose = pose_;
  result.q_w_curr = Eigen::Quaterniond(pose_.block<3, 3>(0, 0)).normalized();
  result.t_w_curr = pose_.block<3, 1>(0, 3);
  result.initialized = true;
  result.valid =
      alignment.num_iterations > 0 && alignment.num_correspondences > 0;
  result.converged = alignment.converged;
  if (!result.valid) {
    return result;
  }

  if (!shouldCreateKeyframe(pose_)) {
    return result;
  }

  std::vector<Eigen::Vector3d> descriptor_points =
      stridePoints(registration_points, params_.descriptor_stride);
  std::vector<Eigen::Vector3d> loop_points =
      stridePoints(map_points, params_.loop_stride);
  if (descriptor_points.empty()) {
    descriptor_points = registration_points;
  }
  if (loop_points.size() < 20) {
    loop_points = descriptor_points;
  }

  int loop_index = -1;
  if (params_.enable_loop_closure && !descriptor_points.empty() &&
      scan_context_.numScans() > 0) {
    const scan_context::LoopCandidate candidate =
        scan_context_.detectLoop(descriptor_points);
    if (candidate.valid &&
        static_cast<int>(keyframes_.size()) - candidate.index >=
            params_.min_loop_index_gap) {
      loop_index = candidate.index;
    }
  }

  const int current_index = static_cast<int>(keyframes_.size());
  addKeyframe(pose_, map_points, descriptor_points, loop_points);
  result.keyframe_added = true;

  if (current_index > 0) {
    odom_edges_.push_back(
        {current_index - 1, current_index,
         relativePose(keyframes_[current_index - 1].raw_pose,
                      keyframes_[current_index].raw_pose)});
  }

  scan_context_.addScan(keyframes_[static_cast<size_t>(current_index)].descriptor_points);

  if (loop_index >= 0 && tryAddLoopEdge(loop_index, current_index)) {
    result.loop_detected = true;
    result.loop_index = loop_index;
  }

  if ((params_.optimize_every_n_keyframes > 0 &&
       static_cast<int>(keyframes_.size()) % params_.optimize_every_n_keyframes ==
           0) ||
      result.loop_detected) {
    optimizePoseGraph();
  }

  result.q_w_curr = keyframes_.back().optimized_pose.q;
  result.t_w_curr = keyframes_.back().optimized_pose.t;
  result.num_keyframes = numKeyframes();
  result.num_loop_edges = numLoopEdges();
  result.submap_points = submapSize();
  return result;
}

bool VgicpSlam::tryAddLoopEdge(int loop_index, int current_index) {
  if (loop_index < 0 || current_index <= loop_index ||
      current_index >= static_cast<int>(keyframes_.size())) {
    return false;
  }
  if (current_index - loop_index < params_.min_loop_index_gap) {
    return false;
  }

  for (const auto& edge : loop_edges_) {
    if (edge.from == loop_index && edge.to == current_index) {
      return false;
    }
  }

  const auto& target = keyframes_[static_cast<size_t>(loop_index)].loop_points;
  const auto& source = keyframes_[static_cast<size_t>(current_index)].loop_points;
  if (target.size() < 20 || source.size() < 20) {
    return false;
  }

  voxel_gicp::VoxelGICPRegistration registration(params_.loop_vgicp);
  registration.setTarget(target);

  const Eigen::Matrix4d initial_guess = poseToMatrix(
      relativePose(keyframes_[static_cast<size_t>(loop_index)].optimized_pose,
                   keyframes_[static_cast<size_t>(current_index)].optimized_pose));
  const voxel_gicp::VoxelGICPResult vgicp_result =
      registration.align(source, initial_guess);

  if (!vgicp_result.converged ||
      vgicp_result.fitness > params_.loop_fitness_threshold ||
      vgicp_result.num_correspondences < params_.min_loop_correspondences) {
    return false;
  }

  LoopEdge edge;
  edge.from = loop_index;
  edge.to = current_index;
  edge.relative_pose = matrixToPose(vgicp_result.transformation);
  edge.fitness = vgicp_result.fitness;
  edge.correspondences = vgicp_result.num_correspondences;
  loop_edges_.push_back(edge);
  return true;
}

void VgicpSlam::optimizePoseGraph() {
  if (keyframes_.size() < 2) {
    return;
  }

  std::vector<std::array<double, 4>> q_params(keyframes_.size());
  std::vector<std::array<double, 3>> t_params(keyframes_.size());
  for (size_t i = 0; i < keyframes_.size(); ++i) {
    const PoseState& pose = keyframes_[i].optimized_pose;
    q_params[i] = {pose.q.x(), pose.q.y(), pose.q.z(), pose.q.w()};
    t_params[i] = {pose.t.x(), pose.t.y(), pose.t.z()};
  }

  ceres::Problem problem;
  for (size_t i = 0; i < keyframes_.size(); ++i) {
    problem.AddParameterBlock(q_params[i].data(), 4);
    localization_zoo::SetEigenQuaternionManifold(problem, q_params[i].data());
    problem.AddParameterBlock(t_params[i].data(), 3);
  }
  problem.SetParameterBlockConstant(q_params.front().data());
  problem.SetParameterBlockConstant(t_params.front().data());

  for (const auto& edge : odom_edges_) {
    problem.AddResidualBlock(
        PoseGraphEdgeCost::Create(edge.relative_pose.q, edge.relative_pose.t,
                                  params_.odom_rotation_weight,
                                  params_.odom_translation_weight),
        nullptr, q_params[static_cast<size_t>(edge.from)].data(),
        t_params[static_cast<size_t>(edge.from)].data(),
        q_params[static_cast<size_t>(edge.to)].data(),
        t_params[static_cast<size_t>(edge.to)].data());
  }

  ceres::LossFunction* loop_loss = new ceres::HuberLoss(1.0);
  for (const auto& edge : loop_edges_) {
    problem.AddResidualBlock(
        PoseGraphEdgeCost::Create(edge.relative_pose.q, edge.relative_pose.t,
                                  params_.loop_rotation_weight,
                                  params_.loop_translation_weight),
        loop_loss, q_params[static_cast<size_t>(edge.from)].data(),
        t_params[static_cast<size_t>(edge.from)].data(),
        q_params[static_cast<size_t>(edge.to)].data(),
        t_params[static_cast<size_t>(edge.to)].data());
  }

  ceres::Solver::Options options;
  options.linear_solver_type = ceres::DENSE_QR;
  options.max_num_iterations = 20;
  options.minimizer_progress_to_stdout = false;

  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);

  for (size_t i = 0; i < keyframes_.size(); ++i) {
    Eigen::Map<Eigen::Quaterniond> q(q_params[i].data());
    Eigen::Map<Eigen::Vector3d> t(t_params[i].data());
    keyframes_[i].optimized_pose.q = q.normalized();
    keyframes_[i].optimized_pose.t = t;
  }
}

VgicpSlam::PoseState VgicpSlam::relativePose(const PoseState& from,
                                             const PoseState& to) {
  PoseState relative;
  relative.q = (from.q.conjugate() * to.q).normalized();
  relative.t = from.q.conjugate() * (to.t - from.t);
  return relative;
}

VgicpSlam::PoseState VgicpSlam::matrixToPose(
    const Eigen::Matrix4d& transform) {
  PoseState pose;
  pose.q = Eigen::Quaterniond(transform.block<3, 3>(0, 0)).normalized();
  pose.t = transform.block<3, 1>(0, 3);
  return pose;
}

Eigen::Matrix4d VgicpSlam::poseToMatrix(const PoseState& pose) {
  Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();
  transform.block<3, 3>(0, 0) = pose.q.normalized().toRotationMatrix();
  transform.block<3, 1>(0, 3) = pose.t;
  return transform;
}

std::vector<Eigen::Vector3d> VgicpSlam::stridePoints(
    const std::vector<Eigen::Vector3d>& points, size_t stride) {
  if (points.empty()) {
    return {};
  }

  const size_t effective_stride = std::max<size_t>(1, stride);
  std::vector<Eigen::Vector3d> sampled;
  sampled.reserve(points.size() / effective_stride + 1);
  for (size_t i = 0; i < points.size(); i += effective_stride) {
    sampled.push_back(points[i]);
  }
  return sampled;
}

Eigen::Matrix4d VgicpSlam::latestOptimizedPoseMatrix() const {
  if (keyframes_.empty()) {
    return Eigen::Matrix4d::Identity();
  }
  return poseToMatrix(keyframes_.back().optimized_pose);
}

void VgicpSlam::clear() {
  registration_ = voxel_gicp::VoxelGICPRegistration(params_.front_end);
  scan_context_.clear();
  keyframes_.clear();
  submap_indices_.clear();
  local_map_points_.clear();
  odom_edges_.clear();
  loop_edges_.clear();
  pose_.setIdentity();
  last_delta_.setIdentity();
  last_keyframe_pose_.setIdentity();
  initialized_ = false;
  valid_frame_count_ = 0;
}

}  // namespace vgicp_slam
}  // namespace localization_zoo
