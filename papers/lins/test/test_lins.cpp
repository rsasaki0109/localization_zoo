#include "lins/lins.h"

#include <gtest/gtest.h>

#include <cmath>
#include <random>

using namespace localization_zoo::lins;

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

  for (int i = 0; i < 1200; ++i) {
    add(Eigen::Vector3d(xy(rng), xy(rng), 0.0));
  }
  for (int i = 0; i < 400; ++i) {
    add(Eigen::Vector3d(xy(rng), 15.0, z(rng)));
    add(Eigen::Vector3d(-15.0, xy(rng), z(rng)));
  }
  for (int i = 0; i < 200; ++i) {
    const double theta = angle_dist(rng);
    add(Eigen::Vector3d(5.0 + 0.3 * std::cos(theta),
                        5.0 + 0.3 * std::sin(theta), z(rng)));
  }

  return cloud;
}

std::vector<ImuSample> generateImuSamples(double start_time, double end_time) {
  std::vector<ImuSample> samples;
  for (double timestamp = start_time; timestamp <= end_time + 1e-9;
       timestamp += 0.01) {
    ImuSample sample;
    sample.timestamp = timestamp;
    sample.gyro = Eigen::Vector3d::Zero();
    sample.accel = Eigen::Vector3d(0.0, 0.0, 9.81);
    samples.push_back(sample);
  }
  return samples;
}

}  // namespace

TEST(LINS, TracksShortSequenceWithIteratedFilter) {
  std::mt19937 rng(42);
  LINSParams params;
  params.max_correspondence_distance = 5.0;
  params.registration_voxel_size = 0.9;
  params.map_voxel_size = 1.2;
  params.max_correspondences = 96;
  params.filter.max_iterations = 4;

  LINS pipeline(params);
  LINSResult last_result;
  double time = 0.0;

  for (int frame = 0; frame < 5; ++frame) {
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
  EXPECT_GT(last_result.num_correspondences, 12);
  EXPECT_GT(last_result.num_keyframes, 1);
  EXPECT_LT((last_result.state.position - Eigen::Vector3d(2.4, 0.4, 0.0)).norm(),
            4.0);
}

TEST(LINS, KeepsSlidingWindowedMap) {
  std::mt19937 rng(7);
  LINSParams params;
  params.max_keyframes_in_map = 3;
  params.max_correspondence_distance = 5.0;
  params.registration_voxel_size = 0.9;
  params.map_voxel_size = 1.2;
  params.max_correspondences = 96;
  params.filter.max_iterations = 4;

  LINS pipeline(params);
  LINSResult last_result;
  double time = 0.0;
  for (int frame = 0; frame < 6; ++frame) {
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

TEST(LINS, ClearResetsState) {
  std::mt19937 rng(11);
  LINSParams params;
  params.max_correspondences = 96;
  params.filter.max_iterations = 4;
  LINS pipeline(params);
  double time = 0.0;

  for (int frame = 0; frame < 3; ++frame) {
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
  EXPECT_TRUE(pipeline.state().position.isZero(0.0));
}
