#pragma once

#include "nn_zupt/cnn1d.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <deque>
#include <string>
#include <vector>

namespace localization_zoo {
namespace nn_zupt {

struct ImuReading {
  double stamp = 0.0;
  Eigen::Vector3d gyro = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel = Eigen::Vector3d::Zero();
};

struct NnZuptParams {
  std::string weights_path = "papers/nn_zupt/weights/nn_zupt_kitti.json";
  int window_size = 50;
  int channels = 6;

  Eigen::Vector3d gyro_bias = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel_bias = Eigen::Vector3d::Zero();
  Eigen::Matrix3d body_to_vehicle = Eigen::Matrix3d::Identity();

  /// NN stop probability threshold (paper adaptive detection).
  double stop_prob_threshold = 0.5;
  /// Fixed-threshold fallback (accel/gyro SHOE-like ablation).
  bool use_threshold_detector = false;
  double gyro_stop_threshold = 0.05;
  double accel_norm_tolerance = 0.8;

  bool enable_nhc = true;
  double nhc_gain = 8.0;
  bool enable_zupt = true;

  int forward_axis = 0;
};

struct NnZuptStepStats {
  double stop_prob = 0.0;
  bool zupt_active = false;
  bool nn_used = false;
};

/// NN-ZUPT vehicle INS dead reckoning (Li et al., Meas. Sci. Technol. 2023).
class NnZuptPipeline {
public:
  explicit NnZuptPipeline(const NnZuptParams& params = NnZuptParams());

  bool loadWeights(std::string* error = nullptr);
  void reset();
  void setInitialPose(const Eigen::Matrix3d& R, const Eigen::Vector3d& p);

  NnZuptStepStats processImu(const ImuReading& imu);
  Eigen::Matrix4d pose() const;

private:
  void pushWindow(const Eigen::Vector3d& gyro, const Eigen::Vector3d& accel);
  ZuptForwardResult infer() const;
  bool thresholdStop(const Eigen::Vector3d& gyro,
                     const Eigen::Vector3d& accel) const;
  void applyNhc(Eigen::Vector3d* v_body) const;
  Eigen::Vector3d bodyForwardAxis() const;

  NnZuptParams params_;
  ZuptCnnModel model_;
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
    const std::vector<double>& query_stamps, const NnZuptParams& params,
    long* zupt_frames = nullptr, long* nn_frames = nullptr,
    std::string* error = nullptr);

}  // namespace nn_zupt
}  // namespace localization_zoo
