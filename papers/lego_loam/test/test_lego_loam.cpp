#include "lego_loam/lego_loam.h"

#include <gtest/gtest.h>

#include <random>

using namespace localization_zoo::lego_loam;

namespace {

using localization_zoo::aloam::PointCloud;
using localization_zoo::aloam::PointCloudPtr;
using localization_zoo::aloam::PointT;

constexpr double kSensorHeight = 1.8;

PointCloudPtr generateGroundVehicleScan(const Eigen::Matrix4d& pose,
                                        std::mt19937& rng) {
  auto cloud = PointCloudPtr(new PointCloud);
  std::normal_distribution<float> noise(0.0f, 0.01f);
  std::uniform_real_distribution<double> xy(-30.0, 30.0);
  std::uniform_real_distribution<double> z(0.0, 5.0);
  std::uniform_real_distribution<double> pillar_angle(0.0, 2.0 * M_PI);

  const Eigen::Matrix3d rotation = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d translation = pose.block<3, 1>(0, 3);

  auto add_world_point = [&](const Eigen::Vector3d& world_point) {
    const Eigen::Vector3d sensor_point =
        rotation.transpose() * (world_point - translation);
    const double range = sensor_point.norm();
    if (range < 1.0 || range > 80.0) {
      return;
    }

    PointT point;
    point.x = sensor_point.x() + noise(rng);
    point.y = sensor_point.y() + noise(rng);
    point.z = sensor_point.z() + noise(rng);
    point.intensity = 0.0f;
    cloud->push_back(point);
  };

  for (int i = 0; i < 4000; ++i) {
    add_world_point(Eigen::Vector3d(xy(rng), xy(rng), 0.0));
  }
  for (int i = 0; i < 1200; ++i) {
    add_world_point(Eigen::Vector3d(xy(rng), 18.0, z(rng)));
    add_world_point(Eigen::Vector3d(-14.0, xy(rng), z(rng)));
  }
  for (int i = 0; i < 800; ++i) {
    const double theta = pillar_angle(rng);
    add_world_point(
        Eigen::Vector3d(8.0 + 0.7 * std::cos(theta), 6.0 + 0.7 * std::sin(theta),
                        z(rng)));
  }

  return cloud;
}

}  // namespace

TEST(LeGOLOAM, ExtractsGroundAwareFeatures) {
  std::mt19937 rng(42);
  GroundAwareScanRegistrationParams params;
  params.sensor_height = static_cast<float>(kSensorHeight);
  GroundAwareScanRegistration registration(params);

  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  pose(2, 3) = kSensorHeight;
  const auto cloud = generateGroundVehicleScan(pose, rng);
  const auto features = registration.extract(cloud);

  EXPECT_GT(features.num_ground_points, 100u);
  EXPECT_GT(features.features.corner_less_sharp->size(), 20u);
  EXPECT_GT(features.features.surf_less_flat->size(), 50u);
  EXPECT_GT(features.features.full_cloud->size(), 100u);
}

TEST(LeGOLOAM, FullPipelineTracksShortSequence) {
  std::mt19937 rng(7);
  LeGOLOAMParams params;
  params.scan_registration.sensor_height = static_cast<float>(kSensorHeight);
  params.scan_registration.curvature_threshold = 0.04f;
  params.scan_registration.ground_scan_limit = 6;
  params.odometry.distance_sq_threshold = 16.0;
  params.mapping.knn_max_dist_sq = 4.0;

  LeGOLOAM pipeline(params);
  LeGOLOAMResult last_result;

  for (int frame = 0; frame < 6; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = 0.6 * frame;
    pose(1, 3) = 0.1 * frame;
    pose(2, 3) = kSensorHeight;
    const auto cloud = generateGroundVehicleScan(pose, rng);
    last_result = pipeline.process(cloud);
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_GT(last_result.num_ground_points, 100u);
  EXPECT_GT(last_result.num_corner_features, 20u);
  EXPECT_GT(last_result.num_surface_features, 50u);
  EXPECT_LT((last_result.t_w_curr - Eigen::Vector3d(3.0, 0.5, kSensorHeight)).norm(),
            4.0);
}
