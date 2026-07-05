#include "imu_dead_reckoning/imu_dead_reckoning.h"

#include <cmath>
#include <cstdio>

namespace localization_zoo {
namespace imu_dead_reckoning {
namespace {

constexpr double kStandardGravity = 9.80665;

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

ImuDeadReckoningPipeline::ImuDeadReckoningPipeline(
    const ImuDeadReckoningParams& params)
    : params_(params) {}

void ImuDeadReckoningPipeline::reset() {
  R_ = Eigen::Matrix3d::Identity();
  v_.setZero();
  p_.setZero();
  gyro_bias_.setZero();
  accel_bias_.setZero();
  g_w_.setZero();
  gravity_magnitude_ = kStandardGravity;
  static_initialized_ = false;
  static_buffer_.clear();
  warning_.clear();
  has_last_stamp_ = false;
  last_stamp_ = 0.0;
  has_prev_gyro_ = false;
  prev_gyro_corrected_.setZero();
  gyro_norm_window_.clear();
  accel_norm_window_.clear();
}

void ImuDeadReckoningPipeline::finalizeStaticInit(
    const std::vector<ImuReading>& window) {
  if (window.empty()) {
    static_initialized_ = true;
    return;
  }

  Eigen::Vector3d gyro_sum = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel_sum = Eigen::Vector3d::Zero();
  for (const auto& r : window) {
    gyro_sum += r.gyro;
    accel_sum += r.accel;
  }
  const double n = static_cast<double>(window.size());
  const Eigen::Vector3d gyro_mean = gyro_sum / n;
  const Eigen::Vector3d accel_mean = accel_sum / n;

  gyro_bias_ = params_.estimate_gyro_bias ? gyro_mean : Eigen::Vector3d::Zero();

  double gyro_var_sum = 0.0;
  for (const auto& r : window) {
    gyro_var_sum += (r.gyro - gyro_mean).squaredNorm();
  }
  const double gyro_std = std::sqrt(gyro_var_sum / n);
  if (gyro_std > params_.static_gyro_std_gate) {
    char buf[192];
    std::snprintf(buf, sizeof(buf),
                  "static-init gyro std %.4f rad/s exceeds gate %.4f rad/s "
                  "(non-static IMU at start?)",
                  gyro_std, params_.static_gyro_std_gate);
    warning_ = buf;
  }

  const double accel_mean_norm = accel_mean.norm();
  gravity_magnitude_ = params_.gravity_from_static_norm ? accel_mean_norm
                                                        : kStandardGravity;
  const Eigen::Vector3d accel_dir =
      accel_mean_norm > 1e-9 ? (accel_mean / accel_mean_norm).eval()
                             : Eigen::Vector3d::UnitZ();

  // Initial roll/pitch aligns the measured static specific-force direction
  // with the world +z axis; yaw is left at zero (no magnetometer). The
  // shortest-arc rotation between two vectors has its axis in the plane
  // orthogonal to both, i.e. horizontal here, so it never introduces yaw.
  R_ = Eigen::Quaterniond::FromTwoVectors(accel_dir, Eigen::Vector3d::UnitZ())
           .toRotationMatrix();

  // g_w is derived (not hard-coded to +z/-z) so both IMU gravity sign
  // conventions work: it is defined as exactly the quantity that makes the
  // static-window specific force cancel, R_ * accel_mean - g_w = 0.
  g_w_ = R_ * (accel_dir * gravity_magnitude_);

  if (params_.estimate_accel_bias) {
    // Residual after roll/pitch alignment against nominal vertical gravity.
    const Eigen::Vector3d expected_static =
        R_.transpose() * (Eigen::Vector3d::UnitZ() * gravity_magnitude_);
    accel_bias_ = accel_mean - expected_static;
  } else {
    accel_bias_.setZero();
  }

  v_.setZero();
  p_.setZero();
  static_initialized_ = true;
}

Eigen::Vector3d ImuDeadReckoningPipeline::bodyForwardAxis() const {
  Eigen::Vector3d axis = Eigen::Vector3d::Zero();
  if (params_.forward_axis >= 0 && params_.forward_axis < 3) {
    axis[params_.forward_axis] = 1.0;
  } else {
    axis.x() = 1.0;
  }
  return axis;
}

void ImuDeadReckoningPipeline::applyNhc(Eigen::Vector3d* v_body, double dt,
                                          bool* nhc_active) const {
  if (!params_.enable_nhc || v_body == nullptr) return;
  const Eigen::Vector3d fwd = bodyForwardAxis();
  const double v_forward = v_body->dot(fwd);
  const Eigen::Vector3d target = v_forward * fwd;
  if (params_.nhc_gain <= 0.0) {
    if ((target - *v_body).norm() > 1e-12) {
      *v_body = target;
      if (nhc_active) *nhc_active = true;
    }
    return;
  }
  const double alpha = std::min(1.0, params_.nhc_gain * dt);
  const Eigen::Vector3d updated = (1.0 - alpha) * (*v_body) + alpha * target;
  if ((updated - *v_body).norm() > 1e-12) {
    *v_body = updated;
    if (nhc_active) *nhc_active = true;
  }
}

void ImuDeadReckoningPipeline::initializeStatic(
    const std::vector<ImuReading>& static_window) {
  finalizeStaticInit(static_window);
  if (!static_window.empty()) {
    has_last_stamp_ = true;
    last_stamp_ = static_window.back().stamp;
    has_prev_gyro_ = true;
    prev_gyro_corrected_ = static_window.back().gyro - gyro_bias_;
  }
}

ImuDeadReckoningStepStats ImuDeadReckoningPipeline::processImu(
    const ImuReading& imu) {
  ImuDeadReckoningStepStats stats;

  if (!static_initialized_) {
    static_buffer_.push_back(imu);
    const double elapsed =
        static_buffer_.back().stamp - static_buffer_.front().stamp;
    if (elapsed >= params_.static_init_duration_s) {
      const std::vector<ImuReading> window = static_buffer_;
      static_buffer_.clear();
      initializeStatic(window);
    }
    return stats;
  }

  if (!has_last_stamp_) {
    has_last_stamp_ = true;
    last_stamp_ = imu.stamp;
    has_prev_gyro_ = true;
    prev_gyro_corrected_ = imu.gyro - gyro_bias_;
    return stats;
  }

  const double raw_dt = imu.stamp - last_stamp_;
  if (raw_dt <= 0.0) {
    // Non-monotonic timestamp: drop the sample, do not disturb state.
    return stats;
  }
  const double dt = std::min(raw_dt, params_.max_dt);
  last_stamp_ = imu.stamp;

  const Eigen::Vector3d gyro_corrected = imu.gyro - gyro_bias_;
  const Eigen::Vector3d gyro_for_integration =
      params_.midpoint_integration && has_prev_gyro_
          ? Eigen::Vector3d(0.5 * (gyro_corrected + prev_gyro_corrected_))
          : gyro_corrected;
  prev_gyro_corrected_ = gyro_corrected;
  has_prev_gyro_ = true;

  R_ = R_ * expSO3(gyro_for_integration * dt);

  const Eigen::Vector3d accel_corrected = imu.accel - accel_bias_;
  const Eigen::Vector3d accel_w = R_ * accel_corrected - g_w_;
  p_ += v_ * dt + 0.5 * accel_w * dt * dt;
  v_ += accel_w * dt;

  if (params_.enable_zupt) {
    gyro_norm_window_.push_back(gyro_corrected.norm());
    accel_norm_window_.push_back(imu.accel.norm());
    while (static_cast<int>(gyro_norm_window_.size()) > params_.zupt_window) {
      gyro_norm_window_.pop_front();
      accel_norm_window_.pop_front();
    }
    if (static_cast<int>(gyro_norm_window_.size()) == params_.zupt_window) {
      double gyro_sum = 0.0;
      double accel_sum = 0.0;
      for (double g : gyro_norm_window_) gyro_sum += g;
      for (double a : accel_norm_window_) accel_sum += a;
      const double gyro_mean = gyro_sum / params_.zupt_window;
      const double accel_mean = accel_sum / params_.zupt_window;
      const bool still = gyro_mean < params_.zupt_gyro_threshold &&
                          std::abs(accel_mean - gravity_magnitude_) <
                              params_.zupt_accel_tolerance;
      if (still) {
        v_.setZero();
        stats.zupt_active = true;
      }
    }
  }

  if (params_.enable_nhc && !stats.zupt_active) {
    Eigen::Vector3d v_body = R_.transpose() * v_;
    applyNhc(&v_body, dt, &stats.nhc_active);
    v_ = R_ * v_body;
  }

  return stats;
}

Eigen::Matrix4d ImuDeadReckoningPipeline::pose() const {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = R_;
  T.block<3, 1>(0, 3) = p_;
  return T;
}

Eigen::Vector3d ImuDeadReckoningPipeline::velocity() const { return v_; }

std::vector<Eigen::Matrix4d> integrateImuTrajectory(
    const std::vector<ImuReading>& imu,
    const std::vector<double>& frame_timestamps,
    const ImuDeadReckoningParams& params, long* zupt_frames,
    std::string* error) {
  if (imu.empty()) {
    if (error) *error = "no IMU samples provided.";
    return {};
  }

  ImuDeadReckoningPipeline pipeline(params);
  std::vector<Eigen::Matrix4d> poses;
  poses.reserve(frame_timestamps.size());
  size_t imu_idx = 0;
  long zupt = 0;

  for (double q : frame_timestamps) {
    while (imu_idx < imu.size() && imu[imu_idx].stamp <= q + 1e-9) {
      const auto stats = pipeline.processImu(imu[imu_idx]);
      if (stats.zupt_active) ++zupt;
      ++imu_idx;
    }
    poses.push_back(pipeline.pose());
  }

  if (zupt_frames) *zupt_frames = zupt;
  // Surface a non-fatal static-init warning (if any) to the caller even on
  // success; the runner folds this into its note string.
  if (error) *error = pipeline.warning();
  return poses;
}

}  // namespace imu_dead_reckoning
}  // namespace localization_zoo
