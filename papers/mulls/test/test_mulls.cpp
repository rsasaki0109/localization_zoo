#include "mulls/mulls.h"

#include <gtest/gtest.h>

#include <random>

using namespace localization_zoo::mulls;

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

TEST(MULLS, TracksShortSequenceWithPointMetric) {
  std::mt19937 rng(42);
  MULLSParams params;
  params.mapping.knn_max_dist_sq = 4.0;
  params.mapping.point_neighbor_max_dist_sq = 4.0;
  params.mapping.point_weight = 0.2;

  MULLS pipeline(params);
  MULLSResult last_result;

  for (int frame = 0; frame < 6; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = 0.6 * frame;
    pose(1, 3) = 0.1 * frame;
    const auto cloud = generateLidarScan(pose, rng);
    last_result = pipeline.process(cloud);
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_GT(last_result.num_corner_features, 0u);
  EXPECT_GT(last_result.num_surface_features, 0u);
  EXPECT_GT(last_result.num_line_constraints, 10);
  EXPECT_GT(last_result.num_plane_constraints, 10);
  EXPECT_GT(last_result.num_point_constraints, 10);
  EXPECT_LT((last_result.t_w_curr - Eigen::Vector3d(3.0, 0.5, 0.0)).norm(), 4.0);
}

TEST(MULLS, KeepsSlidingWindowedMap) {
  std::mt19937 rng(7);
  MULLSParams params;
  params.mapping.max_frames_in_map = 2;
  params.mapping.knn_max_dist_sq = 4.0;
  params.mapping.point_neighbor_max_dist_sq = 4.0;

  MULLS pipeline(params);
  MULLSResult last_result;
  for (int frame = 0; frame < 5; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = 0.5 * frame;
    const auto cloud = generateLidarScan(pose, rng);
    last_result = pipeline.process(cloud);
  }

  EXPECT_LE(last_result.num_frames_in_map, 2u);
  EXPECT_EQ(pipeline.numFramesInMap(), 2u);
}
