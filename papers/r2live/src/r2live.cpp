#include "r2live/r2live.h"

#include <ceres/ceres.h>

#include <algorithm>
#include <array>
#include <cmath>

namespace localization_zoo {
namespace r2live {

namespace {

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

struct VisualReprojectionCost {
  VisualReprojectionCost(const CameraIntrinsics& camera,
                         const Eigen::Matrix3d& body_R_camera,
                         const Eigen::Vector3d& body_t_camera,
                         const Eigen::Vector3d& landmark_world,
                         const Eigen::Vector2d& pixel,
                         double pixel_sigma)
      : camera_(camera),
        body_R_camera_(body_R_camera),
        body_t_camera_(body_t_camera),
        landmark_world_(landmark_world),
        pixel_(pixel),
        pixel_sigma_(pixel_sigma) {}

  template <typename T>
  bool operator()(const T* const q_ptr, const T* const t_ptr,
                  T* residuals_ptr) const {
    Eigen::Map<const Eigen::Quaternion<T>> q_wb(q_ptr);
    Eigen::Map<const Eigen::Matrix<T, 3, 1>> t_wb(t_ptr);

    const Eigen::Quaternion<T> q_bc(
        T(Eigen::Quaterniond(body_R_camera_).w()),
        T(Eigen::Quaterniond(body_R_camera_).x()),
        T(Eigen::Quaterniond(body_R_camera_).y()),
        T(Eigen::Quaterniond(body_R_camera_).z()));
    const Eigen::Quaternion<T> q_wc = q_wb * q_bc;
    const Eigen::Matrix<T, 3, 1> t_wc =
        t_wb + q_wb * body_t_camera_.cast<T>();
    const Eigen::Matrix<T, 3, 1> p_w = landmark_world_.cast<T>();
    const Eigen::Matrix<T, 3, 1> p_c = q_wc.conjugate() * (p_w - t_wc);

    if (p_c.x() <= T(1e-6)) {
      residuals_ptr[0] = T(0);
      residuals_ptr[1] = T(0);
      return true;
    }

    const T u = T(camera_.fx) * p_c.y() / p_c.x() + T(camera_.cx);
    const T v = T(camera_.fy) * p_c.z() / p_c.x() + T(camera_.cy);

    residuals_ptr[0] = (u - T(pixel_.x())) / T(pixel_sigma_);
    residuals_ptr[1] = (v - T(pixel_.y())) / T(pixel_sigma_);
    return true;
  }

  static ceres::CostFunction* Create(const CameraIntrinsics& camera,
                                     const Eigen::Matrix3d& body_R_camera,
                                     const Eigen::Vector3d& body_t_camera,
                                     const Eigen::Vector3d& landmark_world,
                                     const Eigen::Vector2d& pixel,
                                     double pixel_sigma) {
    return new ceres::AutoDiffCostFunction<VisualReprojectionCost, 2, 4, 3>(
        new VisualReprojectionCost(camera, body_R_camera, body_t_camera,
                                   landmark_world, pixel, pixel_sigma));
  }

