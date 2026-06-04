#include "small_mighty/small_mighty.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::small_mighty;

namespace {

// 面 (床/壁) と線 (柱の縁) を含むシーン。
std::vector<Eigen::Vector3d> makeScene(double half = 10.0, double step = 0.5) {
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
  // 線特徴: 縦の柱 (z 方向に並ぶ点列)。
  for (double z = -half; z <= half; z += step) {
    pts.emplace_back(3.0, 3.0, z);
    pts.emplace_back(-4.0, 2.0, z);
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

SmallMightyParams baseParams() {
  SmallMightyParams p;
  p.voxel_size = 1.0;
  return p;
}

}  // namespace

// (1) 良条件シーンで既知の並進を回復する。
TEST(SmallMighty, RecoversKnownTranslation) {
  SmallMightyPipeline pipe(baseParams());

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

// (2) contribution weighting: 安定 (分布平均より上) な特徴ほど重みが大きい。
TEST(SmallMighty, StableFeaturesGetHigherWeight) {
  SmallMightyPipeline pipe(baseParams());
  const auto room = makeScene();
  const auto feats = pipe.selectFeatures(room);

  ASSERT_FALSE(feats.empty());
  // 全特徴の重みは 1.0 以上 (1 + gain·clamp(z,0,cap))。
  double max_w = 0.0;
  bool has_planar = false, has_edge = false;
  for (const auto& f : feats) {
    EXPECT_GE(f.weight, 1.0);
    max_w = std::max(max_w, f.weight);
    has_planar |= !f.is_edge;
    has_edge |= f.is_edge;
  }
  // 平均より上の安定特徴が存在し、重みが 1.0 を超える。
  EXPECT_GT(max_w, 1.0);
  // 面・線の両特徴タイプが選ばれる。
  EXPECT_TRUE(has_planar);
  EXPECT_TRUE(has_edge);
}

// (3) contribution_gain=0 で重みが一様 (アブレーション) でも収束する。
TEST(SmallMighty, UniformWeightStillConverges) {
  SmallMightyParams p = baseParams();
  p.contribution_gain = 0.0;
  SmallMightyPipeline pipe(p);

  const auto room = makeScene();
  pipe.registerFrame(room);

  const auto feats = pipe.selectFeatures(room);
  for (const auto& f : feats) EXPECT_NEAR(f.weight, 1.0, 1e-9);

  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.4;
  const auto moved = transformAll(room, move.inverse());
  const auto res = pipe.registerFrame(moved);
  EXPECT_NEAR(res.pose(0, 3), 0.4, 0.12);
}
