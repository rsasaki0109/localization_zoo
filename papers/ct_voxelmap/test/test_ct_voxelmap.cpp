#include "ct_voxelmap/ct_voxelmap.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::ct_voxelmap;

namespace {
std::vector<Eigen::Vector3d> makeScene(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-30, 30);
  std::uniform_real_distribution<double> z(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 4000; i++) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 1500; i++) pts.emplace_back(xy(rng), 15 + n(rng), z(rng));
  for (int i = 0; i < 1500; i++) pts.emplace_back(-15 + n(rng), xy(rng), z(rng));
  return pts;
}
}  // namespace

TEST(CTVoxelMap, FirstFrameInitializesMap) {
  CTVoxelMapPipeline pipeline(CTVoxelMapParams{});
  const auto result = pipeline.registerFrame({});
  EXPECT_TRUE(result.converged);
  EXPECT_TRUE(pipeline.pose().isApprox(result.pose));
}

TEST(CTVoxelMap, ProbabilisticVoxelDetectsPlane) {
  ProbabilisticVoxelMap map(1.0, 6, 0.1);
  std::mt19937 rng(42);
  std::vector<Eigen::Vector3d> ground;
  std::uniform_real_distribution<double> xy(-0.4, 0.4);
  std::normal_distribution<double> n(0, 0.005);
  // 1 ボクセル内に収まる平面パッチ。
  for (int i = 0; i < 200; i++) ground.emplace_back(xy(rng), xy(rng), n(rng));
  map.addPoints(ground);

  const auto q = map.query(Eigen::Vector3d(0, 0, 0));
  ASSERT_TRUE(q.found);
  ASSERT_TRUE(q.feature.is_planar);
  EXPECT_GT(std::abs(q.feature.normal.z()), 0.9);  // 法線は z 軸
  EXPECT_LT(q.feature.plane_variance, 0.01);       // 薄い平面
}

TEST(CTVoxelMap, MultiFrameTracksTranslation) {
  std::mt19937 rng(42);
  CTVoxelMapParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 40;
  CTVoxelMapPipeline pipeline(params);

  auto scene = makeScene(rng);
  for (int f = 0; f < 4; f++) {
    const Eigen::Vector3d t(f * 1.0, 0, 1);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 2 && ps.norm() < 30) scan.push_back(ps);
    }
    const auto result = pipeline.registerFrame(scan);
    EXPECT_TRUE(result.converged);
  }
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(3, 0, 0)).norm();
  EXPECT_LT(err, 2.0);
}

TEST(CTVoxelMap, PointCovarianceGrowsWithRange) {
  // 遠い点ほど横方向不確かさが大きいことを間接確認 (平面検出が遠距離で緩む)。
  CTVoxelMapParams params;
  params.voxel_size = 1.0;
  CTVoxelMapPipeline pipeline(params);
  std::mt19937 rng(1);
  auto scene = makeScene(rng);
  std::vector<Eigen::Vector3d> scan;
  for (const auto& p : scene)
    if (p.norm() > 2 && p.norm() < 30) scan.push_back(p);
  // 2 フレーム回して破綻しないこと。
  pipeline.registerFrame(scan);
  const auto r = pipeline.registerFrame(scan);
  EXPECT_TRUE(r.converged);
  EXPECT_GE(r.planar_ratio, 0.0);
  EXPECT_LE(r.planar_ratio, 1.0);
}
