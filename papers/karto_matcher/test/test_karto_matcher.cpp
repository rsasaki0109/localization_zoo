#include "karto_matcher/karto_matcher.h"
#include <gtest/gtest.h>
#include <cmath>

using namespace localization_zoo::karto_matcher;

namespace {

LaserScan makeBoxScan(double x, double y, double yaw, int beams = 180) {
  LaserScan scan;
  scan.angle_min = -M_PI;
  scan.angle_max = M_PI;
  scan.angle_increment = (scan.angle_max - scan.angle_min) / (beams - 1);
  scan.range_min = 0.1;
  scan.range_max = 30.0;
  scan.ranges.resize(static_cast<size_t>(beams));
  const double c = std::cos(yaw);
  const double s = std::sin(yaw);
  for (int i = 0; i < beams; ++i) {
    const double a = scan.angle_min + i * scan.angle_increment;
    const double dx = std::cos(a);
    const double dy = std::sin(a);
    const double wx = c * dx - s * dy;
    const double wy = s * dx + c * dy;
    double t = 30.0;
    if (wx > 1e-6) t = std::min(t, (10.0 - x) / wx);
    if (wx < -1e-6) t = std::min(t, (-10.0 - x) / wx);
    if (wy > 1e-6) t = std::min(t, (10.0 - y) / wy);
    if (wy < -1e-6) t = std::min(t, (-10.0 - y) / wy);
    scan.ranges[static_cast<size_t>(i)] = std::max(0.1, t);
  }
  return scan;
}

}  // namespace

TEST(KartoMatcher, FirstScanInitializes) {
  KartoMatcherEstimator est(KartoMatcherParams{});
  const auto res = est.registerScan(makeBoxScan(0, 0, 0));
  EXPECT_TRUE(res.valid);
  EXPECT_GT(est.mapSize(), 20u);
}

TEST(KartoMatcher, PureTranslationInBox) {
  KartoMatcherParams params;
  params.search_xy_range = 0.5;
  KartoMatcherEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0));
  for (int i = 1; i <= 5; ++i) {
    const auto res = est.registerScan(makeBoxScan(i * 0.2, 0, 0));
    EXPECT_TRUE(res.valid);
  }
  const double err = (est.pose().block<2, 1>(0, 2) - Eigen::Vector2d(1.0, 0)).norm();
  EXPECT_LT(err, 0.35);
}

TEST(KartoMatcher, PureRotationInBox) {
  KartoMatcherParams params;
  params.search_yaw_range = 0.2;
  KartoMatcherEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0));
  const double yaw_step = 0.03;
  for (int i = 1; i <= 5; ++i) {
    const auto res = est.registerScan(makeBoxScan(0, 0, i * yaw_step));
    EXPECT_TRUE(res.valid);
  }
  const double est_yaw = std::atan2(est.pose()(1, 0), est.pose()(0, 0));
  EXPECT_NEAR(est_yaw, 5 * yaw_step, 0.08);
}

TEST(KartoMatcher, CoupledTranslationAndRotation) {
  KartoMatcherParams params;
  params.search_xy_range = 0.5;
  params.search_yaw_range = 0.2;
  KartoMatcherEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0));
  for (int i = 1; i <= 15; ++i) {
    const auto res = est.registerScan(makeBoxScan(0.2 * i, 0, 0.02 * i));
    EXPECT_TRUE(res.valid);
  }
  const double err = (est.pose().block<2, 1>(0, 2) - Eigen::Vector2d(3.0, 0)).norm();
  EXPECT_LT(err, 0.85);
}

TEST(KartoMatcher, LocalMapAccumulatesFeatures) {
  KartoMatcherParams params;
  params.local_map_radius = 20.0;
  KartoMatcherEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0));
  const size_t after_first = est.mapSize();
  for (int i = 1; i <= 10; ++i) {
    est.registerScan(makeBoxScan(0.3 * i, 0, 0));
  }
  EXPECT_GT(est.mapSize(), after_first);
  EXPECT_LT(est.mapSize(), after_first * 15u);
}

TEST(KartoMatcher, Pose2DUtility) {
  const auto T = pose2D(1.0, 2.0, M_PI / 4);
  EXPECT_NEAR(T(0, 2), 1.0, 1e-9);
  EXPECT_NEAR(T(1, 2), 2.0, 1e-9);
}

TEST(KartoMatcher, RobotFrameLocalMapTracksSlowMotion) {
  KartoMatcherParams params;
  params.search_xy_range = 0.5;
  params.search_yaw_range = 0.2;
  params.local_map_radius = 20.0;
  params.local_map_voxel_size = 0.15;
  KartoMatcherEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0));
  for (int i = 1; i <= 20; ++i) {
    const double x = 0.08 * i;
    const double yaw = 0.015 * i;
    const auto res = est.registerScan(makeBoxScan(x, 0, yaw));
    EXPECT_TRUE(res.valid);
  }
  const double err = (est.pose().block<2, 1>(0, 2) - Eigen::Vector2d(1.6, 0)).norm();
  EXPECT_LT(err, 0.5);
}

TEST(KartoMatcher, BranchAndBoundMatchesBruteForceOnTranslation) {
  KartoMatcherParams bnb_params;
  bnb_params.search_xy_range = 0.4;
  bnb_params.use_branch_and_bound = true;
  KartoMatcherParams brute_params = bnb_params;
  brute_params.use_branch_and_bound = false;

  KartoMatcherEstimator bnb(bnb_params);
  KartoMatcherEstimator brute(brute_params);
  bnb.registerScan(makeBoxScan(0, 0, 0));
  brute.registerScan(makeBoxScan(0, 0, 0));
  for (int i = 1; i <= 3; ++i) {
    const auto scan = makeBoxScan(0.25 * i, 0, 0);
    const auto bnb_res = bnb.registerScan(scan);
    const auto brute_res = brute.registerScan(scan);
    EXPECT_TRUE(bnb_res.valid);
    EXPECT_TRUE(brute_res.valid);
  }
  const double err =
      (bnb.pose().block<2, 1>(0, 2) - brute.pose().block<2, 1>(0, 2)).norm();
  EXPECT_LT(err, 0.4);
}