  CameraIntrinsics camera_;
  Eigen::Matrix3d body_R_camera_;
  Eigen::Vector3d body_t_camera_;
  Eigen::Vector3d landmark_world_;
  Eigen::Vector2d pixel_;
  double pixel_sigma_;
};

}  // namespace

R2Live::R2Live(const R2LiveParams& params)
    : params_(params), front_end_(params.front_end) {}

void R2Live::setLandmarks(const std::vector<Landmark>& landmarks) {
  landmarks_ = landmarks;
  landmark_lookup_.clear();
  landmark_lookup_.reserve(landmarks.size());
  for (size_t i = 0; i < landmarks.size(); ++i) {
    landmark_lookup_[landmarks[i].id] = i;
  }
}

void R2Live::clearLandmarks() {
  landmarks_.clear();
  landmark_lookup_.clear();
}

R2LiveResult R2Live::process(
    const std::vector<Eigen::Vector3d>& raw_points,
    const std::vector<VisualObservation>& visual_observations,
    const std::vector<imu_preintegration::ImuSample>& imu_samples) {
  R2LiveResult result;
  const fast_lio2::FastLio2Result front_end_result =
      front_end_.process(raw_points, imu_samples);
  result.front_end_state = front_end_result.state;
  result.initialized = front_end_result.initialized;
  result.imu_used = front_end_result.imu_used;
  result.num_keyframes = numKeyframes();

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

  Keyframe keyframe;
  keyframe.raw_pose = mapped_pose;
  keyframe.optimized_pose = mapped_pose;
  keyframe.observations = matchObservations(mapped_pose, visual_observations);

  const int current_index = static_cast<int>(keyframes_.size());
  keyframes_.push_back(std::move(keyframe));
  result.keyframe_added = true;
  result.visual_used =
      static_cast<int>(keyframes_.back().observations.size()) >=
      params_.min_visual_observations;
  result.num_visual_factors =
      static_cast<int>(keyframes_.back().observations.size());

  if (current_index > 0) {
    odom_edges_.push_back(
        {current_index - 1, current_index,
         relativePose(keyframes_[current_index - 1].raw_pose, mapped_pose)});
  }

  if ((params_.optimize_every_n_keyframes > 0 &&
       static_cast<int>(keyframes_.size()) % params_.optimize_every_n_keyframes == 0) ||
      result.visual_used) {
    optimizePoseGraph();
  }

  result.q_w_curr = keyframes_.back().optimized_pose.q;
  result.t_w_curr = keyframes_.back().optimized_pose.t;
  result.num_keyframes = numKeyframes();
  return result;
}

Eigen::Matrix4d R2Live::latestOptimizedPoseMatrix() const {
  if (keyframes_.empty()) {
    return Eigen::Matrix4d::Identity();
  }
  return poseToMatrix(keyframes_.back().optimized_pose);
}

void R2Live::clear() {
  front_end_.clear();
  keyframes_.clear();
  odom_edges_.clear();
  valid_frame_count_ = 0;
}

bool R2Live::shouldCreateKeyframe(const PoseState& pose) const {
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

std::vector<R2Live::MatchedObservation> R2Live::matchObservations(
    const PoseState& pose,
    const std::vector<VisualObservation>& visual_observations) const {
  std::vector<MatchedObservation> matched;
  matched.reserve(visual_observations.size());
  for (const auto& observation : visual_observations) {
    const auto it = landmark_lookup_.find(observation.landmark_id);
    if (it == landmark_lookup_.end() || !observation.pixel.allFinite()) {
      continue;
    }

    Eigen::Vector2d predicted_pixel;
    if (!projectLandmark(pose, landmarks_[it->second].position, &predicted_pixel)) {
      continue;
    }

    matched.push_back({static_cast<int>(it->second), observation.pixel});
  }
  return matched;
}

bool R2Live::projectLandmark(const PoseState& pose,
                             const Eigen::Vector3d& landmark_world,
                             Eigen::Vector2d* pixel) const {
  const Eigen::Quaterniond q_wc = pose.q * Eigen::Quaterniond(params_.body_R_camera);
  const Eigen::Vector3d t_wc = pose.t + pose.q * params_.body_t_camera;
  const Eigen::Vector3d p_c = q_wc.conjugate() * (landmark_world - t_wc);
  if (p_c.x() <= 1e-6) {
    return false;
  }

  const double u = params_.camera.fx * p_c.y() / p_c.x() + params_.camera.cx;
  const double v = params_.camera.fy * p_c.z() / p_c.x() + params_.camera.cy;
  if (u < 0.0 || u >= params_.camera.width ||
      v < 0.0 || v >= params_.camera.height) {
    return false;
  }

  if (pixel != nullptr) {
    pixel->x() = u;
    pixel->y() = v;
  }
  return true;
}

void R2Live::optimizePoseGraph() {
  if (keyframes_.size() < 2) {
    return;
  }

  std::vector<std::array<double, 4>> q_params(keyframes_.size());
  std::vector<std::array<double, 3>> t_params(keyframes_.size());
  int total_visual_factors = 0;
  for (size_t i = 0; i < keyframes_.size(); ++i) {
    const PoseState& pose = keyframes_[i].optimized_pose;
    q_params[i] = {pose.q.x(), pose.q.y(), pose.q.z(), pose.q.w()};
    t_params[i] = {pose.t.x(), pose.t.y(), pose.t.z()};
    total_visual_factors += static_cast<int>(keyframes_[i].observations.size());
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

  ceres::LossFunction* visual_loss = new ceres::HuberLoss(
      params_.visual_huber_delta / std::max(1e-6, params_.visual_pixel_sigma));
  for (size_t i = 0; i < keyframes_.size(); ++i) {
    if (static_cast<int>(keyframes_[i].observations.size()) <
        params_.min_visual_observations) {
      continue;
    }

    for (const auto& observation : keyframes_[i].observations) {
      problem.AddResidualBlock(
          VisualReprojectionCost::Create(
              params_.camera, params_.body_R_camera, params_.body_t_camera,
              landmarks_[static_cast<size_t>(observation.landmark_index)].position,
              observation.pixel, params_.visual_pixel_sigma),
          visual_loss, q_params[i].data(), t_params[i].data());
    }
  }

  if (total_visual_factors == 0) {
    return;
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

R2Live::PoseState R2Live::relativePose(const PoseState& from,
                                       const PoseState& to) {
  PoseState relative;
  relative.q = (from.q.conjugate() * to.q).normalized();
  relative.t = from.q.conjugate() * (to.t - from.t);
  return relative;
}

Eigen::Matrix4d R2Live::poseToMatrix(const PoseState& pose) {
  Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();
  transform.block<3, 3>(0, 0) = pose.q.normalized().toRotationMatrix();
  transform.block<3, 1>(0, 3) = pose.t;
  return transform;
}

double R2Live::rotationAngle(const Eigen::Quaterniond& quat) {
  const double clamped = std::clamp(std::abs(quat.normalized().w()), 0.0, 1.0);
  return 2.0 * std::acos(clamped);
}

}  // namespace r2live
}  // namespace localization_zoo
