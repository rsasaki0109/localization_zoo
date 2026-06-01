#include "rko_lio/rko_lio.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::rko_lio;
using localization_zoo::imu_preintegration::ImuSample;

namespace {
std::vector<Eigen::Vector3d> makeScene(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-30, 30);
  std::uniform_real_distribution<double> z(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 3000; i++) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 1000; i++) pts.emplace_back(xy(rng), 15 + n(rng), z(rng));
  for (int i = 0; i < 1000; i++) pts.emplace_back(-15 + n(rng), xy(rng), z(rng));
  return pts;
}

// 一定 yaw レートを表す合成 IMU バッチ ([t0, t0+dt] を分割)。
std::vector<ImuSample> makeYawImu(double yaw_rate, double dt, double t0,
                                  int samples) {
  std::vector<ImuSample> out;
  const double step = dt / samples;
  for (int k = 0; k <= samples; k++) {
    ImuSample s;
    s.timestamp = t0 + k * step;
    s.gyro = Eigen::Vector3d(0, 0, yaw_rate);
    s.accel = Eigen::Vector3d(0, 0, 9.81);
    out.push_back(s);
  }
  return out;
}
}  // namespace

TEST(RKOLIO, FirstFrameEmptyInitializesMap) {
  RKOLIOPipeline pipeline(RKOLIOParams{});
  const auto result = pipeline.registerFrame({}, {});
  EXPECT_TRUE(result.converged);
  EXPECT_FALSE(result.used_imu);
  EXPECT_TRUE(pipeline.pose().isApprox(result.pose));
}

TEST(RKOLIO, MultiFrameTracksWithImuPrior) {
  std::mt19937 rng(42);
  RKOLIOParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;
  RKOLIOPipeline pipeline(params);

  auto scene = makeScene(rng);
  const double dt = 0.1;
  const double yaw_per_frame = 0.02;  // rad
  bool saw_imu = false;
  for (int f = 0; f < 3; f++) {
    const Eigen::Matrix3d R =
        Eigen::AngleAxisd(f * yaw_per_frame, Eigen::Vector3d::UnitZ())
            .toRotationMatrix();
    const Eigen::Vector3d t(f * 1.0, 0, 1);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = R.transpose() * (p - t);
      if (ps.norm() > 3 && ps.norm() < 30) scan.push_back(ps);
    }
    std::vector<ImuSample> imu;
    if (f > 0) imu = makeYawImu(yaw_per_frame / dt, dt, f * dt, 10);
    const auto result = pipeline.registerFrame(scan, imu);
    EXPECT_TRUE(result.converged);
    if (f > 0 && result.used_imu) saw_imu = true;
  }
  EXPECT_TRUE(saw_imu);  // IMU 回転先験が使われたこと
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(2, 0, 0)).norm();
  EXPECT_LT(err, 2.0);
}

TEST(RKOLIO, FallsBackWithoutImu) {
  std::mt19937 rng(7);
  RKOLIOParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;
  RKOLIOPipeline pipeline(params);

  auto scene = makeScene(rng);
  for (int f = 0; f < 3; f++) {
    const Eigen::Vector3d t(f * 1.0, 0, 1);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 3 && ps.norm() < 30) scan.push_back(ps);
    }
    const auto result = pipeline.registerFrame(scan, {});  // IMU 無し
    EXPECT_TRUE(result.converged);
    EXPECT_FALSE(result.used_imu);
  }
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(2, 0, 0)).norm();
  EXPECT_LT(err, 2.0);
}
