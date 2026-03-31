#include "fast_lio_slam/fast_lio_slam.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::fast_lio_slam;

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

FastLioSlamParams makeBaseParams() {
  FastLioSlamParams params;
  params.front_end.max_iterations = 6;
  params.front_end.ceres_max_iterations = 5;
  params.front_end.max_correspondence_distance = 4.0;
  params.front_end.planarity_threshold = 0.08;
  params.front_end.keypoint_voxel_size = 0.6;
  params.front_end.velocity_prior_weight = 0.1;
  params.keyframe_translation_threshold = 0.2;
  params.descriptor_stride = 2;
  params.loop_stride = 3;
  return params;
}

}  // namespace

TEST(FastLioSlam, TracksShortSequenceWithDirectFrontEnd) {
  FastLioSlamParams params = makeBaseParams();
  params.enable_loop_closure = false;

  FastLioSlam pipeline(params);
  FastLioSlamResult last_result;

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
  EXPECT_GT(last_result.num_keyframes, 3);
  EXPECT_EQ(last_result.num_loop_edges, 0);
  EXPECT_LT((last_result.t_w_curr - Eigen::Vector3d(3.5, 0.5, 0.0)).norm(), 4.0);
}

TEST(FastLioSlam, DetectsLoopClosureOnOutAndBackRoute) {
  FastLioSlamParams params = makeBaseParams();
  params.scan_context.exclude_recent_frames = 2;
  params.scan_context.distance_threshold = 0.20;
  params.min_loop_index_gap = 4;
  params.loop_gicp.max_correspondence_distance = 5.0;
  params.loop_fitness_threshold = 1.5;
  params.min_loop_correspondences = 60;

  FastLioSlam pipeline(params);
  bool saw_loop = false;
  FastLioSlamResult last_result;

  const std::vector<double> x_positions = {0.0, 1.0, 2.0, 3.0, 2.0, 1.0, 0.0};
  for (size_t frame = 0; frame < x_positions.size(); ++frame) {
    const Eigen::Matrix4d pose = makePose(x_positions[frame], 0.0, 0.0);
    const std::vector<Eigen::Vector3d> scan = generateScan(pose);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kScanPeriod, frame * kScanPeriod,
                                    0.0, 0.0);
    last_result = pipeline.process(scan, imu_samples);
    saw_loop = saw_loop || last_result.loop_detected;
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_TRUE(saw_loop);
  EXPECT_GT(pipeline.numLoopEdges(), 0);
  EXPECT_GT(last_result.num_keyframes, 4);
}

TEST(FastLioSlam, ClearResetsFrontEndAndGraph) {
  FastLioSlam pipeline(makeBaseParams());
  pipeline.process(generateScan(makePose(0.0, 0.0, 0.0)));
  ASSERT_TRUE(pipeline.initialized());
  ASSERT_GT(pipeline.numKeyframes(), 0);

  pipeline.clear();

  EXPECT_FALSE(pipeline.initialized());
  EXPECT_EQ(pipeline.numKeyframes(), 0);
  EXPECT_EQ(pipeline.numLoopEdges(), 0);
  EXPECT_EQ(pipeline.latestOptimizedPoseMatrix(), Eigen::Matrix4d::Identity());
}
