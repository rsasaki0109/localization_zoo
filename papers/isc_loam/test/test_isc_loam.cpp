#include "isc_loam/isc_loam.h"

#include <gtest/gtest.h>

#include <Eigen/Geometry>

#include <cmath>
#include <vector>

using namespace localization_zoo::isc_loam;

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

float intensityFromWorldPoint(const Eigen::Vector3d& world_point) {
  const double base = 15.0 + 0.3 * world_point.x() + 0.2 * world_point.y() +
                      0.5 * world_point.z();
  const double harmonic = 4.0 * std::sin(0.35 * world_point.x()) +
                          3.0 * std::cos(0.27 * world_point.y());
  return static_cast<float>(std::max(1.0, base + harmonic));
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
    point.intensity = intensityFromWorldPoint(world_point);
    cloud->push_back(point);
  }

  return cloud;
}

PointCloudPtr rotateCloudYaw(const PointCloudPtr& cloud, double yaw_rad) {
  auto rotated = PointCloudPtr(new PointCloud);
  const Eigen::Matrix3d rotation =
      Eigen::AngleAxisd(yaw_rad, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  rotated->reserve(cloud->size());
  for (const auto& point : cloud->points) {
    const Eigen::Vector3d p(point.x, point.y, point.z);
    const Eigen::Vector3d pr = rotation * p;
    PointT out = point;
    out.x = static_cast<float>(pr.x());
    out.y = static_cast<float>(pr.y());
    out.z = static_cast<float>(pr.z());
    rotated->push_back(out);
  }
  return rotated;
}

int cyclicShiftError(int lhs, int rhs, int modulus) {
  const int wrapped = (lhs - rhs + modulus) % modulus;
  return std::min(wrapped, modulus - wrapped);
}

IscLoamParams makeBaseParams() {
  IscLoamParams params;
  params.front_end.input_point_stride = 1;
  params.front_end.mapping_update_interval = 1;
  params.keyframe_translation_threshold = 0.2;
  params.scan_context.exclude_recent_frames = 2;
  params.scan_context.distance_threshold = 0.20;
  params.scan_context.search_ratio = 0.2;
  params.loop_gicp.max_correspondence_distance = 5.0;
  params.loop_gicp.max_iterations = 20;
  params.loop_gicp.k_neighbors = 15;
  params.loop_fitness_threshold = 1.5;
  params.min_loop_correspondences = 60;
  params.loop_stride = 3;
  return params;
}

}  // namespace

TEST(IscLoam, IntensityScanContextDetectsYawRotatedLoopCandidate) {
  IntensityScanContextParams params;
  params.exclude_recent_frames = 0;
  params.num_candidates = 3;
  params.distance_threshold = 0.20;

  IntensityScanContextManager manager(params);
  const auto base_scan = generateLidarScan(makePose(0.0, 0.0, 0.0));
  manager.addScan(base_scan);

  const double yaw_deg = 24.0;
  const auto rotated_query = rotateCloudYaw(base_scan, yaw_deg * M_PI / 180.0);
  const IntensityLoopCandidate candidate = manager.detectLoop(rotated_query);

  ASSERT_TRUE(candidate.valid);
  EXPECT_EQ(candidate.index, 0);
  EXPECT_LT(candidate.distance, 0.20);

  const int expected_shift =
      static_cast<int>(std::round(yaw_deg / 360.0 * params.num_sectors)) %
      params.num_sectors;
  const int reverse_shift =
      (params.num_sectors - expected_shift) % params.num_sectors;
  EXPECT_LE(std::min(cyclicShiftError(candidate.yaw_shift, expected_shift,
                                      params.num_sectors),
                     cyclicShiftError(candidate.yaw_shift, reverse_shift,
                                      params.num_sectors)),
            1);
}

TEST(IscLoam, TracksShortSequenceWithFloamFrontEnd) {
  IscLoam pipeline(makeBaseParams());
  IscLoamResult last_result;

  for (int frame = 0; frame < 6; ++frame) {
    const double yaw = frame * 2.0 * M_PI / 180.0;
    last_result =
        pipeline.process(generateLidarScan(makePose(0.7 * frame, 0.1 * frame, yaw)));
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_GT(last_result.num_keyframes, 3);
  EXPECT_EQ(last_result.num_loop_edges, 0);
  EXPECT_LT((last_result.t_w_curr - Eigen::Vector3d(3.5, 0.5, 0.0)).norm(), 5.0);
}

TEST(IscLoam, DetectsLoopClosureOnOutAndBackRoute) {
  IscLoamParams params = makeBaseParams();
  params.min_loop_index_gap = 4;

  IscLoam pipeline(params);
  bool saw_loop = false;
  IscLoamResult last_result;

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

TEST(IscLoam, ClearResetsFrontEndAndGraph) {
  IscLoam pipeline(makeBaseParams());
  for (int frame = 0; frame < 6; ++frame) {
    const double yaw = frame * 2.0 * M_PI / 180.0;
    pipeline.process(generateLidarScan(makePose(0.7 * frame, 0.1 * frame, yaw)));
  }
  ASSERT_GT(pipeline.numKeyframes(), 0);

  pipeline.clear();

  EXPECT_EQ(pipeline.numKeyframes(), 0);
  EXPECT_EQ(pipeline.numLoopEdges(), 0);
  EXPECT_EQ(pipeline.latestOptimizedPoseMatrix(), Eigen::Matrix4d::Identity());
}
