#include "floam/floam.h"

#include <gtest/gtest.h>

#include <random>

using namespace localization_zoo::floam;

namespace {

using localization_zoo::aloam::PointCloud;
using localization_zoo::aloam::PointCloudPtr;
using localization_zoo::aloam::PointT;

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

  for (int i = 0; i < 3000; ++i) {
    add(Eigen::Vector3d(xy(rng), xy(rng), 0.0));
  }
  for (int i = 0; i < 1000; ++i) {
    add(Eigen::Vector3d(xy(rng), 15.0, z(rng)));
    add(Eigen::Vector3d(-15.0, xy(rng), z(rng)));
  }
  for (int i = 0; i < 500; ++i) {
    const float theta = angle_dist(rng);
    add(Eigen::Vector3d(5.0 + 0.3 * std::cos(theta),
                        5.0 + 0.3 * std::sin(theta), z(rng)));
  }

  return cloud;
}

}  // namespace

TEST(FLOAM, TracksShortSequenceWithSparseMapping) {
  std::mt19937 rng(42);
  FLoam pipeline;
  FLoamResult last_result;

  for (int frame = 0; frame < 6; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = static_cast<double>(frame);
    last_result = pipeline.process(generateLidarScan(pose, rng));
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_GT(last_result.mapping_updates, 0);
  EXPECT_GT(last_result.num_corner_less_sharp, 0u);
  EXPECT_GT(last_result.num_surf_less_flat, 0u);
  EXPECT_LT(last_result.num_processed_points, last_result.num_input_points);
  EXPECT_LT((last_result.t_w_curr - Eigen::Vector3d(5.0, 0.0, 0.0)).norm(), 5.0);
}

TEST(FLOAM, MappingUpdatesAreThrottled) {
  std::mt19937 rng(7);
  FLoamParams params;
  params.mapping_update_interval = 3;
  params.input_point_stride = 3;

  FLoam pipeline(params);
  FLoamResult last_result;

  for (int frame = 0; frame < 7; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = 0.8 * frame;
    last_result = pipeline.process(generateLidarScan(pose, rng));
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_EQ(pipeline.frameCount(), 6);
  EXPECT_LT(pipeline.mappingUpdates(), pipeline.frameCount());
  EXPECT_GT(pipeline.mappingUpdates(), 0);
}

TEST(FLOAM, ClearResetsState) {
  std::mt19937 rng(11);
  FLoam pipeline;

  for (int frame = 0; frame < 4; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = 0.6 * frame;
    pipeline.process(generateLidarScan(pose, rng));
  }

  EXPECT_GT(pipeline.frameCount(), 0);
  EXPECT_GT(pipeline.mappingUpdates(), 0);
  pipeline.clear();
  EXPECT_EQ(pipeline.frameCount(), 0);
  EXPECT_EQ(pipeline.mappingUpdates(), 0);
  EXPECT_EQ(pipeline.position(), Eigen::Vector3d::Zero());
}
