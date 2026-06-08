#include "mcgicp/mcgicp.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::mcgicp;

namespace {

std::vector<PointI> makeBox(double half = 10.0, double step = 0.5,
                            double intensity = 0.3) {
  std::vector<PointI> pts;
  for (double a = -half; a <= half; a += step)
    for (double b = -half; b <= half; b += step) {
      pts.push_back({Eigen::Vector3d(a, b, -half), intensity});
      pts.push_back({Eigen::Vector3d(a, b, half), intensity});
      pts.push_back({Eigen::Vector3d(a, -half, b), intensity});
      pts.push_back({Eigen::Vector3d(a, half, b), intensity});
      pts.push_back({Eigen::Vector3d(-half, a, b), intensity});
      pts.push_back({Eigen::Vector3d(half, a, b), intensity});
    }
  return pts;
}

std::vector<PointI> transformAll(const std::vector<PointI>& pts,
                                 const Eigen::Matrix4d& T) {
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  std::vector<PointI> out(pts.size());
  for (size_t i = 0; i < pts.size(); i++) {
    out[i].p = R * pts[i].p + t;
    out[i].intensity = pts[i].intensity;
  }
  return out;
}

McGicpLoParams baseParams() {
  McGicpLoParams p;
  p.voxel_size = 1.0;
  p.intensity_scale = 2.0;
  p.planarity_threshold = 0.3;
  return p;
}

}  // namespace

// (1) 同一 xyz で強度が違う点は 4D Mahalanobis が大きい。
TEST(McGicp, IntensityIncreasesMahalanobis) {
  const double s = 2.0;
  std::vector<PointI> neighbors = makeBox(4.0, 0.5, 0.2);
  const Mat4 cov = McGicpCore::computeCovariance(neighbors, s, 1e-3, 0.05);

  const Vec4 same_i =
      McGicpCore::toVec4({Eigen::Vector3d(0.0, 0.0, 0.0), 0.2}, s);
  const Vec4 diff_i =
      McGicpCore::toVec4({Eigen::Vector3d(0.0, 0.0, 0.0), 0.9}, s);
  const Vec4 ref = McGicpCore::toVec4({Eigen::Vector3d(0.1, 0.0, 0.0), 0.2}, s);

  const double d_match = McGicpCore::mahalanobisSquared(same_i, ref, cov, cov);
  const double d_mismatch =
      McGicpCore::mahalanobisSquared(diff_i, ref, cov, cov);
  EXPECT_GT(d_mismatch, d_match);
}

// (2) 良条件で前進並進を回復する。
TEST(McGicp, RecoversForwardTranslation) {
  McGicpLoPipeline pipe(baseParams());
  const auto box = makeBox();
  pipe.registerFrame(box);

  Eigen::Matrix4d m1 = Eigen::Matrix4d::Identity();
  m1(0, 3) = 0.5;
  pipe.registerFrame(transformAll(box, m1.inverse()));

  Eigen::Matrix4d m2 = Eigen::Matrix4d::Identity();
  m2(0, 3) = 1.0;
  const auto res = pipe.registerFrame(transformAll(box, m2.inverse()));

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 1.0, 0.15);
  EXPECT_NEAR(res.pose(1, 3), 0.0, 0.15);
}

// (3) 幾何的に曖昧な対応でも強度が一致すれば誤差が小さい。
TEST(McGicp, IntensityResolvesAmbiguity) {
  const double s = 2.0;
  std::vector<PointI> neighbors;
  neighbors.push_back({Eigen::Vector3d(0.0, 0.0, 0.0), 0.1});
  neighbors.push_back({Eigen::Vector3d(0.05, 0.0, 0.0), 0.9});
  neighbors.push_back({Eigen::Vector3d(-0.05, 0.0, 0.0), 0.1});
  neighbors.push_back({Eigen::Vector3d(0.0, 0.05, 0.0), 0.9});
  const Mat4 cov = McGicpCore::computeCovariance(neighbors, s, 1e-3, 0.05);

  const Vec4 query = McGicpCore::toVec4({Eigen::Vector3d(0.02, 0.0, 0.0), 0.88}, s);
  const Vec4 bright =
      McGicpCore::toVec4({Eigen::Vector3d(0.05, 0.0, 0.0), 0.9}, s);
  const Vec4 dark =
      McGicpCore::toVec4({Eigen::Vector3d(-0.05, 0.0, 0.0), 0.1}, s);

  const double d_bright = McGicpCore::mahalanobisSquared(query, bright, cov, cov);
  const double d_dark = McGicpCore::mahalanobisSquared(query, dark, cov, cov);
  EXPECT_LT(d_bright, d_dark);
}
