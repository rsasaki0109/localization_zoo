#include "dlio/dlio.h"

#include <gtest/gtest.h>

#include <cmath>
#include <random>

using namespace localization_zoo::dlio;

namespace {

using localization_zoo::aloam::PointCloud;
using localization_zoo::aloam::PointCloudPtr;
using localization_zoo::aloam::PointT;
using localization_zoo::imu_preintegration::ImuSample;

PointCloudPtr generateLidarScan(const Eigen::Matrix4d& pose, std::mt19937& rng) {
  auto cloud = PointCloudPtr(new PointCloud);
  std::normal_distribution<float> noise(0.0f, 0.01f);
  std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * M_PI);
  std::uniform_real_distribution<double> xy(-30.0, 30.0);
  std::uniform_real_distribution<double> z(0.0, 4.0);

  const Eigen::Matrix3d rotation = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d translation = pose.block<3, 1>(0, 3);

  auto add = [&](const Eigen::Vector3d& world_point) {
    const Eigen::Vector3d sensor_point =
        rotation.transpose() * (world_point - translation);
    PointT point;
    point.x = sensor_point.x() + noise(rng);
    point.y = sensor_point.y() + noise(rng);
    point.z = sensor_point.z() + noise(rng);
    point.intensity = 0.0f;
    cloud->push_back(point);
  };

  for (int i = 0; i < 3500; ++i) {
    add(Eigen::Vector3d(xy(rng), xy(rng), 0.0));
  }
  for (int i = 0; i < 1200; ++i) {
    add(Eigen::Vector3d(xy(rng), 15.0, z(rng)));
    add(Eigen::Vector3d(-15.0, xy(rng), z(rng)));
  }
  for (int i = 0; i < 600; ++i) {
    const double theta = angle_dist(rng);
    add(Eigen::Vector3d(5.0 + 0.3 * std::cos(theta),
                        5.0 + 0.3 * std::sin(theta), z(rng)));
  }

  return cloud;
}

std::vector<ImuSample> generateImuSamples(
    double start_time, double end_time,
    const Eigen::Vector3d& accel = Eigen::Vector3d(0.0, 0.0, 9.81),
    const Eigen::Vector3d& gyro = Eigen::Vector3d::Zero()) {
  std::vector<ImuSample> samples;
  for (double timestamp = start_time; timestamp <= end_time + 1e-9;
       timestamp += 0.01) {
    ImuSample sample;
    sample.timestamp = timestamp;
    sample.gyro = gyro;
    sample.accel = accel;
    samples.push_back(sample);
  }
  return samples;
}

}  // namespace

TEST(DLIO, TracksShortSequenceWithImuPrediction) {
  std::mt19937 rng(42);
  DLIOParams params;
  params.gicp.max_correspondence_distance = 5.0;
  params.gicp.max_iterations = 20;
  params.gicp.k_neighbors = 15;
  params.registration_voxel_size = 0.7;
  params.map_voxel_size = 1.0;

  DLIO pipeline(params);
  DLIOResult last_result;
  double time = 0.0;

  for (int frame = 0; frame < 6; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = 0.6 * frame;
    pose(1, 3) = 0.1 * frame;

    std::vector<ImuSample> imu_samples;
    if (frame > 0) {
      imu_samples = generateImuSamples(time - 0.1, time);
    }

    last_result = pipeline.process(generateLidarScan(pose, rng), imu_samples);
    time += 0.1;
  }

  ASSERT_TRUE(last_result.initialized);
  ASSERT_TRUE(last_result.valid);
  EXPECT_TRUE(last_result.converged);
  EXPECT_TRUE(last_result.imu_used);
  EXPECT_GT(last_result.num_correspondences, 50);
  EXPECT_GT(last_result.num_keyframes, 1);
  EXPECT_LT((last_result.state.pose.block<3, 1>(0, 3) -
             Eigen::Vector3d(3.0, 0.5, 0.0))
                .norm(),
            4.0);
}

TEST(DLIO, KeepsSlidingWindowedMap) {
  std::mt19937 rng(7);
  DLIOParams params;
  params.max_keyframes_in_map = 3;
  params.gicp.max_correspondence_distance = 5.0;
  params.registration_voxel_size = 0.8;
  params.map_voxel_size = 1.0;

  DLIO pipeline(params);
  DLIOResult last_result;
  double time = 0.0;
  for (int frame = 0; frame < 8; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = 0.5 * frame;

    std::vector<ImuSample> imu_samples;
    if (frame > 0) {
      imu_samples = generateImuSamples(time - 0.1, time);
    }

    last_result = pipeline.process(generateLidarScan(pose, rng), imu_samples);
    time += 0.1;
  }

  EXPECT_LE(last_result.num_keyframes, 3);
  EXPECT_EQ(pipeline.numKeyframes(), 3);
}

TEST(DLIO, ClearResetsState) {
  std::mt19937 rng(11);
  DLIO pipeline;
  double time = 0.0;

  for (int frame = 0; frame < 4; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = 0.4 * frame;

    std::vector<ImuSample> imu_samples;
    if (frame > 0) {
      imu_samples = generateImuSamples(time - 0.1, time);
    }

    pipeline.process(generateLidarScan(pose, rng), imu_samples);
    time += 0.1;
  }

  EXPECT_GT(pipeline.numKeyframes(), 0);
  pipeline.clear();
  EXPECT_EQ(pipeline.numKeyframes(), 0);
  EXPECT_EQ(pipeline.mapSize(), 0u);
  EXPECT_TRUE(pipeline.state().pose.isApprox(Eigen::Matrix4d::Identity()));
}

TEST(DLIO, ImuFusionKeepsMotionPriorInStateUpdate) {
  std::mt19937 rng(123);
  const auto static_scan = generateLidarScan(Eigen::Matrix4d::Identity(), rng);

  DLIOParams no_fusion_params;
  no_fusion_params.gicp.max_correspondence_distance = 5.0;
  no_fusion_params.gicp.max_iterations = 20;
  no_fusion_params.gicp.k_neighbors = 15;
  no_fusion_params.registration_voxel_size = 0.7;
  no_fusion_params.map_voxel_size = 1.0;
  no_fusion_params.imu_rotation_fusion_weight = 0.0;
  no_fusion_params.imu_translation_fusion_weight = 0.0;
  no_fusion_params.imu_velocity_fusion_weight = 0.0;

  DLIOParams fused_params = no_fusion_params;
  fused_params.imu_rotation_fusion_weight = 1.0;
  fused_params.imu_translation_fusion_weight = 1.0;
  fused_params.imu_velocity_fusion_weight = 1.0;

  DLIO no_fusion_pipeline(no_fusion_params);
  DLIO fused_pipeline(fused_params);

  no_fusion_pipeline.process(static_scan);
  fused_pipeline.process(static_scan);

  const auto imu_samples = generateImuSamples(
      0.0, 0.1, Eigen::Vector3d(30.0, 0.0, 9.81));

  const auto no_fusion_result =
      no_fusion_pipeline.process(static_scan, imu_samples);
  const auto fused_result =
      fused_pipeline.process(static_scan, imu_samples);

  ASSERT_TRUE(no_fusion_result.initialized);
  ASSERT_TRUE(fused_result.initialized);
  EXPECT_TRUE(no_fusion_result.imu_used);
  EXPECT_TRUE(fused_result.imu_used);
  EXPECT_GT(fused_result.state.pose(0, 3),
            no_fusion_result.state.pose(0, 3) + 0.03);
  EXPECT_GT(fused_result.state.velocity.x(),
            no_fusion_result.state.velocity.x() + 0.3);
}
