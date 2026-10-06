#include "attitude_estimation/complementary.h"

#include <Eigen/Geometry>

#include <algorithm>
#include <cmath>

namespace localization_zoo {
namespace attitude_estimation {

namespace {

// Scale a delta quaternion towards identity by `gain` (eqs. 50-52): LERP when
// it is already close to identity (q0 > 0.9), SLERP otherwise.
Quat scaleDelta(Quat dq, double gain, bool legacy_ros) {
  if (legacy_ros ? dq[0] < 0.0 : dq[0] <= 0.9) {
    const double angle = std::acos(std::clamp(dq[0], -1.0, 1.0));
    const double A = std::sin(angle * (1.0 - gain)) / std::sin(angle);
    const double B = std::sin(angle * gain) / std::sin(angle);
    dq = Quat(A + B * dq[0], B * dq[1], B * dq[2], B * dq[3]);
  } else {
    dq = Quat((1.0 - gain) + gain * dq[0], gain * dq[1], gain * dq[2], gain * dq[3]);
  }
  return quatNormalized(dq);
}

// Inclination-only quaternion from a normalised gravity reading (Sec. 4,
// eqs. 18-25), global relative to local with zero yaw.
Quat accelerometerQuat(const Eigen::Vector3d& a) {
  if (a.z() >= 0) {
    const double q0 = std::sqrt((a.z() + 1) * 0.5);
    return Quat(q0, -a.y() / (2 * q0), a.x() / (2 * q0), 0);
  }
  const double X = std::sqrt((1 - a.z()) * 0.5);
  return Quat(-a.y() / (2 * X), X, 0, a.x() / (2 * X));
}

// Heading-only quaternion that turns the horizontal field l = (lx, ly) onto
// +x (eqs. 26-28 and 58).
Quat headingQuat(double lx, double ly) {
  const double gamma = lx * lx + ly * ly;
  const double beta = std::sqrt(gamma + lx * std::sqrt(gamma));
  return Quat(beta / std::sqrt(2.0 * gamma), 0, 0, ly / (std::sqrt(2.0) * beta));
}

constexpr double kGravity = 9.81;

}  // namespace

// ---------------------------------------------------------------------------
// Valenti et al. 2015

void ValentiFilter::updateBias(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc) {
  // Sec. 5.4: low-pass the gyro into the bias only in a steady state. The
  // steady-state test is the ROS package's (|a| within 0.1 of g, per-axis
  // change of the rate <= 0.01 rad/s, per-axis rate within 0.2 of the bias).
  const bool steady =
      std::abs(acc.norm() - kGravity) <= 0.1 &&
      (gyr - previous_gyr_).cwiseAbs().maxCoeff() <= 0.01 &&
      (gyr - bias_).cwiseAbs().maxCoeff() <= 0.2;
  if (steady) bias_ += params_.alpha_bias * (gyr - bias_);
  previous_gyr_ = gyr;
}

Quat ValentiFilter::predict(const Eigen::Vector3d& gyr) const {
  // Eqs. 38-42: q_dot = -1/2 omega (x) q, forward Euler, normalised.
  const Eigen::Vector3d w = gyr - bias_;
  const Quat q_dot = -0.5 * quatMultiply(Quat(0, w.x(), w.y(), w.z()), q_);
  return quatNormalized(q_ + q_dot / params_.sampling_rate);
}

Quat ValentiFilter::accCorrection(const Eigen::Vector3d& acc, const Quat& predicted) const {
  // Eqs. 44-47: predicted gravity in the global frame and the delta
  // quaternion that rotates it onto (0, 0, 1).
  const Eigen::Vector3d g = quatRotate(quatConjugate(predicted), acc.normalized());
  const double dq0 = std::sqrt((g.z() + 1) * 0.5);
  return Quat(dq0, -g.y() / (2 * dq0), g.x() / (2 * dq0), 0);
}

double ValentiFilter::accGain(const Eigen::Vector3d& acc) const {
  if (!params_.adaptive_gain) return params_.alpha_acc;
  // Sec. 5.3: gain factor 1 below 10 % magnitude error, 0 above 20 %.
  const double error = std::abs(acc.norm() - kGravity) / kGravity;
  const double factor = error < 0.1 ? 1.0 : (error < 0.2 ? (0.2 - error) / 0.1 : 0.0);
  return factor * params_.alpha_acc;
}

void ValentiFilter::updateImu(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc) {
  if (!initialized_) {
    q_ = accelerometerQuat(acc.normalized());
    initialized_ = true;
    return;
  }
  if (params_.bias_estimation) updateBias(gyr, acc);
  const Quat predicted = predict(gyr);
  const Quat dq = scaleDelta(accCorrection(acc, predicted), accGain(acc), params_.legacy_ros_interpolation);
  q_ = quatNormalized(quatMultiply(predicted, dq));
}

void ValentiFilter::update(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc,
                           const Eigen::Vector3d& mag) {
  if (!initialized_) {
    // Sec. 4: q = q_acc (x) q_mag from the first sample.
    const Quat q_acc = accelerometerQuat(acc.normalized());
    const Eigen::Vector3d l = quatRotate(quatConjugate(q_acc), mag);
    q_ = quatMultiply(q_acc, headingQuat(l.x(), l.y()));
    initialized_ = true;
    return;
  }
  if (params_.bias_estimation) updateBias(gyr, acc);
  const Quat predicted = predict(gyr);
  const Quat dq_acc = scaleDelta(accCorrection(acc, predicted), accGain(acc), params_.legacy_ros_interpolation);
  const Quat corrected = quatMultiply(predicted, dq_acc);  // eq. 53
  // Eqs. 54-59: heading-only correction from the field in the global frame.
  const Eigen::Vector3d l = quatRotate(quatConjugate(corrected), mag);
  const Quat dq_mag = scaleDelta(headingQuat(l.x(), l.y()), params_.beta_mag, params_.legacy_ros_interpolation);
  q_ = quatNormalized(quatMultiply(corrected, dq_mag));
}

// ---------------------------------------------------------------------------
// Seel and Ruppin 2017 (port of CsgOriEstIMU)

SeelFilter::SeelFilter(const SeelParams& params) : params_(params) {
  q_ = quatNormalized(q_);
}

void SeelFilter::setGains(double tau_acc, double tau_mag, double zeta) {
  // Half-life time constants to per-sample gains, and the bias gain.
  const double f = params_.sampling_rate;
  gain_acc_ = 1.0 - 1.4 * tau_acc * f / (1.4 * tau_acc * f + 1);
  gain_mag_ = 1.0 - 1.4 * tau_mag * f / (1.4 * tau_mag * f + 1);
  gain_bias_ = zeta * zeta / 160.0 * 1.4 * f;
}

void SeelFilter::update(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc,
                        const Eigen::Vector3d& mag) {
  const double f = params_.sampling_rate;
  // Faster time constants while the filter starts, and no bias estimation
  // before tau_acc / 2 of valid accelerometer data.
  const double t_start = (valid_acc_count_ + 1) / f / 2;
  const double tau_acc = std::min(t_start, params_.tau_acc);
  if (t_start < params_.tau_acc / 2) {
    setGains(tau_acc, std::min(t_start, params_.tau_mag), 0.0);
  } else {
    // (The reference keeps tau_mag >= 100 s fixed here.)
    setGains(tau_acc, params_.tau_mag < 100 ? std::min(t_start, params_.tau_mag) : params_.tau_mag,
             params_.zeta);
  }

  // Accelerometer rating: down-weight while |a| deviates from 9.81 within the
  // last 10 samples.
  double rating = 1.0;
  if (params_.acc_rating > 0) {
    std::rotate(rating_window_.begin(), rating_window_.begin() + 1, rating_window_.end());
    rating_window_.back() = std::abs(acc.norm() - 9.81);
    rating = 1.0 / (1.0 + *std::max_element(rating_window_.begin(), rating_window_.end()) *
                              params_.acc_rating);
  }

  // Gyro prediction (q_ maps sensor to fixed frame; body-frame increment).
  const Eigen::Vector3d w = gyr + stored_bias_;
  Quat q_gyr = q_;
  if (!w.isZero()) {
    const double half_angle = w.norm() / (2 * f);
    const Eigen::Vector3d axis = w.normalized() * std::sin(half_angle);
    q_gyr = quatMultiply(q_, Quat(std::cos(half_angle), axis.x(), axis.y(), axis.z()));
  }

  // Inclination correction about the axis acc x g_ref (in the sensor frame).
  Quat q_acc = q_gyr;
  if (!acc.isZero()) {
    valid_acc_count_ += 1;
    const Eigen::Vector3d g_ref = quatRotate(quatConjugate(q_gyr), Eigen::Vector3d::UnitZ());
    const double cosine = acc.normalized().dot(g_ref);
    if (cosine < 1 && cosine > -1) {
      const double error = std::acos(cosine);
      const double kp = gain_acc_ * rating;
      const Eigen::Vector3d axis = acc.cross(g_ref).normalized();
      const double half = kp * error / 2;
      const Eigen::Vector3d v = axis * std::sin(half);
      q_acc = quatMultiply(q_gyr, Quat(std::cos(half), v.x(), v.y(), v.z()));
      stored_bias_ += kp * kp / (1 - kp) * gain_bias_ * error * axis;
    }
  }

  // Heading correction with the field projected onto the horizontal plane.
  Quat q_mag = q_acc;
  if (!mag.isZero()) {
    const Eigen::Vector3d g_ref = quatRotate(quatConjugate(q_acc), Eigen::Vector3d::UnitZ());
    const Eigen::Vector3d m_ref = quatRotate(quatConjugate(q_acc), Eigen::Vector3d::UnitY());
    const double vertical = g_ref.dot(mag);
    const double n = mag.norm();
    if (vertical < n && vertical > -n) {
      const Eigen::Vector3d horizontal = mag - vertical * g_ref;
      const double cosine = horizontal.normalized().dot(m_ref);
      if (cosine < 1 && cosine > -1) {
        const double error = std::acos(cosine);
        const double half = gain_mag_ * error / 2;
        const Eigen::Vector3d axis = horizontal.cross(m_ref).normalized();
        const Eigen::Vector3d v = axis * std::sin(half);
        q_mag = quatMultiply(q_acc, Quat(std::cos(half), v.x(), v.y(), v.z()));
        stored_bias_ += gain_mag_ * gain_mag_ / (1 - gain_mag_) * gain_bias_ * error * axis;
      }
    }
  }

  q_ = quatNormalized(q_mag);
  if (q_[0] < 0) q_ = -q_;
}

}  // namespace attitude_estimation
}  // namespace localization_zoo
