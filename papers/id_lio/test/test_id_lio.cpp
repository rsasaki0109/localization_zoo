#include "id_lio/id_lio.h"

#include <gtest/gtest.h>

#include <Eigen/Geometry>

#include <random>
#include <vector>

using namespace localization_zoo::id_lio;
using localization_zoo::imu_preintegration::ImuSample;

namespace {

std::vector<Eigen::Vector3d> makeRoom(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-10.0, 10.0);
  std::uniform_real_distribution<double> z(0.0, 4.0);
  std::normal_distribution<double> n(0.0, 0.01);
  for (int i = 0; i < 10000; ++i) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 5000; ++i) pts.emplace_back(xy(rng), 10.0 + n(rng), z(rng));
  for (int i = 0; i < 5000; ++i) pts.emplace_back(-10.0 + n(rng), xy(rng), z(rng));
  for (int i = 0; i < 5000; ++i) pts.emplace_back(xy(rng), -10.0 + n(rng), z(rng));
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

IDLIOParams testParams() {
  IDLIOParams p;
  p.voxel_size = 0.8;
  p.min_range = 0.2;
  p.max_range = 30.0;
  p.max_icp_iterations = 35;
  p.max_correspondence_dist = 1.5;
  p.range_image_width = 360;
  p.range_image_height = 64;
  p.fov_up_deg = 25.0;
  p.fov_down_deg = -45.0;
  p.new_object_margin = 0.4;
  p.disappearance_margin = 0.4;
  p.delayed_removal_frames = 2;
  p.map_update_radius = 0.4;
  return p;
}

}  // namespace

TEST(IDLIO, FirstFrameCreatesIndexedMap) {
  std::mt19937 rng(4);
  IDLIOPipeline pipe(testParams());
  const auto r = pipe.registerFrame(scanFromPose(makeRoom(rng), {0.0, 0.0, 1.0}), {});
  EXPECT_TRUE(r.converged);
  EXPECT_GT(pipe.mapSize(), static_cast<size_t>(100));
  EXPECT_EQ(pipe.mapSize(), pipe.indexedPoints());
}

TEST(IDLIO, TracksTranslationSequence) {
  std::mt19937 rng(5);
  const auto world = makeRoom(rng);
  IDLIOPipeline pipe(testParams());
  pipe.registerFrame(scanFromPose(world, {0.0, 0.0, 1.0}), {});
  for (int i = 1; i <= 5; ++i) {
    const auto r = pipe.registerFrame(scanFromPose(world, {0.25 * i, 0.0, 1.0}), {});
    EXPECT_TRUE(r.converged) << i;
  }
  const Eigen::Vector3d est = pipe.pose().block<3, 1>(0, 3);
  EXPECT_LT((est - Eigen::Vector3d(1.25, 0.0, 0.0)).norm(), 0.35);
}

TEST(IDLIO, UsesImuRotationPriorWhenAvailable) {
  std::mt19937 rng(6);
  const auto world = makeRoom(rng);
  IDLIOPipeline pipe(testParams());
  pipe.registerFrame(scanFromPose(world, {0.0, 0.0, 1.0}), {});
  const auto r = pipe.registerFrame(scanFromPose(world, {0.0, 0.0, 1.0}, 0.06),
                                    yawImu(0.06));
  EXPECT_TRUE(r.used_imu);
  EXPECT_TRUE(r.converged);
  const Eigen::Matrix3d R = r.pose.block<3, 3>(0, 0);
  EXPECT_NEAR(std::atan2(R(1, 0), R(0, 0)), 0.06, 0.04);
}

TEST(IDLIO, DownweightsNewForegroundDynamicPoints) {
  std::mt19937 rng(7);
  auto world = makeRoom(rng);
  IDLIOPipeline pipe(testParams());
  pipe.registerFrame(scanFromPose(world, {0.0, 0.0, 1.0}), {});

  auto scan = scanFromPose(world, {0.0, 0.0, 1.0});
  for (double y = -1.0; y <= 1.0; y += 0.08) {
    for (double z = -0.4; z <= 0.8; z += 0.08) {
      scan.emplace_back(2.0, y, z);
    }
  }
  const auto r = pipe.registerFrame(scan, {});
  EXPECT_GT(r.num_dynamic_points, 5);
  EXPECT_TRUE(r.converged);
  EXPECT_GT(pipe.dynamicMapPoints(), static_cast<size_t>(0));
}

TEST(IDLIO, DelayedRemovalMarksMissingIndexedPoints) {
  IDLIOParams params = testParams();
  params.voxel_size = 0.4;
  params.delayed_removal_frames = 2;
  IDLIOPipeline pipe(params);

  std::vector<Eigen::Vector3d> first;
  for (double y = -2.0; y <= 2.0; y += 0.08) {
    for (double z = -0.5; z <= 1.5; z += 0.08) first.emplace_back(4.0, y, z);
  }
  for (double y = -2.0; y <= 2.0; y += 0.08) {
    for (double z = -0.5; z <= 1.5; z += 0.08) first.emplace_back(8.0, y, z);
  }
  pipe.registerFrame(first, {});

  std::vector<Eigen::Vector3d> later;
  for (double y = -2.0; y <= 2.0; y += 0.08) {
    for (double z = -0.5; z <= 1.5; z += 0.08) later.emplace_back(8.0, y, z);
  }
  pipe.registerFrame(later, {});
  const auto r = pipe.registerFrame(later, {});
  EXPECT_GT(r.num_removed_points, 0);
}
