#include "pcr_dat/pcr_dat.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::pcr_dat;

namespace {

// 密な壁面 (rich) と疎な点 (sparse) を混在させたシーン。
std::vector<Eigen::Vector3d> makeScene(double half = 10.0, double step = 0.5) {
  std::vector<Eigen::Vector3d> pts;
  for (double a = -half; a <= half; a += step)
    for (double b = -half; b <= half; b += step) {
      pts.emplace_back(a, b, -half);  // 床
      pts.emplace_back(a, b, half);   // 天井
      pts.emplace_back(a, -half, b);  // 壁
      pts.emplace_back(a, half, b);
      pts.emplace_back(-half, a, b);
      pts.emplace_back(half, a, b);
    }
  // 疎な孤立点 (rich しきい値未満になるよう離して配置)。
  for (double a = -half; a <= half; a += 4.0)
    pts.emplace_back(a, 0.3 * a, 0.1 * a);
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

PcrDatParams baseParams() {
  PcrDatParams p;
  p.voxel_size = 1.0;
  p.distribution_min_points = 8;
  p.distance_sigma = 0.1;
  return p;
}

}  // namespace

// (1) 良条件シーンで既知の並進を回復する。
TEST(PcrDat, RecoversKnownTranslation) {
  PcrDatPipeline pipe(baseParams());

  const auto room = makeScene();
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

// (2) PCR-DAT のコア: 分布(rich)ファクタと距離(sparse)ファクタが両方使われる。
TEST(PcrDat, UsesBothFactorTypes) {
  PcrDatPipeline pipe(baseParams());

  const auto room = makeScene();
  pipe.registerFrame(room);

  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(1, 3) = 0.2;
  const auto moved = transformAll(room, move.inverse());
  const auto res = pipe.registerFrame(moved);

  // 密な壁面は分布ファクタ、孤立点は距離ファクタに振り分けられる。
  EXPECT_GT(res.num_distribution, 0);
  EXPECT_GT(res.num_distance, 0);
  EXPECT_EQ(res.num_distribution + res.num_distance, res.num_correspondences);
}

// (3) 距離ファクタのみ (分布を無効化) でも収束する (退化耐性)。
TEST(PcrDat, DistanceOnlyStillConverges) {
  PcrDatParams p = baseParams();
  p.distribution_min_points = 100000;  // rich 判定を実質無効化
  PcrDatPipeline pipe(p);

  const auto room = makeScene();
  pipe.registerFrame(room);

  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.4;
  const auto moved = transformAll(room, move.inverse());
  const auto res = pipe.registerFrame(moved);

  EXPECT_EQ(res.num_distribution, 0);
  EXPECT_GT(res.num_distance, 0);
  EXPECT_NEAR(res.pose(0, 3), 0.4, 0.12);
}
