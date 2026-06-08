#include "nn_zupt/cnn1d.h"
#include "nn_zupt/nn_zupt.h"

#include <gtest/gtest.h>

using namespace localization_zoo::nn_zupt;

namespace {

ZuptCnnModel tinyModel() {
  ZuptCnnModel m;
  m.window_size = 4;
  m.channels = 2;
  Conv1dLayer c0{2, 2, 3,
                 {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0},
                 {0, 0}};
  m.convs.push_back(c0);
  m.pool_kernel.push_back(2);
  m.fcs.push_back(DenseLayer{4, 4, std::vector<double>(16, 0.1), {0, 0, 0, 0}});
  m.fcs.push_back(DenseLayer{4, 1, std::vector<double>(4, 0.01), {0}});
  return m;
}

}  // namespace

TEST(NnZupt, ForwardProducesProbability) {
  const auto m = tinyModel();
  std::vector<double> window(8, 0.1);
  const auto out = forwardZuptCnn(m, window);
  EXPECT_GE(out.stop_prob, 0.0);
  EXPECT_LE(out.stop_prob, 1.0);
}

TEST(NnZupt, ThresholdDetectorZerosVelocity) {
  NnZuptParams params;
  params.use_threshold_detector = true;
  params.weights_path = "/nonexistent";
  NnZuptPipeline pipe(params);
  pipe.setInitialPose(Eigen::Matrix3d::Identity(), Eigen::Vector3d::Zero());
  ImuReading imu;
  imu.stamp = 0.02;
  imu.gyro.setZero();
  imu.accel = Eigen::Vector3d(0.0, 0.0, -9.80665);
  const auto stats = pipe.processImu(imu);
  EXPECT_TRUE(stats.zupt_active);
}

TEST(NnZupt, IntegrateTrajectoryRequiresWeights) {
  std::vector<ImuReading> imu(10);
  for (int i = 0; i < 10; ++i) {
    imu[i].stamp = 0.02 * i;
  }
  NnZuptParams params;
  params.weights_path = "/nonexistent";
  auto poses = integrateImuTrajectory(imu, {0.0, 0.1}, params);
  EXPECT_TRUE(poses.empty());
}
