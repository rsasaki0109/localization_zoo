#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <deque>
#include <string>
#include <vector>

namespace localization_zoo {
namespace imu_dead_reckoning {

struct ImuReading {
  double stamp = 0.0;
  Eigen::Vector3d gyro = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel = Eigen::Vector3d::Zero();
};

struct ImuDeadReckoningParams {
  // Static initialization.
  double static_init_duration_s = 2.0;   // window at start assumed static.
  double static_gyro_std_gate = 0.05;    // rad/s; warn (do not abort) if exceeded.
  bool estimate_gyro_bias = true;        // from static window mean.
  bool gravity_from_static_norm = true;  // |g| from static accel mean norm; else 9.80665.

  /// Static-init quality gate (opt-in). IMU alone cannot distinguish a
  /// constant-velocity cruise from true rest, but it CAN detect an unusably
  /// short averaging window (compressed/sparse timelines) or road
  /// vibration/attitude motion in the accel norm. When enabled and the window
  /// fails the gate, gyro/accel bias estimation is skipped automatically (bias
  /// stays zero) instead of applying a bias contaminated by non-static motion.
  /// This auto-fixes the KITTI gyro-bias reversal without the caller having to
  /// know to pass --imu-dr-no-gyro-bias by hand. Roll/pitch alignment and
  /// gravity magnitude are still taken from the window (|a| ~= g holds even at
  /// constant velocity), only the bias terms are distrusted.
  bool motion_gated_static_init = false;
  int min_static_samples = 5;          // reject windows with fewer samples.
  double static_accel_std_gate = 0.5;  // m/s^2; std of accel norm over window.

  // Propagation.
  bool midpoint_integration = true;  // gyro midpoint rule; false = forward Euler.
  bool rk4_integration = false;      // RK4 attitude ODE; overrides midpoint/Euler.
  double max_dt = 0.5;               // clamp dt across sample gaps.

  // Opt-in aids (default OFF: pure dead reckoning).
  bool enable_zupt = false;
  double zupt_gyro_threshold = 0.05;    // rad/s, window mean gyro norm.
  double zupt_accel_tolerance = 0.8;    // m/s^2, | |a| - g | window gate.
  int zupt_window = 10;                 // samples.

  /// Attitude leveling on detected-stationary samples: when the ZUPT gate says
  /// "stationary" the bias-corrected accelerometer measures gravity, so
  /// roll/pitch can be re-aligned toward the world gravity vector g_w. The
  /// correction axis is horizontal (cross of two near-vertical vectors) so it
  /// only touches roll/pitch, never yaw (no heading observability from accel).
  /// Attacks the dominant attitude-drift error source directly, complementing
  /// ZUPT's velocity reset. Shares the ZUPT stationary gate but does NOT
  /// require enable_zupt (velocity reset) to be on.
  bool enable_leveling = false;
  double leveling_gain = 0.05;  // fraction of the roll/pitch error per sample.

  /// Zero Angular Rate Update: on detected-stationary samples the true angular
  /// rate is zero, so the raw gyro reading is a fresh online estimate of the
  /// gyro bias. Low-pass the running gyro_bias toward it to track slow bias
  /// drift the one-shot static-init estimate cannot. Shares the ZUPT gate.
  bool enable_zaru = false;
  double zaru_gain = 0.01;  // low-pass rate toward the measured stationary gyro.

  /// Non-holonomic constraint: damp lateral/vertical body velocity toward zero
  /// (vehicle cannot slide sideways). Hard projection when nhc_gain <= 0;
  /// soft exponential pull when nhc_gain > 0 (same dt-scaled form as NHC-Net).
  bool enable_nhc = false;
  double nhc_gain = 0.0;  // 0 = hard forward-only projection.
  int forward_axis = 0;   // body axis index for forward (0=x, KITTI Velodyne).

  /// Estimate accelerometer bias from the static window residual after gravity
  /// alignment (captures scale/orthogonal bias not absorbed by g_w init).
  bool estimate_accel_bias = false;

