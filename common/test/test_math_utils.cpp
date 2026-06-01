#include "common/math_utils.h"
#include "test_thresholds.h"

#include <gtest/gtest.h>

#include <Eigen/Geometry>

#include <cmath>
#include <random>

using namespace localization_zoo::math;
using localization_zoo::test::kEpsilonAngleBound;
using localization_zoo::test::kEpsilonAngleEquiv;
using localization_zoo::test::kEpsilonOrthonormal;
using localization_zoo::test::kEpsilonRotation;
using localization_zoo::test::kEpsilonSkew;

namespace {

TEST(MathUtils, SkewMatchesCrossProduct) {
  std::mt19937 rng(2026);
  std::uniform_real_distribution<double> u(-2.0, 2.0);
  for (int i = 0; i < 32; ++i) {
    const Eigen::Vector3d a(u(rng), u(rng), u(rng));
    const Eigen::Vector3d b(u(rng), u(rng), u(rng));
    EXPECT_TRUE((skew(a) * b).isApprox(a.cross(b), kEpsilonSkew));
  }
}

TEST(MathUtils, SkewIsSkewSymmetric) {
  const Eigen::Vector3d v(0.3, -1.1, 2.4);
  const Eigen::Matrix3d S = skew(v);
  EXPECT_TRUE(S.transpose().isApprox(-S, kEpsilonSkew));
}

TEST(MathUtils, NormalizeAngleMapsToMinusPiToPi) {
  const std::vector<double> inputs = {
      0.0,
      M_PI,
      -M_PI,
      4.2 * M_PI,
      -7.8 * M_PI,
      123.456,
  };
  for (double a : inputs) {
    const double n = normalizeAngle(a);
    EXPECT_GE(n, -M_PI - kEpsilonAngleBound);
    EXPECT_LE(n, M_PI + kEpsilonAngleBound);
  }
}

TEST(MathUtils, NormalizeAngleClearsToKnownValues) {
  // Fixed input → fixed output (within kEpsilonAngleEquiv).
  EXPECT_NEAR(normalizeAngle(0.0), 0.0, kEpsilonAngleEquiv);
  EXPECT_NEAR(normalizeAngle(3.0 * M_PI), M_PI, kEpsilonAngleEquiv);
  EXPECT_NEAR(normalizeAngle(-3.0 * M_PI), -M_PI, kEpsilonAngleEquiv);
}

TEST(MathUtils, NormalizeAnglePreservesEquivalentAngles) {
  const double base = 0.37;
  const double wrapped = base + 6.0 * M_PI;
  EXPECT_NEAR(normalizeAngle(base), normalizeAngle(wrapped), kEpsilonAngleEquiv);
}

TEST(MathUtils, FromRPYZeroIsIdentity) {
  const Eigen::Matrix3d R = fromRPY(0.0, 0.0, 0.0);
  EXPECT_TRUE(R.isIdentity(kEpsilonRotation));
}

TEST(MathUtils, FromRPYPureYawRotatesXY) {
  const double yaw = 0.42;
  const Eigen::Matrix3d R = fromRPY(0.0, 0.0, yaw);
  const Eigen::Vector3d v(1.0, 0.0, 0.0);
  const Eigen::Vector3d expected(std::cos(yaw), std::sin(yaw), 0.0);
  EXPECT_TRUE((R * v).isApprox(expected, kEpsilonRotation));
}

TEST(MathUtils, FromRPYIsOrthonormal) {
  std::mt19937 rng(9);
  std::uniform_real_distribution<double> angle(-0.4, 0.4);
  for (int i = 0; i < 20; ++i) {
    const Eigen::Matrix3d R =
        fromRPY(angle(rng), angle(rng), angle(rng));
    EXPECT_TRUE((R * R.transpose()).isIdentity(kEpsilonOrthonormal));
    EXPECT_NEAR(R.determinant(), 1.0, kEpsilonOrthonormal);
  }
}

}  // namespace
