#include "imu_dead_reckoning/imu_dead_reckoning.h"

#include <Eigen/SVD>

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

Eigen::Matrix3d skew(const Eigen::Vector3d& w) {
  Eigen::Matrix3d K;
  K << 0, -w.z(), w.y(), w.z(), 0, -w.x(), -w.y(), w.x(), 0;
  return K;
}

// Nearest rotation matrix (SVD projection onto SO(3)) to clean up the small
// non-orthogonality that explicit RK4 leaves after each step.
Eigen::Matrix3d orthonormalize(const Eigen::Matrix3d& M) {
  Eigen::JacobiSVD<Eigen::Matrix3d> svd(
      M, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Eigen::Matrix3d U = svd.matrixU();
  const Eigen::Matrix3d V = svd.matrixV();
  if ((U * V.transpose()).determinant() < 0.0) U.col(2) *= -1.0;
  return U * V.transpose();
}

// Classical RK4 for the attitude ODE dR/dt = R * skew(omega(t)), with omega
// interpolated linearly between the previous (w0) and current (w1) corrected
// gyro over the step. Result is re-orthonormalized back onto SO(3).
Eigen::Matrix3d rk4Rotation(const Eigen::Matrix3d& R0, const Eigen::Vector3d& w0,
                            const Eigen::Vector3d& w1, double dt) {
  const Eigen::Matrix3d Omega0 = skew(w0);
  const Eigen::Matrix3d OmegaMid = skew(0.5 * (w0 + w1));
  const Eigen::Matrix3d Omega1 = skew(w1);
  const Eigen::Matrix3d k1 = R0 * Omega0;
  const Eigen::Matrix3d k2 = (R0 + 0.5 * dt * k1) * OmegaMid;
  const Eigen::Matrix3d k3 = (R0 + 0.5 * dt * k2) * OmegaMid;
  const Eigen::Matrix3d k4 = (R0 + dt * k3) * Omega1;
  const Eigen::Matrix3d R =
      R0 + (dt / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
  return orthonormalize(R);
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
  const Eigen::Vector3d accel_dir =
      accel_mean_norm > 1e-9 ? (accel_mean / accel_mean_norm).eval()
                             : Eigen::Vector3d::UnitZ();

  // With accel-bias estimation on, the deviation of the static accel norm
  // from a known gravity magnitude IS the bias signal, so gravity must come
  // from a fixed reference (standard gravity) rather than the static norm --
  // otherwise g_w absorbs the whole static mean and the residual is zero by
  // construction (accel_mean - accel_dir * accel_mean_norm == 0).
  const bool use_static_norm =
      params_.gravity_from_static_norm && !params_.estimate_accel_bias;
  gravity_magnitude_ = use_static_norm ? accel_mean_norm : kStandardGravity;

  // Initial roll/pitch aligns the measured static specific-force direction
  // with the world +z axis; yaw is left at zero (no magnetometer). The
  // shortest-arc rotation between two vectors has its axis in the plane
  // orthogonal to both, i.e. horizontal here, so it never introduces yaw.
  R_ = Eigen::Quaterniond::FromTwoVectors(accel_dir, Eigen::Vector3d::UnitZ())
           .toRotationMatrix();

  // g_w is derived (not hard-coded to +z/-z) so both IMU gravity sign
  // conventions work: it is defined as exactly the quantity that makes the
  // bias-corrected static specific force cancel, R_ * (accel_mean - b) = g_w.
  g_w_ = R_ * (accel_dir * gravity_magnitude_);

  if (params_.estimate_accel_bias) {
    // Residual of the static mean against the fixed-magnitude gravity vector
    // rotated into the body frame: any leftover is attributed to accel bias.
    const Eigen::Vector3d expected_static = accel_dir * gravity_magnitude_;
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
  if (params_.rk4_integration && has_prev_gyro_) {
    R_ = rk4Rotation(R_, prev_gyro_corrected_, gyro_corrected, dt);
  } else {
    const Eigen::Vector3d gyro_for_integration =
        params_.midpoint_integration && has_prev_gyro_
            ? Eigen::Vector3d(0.5 * (gyro_corrected + prev_gyro_corrected_))
            : gyro_corrected;
    R_ = R_ * expSO3(gyro_for_integration * dt);
  }
  prev_gyro_corrected_ = gyro_corrected;
  has_prev_gyro_ = true;

  const Eigen::Vector3d accel_corrected = imu.accel - accel_bias_;
  const Eigen::Vector3d accel_w = R_ * accel_corrected - g_w_;
  p_ += v_ * dt + 0.5 * accel_w * dt * dt;
  v_ += accel_w * dt;

  if (params_.enable_zupt) {
    gyro_norm_window_.push_back(gyro_corrected.norm());
    // Bias-corrected norm keeps the stationary gate consistent with
    // gravity_magnitude_: with accel-bias estimation on, gravity is fixed to
    // standard gravity and the corrected static norm equals it by
    // construction; with estimation off the bias is zero and this reduces to
    // the raw norm as before.
    accel_norm_window_.push_back(accel_corrected.norm());
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
