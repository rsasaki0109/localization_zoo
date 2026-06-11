#include "rf_lio/rf_lio.h"

#include <gtest/gtest.h>

#include <Eigen/Geometry>

#include <random>
#include <vector>

using namespace localization_zoo::rf_lio;
using localization_zoo::imu_preintegration::ImuSample;

namespace {

std::vector<Eigen::Vector3d> makeRoom(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-10.0, 10.0);
  std::uniform_real_distribution<double> z(0.0, 4.0);
  std::normal_distribution<double> n(0.0, 0.01);
  for (int i = 0; i < 5000; ++i) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 2500; ++i) pts.emplace_back(xy(rng), 10.0 + n(rng), z(rng));
  for (int i = 0; i < 2500; ++i) pts.emplace_back(-10.0 + n(rng), xy(rng), z(rng));
  for (int i = 0; i < 2500; ++i) pts.emplace_back(xy(rng), -10.0 + n(rng), z(rng));
  return pts;
}

std::vector<Eigen::Vector3d> scanFromPose(const std::vector<Eigen::Vector3d>& world,
                                          const Eigen::Vector3d& t,
                                          double yaw = 0.0) {
  const Eigen::Matrix3d R =
      Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  std::vector<Eigen::Vector3d> scan;
  scan.reserve(world.size());
  for (const auto& p_w : world) {
    const Eigen::Vector3d p_s = R.transpose() * (p_w - t);
    const double r = p_s.norm();
    if (r > 0.2 && r < 28.0) scan.push_back(p_s);
  }
  return scan;
}

RfLioParams testParams() {
  RfLioParams params;
  auto& p = params.backend;
  p.voxel_size = 0.8;
  p.min_range = 0.2;
  p.max_range = 30.0;
  p.max_icp_iterations = 35;
  p.max_correspondence_dist = 1.5;
  p.range_image_width = 360;
  p.range_image_height = 64;
  p.fov_up_deg = 25.0;
  p.fov_down_deg = -45.0;
  p.new_object_margin = 0.6;
  p.disappearance_margin = 0.4;
  p.delayed_removal_frames = 1;
  p.map_update_radius = 0.4;
  params.range_image_width = 360;
  params.range_image_height = 64;
  params.fov_up_deg = 25.0;
  params.fov_down_deg = -45.0;
  params.min_range = 0.2;
  params.max_range = 30.0;
  params.foreground_margin = 0.6;
  params.min_keep_points = 50;
  params.max_removal_fraction = 0.5;
  return params;
}

std::vector<ImuSample> yawImu(double yaw, double dt = 0.1) {
  std::vector<ImuSample> imu;
  for (int i = 0; i <= 10; ++i) {
    ImuSample s;
    s.timestamp = static_cast<double>(i) * dt / 10.0;
    s.gyro = Eigen::Vector3d(0.0, 0.0, yaw / dt);
    s.accel = Eigen::Vector3d(0.0, 0.0, 9.81);
    imu.push_back(s);
  }
  return imu;
}

}  // namespace

TEST(RfLio, FirstFrameBuildsBackendMap) {
  std::mt19937 rng(20);
  RfLioPipeline pipe(testParams());
  const auto r = pipe.registerFrame(scanFromPose(makeRoom(rng), {0.0, 0.0, 1.0}), {});
  EXPECT_TRUE(r.backend.converged);
  EXPECT_EQ(r.removal_first_points, 0);
  EXPECT_GT(pipe.mapSize(), static_cast<size_t>(100));
}

TEST(RfLio, RemovalFirstDropsForegroundObjectBeforeBackend) {
  std::mt19937 rng(21);
  const auto world = makeRoom(rng);
  RfLioPipeline pipe(testParams());
  pipe.registerFrame(scanFromPose(world, {0.0, 0.0, 1.0}), {});

  auto scan = scanFromPose(world, {0.0, 0.0, 1.0});
  for (double y = -1.0; y <= 1.0; y += 0.06) {
    for (double z = -0.5; z <= 1.0; z += 0.06) {
      scan.emplace_back(2.2, y, z);
    }
  }
  const auto r = pipe.registerFrame(scan, {});
  EXPECT_GT(r.candidate_points, 10);
  EXPECT_GT(r.removal_first_points, 10);
  EXPECT_LT(r.kept_points, static_cast<int>(scan.size()));
  EXPECT_TRUE(r.backend.converged);
}

TEST(RfLio, TracksShortTranslationAfterRemovalFirstFiltering) {
  std::mt19937 rng(22);
  const auto world = makeRoom(rng);
  RfLioPipeline pipe(testParams());
  pipe.registerFrame(scanFromPose(world, {0.0, 0.0, 1.0}), {});
  for (int i = 1; i <= 5; ++i) {
    const auto r = pipe.registerFrame(scanFromPose(world, {0.22 * i, 0.0, 1.0}), {});
    EXPECT_TRUE(r.backend.converged) << i;
  }
  const Eigen::Vector3d est = pipe.pose().block<3, 1>(0, 3);
  EXPECT_LT((est - Eigen::Vector3d(1.1, 0.0, 0.0)).norm(), 0.45);
}

TEST(RfLio, PreservesImuRotationPriorThroughBackend) {
  std::mt19937 rng(23);
  const auto world = makeRoom(rng);
  RfLioPipeline pipe(testParams());
  pipe.registerFrame(scanFromPose(world, {0.0, 0.0, 1.0}), {});
  const auto r = pipe.registerFrame(scanFromPose(world, {0.0, 0.0, 1.0}, 0.05),
                                    yawImu(0.05));
  EXPECT_TRUE(r.backend.used_imu);
  EXPECT_TRUE(r.backend.converged);
}
