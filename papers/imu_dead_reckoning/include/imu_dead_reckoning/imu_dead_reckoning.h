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

  // Propagation.
  bool midpoint_integration = true;  // gyro midpoint rule; false = forward Euler.
  double max_dt = 0.5;               // clamp dt across sample gaps.

  // Opt-in aids (default OFF: pure dead reckoning).
  bool enable_zupt = false;
  double zupt_gyro_threshold = 0.05;    // rad/s, window mean gyro norm.
  double zupt_accel_tolerance = 0.8;    // m/s^2, | |a| - g | window gate.
  int zupt_window = 10;                 // samples.

  /// Non-holonomic constraint: damp lateral/vertical body velocity toward zero
  /// (vehicle cannot slide sideways). Hard projection when nhc_gain <= 0;
  /// soft exponential pull when nhc_gain > 0 (same dt-scaled form as NHC-Net).
  bool enable_nhc = false;
  double nhc_gain = 0.0;  // 0 = hard forward-only projection.
  int forward_axis = 0;   // body axis index for forward (0=x, KITTI Velodyne).

  /// Estimate accelerometer bias from the static window residual after gravity
  /// alignment (captures scale/orthogonal bias not absorbed by g_w init).
  bool estimate_accel_bias = false;
};

struct ImuDeadReckoningStepStats {
  bool zupt_active = false;
  bool nhc_active = false;
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

  bool staticInitialized() const { return static_initialized_; }

  /// Non-empty if the static-init window's gyro noise exceeded
  /// static_gyro_std_gate (informational only; init still proceeds).
  const std::string& warning() const { return warning_; }

private:
  void finalizeStaticInit(const std::vector<ImuReading>& window);
  void applyNhc(Eigen::Vector3d* v_body, double dt, bool* nhc_active) const;
  Eigen::Vector3d bodyForwardAxis() const;

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
  std::vector<ImuReading> static_buffer_;
  std::string warning_;

  bool has_last_stamp_ = false;
  double last_stamp_ = 0.0;
  bool has_prev_gyro_ = false;
  Eigen::Vector3d prev_gyro_corrected_ = Eigen::Vector3d::Zero();

  std::deque<double> gyro_norm_window_;
  std::deque<double> accel_norm_window_;
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
