#include "odonet/cnn1d.h"
#include "odonet/odonet.h"

#include <gtest/gtest.h>

#include <cmath>
#include <fstream>

using namespace localization_zoo::odonet;

namespace {

Cnn1dModel tinyModel() {
  Cnn1dModel m;
  m.window_size = 4;
  m.channels = 2;
  m.speed_scale = 10.0;

  Conv1dLayer c0;
  c0.in_channels = 2;
  c0.out_channels = 2;
  c0.kernel_size = 3;
  c0.bias = {0.0, 0.0};
  c0.weight = {
      1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
  };
  m.convs.push_back(c0);
  m.pool_kernel.push_back(2);

  DenseLayer fc;
  fc.in_features = 4;
  fc.out_features = 1;
  fc.bias = {0.0};
  fc.weight = {0.1, 0.1, 0.1, 0.1};
  m.fcs.push_back(fc);
  return m;
}

}  // namespace

TEST(OdoNet, CnnForwardIsNonNegative) {
  const Cnn1dModel model = tinyModel();
  std::vector<double> window = {
      0.1, 0.0, 0.2, 0.0, 0.1, 0.0, 0.2, 0.0,
  };
  const double speed = forwardSpeed(model, window);
  EXPECT_GE(speed, 0.0);
}

TEST(OdoNet, DataCleaningRemovesMeanBias) {
  OdoNetParams params;
  params.enable_cnn_speed = false;
  params.weights_path = "/nonexistent";
  params.gyro_bias = Eigen::Vector3d(0.01, -0.02, 0.005);
  params.accel_bias = Eigen::Vector3d(0.1, 0.0, -0.05);

  OdoNetPipeline pipe(params);
  pipe.setInitialPose(Eigen::Matrix3d::Identity(), Eigen::Vector3d::Zero());

  ImuReading imu;
  imu.stamp = 0.02;
  imu.gyro = params.gyro_bias;
  imu.accel = params.accel_bias;
  pipe.processImu(imu);

  imu.stamp = 0.04;
  imu.gyro = params.gyro_bias + Eigen::Vector3d(0.001, 0.0, 0.0);
  imu.accel = params.accel_bias;
  const auto stats = pipe.processImu(imu);
  EXPECT_FALSE(stats.cnn_used);
  EXPECT_NEAR(pipe.state().p.norm(), 0.0, 1e-6);
}

TEST(OdoNet, NhcKeepsForwardVelocity) {
  OdoNetParams params;
  params.enable_cnn_speed = false;
  params.enable_zupt = false;
  params.nhc_gain = 8.0;
  OdoNetPipeline pipe(params);
  pipe.setInitialPose(Eigen::Matrix3d::Identity(), Eigen::Vector3d::Zero());

  for (int i = 1; i <= 5; ++i) {
    ImuReading imu;
    imu.stamp = 0.02 * i;
    imu.gyro.setZero();
    imu.accel = Eigen::Vector3d(0.0, 0.0, 0.0);
    pipe.processImu(imu);
  }
  EXPECT_NEAR(pipe.state().p.x(), 0.0, 1e-9);
  EXPECT_NEAR(pipe.state().p.y(), 0.0, 1e-9);
}

TEST(OdoNet, IntegrateImuTrajectoryMatchesQueryCount) {
  std::vector<ImuReading> imu;
  for (int i = 0; i < 20; ++i) {
    ImuReading r;
    r.stamp = 0.02 * i;
    r.gyro.setZero();
    r.accel.setZero();
    imu.push_back(r);
  }
  const std::vector<double> queries = {0.0, 0.1, 0.2, 0.3};
  OdoNetParams params;
  params.enable_cnn_speed = false;
  auto poses = integrateImuTrajectory(imu, queries, params);
  ASSERT_EQ(poses.size(), queries.size());
}
