#include "attitude_estimation/attitude_estimation.h"

#include <Eigen/Geometry>

#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace attitude_estimation {

Quat quatMultiply(const Quat& a, const Quat& b) {
  return Quat(a[0] * b[0] - a[1] * b[1] - a[2] * b[2] - a[3] * b[3],
              a[0] * b[1] + a[1] * b[0] + a[2] * b[3] - a[3] * b[2],
              a[0] * b[2] - a[1] * b[3] + a[2] * b[0] + a[3] * b[1],
              a[0] * b[3] + a[1] * b[2] - a[2] * b[1] + a[3] * b[0]);
}

Quat quatConjugate(const Quat& q) { return Quat(q[0], -q[1], -q[2], -q[3]); }

Quat quatNormalized(const Quat& q) { return q / q.norm(); }

Eigen::Vector3d quatRotate(const Quat& q, const Eigen::Vector3d& v) {
  const Quat r = quatMultiply(quatMultiply(q, Quat(0.0, v.x(), v.y(), v.z())),
                              quatConjugate(q));
  return r.tail<3>();
}

namespace {

// broad_utils.quatFromRotMat, ported as is.
Quat quatFromRotMat(const Eigen::Matrix3d& R) {
  const double w_sq = (1 + R(0, 0) + R(1, 1) + R(2, 2)) / 4;
  const double x_sq = (1 + R(0, 0) - R(1, 1) - R(2, 2)) / 4;
  const double y_sq = (1 - R(0, 0) + R(1, 1) - R(2, 2)) / 4;
  const double z_sq = (1 - R(0, 0) - R(1, 1) + R(2, 2)) / 4;
  return Quat(std::sqrt(w_sq),
              std::copysign(std::sqrt(x_sq), R(2, 1) - R(1, 2)),
              std::copysign(std::sqrt(y_sq), R(0, 2) - R(2, 0)),
              std::copysign(std::sqrt(z_sq), R(1, 0) - R(0, 1)));
}

// Predicted gravity direction in the sensor frame, R(q)^T e_z.
Eigen::Vector3d gravityInSensor(const Quat& q) {
  return Eigen::Vector3d(2 * (q[1] * q[3] - q[0] * q[2]),
                         2 * (q[0] * q[1] + q[2] * q[3]),
                         2 * (0.5 - q[1] * q[1] - q[2] * q[2]));
}

// Predicted earth-field direction R(q)^T (bx, 0, bz) in the sensor frame.
Eigen::Vector3d fieldInSensor(const Quat& q, double bx, double bz) {
  return Eigen::Vector3d(
      bx * (1 - 2 * (q[2] * q[2] + q[3] * q[3])) + 2 * bz * (q[1] * q[3] - q[0] * q[2]),
      2 * bx * (q[1] * q[2] - q[0] * q[3]) + 2 * bz * (q[0] * q[1] + q[2] * q[3]),
      2 * bx * (q[0] * q[2] + q[1] * q[3]) + bz * (1 - 2 * (q[1] * q[1] + q[2] * q[2])));
}

Quat gyroRate(const Quat& q, const Eigen::Vector3d& gyr) {
  return 0.5 * quatMultiply(q, Quat(0.0, gyr.x(), gyr.y(), gyr.z()));
}

bool isZero(const Eigen::Vector3d& v) { return v.x() == 0 && v.y() == 0 && v.z() == 0; }

}  // namespace

Quat quatFromAccMag(const Eigen::Vector3d& acc, const Eigen::Vector3d& mag) {
  const Eigen::Vector3d z = acc;
  const Eigen::Vector3d x = z.cross(-mag).cross(z);
  const Eigen::Vector3d y = z.cross(x);
  Eigen::Matrix3d R;
  R.col(0) = x.normalized();
  R.col(1) = y.normalized();
  R.col(2) = z.normalized();
  return quatFromRotMat(R);
}

Quat enuCorrection() { return Quat(1.0 / std::sqrt(2.0), 0.0, 0.0, 1.0 / std::sqrt(2.0)); }

// ---------------------------------------------------------------------------
// Madgwick: q_dot = 0.5 q (x) (0, w) - beta * grad / |grad|, grad = J^T f
// (report eqs. 25-34 for gravity, 29-31 and 45-46 for the earth field).

void MadgwickFilter::integrate(Quat q_dot) {
  q_ = quatNormalized(q_ + q_dot / params_.sampling_rate);
}

void MadgwickFilter::updateImu(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc) {
  Quat q_dot = gyroRate(q_, gyr);
  if (!isZero(acc)) {
    const Quat& q = q_;
    const Eigen::Vector3d f = gravityInSensor(q) - acc.normalized();
    Eigen::Matrix<double, 3, 4> J;
    J << -2 * q[2], 2 * q[3], -2 * q[0], 2 * q[1],
          2 * q[1], 2 * q[0],  2 * q[3], 2 * q[2],
          0,       -4 * q[1], -4 * q[2], 0;
    const Quat grad = J.transpose() * f;
    if (grad.norm() > 0) q_dot -= params_.beta * grad.normalized();
  }
  integrate(q_dot);
}

