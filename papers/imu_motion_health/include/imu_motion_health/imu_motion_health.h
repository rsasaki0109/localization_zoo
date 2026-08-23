#pragma once

// A small, dependency-light IMU-only motion and sensor-health front end.
//
// The class in this header deliberately does not depend on ROS, a point-cloud
// library, or a particular sensor driver.  A driver only has to turn its
// messages into ImuSample and feed them to process().  The output is a
// snapshot, rather than a callback, which makes the component convenient in a
// real-time loop as well as in an offline replay or a test.

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>

namespace localization_zoo {
namespace imu_motion_health {

struct ImuSample {
  double timestamp = 0.0;             // seconds, monotonic sensor time
  Eigen::Vector3d gyro = Eigen::Vector3d::Zero();   // rad/s
  Eigen::Vector3d accel = Eigen::Vector3d::Zero();  // m/s^2 (specific force)
};

// The alias is useful when migrating code that already calls its input an
// "IMU reading".  It is intentionally the same type, not a second wire type.
using ImuReading = ImuSample;

struct ImuMotionHealthParams {
  // Startup calibration.  The first window is assumed to be at rest.  This is
  // an explicit, honest assumption: an IMU cannot tell a constant-velocity
  // cruise from rest without another sensor.
  double startup_duration_s = 1.0;
  std::size_t startup_min_samples = 20;
  bool estimate_gyro_bias = true;
  bool estimate_accel_bias = false;
  double gravity_magnitude = 9.80665;
  double startup_max_gyro_std = 0.08;       // rad/s, vector RMS std
  double startup_max_accel_norm_std = 0.35; // m/s^2
  double startup_gravity_tolerance = 1.5;   // m/s^2

  // Timestamp/data integrity.  A gap is reported and the corresponding
  // integration interval is discarded (never silently integrated with a
  // made-up dt).
  double max_gap_s = 0.25;
  double min_dt_s = 1e-7;

  // Set these to the actual full-scale limits of the selected IMU.  A sample
  // at or beyond a limit is counted as saturated and is not integrated.
  double gyro_saturation_rad_s = 34.0;
  double accel_saturation_mps2 = 156.9;  // 16 g

  // Motion gates.  They are applied to short rolling windows where possible,
  // so one noisy sample does not make a stationary device "moving".
  double stationary_gyro_threshold = 0.06;       // rad/s
  double stationary_accel_tolerance = 0.45;      // m/s^2 from |g|
  double moving_gyro_threshold = 0.16;            // rad/s
  double moving_accel_threshold = 0.65;           // m/s^2, linear accel
  double impact_accel_threshold = 25.0;           // m/s^2 resultant
  double impact_gyro_threshold = 8.0;             // rad/s resultant
  double fall_freefall_threshold = 2.5;           // m/s^2 resultant
  double fall_min_duration_s = 0.08;
  double event_hold_duration_s = 0.20;
  // A fall can end in a quiet, permanently tilted pose without a clean
  // free-fall sample.  Detect that case from the estimated body tilt and, when
  // the gyro has not captured the pose change, from the gravity direction.
  double tilt_angle_threshold_deg = 55.0;
  double tilt_min_duration_s = 0.15;
  double vibration_rms_threshold = 1.0;           // m/s^2 norm RMS
  std::size_t motion_window_samples = 20;

  // Slow stationary-only bias tracking and gravity leveling.  Both are
  // deliberately conservative; they never touch yaw, which acceleration
  // cannot observe.
  double stationary_bias_gain = 0.01;
  double leveling_gain = 0.03;

  // Integration guard.  The public position/velocity are explicitly
  // short-term relative dead reckoning, not globally bounded localization.
  double max_integration_dt_s = 0.10;
  bool zero_velocity_when_stationary = true;
};

enum class MotionState {
  kInitializing,
  kStationary,
  kMoving,
  kImpact,
  kFall,
  kVibration,
  kUnknown,

  // Readable aliases for applications that do not use the repository's k-
  // prefix convention.
  Initializing = kInitializing,
  Stationary = kStationary,
  Moving = kMoving,
  Impact = kImpact,
  Fall = kFall,
  Vibration = kVibration,
  Unknown = kUnknown,
};

enum class HealthState {
  kInitializing,
  kReady,
  kDegraded,
  kInvalid,

  Initializing = kInitializing,
  Ready = kReady,
  Degraded = kDegraded,
  Invalid = kInvalid,
};

struct ImuMotionHealthCounters {
  std::uint64_t samples = 0;
  std::uint64_t accepted_samples = 0;
  std::uint64_t rejected_samples = 0;
  std::uint64_t nonfinite_samples = 0;
  std::uint64_t nonmonotonic_timestamps = 0;
  std::uint64_t timestamp_gaps = 0;
  std::uint64_t gyro_saturated_samples = 0;
  std::uint64_t accel_saturated_samples = 0;

  std::uint64_t stationary_samples = 0;
  std::uint64_t moving_samples = 0;
  std::uint64_t impact_samples = 0;
  std::uint64_t fall_samples = 0;
  std::uint64_t vibration_samples = 0;
};

struct ImuMotionHealthState {
  // Stream/data status.
  bool sample_accepted = false;
  bool integrated = false;
  bool nonfinite = false;
  bool nonmonotonic = false;
  bool gap = false;
  bool gyro_saturated = false;
  bool accel_saturated = false;
  bool saturated = false;
  double timestamp = 0.0;
  double dt = 0.0;
  double sample_rate_hz = 0.0;

  // Startup calibration and quality evidence.
  bool startup_complete = false;
  bool startup_quality_ok = false;
  bool startup_bias_trusted = false;
  std::size_t startup_sample_count = 0;
  double startup_gyro_std = 0.0;
  double startup_accel_norm_std = 0.0;
  double startup_gravity_norm = 0.0;

