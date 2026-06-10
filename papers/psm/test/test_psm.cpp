#include "psm/psm.h"
#include <gtest/gtest.h>
#include <cmath>

using namespace localization_zoo::psm;

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

TEST(PSM, FirstScanInitializes) {
  PSMEstimator est(PSMParams{});
  const auto res = est.registerScan(makeBoxScan(0, 0, 0));
  EXPECT_TRUE(res.valid);
}

TEST(PSM, PureTranslationInBox) {
  PSMParams params;
  params.search_xy_range = 0.5;
  params.search_yaw_range = 0.1;
  PSMEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0));
  for (int i = 1; i <= 5; ++i) {
    const auto res = est.registerScan(makeBoxScan(i * 0.2, 0, 0));
    EXPECT_TRUE(res.valid);
  }
  const double err = (est.pose().block<2, 1>(0, 2) - Eigen::Vector2d(1.0, 0)).norm();
  EXPECT_LT(err, 0.15);
}

TEST(PSM, PureRotationInBox) {
  PSMParams params;
  params.search_xy_range = 0.3;
  params.search_yaw_range = 0.2;
  PSMEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0));
  const double yaw_step = 0.03;
  for (int i = 1; i <= 5; ++i) {
    const auto res = est.registerScan(makeBoxScan(0, 0, i * yaw_step));
    EXPECT_TRUE(res.valid);
  }
  const double est_yaw = std::atan2(est.pose()(1, 0), est.pose()(0, 0));
  EXPECT_NEAR(est_yaw, 5 * yaw_step, 0.05);
}

TEST(PSM, CoupledTranslationAndRotation) {
  PSMParams params;
  params.search_xy_range = 0.6;
  params.search_yaw_range = 0.2;
  PSMEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0));
  for (int i = 1; i <= 20; ++i) {
    const auto res = est.registerScan(makeBoxScan(0.2 * i, 0, 0.02 * i));
    EXPECT_TRUE(res.valid);
  }
  const double err = (est.pose().block<2, 1>(0, 2) - Eigen::Vector2d(4.0, 0)).norm();
  EXPECT_LT(err, 0.5);
}

TEST(PSM, Pose2DUtility) {
  const auto T = pose2D(1.0, 2.0, M_PI / 4);
  EXPECT_NEAR(T(0, 2), 1.0, 1e-9);
  EXPECT_NEAR(T(1, 2), 2.0, 1e-9);
}

TEST(PSM, RobotFrameLocalMapTracksTranslation) {
  PSMParams params;
  params.search_xy_range = 0.5;
  params.use_local_map = true;
  params.local_map_radius = 20.0;
  params.local_map_voxel_size = 0.15;
  PSMEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0));
  for (int i = 1; i <= 10; ++i) {
    const auto res = est.registerScan(makeBoxScan(i * 0.2, 0, 0));
    EXPECT_TRUE(res.valid);
  }
  const double err = (est.pose().block<2, 1>(0, 2) - Eigen::Vector2d(2.0, 0)).norm();
  EXPECT_LT(err, 0.2);
  EXPECT_GT(est.mapSize(), static_cast<size_t>(100));
}

TEST(PSM, LocalMapImprovesLongTranslationVsScanToScan) {
  PSMParams scan_params;
  scan_params.search_xy_range = 0.5;
  PSMEstimator scan_est(scan_params);
  scan_est.registerScan(makeBoxScan(0, 0, 0));
  for (int i = 1; i <= 15; ++i) {
    scan_est.registerScan(makeBoxScan(i * 0.2, 0, 0));
  }
  const double scan_err =
      (scan_est.pose().block<2, 1>(0, 2) - Eigen::Vector2d(3.0, 0)).norm();

  PSMParams map_params;
  map_params.search_xy_range = 0.5;
  map_params.use_local_map = true;
  map_params.local_map_radius = 20.0;
  map_params.local_map_voxel_size = 0.15;
  PSMEstimator map_est(map_params);
  map_est.registerScan(makeBoxScan(0, 0, 0));
  for (int i = 1; i <= 15; ++i) {
    map_est.registerScan(makeBoxScan(i * 0.2, 0, 0));
  }
  const double map_err =
      (map_est.pose().block<2, 1>(0, 2) - Eigen::Vector2d(3.0, 0)).norm();

  EXPECT_LT(map_err, scan_err);
  EXPECT_LT(map_err, 0.35);
}
