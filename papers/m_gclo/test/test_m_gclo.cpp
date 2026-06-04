#include "m_gclo/m_gclo.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::m_gclo;

namespace {

// 地面 (水平床, z=-1.7 付近) と非地面 (壁/箱) を含むシーン。
std::vector<Eigen::Vector3d> makeScene(double half = 12.0, double step = 0.5) {
  std::vector<Eigen::Vector3d> pts;
  // 地面 (広い水平面、センサ高 0 より下)。
  for (double a = -half; a <= half; a += step)
    for (double b = -half; b <= half; b += step) pts.emplace_back(a, b, -1.7);
  // 非地面 (鉛直な壁)。
  for (double a = -half; a <= half; a += step)
    for (double z = -1.5; z <= 3.0; z += step) {
      pts.emplace_back(a, -half, z);
      pts.emplace_back(a, half, z);
      pts.emplace_back(-half, a, z);
      pts.emplace_back(half, a, z);
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

MGcloParams baseParams() {
  MGcloParams p;
  p.voxel_size = 1.0;
  return p;
}

}  // namespace

// (1) 良条件シーンで既知の並進を回復する。
TEST(MGclo, RecoversKnownTranslation) {
  MGcloPipeline pipe(baseParams());

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

// (2) M-GCLO のコア: 地面平面拘束と非地面 NDT 拘束が両方使われる。
TEST(MGclo, UsesGroundAndNongroundConstraints) {
  MGcloPipeline pipe(baseParams());

  const auto room = makeScene();
  pipe.registerFrame(room);

  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.2;
  move(2, 3) = 0.1;  // 鉛直方向にも微小移動
  const auto moved = transformAll(room, move.inverse());
  const auto res = pipe.registerFrame(moved);

  EXPECT_GT(res.num_ground, 0);
  EXPECT_GT(res.num_nonground, 0);
  EXPECT_EQ(res.num_ground + res.num_nonground, res.num_correspondences);
}

// (3) M-GCLO の主張: 地面平面拘束が鉛直 (z) 精度を底上げする。
//     地面拘束あり (ground_weight 大) は z を回復し、地面拘束なし (非地面 NDT のみ)
//     より真値に近い。鉛直構造のみでは z は弱拘束で外れやすい。
TEST(MGclo, GroundConstraintImprovesVertical) {
  auto runZ = [](double ground_weight) {
    MGcloParams p = baseParams();
    p.ground_weight = ground_weight;  // 0 で地面拘束を無効化
    MGcloPipeline pipe(p);
    const auto room = makeScene();
    pipe.registerFrame(room);
    Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
    move(2, 3) = 0.3;  // 鉛直方向の移動
    const auto moved = transformAll(room, move.inverse());
    return pipe.registerFrame(moved).pose(2, 3);
  };

  const double with_ground = runZ(20.0);  // 地面拘束を効かせる
  const double no_ground = runZ(0.0);     // 非地面 NDT のみ

  // 地面拘束ありは鉛直を回復する。
  EXPECT_NEAR(with_ground, 0.3, 0.1);
  // 地面拘束ありは地面拘束なしより真値 (0.3) に近い。
  EXPECT_LT(std::abs(with_ground - 0.3), std::abs(no_ground - 0.3));
}
