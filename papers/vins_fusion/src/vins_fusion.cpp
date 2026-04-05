#include "vins_fusion/vins_fusion.h"

#include <ceres/ceres.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <common/ceres_compat.h>

namespace localization_zoo {
namespace vins_fusion {

namespace {

struct ImuPoseCost {
  ImuPoseCost(const Eigen::Quaterniond& delta_q, const Eigen::Vector3d& delta_t,
              double rotation_weight, double translation_weight)
      : delta_q_(delta_q),
        delta_t_(delta_t),
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
        T(delta_q_.w()), T(delta_q_.x()), T(delta_q_.y()), T(delta_q_.z()));
    const Eigen::Matrix<T, 3, 1> t_meas = delta_t_.template cast<T>();

    const Eigen::Quaternion<T> q_est = q_i.conjugate() * q_j;
    const Eigen::Quaternion<T> q_err = q_meas.conjugate() * q_est;
    const Eigen::Matrix<T, 3, 1> t_err = q_i.conjugate() * (t_j - t_i) - t_meas;

    Eigen::Map<Eigen::Matrix<T, 6, 1>> residuals(residuals_ptr);
    residuals.template head<3>() =
        T(rotation_weight_) * T(2.0) * q_err.vec();
    residuals.template tail<3>() =
        T(translation_weight_) * t_err;
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Quaterniond& delta_q,
                                     const Eigen::Vector3d& delta_t,
                                     double rotation_weight,
                                     double translation_weight) {
    return new ceres::AutoDiffCostFunction<ImuPoseCost, 6, 4, 3, 4, 3>(
        new ImuPoseCost(delta_q, delta_t, rotation_weight, translation_weight));
  }

