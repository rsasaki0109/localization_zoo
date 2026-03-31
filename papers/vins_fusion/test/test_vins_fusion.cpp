#include "vins_fusion/vins_fusion.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::vins_fusion;

namespace {

using localization_zoo::imu_preintegration::ImuSample;

constexpr double kFrameDt = 0.1;

Eigen::Matrix4d makePose(double x, double y, double z, double yaw_rad) {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  pose.block<3, 3>(0, 0) =
      Eigen::AngleAxisd(yaw_rad, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  pose.block<3, 1>(0, 3) = Eigen::Vector3d(x, y, z);
  return pose;
}

std::vector<Landmark> makeLandmarks() {
  std::vector<Landmark> landmarks;
  int id = 0;
  for (double x = 3.0; x <= 24.0; x += 2.0) {
    for (double y : {-6.0, -3.0, 0.0, 3.0, 6.0}) {
      landmarks.push_back(
          Landmark{id++, Eigen::Vector3d(x, y, 1.4 + 0.2 * std::sin(0.2 * x + y))});
      landmarks.push_back(
          Landmark{id++, Eigen::Vector3d(x, y, 2.3 + 0.1 * std::cos(0.3 * x - y))});
    }
  }
  return landmarks;
}

bool projectLandmark(const Landmark& landmark, const Eigen::Matrix4d& pose,
                     const VinsFusionParams& params, Eigen::Vector2d* pixel) {
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
    const VinsFusionParams& params) {
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
    sample.accel = Eigen::Vector3d::Zero();
    samples.push_back(sample);
  }
  return samples;
}

VinsFusionParams makeBaseParams() {
  VinsFusionParams params;
  params.initial_velocity = Eigen::Vector3d(7.0, 0.0, 0.0);
  params.keyframe_translation_threshold = 0.2;
  params.max_keyframes = 10;
  params.min_visual_observations = 8;
  return params;
}

}  // namespace

TEST(VINSFusion, VisualFactorsImproveImuPrediction) {
  VinsFusionParams params = makeBaseParams();
  const auto landmarks = makeLandmarks();

  VinsFusion pipeline(params);
  pipeline.setLandmarks(landmarks);

  VinsFusionResult last_result;
  for (int frame = 0; frame < 6; ++frame) {
    const double yaw = frame * 2.0 * M_PI / 180.0;
    const Eigen::Matrix4d pose = makePose(0.6 * frame, 0.1 * frame, 1.0, yaw);
    const std::vector<VisualObservation> observations =
        generateObservations(pose, landmarks, params);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kFrameDt, frame * kFrameDt,
                                    (frame - 1) * 2.0 * M_PI / 180.0, yaw);

    ASSERT_GE(static_cast<int>(observations.size()), params.min_visual_observations);
    last_result = pipeline.process(observations, imu_samples);
  }

  const Eigen::Vector3d ground_truth(3.0, 0.5, 1.0);
  const double raw_error = (last_result.raw_t_w_curr - ground_truth).norm();
  const double optimized_error = (last_result.t_w_curr - ground_truth).norm();

  ASSERT_TRUE(last_result.valid);
  EXPECT_TRUE(last_result.imu_used);
  EXPECT_TRUE(last_result.visual_used);
  EXPECT_GT(last_result.num_visual_factors, 7);
  EXPECT_GT(last_result.num_keyframes, 3);
  EXPECT_LT(optimized_error, raw_error);
  EXPECT_LT(optimized_error, 0.6);
}

TEST(VINSFusion, SlidingWindowBoundsKeyframes) {
  VinsFusionParams params = makeBaseParams();
  params.max_keyframes = 2;
  const auto landmarks = makeLandmarks();

  VinsFusion pipeline(params);
  pipeline.setLandmarks(landmarks);

  for (int frame = 0; frame < 4; ++frame) {
    const Eigen::Matrix4d pose = makePose(0.6 * frame, 0.0, 1.0, 0.0);
    const std::vector<VisualObservation> observations =
        generateObservations(pose, landmarks, params);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kFrameDt, frame * kFrameDt,
                                    0.0, 0.0);
    pipeline.process(observations, imu_samples);
  }

  EXPECT_TRUE(pipeline.initialized());
  EXPECT_EQ(pipeline.numKeyframes(), 2);
  EXPECT_EQ(pipeline.numImuEdges(), 1);
  EXPECT_EQ(pipeline.numLandmarks(), landmarks.size());
  EXPECT_TRUE(pipeline.latestOptimizedPoseMatrix().allFinite());
}

TEST(VINSFusion, ClearResetsStateAndKeepsLandmarksUntilExplicitlyCleared) {
  VinsFusionParams params = makeBaseParams();
  const auto landmarks = makeLandmarks();

  VinsFusion pipeline(params);
  pipeline.setLandmarks(landmarks);

  for (int frame = 0; frame < 3; ++frame) {
    const Eigen::Matrix4d pose = makePose(0.6 * frame, 0.0, 1.0, 0.0);
    const std::vector<VisualObservation> observations =
        generateObservations(pose, landmarks, params);
    const std::vector<ImuSample> imu_samples =
        frame == 0 ? std::vector<ImuSample>()
                   : makeImuSamples((frame - 1) * kFrameDt, frame * kFrameDt,
                                    0.0, 0.0);
    pipeline.process(observations, imu_samples);
  }

  ASSERT_TRUE(pipeline.initialized());
  ASSERT_GT(pipeline.numKeyframes(), 0);
  ASSERT_GT(pipeline.numLandmarks(), 0u);

  pipeline.clear();

  EXPECT_FALSE(pipeline.initialized());
  EXPECT_EQ(pipeline.numKeyframes(), 0);
  EXPECT_EQ(pipeline.numImuEdges(), 0);
  EXPECT_TRUE(
      pipeline.latestOptimizedPoseMatrix().isApprox(Eigen::Matrix4d::Identity()));
  EXPECT_EQ(pipeline.numLandmarks(), landmarks.size());

  pipeline.clearLandmarks();
  EXPECT_EQ(pipeline.numLandmarks(), 0u);
}
