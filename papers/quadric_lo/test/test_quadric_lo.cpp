#include "quadric_lo/quadric_lo.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::quadric_lo;

namespace {

// 平面 (壁/床) + 曲面 (円柱) を含むシーン。
std::vector<Eigen::Vector3d> makeScene(double half = 10.0, double step = 0.4) {
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
  // 曲面: 半径 2 の鉛直円柱 (中心 (3,3))。
  for (double th = 0; th < 2 * M_PI; th += 0.2)
    for (double z = -half; z <= half; z += step)
      pts.emplace_back(3.0 + 2.0 * std::cos(th), 3.0 + 2.0 * std::sin(th), z);
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

QuadricLoParams baseParams() {
  QuadricLoParams p;
  p.voxel_size = 1.0;
  return p;
}

}  // namespace

// (1) quadric 単項式と行列復元の整合 (q(y)=yᵀAy+bᵀy+c)。
TEST(QuadricLo, MonomialMatrixRoundtrip) {
  Vec10 theta;
  theta << 1.0, 2.0, 3.0, 0.5, -0.5, 0.25, 1.0, -2.0, 3.0, -4.0;
  const Eigen::Vector3d y(0.3, -0.7, 0.4);

  const double q_mono = quadricMonomials(y).dot(theta);
  const Eigen::Matrix3d A = quadricMatrixA(theta);
  const Eigen::Vector3d b = theta.segment<3>(6);
  const double q_mat = y.transpose() * A * y + b.dot(y) + theta(9);

  EXPECT_NEAR(q_mono, q_mat, 1e-9);
}

// (2) 良条件シーンで既知の並進を回復する。
TEST(QuadricLo, RecoversKnownTranslation) {
  QuadricLoPipeline pipe(baseParams());

  const auto room = makeScene();
  pipe.registerFrame(room);

  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.5;
  move(1, 3) = -0.3;
  const auto moved = transformAll(room, move.inverse());
  const auto res = pipe.registerFrame(moved);

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 0.5, 0.12);
  EXPECT_NEAR(res.pose(1, 3), -0.3, 0.12);
}

// (3) Quadric-LO のコア: 曲面 (円柱) では quadric 拘束が使われる。
TEST(QuadricLo, UsesQuadricConstraints) {
  QuadricLoPipeline pipe(baseParams());

  const auto room = makeScene();
  pipe.registerFrame(room);

  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.2;
  const auto moved = transformAll(room, move.inverse());
  const auto res = pipe.registerFrame(moved);

  // 円柱の曲面領域で quadric 当てはめが成立し、quadric 拘束が使われる。
  EXPECT_GT(res.num_quadric, 0);
  EXPECT_EQ(res.num_quadric + res.num_plane, res.num_correspondences);
}
