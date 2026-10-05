#pragma once

// IMU attitude (orientation) estimation filters and the BROAD error metric.
//
// Quaternions are Eigen::Vector4d in (w, x, y, z) order and describe the
// sensor frame relative to the earth frame (v_earth = q * v_sensor * q^-1),
// matching the BROAD dataset convention.

#include <Eigen/Core>

#include <vector>

namespace localization_zoo {
namespace attitude_estimation {

using Quat = Eigen::Vector4d;  // (w, x, y, z)

Quat quatMultiply(const Quat& a, const Quat& b);
Quat quatConjugate(const Quat& q);
Quat quatNormalized(const Quat& q);
/// Rotates v (sensor frame) into the earth frame: q * (0, v) * q^-1.
Eigen::Vector3d quatRotate(const Quat& q, const Eigen::Vector3d& v);

/// Initial orientation from one accelerometer and magnetometer sample (BROAD
/// example_code quatFromAccMag): z along the specific force, x towards
/// magnetic north projected onto the horizontal plane (NWU-like frame).
Quat quatFromAccMag(const Eigen::Vector3d& acc, const Eigen::Vector3d& mag);

/// Quaternion that maps the filters' x-north earth frame to ENU (90 deg
/// about z), applied by BROAD before comparing with the OMC reference.
Quat enuCorrection();

struct MadgwickParams {
  double beta = 0.1;             // gradient-descent gain [rad/s]
  double sampling_rate = 100.0;  // [Hz]
  // The widely used x-io C implementation computes the earth-field reference
  // b at half the magnitude the paper's objective function implies (see
  // https://github.com/dlaidig/broad/issues/1). true reproduces that code.
  bool legacy_xio_field_scale = false;
};

/// Madgwick, "An efficient orientation filter for inertial and
/// inertial/magnetic sensor arrays", report 2010 (gradient-descent filter).
class MadgwickFilter {
 public:
  explicit MadgwickFilter(const MadgwickParams& params) : params_(params) {}
  void setState(const Quat& q) { q_ = quatNormalized(q); }
  const Quat& state() const { return q_; }
  /// 9D update: gyr [rad/s], acc and mag in any consistent unit.
  void update(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc,
              const Eigen::Vector3d& mag);
  /// 6D update (no magnetometer).
  void updateImu(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc);

 private:
  void integrate(Quat q_dot);
  MadgwickParams params_;
  Quat q_ = Quat(1.0, 0.0, 0.0, 0.0);
};

struct MahonyParams {
  double kp = 0.5;               // proportional gain [rad/s]
  double ki = 0.0;               // integral gain [rad/s^2]
  double sampling_rate = 100.0;  // [Hz]
};

/// Mahony, Hamel, Pflimlin, "Nonlinear complementary filters on the special
/// orthogonal group", IEEE TAC 2008 (explicit complementary filter with bias
/// estimation), in the quaternion form of the x-io implementation.
class MahonyFilter {
 public:
  explicit MahonyFilter(const MahonyParams& params) : params_(params) {}
  void setState(const Quat& q) { q_ = quatNormalized(q); }
  const Quat& state() const { return q_; }
  /// Integral feedback term (= minus the estimated gyroscope bias).
  const Eigen::Vector3d& integralFeedback() const { return integral_; }
  void update(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc,
              const Eigen::Vector3d& mag);
  void updateImu(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc);

 private:
  void correctAndIntegrate(Eigen::Vector3d gyr, const Eigen::Vector3d& error);
  MahonyParams params_;
  Quat q_ = Quat(1.0, 0.0, 0.0, 0.0);
  Eigen::Vector3d integral_ = Eigen::Vector3d::Zero();
};

struct OrientationErrors {
  double total_rmse_deg = 0.0;
  double heading_rmse_deg = 0.0;
  double inclination_rmse_deg = 0.0;
  int samples = 0;  // movement samples with a valid reference
};

/// BROAD error metric (example_code/broad_utils.py calculateRMSE): error
/// quaternion q_est * q_ref^-1 in the earth frame, total / heading /
/// inclination angles RMS-averaged over samples with movement[i] true.
/// Samples whose reference contains NaN are skipped (nanmean).
OrientationErrors broadErrors(const std::vector<Quat>& estimate,
                              const std::vector<Quat>& reference,
                              const std::vector<bool>& movement);

}  // namespace attitude_estimation
}  // namespace localization_zoo
