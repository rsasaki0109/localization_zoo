#include "kiss_icp/kiss_icp.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::kiss_icp;

namespace {
std::vector<Eigen::Vector3d> makeScene(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-30, 30);
  std::uniform_real_distribution<double> z(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 3000; i++) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 1000; i++) pts.emplace_back(xy(rng), 15+n(rng), z(rng));
  for (int i = 0; i < 1000; i++) pts.emplace_back(-15+n(rng), xy(rng), z(rng));
  return pts;
}
}

TEST(KISSICP, FirstFrameEmptyInitializesMap) {
  KISSICPPipeline pipeline(KISSICPParams{});
  const auto result = pipeline.registerFrame({});
  EXPECT_TRUE(result.converged);
  EXPECT_TRUE(pipeline.pose().isApprox(result.pose));
}

TEST(KISSICP, VoxelHashMap) {
  VoxelHashMap map(1.0);
  std::mt19937 rng(42);
  auto pts = makeScene(rng);
  map.addPoints(pts);
  EXPECT_GT(map.size(), 0u);
  EXPECT_LT(map.size(), pts.size());
}

TEST(KISSICP, FullVoxelUpdatePolicyIsOptIn) {
  const std::vector<Eigen::Vector3d> old_point = {
      Eigen::Vector3d(0.1, 0.1, 0.1)};
  const std::vector<Eigen::Vector3d> new_point = {
      Eigen::Vector3d(0.8, 0.1, 0.1)};

  VoxelHashMap frozen(1.0, 1, false);
  frozen.addPoints(old_point);
  frozen.addPoints(new_point);
  EXPECT_FALSE(frozen.getCorrespondences(new_point, 0.1).front().found);

  VoxelHashMap updating(1.0, 1, true);
  updating.addPoints(old_point);
  updating.addPoints(new_point);
  EXPECT_TRUE(updating.getCorrespondences(new_point, 0.1).front().found);
}

