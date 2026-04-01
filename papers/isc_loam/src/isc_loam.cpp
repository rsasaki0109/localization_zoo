#include "isc_loam/isc_loam.h"

#include <ceres/ceres.h>

#include <algorithm>
#include <array>
#include <limits>

namespace localization_zoo {
namespace isc_loam {

namespace {

constexpr double kTwoPi = 2.0 * M_PI;

double wrapAngle(double angle) {
  while (angle < 0.0) {
    angle += kTwoPi;
  }
  while (angle >= kTwoPi) {
    angle -= kTwoPi;
  }
  return angle;
}

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

Eigen::MatrixXd IntensityScanContextManager::makeDescriptor(
    const aloam::PointCloudConstPtr& cloud) const {
  Eigen::MatrixXd descriptor =
      Eigen::MatrixXd::Zero(params_.num_rings, params_.num_sectors);

  if (cloud == nullptr || params_.num_rings <= 0 || params_.num_sectors <= 0 ||
      params_.max_radius <= 0.0) {
    return descriptor;
  }

  for (const auto& point : cloud->points) {
    if (!std::isfinite(point.x) || !std::isfinite(point.y) ||
        !std::isfinite(point.z) || !std::isfinite(point.intensity)) {
      continue;
    }

    const double range = std::hypot(point.x, point.y);
    if (range <= 1e-6 || range > params_.max_radius) {
      continue;
    }

    const double angle = wrapAngle(std::atan2(point.y, point.x));
    const int ring_idx = std::min(
        params_.num_rings - 1,
        static_cast<int>(std::floor(range / params_.max_radius * params_.num_rings)));
    const int sector_idx = std::min(
        params_.num_sectors - 1,
        static_cast<int>(std::floor(angle / kTwoPi * params_.num_sectors)));
    const double value = std::max(0.0, params_.intensity_scale * point.intensity);
    descriptor(ring_idx, sector_idx) = std::max(descriptor(ring_idx, sector_idx), value);
  }

  return descriptor;
}

Eigen::VectorXd IntensityScanContextManager::makeRingKey(
    const Eigen::MatrixXd& descriptor) const {
  if (descriptor.rows() == 0) {
    return Eigen::VectorXd();
  }
  return descriptor.rowwise().mean();
}

Eigen::VectorXd IntensityScanContextManager::makeSectorKey(
    const Eigen::MatrixXd& descriptor) const {
  if (descriptor.cols() == 0) {
    return Eigen::VectorXd();
  }
  return descriptor.colwise().mean().transpose();
}

int IntensityScanContextManager::addScan(const aloam::PointCloudConstPtr& cloud) {
  StoredContext context;
  context.descriptor = makeDescriptor(cloud);
  context.ring_key = makeRingKey(context.descriptor);
  context.sector_key = makeSectorKey(context.descriptor);
  contexts_.push_back(std::move(context));
  return static_cast<int>(contexts_.size()) - 1;
}

IntensityLoopCandidate IntensityScanContextManager::detectLoop(
    const aloam::PointCloudConstPtr& cloud) const {
  IntensityLoopCandidate result;
  if (contexts_.empty()) {
    return result;
  }

  const Eigen::MatrixXd query_descriptor = makeDescriptor(cloud);
  const Eigen::VectorXd query_ring_key = makeRingKey(query_descriptor);
  const Eigen::VectorXd query_sector_key = makeSectorKey(query_descriptor);

  const int searchable_size = std::max(
      0, static_cast<int>(contexts_.size()) - params_.exclude_recent_frames);
  if (searchable_size <= 0) {
    return result;
  }

  std::vector<std::pair<double, int>> ring_key_distances;
  ring_key_distances.reserve(static_cast<size_t>(searchable_size));
  for (int i = 0; i < searchable_size; ++i) {
    const double distance = (contexts_[i].ring_key - query_ring_key).norm();
    ring_key_distances.emplace_back(distance, i);
  }

  const int candidate_count =
      std::min(params_.num_candidates, static_cast<int>(ring_key_distances.size()));
  std::partial_sort(ring_key_distances.begin(),
                    ring_key_distances.begin() + candidate_count,
                    ring_key_distances.end());

  double best_distance = std::numeric_limits<double>::infinity();
  int best_shift = 0;
  int best_index = -1;
  for (int i = 0; i < candidate_count; ++i) {
    int shift = 0;
    const double distance = compareDescriptors(
        query_descriptor, query_sector_key,
        contexts_[static_cast<size_t>(ring_key_distances[i].second)], &shift);
    if (distance < best_distance) {
      best_distance = distance;
      best_shift = shift;
      best_index = ring_key_distances[i].second;
    }
  }

  if (best_index >= 0 && best_distance < params_.distance_threshold) {
    result.valid = true;
    result.index = best_index;
    result.distance = best_distance;
    result.yaw_shift = best_shift;
    result.yaw_offset_rad =
        static_cast<double>(best_shift) * kTwoPi / params_.num_sectors;
  }
  return result;
}

void IntensityScanContextManager::clear() { contexts_.clear(); }

int IntensityScanContextManager::alignSectorKeys(const Eigen::VectorXd& query,
                                                 const Eigen::VectorXd& candidate) const {
  if (query.size() == 0 || candidate.size() != query.size()) {
    return 0;
  }

  double best_distance = std::numeric_limits<double>::infinity();
  int best_shift = 0;
  for (int shift = 0; shift < query.size(); ++shift) {
    double distance = 0.0;
    for (int col = 0; col < query.size(); ++col) {
      const int shifted_col = (col + shift) % query.size();
      distance += std::abs(query[col] - candidate[shifted_col]);
    }
    if (distance < best_distance) {
      best_distance = distance;
      best_shift = shift;
    }
  }
  return best_shift;
}

double IntensityScanContextManager::compareDescriptors(
    const Eigen::MatrixXd& query, const Eigen::VectorXd& query_sector_key,
    const StoredContext& candidate, int* best_shift) const {
  if (query.rows() == 0 || query.cols() == 0 ||
      candidate.descriptor.rows() != query.rows() ||
      candidate.descriptor.cols() != query.cols()) {
    if (best_shift != nullptr) {
      *best_shift = 0;
    }
    return 1.0;
  }

  const int initial_shift = alignSectorKeys(query_sector_key, candidate.sector_key);
  const int search_radius = std::max(
      0, static_cast<int>(std::round(params_.search_ratio * query.cols())));

  double best_distance = std::numeric_limits<double>::infinity();
  int best_local_shift = initial_shift;
  for (int offset = -search_radius; offset <= search_radius; ++offset) {
    const int shift = (initial_shift + offset + query.cols()) % query.cols();
    const Eigen::MatrixXd shifted = circularShiftColumns(candidate.descriptor, shift);

    double similarity = 0.0;
    int valid_columns = 0;
    for (int col = 0; col < query.cols(); ++col) {
      const Eigen::VectorXd query_col = query.col(col);
      const Eigen::VectorXd candidate_col = shifted.col(col);
      const double query_norm = query_col.norm();
      const double candidate_norm = candidate_col.norm();
      if (query_norm < 1e-6 || candidate_norm < 1e-6) {
        continue;
      }
      similarity += query_col.dot(candidate_col) / (query_norm * candidate_norm);
      ++valid_columns;
    }

    const double distance =
        valid_columns > 0 ? 1.0 - similarity / valid_columns : 1.0;
    if (distance < best_distance) {
      best_distance = distance;
      best_local_shift = shift;
    }
  }

  if (best_shift != nullptr) {
    *best_shift = best_local_shift;
  }
  return best_distance;
}

Eigen::MatrixXd IntensityScanContextManager::circularShiftColumns(
    const Eigen::MatrixXd& matrix, int shift) {
  if (matrix.cols() == 0) {
    return matrix;
  }

  Eigen::MatrixXd shifted = matrix;
  const int wrapped_shift = (shift % matrix.cols() + matrix.cols()) % matrix.cols();
  for (int col = 0; col < matrix.cols(); ++col) {
    shifted.col(col) = matrix.col((col + wrapped_shift) % matrix.cols());
  }
  return shifted;
}

IscLoamParams::IscLoamParams() {
  front_end.input_point_stride = 1;
  front_end.mapping_update_interval = 1;

  scan_context.exclude_recent_frames = 5;
  scan_context.num_candidates = 5;
  scan_context.distance_threshold = 0.18;
  scan_context.search_ratio = 0.2;

  loop_gicp.max_correspondence_distance = 5.0;
  loop_gicp.max_iterations = 20;
  loop_gicp.k_neighbors = 15;
}

IscLoam::IscLoam(const IscLoamParams& params)
    : params_(params),
      front_end_(params.front_end),
      scan_context_(params.scan_context) {}

IscLoamResult IscLoam::process(const aloam::PointCloudConstPtr& cloud) {
  IscLoamResult result;
  result.front_end_result = front_end_.process(cloud);
  result.q_w_curr = result.front_end_result.q_w_curr;
  result.t_w_curr = result.front_end_result.t_w_curr;
  result.num_keyframes = numKeyframes();
  result.num_loop_edges = numLoopEdges();

  if (!result.front_end_result.valid) {
    return result;
  }

  PoseState mapped_pose;
  mapped_pose.q = result.front_end_result.q_w_curr.normalized();
  mapped_pose.t = result.front_end_result.t_w_curr;
  result.valid = true;
  valid_frame_count_++;

  if (!shouldCreateKeyframe(mapped_pose)) {
    return result;
  }

  int loop_index = -1;
  if (params_.enable_loop_closure && cloud != nullptr && scan_context_.numScans() > 0) {
    const IntensityLoopCandidate candidate = scan_context_.detectLoop(cloud);
    if (candidate.valid &&
        static_cast<int>(keyframes_.size()) - candidate.index >=
            params_.min_loop_index_gap) {
      loop_index = candidate.index;
    }
  }

  Keyframe keyframe;
  keyframe.raw_pose = mapped_pose;
  keyframe.optimized_pose = mapped_pose;
  keyframe.loop_points = makeLoopPoints(cloud, params_.loop_stride);
  if (keyframe.loop_points.size() < 20) {
    keyframe.loop_points = makeLoopPoints(cloud, 1);
  }

  const int current_index = static_cast<int>(keyframes_.size());
  keyframes_.push_back(std::move(keyframe));
  result.keyframe_added = true;

  if (current_index > 0) {
    odom_edges_.push_back(
        {current_index - 1, current_index,
         relativePose(keyframes_[static_cast<size_t>(current_index - 1)].raw_pose,
                      mapped_pose)});
  }

  scan_context_.addScan(cloud);

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

void IscLoam::clear() {
  front_end_ = floam::FLoam(params_.front_end);
  scan_context_.clear();
  keyframes_.clear();
  odom_edges_.clear();
  loop_edges_.clear();
  valid_frame_count_ = 0;
}

Eigen::Matrix4d IscLoam::latestOptimizedPoseMatrix() const {
  if (keyframes_.empty()) {
    return Eigen::Matrix4d::Identity();
  }
  return poseToMatrix(keyframes_.back().optimized_pose);
}

bool IscLoam::shouldCreateKeyframe(const PoseState& pose) const {
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

std::vector<Eigen::Vector3d> IscLoam::makeLoopPoints(
    const aloam::PointCloudConstPtr& cloud, size_t stride) const {
  std::vector<Eigen::Vector3d> points;
  if (cloud == nullptr) {
    return points;
  }

  const size_t effective_stride = std::max<size_t>(1, stride);
  points.reserve(cloud->size() / effective_stride + 1);
  for (size_t i = 0; i < cloud->size(); i += effective_stride) {
    const auto& point = cloud->points[i];
    if (!std::isfinite(point.x) || !std::isfinite(point.y) ||
        !std::isfinite(point.z)) {
      continue;
    }
    const Eigen::Vector3d eigen_point(point.x, point.y, point.z);
    const double range = eigen_point.norm();
    if (range < params_.min_range || range > params_.max_range) {
      continue;
    }
    points.push_back(eigen_point);
  }
  return points;
}

bool IscLoam::tryAddLoopEdge(int loop_index, int current_index) {
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

  gicp::GICPRegistration registration(params_.loop_gicp);
  registration.setTarget(target);

  const Eigen::Matrix4d initial_guess = poseToMatrix(
      relativePose(keyframes_[static_cast<size_t>(current_index)].optimized_pose,
                   keyframes_[static_cast<size_t>(loop_index)].optimized_pose));
  const gicp::GICPResult gicp_result = registration.align(source, initial_guess);
  if (!gicp_result.converged ||
      gicp_result.fitness > params_.loop_fitness_threshold ||
      gicp_result.num_correspondences < params_.min_loop_correspondences) {
    return false;
  }

  LoopEdge edge;
  edge.from = loop_index;
  edge.to = current_index;
  edge.relative_pose = matrixToPose(gicp_result.transformation.inverse());
  edge.fitness = gicp_result.fitness;
  edge.correspondences = gicp_result.num_correspondences;
  loop_edges_.push_back(edge);
  return true;
}

void IscLoam::optimizePoseGraph() {
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
    problem.AddParameterBlock(q_params[i].data(), 4,
                              new ceres::EigenQuaternionParameterization());
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

IscLoam::PoseState IscLoam::relativePose(const PoseState& from,
                                         const PoseState& to) {
  PoseState relative;
  relative.q = (from.q.conjugate() * to.q).normalized();
  relative.t = from.q.conjugate() * (to.t - from.t);
  return relative;
}

IscLoam::PoseState IscLoam::matrixToPose(const Eigen::Matrix4d& transform) {
  PoseState pose;
  pose.q = Eigen::Quaterniond(transform.block<3, 3>(0, 0)).normalized();
  pose.t = transform.block<3, 1>(0, 3);
  return pose;
}

Eigen::Matrix4d IscLoam::poseToMatrix(const PoseState& pose) {
  Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();
  transform.block<3, 3>(0, 0) = pose.q.normalized().toRotationMatrix();
  transform.block<3, 1>(0, 3) = pose.t;
  return transform;
}

}  // namespace isc_loam
}  // namespace localization_zoo
