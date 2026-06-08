#pragma once

#include "nhc_net/cnn1d.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <deque>
#include <string>
#include <vector>

namespace localization_zoo {
namespace nhc_net {

struct ImuReading {
  double stamp = 0.0;
  Eigen::Vector3d gyro = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel = Eigen::Vector3d::Zero();
};

struct NhcNetParams {
  std::string weights_path = "papers/nhc_net/weights/nhc_net_kitti.json";
  int window_size = 50;
  int channels = 6;

  Eigen::Vector3d gyro_bias = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel_bias = Eigen::Vector3d::Zero();
  Eigen::Matrix3d body_to_vehicle = Eigen::Matrix3d::Identity();

  /// Soft NHC pull strength toward CNN-predicted lateral/vertical velocity.
  double nhc_gain = 6.0;
  /// Scale NHC gain by class confidence (motion state reliability).
  bool adaptive_gain = true;
  bool enable_zupt = true;
  int stop_class_id = 0;

  int forward_axis = 0;
};

struct NhcNetStepStats {
  int motion_class = 0;
  double lateral_target = 0.0;
  double vertical_target = 0.0;
  double class_confidence = 0.0;
  bool zupt_active = false;
};

/// VMSC-based adaptive NHC dead reckoning (Li et al., GPS Solutions 2023).
class NhcNetPipeline {
public:
  explicit NhcNetPipeline(const NhcNetParams& params = NhcNetParams());

  bool loadWeights(std::string* error = nullptr);
  void reset();
  void setInitialPose(const Eigen::Matrix3d& R, const Eigen::Vector3d& p);

  NhcNetStepStats processImu(const ImuReading& imu);
  Eigen::Matrix4d pose() const;

private:
  void pushWindow(const Eigen::Vector3d& gyro, const Eigen::Vector3d& accel);
  VmscForwardResult infer() const;
  Eigen::Vector3d bodyForwardAxis() const;

  NhcNetParams params_;
  VmscCnnModel model_;
  bool weights_loaded_ = false;
  Eigen::Matrix3d R_ = Eigen::Matrix3d::Identity();
  Eigen::Vector3d v_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d p_ = Eigen::Vector3d::Zero();
  double last_stamp_ = 0.0;
  bool initialized_ = false;
  std::deque<Eigen::Vector3d> gyro_window_;
  std::deque<Eigen::Vector3d> accel_window_;
};

std::vector<Eigen::Matrix4d> integrateImuTrajectory(
    const std::vector<ImuReading>& imu_samples,
    const std::vector<double>& query_stamps, const NhcNetParams& params,
    long* zupt_frames = nullptr, long* cnn_frames = nullptr,
    std::string* error = nullptr);

}  // namespace nhc_net
}  // namespace localization_zoo
