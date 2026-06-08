#include "rf2o/rf2o.h"
#include <gtest/gtest.h>
#include <cmath>

using namespace localization_zoo::rf2o;

namespace {

LaserScan makeBoxScan(double x, double y, double yaw, int beams = 360) {
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

TEST(RF2O, FirstScanInitializes) {
  RF2OEstimator est(RF2OParams{});
  const auto scan = makeBoxScan(0, 0, 0);
  const auto res = est.registerScan(scan, 0.1);
  EXPECT_TRUE(res.valid);
}

TEST(RF2O, PureTranslationInBox) {
  RF2OParams params;
  params.coarse_to_fine_levels = 3;
  RF2OEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0), 0.1);
  const double dt = 0.1;
  for (int i = 1; i <= 5; ++i) {
    const auto res = est.registerScan(makeBoxScan(i * 0.2, 0, 0), dt);
    EXPECT_TRUE(res.valid);
  }
  const double err = (est.pose().block<2, 1>(0, 2) - Eigen::Vector2d(1.0, 0)).norm();
  EXPECT_LT(err, 0.5);
}

TEST(RF2O, PureRotationInBox) {
  RF2OParams params;
  params.coarse_to_fine_levels = 3;
  RF2OEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0), 0.1);
  const double dt = 0.1;
  const double yaw_step = 0.03;
  for (int i = 1; i <= 5; ++i) {
    const auto res = est.registerScan(makeBoxScan(0, 0, i * yaw_step), dt);
    EXPECT_TRUE(res.valid);
  }
  const double est_yaw = std::atan2(est.pose()(1, 0), est.pose()(0, 0));
  EXPECT_NEAR(est_yaw, 5 * yaw_step, 0.15);
}

TEST(RF2O, CoupledTranslationAndRotation) {
  RF2OParams params;
  params.coarse_to_fine_levels = 5;
  RF2OEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0), 0.1);
  const double dt = 0.1;
  for (int i = 1; i <= 20; ++i) {
    const auto res = est.registerScan(makeBoxScan(0.2 * i, 0, 0.02 * i), dt);
    EXPECT_TRUE(res.valid);
  }
  const double err = (est.pose().block<2, 1>(0, 2) - Eigen::Vector2d(4.0, 0)).norm();
  EXPECT_LT(err, 1.0);
}

TEST(RF2O, Pose2DUtility) {
  const auto T = pose2D(1.0, 2.0, M_PI / 4);
  EXPECT_NEAR(T(0, 2), 1.0, 1e-9);
  EXPECT_NEAR(T(1, 2), 2.0, 1e-9);
}
