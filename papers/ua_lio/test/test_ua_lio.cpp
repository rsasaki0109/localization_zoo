#include "ua_lio/ua_lio.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::ua_lio;
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
std::vector<ImuSample> cvImu(int f) {
  std::vector<ImuSample> imu;
  for (int k = 0; k < 10; k++) {
    ImuSample s;
    s.timestamp = f + 0.1 * k;
    s.gyro = Eigen::Vector3d::Zero();
    s.accel = Eigen::Vector3d(0, 0, 9.81);
    imu.push_back(s);
  }
  return imu;
}
}  // namespace

TEST(UALIO, FirstFrameInitializesMap) {
  UALIOPipeline pipeline(UALIOParams{});
  const auto result = pipeline.registerFrame({}, {});
  EXPECT_TRUE(result.converged);
}

TEST(UALIO, GaussianVoxelStoresDistribution) {
  GaussianVoxelMap map(1.0, 6);
  std::mt19937 rng(42);
  std::normal_distribution<double> n(0, 0.005);
  std::uniform_real_distribution<double> a(-0.4, 0.4);
  std::vector<Eigen::Vector3d> pts;
  for (int i = 0; i < 200; i++) pts.emplace_back(a(rng), a(rng), n(rng));
  map.addPoints(pts);
  const auto f = map.query(Eigen::Vector3d(0, 0, 0));
  ASSERT_TRUE(f.found);
  ASSERT_TRUE(f.has_normal);
  EXPECT_GT(std::abs(f.normal.z()), 0.9);  // 地面法線
  EXPECT_GT(f.cov(0, 0), f.cov(2, 2));     // x 方向の分散 > z 方向 (薄い平面)
}

TEST(UALIO, TracksTranslationAndGroundConstraint) {
  std::mt19937 rng(42);
  UALIOParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 40;
  UALIOPipeline pipeline(params);

  auto scene = makeRoom(rng);
  int total_ground = 0;
  for (int f = 0; f < 4; f++) {
    const Eigen::Vector3d t(f * 0.5, 0, 0);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 1 && ps.norm() < 25) scan.push_back(ps);
    }
    const auto result = pipeline.registerFrame(scan, cvImu(f));
    EXPECT_TRUE(result.converged);
    total_ground += result.ground_cells;
  }
  EXPECT_GT(total_ground, 0);  // 地面拘束が効いている
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(1.5, 0, 0)).norm();
  EXPECT_LT(err, 1.0);
}
