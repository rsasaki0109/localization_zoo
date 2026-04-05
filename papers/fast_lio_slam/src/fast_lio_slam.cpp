#include "fast_lio_slam/fast_lio_slam.h"

#include <ceres/ceres.h>

#include <algorithm>
#include <array>

namespace localization_zoo {
namespace fast_lio_slam {

namespace {

double rotationAngle(const Eigen::Quaterniond& q) {
  const double clamped = std::clamp(std::abs(q.normalized().w()), 0.0, 1.0);
  return 2.0 * std::acos(clamped);
}

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

FastLioSlamParams::FastLioSlamParams() {
  scan_context.exclude_recent_frames = 5;
  scan_context.num_candidates = 5;
  scan_context.distance_threshold = 0.18;
  scan_context.search_ratio = 0.2;
  loop_gicp.max_correspondence_distance = 5.0;
  loop_gicp.max_iterations = 20;
  loop_gicp.k_neighbors = 15;
}

FastLioSlam::FastLioSlam(const FastLioSlamParams& params)
    : params_(params),
      front_end_(params.front_end),
      scan_context_(params.scan_context) {}

FastLioSlamResult FastLioSlam::process(
    const std::vector<Eigen::Vector3d>& raw_points,
    const std::vector<imu_preintegration::ImuSample>& imu_samples) {
  FastLioSlamResult result;
  const fast_lio2::FastLio2Result front_end_result =
      front_end_.process(raw_points, imu_samples);
  result.front_end_state = front_end_result.state;
  result.initialized = front_end_result.initialized;
  result.imu_used = front_end_result.imu_used;
  result.num_keyframes = numKeyframes();
  result.num_loop_edges = numLoopEdges();

  if (!front_end_result.initialized) {
    return result;
  }

  PoseState mapped_pose;
  mapped_pose.q = front_end_result.state.quat.normalized();
  mapped_pose.t = front_end_result.state.trans;
  result.q_w_curr = mapped_pose.q;
  result.t_w_curr = mapped_pose.t;

  if (!front_end_result.valid && !keyframes_.empty()) {
    return result;
  }

  result.valid = true;
  valid_frame_count_++;
  if (!shouldCreateKeyframe(mapped_pose)) {
    return result;
  }

  std::vector<Eigen::Vector3d> descriptor_points =
      stridePoints(raw_points, params_.descriptor_stride);
  std::vector<Eigen::Vector3d> loop_points =
      stridePoints(raw_points, params_.loop_stride);
  if (descriptor_points.empty()) {
    descriptor_points = raw_points;
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

  Keyframe keyframe;
  keyframe.raw_pose = mapped_pose;
  keyframe.optimized_pose = mapped_pose;
  keyframe.descriptor_points = std::move(descriptor_points);
  keyframe.loop_points = std::move(loop_points);

  const int current_index = static_cast<int>(keyframes_.size());
  keyframes_.push_back(std::move(keyframe));
  result.keyframe_added = true;

  if (current_index > 0) {
    odom_edges_.push_back(
        {current_index - 1, current_index,
         relativePose(keyframes_[current_index - 1].raw_pose, mapped_pose)});
  }

  scan_context_.addScan(keyframes_.back().descriptor_points);

  if (loop_index >= 0 && tryAddLoopEdge(loop_index, current_index)) {
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
  return result;
}

Eigen::Matrix4d FastLioSlam::latestOptimizedPoseMatrix() const {
  if (keyframes_.empty()) {
    return Eigen::Matrix4d::Identity();
  }
  return poseToMatrix(keyframes_.back().optimized_pose);
}

void FastLioSlam::clear() {
  front_end_.clear();
  scan_context_.clear();
  keyframes_.clear();
  odom_edges_.clear();
  loop_edges_.clear();
  valid_frame_count_ = 0;
}

bool FastLioSlam::shouldCreateKeyframe(const PoseState& pose) const {
  if (keyframes_.empty()) {
    return true;
  }

  const PoseState& last = keyframes_.back().raw_pose;
  const double translation = (pose.t - last.t).norm();
  const double rotation = rotationAngle(last.q.conjugate() * pose.q);

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

bool FastLioSlam::tryAddLoopEdge(int loop_index, int current_index) {
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

  const auto& target = keyframes_[loop_index].loop_points;
  const auto& source = keyframes_[current_index].loop_points;
  if (target.size() < 20 || source.size() < 20) {
    return false;
  }

  gicp::GICPRegistration registration(params_.loop_gicp);
  registration.setTarget(target);

  const Eigen::Matrix4d initial_guess = poseToMatrix(
      relativePose(keyframes_[loop_index].optimized_pose,
                   keyframes_[current_index].optimized_pose));
  const gicp::GICPResult gicp_result =
      registration.align(source, initial_guess);

  if (!gicp_result.converged ||
      gicp_result.fitness > params_.loop_fitness_threshold ||
      gicp_result.num_correspondences < params_.min_loop_correspondences) {
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

void FastLioSlam::optimizePoseGraph() {
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
    problem.SetManifold(q_params[i].data(), new ceres::EigenQuaternionManifold());
    problem.AddParameterBlock(t_params[i].data(), 3);
  }
  problem.SetParameterBlockConstant(q_params.front().data());
  problem.SetParameterBlockConstant(t_params.front().data());

  for (const auto& edge : odom_edges_) {
    problem.AddResidualBlock(
        PoseGraphEdgeCost::Create(edge.relative_pose.q, edge.relative_pose.t,
                                  params_.odom_rotation_weight,
                                  params_.odom_translation_weight),
        nullptr, q_params[edge.from].data(), t_params[edge.from].data(),
        q_params[edge.to].data(), t_params[edge.to].data());
  }

  ceres::LossFunction* loop_loss = new ceres::HuberLoss(1.0);
  for (const auto& edge : loop_edges_) {
    problem.AddResidualBlock(
        PoseGraphEdgeCost::Create(edge.relative_pose.q, edge.relative_pose.t,
                                  params_.loop_rotation_weight,
                                  params_.loop_translation_weight),
        loop_loss, q_params[edge.from].data(), t_params[edge.from].data(),
        q_params[edge.to].data(), t_params[edge.to].data());
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

FastLioSlam::PoseState FastLioSlam::relativePose(const PoseState& from,
                                                 const PoseState& to) {
  PoseState relative;
  relative.q = (from.q.conjugate() * to.q).normalized();
  relative.t = from.q.conjugate() * (to.t - from.t);
  return relative;
}

FastLioSlam::PoseState FastLioSlam::matrixToPose(
    const Eigen::Matrix4d& transform) {
  PoseState pose;
  pose.q = Eigen::Quaterniond(transform.block<3, 3>(0, 0)).normalized();
  pose.t = transform.block<3, 1>(0, 3);
  return pose;
}

Eigen::Matrix4d FastLioSlam::poseToMatrix(const PoseState& pose) {
  Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();
  transform.block<3, 3>(0, 0) = pose.q.normalized().toRotationMatrix();
  transform.block<3, 1>(0, 3) = pose.t;
  return transform;
}

std::vector<Eigen::Vector3d> FastLioSlam::stridePoints(
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

}  // namespace fast_lio_slam
}  // namespace localization_zoo
