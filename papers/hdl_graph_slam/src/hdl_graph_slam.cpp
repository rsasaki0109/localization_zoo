#include "hdl_graph_slam/hdl_graph_slam.h"

#include <ceres/ceres.h>

#include <algorithm>
#include <array>
#include <unordered_map>
#include <common/ceres_compat.h>

namespace localization_zoo {
namespace hdl_graph_slam {

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

struct FloorHeightCost {
  FloorHeightCost(double floor_height, double weight)
      : floor_height_(floor_height), weight_(weight) {}

  template <typename T>
  bool operator()(const T* const t_ptr, T* residual_ptr) const {
    residual_ptr[0] = T(weight_) * (t_ptr[2] - T(floor_height_));
    return true;
  }

  static ceres::CostFunction* Create(double floor_height, double weight) {
    return new ceres::AutoDiffCostFunction<FloorHeightCost, 1, 3>(
        new FloorHeightCost(floor_height, weight));
  }

  double floor_height_;
  double weight_;
};

}  // namespace

HdlGraphSlamParams::HdlGraphSlamParams() {
  scan_context.exclude_recent_frames = 5;
  scan_context.num_candidates = 5;
  scan_context.distance_threshold = 0.18;
  scan_context.search_ratio = 0.2;
  loop_gicp.max_correspondence_distance = 5.0;
  loop_gicp.max_iterations = 20;
  loop_gicp.k_neighbors = 15;
  ndt.resolution = 1.0;
  ndt.max_iterations = 20;
  ndt.step_size = 0.2;
  ndt.convergence_threshold = 1e-3;
}

HdlGraphSlam::HdlGraphSlam(const HdlGraphSlamParams& params)
    : params_(params),
      registration_(params.ndt),
      scan_context_(params.scan_context) {}

std::vector<Eigen::Vector3d> HdlGraphSlam::preprocess(
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

std::vector<Eigen::Vector3d> HdlGraphSlam::transformPoints(
    const std::vector<Eigen::Vector3d>& points, const Eigen::Matrix4d& pose) {
  const Eigen::Matrix3d rotation = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d translation = pose.block<3, 1>(0, 3);

  std::vector<Eigen::Vector3d> transformed(points.size());
  for (size_t i = 0; i < points.size(); ++i) {
    transformed[i] = rotation * points[i] + translation;
  }
  return transformed;
}

double HdlGraphSlam::rotationAngle(const Eigen::Matrix4d& transform) {
  const Eigen::Quaterniond quat(transform.block<3, 3>(0, 0));
  const double clamped = std::clamp(std::abs(quat.normalized().w()), 0.0, 1.0);
  return 2.0 * std::acos(clamped);
}

bool HdlGraphSlam::shouldCreateKeyframe(const Eigen::Matrix4d& candidate_pose) const {
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

void HdlGraphSlam::rebuildSubmap() {
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

void HdlGraphSlam::addKeyframe(
    const Eigen::Matrix4d& pose, const std::vector<Eigen::Vector3d>& map_points,
    const std::vector<Eigen::Vector3d>& descriptor_points,
    const std::vector<Eigen::Vector3d>& loop_points) {
  Keyframe keyframe;
  keyframe.raw_pose = matrixToPose(pose);
  keyframe.optimized_pose = keyframe.raw_pose;
  keyframe.world_points = transformPoints(map_points, pose);
  keyframe.descriptor_points = descriptor_points;
  keyframe.loop_points = loop_points;
  keyframe.frame_index =
      static_cast<size_t>(std::max(0, valid_frame_count_ - 1));

  keyframes_.push_back(std::move(keyframe));
  submap_indices_.push_back(static_cast<int>(keyframes_.size()) - 1);
  while (static_cast<int>(submap_indices_.size()) > params_.max_keyframes_in_submap) {
    submap_indices_.pop_front();
  }
  last_keyframe_pose_ = pose;
  rebuildSubmap();
}

HdlGraphSlamResult HdlGraphSlam::process(const aloam::PointCloudConstPtr& cloud) {
  HdlGraphSlamResult result;
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

    const std::vector<Eigen::Vector3d> descriptor_points =
        stridePoints(registration_points, params_.descriptor_stride);
    std::vector<Eigen::Vector3d> loop_points =
        stridePoints(map_points, params_.loop_stride);
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
    result.keyframe_added = true;
    result.num_keyframes = numKeyframes();
    result.num_loop_edges = numLoopEdges();
    result.submap_points = submapSize();
    return result;
  }

  const Eigen::Matrix4d prediction = pose_ * last_delta_;
  const Eigen::Matrix4d previous_pose = pose_;
  const ndt::NDTResult alignment =
      registration_.align(registration_points, prediction);
  pose_ = alignment.transformation;
  last_delta_ = previous_pose.inverse() * pose_;

  result.pose = pose_;
  result.q_w_curr = Eigen::Quaterniond(pose_.block<3, 3>(0, 0)).normalized();
  result.t_w_curr = pose_.block<3, 1>(0, 3);
  result.front_end_result = alignment;
  result.initialized = true;
  result.valid = alignment.iterations > 0;
  result.converged = alignment.converged;

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
  double loop_descriptor_distance =
      std::numeric_limits<double>::infinity();
  if (params_.enable_loop_closure && !descriptor_points.empty() &&
      scan_context_.numScans() > 0) {
    const scan_context::LoopCandidate candidate =
        scan_context_.detectLoop(descriptor_points);
    if (candidate.valid &&
        static_cast<int>(keyframes_.size()) - candidate.index >=
            params_.min_loop_index_gap) {
      loop_index = candidate.index;
      loop_descriptor_distance = candidate.distance;
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

  if (loop_index >= 0 &&
      tryAddLoopEdge(loop_index, current_index,
                     loop_descriptor_distance)) {
    result.loop_detected = true;
    result.loop_index = loop_index;
  }

  if ((params_.optimize_every_n_keyframes > 0 &&
       static_cast<int>(keyframes_.size()) % params_.optimize_every_n_keyframes == 0) ||
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

HdlGraphSlamResult HdlGraphSlam::processExternalOdometry(
    const aloam::PointCloudConstPtr& cloud,
    const Eigen::Matrix4d& odometry_pose) {
  HdlGraphSlamResult result;
  result.pose = odometry_pose;
  result.q_w_curr =
      Eigen::Quaterniond(odometry_pose.block<3, 3>(0, 0)).normalized();
  result.t_w_curr = odometry_pose.block<3, 1>(0, 3);
  result.initialized = initialized_;
  result.num_keyframes = numKeyframes();
  result.num_loop_edges = numLoopEdges();
  if (!odometry_pose.array().isFinite().all()) {
    return result;
  }

  const std::vector<Eigen::Vector3d> descriptor_source =
      preprocess(cloud, params_.registration_voxel_size);
  const std::vector<Eigen::Vector3d> loop_source =
      preprocess(cloud, params_.map_voxel_size);
  if (descriptor_source.empty() || loop_source.empty()) {
    return result;
  }
  const std::vector<Eigen::Vector3d> descriptor_points =
      stridePoints(descriptor_source, params_.descriptor_stride);
  std::vector<Eigen::Vector3d> loop_points =
      stridePoints(loop_source, params_.loop_stride);
  if (loop_points.size() < 20) {
    loop_points = descriptor_points;
  }

  ++valid_frame_count_;
  pose_ = odometry_pose;
  if (!initialized_) {
    last_keyframe_pose_ = odometry_pose;
    addKeyframe(odometry_pose, {}, descriptor_points, loop_points);
    scan_context_.addScan(descriptor_points);
    initialized_ = true;
    result.initialized = true;
    result.valid = true;
    result.keyframe_added = true;
    result.num_keyframes = numKeyframes();
    return result;
  }

  result.initialized = true;
  result.valid = true;
  if (!shouldCreateKeyframe(odometry_pose)) {
    if (loopCorrectionEnabled()) {
      const Eigen::Matrix4d correction =
          latestOptimizedPoseMatrix() *
          poseToMatrix(keyframes_.back().raw_pose).inverse();
      result.pose = correction * odometry_pose;
    } else {
      result.pose = odometry_pose;
    }
    result.q_w_curr =
        Eigen::Quaterniond(result.pose.block<3, 3>(0, 0)).normalized();
    result.t_w_curr = result.pose.block<3, 1>(0, 3);
    return result;
  }

  int loop_index = -1;
  double loop_descriptor_distance =
      std::numeric_limits<double>::infinity();
  if (params_.enable_loop_closure && !descriptor_points.empty() &&
      scan_context_.numScans() > 0) {
    const scan_context::LoopCandidate candidate =
        scan_context_.detectLoop(descriptor_points);
    if (candidate.valid &&
        static_cast<int>(keyframes_.size()) - candidate.index >=
            params_.min_loop_index_gap) {
      loop_index = candidate.index;
      loop_descriptor_distance = candidate.distance;
      ++loop_candidate_detections_;
      loop_candidate_distance_sum_ += candidate.distance;
      loop_candidate_distance_min_ =
          std::min(loop_candidate_distance_min_, candidate.distance);
    }
  }

  const int current_index = static_cast<int>(keyframes_.size());
  const Eigen::Matrix4d propagated_optimized_pose =
      latestOptimizedPoseMatrix() *
      poseToMatrix(keyframes_.back().raw_pose).inverse() *
      odometry_pose;
  addKeyframe(odometry_pose, {}, descriptor_points, loop_points);
  keyframes_.back().optimized_pose =
      matrixToPose(propagated_optimized_pose);
  result.keyframe_added = true;
  odom_edges_.push_back(
      {current_index - 1, current_index,
       relativePose(keyframes_[static_cast<size_t>(current_index - 1)].raw_pose,
                    keyframes_[static_cast<size_t>(current_index)].raw_pose)});
  scan_context_.addScan(
      keyframes_[static_cast<size_t>(current_index)].descriptor_points);

  if (loop_index >= 0) {
    if (tryAddLoopEdge(loop_index, current_index,
                       loop_descriptor_distance)) {
      result.loop_detected = true;
      result.loop_index = loop_index;
    } else {
      ++loop_registration_rejections_;
    }
  }
  if ((params_.optimize_every_n_keyframes > 0 &&
       static_cast<int>(keyframes_.size()) %
               params_.optimize_every_n_keyframes ==
           0) ||
      result.loop_detected) {
    optimizePoseGraph();
  }

  result.pose =
      loopCorrectionEnabled() ? latestOptimizedPoseMatrix() : odometry_pose;
  result.q_w_curr =
      Eigen::Quaterniond(result.pose.block<3, 3>(0, 0)).normalized();
  result.t_w_curr = result.pose.block<3, 1>(0, 3);
  result.num_keyframes = numKeyframes();
  result.num_loop_edges = numLoopEdges();
  return result;
}

std::vector<Eigen::Matrix4d> HdlGraphSlam::correctedExternalTrajectory(
    const std::vector<Eigen::Matrix4d>& odometry_poses,
    double correction_gain) const {
  if (odometry_poses.empty() || keyframes_.empty() ||
      !loopCorrectionEnabled()) {
    return odometry_poses;
  }
  std::vector<Eigen::Matrix4d> corrected = odometry_poses;
  std::vector<Eigen::Matrix4d> corrections;
  corrections.reserve(keyframes_.size());
  for (const auto& keyframe : keyframes_) {
    corrections.push_back(
        poseToMatrix(keyframe.optimized_pose) *
        poseToMatrix(keyframe.raw_pose).inverse());
  }

  size_t segment = 0;
  for (size_t frame = 0; frame < corrected.size(); ++frame) {
    while (segment + 1 < keyframes_.size() &&
           frame > keyframes_[segment + 1].frame_index) {
      ++segment;
    }
    Eigen::Matrix4d correction = corrections[segment];
    if (segment + 1 < keyframes_.size()) {
      const size_t from = keyframes_[segment].frame_index;
      const size_t to = keyframes_[segment + 1].frame_index;
      const double alpha =
          to > from
              ? std::clamp(static_cast<double>(frame - from) /
                               static_cast<double>(to - from),
                           0.0, 1.0)
              : 1.0;
      const Eigen::Quaterniond from_q(
          corrections[segment].block<3, 3>(0, 0));
      const Eigen::Quaterniond to_q(
          corrections[segment + 1].block<3, 3>(0, 0));
      correction.block<3, 3>(0, 0) =
          from_q.slerp(alpha, to_q).normalized().toRotationMatrix();
      correction.block<3, 1>(0, 3) =
          (1.0 - alpha) *
              corrections[segment].block<3, 1>(0, 3) +
          alpha * corrections[segment + 1].block<3, 1>(0, 3);
    }
    const double gain = std::clamp(correction_gain, 0.0, 1.0);
    const Eigen::Quaterniond correction_q(
        correction.block<3, 3>(0, 0));
    correction.block<3, 3>(0, 0) =
        Eigen::Quaterniond::Identity()
            .slerp(gain, correction_q)
            .normalized()
            .toRotationMatrix();
    correction.block<3, 1>(0, 3) *= gain;
    correction.row(3) << 0.0, 0.0, 0.0, 1.0;
    corrected[frame] = correction * odometry_poses[frame];
  }
  return corrected;
}

int HdlGraphSlam::numLoopClusters() const {
  const int radius = std::max(0, params_.loop_cluster_keyframe_radius);
  std::vector<std::pair<int, int>> representatives;
  representatives.reserve(loop_edges_.size());
  for (const auto& edge : loop_edges_) {
    bool clustered = false;
    for (const auto& representative : representatives) {
      if (std::abs(edge.from - representative.first) <= radius &&
          std::abs(edge.to - representative.second) <= radius) {
        clustered = true;
        break;
      }
    }
    if (!clustered) {
      representatives.emplace_back(edge.from, edge.to);
    }
  }
  return static_cast<int>(representatives.size());
}

bool HdlGraphSlam::tryAddLoopEdge(int loop_index, int current_index,
                                  double descriptor_distance) {
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

  auto build_submap = [&](int center, int first, int last) {
    std::vector<Eigen::Vector3d> submap;
    const Eigen::Matrix4d center_pose =
        poseToMatrix(keyframes_[static_cast<size_t>(center)].raw_pose);
    for (int index = first; index <= last; ++index) {
      const auto& points =
          keyframes_[static_cast<size_t>(index)].loop_points;
      const Eigen::Matrix4d frame_to_center =
          center_pose.inverse() *
          poseToMatrix(keyframes_[static_cast<size_t>(index)].raw_pose);
      const auto transformed = transformPoints(points, frame_to_center);
      submap.insert(submap.end(), transformed.begin(), transformed.end());
    }
    return submap;
  };
  const int half_window = std::max(0, params_.loop_submap_half_window);
  const int last_keyframe = static_cast<int>(keyframes_.size()) - 1;
  const std::vector<Eigen::Vector3d> target = build_submap(
      loop_index, std::max(0, loop_index - half_window),
      std::min(last_keyframe, loop_index + half_window));
  const std::vector<Eigen::Vector3d> source = build_submap(
      current_index, std::max(0, current_index - half_window),
      current_index);
  if (target.size() < 20 || source.size() < 20) {
    return false;
  }

  gicp::GICPRegistration registration(params_.loop_gicp);
  registration.setTarget(target);

  const Eigen::Matrix4d initial_guess = poseToMatrix(
      relativePose(keyframes_[static_cast<size_t>(loop_index)].optimized_pose,
                   keyframes_[static_cast<size_t>(current_index)].optimized_pose));
  const gicp::GICPResult gicp_result =
      registration.align(source, initial_guess);

  HdlGraphSlamLoopAttempt attempt;
  attempt.from = loop_index;
  attempt.to = current_index;
  attempt.descriptor_distance = descriptor_distance;
  attempt.converged = gicp_result.converged;
  attempt.fitness = gicp_result.fitness;
  attempt.rmse = gicp_result.rmse;
  attempt.correspondences = gicp_result.num_correspondences;
  attempt.accepted =
      gicp_result.converged &&
      gicp_result.fitness <= params_.loop_fitness_threshold &&
      gicp_result.rmse <= params_.loop_rmse_threshold &&
      gicp_result.num_correspondences >= params_.min_loop_correspondences;
  loop_attempts_.push_back(attempt);
  if (!attempt.accepted) {
    return false;
  }

  LoopEdge edge;
  edge.from = loop_index;
  edge.to = current_index;
  edge.relative_pose = matrixToPose(gicp_result.transformation);
  edge.fitness = gicp_result.fitness;
  edge.correspondences = gicp_result.num_correspondences;
  loop_edges_.push_back(edge);
  return true;
}

void HdlGraphSlam::optimizePoseGraph() {
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

  if (params_.enable_floor_constraint && params_.floor_weight > 0.0) {
    for (size_t i = 1; i < keyframes_.size(); ++i) {
      problem.AddResidualBlock(
          FloorHeightCost::Create(params_.floor_height, params_.floor_weight),
          nullptr, t_params[i].data());
    }
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

HdlGraphSlam::PoseState HdlGraphSlam::relativePose(const PoseState& from,
                                                   const PoseState& to) {
  PoseState relative;
  relative.q = (from.q.conjugate() * to.q).normalized();
  relative.t = from.q.conjugate() * (to.t - from.t);
  return relative;
}

HdlGraphSlam::PoseState HdlGraphSlam::matrixToPose(
    const Eigen::Matrix4d& transform) {
  PoseState pose;
  pose.q = Eigen::Quaterniond(transform.block<3, 3>(0, 0)).normalized();
  pose.t = transform.block<3, 1>(0, 3);
  return pose;
}

Eigen::Matrix4d HdlGraphSlam::poseToMatrix(const PoseState& pose) {
  Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();
  transform.block<3, 3>(0, 0) = pose.q.normalized().toRotationMatrix();
  transform.block<3, 1>(0, 3) = pose.t;
  return transform;
}

std::vector<Eigen::Vector3d> HdlGraphSlam::stridePoints(
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

Eigen::Matrix4d HdlGraphSlam::latestOptimizedPoseMatrix() const {
  if (keyframes_.empty()) {
    return Eigen::Matrix4d::Identity();
  }
  return poseToMatrix(keyframes_.back().optimized_pose);
}

void HdlGraphSlam::clear() {
  registration_ = ndt::NDTRegistration(params_.ndt);
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
  loop_candidate_detections_ = 0;
  loop_registration_rejections_ = 0;
  loop_candidate_distance_sum_ = 0.0;
  loop_candidate_distance_min_ = std::numeric_limits<double>::infinity();
  loop_attempts_.clear();
}

}  // namespace hdl_graph_slam
}  // namespace localization_zoo
