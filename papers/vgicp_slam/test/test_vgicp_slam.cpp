#include "vgicp_slam/vgicp_slam.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::vgicp_slam;

namespace {

using localization_zoo::aloam::PointCloud;
using localization_zoo::aloam::PointCloudPtr;
using localization_zoo::aloam::PointT;

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

    for (double x = -18.0; x <= 18.0; x += 2.0) {
      for (double y = -18.0; y <= 18.0; y += 2.0) {
        map_points.emplace_back(x, y, 0.0);
      }
    }

    for (double x = -15.0; x <= 15.0; x += 1.5) {
      for (double z = 0.0; z <= 4.0; z += 0.5) {
        map_points.emplace_back(x, 16.0, z);
        map_points.emplace_back(-12.0, x, z);
      }
    }

    for (double y = -10.0; y <= 10.0; y += 1.2) {
      for (double z = 0.0; z <= 4.0; z += 0.5) {
        map_points.emplace_back(10.0, y, 0.4 + 0.1 * (y + 10.0) + 0.2 * z);
      }
    }

    for (double deg = 0.0; deg < 360.0; deg += 20.0) {
      const double theta = deg * M_PI / 180.0;
      for (double z = 0.0; z <= 4.0; z += 0.4) {
        map_points.emplace_back(5.0 + 0.8 * std::cos(theta),
                                5.0 + 0.8 * std::sin(theta), z);
        map_points.emplace_back(-7.0 + 0.7 * std::cos(theta),
                                9.0 + 0.7 * std::sin(theta), z);
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
    if (range < 1.0 || range > 50.0) {
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

VgicpSlamParams makeBaseParams() {
  VgicpSlamParams params;
  params.front_end.voxel_resolution = 0.8;
  params.front_end.min_points_per_voxel = 1;
  params.front_end.max_correspondence_distance = 4.5;
  params.front_end.max_iterations = 15;
  params.front_end.rotation_epsilon = 1e-3;
  params.front_end.translation_epsilon = 1e-3;
  params.registration_voxel_size = 0.5;
  params.map_voxel_size = 0.7;
  params.keyframe_translation_threshold = 0.2;
  params.scan_context.exclude_recent_frames = 2;
  params.scan_context.distance_threshold = 0.20;
  params.loop_vgicp = params.front_end;
  params.loop_vgicp.voxel_resolution = 0.9;
  params.loop_vgicp.max_correspondence_distance = 5.0;
  params.loop_vgicp.max_iterations = 20;
  params.loop_fitness_threshold = 2.0;
  params.min_loop_correspondences = 20;
  params.descriptor_stride = 2;
  params.loop_stride = 3;
  return params;
}

}  // namespace

TEST(VgicpSlam, TracksShortSequenceWithVoxelGicpFrontEnd) {
  VgicpSlam pipeline(makeBaseParams());
  VgicpSlamResult last_result;

  for (int frame = 0; frame < 6; ++frame) {
    const double yaw = frame * 2.0 * M_PI / 180.0;
    last_result =
        pipeline.process(generateLidarScan(makePose(0.7 * frame, 0.1 * frame, yaw)));
  }

  ASSERT_TRUE(last_result.initialized);
  ASSERT_TRUE(last_result.valid);
  EXPECT_TRUE(last_result.converged);
  EXPECT_GT(last_result.num_keyframes, 3);
  EXPECT_LT((last_result.t_w_curr - Eigen::Vector3d(3.5, 0.5, 0.0)).norm(), 4.5);
  EXPECT_LT(std::abs(last_result.t_w_curr.z()), 0.5);
}

TEST(VgicpSlam, DetectsLoopClosureOnOutAndBackRoute) {
  VgicpSlamParams params = makeBaseParams();
  params.min_loop_index_gap = 4;

  VgicpSlam pipeline(params);
  bool saw_loop = false;
  VgicpSlamResult last_result;

  const std::vector<double> x_positions = {0.0, 1.0, 2.0, 3.0, 2.0, 1.0, 0.0};
  for (double x : x_positions) {
    last_result = pipeline.process(generateLidarScan(makePose(x, 0.0, 0.0)));
    saw_loop = saw_loop || last_result.loop_detected;
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_TRUE(saw_loop);
  EXPECT_GT(pipeline.numLoopEdges(), 0);
  EXPECT_GT(last_result.num_keyframes, 4);
}

TEST(VgicpSlam, ClearResetsStateAndGraph) {
  VgicpSlam pipeline(makeBaseParams());
  pipeline.process(generateLidarScan(makePose(0.0, 0.0, 0.0)));
  ASSERT_TRUE(pipeline.initialized());
  ASSERT_GT(pipeline.numKeyframes(), 0);

  pipeline.clear();

  EXPECT_FALSE(pipeline.initialized());
  EXPECT_EQ(pipeline.numKeyframes(), 0);
  EXPECT_EQ(pipeline.numLoopEdges(), 0);
  EXPECT_EQ(pipeline.submapSize(), 0u);
  EXPECT_TRUE(pipeline.pose().isApprox(Eigen::Matrix4d::Identity()));
}
