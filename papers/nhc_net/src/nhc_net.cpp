#include "nhc_net/nhc_net.h"

#include <cmath>

namespace localization_zoo {
namespace nhc_net {
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

}  // namespace

NhcNetPipeline::NhcNetPipeline(const NhcNetParams& params) : params_(params) {}

bool NhcNetPipeline::loadWeights(std::string* error) {
  weights_loaded_ = loadVmscCnnJson(params_.weights_path, &model_, error);
  if (weights_loaded_) {
    params_.window_size = model_.window_size;
    params_.channels = model_.channels;
  }
  return weights_loaded_;
}

void NhcNetPipeline::reset() {
  R_ = Eigen::Matrix3d::Identity();
  v_.setZero();
  p_.setZero();
  last_stamp_ = 0.0;
  initialized_ = false;
  gyro_window_.clear();
  accel_window_.clear();
}

void NhcNetPipeline::setInitialPose(const Eigen::Matrix3d& R,
                                    const Eigen::Vector3d& p) {
  R_ = R;
  p_ = p;
  v_.setZero();
  initialized_ = true;
}

Eigen::Vector3d NhcNetPipeline::bodyForwardAxis() const {
  Eigen::Vector3d axis = Eigen::Vector3d::Zero();
  axis[params_.forward_axis] = 1.0;
  return axis;
}

void NhcNetPipeline::pushWindow(const Eigen::Vector3d& gyro,
                                const Eigen::Vector3d& accel) {
  gyro_window_.push_back(gyro);
  accel_window_.push_back(accel);
  while (static_cast<int>(gyro_window_.size()) > params_.window_size) {
    gyro_window_.pop_front();
    accel_window_.pop_front();
  }
}

VmscForwardResult NhcNetPipeline::infer() const {
  if (!weights_loaded_ ||
      static_cast<int>(gyro_window_.size()) < params_.window_size) {
    return {};
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
  return forwardVmsc(model_, window);
}

NhcNetStepStats NhcNetPipeline::processImu(const ImuReading& imu) {
  NhcNetStepStats stats;
  if (!initialized_) {
    last_stamp_ = imu.stamp;
    initialized_ = true;
    return stats;
  }

  const double dt = std::max(1e-4, imu.stamp - last_stamp_);
  last_stamp_ = imu.stamp;

  Eigen::Vector3d gyro = params_.body_to_vehicle * (imu.gyro - params_.gyro_bias);
  Eigen::Vector3d accel = params_.body_to_vehicle * (imu.accel - params_.accel_bias);
  pushWindow(gyro, accel);

  R_ = R_ * expSO3(gyro * dt);

  // Forward speed aiding (GNSS / pseudo-odometer) is out of scope; keep body
  // velocity from the previous step like OdoNet without CNN speed regression.
  Eigen::Vector3d v_body = R_.transpose() * v_;

  if (weights_loaded_ &&
      static_cast<int>(gyro_window_.size()) >= params_.window_size) {
    const VmscForwardResult pred = infer();
    stats.motion_class = pred.motion_class;
    stats.lateral_target = pred.lateral_vel;
    stats.vertical_target = pred.vertical_vel;
    stats.class_confidence =
        pred.class_probs.empty() ? 0.0 : pred.class_probs[static_cast<size_t>(pred.motion_class)];

    const double gain =
        params_.adaptive_gain
            ? params_.nhc_gain * std::max(0.2, stats.class_confidence)
            : params_.nhc_gain;
    const Eigen::Vector3d fwd = bodyForwardAxis();
    const double v_forward = v_body.dot(fwd);

    Eigen::Vector3d target = v_forward * fwd;
    if (params_.forward_axis == 0) {
      target.y() = stats.lateral_target;
      target.z() = stats.vertical_target;
    } else if (params_.forward_axis == 1) {
      target.x() = stats.lateral_target;
      target.z() = stats.vertical_target;
    } else {
      target.x() = stats.lateral_target;
      target.y() = stats.vertical_target;
    }

    const double alpha = std::min(1.0, gain * dt);
    v_body = (1.0 - alpha) * v_body + alpha * target;

    if (params_.enable_zupt && pred.motion_class == params_.stop_class_id) {
      v_body.setZero();
      stats.zupt_active = true;
    }
  }

  v_ = R_ * v_body;
  p_ += v_ * dt;
  return stats;
}

Eigen::Matrix4d NhcNetPipeline::pose() const {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = R_;
  T.block<3, 1>(0, 3) = p_;
  return T;
}

std::vector<Eigen::Matrix4d> integrateImuTrajectory(
    const std::vector<ImuReading>& imu_samples,
    const std::vector<double>& query_stamps, const NhcNetParams& params,
    long* zupt_frames, long* cnn_frames, std::string* error) {
  NhcNetPipeline pipeline(params);
  if (!pipeline.loadWeights(error)) return {};

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
      if (stats.class_confidence > 0.0) ++cnn;
      ++imu_idx;
    }
    poses.push_back(pipeline.pose());
  }

  if (zupt_frames) *zupt_frames = zupt;
  if (cnn_frames) *cnn_frames = cnn;
  return poses;
}

}  // namespace nhc_net
}  // namespace localization_zoo
