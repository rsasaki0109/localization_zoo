#include <gtest/gtest.h>

#include <cmath>
#include <random>

#include "l_lo/l_lo.h"

using namespace localization_zoo::l_lo;

namespace {

Polygon square(double cx, double cy, double half) {
  return {{cx - half, cy - half}, {cx + half, cy - half}, {cx + half, cy + half}, {cx - half, cy + half}};
}

}  // namespace

TEST(LLO, HullAreaAndIntersection) {
  std::vector<Eigen::Vector2d> pts = {{0, 0}, {2, 0}, {2, 2}, {0, 2}, {1, 1}, {0.5, 1.5}};
  const auto hull = convexHull(pts);
  EXPECT_EQ(hull.size(), 4u);
  EXPECT_NEAR(polygonArea(hull), 4.0, 1e-12);
  EXPECT_NEAR(polygonArea(convexIntersection(square(0, 0, 1), square(1, 1, 1))), 1.0, 1e-9);
  EXPECT_NEAR(polygonArea(convexIntersection(square(0, 0, 1), square(5, 5, 1))), 0.0, 1e-12);
}

TEST(LLO, TurningFunctionIsScaleInvariantHausdorffIsNot) {
  // Same shape, different size: equal turning-function area (shape), positive
  // Hausdorff distance (dimension), eqs. (5)-(9).
  const Polygon small = square(10, 0, 1), large = square(10, 0, 2);
  EXPECT_NEAR(turningFunctionArea(small), turningFunctionArea(large), 1e-9);
  EXPECT_NEAR(hausdorffDistance(small, large), std::sqrt(2.0), 1e-9);
  const Polygon triangle = {{9, -1}, {11, -1}, {10, 1}};
  EXPECT_GT(std::abs(turningFunctionArea(small) - turningFunctionArea(triangle)), 0.1);
  EXPECT_LT(hullSimilarity(small, small, 0.5, 0.5), 1e-12);
}

TEST(LLO, OverlapSearchRecoversPlanarMotion) {
  const Eigen::Vector3d truth(0.8, -0.3, 0.03);
  std::vector<Polygon> previous, current;
  const Polygon shapes[] = {square(8, 3, 1.0), square(-6, 7, 0.6), {{12, -5}, {15, -5}, {14, -2}},
                            square(3, -9, 1.5)};
  for (const auto& shape : shapes) {
    previous.push_back(shape);
    // Frame-k coordinates of the same landmark: inverse of the truth motion.
    Polygon in_k;
    for (const auto& v : shape) {
      const Eigen::Vector2d t = v - truth.head<2>();
      in_k.emplace_back(std::cos(truth.z()) * t.x() + std::sin(truth.z()) * t.y(),
                        -std::sin(truth.z()) * t.x() + std::cos(truth.z()) * t.y());
    }
    current.push_back(in_k);
  }
  LLOOdometry odom;
  int rounds = 0;
  double overlap = 0.0;
  const Eigen::Vector3d estimate =
      odom.estimateHorizontal(previous, current, Eigen::Vector3d(0.6, -0.2, 0.0), &rounds, &overlap);
  EXPECT_NEAR(estimate.x(), truth.x(), 0.02);
  EXPECT_NEAR(estimate.y(), truth.y(), 0.02);
  EXPECT_NEAR(estimate.z(), truth.z(), 0.004);
  EXPECT_GT(overlap, 0.98);
}

TEST(LLO, OdometryTracksForwardMotionInASyntheticStreet) {
  // Flat ground plus box-shaped landmarks (cars, poles, walls) at varying
  // positions; the sensor drives 1 m per frame along x and yaws slightly.
  std::mt19937 rng(3);
  std::uniform_real_distribution<double> u(0.0, 1.0);
  struct Box { Eigen::Vector3d c, half; };
  std::vector<Box> boxes;
  for (int i = 0; i < 40; ++i) {
    boxes.push_back({Eigen::Vector3d(-20 + 2.5 * i, (i % 2 ? 7.0 : -7.0) + 2 * u(rng), -0.9 + 0.5 * u(rng)),
                     Eigen::Vector3d(0.4 + 1.5 * u(rng), 0.4 + 1.0 * u(rng), 0.5 + 1.0 * u(rng))});
  }
  std::vector<Eigen::Vector3d> world;
  for (int i = 0; i < 20000; ++i) world.emplace_back(-40 + 120 * u(rng), -20 + 40 * u(rng), -1.73);
  for (const auto& b : boxes) {
    for (int i = 0; i < 1500; ++i) {
      Eigen::Vector3d p = b.c + Eigen::Vector3d((2 * u(rng) - 1) * b.half.x(), (2 * u(rng) - 1) * b.half.y(),
                                                (2 * u(rng) - 1) * b.half.z());
      const int face = i % 4;  // points on the vertical faces
      if (face == 0) p.x() = b.c.x() + b.half.x();
      if (face == 1) p.x() = b.c.x() - b.half.x();
      if (face == 2) p.y() = b.c.y() + b.half.y();
      if (face == 3) p.y() = b.c.y() - b.half.y();
      world.push_back(p);
    }
  }
  LLOParams params;
  LLOOdometry odom(params);
  Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
  for (int k = 0; k < 8; ++k) {
    if (k > 0) {
      Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
      step.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.01, Eigen::Vector3d::UnitZ()).toRotationMatrix();
      step(0, 3) = 1.0;
      truth = truth * step;
    }
    const Eigen::Matrix4d inv = truth.inverse();
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : world) scan.push_back(inv.block<3, 3>(0, 0) * p + inv.block<3, 1>(0, 3));
    const auto r = odom.registerFrame(scan);
    if (k > 0) EXPECT_GE(r.matches, 3) << "frame " << k;
  }
  EXPECT_LT((odom.pose().block<2, 1>(0, 3) - truth.block<2, 1>(0, 3)).norm(), 0.15);
}
