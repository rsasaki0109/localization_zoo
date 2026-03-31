#include "hdl_graph_slam/hdl_graph_slam.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::hdl_graph_slam;

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

    PointT point;
    point.x = static_cast<float>(sensor_point.x());
    point.y = static_cast<float>(sensor_point.y());
    point.z = static_cast<float>(sensor_point.z());
    point.intensity = 0.0f;
    cloud->push_back(point);
  }

  return cloud;
}

HdlGraphSlamParams makeBaseParams() {
  HdlGraphSlamParams params;
  params.ndt.resolution = 1.0;
  params.ndt.max_iterations = 20;
  params.ndt.step_size = 0.2;
  params.ndt.convergence_threshold = 1e-3;
  params.keyframe_translation_threshold = 0.2;
  params.scan_context.exclude_recent_frames = 2;
  params.descriptor_stride = 2;
  params.loop_stride = 3;
  params.loop_gicp.max_correspondence_distance = 5.0;
  params.loop_fitness_threshold = 1.5;
  params.min_loop_correspondences = 60;
  return params;
}

}  // namespace

TEST(HdlGraphSlam, TracksShortSequenceWithNdtFrontEnd) {
  HdlGraphSlam pipeline(makeBaseParams());
  HdlGraphSlamResult last_result;

  for (int frame = 0; frame < 6; ++frame) {
    const double yaw = frame * 2.0 * M_PI / 180.0;
    const Eigen::Matrix4d pose = makePose(0.7 * frame, 0.1 * frame, yaw);
    last_result = pipeline.process(generateLidarScan(pose));
  }

  ASSERT_TRUE(last_result.initialized);
  ASSERT_TRUE(last_result.valid);
  EXPECT_GT(last_result.num_keyframes, 3);
  EXPECT_LT((last_result.t_w_curr - Eigen::Vector3d(3.5, 0.5, 0.0)).norm(), 4.5);
  EXPECT_LT(std::abs(last_result.t_w_curr.z()), 0.5);
}

TEST(HdlGraphSlam, DetectsLoopClosureOnOutAndBackRoute) {
  HdlGraphSlamParams params = makeBaseParams();
  params.min_loop_index_gap = 4;
  params.scan_context.distance_threshold = 0.20;

  HdlGraphSlam pipeline(params);
  bool saw_loop = false;
  HdlGraphSlamResult last_result;

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

TEST(HdlGraphSlam, ClearResetsStateAndGraph) {
  HdlGraphSlam pipeline(makeBaseParams());
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
