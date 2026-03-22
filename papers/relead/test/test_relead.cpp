#include "relead/cesikf.h"
#include "relead/degeneracy_detection.h"

#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::relead;

namespace {

std::vector<PointWithNormal> makeCorrespondences(bool tunnel, std::mt19937& rng) {
  std::vector<PointWithNormal> corrs;
  std::normal_distribution<double> noise(0.0, 0.01);
  std::uniform_real_distribution<double> xy(-20, 20), z(0, 3);

  // Ground
  for (int i = 0; i < 200; i++) {
    PointWithNormal pn;
    pn.point = Eigen::Vector3d(xy(rng), xy(rng), noise(rng));
    pn.normal = Eigen::Vector3d(0, 0, 1);
    pn.closest = pn.point; pn.closest.z() = 0;
    corrs.push_back(pn);
  }

  if (!tunnel) {
    for (int i = 0; i < 100; i++) {
      PointWithNormal pn;
      pn.point = Eigen::Vector3d(xy(rng), 10.0 + noise(rng), z(rng));
      pn.normal = Eigen::Vector3d(0, 1, 0);
      pn.closest = pn.point; pn.closest.y() = 10.0;
      corrs.push_back(pn);
    }
    for (int i = 0; i < 100; i++) {
      PointWithNormal pn;
      pn.point = Eigen::Vector3d(-10.0 + noise(rng), xy(rng), z(rng));
      pn.normal = Eigen::Vector3d(-1, 0, 0);
      pn.closest = pn.point; pn.closest.x() = -10.0;
      corrs.push_back(pn);
    }
  } else {
    for (int i = 0; i < 200; i++) {
      double th = std::uniform_real_distribution<double>(0, 2 * M_PI)(rng);
      Eigen::Vector3d n(0, std::cos(th), std::sin(th));
      Eigen::Vector3d wp(xy(rng), 3 * std::cos(th), 3 * std::sin(th));
      PointWithNormal pn;
      pn.point = wp; pn.normal = n; pn.closest = wp;
      corrs.push_back(pn);
    }
  }
  return corrs;
}

}  // namespace

TEST(RELEAD, NoDegeneracyInNormalEnv) {
  std::mt19937 rng(42);
  auto corrs = makeCorrespondences(false, rng);
  DegeneracyDetection det;
  auto info = det.detect(corrs);
  EXPECT_FALSE(info.hasDegeneracy());
}

TEST(RELEAD, DetectsTunnelDegeneracy) {
  std::mt19937 rng(42);
  auto corrs = makeCorrespondences(true, rng);
  DegeneracyDetection det;
  auto info = det.detect(corrs);
  EXPECT_TRUE(info.hasTranslationDegeneracy());
}

TEST(RELEAD, CESIKFUpdate) {
  std::mt19937 rng(42);
  CESIKF filter;
  filter.state().position = Eigen::Vector3d(0.5, 0.3, 1.2);
  auto corrs = makeCorrespondences(false, rng);
  auto result = filter.update(corrs);
  double err = (result.state.position - Eigen::Vector3d(0, 0, 1)).norm();
  EXPECT_LT(err, 1.0);
}

TEST(RELEAD, ConstrainedUpdatePreservesDegDir) {
  std::mt19937 rng(42);
  CESIKF filter;
  filter.state().position = Eigen::Vector3d(0.5, 0.0, 1.0);
  auto corrs = makeCorrespondences(true, rng);
  auto result = filter.update(corrs);
  EXPECT_TRUE(result.degeneracy_info.hasDegeneracy());
  // x方向（退化）は初期値付近に留まるはず
  EXPECT_NEAR(result.state.position.x(), 0.5, 0.1);
}

TEST(RELEAD, IMUPrediction) {
  CESIKF filter;
  filter.state().velocity = Eigen::Vector3d(1, 0, 0);
  filter.state().gravity = Eigen::Vector3d(0, 0, -9.81);

  ImuMeasurement imu;
  imu.linear_acceleration = Eigen::Vector3d(0, 0, 9.81);
  for (int i = 0; i < 100; i++) filter.predict(imu, 0.01);

  EXPECT_NEAR(filter.state().position.x(), 1.0, 0.01);
  EXPECT_NEAR(filter.state().position.y(), 0.0, 0.01);
  EXPECT_NEAR(filter.state().position.z(), 0.0, 0.01);
}
