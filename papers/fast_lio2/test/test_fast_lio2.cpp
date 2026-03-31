#include "fast_lio2/fast_lio2.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::fast_lio2;

namespace {

using localization_zoo::imu_preintegration::ImuSample;

constexpr double kScanPeriod = 0.1;

Eigen::Matrix4d makePose(double x, double y, double yaw_rad) {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  pose.block<3, 3>(0, 0) =
      Eigen::AngleAxisd(yaw_rad, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  pose(0, 3) = x;
  pose(1, 3) = y;
  return pose;
}

const std::vector<Eigen::Vector3d>& worldPoints() {
  static const std::vector<Eigen::Vector3d> points = [] {
    std::vector<Eigen::Vector3d> map_points;

    for (double x = -20.0; x <= 20.0; x += 2.0) {
      for (double y = -20.0; y <= 20.0; y += 2.0) {
        map_points.emplace_back(x, y, 0.0);
      }
    }

    for (double x = -18.0; x <= 18.0; x += 1.5) {
      for (double z = 0.0; z <= 4.0; z += 0.5) {
        map_points.emplace_back(x, 18.0, z);
        map_points.emplace_back(-14.0, x, z);
      }
    }

    for (double y = -12.0; y <= 10.0; y += 1.2) {
      for (double z = 0.0; z <= 5.0; z += 0.5) {
        map_points.emplace_back(12.0, y, 0.5 + 0.1 * (y + 12.0) + 0.2 * z);
      }
    }

    for (double deg = 0.0; deg < 360.0; deg += 15.0) {
      const double theta = deg * M_PI / 180.0;
      for (double z = 0.0; z <= 4.5; z += 0.3) {
        map_points.emplace_back(6.0 + 0.8 * std::cos(theta),
                                6.0 + 0.8 * std::sin(theta), z);
        map_points.emplace_back(-8.0 + 0.6 * std::cos(theta),
                                10.0 + 0.6 * std::sin(theta), z);
      }
    }

    return map_points;
  }();
  return points;
}

std::vector<Eigen::Vector3d> generateScan(const Eigen::Matrix4d& pose) {
  std::vector<Eigen::Vector3d> raw_points;
  raw_points.reserve(worldPoints().size());

  const Eigen::Matrix3d rotation = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d translation = pose.block<3, 1>(0, 3);
  for (const auto& world_point : worldPoints()) {
    const Eigen::Vector3d sensor_point =
        rotation.transpose() * (world_point - translation);
    const double range = sensor_point.norm();
    if (range < 1.0 || range > 60.0) {
      continue;
    }
    raw_points.push_back(sensor_point);
  }
  return raw_points;
}

std::vector<ImuSample> makeImuSamples(double start_time, double end_time,
                                      double yaw_start, double yaw_end) {
  std::vector<ImuSample> samples;
  const int num_samples = 6;
  const double dt = (end_time - start_time) / (num_samples - 1);
  const double yaw_rate =
      end_time > start_time ? (yaw_end - yaw_start) / (end_time - start_time) : 0.0;

  for (int i = 0; i < num_samples; ++i) {
    ImuSample sample;
    sample.timestamp = start_time + dt * i;
    sample.gyro = Eigen::Vector3d(0.0, 0.0, yaw_rate);
    sample.accel = Eigen::Vector3d(0.0, 0.0, 9.81);
    samples.push_back(sample);
  }

  return samples;
}

}  // namespace

TEST(FastLio2, TracksShortSequence) {
  FastLio2Params params;
  params.max_iterations = 6;
  params.ceres_max_iterations = 5;
  params.max_correspondence_distance = 4.0;
  params.planarity_threshold = 0.08;
  params.keypoint_voxel_size = 0.6;
  params.velocity_prior_weight = 0.1;

  FastLio2 pipeline(params);
  FastLio2Result last_result;

  for (int frame = 0; frame < 6; ++frame) {
    const double yaw = frame * 2.0 * M_PI / 180.0;
    const Eigen::Matrix4d pose = makePose(0.7 * frame, 0.1 * frame, yaw);
    const std::vector<Eigen::Vector3d> scan = generateScan(pose);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kScanPeriod, frame * kScanPeriod,
                                    (frame - 1) * 2.0 * M_PI / 180.0, yaw);
    last_result = pipeline.process(scan, imu_samples);
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_TRUE(last_result.imu_used);
  EXPECT_GT(last_result.num_correspondences, 30);
  EXPECT_GT(last_result.map_size, 0u);
  EXPECT_LT(
      (last_result.state.trans - Eigen::Vector3d(3.5, 0.5, 0.0)).norm(), 3.0);
}

TEST(FastLio2, SlidingWindowMapBoundsFrameCount) {
  FastLio2Params params;
  params.max_frames_in_map = 2;
  params.max_correspondence_distance = 4.0;
  params.planarity_threshold = 0.08;

  FastLio2 pipeline(params);
  for (int frame = 0; frame < 4; ++frame) {
    const Eigen::Matrix4d pose = makePose(static_cast<double>(frame), 0.0, 0.0);
    const std::vector<Eigen::Vector3d> scan = generateScan(pose);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kScanPeriod, frame * kScanPeriod,
                                    0.0, 0.0);
    pipeline.process(scan, imu_samples);
  }

  EXPECT_TRUE(pipeline.initialized());
  EXPECT_EQ(pipeline.numFramesInMap(), 2u);
  EXPECT_GT(pipeline.mapSize(), 0u);
}

TEST(FastLio2, ClearResetsStateAndMap) {
  FastLio2 pipeline;
  pipeline.process(generateScan(makePose(0.0, 0.0, 0.0)));
  ASSERT_TRUE(pipeline.initialized());
  ASSERT_GT(pipeline.mapSize(), 0u);

  pipeline.clear();

  EXPECT_FALSE(pipeline.initialized());
  EXPECT_EQ(pipeline.mapSize(), 0u);
  EXPECT_EQ(pipeline.numFramesInMap(), 0u);
  EXPECT_EQ(pipeline.state().trans, Eigen::Vector3d::Zero());
}
