#include "vibration_lio/vibration_lio.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::vibration_lio;
using localization_zoo::imu_preintegration::ImuSample;

namespace {
std::vector<Eigen::Vector3d> makeRoom(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-12, 12);
  std::uniform_real_distribution<double> z(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 12000; i++) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 6000; i++) pts.emplace_back(xy(rng), 10 + n(rng), z(rng));
  for (int i = 0; i < 6000; i++) pts.emplace_back(-10 + n(rng), xy(rng), z(rng));
  for (int i = 0; i < 6000; i++) pts.emplace_back(xy(rng), -10 + n(rng), z(rng));
  return pts;
}

// 振動つき IMU (gyro/accel に分散を与える)。
std::vector<ImuSample> vibImu(int f, double vib, std::mt19937& rng) {
  std::normal_distribution<double> g(0, vib);
  std::vector<ImuSample> imu;
  for (int k = 0; k < 10; k++) {
    ImuSample s;
    s.timestamp = f + 0.1 * k;
    s.gyro = Eigen::Vector3d(g(rng), g(rng), g(rng));
    s.accel = Eigen::Vector3d(g(rng), g(rng), 9.81 + g(rng));
    imu.push_back(s);
  }
  return imu;
}
}  // namespace

TEST(VibrationLIO, FirstFrameInitializesMap) {
  VibrationLIOPipeline pipeline(VibrationLIOParams{});
  const auto result = pipeline.registerFrame({}, {});
  EXPECT_TRUE(result.converged);
}

TEST(VibrationLIO, TracksTranslation) {
  std::mt19937 rng(42), irng(1);
  VibrationLIOParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 40;
  VibrationLIOPipeline pipeline(params);

  auto scene = makeRoom(rng);
  for (int f = 0; f < 4; f++) {
    const Eigen::Vector3d t(f * 0.5, 0, 0);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 1 && ps.norm() < 25) scan.push_back(ps);
    }
    const auto result = pipeline.registerFrame(scan, vibImu(f, 0.02, irng));
    EXPECT_TRUE(result.converged);
    if (f > 0) EXPECT_TRUE(result.used_imu);
  }
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(1.5, 0, 0)).norm();
  EXPECT_LT(err, 1.0);
}

TEST(VibrationLIO, VibrationIntensityIncreasesWithImuNoise) {
  // 振動の大きい IMU ほど推定振動強度 k_ω/k_v が大きくなるはず。
  std::mt19937 rng(42), low(1), high(2);
  VibrationLIOParams params;
  params.voxel_size = 1.0;
  auto scene = makeRoom(rng);

  auto runWith = [&](double vib, std::mt19937& irng) {
    VibrationLIOPipeline pipe(params);
    double last_k = 0.0;
    for (int f = 0; f < 3; f++) {
      std::vector<Eigen::Vector3d> scan;
      const Eigen::Vector3d t(f * 0.3, 0, 0);
      for (const auto& p : scene) {
        const Eigen::Vector3d ps = p - t;
        if (ps.norm() > 1 && ps.norm() < 25) scan.push_back(ps);
      }
      const auto r = pipe.registerFrame(scan, vibImu(f, vib, irng));
      last_k = r.vibration_omega;
    }
    return last_k;
  };

  const double k_low = runWith(0.01, low);
  const double k_high = runWith(0.3, high);
  EXPECT_GT(k_high, k_low * 2.0);  // 振動強度推定が IMU ノイズに追従
}
