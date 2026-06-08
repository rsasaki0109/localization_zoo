#include "kinematic_icp/kinematic_icp.h"
#include <gtest/gtest.h>
#include <cmath>

using namespace localization_zoo::kinematic_icp;

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

WheelOdom deltaFromPoses(double x0, double y0, double yaw0, double x1, double y1, double yaw1) {
  const double dx = x1 - x0;
  const double dy = y1 - y0;
  const double c = std::cos(yaw0);
  const double s = std::sin(yaw0);
  WheelOdom odom;
  odom.forward_m = c * dx + s * dy;
  odom.yaw_rad = yaw1 - yaw0;
  return odom;
}

}  // namespace

TEST(KinematicICP, FirstScanInitializes) {
  KinematicICPEstimator est(KinematicICPParams{});
  const auto res = est.registerScan(makeBoxScan(0, 0, 0));
  EXPECT_TRUE(res.valid);
}

TEST(KinematicICP, PureTranslationWithWheelOdom) {
  KinematicICPParams params;
  params.max_correspondence_distance = 1.0;
  KinematicICPEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0));
  for (int i = 1; i <= 5; ++i) {
    WheelOdom odom{0.2, 0.0};
    const auto res = est.registerScan(makeBoxScan(i * 0.2, 0, 0), odom);
    EXPECT_TRUE(res.valid);
  }
  const double err = (est.pose().block<2, 1>(0, 2) - Eigen::Vector2d(1.0, 0)).norm();
  EXPECT_LT(err, 0.2);
}

TEST(KinematicICP, PureRotationWithWheelOdom) {
  KinematicICPParams params;
  params.max_correspondence_distance = 1.0;
  KinematicICPEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0));
  const double yaw_step = 0.03;
  for (int i = 1; i <= 5; ++i) {
    WheelOdom odom{0.0, yaw_step};
    const auto res = est.registerScan(makeBoxScan(0, 0, i * yaw_step), odom);
    EXPECT_TRUE(res.valid);
  }
  const double est_yaw = std::atan2(est.pose()(1, 0), est.pose()(0, 0));
  EXPECT_NEAR(est_yaw, 5 * yaw_step, 0.08);
}

TEST(KinematicICP, CoupledMotionWithWheelOdom) {
  KinematicICPParams params;
  params.max_correspondence_distance = 1.0;
  KinematicICPEstimator est(params);
  est.registerScan(makeBoxScan(0, 0, 0));
  double x = 0.0;
  double y = 0.0;
  double yaw = 0.0;
  for (int i = 1; i <= 15; ++i) {
    const double nx = 0.2 * i;
    const double ny = 0.0;
    const double nyaw = 0.02 * i;
    const WheelOdom odom = deltaFromPoses(x, y, yaw, nx, ny, nyaw);
    const auto res = est.registerScan(makeBoxScan(nx, ny, nyaw), odom);
    EXPECT_TRUE(res.valid);
    x = nx;
    y = ny;
    yaw = nyaw;
  }
  const double err = (est.pose().block<2, 1>(0, 2) - Eigen::Vector2d(3.0, 0)).norm();
  EXPECT_LT(err, 0.6);
}

TEST(KinematicICP, Pose2DUtility) {
  const auto T = pose2D(1.0, 2.0, M_PI / 4);
  EXPECT_NEAR(T(0, 2), 1.0, 1e-9);
  EXPECT_NEAR(T(1, 2), 2.0, 1e-9);
}
