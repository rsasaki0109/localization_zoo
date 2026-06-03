#include "adaptive_icp/adaptive_icp.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::adaptive_icp;

namespace {
// 地面 + 2 枚の壁からなる合成シーン (平面が支配的)。
std::vector<Eigen::Vector3d> makeScene(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-30, 30);
  std::uniform_real_distribution<double> z(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 3000; i++) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 1000; i++) pts.emplace_back(xy(rng), 15 + n(rng), z(rng));
  for (int i = 0; i < 1000; i++) pts.emplace_back(-15 + n(rng), xy(rng), z(rng));
  return pts;
}
}  // namespace

TEST(AdaptiveICP, FirstFrameEmptyInitializesMap) {
  AdaptiveICPPipeline pipeline(AdaptiveICPParams{});
  const auto result = pipeline.registerFrame({});
  EXPECT_TRUE(result.converged);
  EXPECT_TRUE(pipeline.pose().isApprox(result.pose));
}

TEST(AdaptiveICP, VoxelHashMapEstimatesPlanarNormals) {
  VoxelHashMap map(1.0);
  std::mt19937 rng(42);
  std::vector<Eigen::Vector3d> ground;
  std::uniform_real_distribution<double> xy(-10, 10);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 5000; i++) ground.emplace_back(xy(rng), xy(rng), n(rng));
  map.addPoints(ground);
  EXPECT_GT(map.size(), 0u);

  std::vector<Eigen::Vector3d> query = {Eigen::Vector3d(0, 0, 0.0)};
  const auto corr = map.getCorrespondences(query, 2.0, 5);
  ASSERT_TRUE(corr[0].found);
  ASSERT_TRUE(corr[0].has_normal);
  EXPECT_GT(corr[0].planarity, 0.7);
  EXPECT_GT(std::abs(corr[0].normal.z()), 0.9);
}

TEST(AdaptiveICP, MultiFrameTracksTranslation) {
  std::mt19937 rng(42);
  AdaptiveICPParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;
  AdaptiveICPPipeline pipeline(params);

  auto scene = makeScene(rng);
  for (int f = 0; f < 4; f++) {
    const Eigen::Matrix3d R =
        Eigen::AngleAxisd(f * 0.02, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    const Eigen::Vector3d t(f * 1.0, 0, 1);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = R.transpose() * (p - t);
      if (ps.norm() > 3 && ps.norm() < 30) scan.push_back(ps);
    }
    const auto result = pipeline.registerFrame(scan);
    EXPECT_TRUE(result.converged);
  }
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(3, 0, 0)).norm();
  EXPECT_LT(err, 2.0);
}

TEST(AdaptiveICP, ReliableInitialPoseRejectsBadCoarseSolution) {
  // tau を 0 にすると粗解は常に棄却され、動き予測が初期姿勢になる。
  AdaptiveICPParams params;
  params.voxel_size = 1.0;
  params.reliable_translation_tau = 0.0;
  AdaptiveICPPipeline pipeline(params);

  std::mt19937 rng(7);
  auto scene = makeScene(rng);
  for (int f = 0; f < 3; f++) {
    const Eigen::Vector3d t(f * 0.5, 0, 1);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 3 && ps.norm() < 30) scan.push_back(ps);
    }
    const auto result = pipeline.registerFrame(scan);
    EXPECT_TRUE(result.converged);
    if (f > 0) EXPECT_FALSE(result.used_coarse_pose);
  }
}
