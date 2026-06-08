#include "nn_zupt/nn_zupt.h"

#include <cmath>

namespace localization_zoo {
namespace nn_zupt {
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

NnZuptPipeline::NnZuptPipeline(const NnZuptParams& params) : params_(params) {}

bool NnZuptPipeline::loadWeights(std::string* error) {
  weights_loaded_ = loadZuptCnnJson(params_.weights_path, &model_, error);
  if (weights_loaded_) {
    params_.window_size = model_.window_size;
    params_.channels = model_.channels;
  }
  return weights_loaded_;
}

void NnZuptPipeline::reset() {
  R_ = Eigen::Matrix3d::Identity();
  v_.setZero();
  p_.setZero();
  last_stamp_ = 0.0;
  initialized_ = false;
  gyro_window_.clear();
  accel_window_.clear();
}

void NnZuptPipeline::setInitialPose(const Eigen::Matrix3d& R,
                                    const Eigen::Vector3d& p) {
  R_ = R;
  p_ = p;
  v_.setZero();
  initialized_ = true;
}

Eigen::Vector3d NnZuptPipeline::bodyForwardAxis() const {
  Eigen::Vector3d axis = Eigen::Vector3d::Zero();
  axis[params_.forward_axis] = 1.0;
  return axis;
}

void NnZuptPipeline::pushWindow(const Eigen::Vector3d& gyro,
                                const Eigen::Vector3d& accel) {
  gyro_window_.push_back(gyro);
  accel_window_.push_back(accel);
  while (static_cast<int>(gyro_window_.size()) > params_.window_size) {
    gyro_window_.pop_front();
    accel_window_.pop_front();
  }
}

ZuptForwardResult NnZuptPipeline::infer() const {
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
  return forwardZuptCnn(model_, window);
}

bool NnZuptPipeline::thresholdStop(const Eigen::Vector3d& gyro,
                                   const Eigen::Vector3d& accel) const {
  const double gyro_norm = gyro.norm();
  const double accel_err = std::abs(accel.norm() - 9.80665);
  return gyro_norm < params_.gyro_stop_threshold &&
         accel_err < params_.accel_norm_tolerance;
}

void NnZuptPipeline::applyNhc(Eigen::Vector3d* v_body) const {
  if (!params_.enable_nhc || v_body == nullptr) return;
  const Eigen::Vector3d fwd = bodyForwardAxis();
  const double v_forward = v_body->dot(fwd);
  *v_body = v_forward * fwd;
}

NnZuptStepStats NnZuptPipeline::processImu(const ImuReading& imu) {
  NnZuptStepStats stats;
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

  Eigen::Vector3d v_body = R_.transpose() * v_;

  bool stop_detected = false;
  if (params_.use_threshold_detector) {
    stop_detected = thresholdStop(gyro, accel);
  } else if (weights_loaded_ &&
             static_cast<int>(gyro_window_.size()) >= params_.window_size) {
    const ZuptForwardResult pred = infer();
    stats.stop_prob = pred.stop_prob;
    stats.nn_used = true;
    stop_detected = pred.stop_prob >= params_.stop_prob_threshold;
  }

  if (!stop_detected) {
    applyNhc(&v_body);
  }

  if (params_.enable_zupt && stop_detected) {
    v_body.setZero();
    stats.zupt_active = true;
  }

  v_ = R_ * v_body;
  p_ += v_ * dt;
  return stats;
}

Eigen::Matrix4d NnZuptPipeline::pose() const {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = R_;
  T.block<3, 1>(0, 3) = p_;
  return T;
}

std::vector<Eigen::Matrix4d> integrateImuTrajectory(
    const std::vector<ImuReading>& imu_samples,
    const std::vector<double>& query_stamps, const NnZuptParams& params,
    long* zupt_frames, long* nn_frames, std::string* error) {
  NnZuptPipeline pipeline(params);
  if (!params.use_threshold_detector && !pipeline.loadWeights(error)) {
    return {};
  }

  pipeline.setInitialPose(Eigen::Matrix3d::Identity(), Eigen::Vector3d::Zero());
  std::vector<Eigen::Matrix4d> poses;
  poses.reserve(query_stamps.size());
  size_t imu_idx = 0;
  long zupt = 0;
  long nn = 0;

  for (double q : query_stamps) {
    while (imu_idx < imu_samples.size() &&
           imu_samples[imu_idx].stamp <= q + 1e-9) {
      const auto stats = pipeline.processImu(imu_samples[imu_idx]);
      if (stats.zupt_active) ++zupt;
      if (stats.nn_used) ++nn;
      ++imu_idx;
    }
    poses.push_back(pipeline.pose());
  }

  if (zupt_frames) *zupt_frames = zupt;
  if (nn_frames) *nn_frames = nn;
  return poses;
}

}  // namespace nn_zupt
}  // namespace localization_zoo
