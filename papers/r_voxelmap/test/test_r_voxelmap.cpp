#include "r_voxelmap/r_voxelmap.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::r_voxelmap;

namespace {
// 密な室内様シーン (平面ベースのマッピングが成立する点密度)。
std::vector<Eigen::Vector3d> makeScene(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-12, 12);
  std::uniform_real_distribution<double> z(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 12000; i++) pts.emplace_back(xy(rng), xy(rng), n(rng));    // 地面
  for (int i = 0; i < 6000; i++) pts.emplace_back(xy(rng), 10 + n(rng), z(rng)); // 壁 y=10
  for (int i = 0; i < 6000; i++) pts.emplace_back(-10 + n(rng), xy(rng), z(rng));// 壁 x=-10
  for (int i = 0; i < 6000; i++) pts.emplace_back(xy(rng), -10 + n(rng), z(rng));// 壁 y=-10
  return pts;
}
}  // namespace

TEST(RVoxelMap, FirstFrameInitializesMap) {
  RVoxelMapPipeline pipeline(RVoxelMapParams{});
  const auto result = pipeline.registerFrame({});
  EXPECT_TRUE(result.converged);
  EXPECT_TRUE(pipeline.pose().isApprox(result.pose));
}

TEST(RVoxelMap, RecursivePlaneFitDetectsPlane) {
  RecursiveVoxelMap map(2.0, 6, 0.1, 0.1, 2);
  std::mt19937 rng(42);
  std::vector<Eigen::Vector3d> ground;
  std::uniform_real_distribution<double> xy(-0.9, 0.9);
  std::normal_distribution<double> n(0, 0.005);
  for (int i = 0; i < 300; i++) ground.emplace_back(xy(rng), xy(rng), n(rng));
  map.addPoints(ground);
  const auto pl = map.query(Eigen::Vector3d(0, 0, 0));
  ASSERT_TRUE(pl.valid);
  EXPECT_GT(std::abs(pl.normal.z()), 0.9);
  EXPECT_LT(pl.plane_variance, 0.01);
}

TEST(RVoxelMap, RecursionRecoversPlaneFromMixedVoxel) {
  // 1 ルートボクセル内に 2 つの直交平面 (地面 + 壁)。ルートでは平面が取れず、
  // 再帰でそれぞれの子レベルに有効平面が現れるはず。
  RecursiveVoxelMap map(4.0, 6, 0.08, 0.08, 2);
  std::mt19937 rng(1);
  std::uniform_real_distribution<double> a(-1.9, 1.9);
  std::normal_distribution<double> n(0, 0.005);
  std::vector<Eigen::Vector3d> pts;
  for (int i = 0; i < 400; i++) pts.emplace_back(a(rng), a(rng), -2 + n(rng));  // 地面 z=-2
  for (int i = 0; i < 400; i++) pts.emplace_back(-2 + n(rng), a(rng), a(rng));  // 壁 x=-2
  map.addPoints(pts);
  const auto pl_ground = map.query(Eigen::Vector3d(1.0, 1.0, -1.95));
  const auto pl_wall = map.query(Eigen::Vector3d(-1.95, 1.0, 1.0));
  ASSERT_TRUE(pl_ground.valid);
  ASSERT_TRUE(pl_wall.valid);
  EXPECT_GT(std::abs(pl_ground.normal.z()), 0.85);
  EXPECT_GT(std::abs(pl_wall.normal.x()), 0.85);
}

TEST(RVoxelMap, MultiFrameTracksTranslation) {
  std::mt19937 rng(42);
  RVoxelMapParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 40;
  RVoxelMapPipeline pipeline(params);

  auto scene = makeScene(rng);
  for (int f = 0; f < 4; f++) {
    const Eigen::Vector3d t(f * 0.5, 0, 0);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 1 && ps.norm() < 25) scan.push_back(ps);
    }
    const auto result = pipeline.registerFrame(scan);
    EXPECT_TRUE(result.converged);
  }
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(1.5, 0, 0)).norm();
  EXPECT_LT(err, 1.0);
}

TEST(RVoxelMap, ScanToScanFallbackTracksWhenMapMatchIsRejected) {
  std::mt19937 rng(7);
  RVoxelMapParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 20;
  params.min_map_matched_ratio = 1.1;  // force the recovery path
  params.enable_scan_to_scan_fallback = true;
  params.min_fallback_matched_ratio = 0.05;
  RVoxelMapPipeline pipeline(params);

  const auto scene = makeScene(rng);
  for (int f = 0; f < 2; ++f) {
    const Eigen::Vector3d t(f * 0.5, 0, 0);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 1 && ps.norm() < 25) scan.push_back(ps);
    }
    const auto result = pipeline.registerFrame(scan);
    if (f == 1) {
      EXPECT_TRUE(result.converged);
      EXPECT_TRUE(result.used_fallback);
      EXPECT_GT(result.fallback_matched_ratio, 0.1);
    }
  }

  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(0.5, 0, 0)).norm();
  EXPECT_LT(err, 0.5);
}

TEST(RVoxelMap, ScanToScanFallbackValidatesMapDisagreement) {
  std::mt19937 rng(9);
  RVoxelMapParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 20;
  params.enable_scan_to_scan_fallback = true;
  params.min_fallback_matched_ratio = 0.05;
  params.max_map_fallback_translation_delta = 1e-9;
  params.max_map_fallback_rotation_delta = 1e-9;
  RVoxelMapPipeline pipeline(params);

  const auto scene = makeScene(rng);
  for (int f = 0; f < 2; ++f) {
    const Eigen::Vector3d t(f * 0.5, 0, 0);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 1 && ps.norm() < 25) scan.push_back(ps);
    }
    const auto result = pipeline.registerFrame(scan);
    if (f == 1) {
      EXPECT_TRUE(result.converged);
      EXPECT_TRUE(result.used_fallback);
      EXPECT_TRUE(result.fallback_disagreement);
      EXPECT_GT(result.fallback_matched_ratio, 0.1);
    }
  }
}