TEST(KISSICP, ModelDeviationErrorIncludesTranslationAndRangeScaledRotation) {
  Eigen::Matrix4d deviation = Eigen::Matrix4d::Identity();
  deviation.block<3, 1>(0, 3) = Eigen::Vector3d(3.0, 4.0, 0.0);
  deviation.block<3, 3>(0, 0) =
      Eigen::AngleAxisd(0.2, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  const double expected = 5.0 + 2.0 * 100.0 * std::sin(0.1);
  EXPECT_NEAR(modelDeviationError(deviation, 100.0), expected, 1e-12);
}

TEST(KISSICP, ModelDeviationThresholdStartsAtConfiguredSigma) {
  KISSICPParams params;
  params.initial_threshold = 1.25;
  params.use_model_deviation_threshold = true;
  KISSICPPipeline pipeline(params);
  EXPECT_DOUBLE_EQ(pipeline.adaptiveThreshold(), 1.25);
  pipeline.registerFrame({});
  pipeline.registerFrame({});
  EXPECT_DOUBLE_EQ(pipeline.adaptiveThreshold(), 1.25);
}

TEST(KISSICP, ElevationCorrectionPreservesRangeAndRaisesPoint) {
  std::vector<Eigen::Vector3d> points = {
      Eigen::Vector3d(10.0, 0.0, 0.0),
      Eigen::Vector3d(0.0, 5.0, 5.0)};
  const std::vector<double> original_ranges = {
      points[0].norm(), points[1].norm()};
  correctElevationAngle(points, 0.1);
  EXPECT_NEAR(points[0].x(), 10.0 * std::cos(0.1), 1e-12);
  EXPECT_NEAR(points[0].z(), 10.0 * std::sin(0.1), 1e-12);
  EXPECT_GT(points[1].z(), 5.0);
  EXPECT_NEAR(points[0].norm(), original_ranges[0], 1e-12);
  EXPECT_NEAR(points[1].norm(), original_ranges[1], 1e-12);
}

TEST(KISSICP, DeskewScanToEndUsesConstantVelocitySE3) {
  const std::vector<Eigen::Vector3d> points = {
      Eigen::Vector3d(5.0, 0.0, 0.0),
      Eigen::Vector3d(5.0, 0.0, 0.0),
      Eigen::Vector3d(5.0, 0.0, 0.0)};
  const std::vector<double> times = {0.0, 0.5, 1.0};
  Eigen::Matrix4d relative_motion = Eigen::Matrix4d::Identity();
  relative_motion.block<3, 1>(0, 3) = Eigen::Vector3d(1.0, 0.0, 0.0);

  const auto deskewed =
      deskewScanToEnd(points, times, relative_motion);
  EXPECT_NEAR(deskewed[0].x(), 4.0, 1e-12);
  EXPECT_NEAR(deskewed[1].x(), 4.5, 1e-12);
  EXPECT_NEAR(deskewed[2].x(), 5.0, 1e-12);
}

TEST(KISSICP, DeskewScanToEndRejectsMismatchedTimestamps) {
  const std::vector<Eigen::Vector3d> points = {
      Eigen::Vector3d(1.0, 2.0, 3.0)};
  const auto deskewed =
      deskewScanToEnd(points, {}, Eigen::Matrix4d::Identity());
  ASSERT_EQ(deskewed.size(), 1u);
  EXPECT_TRUE(deskewed.front().isApprox(points.front()));
}

TEST(KISSICP, MotionGuardChecksTranslationAndRotation) {
  Eigen::Matrix4d motion = Eigen::Matrix4d::Identity();
  motion.block<3, 1>(0, 3) = Eigen::Vector3d(1.5, 0.0, 0.0);
  motion.block<3, 3>(0, 0) =
      Eigen::AngleAxisd(0.2, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  EXPECT_TRUE(motionWithinLimits(motion, 2.0, 0.3));
  EXPECT_FALSE(motionWithinLimits(motion, 1.0, 0.3));
  EXPECT_FALSE(motionWithinLimits(motion, 2.0, 0.1));
}

TEST(KISSICP, AdaptiveMotionGuardAcceptsConsistentHighSpeedTranslation) {
  Eigen::Matrix4d previous = Eigen::Matrix4d::Identity();
  previous(0, 3) = 1.9;
  Eigen::Matrix4d candidate = Eigen::Matrix4d::Identity();
  candidate(0, 3) = 2.15;
  EXPECT_FALSE(motionWithinLimits(candidate, 2.0, 0.3));
  EXPECT_TRUE(motionWithinAdaptiveLimits(candidate, previous, 2.0, 0.3,
                                         2.0, 0.5, 0.1));
}

TEST(KISSICP, AdaptiveMotionGuardRejectsInconsistentJumpAndRotation) {
  Eigen::Matrix4d previous = Eigen::Matrix4d::Identity();
  previous(0, 3) = 1.9;
  Eigen::Matrix4d jump = Eigen::Matrix4d::Identity();
  jump(0, 3) = 3.0;
  EXPECT_FALSE(motionWithinAdaptiveLimits(jump, previous, 2.0, 0.3,
                                          2.0, 0.5, 0.1));

  Eigen::Matrix4d rotation = previous;
  rotation.block<3, 3>(0, 0) =
      Eigen::AngleAxisd(0.4, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  EXPECT_FALSE(motionWithinAdaptiveLimits(rotation, previous, 2.0, 0.3,
                                          2.0, 0.5, 0.1));
}

TEST(KISSICP, PairMatcher) {
  std::mt19937 rng(42);
  auto scene = makeScene(rng);

  const Eigen::Matrix3d R =
      Eigen::AngleAxisd(0.03, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  const Eigen::Vector3d t(0.8, -0.2, 0.0);

  std::vector<Eigen::Vector3d> target;
  std::vector<Eigen::Vector3d> source;
  for (const auto& p : scene) {
    if (p.norm() > 3 && p.norm() < 30) target.push_back(p);
    const Eigen::Vector3d ps = R.transpose() * (p - t);
    if (ps.norm() > 3 && ps.norm() < 30) source.push_back(ps);
  }

  KISSMatcherParams params;
  params.target_voxel_size = 0.5;
  params.source_voxel_size = 0.5;
  params.max_correspondence_distance = 1.5;
  params.max_icp_iterations = 40;
  params.min_correspondences = 50;
  KISSMatcher matcher(params);
  matcher.setTarget(target);

  const auto result = matcher.align(source);
  EXPECT_TRUE(result.converged);
  EXPECT_GT(result.num_correspondences, 50);
  EXPECT_LT((result.transform.block<3, 1>(0, 3) - t).norm(), 0.15);
  EXPECT_LT((result.transform.block<3, 3>(0, 0) - R).norm(), 0.15);
}

TEST(KISSICP, MultiFrame) {
  std::mt19937 rng(42);
  KISSICPParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;  // テスト用に制限
  KISSICPPipeline pipeline(params);

  auto scene = makeScene(rng);
  for (int f = 0; f < 3; f++) {
    // センサ座標に変換
    Eigen::Matrix3d R = Eigen::AngleAxisd(f * 0.02, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    Eigen::Vector3d t(f * 1.0, 0, 1);
    std::vector<Eigen::Vector3d> scan;
    for (auto& p : scene) {
      Eigen::Vector3d ps = R.transpose() * (p - t);
      if (ps.norm() > 3 && ps.norm() < 30) scan.push_back(ps);
    }
    auto result = pipeline.registerFrame(scan);
    EXPECT_TRUE(result.converged);
  }
  // The pipeline reports odometry relative to the first frame, whose
  // synthetic sensor height is treated as the origin here.
  double err =
      (pipeline.pose().block<3,1>(0,3) - Eigen::Vector3d(2,0,0)).norm();
  EXPECT_LT(err, 2.0);
}