  MotionState motion_state = MotionState::kInitializing;
  HealthState health_state = HealthState::kInitializing;
  // Convenience booleans for telemetry serializers and simple clients.
  bool stationary = false;
  bool moving = false;
  bool impact = false;
  bool fall = false;
  bool vibration = false;
  bool degraded = false;
  double confidence = 0.0;  // [0, 1], confidence in this snapshot

  // Instantaneous/rolling metrics.
  double gyro_norm = 0.0;
  double accel_norm = 0.0;
  double linear_accel_norm = 0.0;
  double vibration_rms = 0.0;
  double freefall_duration_s = 0.0;
  double tilt_angle_rad = 0.0;
  double tilt_angle_deg = 0.0;
  bool tilted = false;

  // IMU calibration estimates.  accel_bias is exposed even when estimation
  // is disabled (in that case it remains zero).
  Eigen::Vector3d gyro_bias = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel_bias = Eigen::Vector3d::Zero();
  Eigen::Vector3d gravity_world = Eigen::Vector3d(0.0, 0.0, 9.80665);
  double gravity_magnitude = 9.80665;

  // World-from-body orientation and short-term relative motion, anchored at
  // the first post-startup state.  Position is not globally observable from
  // six-axis IMU data and will drift over long runs.
  Eigen::Quaterniond orientation = Eigen::Quaterniond::Identity();
  Eigen::Vector3d velocity = Eigen::Vector3d::Zero();
  Eigen::Vector3d position = Eigen::Vector3d::Zero();
  Eigen::Vector3d relative_velocity = Eigen::Vector3d::Zero();
  Eigen::Vector3d relative_position = Eigen::Vector3d::Zero();

  std::string diagnostic;
};

using ImuMotionHealthSnapshot = ImuMotionHealthState;
using ImuHealthState = ImuMotionHealthState;

const char* motionStateName(MotionState state);
const char* healthStateName(HealthState state);

// Stable, dependency-free JSON for a snapshot.  It contains scalar/vector
// fields useful for a CLI or a log file and intentionally has no locale
// dependence.  The member equivalent is ImuMotionHealth::toJson().
std::string toJson(const ImuMotionHealthState& state);

class ImuMotionHealth {
 public:
  using Params = ImuMotionHealthParams;
  using Sample = ImuSample;
  using State = ImuMotionHealthState;
  using Snapshot = ImuMotionHealthState;
  using Counters = ImuMotionHealthCounters;

  explicit ImuMotionHealth(
      const ImuMotionHealthParams& params = ImuMotionHealthParams());

  void reset();

  // Process exactly one sample.  Invalid samples still produce a snapshot
  // with the relevant diagnostic flag and increment a counter; they are never
  // integrated.  Returning by value makes it safe for a caller to enqueue the
  // result while snapshot() remains a cheap read-only view.
  ImuMotionHealthState process(const ImuSample& sample);

  ImuMotionHealthState process(double timestamp, const Eigen::Vector3d& gyro,
                               const Eigen::Vector3d& accel) {
    ImuSample sample;
    sample.timestamp = timestamp;
    sample.gyro = gyro;
    sample.accel = accel;
    return process(sample);
  }

  const ImuMotionHealthState& snapshot() const { return state_; }
  const ImuMotionHealthCounters& counters() const { return counters_; }
  const ImuMotionHealthParams& params() const { return params_; }

  bool startupComplete() const { return startup_complete_; }
  bool startupQualityOk() const { return startup_quality_ok_; }
  bool ready() const { return state_.health_state == HealthState::kReady; }
  bool degraded() const {
    return state_.health_state == HealthState::kDegraded;
  }
  std::string toJson() const { return imu_motion_health::toJson(state_); }

 private:
  void finalizeStartup();
  void classify(double timestamp, double dt, bool had_data_error,
                const Eigen::Vector3d& corrected_accel);
  void updateStateAliases();
  void updateConfidence(bool had_data_error);
  void makeDiagnostic(bool had_data_error);
  void clearRuntimeState();

  ImuMotionHealthParams params_;
  ImuMotionHealthState state_;
  ImuMotionHealthCounters counters_;

  bool have_timestamp_ = false;
  double last_timestamp_ = 0.0;
  double first_timestamp_ = 0.0;
  double last_integration_timestamp_ = 0.0;
  double anomaly_until_ = 0.0;
  bool have_anomaly_timestamp_ = false;

  bool startup_complete_ = false;
  bool startup_quality_ok_ = false;
  bool startup_had_data_error_ = false;
  std::deque<ImuSample> startup_samples_;
  double freefall_start_timestamp_ = 0.0;
  bool freefall_active_ = false;
  double impact_until_ = 0.0;
  double fall_until_ = 0.0;
  double vibration_until_ = 0.0;
  bool tilt_active_ = false;
  double tilt_start_timestamp_ = 0.0;

  std::deque<double> gyro_norm_window_;
  std::deque<double> accel_norm_window_;
  std::deque<double> linear_accel_norm_window_;
  std::deque<Eigen::Vector3d> accel_window_;
  double sample_rate_ema_hz_ = 0.0;

  Eigen::Quaterniond orientation_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d velocity_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d position_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d gyro_bias_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel_bias_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d gravity_world_ = Eigen::Vector3d(0.0, 0.0, 9.80665);
  double gravity_magnitude_ = 9.80665;
};

// Names used by a few applications that call all streaming filters
// "pipelines".  This does not create a second implementation or ABI.
using ImuMotionHealthPipeline = ImuMotionHealth;
using ImuHealthMonitor = ImuMotionHealth;

}  // namespace imu_motion_health
}  // namespace localization_zoo
