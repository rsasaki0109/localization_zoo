#include "clins/clins.h"

#include <algorithm>

namespace localization_zoo {
namespace clins {

CLINSParams::CLINSParams() {
  registration.max_iterations = 12;
  registration.ceres_max_iterations = 4;
  registration.planarity_threshold = 0.1;
  registration.imu_rotation_weight = 4.0;
  registration.imu_translation_weight = 2.0;
  registration.velocity_prior_weight = 0.1;
  registration.estimate_imu_bias = true;
  registration.fixed_lag_state_window = 4;
  registration.fixed_lag_gyro_bias_weight_scale = 0.25;
  registration.fixed_lag_accel_bias_weight_scale = 0.25;
}

CLINS::CLINS(const CLINSParams& params)
    : params_(params), registration_(params.registration) {}

double CLINS::rotationAngle(const Eigen::Quaterniond& quat) {
  const double clamped = std::clamp(std::abs(quat.normalized().w()), 0.0, 1.0);
  return 2.0 * std::acos(clamped);
}

std::vector<Eigen::Vector3d> CLINS::transformPoints(
    const std::vector<ct_icp::TimedPoint>& timed_points,
    const ct_icp::TrajectoryFrame& frame) const {
  std::vector<Eigen::Vector3d> world_points;
  world_points.reserve(timed_points.size());
  for (const auto& point : timed_points) {
    world_points.push_back(frame.transformPoint(point.raw_point, point.timestamp));
  }
  return world_points;
}

bool CLINS::shouldAddKeyframe(const ct_icp::SE3& pose) const {
  if (!initialized_) {
    return true;
  }

  const double translation = (pose.trans - last_keyframe_pose_.trans).norm();
  const double rotation =
      rotationAngle(last_keyframe_pose_.quat.conjugate() * pose.quat);

  if (params_.keyframe_stride > 0 &&
      frame_count_ % params_.keyframe_stride == 0) {
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

CLINSResult CLINS::process(
    const std::vector<ct_icp::TimedPoint>& timed_points,
    const std::vector<imu_preintegration::ImuSample>& imu_samples) {
  CLINSResult result;
  result.state = state_;
  result.initialized = initialized_;
  result.num_frames = frame_count_;
  result.map_size = registration_.mapSize();

  if (timed_points.empty()) {
    return result;
  }

  if (!initialized_) {
    state_.frame.begin_pose = ct_icp::SE3();
    state_.frame.end_pose = ct_icp::SE3();
    state_.frame.begin_timestamp = 0.0;
    state_.frame.end_timestamp = 1.0;
    state_.begin_velocity = Eigen::Vector3d::Zero();
    state_.end_velocity = Eigen::Vector3d::Zero();
    registration_.addPointsToMap(transformPoints(timed_points, state_.frame));
    last_keyframe_pose_ = state_.frame.end_pose;
    initialized_ = true;
    frame_count_ = 1;

    result.state = state_;
    result.initialized = true;
    result.valid = true;
    result.keyframe_added = true;
    result.num_frames = frame_count_;
    result.map_size = registration_.mapSize();
    return result;
  }

  ct_lio::CTLIOState initial_state = state_;
  initial_state.frame.begin_pose = state_.frame.end_pose;
  initial_state.frame.end_pose = state_.frame.end_pose;
  initial_state.frame.begin_timestamp = 0.0;
  initial_state.frame.end_timestamp = 1.0;
  initial_state.begin_velocity = state_.end_velocity;
  initial_state.end_velocity = state_.end_velocity;

  const ct_lio::CTLIOResult registration_result =
      registration_.registerFrame(timed_points, initial_state, imu_samples);
  state_ = registration_result.state;
  result.registration_result = registration_result;
  result.state = state_;
  result.initialized = true;
  result.valid =
      registration_result.converged || registration_result.num_iterations > 0;
  frame_count_++;

  const double delta_t = registration_result.preintegration.delta_t > 0.0
                             ? registration_result.preintegration.delta_t
                             : params_.scan_duration;
  if (delta_t > 0.0) {
    state_.end_velocity =
        (state_.frame.end_pose.trans - state_.frame.begin_pose.trans) / delta_t;
    result.state.end_velocity = state_.end_velocity;
  }

  if (shouldAddKeyframe(state_.frame.end_pose)) {
    registration_.addPointsToMap(transformPoints(timed_points, state_.frame));
    last_keyframe_pose_ = state_.frame.end_pose;
    result.keyframe_added = true;
  }

  result.num_frames = frame_count_;
  result.map_size = registration_.mapSize();
  return result;
}

void CLINS::clear() {
  registration_ = ct_lio::CTLIORegistration(params_.registration);
  state_ = ct_lio::CTLIOState();
  last_keyframe_pose_ = ct_icp::SE3();
  initialized_ = false;
  frame_count_ = 0;
}

}  // namespace clins
}  // namespace localization_zoo
