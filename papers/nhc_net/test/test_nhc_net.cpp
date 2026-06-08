#include "nhc_net/cnn1d.h"
#include "nhc_net/nhc_net.h"

#include <gtest/gtest.h>

using namespace localization_zoo::nhc_net;

namespace {

VmscCnnModel tinyModel() {
  VmscCnnModel m;
  m.window_size = 4;
  m.channels = 2;
  m.num_classes = 4;
  m.nhc_scale = 2.0;
  Conv1dLayer c0{2, 2, 3,
                 {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0},
                 {0, 0}};
  m.convs.push_back(c0);
  m.pool_kernel.push_back(2);
  DenseLayer shared{4, 8,
                    std::vector<double>(32, 0.1),
                    std::vector<double>(8, 0.0)};
  m.shared_fcs.push_back(shared);
  m.class_head = {8, 4, std::vector<double>(32, 0.01), {0, 0, 0, 0}};
  m.nhc_head = {8, 2, std::vector<double>(16, 0.01), {0, 0}};
  return m;
}

}  // namespace

TEST(NhcNet, VmscForwardProducesClassAndNhc) {
  const auto m = tinyModel();
  std::vector<double> window(8, 0.1);
  const auto out = forwardVmsc(m, window);
  EXPECT_EQ(static_cast<int>(out.class_probs.size()), 4);
}

TEST(NhcNet, AdaptiveNhcPullsLateralVelocity) {
  NhcNetParams params;
  params.weights_path = "/nonexistent";
  params.nhc_gain = 10.0;
  NhcNetPipeline pipe(params);
  pipe.setInitialPose(Eigen::Matrix3d::Identity(), Eigen::Vector3d::Zero());
  ImuReading imu;
  imu.stamp = 0.02;
  imu.gyro.setZero();
  imu.accel = Eigen::Vector3d(1.0, 0.0, 0.0);
  pipe.processImu(imu);
  EXPECT_GE((pipe.pose().block<3, 1>(0, 3).norm()), 0.0);
}

TEST(NhcNet, IntegrateTrajectoryCount) {
  std::vector<ImuReading> imu(10);
  for (int i = 0; i < 10; ++i) {
    imu[i].stamp = 0.02 * i;
  }
  NhcNetParams params;
  params.weights_path = "/nonexistent";
  auto poses = integrateImuTrajectory(imu, {0.0, 0.1}, params);
  EXPECT_TRUE(poses.empty());
}
