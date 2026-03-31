#include "clins/clins.h"

#include <gtest/gtest.h>

#include <random>

using namespace localization_zoo::clins;

namespace {

std::vector<Eigen::Vector3d> generateEnv(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> points;
  std::normal_distribution<double> noise(0.0, 0.01);
  std::uniform_real_distribution<double> xy(-30.0, 30.0);
  std::uniform_real_distribution<double> z(0.0, 4.0);

  for (int i = 0; i < 2500; ++i) {
    points.emplace_back(xy(rng), xy(rng), noise(rng));
  }
  for (int i = 0; i < 1000; ++i) {
    points.emplace_back(xy(rng), 15.0 + noise(rng), z(rng));
  }
  for (int i = 0; i < 1000; ++i) {
    points.emplace_back(-15.0 + noise(rng), xy(rng), z(rng));
  }
  return points;
}

localization_zoo::ct_icp::TrajectoryFrame makeFrame(double begin_x, double end_x) {
  localization_zoo::ct_icp::TrajectoryFrame frame;
  frame.begin_pose.trans = Eigen::Vector3d(begin_x, 0.0, 1.0);
  frame.end_pose.trans = Eigen::Vector3d(end_x, 0.0, 1.0);
  return frame;
}

std::vector<localization_zoo::ct_icp::TimedPoint> simulateScan(
    const std::vector<Eigen::Vector3d>& env,
    const localization_zoo::ct_icp::TrajectoryFrame& frame,
    std::mt19937& rng) {
  std::vector<localization_zoo::ct_icp::TimedPoint> timed_points;
  std::normal_distribution<double> noise(0.0, 0.02);

  for (const auto& world_point : env) {
    const double timestamp = std::uniform_real_distribution<double>(0.0, 1.0)(rng);
    const localization_zoo::ct_icp::SE3 pose =
        localization_zoo::ct_icp::SE3::interpolate(
            frame.begin_pose, frame.end_pose, frame.getAlpha(timestamp));
    const Eigen::Vector3d sensor_point = pose.inverse() * world_point;
    if (sensor_point.norm() > 30.0 || sensor_point.norm() < 0.5) {
      continue;
    }

    localization_zoo::ct_icp::TimedPoint point;
    point.raw_point =
        sensor_point + Eigen::Vector3d(noise(rng), noise(rng), noise(rng));
    point.timestamp = timestamp;
    timed_points.push_back(point);
    if (timed_points.size() > 2500) {
      break;
    }
  }

  return timed_points;
}

std::vector<localization_zoo::imu_preintegration::ImuSample> gravityImu(
    double duration, double dt) {
  std::vector<localization_zoo::imu_preintegration::ImuSample> samples;
  for (int i = 0; i <= static_cast<int>(duration / dt); ++i) {
    localization_zoo::imu_preintegration::ImuSample sample;
    sample.timestamp = i * dt;
    sample.gyro = Eigen::Vector3d::Zero();
    sample.accel = Eigen::Vector3d(0.0, 0.0, 9.81);
    samples.push_back(sample);
  }
  return samples;
}

std::vector<localization_zoo::ct_icp::TimedPoint> simpleScan(double x_center) {
  std::vector<localization_zoo::ct_icp::TimedPoint> scan;
  for (int i = 0; i < 10; ++i) {
    localization_zoo::ct_icp::TimedPoint point;
    point.raw_point = Eigen::Vector3d(x_center + 0.01 * i, 0.0, 0.0);
    point.timestamp = 0.5;
    scan.push_back(point);
  }
  return scan;
}

}  // namespace

TEST(CLINS, TracksShortSequenceWithContinuousTimeMap) {
  std::mt19937 rng(42);
  const auto env = generateEnv(rng);

  CLINSParams params;
  params.registration.voxel_resolution = 1.0;
  params.registration.max_frames_in_map = 10;
  params.keyframe_translation_threshold = 0.2;

  CLINS pipeline(params);
  CLINSResult last_result;

  for (int frame = 0; frame < 6; ++frame) {
    const auto trajectory = makeFrame(0.6 * frame, 0.6 * (frame + 1));
    const auto scan = simulateScan(env, trajectory, rng);
    const auto imu = gravityImu(0.1, 0.01);
    last_result = pipeline.process(scan, imu);
  }

  ASSERT_TRUE(last_result.initialized);
  ASSERT_TRUE(last_result.valid);
  EXPECT_GT(last_result.registration_result.num_correspondences, 0);
  EXPECT_GT(last_result.map_size, 0u);
  EXPECT_LT((last_result.state.frame.end_pose.trans -
             Eigen::Vector3d(3.6, 0.0, 1.0))
                .norm(),
            2.0);
}

TEST(CLINS, SlidingWindowMapBoundsFrameCount) {
  CLINSParams params;
  params.registration.voxel_resolution = 1.0;
  params.registration.max_frames_in_map = 2;
  params.keyframe_stride = 1;
  params.keyframe_translation_threshold = 0.0;

  CLINS pipeline(params);
  pipeline.process(simpleScan(0.0));
  pipeline.process(simpleScan(10.0));
  pipeline.process(simpleScan(20.0));

  EXPECT_TRUE(pipeline.initialized());
  EXPECT_EQ(pipeline.numFramesInMap(), 2u);
  EXPECT_EQ(pipeline.mapSize(), 2u);
}

TEST(CLINS, ClearResetsStateAndMap) {
  CLINS pipeline;
  pipeline.process(simpleScan(0.0));
  pipeline.process(simpleScan(10.0));
  ASSERT_TRUE(pipeline.initialized());
  ASSERT_GT(pipeline.mapSize(), 0u);

  pipeline.clear();

  EXPECT_FALSE(pipeline.initialized());
  EXPECT_EQ(pipeline.numFrames(), 0);
  EXPECT_EQ(pipeline.mapSize(), 0u);
  EXPECT_EQ(pipeline.numFramesInMap(), 0u);
  EXPECT_EQ(pipeline.state().frame.end_pose.trans, Eigen::Vector3d::Zero());
}
