#pragma once

// Two more orientation filters from the VQF paper's comparison (Table 1):
//
// - ValentiFilter: R. G. Valenti, I. Dryanovski, J. Xiao, "Keeping a good
//   attitude: a quaternion-based orientation filter for IMUs and MARGs",
//   Sensors 15(8), 2015 (open access). Written from the paper; the bias
//   steady-state thresholds and the first-sample initialisation, which the
//   paper leaves to the implementation, follow the authors' ROS package
//   (imu_tools / imu_complementary_filter, BSD).
// - SeelFilter: T. Seel, S. Ruppin, "Eliminating the effect of magnetic
//   disturbances on the inclination estimates of inertial sensors", IFAC 2017.
//   The paper is not openly available; this is a port of the authors' MIT
//   implementation (CsgOriEstIMU, distributed in qmt as oriEstIMU).

#include "attitude_estimation/attitude_estimation.h"

#include <array>

namespace localization_zoo {
namespace attitude_estimation {

struct ValentiParams {
  double sampling_rate = 100.0;  // [Hz]
  double alpha_acc = 0.01;       // accelerometer gain alpha (eq. 50)
  double beta_mag = 0.01;        // magnetometer gain beta
  bool bias_estimation = true;   // Sec. 5.4
  double alpha_bias = 0.01;      // bias low-pass gain in steady state
  bool adaptive_gain = false;    // Sec. 5.3
  // The ROS package interpolated with SLERP only for q0 < 0 (effectively
  // always LERP) until 2026-09 (imu_tools #234); the paper's switch is
  // q0 <= 0.9. true reproduces the old package, e.g. the VQF paper's numbers.
  bool legacy_ros_interpolation = false;
};

/// Quaternion complementary filter (Valenti et al. 2015). Internally the state
/// is the global frame relative to the local frame (paper convention); state()
/// returns the sensor orientation in a north-west-up frame (x north).
class ValentiFilter {
 public:
  explicit ValentiFilter(const ValentiParams& params) : params_(params) {}
  void update(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc,
              const Eigen::Vector3d& mag);
  void updateImu(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc);
  Quat state() const { return quatConjugate(q_); }
  const Eigen::Vector3d& bias() const { return bias_; }

 private:
  void updateBias(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc);
  Quat predict(const Eigen::Vector3d& gyr) const;
  Quat accCorrection(const Eigen::Vector3d& acc, const Quat& predicted) const;
  double accGain(const Eigen::Vector3d& acc) const;

  ValentiParams params_;
  Quat q_ = Quat(1, 0, 0, 0);  // global relative to local
  bool initialized_ = false;
  Eigen::Vector3d bias_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d previous_gyr_ = Eigen::Vector3d::Zero();
};

struct SeelParams {
  double sampling_rate = 100.0;  // [Hz]
  double tau_acc = 1.0;          // half-life of the inclination error [s]
  double tau_mag = 3.0;          // half-life of the heading error [s]
  double zeta = 0.0;             // bias estimation strength (0 = off)
  double acc_rating = 1.0;       // down-rate the accelerometer at |a| != g (0 = off)
};

/// Seel and Ruppin 2017 (port of CsgOriEstIMU): inclination correction from
/// the accelerometer, purely horizontal (heading-only) magnetometer
/// correction, PI-type gyro bias estimation. state() is the sensor
/// orientation in ENU (the magnetic reference is +y).
class SeelFilter {
 public:
  explicit SeelFilter(const SeelParams& params);
  /// mag = zero vector skips the magnetometer correction (6D).
  void update(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc,
              const Eigen::Vector3d& mag);
  const Quat& state() const { return q_; }
  /// Estimated gyro bias (to be subtracted from the gyro) [rad/s].
  Eigen::Vector3d bias() const { return -stored_bias_; }

 private:
  void setGains(double tau_acc, double tau_mag, double zeta);

  SeelParams params_;
  Quat q_ = Quat(0.5, 0.5, 0.5, 0.5);
  Eigen::Vector3d stored_bias_ = Eigen::Vector3d::Zero();  // added to the gyro
  double gain_acc_ = 0.0, gain_mag_ = 0.0, gain_bias_ = 0.0;
  double valid_acc_count_ = 0.0;
  std::array<double, 10> rating_window_{};
};

}  // namespace attitude_estimation
}  // namespace localization_zoo
