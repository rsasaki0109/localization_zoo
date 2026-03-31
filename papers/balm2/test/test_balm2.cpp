#include "balm2/balm2.h"

#include <gtest/gtest.h>

#include <cmath>
#include <random>

using namespace localization_zoo::balm2;

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

}  // namespace

TEST(BALM2, TracksShortSequenceWithLocalBundleAdjustment) {
  std::mt19937 rng(42);
  Balm2Params params;
  params.window_size = 4;
  params.knn_max_dist_sq = 6.0;
  params.corner_stride = 3;
  params.surface_stride = 4;

  BALM2 pipeline(params);
  Balm2Result last_result;

  for (int frame = 0; frame < 6; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = 0.6 * frame;
    pose(1, 3) = 0.1 * frame;
    last_result = pipeline.process(generateLidarScan(pose, rng));
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_GT(last_result.num_keyframes, 0);
  EXPECT_GT(last_result.num_window_keyframes, 1);
  EXPECT_GT(last_result.num_line_constraints, 10);
  EXPECT_GT(last_result.num_plane_constraints, 10);
  EXPECT_LT((last_result.t_w_curr - Eigen::Vector3d(3.0, 0.5, 0.0)).norm(), 4.0);
}

TEST(BALM2, KeepsBoundedOptimizationWindow) {
  std::mt19937 rng(7);
  Balm2Params params;
  params.window_size = 3;
  params.knn_max_dist_sq = 6.0;
  params.corner_stride = 3;
  params.surface_stride = 4;

  BALM2 pipeline(params);
  Balm2Result last_result;
  for (int frame = 0; frame < 8; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = 0.5 * frame;
    last_result = pipeline.process(generateLidarScan(pose, rng));
  }

  EXPECT_EQ(pipeline.currentWindowSize(), 3);
  EXPECT_LE(last_result.num_window_keyframes, 3);
  EXPECT_GE(last_result.num_keyframes, 6);
}

TEST(BALM2, ClearResetsState) {
  std::mt19937 rng(11);
  BALM2 pipeline;

  for (int frame = 0; frame < 4; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = 0.4 * frame;
    pipeline.process(generateLidarScan(pose, rng));
  }

  EXPECT_GT(pipeline.numKeyframes(), 0);
  pipeline.clear();
  EXPECT_EQ(pipeline.numKeyframes(), 0);
  EXPECT_EQ(pipeline.currentWindowSize(), 0);
}
