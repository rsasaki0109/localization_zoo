#include "attitude_estimation/vqf.h"

#include <Eigen/LU>

#include <algorithm>
#include <cmath>

namespace localization_zoo {
namespace attitude_estimation {

namespace {

constexpr double kDeg = M_PI / 180.0;

double wrapToPi(double a) {
  while (a > M_PI) a -= 2 * M_PI;
  while (a < -M_PI) a += 2 * M_PI;
  return a;
}

// Rotation quaternion for angle |v| around v (Algorithm 1, line 8).
Quat rotationQuat(const Eigen::Vector3d& v) {
  const double angle = v.norm();
  if (angle < 1e-15) return Quat(1, 0, 0, 0);
  const Eigen::Vector3d axis = v / angle * std::sin(angle / 2);
  return Quat(std::cos(angle / 2), axis.x(), axis.y(), axis.z());
}

Eigen::Matrix3d rotationMatrix(const Quat& q) {
  const double w = q[0], x = q[1], y = q[2], z = q[3];
  Eigen::Matrix3d R;
  R << 1 - 2 * (y * y + z * z), 2 * (x * y - w * z), 2 * (x * z + w * y),
       2 * (x * y + w * z), 1 - 2 * (x * x + z * z), 2 * (y * z - w * x),
       2 * (x * z - w * y), 2 * (y * z + w * x), 1 - 2 * (x * x + y * y);
  return R;
}

}  // namespace

VQF::VQF(const VQFParams& params)
    : params_(params), ts_(1.0 / params.sampling_rate),
      mag_reject_time_(params.mag_max_rejection_time) {
  // Appendix E: parametrisation independent of the sampling rate.
  const double sigma_init = params_.bias_sigma_init * kDeg;
  bias_P_ = sigma_init * sigma_init * Eigen::Matrix3d::Identity();
  bias_v_ = std::pow(0.1 * kDeg, 2) * ts_ / params_.bias_forgetting_time;
  const double s_motion = params_.bias_sigma_motion * kDeg;
  const double s_rest = params_.bias_sigma_rest * kDeg;
  bias_w_motion_ = std::pow(s_motion, 4) / bias_v_ + s_motion * s_motion;
  bias_w_rest_ = std::pow(s_rest, 4) / bias_v_ + s_rest * s_rest;
}

VQF::Coefficients VQF::coefficients(double tau) const {
  // Eq. (8): f_c = sqrt(2) / (2 pi tau); bilinear-transform Butterworth.
  const double fc = std::sqrt(2.0) / (2 * M_PI * tau);
  const double C = std::tan(M_PI * fc * ts_);
  const double D = C * C + std::sqrt(2.0) * C + 1;
  Coefficients c;
  c.b0 = C * C / D;
  c.b1 = 2 * c.b0;
  c.b2 = c.b0;
  c.a1 = 2 * (C * C - 1) / D;
  c.a2 = (1 - std::sqrt(2.0) * C + C * C) / D;
  return c;
}

template <int N>
Eigen::Matrix<double, N, 1> VQF::LowPass<N>::step(const Eigen::Matrix<double, N, 1>& x,
                                                  const VQF& owner, double tau) {
  const int init_samples = std::max(1, static_cast<int>(std::lround(tau / owner.ts_)));
  const Coefficients c = owner.coefficients(tau);
  if (!started) {
    if (count == 0) sum.setZero();
    sum += x;
    ++count;
    const Eigen::Matrix<double, N, 1> mean = sum / count;
    if (count >= init_samples) {
      // Steady state of the transposed direct form II at input = output = mean.
      s0 = (1 - c.b0) * mean;
      s1 = (c.b2 - c.a2) * mean;
      started = true;
    }
    return mean;
  }
  const Eigen::Matrix<double, N, 1> y = c.b0 * x + s0;
  s0 = c.b1 * x - c.a1 * y + s1;
  s1 = c.b2 * x - c.a2 * y;
  return y;
}

Quat VQF::quat6D() const { return quatMultiply(acc_quat_, gyr_quat_); }

Quat VQF::quat9D() const {
  return quatMultiply(Quat(std::cos(delta_ / 2), 0, 0, std::sin(delta_ / 2)), quat6D());
}

void VQF::update(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc) {
  if (params_.rest_bias_estimation) detectRest(gyr, acc);
  updateGyr(gyr);
  updateAcc(acc);
}

void VQF::update(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc,
                 const Eigen::Vector3d& mag) {
  update(gyr, acc);
  updateMag(mag);
}

void VQF::updateGyr(const Eigen::Vector3d& gyr) {
  // Eq. (3) with the bias estimate removed.
  gyr_quat_ = quatNormalized(quatMultiply(gyr_quat_, rotationQuat((gyr - bias_) * ts_)));
}

void VQF::detectRest(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc) {
  // Appendix C: deviation from a 0.5 s low-pass in the sensor frame must stay
  // below the thresholds for 1.5 s.
  const Eigen::Vector3d gyr_lp = rest_gyr_lp_.step(gyr, *this, params_.rest_filter_tau);
  const Eigen::Vector3d acc_lp = rest_acc_lp_.step(acc, *this, params_.rest_filter_tau);
  last_gyr_lp_ = gyr_lp;
  rest_time_ += ts_;
  if ((gyr - gyr_lp).norm() > params_.rest_th_gyr * kDeg ||
      (acc - acc_lp).norm() > params_.rest_th_acc) {
    rest_time_ = 0.0;
  }
  rest_detected_ = rest_time_ >= params_.rest_min_time;
}

void VQF::updateAcc(const Eigen::Vector3d& acc) {
  if (acc.isZero()) return;
  // Algorithm 1, lines 9-14.
  const Eigen::Vector3d acc_i = quatRotate(gyr_quat_, acc);
  const Eigen::Vector3d acc_lp_i = acc_lp_.step(acc_i, *this, params_.tau_acc);
  const Eigen::Vector3d a = quatRotate(acc_quat_, acc_lp_i).normalized();

  if (params_.rest_bias_estimation || params_.motion_bias_estimation) {
    biasStep(rotationMatrix(quat6D()), a.x(), a.y());
  }

  const double qw = std::sqrt((a.z() + 1) / 2);
  if (qw > 1e-6) {
    const Quat correction(qw, a.y() / (2 * qw), -a.x() / (2 * qw), 0.0);
    acc_quat_ = quatNormalized(quatMultiply(correction, acc_quat_));
  }
}

void VQF::biasStep(const Eigen::Matrix3d& R, double ax, double ay) {
  // Algorithm 2, lines 22-40, and eqs. (19), (42).
  Eigen::Matrix<double, 9, 1> r_flat = Eigen::Map<const Eigen::Matrix<double, 9, 1>>(R.data());
  const Eigen::Matrix<double, 9, 1> r_lp = R_lp_.step(r_flat, *this, params_.tau_acc);
  const Eigen::Matrix3d R_lp = Eigen::Map<const Eigen::Matrix3d>(r_lp.data());
  const Eigen::Vector3d bias_e_lp = bias_lp_.step(R * bias_, *this, params_.tau_acc);

  Eigen::Vector3d y;
  Eigen::Matrix3d C;
  Eigen::Vector3d w;
  if (params_.rest_bias_estimation && rest_detected_) {
    y = last_gyr_lp_;
    C.setIdentity();
    w = Eigen::Vector3d::Constant(bias_w_rest_);
  } else if (params_.motion_bias_estimation) {
    y = Eigen::Vector3d(-ay / ts_ + bias_e_lp.x(), ax / ts_ + bias_e_lp.y(), 0.0);
    C = R_lp;
    w = Eigen::Vector3d(bias_w_motion_, bias_w_motion_,
                        bias_w_motion_ / params_.bias_vertical_forgetting_factor);
  } else {
    return;
  }

  bias_P_ += bias_v_ * Eigen::Matrix3d::Identity();
  const Eigen::Matrix3d S = Eigen::Matrix3d(w.asDiagonal()) + C * bias_P_ * C.transpose();
  const Eigen::Matrix3d K = bias_P_ * C.transpose() * S.inverse();
  const double clip = params_.bias_clip * kDeg;
  const Eigen::Vector3d innovation = (y - C * bias_).cwiseMax(-clip).cwiseMin(clip);
  bias_ += K * innovation;
  bias_P_ -= K * C * bias_P_;
  bias_ = bias_.cwiseMax(-clip).cwiseMin(clip);
}

void VQF::updateMag(const Eigen::Vector3d& mag) {
  if (mag.isZero()) return;
  // Algorithm 1, lines 17-19; Algorithm 3 adjusts the gain.
  const Eigen::Vector3d m = quatRotate(quat6D(), mag);
  const double k_mag = 1 - std::exp(-ts_ / params_.tau_mag);  // eq. (7)
  double gain_factor = 1.0;

  if (params_.mag_disturbance_rejection) {
    const double k_ref = 1 - std::exp(-ts_ / params_.mag_ref_tau);
    const double norm_raw = mag.norm();
    const double dip_raw = -std::asin(std::clamp(m.z() / norm_raw, -1.0, 1.0));
    const Eigen::Vector2d nd =
        mag_lp_.step(Eigen::Vector2d(norm_raw, dip_raw), *this, params_.mag_current_tau);
    const double n = nd.x(), dip = nd.y();
    const double dip_th = params_.mag_dip_th * kDeg;

    // Detection against the current reference.
    if (mag_ref_norm_ > 0 && std::abs(n - mag_ref_norm_) < params_.mag_norm_th * mag_ref_norm_ &&
        std::abs(dip - mag_ref_dip_) < dip_th) {
      mag_undisturbed_time_ += ts_;
      if (mag_undisturbed_time_ >= params_.mag_min_undisturbed_time) {
        mag_disturbed_ = false;
        mag_ref_norm_ += k_ref * (n - mag_ref_norm_);
        mag_ref_dip_ += k_ref * (dip - mag_ref_dip_);
      }
    } else {
      mag_disturbed_ = true;
      mag_undisturbed_time_ = 0.0;
    }

    // Acceptance of a new, consistent field as reference.
    if (mag_cand_norm_ > 0 && std::abs(n - mag_cand_norm_) < params_.mag_norm_th * mag_cand_norm_ &&
        std::abs(dip - mag_cand_dip_) < dip_th) {
      // Movement is judged on the rest detector's low-passed gyro (the paper
      // writes |omega|; matched to the reference implementation).
      if (last_gyr_lp_.norm() >= params_.mag_new_min_gyr * kDeg) mag_cand_time_ += ts_;
      mag_cand_norm_ += k_ref * (n - mag_cand_norm_);
      mag_cand_dip_ += k_ref * (dip - mag_cand_dip_);
      const double needed =
          mag_ref_norm_ > 0 ? params_.mag_new_time : params_.mag_new_first_time;
      if (mag_disturbed_ && mag_cand_time_ >= needed) {
        mag_ref_norm_ = mag_cand_norm_;
        mag_ref_dip_ = mag_cand_dip_;
        mag_disturbed_ = false;
        mag_undisturbed_time_ = params_.mag_min_undisturbed_time;
      }
    } else {
      mag_cand_time_ = 0.0;
      mag_cand_norm_ = n;
      mag_cand_dip_ = dip;
    }

    // Rejection. The rejection timer starts at its limit (not stated in the
    // paper): without a reference yet, correct at the reduced gain rather
    // than not at all.
    if (mag_disturbed_) {
      if (mag_reject_time_ <= params_.mag_max_rejection_time) {
        mag_reject_time_ += ts_;
        gain_factor = 0.0;
      } else {
        gain_factor = 1.0 / params_.mag_rejection_factor;
      }
    } else {
      mag_reject_time_ =
          std::max(mag_reject_time_ - params_.mag_rejection_factor * ts_, 0.0);
    }
  }

  // Appendix A.3: average the first measurements (k = 1, 1/2, 1/3, ...) while
  // that gain is larger than k_mag; the averaging ignores the rejection.
  double k = k_mag * gain_factor;
  if (k_mag_init_ != 0.0) {
    k = std::max(k, k_mag_init_);
    k_mag_init_ = k_mag_init_ / (1 + k_mag_init_);
    if (k_mag_init_ * params_.tau_mag < ts_) k_mag_init_ = 0.0;  // t > tau_mag
  }
  if (k == 0.0) return;
  const double delta_mag = std::atan2(m.x(), m.y());
  delta_ = wrapToPi(delta_ + k * wrapToPi(delta_mag - delta_));
}

}  // namespace attitude_estimation
}  // namespace localization_zoo
