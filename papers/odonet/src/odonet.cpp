#include "odonet/odonet.h"

#include <cmath>

namespace localization_zoo {
namespace odonet {
namespace {

Eigen::Matrix3d expSO3(const Eigen::Vector3d& w) {
  const double t = w.norm();
  if (t < 1e-10) return Eigen::Matrix3d::Identity();
  const Eigen::Vector3d a = w / t;
  Eigen::Matrix3d K;
  K << 0, -a.z(), a.y(), a.z(), 0, -a.x(), -a.y(), a.x(), 0;
  return Eigen::Matrix3d::Identity() + std::sin(t) * K +
         (1.0 - std::cos(t)) * K * K;
}

Eigen::Vector3d worldToBodyVel(const Eigen::Matrix3d& R,
                               const Eigen::Vector3d& v_world) {
  return R.transpose() * v_world;
}

Eigen::Vector3d bodyToWorldVel(const Eigen::Matrix3d& R,
                              const Eigen::Vector3d& v_body) {
  return R * v_body;
}

}  // namespace

OdoNetPipeline::OdoNetPipeline(const OdoNetParams& params) : params_(params) {
  params_.window_size = params.window_size;
  params_.channels = params.channels;
}

bool OdoNetPipeline::loadWeights(std::string* error) {
  weights_loaded_ = loadCnn1dModelJson(params_.weights_path, &model_, error);
  if (weights_loaded_) {
    params_.window_size = model_.window_size;
    params_.channels = model_.channels;
    params_.speed_scale = model_.speed_scale;
  }
  return weights_loaded_;
}

void OdoNetPipeline::reset() {
  state_ = OdoNetState{};
  gyro_window_.clear();
  accel_window_.clear();
}

void OdoNetPipeline::setInitialPose(const Eigen::Matrix3d& R,
                                    const Eigen::Vector3d& p) {
  state_.R = R;
  state_.p = p;
  state_.v.setZero();
  state_.filtered_speed = 0.0;
  state_.initialized = true;
}

Eigen::Vector3d OdoNetPipeline::bodyForwardAxis() const {
  Eigen::Vector3d axis = Eigen::Vector3d::Zero();
  axis[params_.forward_axis] = 1.0;
  return axis;
}

void OdoNetPipeline::applyNhc(Eigen::Vector3d* v_body) const {
  if (!params_.enable_nhc || v_body == nullptr) return;
  const Eigen::Vector3d fwd = bodyForwardAxis();
  const double v_forward = v_body->dot(fwd);
  *v_body = v_forward * fwd;
}

void OdoNetPipeline::pushWindowSample(const Eigen::Vector3d& gyro,
                                      const Eigen::Vector3d& accel) {
  gyro_window_.push_back(gyro);
  accel_window_.push_back(accel);
  while (static_cast<int>(gyro_window_.size()) > params_.window_size) {
    gyro_window_.pop_front();
    accel_window_.pop_front();
  }
}

double OdoNetPipeline::regressSpeed() const {
  if (!weights_loaded_ ||
      static_cast<int>(gyro_window_.size()) < params_.window_size) {
    return -1.0;
  }
  std::vector<double> window(static_cast<size_t>(params_.window_size *
                                                 params_.channels));
  for (int t = 0; t < params_.window_size; ++t) {
    const Eigen::Vector3d g = gyro_window_[static_cast<size_t>(t)];
    const Eigen::Vector3d a = accel_window_[static_cast<size_t>(t)];
    const size_t base = static_cast<size_t>(t * params_.channels);
    window[base + 0] = g.x();
    window[base + 1] = g.y();
    window[base + 2] = g.z();
    window[base + 3] = a.x();
    window[base + 4] = a.y();
    window[base + 5] = a.z();
  }
  return forwardSpeed(model_, window);
}

OdoNetStepStats OdoNetPipeline::processImu(const ImuReading& imu) {
  OdoNetStepStats stats;
  if (!state_.initialized) {
    state_.last_stamp = imu.stamp;
    state_.initialized = true;
    return stats;
  }

  const double dt = std::max(1e-4, imu.stamp - state_.last_stamp);
  state_.last_stamp = imu.stamp;

  Eigen::Vector3d gyro = imu.gyro - params_.gyro_bias;
  Eigen::Vector3d accel = imu.accel - params_.accel_bias;
  gyro = params_.body_to_vehicle * gyro;
  accel = params_.body_to_vehicle * accel;

  pushWindowSample(gyro, accel);

  state_.R = state_.R * expSO3(gyro * dt);

  if (params_.enable_cnn_speed && weights_loaded_ &&
      static_cast<int>(gyro_window_.size()) >= params_.window_size) {
    stats.regressed_speed = regressSpeed();
    stats.cnn_used = stats.regressed_speed >= 0.0;
    if (stats.cnn_used) {
      const double alpha = params_.speed_filter_alpha;
      state_.filtered_speed =
          (1.0 - alpha) * state_.filtered_speed + alpha * stats.regressed_speed;
    }
  }
  stats.filtered_speed = state_.filtered_speed;

  Eigen::Vector3d v_body = worldToBodyVel(state_.R, state_.v);
  if (stats.cnn_used) {
    v_body = stats.filtered_speed * bodyForwardAxis();
  }
  applyNhc(&v_body);

  if (params_.enable_zupt && stats.filtered_speed < params_.zupt_speed_threshold) {
    v_body.setZero();
    stats.zupt_active = true;
  }

  state_.v = bodyToWorldVel(state_.R, v_body);
  state_.p += state_.v * dt;
  return stats;
}

Eigen::Matrix4d OdoNetPipeline::pose() const {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = state_.R;
  T.block<3, 1>(0, 3) = state_.p;
  return T;
}

std::vector<Eigen::Matrix4d> integrateImuTrajectory(
    const std::vector<ImuReading>& imu_samples,
    const std::vector<double>& query_stamps, const OdoNetParams& params,
    long* zupt_frames, long* cnn_frames, std::string* error) {
  OdoNetPipeline pipeline(params);
  if (params.enable_cnn_speed && !pipeline.loadWeights(error)) {
    return {};
  }
  pipeline.setInitialPose(Eigen::Matrix3d::Identity(), Eigen::Vector3d::Zero());

  std::vector<Eigen::Matrix4d> poses;
  poses.reserve(query_stamps.size());
  size_t imu_idx = 0;
  long zupt = 0;
  long cnn = 0;

  for (double q : query_stamps) {
    while (imu_idx < imu_samples.size() &&
           imu_samples[imu_idx].stamp <= q + 1e-9) {
      const auto stats = pipeline.processImu(imu_samples[imu_idx]);
      if (stats.zupt_active) ++zupt;
      if (stats.cnn_used) ++cnn;
      ++imu_idx;
    }
    poses.push_back(pipeline.pose());
  }

  if (zupt_frames) *zupt_frames = zupt;
  if (cnn_frames) *cnn_frames = cnn;
  return poses;
}

}  // namespace odonet
}  // namespace localization_zoo
