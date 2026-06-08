#pragma once

#include "odonet/cnn1d.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <deque>
#include <string>
#include <vector>

namespace localization_zoo {
namespace odonet {

struct ImuReading {
  double stamp = 0.0;
  Eigen::Vector3d gyro = Eigen::Vector3d::Zero();   // rad/s
  Eigen::Vector3d accel = Eigen::Vector3d::Zero();    // m/s^2
};

struct OdoNetParams {
  /// Path to exported CNN weights (JSON).
  std::string weights_path = "papers/odonet/weights/odonet_kitti.json";

  int window_size = 50;
  int channels = 6;
  double speed_scale = 30.0;
  double imu_rate_hz = 50.0;

  /// Data cleaning: subtract these biases before the CNN (paper §III-B).
  Eigen::Vector3d gyro_bias = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel_bias = Eigen::Vector3d::Zero();
  /// Optional body→vehicle rotation (3×3 row-major). Identity when unknown.
  Eigen::Matrix3d body_to_vehicle = Eigen::Matrix3d::Identity();

  /// Zero-velocity threshold (paper: 0.1 m/s).
  double zupt_speed_threshold = 0.1;
  /// Exponential low-pass on regressed speed (approximates paper FIR 0.1 Hz).
  double speed_filter_alpha = 0.15;

  /// NHC: lateral/vertical body velocity damping toward zero.
  double nhc_gain = 8.0;
  bool enable_nhc = true;
  bool enable_zupt = true;
  bool enable_cnn_speed = true;

  /// Vehicle forward axis in body frame (KITTI Velodyne: x).
  int forward_axis = 0;
};

struct OdoNetState {
  Eigen::Matrix3d R = Eigen::Matrix3d::Identity();  // body→world
  Eigen::Vector3d v = Eigen::Vector3d::Zero();      // world velocity
  Eigen::Vector3d p = Eigen::Vector3d::Zero();    // world position
  double last_stamp = 0.0;
  double filtered_speed = 0.0;
  bool initialized = false;
};

struct OdoNetStepStats {
  double regressed_speed = 0.0;
  double filtered_speed = 0.0;
  bool zupt_active = false;
  bool cnn_used = false;
};

/// OdoNet pseudo-odometer + strapdown INS with NHC/ZUPT (arXiv:2109.03091).
class OdoNetPipeline {
public:
  explicit OdoNetPipeline(const OdoNetParams& params = OdoNetParams());

  bool loadWeights(std::string* error = nullptr);
  void reset();
  void setInitialPose(const Eigen::Matrix3d& R, const Eigen::Vector3d& p);

  /// Integrate one IMU sample; may update velocity from CNN speed + NHC/ZUPT.
  OdoNetStepStats processImu(const ImuReading& imu);

  /// Pose at current INS state (4×4, anchor-compatible).
  Eigen::Matrix4d pose() const;

  const OdoNetState& state() const { return state_; }
  const Cnn1dModel& model() const { return model_; }

private:
  Eigen::Vector3d bodyForwardAxis() const;
  void applyNhc(Eigen::Vector3d* v_body) const;
  void pushWindowSample(const Eigen::Vector3d& gyro,
                        const Eigen::Vector3d& accel);
  double regressSpeed() const;

  OdoNetParams params_;
  Cnn1dModel model_;
  bool weights_loaded_ = false;
  OdoNetState state_;
  std::deque<Eigen::Vector3d> gyro_window_;
  std::deque<Eigen::Vector3d> accel_window_;
};

/// Run dead-reckoning on IMU stream; sample poses at ``query_stamps``.
std::vector<Eigen::Matrix4d> integrateImuTrajectory(
    const std::vector<ImuReading>& imu_samples,
    const std::vector<double>& query_stamps, const OdoNetParams& params,
    long* zupt_frames = nullptr, long* cnn_frames = nullptr,
    std::string* error = nullptr);

}  // namespace odonet
}  // namespace localization_zoo
