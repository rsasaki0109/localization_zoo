#include "lio_sam/lio_sam.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::lio_sam;

namespace {

using localization_zoo::aloam::PointCloud;
using localization_zoo::aloam::PointCloudPtr;
using localization_zoo::aloam::PointT;
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

PointCloudPtr generateLidarScan(const Eigen::Matrix4d& pose) {
  auto cloud = PointCloudPtr(new PointCloud);
  const Eigen::Matrix3d rotation = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d translation = pose.block<3, 1>(0, 3);

  for (const auto& world_point : worldPoints()) {
    const Eigen::Vector3d sensor_point =
        rotation.transpose() * (world_point - translation);
    const double range = sensor_point.norm();
    if (range < 1.0 || range > 60.0) {
      continue;
    }

    const double vertical_deg =
        std::atan2(sensor_point.z(),
                   std::sqrt(sensor_point.x() * sensor_point.x() +
                             sensor_point.y() * sensor_point.y())) *
        180.0 / M_PI;
    if (vertical_deg < -15.0 || vertical_deg > 15.0) {
      continue;
    }

    PointT point;
    point.x = static_cast<float>(sensor_point.x());
    point.y = static_cast<float>(sensor_point.y());
    point.z = static_cast<float>(sensor_point.z());
    point.intensity = 0.0f;
    cloud->push_back(point);
  }

  return cloud;
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
    sample.accel = Eigen::Vector3d::Zero();
    samples.push_back(sample);
  }
  return samples;
}

}  // namespace

TEST(LIOSAM, TracksShortSequenceWithImuPrior) {
  LioSamParams params;
  params.enable_loop_closure = false;
  params.scan_context.exclude_recent_frames = 2;
  params.odometry.distance_sq_threshold = 16.0;
  params.mapping.knn_max_dist_sq = 4.0;
  params.keyframe_translation_threshold = 0.2;
  params.loop_gicp.max_correspondence_distance = 4.0;

  LioSam pipeline(params);
  LioSamResult last_result;

  for (int frame = 0; frame < 6; ++frame) {
    const double yaw = frame * 2.0 * M_PI / 180.0;
    const Eigen::Matrix4d pose = makePose(0.7 * frame, 0.1 * frame, yaw);
    const auto cloud = generateLidarScan(pose);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kScanPeriod, frame * kScanPeriod,
                                    (frame - 1) * 2.0 * M_PI / 180.0, yaw);
    last_result = pipeline.process(cloud, imu_samples);
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_GT(last_result.num_keyframes, 3);
  EXPECT_GT(pipeline.numImuEdges(), 0);
  EXPECT_LT(
      (last_result.t_w_curr - Eigen::Vector3d(3.5, 0.5, 0.0)).norm(), 5.0);
}

TEST(LIOSAM, DetectsLoopClosureOnOutAndBackRoute) {
  LioSamParams params;
  params.scan_context.exclude_recent_frames = 2;
  params.scan_context.distance_threshold = 0.20;
  params.min_loop_index_gap = 4;
  params.keyframe_translation_threshold = 0.2;
  params.odometry.distance_sq_threshold = 16.0;
  params.mapping.knn_max_dist_sq = 4.0;
  params.loop_gicp.max_correspondence_distance = 5.0;
  params.loop_fitness_threshold = 1.5;
  params.min_loop_correspondences = 60;

  LioSam pipeline(params);
  bool saw_loop = false;
  LioSamResult last_result;

  const std::vector<double> x_positions = {0.0, 1.0, 2.0, 3.0, 2.0, 1.0, 0.0};
  for (size_t frame = 0; frame < x_positions.size(); ++frame) {
    const Eigen::Matrix4d pose = makePose(x_positions[frame], 0.0, 0.0);
    const auto cloud = generateLidarScan(pose);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kScanPeriod, frame * kScanPeriod,
                                    0.0, 0.0);
    last_result = pipeline.process(cloud, imu_samples);
    saw_loop = saw_loop || last_result.loop_detected;
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_TRUE(saw_loop);
  EXPECT_GT(pipeline.numLoopEdges(), 0);
  EXPECT_GT(last_result.num_keyframes, 4);
}
