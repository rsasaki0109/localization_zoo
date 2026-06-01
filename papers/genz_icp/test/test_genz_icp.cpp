#include "genz_icp/genz_icp.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::genz_icp;

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

TEST(GenZICP, FirstFrameEmptyInitializesMap) {
  GenZICPPipeline pipeline(GenZICPParams{});
  const auto result = pipeline.registerFrame({});
  EXPECT_TRUE(result.converged);
  EXPECT_TRUE(pipeline.pose().isApprox(result.pose));
}

TEST(GenZICP, VoxelHashMapEstimatesPlanarNormals) {
  VoxelHashMap map(1.0);
  std::mt19937 rng(42);
  // z≈0 の地面のみ -> 法線は ±z、planarity は高いはず。
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
  EXPECT_GT(std::abs(corr[0].normal.z()), 0.9);  // 法線はほぼ z 軸
}

TEST(GenZICP, MultiFrameTracksTranslation) {
  std::mt19937 rng(42);
  GenZICPParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;
  GenZICPPipeline pipeline(params);

  auto scene = makeScene(rng);
  for (int f = 0; f < 3; f++) {
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
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(2, 0, 0)).norm();
  EXPECT_LT(err, 2.0);
}
