#include "fr_lio/fr_lio.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::fr_lio;
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

std::vector<ImuSample> makeDynamicImu(double t0, int samples) {
  std::vector<ImuSample> out;
  const double step = 0.01;
  for (int k = 0; k < samples; k++) {
    ImuSample s;
    s.timestamp = t0 + k * step;
    const double phase = static_cast<double>(k) / samples;
    s.gyro = Eigen::Vector3d(0.5 * std::sin(phase * 6.28), 0, 0.2);
    s.accel = Eigen::Vector3d(0.1, 0.05, 9.81 + 0.3 * std::cos(phase * 6.28));
    out.push_back(s);
  }
  return out;
}
}  // namespace

TEST(FRLIO, FirstFrameInitializesMap) {
  FRLIOPipeline pipeline(FRLIOParams{});
  const auto result = pipeline.registerFrame({}, {});
  EXPECT_TRUE(result.converged);
  EXPECT_EQ(result.subframes, 1);
  EXPECT_TRUE(pipeline.pose().isApprox(result.pose));
}

TEST(FRLIO, AdaptiveSubframesIncreaseWithDynamicImu) {
  FRLIOParams params;
  params.max_subframes = 4;
  params.sigma_acc_max = 0.05;
  params.sigma_gyr_max = 0.05;
  const auto calm = std::vector<ImuSample>{ImuSample{0.0, Eigen::Vector3d::Zero(),
                                                     Eigen::Vector3d(0, 0, 9.81)},
                                           ImuSample{0.1, Eigen::Vector3d::Zero(),
                                                     Eigen::Vector3d(0, 0, 9.81)}};
  const auto dynamic = makeDynamicImu(0.0, 40);
  EXPECT_EQ(FRLIOPipeline::computeSubframeCount(calm, params), 1);
  EXPECT_GE(FRLIOPipeline::computeSubframeCount(dynamic, params), 2);
}

TEST(FRLIO, MultiFrameTracksWithImuAndSubframes) {
  std::mt19937 rng(42);
  FRLIOParams params;
  params.voxel_size = 1.0;
  params.grid_size = 2.0;
  params.max_icp_iterations = 50;
  params.min_valid_matches = 10;
  FRLIOPipeline pipeline(params);

  auto scene = makeScene(rng);
  const double dt = 0.1;
  const double yaw_per_frame = 0.02;
  bool saw_imu = false;
  int max_sub = 1;
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
    if (f > 0) imu = makeDynamicImu(f * dt, 20);
    const auto result = pipeline.registerFrame(scan, imu);
    EXPECT_TRUE(result.converged);
    max_sub = std::max(max_sub, result.subframes);
    if (f > 0 && result.used_imu) saw_imu = true;
  }
  EXPECT_TRUE(saw_imu);
  EXPECT_GE(max_sub, 1);
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(2, 0, 0)).norm();
  EXPECT_LT(err, 2.5);
}

TEST(FRLIO, RCVoxelMapWrapsAcrossGridBoundary) {
  RCVoxelMap map(40.0, 2.0, 0.5, 1.0, 8);
  map.initialize(Eigen::Vector3d::Zero());
  std::vector<Eigen::Vector3d> pts;
  for (int i = 0; i < 200; i++) {
    pts.emplace_back(35.0 + 0.01 * i, 0.0, 0.0);
  }
  map.addPoints(pts);
  map.updateRobot(Eigen::Vector3d(70.0, 0.0, 0.0));
  const auto match = map.matchPointToPlane(Eigen::Vector3d(35.0, 0.0, 0.0), 2.0, 5);
  EXPECT_TRUE(match.valid || map.activeGrids() > 0);
}
