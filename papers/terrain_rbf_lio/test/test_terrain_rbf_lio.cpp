#include "terrain_rbf_lio/terrain_rbf_lio.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::terrain_rbf_lio;

namespace {

// 対称な立方体 (6 面) → H_tt = 2N·I で良条件。
std::vector<Eigen::Vector3d> makeCube(double half = 10.0, double step = 0.5) {
  std::vector<Eigen::Vector3d> pts;
  for (double a = -half; a <= half; a += step)
    for (double b = -half; b <= half; b += step) {
      pts.emplace_back(a, b, -half);
      pts.emplace_back(a, b, half);
      pts.emplace_back(a, -half, b);
      pts.emplace_back(a, half, b);
      pts.emplace_back(-half, a, b);
      pts.emplace_back(half, a, b);
    }
  return pts;
}

std::vector<Eigen::Vector3d> transformAll(
    const std::vector<Eigen::Vector3d>& pts, const Eigen::Matrix4d& T) {
  std::vector<Eigen::Vector3d> out(pts.size());
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  for (size_t i = 0; i < pts.size(); i++) out[i] = R * pts[i] + t;
  return out;
}

}  // namespace

// (1) 良条件シーンで既知の並進を回復する。
TEST(TerrainRbf, RecoversKnownTranslation) {
  TerrainRbfParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;
  params.terrain_weight = 0.0;  // 拘束 off でも素の ICP として動くこと
  TerrainRbfPipeline pipe(params);

  const auto room = makeCube();
  pipe.registerFrame(room);

  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.5;
  move(1, 3) = -0.3;
  const auto moved = transformAll(room, move.inverse());
  const auto res = pipe.registerFrame(moved);

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 0.5, 0.1);
  EXPECT_NEAR(res.pose(1, 3), -0.3, 0.1);
}

// (2) RBF 地形が傾斜地面を補間し、勾配も回復する。
TEST(TerrainRbf, InterpolatesSlopedGround) {
  TerrainRbfMap terrain(/*cell=*/2.0, /*sigma=*/3.0, /*ground_band=*/1.0);
  std::vector<Eigen::Vector3d> ground;
  for (double x = -20; x <= 20; x += 1.0)
    for (double y = -20; y <= 20; y += 1.0)
      ground.emplace_back(x, y, 0.1 * x);  // z = 0.1 x の傾斜面
  terrain.update(ground);

  bool valid = false;
  double dfdx = 0.0, dfdy = 0.0;
  const double f = terrain.evaluate(5.0, 0.0, &valid, &dfdx, &dfdy);
  EXPECT_TRUE(valid);
  EXPECT_NEAR(f, 0.5, 0.2);     // 0.1 * 5
  EXPECT_NEAR(dfdx, 0.1, 0.05);
  EXPECT_NEAR(dfdy, 0.0, 0.05);
}

// (3) 地形高さは ground (低い面) を追い、屋根 (高い点) に釣られない。
TEST(TerrainRbf, TracksGroundNotRoof) {
  TerrainRbfMap terrain(/*cell=*/2.0, /*sigma=*/3.0, /*ground_band=*/1.0);
  std::vector<Eigen::Vector3d> pts;
  for (double x = -10; x <= 10; x += 0.5)
    for (double y = -10; y <= 10; y += 0.5) {
      pts.emplace_back(x, y, 0.0);  // ground
      pts.emplace_back(x, y, 5.0);  // roof (同セル、高い)
    }
  terrain.update(pts);

  bool valid = false;
  const double f = terrain.evaluate(0.0, 0.0, &valid);
  EXPECT_TRUE(valid);
  EXPECT_LT(f, 1.0);  // ground (0) 近傍。屋根 (5) に釣られない
}
