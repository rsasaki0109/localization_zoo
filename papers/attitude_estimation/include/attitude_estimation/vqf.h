#pragma once

// VQF: Laidig and Seel, "VQF: Highly accurate IMU orientation estimation with
// bias estimation and magnetic disturbance rejection", Information Fusion 91,
// 2023 (arXiv:2203.17024). Written from the paper (Algorithms 1-3 and
// Appendices A-E); constants the paper does not state are taken from the
// documented defaults of the authors' package (see README).

#include "attitude_estimation/attitude_estimation.h"

#include <Eigen/Core>

namespace localization_zoo {
namespace attitude_estimation {

struct VQFParams {
  double sampling_rate = 100.0;  // [Hz]
  double tau_acc = 3.0;          // accelerometer correction time constant [s]
  double tau_mag = 9.0;          // magnetometer correction time constant [s]

  bool rest_bias_estimation = true;
  bool motion_bias_estimation = true;
  bool mag_disturbance_rejection = true;

  // Gyroscope bias estimation (Algorithm 2, Appendix E), deg/s and s.
  double bias_sigma_init = 0.5;
  double bias_forgetting_time = 100.0;
  double bias_clip = 2.0;
  double bias_sigma_motion = 0.1;
  double bias_vertical_forgetting_factor = 0.0001;
  double bias_sigma_rest = 0.03;
  // Rest detection (Appendix C).
  double rest_min_time = 1.5;    // [s]
  double rest_filter_tau = 0.5;  // [s]
  double rest_th_gyr = 2.0;      // [deg/s]
  double rest_th_acc = 0.5;      // [m/s^2]
  // Magnetic disturbance rejection (Algorithm 3).
  double mag_current_tau = 0.05;          // [s] low-pass of norm and dip
  double mag_ref_tau = 20.0;              // [s] k_ref; not stated in the paper
  double mag_norm_th = 0.1;               // relative
  double mag_dip_th = 10.0;               // [deg]
  double mag_new_time = 20.0;             // [s]
  double mag_new_first_time = 5.0;        // [s] first reference; not in the paper
  double mag_new_min_gyr = 20.0;          // [deg/s]
  double mag_min_undisturbed_time = 0.5;  // [s]
  double mag_max_rejection_time = 60.0;   // [s]
  double mag_rejection_factor = 2.0;
};

/// Real-time VQF. BasicVQF = all three extensions disabled.
class VQF {
 public:
  explicit VQF(const VQFParams& params);

  /// gyr [rad/s], acc [m/s^2].
  void update(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc);
  /// gyr [rad/s], acc [m/s^2], mag in any unit.
  void update(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc,
              const Eigen::Vector3d& mag);

  /// Sensor orientation relative to the drifting 6D frame E_i (z up).
  Quat quat6D() const;
  /// Sensor orientation relative to ENU (requires magnetometer updates).
  Quat quat9D() const;
  Eigen::Vector3d biasEstimate() const { return bias_; }  // [rad/s]
  bool restDetected() const { return rest_detected_; }
  bool magDisturbanceDetected() const { return mag_disturbed_; }

 private:
  // Second-order Butterworth low-pass (bilinear transform) whose output is
  // initialised with the mean of the first tau / Ts samples (Appendix A.2).
  template <int N>
  struct LowPass {
    Eigen::Matrix<double, N, 1> s0, s1, sum;
    int count = 0;
    bool started = false;
    Eigen::Matrix<double, N, 1> step(const Eigen::Matrix<double, N, 1>& x,
                                     const VQF& owner, double tau);
  };
  struct Coefficients {
    double b0, b1, b2, a1, a2;
  };
  Coefficients coefficients(double tau) const;

  void updateGyr(const Eigen::Vector3d& gyr);
  void updateAcc(const Eigen::Vector3d& acc);
  void updateMag(const Eigen::Vector3d& mag);
  void detectRest(const Eigen::Vector3d& gyr, const Eigen::Vector3d& acc);
  void biasStep(const Eigen::Matrix3d& R, double ax, double ay);

  VQFParams params_;
  double ts_;

  Quat gyr_quat_ = Quat(1, 0, 0, 0);  // S_i -> I_i (strapdown)
  Quat acc_quat_ = Quat(1, 0, 0, 0);  // I_i -> E_i (inclination correction)
  double delta_ = 0.0;                // heading offset E_i -> E

  LowPass<3> acc_lp_;
  double k_mag_init_ = 1.0;  // heading filter gain 1, 1/2, 1/3, ... at the start

  // Rest detection.
  LowPass<3> rest_gyr_lp_, rest_acc_lp_;
  double rest_time_ = 0.0;
  bool rest_detected_ = false;
  Eigen::Vector3d last_gyr_lp_ = Eigen::Vector3d::Zero();

  // Bias Kalman filter (state in rad/s).
  Eigen::Vector3d bias_ = Eigen::Vector3d::Zero();
  Eigen::Matrix3d bias_P_;
  double bias_v_, bias_w_motion_, bias_w_rest_;
  LowPass<9> R_lp_;
  LowPass<3> bias_lp_;

  // Magnetic disturbance rejection.
  LowPass<2> mag_lp_;
  double mag_ref_norm_ = 0.0, mag_ref_dip_ = 0.0;
  double mag_cand_norm_ = -1.0, mag_cand_dip_ = 0.0;
  double mag_cand_time_ = 0.0, mag_undisturbed_time_ = 0.0;
  double mag_reject_time_;
  bool mag_disturbed_ = false;
};

}  // namespace attitude_estimation
}  // namespace localization_zoo
