#include <gtest/gtest.h>

#include <cmath>
#include <random>

#include "l_lo/l_lo.h"

using namespace localization_zoo::l_lo;

namespace {

Polygon square(double cx, double cy, double half) {
  return {{cx - half, cy - half}, {cx + half, cy - half}, {cx + half, cy + half}, {cx - half, cy + half}};
}

// Flat ground (rising by `grade` along x) plus box-shaped landmarks (cars,
// poles, walls) at varying positions.
std::vector<Eigen::Vector3d> syntheticStreet(double grade) {
  std::mt19937 rng(3);
  std::uniform_real_distribution<double> u(0.0, 1.0);
  struct Box { Eigen::Vector3d c, half; };
  std::vector<Box> boxes;
  for (int i = 0; i < 40; ++i) {
    boxes.push_back({Eigen::Vector3d(-20 + 2.5 * i, (i % 2 ? 7.0 : -7.0) + 2 * u(rng), -0.9 + 0.5 * u(rng)),
                     Eigen::Vector3d(0.4 + 1.5 * u(rng), 0.4 + 1.0 * u(rng), 0.5 + 1.0 * u(rng))});
  }
  std::vector<Eigen::Vector3d> world;
  for (int i = 0; i < 20000; ++i) {
    const double x = -40 + 120 * u(rng);
    world.emplace_back(x, -20 + 40 * u(rng), -1.73 + grade * x);
  }
  for (const auto& b : boxes) {
    for (int i = 0; i < 1500; ++i) {
      Eigen::Vector3d p = b.c + Eigen::Vector3d((2 * u(rng) - 1) * b.half.x(), (2 * u(rng) - 1) * b.half.y(),
                                                (2 * u(rng) - 1) * b.half.z());
      const int face = i % 4;  // points on the vertical faces
      if (face == 0) p.x() = b.c.x() + b.half.x();
      if (face == 1) p.x() = b.c.x() - b.half.x();
      if (face == 2) p.y() = b.c.y() + b.half.y();
      if (face == 3) p.y() = b.c.y() - b.half.y();
      p.z() += grade * p.x();
      world.push_back(p);
    }
  }
  return world;
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
  // The sensor drives 1 m per frame along x and yaws slightly.
  const std::vector<Eigen::Vector3d> world = syntheticStreet(0.0);
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

TEST(LLO, SamePatchVerticalModeFollowsAGrade) {
  // A level sensor drives up a 3 % grade: 1 m along x and 3 cm up per frame.
  // The historical pitch mode has no height term; the same-patch mode recovers
  // the climb from the ground-plane offsets.
  const double grade = 0.03;
  const std::vector<Eigen::Vector3d> world = syntheticStreet(grade);
  for (int mode : {0, 1}) {
    LLOParams params;
    params.pitch_mode = mode;
    LLOOdometry odom(params);
    Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
    for (int k = 0; k < 8; ++k) {
      truth(0, 3) = k;
      truth(2, 3) = grade * k;
      const Eigen::Matrix4d inv = truth.inverse();
      std::vector<Eigen::Vector3d> scan;
      for (const auto& p : world) scan.push_back(inv.block<3, 3>(0, 0) * p + inv.block<3, 1>(0, 3));
      odom.registerFrame(scan);
    }
    if (mode == 0) {
      EXPECT_LT(std::abs(odom.pose()(2, 3)), 0.02);  // only pitch noise moves z
    } else {
      EXPECT_NEAR(odom.pose()(2, 3), truth(2, 3), 0.05);
      EXPECT_LT((odom.pose().block<2, 1>(0, 3) - truth.block<2, 1>(0, 3)).norm(), 0.25);
    }
  }
}

namespace {

// Ground at height(x) plus box landmarks standing on it.
std::vector<Eigen::Vector3d> streetWithHeight(double (*height)(double)) {
  std::mt19937 rng(5);
  std::uniform_real_distribution<double> u(0.0, 1.0);
  std::vector<Eigen::Vector3d> world;
  for (int i = 0; i < 30000; ++i) {
    const double x = -40 + 120 * u(rng);
    world.emplace_back(x, -20 + 40 * u(rng), -1.73 + height(x));
  }
  for (int b = 0; b < 40; ++b) {
    const Eigen::Vector3d c(-20 + 2.5 * b, (b % 2 ? 7.0 : -7.0) + 2 * u(rng), -0.9 + 0.5 * u(rng));
    const Eigen::Vector3d half(0.4 + 1.5 * u(rng), 0.4 + 1.0 * u(rng), 0.5 + 1.0 * u(rng));
    for (int i = 0; i < 1500; ++i) {
      Eigen::Vector3d p = c + Eigen::Vector3d((2 * u(rng) - 1) * half.x(), (2 * u(rng) - 1) * half.y(),
                                              (2 * u(rng) - 1) * half.z());
      const int face = i % 4;
      if (face == 0) p.x() = c.x() + half.x();
      if (face == 1) p.x() = c.x() - half.x();
      if (face == 2) p.y() = c.y() + half.y();
      if (face == 3) p.y() = c.y() - half.y();
      p.z() += height(c.x());
      world.push_back(p);
    }
  }
  return world;
}

std::vector<Eigen::Vector3d> scanAt(const std::vector<Eigen::Vector3d>& world, const Eigen::Matrix4d& pose) {
  const Eigen::Matrix4d inv = pose.inverse();
  std::vector<Eigen::Vector3d> scan;
  scan.reserve(world.size());
  for (const auto& p : world) scan.push_back(inv.block<3, 3>(0, 0) * p + inv.block<3, 1>(0, 3));
  return scan;
}

Eigen::Matrix4d bodyPose(double x, double z, double pitch, double roll) {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = (Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX())).toRotationMatrix();
  T(0, 3) = x;
  T(2, 3) = z;
  return T;
}

double flat(double) { return 0.0; }
double gradeAfter8(double x) { return x < 8.0 ? 0.0 : 0.04 * (x - 8.0); }

}  // namespace

TEST(LLO, GroundMapModeRecoversBodyRollAndPitchOnFlatGround) {
  // Flat road; from frame 3 the body rolls 0.02 rad and pitches -0.015 rad.
  // pitch_mode 3 recovers both from the ground-height map; the paper mode
  // assumes zero roll.
  const auto world = streetWithHeight(flat);
  for (int mode : {0, 3}) {
    LLOParams params;
    params.pitch_mode = mode;
    LLOOdometry odom(params);
    Eigen::Matrix4d truth;
    for (int k = 0; k < 10; ++k) {
      truth = bodyPose(k, 0.0, k >= 3 ? -0.015 : 0.0, k >= 3 ? 0.02 : 0.0);
      odom.registerFrame(scanAt(world, truth));
    }
    const Eigen::Matrix3d R = odom.pose().block<3, 3>(0, 0);
    const double roll = std::atan2(R(2, 1), R(2, 2));
    const double pitch = std::asin(-R(2, 0));
    if (mode == 3) {
      EXPECT_NEAR(roll, 0.02, 0.003);
      EXPECT_NEAR(pitch, -0.015, 0.003);
      EXPECT_LT(std::abs(odom.pose()(2, 3)), 0.03);
    } else {
      EXPECT_LT(std::abs(roll), 1e-6);  // the paper mode never estimates roll
    }
  }
}

TEST(LLO, GroundMapModeFollowsAGradeChange) {
  // Flat for x < 8 m, then a 4 % climb; the body pitches with the road.
  // Frame-to-frame pitch_mode 0 loses height; pitch_mode 3 tracks it.
  const auto world = streetWithHeight(gradeAfter8);
  const double grade_pitch = -std::atan(0.04);
  double final_z[2] = {0.0, 0.0};
  double final_pitch = 0.0;
  Eigen::Matrix4d truth;
  int slot = 0;
  for (int mode : {0, 3}) {
    LLOParams params;
    params.pitch_mode = mode;
    LLOOdometry odom(params);
    for (int k = 0; k < 20; ++k) {
      truth = bodyPose(k, gradeAfter8(k), k >= 8 ? grade_pitch : 0.0, 0.0);
      odom.registerFrame(scanAt(world, truth));
    }
    final_z[slot++] = odom.pose()(2, 3);
    if (mode == 3) final_pitch = std::asin(-odom.pose()(2, 0));
  }
  // Mode 0 recovers only part of the 0.44 m climb; mode 3 tracks it.
  const double error0 = std::abs(final_z[0] - truth(2, 3));
  const double error3 = std::abs(final_z[1] - truth(2, 3));
  EXPECT_GT(error0, 0.15);
  EXPECT_LT(error3, 0.05);
  EXPECT_LT(error3, 0.25 * error0);
  EXPECT_NEAR(final_pitch, grade_pitch, 0.005);
}

TEST(LLO, GroundMapModeLeavesHorizontalMotionUnchangedOnFlatGround) {
  // The height of a sensor point does not depend on yaw, so on flat ground
  // with a level body mode 3 reproduces the horizontal trajectory of mode 0.
  const auto world = streetWithHeight(flat);
  Eigen::Matrix4d poses[2];
  int slot = 0;
  for (int mode : {0, 3}) {
    LLOParams params;
    params.pitch_mode = mode;
    LLOOdometry odom(params);
    Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
    for (int k = 0; k < 8; ++k) {
      if (k > 0) {
        Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
        step.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.01, Eigen::Vector3d::UnitZ()).toRotationMatrix();
        step(0, 3) = 1.0;
        truth = truth * step;
      }
      odom.registerFrame(scanAt(world, truth));
    }
    poses[slot++] = odom.pose();
  }
  EXPECT_LT((poses[0].block<2, 1>(0, 3) - poses[1].block<2, 1>(0, 3)).norm(), 2e-3);
  EXPECT_NEAR(std::atan2(poses[0](1, 0), poses[0](0, 0)), std::atan2(poses[1](1, 0), poses[1](0, 0)), 1e-4);
}
