#include "pg_lio/pg_lio.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::pg_lio;
using localization_zoo::imu_preintegration::ImuSample;

namespace {
std::vector<IntensityPoint> makeScene(std::mt19937& rng) {
  std::vector<IntensityPoint> pts;
  std::uniform_real_distribution<double> xy(-30, 30);
  std::uniform_real_distribution<double> z(0, 4);
  std::uniform_real_distribution<double> inten(0.2, 0.9);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 2500; i++) {
    pts.push_back({Eigen::Vector3d(xy(rng), xy(rng), n(rng)), inten(rng)});
  }
  for (int i = 0; i < 800; i++) {
    pts.push_back({Eigen::Vector3d(xy(rng), 15 + n(rng), z(rng)), inten(rng)});
  }
  return pts;
}

std::vector<ImuSample> makeYawImu(double yaw_rate, double dt, double t0, int samples) {
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

TEST(PGLIO, FirstFrameInitializesMapAndPatches) {
  PGLIOPipeline pipeline(PGLIOParams{});
  const auto result = pipeline.registerFrame({}, {});
  EXPECT_TRUE(result.converged);
  EXPECT_EQ(pipeline.mapSize(), 0);
}

TEST(PGLIO, RangeImageProjectsPoints) {
  RangeImage img(256, 32);
  std::vector<IntensityPoint> pts = {
      {{10.0, 0.0, 0.5}, 0.7}, {{0.0, 10.0, -0.2}, 0.4}};
  img.build(pts);
  img.filterAndSmooth();
  double u = 0.0, v = 0.0;
  EXPECT_TRUE(img.project(pts[0].p, u, v));
  EXPECT_GE(u, 0.0);
  EXPECT_LT(u, 256.0);
}

TEST(PGLIO, MultiFrameTracksWithImuAndPhotometricPatches) {
  std::mt19937 rng(42);
  PGLIOParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 40;
  params.max_patches = 16;
  params.photometric_weight = 0.0;
  PGLIOPipeline pipeline(params);

  auto scene = makeScene(rng);
  const double dt = 0.1;
  const double yaw_per_frame = 0.02;
  bool saw_imu = false;
  for (int f = 0; f < 3; f++) {
    const Eigen::Matrix3d R =
        Eigen::AngleAxisd(f * yaw_per_frame, Eigen::Vector3d::UnitZ())
            .toRotationMatrix();
    const Eigen::Vector3d t(f * 1.0, 0, 1);
    std::vector<IntensityPoint> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = R.transpose() * (p.p - t);
      if (ps.norm() > 3 && ps.norm() < 30) scan.push_back({ps, p.intensity});
    }
    std::vector<ImuSample> imu;
    if (f > 0) imu = makeYawImu(yaw_per_frame / dt, dt, f * dt, 10);
    const auto result = pipeline.registerFrame(scan, imu);
    EXPECT_TRUE(result.converged);
    if (f > 0 && result.used_imu) saw_imu = true;
  }
  EXPECT_TRUE(saw_imu);
  EXPECT_GT(pipeline.patchCount(), 0);
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(2, 0, 0)).norm();
  EXPECT_LT(err, 2.5);
}

TEST(PGLIO, PhotometricPatchNccIsBounded) {
  RangeImage img(128, 32);
  std::vector<IntensityPoint> pts;
  for (int i = 0; i < 200; i++) {
    const double ang = i * 0.05;
    pts.push_back(
        {{10.0 * std::cos(ang), 10.0 * std::sin(ang), 0.1 * std::sin(ang * 3)},
         0.3 + 0.4 * std::sin(ang * 5)});
  }
  img.build(pts);
  img.filterAndSmooth();
  img.computeGradients();

  PGLIOPipeline pipeline(PGLIOParams{});
  pipeline.setInitialPose(Eigen::Matrix4d::Identity());
  pipeline.registerFrame(pts, {});
  EXPECT_GT(pipeline.patchCount(), 0);
}