void MadgwickFilter::update(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc,
                            const Eigen::Vector3d& mag) {
  if (isZero(mag)) {
    updateImu(gyr, acc);
    return;
  }
  Quat q_dot = gyroRate(q_, gyr);
  if (!isZero(acc)) {
    const Quat& q = q_;
    const Eigen::Vector3d a = acc.normalized();
    const Eigen::Vector3d m = mag.normalized();
    // Earth-field reference b = (|h_xy|, 0, h_z) with h = q (x) m (x) q*.
    const Eigen::Vector3d h = quatRotate(q, m);
    double bx = std::hypot(h.x(), h.y());
    double bz = h.z();
    if (params_.legacy_xio_field_scale) {
      bx *= 0.5;
      bz *= 0.5;
    }
    Eigen::Matrix<double, 6, 1> f;
    f.head<3>() = gravityInSensor(q) - a;
    f.tail<3>() = fieldInSensor(q, bx, bz) - m;
    Eigen::Matrix<double, 6, 4> J;
    J << -2 * q[2], 2 * q[3], -2 * q[0], 2 * q[1],
          2 * q[1], 2 * q[0],  2 * q[3], 2 * q[2],
          0,       -4 * q[1], -4 * q[2], 0,
         -2 * bz * q[2], 2 * bz * q[3], -4 * bx * q[2] - 2 * bz * q[0], -4 * bx * q[3] + 2 * bz * q[1],
         -2 * bx * q[3] + 2 * bz * q[1], 2 * bx * q[2] + 2 * bz * q[0], 2 * bx * q[1] + 2 * bz * q[3], -2 * bx * q[0] + 2 * bz * q[2],
          2 * bx * q[2], 2 * bx * q[3] - 4 * bz * q[1], 2 * bx * q[0] - 4 * bz * q[2], 2 * bx * q[1];
    const Quat grad = J.transpose() * f;
    if (grad.norm() > 0) q_dot -= params_.beta * grad.normalized();
  }
  integrate(q_dot);
}

// ---------------------------------------------------------------------------
// Mahony: w_mes = sum_i v_i x v_hat_i, Omega = w + kp w_mes + ki int(w_mes),
// q_dot = 0.5 q (x) (0, Omega) (paper eqs. 32-33 with the x-io earth-field
// reference for the magnetometer direction).

void MahonyFilter::correctAndIntegrate(Eigen::Vector3d gyr, const Eigen::Vector3d& error) {
  const double dt = 1.0 / params_.sampling_rate;
  if (params_.ki > 0) {
    integral_ += params_.ki * error * dt;
    gyr += integral_;
  } else {
    integral_.setZero();
  }
  gyr += params_.kp * error;
  q_ = quatNormalized(q_ + gyroRate(q_, gyr) * dt);
}

void MahonyFilter::updateImu(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc) {
  if (isZero(acc)) {
    q_ = quatNormalized(q_ + gyroRate(q_, gyr) / params_.sampling_rate);
    return;
  }
  correctAndIntegrate(gyr, acc.normalized().cross(gravityInSensor(q_)));
}

void MahonyFilter::update(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc,
                          const Eigen::Vector3d& mag) {
  if (isZero(mag)) {
    updateImu(gyr, acc);
    return;
  }
  if (isZero(acc)) {
    q_ = quatNormalized(q_ + gyroRate(q_, gyr) / params_.sampling_rate);
    return;
  }
  const Eigen::Vector3d a = acc.normalized();
  const Eigen::Vector3d m = mag.normalized();
  const Eigen::Vector3d h = quatRotate(q_, m);
  const Eigen::Vector3d w = fieldInSensor(q_, std::hypot(h.x(), h.y()), h.z());
  correctAndIntegrate(gyr, a.cross(gravityInSensor(q_)) + m.cross(w));
}

// ---------------------------------------------------------------------------

OrientationErrors broadErrors(const std::vector<Quat>& estimate,
                              const std::vector<Quat>& reference,
                              const std::vector<bool>& movement) {
  double total = 0, heading = 0, inclination = 0;
  int n = 0;
  const size_t count = std::min({estimate.size(), reference.size(), movement.size()});
  for (size_t i = 0; i < count; ++i) {
    if (!movement[i] || !reference[i].allFinite()) continue;
    const Quat d = quatNormalized(
        quatMultiply(quatNormalized(estimate[i]), quatConjugate(quatNormalized(reference[i]))));
    const double e_total = 2 * std::acos(std::clamp(std::abs(d[0]), 0.0, 1.0));
    const double e_heading = 2 * std::atan(std::abs(d[3] / d[0]));
    const double e_incl =
        2 * std::acos(std::clamp(std::sqrt(d[0] * d[0] + d[3] * d[3]), 0.0, 1.0));
    total += e_total * e_total;
    heading += e_heading * e_heading;
    inclination += e_incl * e_incl;
    ++n;
  }
  OrientationErrors out;
  out.samples = n;
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double to_deg = 180.0 / M_PI;
  out.total_rmse_deg = n ? std::sqrt(total / n) * to_deg : nan;
  out.heading_rmse_deg = n ? std::sqrt(heading / n) * to_deg : nan;
  out.inclination_rmse_deg = n ? std::sqrt(inclination / n) * to_deg : nan;
  return out;
}

}  // namespace attitude_estimation
}  // namespace localization_zoo
