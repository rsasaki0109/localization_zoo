#include "imu_dead_reckoning/imu_dead_reckoning.h"

#include <Eigen/Dense>
#include <Eigen/SVD>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace localization_zoo {
namespace imu_dead_reckoning {
namespace {

constexpr double kStandardGravity = 9.80665;

// Error-state block offsets in the 15-vector (dp, dv, dtheta, db_g, db_a).
constexpr int kIP = 0;   // position error.
constexpr int kIV = 3;   // velocity error.
constexpr int kITh = 6;  // attitude error (body frame).
constexpr int kIBg = 9;  // gyro-bias error.
constexpr int kIBa = 12; // accel-bias error.

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
  static_gate_tripped_ = false;
  static_buffer_.clear();
  warning_.clear();
  has_last_stamp_ = false;
  last_stamp_ = 0.0;
  has_prev_gyro_ = false;
  prev_gyro_corrected_.setZero();
  gyro_norm_window_.clear();
  accel_norm_window_.clear();
  P_.setZero();
  eskf_initialized_ = false;
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

  // Window noise statistics: gyro std (rotational motion) and the std of the
  // accel norm (road vibration / longitudinal speed changes). Both are near
  // zero for a truly parked IMU. Note neither can flag a constant-velocity
  // cruise -- that is fundamentally unobservable from IMU alone.
  double gyro_var_sum = 0.0;
  double accel_norm_mean = 0.0;
  for (const auto& r : window) {
    gyro_var_sum += (r.gyro - gyro_mean).squaredNorm();
    accel_norm_mean += r.accel.norm();
  }
  const double gyro_std = std::sqrt(gyro_var_sum / n);
  accel_norm_mean /= n;
  double accel_norm_var = 0.0;
  for (const auto& r : window) {
    const double d = r.accel.norm() - accel_norm_mean;
    accel_norm_var += d * d;
  }
  const double accel_norm_std = std::sqrt(accel_norm_var / n);

  // Static-init quality gate: decide whether the window's mean is trustworthy
  // as a bias estimate. Off by default -> always trust (original behaviour).
  const int n_samples = static_cast<int>(window.size());
  const bool gate_too_few =
      params_.motion_gated_static_init && n_samples < params_.min_static_samples;
  const bool gate_gyro_hot =
      params_.motion_gated_static_init && gyro_std > params_.static_gyro_std_gate;
  const bool gate_accel_hot = params_.motion_gated_static_init &&
                              accel_norm_std > params_.static_accel_std_gate;
  static_gate_tripped_ = gate_too_few || gate_gyro_hot || gate_accel_hot;
  const bool trust_bias = !static_gate_tripped_;

  gyro_bias_ = (params_.estimate_gyro_bias && trust_bias)
                   ? gyro_mean
                   : Eigen::Vector3d::Zero();