  /// Error-State Kalman Filter mode. When enabled the same strapdown core is
  /// treated as the ESKF nominal state and a 15-state error covariance (dp, dv,
  /// dtheta, db_g, db_a; body-frame attitude error) is propagated alongside it.
  /// The aid flags above are re-interpreted as measurement updates instead of
  /// hard resets: enable_zupt -> zero-velocity update, enable_nhc -> lateral/
  /// vertical body-velocity update, enable_leveling -> gravity (accel) update
  /// that corrects roll/pitch AND accel bias. Every update is gain-weighted by
  /// the covariance, so gyro/accel bias are estimated online (ZARU becomes
  /// unnecessary and is ignored in this mode). Off by default: pure DR.
  bool enable_eskf = false;
  // Continuous process-noise densities (per sqrt-second) and measurement
  // sigmas. Defaults are order-of-magnitude values for automotive MEMS IMUs.
  double eskf_sigma_accel = 0.2;         // m/s^2, accel white noise -> dv.
  double eskf_sigma_gyro = 0.02;         // rad/s, gyro white noise -> dtheta.
  double eskf_sigma_bias_accel = 1e-3;   // m/s^2 random-walk of accel bias.
  double eskf_sigma_bias_gyro = 1e-4;    // rad/s random-walk of gyro bias.
  double eskf_zupt_sigma = 0.02;         // m/s, ZUPT velocity meas noise.
  double eskf_nhc_sigma = 0.1;           // m/s, NHC lateral/vertical meas noise.
  double eskf_leveling_sigma = 0.3;      // m/s^2, gravity (accel) meas noise.
  // Initial error-covariance std devs, applied at static-init.
  double eskf_init_sigma_att = 0.05;         // rad (roll/pitch/yaw).
  double eskf_init_sigma_vel = 0.1;          // m/s.
  double eskf_init_sigma_pos = 0.0;          // m (anchored, so 0).
  double eskf_init_sigma_bias_gyro = 0.01;   // rad/s.
  double eskf_init_sigma_bias_accel = 0.05;  // m/s^2.
};

struct ImuDeadReckoningStepStats {
  bool zupt_active = false;
  bool nhc_active = false;
  bool leveling_active = false;
  bool zaru_active = false;
  bool eskf_update = false;  // any ESKF measurement update fired this step.
};

/// Pure strapdown IMU-only dead reckoning: the unaided "lower bound" baseline
/// for the LIO family. No lidar, no NHC, no ZUPT unless explicitly enabled.
class ImuDeadReckoningPipeline {
public:
  explicit ImuDeadReckoningPipeline(
      const ImuDeadReckoningParams& params = ImuDeadReckoningParams());

  void reset();

  /// Explicitly seed static initialization from a caller-supplied window
  /// (gyro bias, initial roll/pitch from gravity alignment, gravity
  /// magnitude). If not called, processImu() collects the first
  /// static_init_duration_s of samples internally and initializes from them.
  void initializeStatic(const std::vector<ImuReading>& static_window);

  ImuDeadReckoningStepStats processImu(const ImuReading& imu);

  Eigen::Matrix4d pose() const;
  Eigen::Vector3d velocity() const;

  /// 15x15 error covariance (dp, dv, dtheta, db_g, db_a). Identity-sized zero
  /// matrix unless enable_eskf is on and static init has run. Exposed for
  /// tests / introspection of filter confidence.
  const Eigen::Matrix<double, 15, 15>& covariance() const { return P_; }

  bool staticInitialized() const { return static_initialized_; }

  /// True if motion_gated_static_init was enabled and the static window failed
  /// the quality gate, so bias estimation was auto-skipped for this run.
  bool staticGateTripped() const { return static_gate_tripped_; }

  /// Non-empty if the static-init window's gyro noise exceeded
  /// static_gyro_std_gate, or (with the quality gate on) the window failed it
  /// (informational only; init still proceeds with distrusted biases zeroed).
  const std::string& warning() const { return warning_; }

private:
  void finalizeStaticInit(const std::vector<ImuReading>& window);
  void applyNhc(Eigen::Vector3d* v_body, double dt, bool* nhc_active) const;
  Eigen::Vector3d bodyForwardAxis() const;

  // --- ESKF (enable_eskf) ---
  void initEskf();
  void eskfPropagate(const Eigen::Vector3d& gyro_corrected,
                     const Eigen::Vector3d& accel_corrected, double dt);
  // Generic error-state update: injects K*innovation into the nominal state and
  // shrinks P. H maps the 15-error to the measurement; meas_cov is R.
  void eskfUpdate(const Eigen::MatrixXd& H, const Eigen::VectorXd& innovation,
                  const Eigen::MatrixXd& meas_cov);

  ImuDeadReckoningParams params_;

  Eigen::Matrix3d R_ = Eigen::Matrix3d::Identity();
  Eigen::Vector3d v_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d p_ = Eigen::Vector3d::Zero();

  Eigen::Vector3d gyro_bias_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel_bias_ = Eigen::Vector3d::Zero();
  // World-frame gravity vector g_w subtracted during propagation:
  // a_w = R * a - g_w, derived from the static window so it is self
  // consistent regardless of the IMU's own gravity sign convention.
  Eigen::Vector3d g_w_ = Eigen::Vector3d::Zero();
  double gravity_magnitude_ = 9.80665;

  bool static_initialized_ = false;
  bool static_gate_tripped_ = false;
  std::vector<ImuReading> static_buffer_;
  std::string warning_;

  bool has_last_stamp_ = false;
  double last_stamp_ = 0.0;
  bool has_prev_gyro_ = false;
  Eigen::Vector3d prev_gyro_corrected_ = Eigen::Vector3d::Zero();

  std::deque<double> gyro_norm_window_;
  std::deque<double> accel_norm_window_;

  // ESKF error covariance (dp, dv, dtheta, db_g, db_a). Zero until initEskf().
  Eigen::Matrix<double, 15, 15> P_ = Eigen::Matrix<double, 15, 15>::Zero();
  bool eskf_initialized_ = false;
};

/// Integrates IMU-only strapdown dead reckoning and samples one RELATIVE
/// pose (first = identity) per entry of frame_timestamps.
std::vector<Eigen::Matrix4d> integrateImuTrajectory(
    const std::vector<ImuReading>& imu,
    const std::vector<double>& frame_timestamps,
    const ImuDeadReckoningParams& params, long* zupt_frames = nullptr,
    std::string* error = nullptr);

}  // namespace imu_dead_reckoning
}  // namespace localization_zoo
