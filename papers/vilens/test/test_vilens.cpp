#include "vilens/vilens.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::vilens;

namespace {

using localization_zoo::imu_preintegration::ImuSample;

constexpr double kScanPeriod = 0.1;

Eigen::Matrix4d makePose(double x, double y, double z, double yaw_rad) {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  pose.block<3, 3>(0, 0) =
      Eigen::AngleAxisd(yaw_rad, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  pose.block<3, 1>(0, 3) = Eigen::Vector3d(x, y, z);
  return pose;
}

const std::vector<Eigen::Vector3d>& floorWorldPoints() {
  static const std::vector<Eigen::Vector3d> points = [] {
    std::vector<Eigen::Vector3d> map_points;
    for (double x = 0.0; x <= 24.0; x += 0.8) {
      for (double y = -12.0; y <= 12.0; y += 0.8) {
        map_points.emplace_back(x, y, 0.0);
      }
    }
    return map_points;
  }();
  return points;
}

std::vector<Landmark> makeLandmarks() {
  std::vector<Landmark> landmarks;
  int id = 0;
  for (double x = 4.0; x <= 24.0; x += 2.0) {
    for (double y : {-4.0, 0.0, 4.0}) {
      landmarks.push_back(
          Landmark{id++, Eigen::Vector3d(x, y, 1.5 + 0.2 * std::sin(0.3 * x + y))});
      landmarks.push_back(
          Landmark{id++, Eigen::Vector3d(x, y, 2.4 + 0.1 * std::cos(0.2 * x - y))});
    }
  }
  return landmarks;
}

std::vector<Eigen::Vector3d> generateScan(const Eigen::Matrix4d& pose) {
  std::vector<Eigen::Vector3d> raw_points;
  raw_points.reserve(floorWorldPoints().size());

  const Eigen::Matrix3d rotation = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d translation = pose.block<3, 1>(0, 3);
  for (const auto& world_point : floorWorldPoints()) {
    const Eigen::Vector3d sensor_point =
        rotation.transpose() * (world_point - translation);
    const double range = sensor_point.norm();
    if (range < 0.5 || range > 50.0) {
      continue;
    }
    raw_points.push_back(sensor_point);
  }
  return raw_points;
}

bool projectLandmark(const Landmark& landmark, const Eigen::Matrix4d& pose,
                     const VILENSParams& params, Eigen::Vector2d* pixel) {
  const Eigen::Matrix3d rotation = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d translation = pose.block<3, 1>(0, 3);

  const Eigen::Matrix3d R_wc = rotation * params.body_R_camera;
  const Eigen::Vector3d t_wc = translation + rotation * params.body_t_camera;
  const Eigen::Vector3d point_camera =
      R_wc.transpose() * (landmark.position - t_wc);

  if (point_camera.x() <= 1e-6) {
    return false;
  }

  const double u =
      params.camera.fx * point_camera.y() / point_camera.x() + params.camera.cx;
  const double v =
      params.camera.fy * point_camera.z() / point_camera.x() + params.camera.cy;
  if (u < 0.0 || u >= params.camera.width ||
      v < 0.0 || v >= params.camera.height) {
    return false;
  }

  pixel->x() = u;
  pixel->y() = v;
  return true;
}

std::vector<VisualObservation> generateObservations(
    const Eigen::Matrix4d& pose, const std::vector<Landmark>& landmarks,
    const VILENSParams& params) {
  std::vector<VisualObservation> observations;
  observations.reserve(landmarks.size());
  for (const auto& landmark : landmarks) {
    Eigen::Vector2d pixel;
    if (!projectLandmark(landmark, pose, params, &pixel)) {
      continue;
    }
    observations.push_back(VisualObservation{landmark.id, pixel});
  }
  return observations;
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
    sample.accel = Eigen::Vector3d(0.0, 0.0, 9.81);
    samples.push_back(sample);
  }
  return samples;
}

}  // namespace