  if (static_gate_tripped_) {
    char buf[256];
    std::snprintf(
        buf, sizeof(buf),
        "static-init quality gate tripped (%s%s%ssamples=%d gyro_std=%.4f "
        "accel_std=%.3f): bias estimation auto-skipped",
        gate_too_few ? "too-few-samples " : "", gate_gyro_hot ? "gyro-hot " : "",
        gate_accel_hot ? "accel-hot " : "", n_samples, gyro_std, accel_norm_std);
    warning_ = buf;
  } else if (gyro_std > params_.static_gyro_std_gate) {
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
  // When the quality gate distrusts the window, accel-bias estimation is
  // skipped below, so gravity falls back to the static norm just like the
  // estimate-off path (|a| ~= g holds even at constant velocity).
  const bool estimate_accel_bias_effective =
      params_.estimate_accel_bias && trust_bias;
  const bool use_static_norm =
      params_.gravity_from_static_norm && !estimate_accel_bias_effective;
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

  if (estimate_accel_bias_effective) {
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

  if (params_.enable_eskf) initEskf();
}

void ImuDeadReckoningPipeline::initEskf() {
  P_.setZero();
  const auto sq = [](double s) { return s * s; };
  P_.block<3, 3>(kIP, kIP) =
      sq(params_.eskf_init_sigma_pos) * Eigen::Matrix3d::Identity();
  P_.block<3, 3>(kIV, kIV) =
      sq(params_.eskf_init_sigma_vel) * Eigen::Matrix3d::Identity();
  P_.block<3, 3>(kITh, kITh) =
      sq(params_.eskf_init_sigma_att) * Eigen::Matrix3d::Identity();
  P_.block<3, 3>(kIBg, kIBg) =
      sq(params_.eskf_init_sigma_bias_gyro) * Eigen::Matrix3d::Identity();
  P_.block<3, 3>(kIBa, kIBa) =
      sq(params_.eskf_init_sigma_bias_accel) * Eigen::Matrix3d::Identity();
  eskf_initialized_ = true;
}

// First-order error-state propagation with body-frame attitude error
// (R_true = R_nom * Exp(dtheta)). The nominal state (R_, v_, p_) is integrated
// by the shared strapdown code; here only the covariance advances.
void ImuDeadReckoningPipeline::eskfPropagate(const Eigen::Vector3d& gyro_corrected,
                                             const Eigen::Vector3d& accel_corrected,
                                             double dt) {
  Eigen::Matrix<double, 15, 15> F =
      Eigen::Matrix<double, 15, 15>::Identity();
  // dp/dv
  F.block<3, 3>(kIP, kIV) = Eigen::Matrix3d::Identity() * dt;
  // dv/dtheta = -R [a]_x dt ; dv/db_a = -R dt
  F.block<3, 3>(kIV, kITh) = -R_ * skew(accel_corrected) * dt;
  F.block<3, 3>(kIV, kIBa) = -R_ * dt;
  // dtheta/dtheta = I - [w]_x dt ; dtheta/db_g = -I dt
  F.block<3, 3>(kITh, kITh) =
      Eigen::Matrix3d::Identity() - skew(gyro_corrected) * dt;
  F.block<3, 3>(kITh, kIBg) = -Eigen::Matrix3d::Identity() * dt;

  // Process noise: white accel/gyro drive dv/dtheta; biases random-walk.
  const auto sq = [](double s) { return s * s; };
  Eigen::Matrix<double, 15, 15> Q =
      Eigen::Matrix<double, 15, 15>::Zero();
  Q.block<3, 3>(kIV, kIV) =
      sq(params_.eskf_sigma_accel) * dt * dt * Eigen::Matrix3d::Identity();
  Q.block<3, 3>(kITh, kITh) =
      sq(params_.eskf_sigma_gyro) * dt * dt * Eigen::Matrix3d::Identity();
  Q.block<3, 3>(kIBg, kIBg) =
      sq(params_.eskf_sigma_bias_gyro) * dt * Eigen::Matrix3d::Identity();
  Q.block<3, 3>(kIBa, kIBa) =
      sq(params_.eskf_sigma_bias_accel) * dt * Eigen::Matrix3d::Identity();

  P_ = F * P_ * F.transpose() + Q;
  P_ = 0.5 * (P_ + P_.transpose());  // keep symmetric.
}

void ImuDeadReckoningPipeline::eskfUpdate(const Eigen::MatrixXd& H,
                                          const Eigen::VectorXd& innovation,
                                          const Eigen::MatrixXd& meas_cov) {
  const Eigen::MatrixXd S = H * P_ * H.transpose() + meas_cov;
  const Eigen::MatrixXd K = P_ * H.transpose() * S.inverse();
  const Eigen::Matrix<double, 15, 1> dx = K * innovation;

  // Inject the error into the nominal state.
  p_ += dx.segment<3>(kIP);
  v_ += dx.segment<3>(kIV);
  R_ = R_ * expSO3(dx.segment<3>(kITh));  // body-frame error injection.
  R_ = orthonormalize(R_);
  gyro_bias_ += dx.segment<3>(kIBg);
  accel_bias_ += dx.segment<3>(kIBa);

  // Joseph-form covariance update (numerically stable, stays PSD).
  const Eigen::Matrix<double, 15, 15> I =
      Eigen::Matrix<double, 15, 15>::Identity();
  const Eigen::Matrix<double, 15, 15> IKH = I - K * H;
  P_ = IKH * P_ * IKH.transpose() + K * meas_cov * K.transpose();
  P_ = 0.5 * (P_ + P_.transpose());
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

  // Shared stationary detection: the windowed gyro/accel-norm gate that ZUPT,
  // leveling and ZARU all key off. Computed once, consumed by either the hard
  // aids or the ESKF measurement updates below.
  const bool needs_gate = params_.enable_zupt || params_.enable_leveling ||
                          params_.enable_zaru;
  bool still = false;
  if (needs_gate) {
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
      still = gyro_mean < params_.zupt_gyro_threshold &&
              std::abs(accel_mean - gravity_magnitude_) <
                  params_.zupt_accel_tolerance;
    }
  }

  if (!params_.enable_eskf) {
    // --- Hard-aid path (default): direct resets / projections. ---
    if (still) {
      if (params_.enable_zupt) {
        v_.setZero();
        stats.zupt_active = true;
      }
      if (params_.enable_leveling) {
        // The bias-corrected specific force measures +gravity when static;
        // rotate a gain-limited fraction of the way that maps its current
        // world-frame direction onto g_w. FromTwoVectors yields a horizontal
        // axis, so yaw is left untouched (accel carries no heading).
        const Eigen::Vector3d measured_g_w = R_ * accel_corrected;
        if (measured_g_w.norm() > 1e-9 && g_w_.norm() > 1e-9) {
          const Eigen::Quaterniond q = Eigen::Quaterniond::FromTwoVectors(
              measured_g_w.normalized(), g_w_.normalized());
          const Eigen::AngleAxisd aa(q);
          const double gain =
              std::min(1.0, std::max(0.0, params_.leveling_gain));
          R_ = Eigen::AngleAxisd(gain * aa.angle(), aa.axis())
                   .toRotationMatrix() *
               R_;
          stats.leveling_active = true;
        }
      }
      if (params_.enable_zaru) {
        // True angular rate is zero while stationary, so the raw gyro is a
        // direct bias sample; low-pass the running bias toward it.
        const double gain = std::min(1.0, std::max(0.0, params_.zaru_gain));
        gyro_bias_ = (1.0 - gain) * gyro_bias_ + gain * imu.gyro;
        stats.zaru_active = true;
      }
    }

    if (params_.enable_nhc && !stats.zupt_active) {
      Eigen::Vector3d v_body = R_.transpose() * v_;
      applyNhc(&v_body, dt, &stats.nhc_active);
      v_ = R_ * v_body;
    }
    return stats;
  }

  // --- ESKF path: covariance propagation + gain-weighted measurement updates.
  // The aid flags select which measurements are applied. ZARU is subsumed (the
  // filter estimates gyro bias online), so it is ignored here.
  if (!eskf_initialized_) initEskf();
  eskfPropagate(gyro_corrected, accel_corrected, dt);

  if (params_.enable_zupt && still) {
    // Zero-velocity update: world velocity should be 0.
    Eigen::MatrixXd H = Eigen::MatrixXd::Zero(3, 15);
    H.block<3, 3>(0, kIV) = Eigen::Matrix3d::Identity();
    const Eigen::VectorXd innovation = -v_;  // z - h = 0 - v.
    const Eigen::MatrixXd Rm =
        params_.eskf_zupt_sigma * params_.eskf_zupt_sigma *
        Eigen::Matrix3d::Identity();
    eskfUpdate(H, innovation, Rm);
    stats.zupt_active = true;
    stats.eskf_update = true;
  }

  if (params_.enable_leveling && still) {
    // Gravity update: a_m = R^T g_w + b_a when static. Corrects roll/pitch AND
    // accel bias, gain-weighted (yaw stays unobservable through g_w).
    const Eigen::Vector3d predicted = R_.transpose() * g_w_;
    Eigen::MatrixXd H = Eigen::MatrixXd::Zero(3, 15);
    H.block<3, 3>(0, kITh) = skew(predicted);
    H.block<3, 3>(0, kIBa) = Eigen::Matrix3d::Identity();
    const Eigen::VectorXd innovation = accel_corrected - predicted;
    const Eigen::MatrixXd Rm =
        params_.eskf_leveling_sigma * params_.eskf_leveling_sigma *
        Eigen::Matrix3d::Identity();
    eskfUpdate(H, innovation, Rm);
    stats.leveling_active = true;
    stats.eskf_update = true;
  }

  if (params_.enable_nhc && !(params_.enable_zupt && still)) {
    // Non-holonomic constraint: the two body-velocity components orthogonal to
    // the forward axis should be ~0. Selection S picks those two rows.
    int f = params_.forward_axis;
    if (f < 0 || f > 2) f = 0;
    Eigen::Matrix<double, 2, 3> S = Eigen::Matrix<double, 2, 3>::Zero();
    int row = 0;
    for (int ax = 0; ax < 3; ++ax) {
      if (ax != f) S(row++, ax) = 1.0;
    }
    const Eigen::Vector3d v_body = R_.transpose() * v_;
    Eigen::MatrixXd H = Eigen::MatrixXd::Zero(2, 15);
    H.block<2, 3>(0, kIV) = S * R_.transpose();
    H.block<2, 3>(0, kITh) = S * skew(v_body);
    const Eigen::VectorXd innovation = -(S * v_body);  // z - h = 0 - S v_body.
    const Eigen::MatrixXd Rm = params_.eskf_nhc_sigma * params_.eskf_nhc_sigma *
                               Eigen::Matrix2d::Identity();
    eskfUpdate(H, innovation, Rm);
    stats.nhc_active = true;
    stats.eskf_update = true;
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
