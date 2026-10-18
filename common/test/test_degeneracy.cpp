#include "common/degeneracy.h"

#include <Eigen/Geometry>
#include <gtest/gtest.h>

#include <limits>

using localization_zoo::math::remapDegenerateTranslation;

TEST(CommonDegeneracy, RemovesOnlyWeakDirectionsAndUsesStrictThreshold) {
  const Eigen::Matrix3d normal = Eigen::Vector3d(1.0, 3.0, 100.0).asDiagonal();
  Eigen::Vector3d update(2.0, 4.0, 6.0);
  EXPECT_EQ(remapDegenerateTranslation(normal, 0.03, update), 1);
  EXPECT_EQ(update, Eigen::Vector3d(0.0, 4.0, 6.0));
}

TEST(CommonDegeneracy, IsIndependentOfSensorAxesAndNormalEquationScale) {
  const Eigen::Matrix3d R = Eigen::AngleAxisd(0.7, Eigen::Vector3d(1, 2, 3).normalized()).toRotationMatrix();
  const Eigen::Matrix3d normal = R * Eigen::Vector3d(1, 2, 100).asDiagonal() * R.transpose();
  const Eigen::Vector3d original = R * Eigen::Vector3d(4, 5, 6);
  for (double scale : {1e-4, 1.0, 1e4}) {
    Eigen::Vector3d update = original;
    EXPECT_EQ(remapDegenerateTranslation(scale * normal, 0.03, update), 2);
    EXPECT_LT((update - 6 * R.col(2)).norm(), 1e-12);
  }
}

TEST(CommonDegeneracy, DisabledAndFullyConstrainedUpdatesAreUnchanged) {
  const Eigen::Matrix3d normal = Eigen::Matrix3d::Identity();
  const Eigen::Vector3d original(0.123, -2.5, 4.1);
  for (double ratio : {0.0, -1.0, 0.03}) {
    Eigen::Vector3d update = original;
    EXPECT_EQ(remapDegenerateTranslation(normal, ratio, update), 0);
    EXPECT_EQ(update, original);
  }
}

TEST(CommonDegeneracy, InPlaceSixVectorBlocksLeaveOtherDegreesOfFreedomUntouched) {
  const Eigen::Matrix3d normal = Eigen::Vector3d(1, 10, 100).asDiagonal();
  Eigen::Matrix<double, 6, 1> rotation_first, translation_first;
  rotation_first << 7, 8, 9, 2, 3, 4;
  translation_first << 2, 3, 4, 7, 8, 9;
  EXPECT_EQ(remapDegenerateTranslation(normal, 0.03, rotation_first.tail<3>()), 1);
  EXPECT_EQ(remapDegenerateTranslation(normal, 0.03, translation_first.head<3>()), 1);
  EXPECT_EQ(rotation_first.head<3>(), Eigen::Vector3d(7, 8, 9));
  EXPECT_EQ(translation_first.tail<3>(), Eigen::Vector3d(7, 8, 9));
  EXPECT_EQ(rotation_first.tail<3>(), translation_first.head<3>());
}

TEST(CommonDegeneracy, RankDeficiencyAndZeroSystemRetainLegacySemantics) {
  Eigen::Vector3d update(2, 3, 4);
  EXPECT_EQ(remapDegenerateTranslation(Eigen::Vector3d(0, 0, 100).asDiagonal(), 0.03, update), 2);
  EXPECT_EQ(update, Eigen::Vector3d(0, 0, 4));
  update = Eigen::Vector3d(2, 3, 4);
  EXPECT_EQ(remapDegenerateTranslation(Eigen::Matrix3d::Zero(), 0.03, update), 0);
  EXPECT_EQ(update, Eigen::Vector3d(2, 3, 4));
}

TEST(CommonDegeneracy, InvalidInputsDoNotContaminateFiniteUpdates) {
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double inf = std::numeric_limits<double>::infinity();
  for (double ratio : {nan, inf}) {
    Eigen::Vector3d update(2, 3, 4);
    EXPECT_EQ(remapDegenerateTranslation(Eigen::Matrix3d::Identity(), ratio, update), 0);
    EXPECT_EQ(update, Eigen::Vector3d(2, 3, 4));
  }
  Eigen::Matrix3d normal = Eigen::Matrix3d::Identity();
  normal(0, 0) = nan;
  Eigen::Vector3d update(2, 3, 4);
  EXPECT_EQ(remapDegenerateTranslation(normal, 0.03, update), 0);
  EXPECT_EQ(update, Eigen::Vector3d(2, 3, 4));
}