TEST(VILENS, VisualFusionImprovesDegenerateLidarSequence) {
  VILENSParams params;
  params.registration_voxel_size = 0.6;
  params.map_voxel_size = 0.8;
  params.max_correspondence_distance = 4.0;
  params.max_correspondences = 120;
  params.keyframe_translation_threshold = 0.25;
  params.lidar_prior_translation_weight = 0.1;
  params.lidar_prior_rotation_weight = 0.1;
  params.min_visual_observations = 6;

  const auto landmarks = makeLandmarks();

  VILENS lidar_only(params);
  VILENS visual_pipeline(params);
  visual_pipeline.setLandmarks(landmarks);

  VILENSResult last_lidar;
  VILENSResult last_visual;
  for (int frame = 0; frame < 6; ++frame) {
    const Eigen::Matrix4d pose = makePose(0.6 * frame, 0.0, 1.0, 0.0);
    const std::vector<Eigen::Vector3d> scan = generateScan(pose);
    const std::vector<VisualObservation> observations =
        generateObservations(pose, landmarks, params);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kScanPeriod, frame * kScanPeriod,
                                    0.0, 0.0);

    ASSERT_GE(static_cast<int>(observations.size()), params.min_visual_observations);
    last_lidar = lidar_only.process(scan, {}, imu_samples);
    last_visual = visual_pipeline.process(scan, observations, imu_samples);
  }

  const Eigen::Vector3d ground_truth(3.0, 0.0, 1.0);
  const double lidar_error = (last_lidar.state.position - ground_truth).norm();
  const double visual_error = (last_visual.state.position - ground_truth).norm();

  ASSERT_TRUE(last_visual.valid);
  EXPECT_TRUE(last_visual.visual_used);
  EXPECT_GT(last_visual.num_visual_factors, 5);
  EXPECT_GT(last_visual.map_points, 0u);
  EXPECT_LT(visual_error, lidar_error);
  EXPECT_LT(visual_error, 1.0);
}

TEST(VILENS, SlidingWindowMapBoundsKeyframeCount) {
  VILENSParams params;
  params.max_keyframes_in_map = 2;
  params.keyframe_translation_threshold = 0.25;
  params.lidar_prior_translation_weight = 0.1;
  params.lidar_prior_rotation_weight = 0.1;

  const auto landmarks = makeLandmarks();
  VILENS pipeline(params);
  pipeline.setLandmarks(landmarks);

  for (int frame = 0; frame < 4; ++frame) {
    const Eigen::Matrix4d pose = makePose(0.8 * frame, 0.0, 1.0, 0.0);
    const std::vector<Eigen::Vector3d> scan = generateScan(pose);
    const std::vector<VisualObservation> observations =
        generateObservations(pose, landmarks, params);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kScanPeriod, frame * kScanPeriod,
                                    0.0, 0.0);
    pipeline.process(scan, observations, imu_samples);
  }

  EXPECT_TRUE(pipeline.state().position.allFinite());
  EXPECT_EQ(pipeline.numKeyframes(), 2);
  EXPECT_GT(pipeline.mapSize(), 0u);
  EXPECT_EQ(pipeline.numLandmarks(), landmarks.size());
}

TEST(VILENS, ClearResetsStateAndMap) {
  VILENSParams params;
  params.keyframe_translation_threshold = 0.25;
  params.lidar_prior_translation_weight = 0.1;
  params.lidar_prior_rotation_weight = 0.1;

  const auto landmarks = makeLandmarks();
  VILENS pipeline(params);
  pipeline.setLandmarks(landmarks);

  for (int frame = 0; frame < 3; ++frame) {
    const Eigen::Matrix4d pose = makePose(0.8 * frame, 0.0, 1.0, 0.0);
    const std::vector<Eigen::Vector3d> scan = generateScan(pose);
    const std::vector<VisualObservation> observations =
        generateObservations(pose, landmarks, params);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kScanPeriod, frame * kScanPeriod,
                                    0.0, 0.0);
    pipeline.process(scan, observations, imu_samples);
  }
  ASSERT_GT(pipeline.mapSize(), 0u);
  ASSERT_GT(pipeline.numLandmarks(), 0u);

  pipeline.clear();

  EXPECT_EQ(pipeline.numKeyframes(), 0);
  EXPECT_EQ(pipeline.mapSize(), 0u);
  EXPECT_EQ(pipeline.state().position, Eigen::Vector3d::Zero());
  EXPECT_EQ(pipeline.numLandmarks(), landmarks.size());

  pipeline.clearLandmarks();
  EXPECT_EQ(pipeline.numLandmarks(), 0u);
}