  Eigen::Quaterniond delta_q_;
  Eigen::Vector3d delta_t_;
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
    const Eigen::Matrix<T, 3, 1> p_c =
        q_wc.conjugate() * (landmark_world_.cast<T>() - t_wc);

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

VinsFusion::VinsFusion(const VinsFusionParams& params) : params_(params) {}

void VinsFusion::setLandmarks(const std::vector<Landmark>& landmarks) {
  landmarks_ = landmarks;
  landmark_lookup_.clear();
  landmark_lookup_.reserve(landmarks.size());
  for (size_t i = 0; i < landmarks.size(); ++i) {
    landmark_lookup_[landmarks[i].id] = i;
  }
}

void VinsFusion::clearLandmarks() {
  landmarks_.clear();
  landmark_lookup_.clear();
}

VinsFusionResult VinsFusion::process(
    const std::vector<VisualObservation>& visual_observations,
    const std::vector<imu_preintegration::ImuSample>& imu_samples) {
  VinsFusionResult result;

  if (!initialized_) {
    current_raw_pose_.q = Eigen::Quaterniond::Identity();
    current_raw_pose_.t = Eigen::Vector3d::Zero();
    current_raw_pose_.v = params_.initial_velocity;

    Keyframe keyframe;
    keyframe.raw_pose = current_raw_pose_;
    keyframe.optimized_pose = current_raw_pose_;
    if (params_.enable_visual_factors) {
      keyframe.observations =
          matchObservations(current_raw_pose_, visual_observations);
    }

    keyframes_.push_back(std::move(keyframe));
    initialized_ = true;
    valid_frame_count_ = 1;

    result.raw_q_w_curr = current_raw_pose_.q;
    result.raw_t_w_curr = current_raw_pose_.t;
    result.q_w_curr = current_raw_pose_.q;
    result.t_w_curr = current_raw_pose_.t;
    result.v_w_curr = current_raw_pose_.v;
    result.initialized = true;
    result.valid = true;
    result.keyframe_added = true;
    result.visual_used =
        static_cast<int>(keyframes_.back().observations.size()) >=
        params_.min_visual_observations;
    result.num_visual_factors =
        static_cast<int>(keyframes_.back().observations.size());
    result.num_keyframes = numKeyframes();
    return result;
  }

  bool imu_used = false;
  const PoseState predicted_pose = predictRawPose(imu_samples, &imu_used);

  result.raw_q_w_curr = predicted_pose.q;
  result.raw_t_w_curr = predicted_pose.t;
  result.q_w_curr = predicted_pose.q;
  result.t_w_curr = predicted_pose.t;
  result.v_w_curr = predicted_pose.v;
  result.initialized = true;
  result.imu_used = imu_used;
  result.valid = true;

  current_raw_pose_ = predicted_pose;
  valid_frame_count_++;
  if (!shouldCreateKeyframe(predicted_pose)) {
    result.num_keyframes = numKeyframes();
    result.num_imu_edges = numImuEdges();
    return result;
  }

  Keyframe keyframe;
  keyframe.raw_pose = predicted_pose;
  keyframe.optimized_pose = predicted_pose;
  if (params_.enable_visual_factors) {
    keyframe.observations = matchObservations(predicted_pose, visual_observations);
  }

  const int current_index = static_cast<int>(keyframes_.size());
  keyframes_.push_back(std::move(keyframe));
  result.keyframe_added = true;
  result.num_visual_factors =
      static_cast<int>(keyframes_.back().observations.size());
  result.visual_used =
      params_.enable_visual_factors &&
      result.num_visual_factors >= params_.min_visual_observations;

  if (params_.enable_imu_factors && current_index > 0) {
    const Keyframe& previous = keyframes_[static_cast<size_t>(current_index - 1)];
    ImuEdge edge;
    edge.from = current_index - 1;
    edge.to = current_index;
    edge.delta_q =
        (previous.raw_pose.q.conjugate() * predicted_pose.q).normalized();
    edge.delta_t = previous.raw_pose.q.conjugate() *
                   (predicted_pose.t - previous.raw_pose.t);
    imu_edges_.push_back(edge);
  }

  pruneOldestKeyframe();

  if ((params_.optimize_every_n_keyframes > 0 &&
       numKeyframes() % params_.optimize_every_n_keyframes == 0) ||
      result.visual_used) {
    optimizePoseGraph();
  }

  const PoseState& optimized = keyframes_.back().optimized_pose;
  result.q_w_curr = optimized.q;
  result.t_w_curr = optimized.t;
  result.v_w_curr = optimized.v;
  result.num_keyframes = numKeyframes();
  result.num_imu_edges = numImuEdges();
  return result;
}

Eigen::Matrix4d VinsFusion::latestOptimizedPoseMatrix() const {
  if (keyframes_.empty()) {
    return Eigen::Matrix4d::Identity();
  }
  return poseToMatrix(keyframes_.back().optimized_pose);
}

void VinsFusion::clear() {
  keyframes_.clear();
  imu_edges_.clear();
  current_raw_pose_ = PoseState();
  initialized_ = false;
  valid_frame_count_ = 0;
}

bool VinsFusion::shouldCreateKeyframe(const PoseState& pose) const {
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

VinsFusion::PoseState VinsFusion::predictRawPose(
    const std::vector<imu_preintegration::ImuSample>& imu_samples,
    bool* imu_used) const {
  PoseState predicted = current_raw_pose_;
  if (imu_used != nullptr) {
    *imu_used = false;
  }

  if (!params_.enable_imu_factors || imu_samples.size() < 2) {
    return predicted;
  }

  imu_preintegration::ImuPreintegrator preintegrator(params_.imu_noise);
  preintegrator.integrateBatch(imu_samples);
  if (preintegrator.result().delta_t <= 0.0) {
    return predicted;
  }

  Eigen::Matrix3d R_j = Eigen::Matrix3d::Identity();
  Eigen::Vector3d v_j = Eigen::Vector3d::Zero();
  Eigen::Vector3d p_j = Eigen::Vector3d::Zero();
  preintegrator.predict(current_raw_pose_.q.toRotationMatrix(), current_raw_pose_.v,
                        current_raw_pose_.t, params_.gravity, R_j, v_j, p_j);

  predicted.q = Eigen::Quaterniond(R_j).normalized();
  predicted.t = p_j;
  predicted.v = v_j;
  if (imu_used != nullptr) {
    *imu_used = true;
  }
  return predicted;
}

std::vector<VinsFusion::MatchedObservation> VinsFusion::matchObservations(
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

bool VinsFusion::projectLandmark(const PoseState& pose,
                                 const Eigen::Vector3d& landmark_world,
                                 Eigen::Vector2d* pixel) const {
  const Eigen::Quaterniond q_wc =
      pose.q * Eigen::Quaterniond(params_.body_R_camera);
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

void VinsFusion::optimizePoseGraph() {
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

  for (const auto& edge : imu_edges_) {
    problem.AddResidualBlock(
        ImuPoseCost::Create(edge.delta_q, edge.delta_t,
                            params_.imu_rotation_weight,
                            params_.imu_translation_weight),
        nullptr, q_params[edge.from].data(), t_params[edge.from].data(),
        q_params[edge.to].data(), t_params[edge.to].data());
  }

  if (params_.enable_visual_factors) {
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
    if (i == 0) {
      keyframes_[i].optimized_pose.v = params_.initial_velocity;
    } else {
      const Eigen::Vector3d delta =
          keyframes_[i].optimized_pose.t - keyframes_[i - 1].optimized_pose.t;
      keyframes_[i].optimized_pose.v = 10.0 * delta;
    }
  }
}

void VinsFusion::pruneOldestKeyframe() {
  while (static_cast<int>(keyframes_.size()) > params_.max_keyframes) {
    keyframes_.erase(keyframes_.begin());

    std::vector<ImuEdge> kept_edges;
    kept_edges.reserve(imu_edges_.size());
    for (const auto& edge : imu_edges_) {
      if (edge.from == 0 || edge.to == 0) {
        continue;
      }
      ImuEdge shifted = edge;
      shifted.from -= 1;
      shifted.to -= 1;
      kept_edges.push_back(shifted);
    }
    imu_edges_.swap(kept_edges);
  }
}

Eigen::Matrix4d VinsFusion::poseToMatrix(const PoseState& pose) {
  Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();
  transform.block<3, 3>(0, 0) = pose.q.normalized().toRotationMatrix();
  transform.block<3, 1>(0, 3) = pose.t;
  return transform;
}

double VinsFusion::rotationAngle(const Eigen::Quaterniond& q) {
  const double clamped = std::clamp(std::abs(q.normalized().w()), 0.0, 1.0);
  return 2.0 * std::acos(clamped);
}

}  // namespace vins_fusion
}  // namespace localization_zoo
