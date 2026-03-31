#include "lvi_sam/lvi_sam.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::lvi_sam;

namespace {

using localization_zoo::aloam::PointCloud;
using localization_zoo::aloam::PointCloudPtr;
using localization_zoo::aloam::PointT;
using localization_zoo::imu_preintegration::ImuSample;

constexpr double kScanPeriod = 0.1;

Eigen::Matrix4d makePose(double x, double y, double z, double yaw_rad) {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  pose.block<3, 3>(0, 0) =
      Eigen::AngleAxisd(yaw_rad, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  pose.block<3, 1>(0, 3) = Eigen::Vector3d(x, y, z);
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

    const double vertical_deg =
        std::atan2(sensor_point.z(),
                   std::sqrt(sensor_point.x() * sensor_point.x() +
                             sensor_point.y() * sensor_point.y())) *
        180.0 / M_PI;
    if (vertical_deg < -15.0 || vertical_deg > 15.0) {
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

std::vector<Landmark> makeLandmarks() {
  std::vector<Landmark> landmarks;
  int id = 0;
  for (double x = 2.0; x <= 18.0; x += 2.0) {
    for (double y : {-6.0, -2.0, 2.0, 6.0}) {
      landmarks.push_back(
          Landmark{id++, Eigen::Vector3d(x, y, 1.8 + 0.1 * std::sin(0.3 * x + y))});
      landmarks.push_back(
          Landmark{id++, Eigen::Vector3d(x, y, 2.6 + 0.1 * std::cos(0.2 * x - y))});
    }
  }
  return landmarks;
}

bool projectLandmark(const Landmark& landmark, const Eigen::Matrix4d& pose,
                     const LviSamParams& params, Eigen::Vector2d* pixel) {
  const Eigen::Matrix3d rotation = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d translation = pose.block<3, 1>(0, 3);

  const Eigen::Quaterniond q_wc = Eigen::Quaterniond(rotation) *
                                  Eigen::Quaterniond(params.body_R_camera);
  const Eigen::Vector3d t_wc = translation + rotation * params.body_t_camera;
  const Eigen::Vector3d point_camera =
      q_wc.conjugate() * (landmark.position - t_wc);
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
    const LviSamParams& params) {
  std::vector<VisualObservation> observations;
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
    sample.accel = Eigen::Vector3d::Zero();
    samples.push_back(sample);
  }
  return samples;
}

LviSamParams makeBaseParams() {
  LviSamParams params;
  params.scan_context.exclude_recent_frames = 2;
  params.odometry.distance_sq_threshold = 16.0;
  params.mapping.knn_max_dist_sq = 4.0;
  params.keyframe_translation_threshold = 0.2;
  params.loop_gicp.max_correspondence_distance = 4.0;
  params.min_visual_observations = 6;
  return params;
}

}  // namespace

TEST(LVISAM, TracksSequenceWithVisualFactors) {
  LviSamParams params = makeBaseParams();
  params.enable_loop_closure = false;
  const auto landmarks = makeLandmarks();

  LviSam pipeline(params);
  pipeline.setLandmarks(landmarks);

  LviSamResult last_result;
  for (int frame = 0; frame < 6; ++frame) {
    const double yaw = frame * 2.0 * M_PI / 180.0;
    const Eigen::Matrix4d pose = makePose(0.7 * frame, 0.1 * frame, 0.0, yaw);
    const auto cloud = generateLidarScan(pose);
    const std::vector<VisualObservation> observations =
        generateObservations(pose, landmarks, params);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kScanPeriod, frame * kScanPeriod,
                                    (frame - 1) * 2.0 * M_PI / 180.0, yaw);

    ASSERT_GE(static_cast<int>(observations.size()), params.min_visual_observations);
    last_result = pipeline.process(cloud, observations, imu_samples);
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_TRUE(last_result.visual_used);
  EXPECT_GT(last_result.num_visual_factors, 5);
  EXPECT_GT(last_result.num_keyframes, 3);
  EXPECT_GT(pipeline.numImuEdges(), 0);
  EXPECT_EQ(pipeline.numLandmarks(), landmarks.size());
  EXPECT_TRUE(pipeline.latestOptimizedPoseMatrix().allFinite());
  EXPECT_LT(
      (last_result.t_w_curr - Eigen::Vector3d(3.5, 0.5, 0.0)).norm(), 5.0);
}

TEST(LVISAM, DetectsLoopClosureOnOutAndBackRoute) {
  LviSamParams params = makeBaseParams();
  params.scan_context.distance_threshold = 0.20;
  params.min_loop_index_gap = 4;
  params.loop_fitness_threshold = 1.5;
  params.min_loop_correspondences = 60;
  const auto landmarks = makeLandmarks();

  LviSam pipeline(params);
  pipeline.setLandmarks(landmarks);

  bool saw_loop = false;
  bool saw_visual = false;
  LviSamResult last_result;

  const std::vector<double> x_positions = {0.0, 1.0, 2.0, 3.0, 2.0, 1.0, 0.0};
  for (size_t frame = 0; frame < x_positions.size(); ++frame) {
    const Eigen::Matrix4d pose = makePose(x_positions[frame], 0.0, 0.0, 0.0);
    const auto cloud = generateLidarScan(pose);
    const std::vector<VisualObservation> observations =
        generateObservations(pose, landmarks, params);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kScanPeriod, frame * kScanPeriod,
                                    0.0, 0.0);
    last_result = pipeline.process(cloud, observations, imu_samples);
    saw_loop = saw_loop || last_result.loop_detected;
    saw_visual = saw_visual || last_result.visual_used;
  }

  ASSERT_TRUE(last_result.valid);
  EXPECT_TRUE(saw_loop);
  EXPECT_TRUE(saw_visual);
  EXPECT_GT(pipeline.numLoopEdges(), 0);
  EXPECT_GT(last_result.num_keyframes, 4);
}

TEST(LVISAM, ClearResetsGraphAndKeepsLandmarksUntilExplicitlyCleared) {
  LviSamParams params = makeBaseParams();
  params.enable_loop_closure = false;
  const auto landmarks = makeLandmarks();

  LviSam pipeline(params);
  pipeline.setLandmarks(landmarks);

  for (int frame = 0; frame < 3; ++frame) {
    const Eigen::Matrix4d pose = makePose(0.5 * frame, 0.0, 0.0, 0.0);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kScanPeriod, frame * kScanPeriod,
                                    0.0, 0.0);
    pipeline.process(generateLidarScan(pose),
                     generateObservations(pose, landmarks, params),
                     imu_samples);
  }

  ASSERT_TRUE(pipeline.initialized());
  ASSERT_GT(pipeline.numKeyframes(), 0);

  pipeline.clear();

  EXPECT_FALSE(pipeline.initialized());
  EXPECT_EQ(pipeline.numKeyframes(), 0);
  EXPECT_TRUE(
      pipeline.latestOptimizedPoseMatrix().isApprox(Eigen::Matrix4d::Identity()));
  EXPECT_EQ(pipeline.numLandmarks(), landmarks.size());

  pipeline.clearLandmarks();
  EXPECT_EQ(pipeline.numLandmarks(), 0u);
}
