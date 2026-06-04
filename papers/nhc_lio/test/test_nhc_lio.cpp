#include "nhc_lio/nhc_lio.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::nhc_lio;

namespace {

// 全方位を囲む箱 (良条件)。
std::vector<Eigen::Vector3d> makeBox(double half = 10.0, double step = 0.5) {
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

// 横 (y) が弱拘束な退化シーン: 床 + 前方壁のみ (側壁なし)。
std::vector<Eigen::Vector3d> makeLateralAmbiguous(double half = 12.0,
                                                  double step = 0.4) {
  std::vector<Eigen::Vector3d> pts;
  for (double a = -half; a <= half; a += step)
    for (double b = -half; b <= half; b += step) {
      pts.emplace_back(a, b, -2.0);  // 床
      pts.emplace_back(15.0, a, b);  // 前方壁 (x=15)
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

NhcLioParams baseParams() {
  NhcLioParams p;
  p.voxel_size = 1.0;
  return p;
}

}  // namespace

// (1) 良条件で前進並進を回復する (NHC は前進運動を妨げない)。
TEST(NhcLio, RecoversForwardTranslation) {
  NhcLioPipeline pipe(baseParams());
  const auto box = makeBox();
  pipe.registerFrame(box);

  // 1 フレーム目で速度を立てる (NHC は 2 フレーム目以降に効く)。
  Eigen::Matrix4d m1 = Eigen::Matrix4d::Identity();
  m1(0, 3) = 0.5;
  pipe.registerFrame(transformAll(box, m1.inverse()));

  Eigen::Matrix4d m2 = Eigen::Matrix4d::Identity();
  m2(0, 3) = 1.0;  // 累積前進
  const auto res = pipe.registerFrame(transformAll(box, m2.inverse()));

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 1.0, 0.12);
  EXPECT_NEAR(res.pose(1, 3), 0.0, 0.12);
}

// (2) adaptive 重み: 急旋回の次フレームでは NHC 重みが減衰する。
TEST(NhcLio, AdaptiveWeightDecaysOnTurn) {
  NhcLioParams p = baseParams();
  p.nhc_weight = 5.0;
  p.nhc_yaw_ref = 0.03;
  NhcLioPipeline pipe(p);
  const auto box = makeBox();
  pipe.registerFrame(box);

  // 直進フレーム。
  Eigen::Matrix4d straight = Eigen::Matrix4d::Identity();
  straight(0, 3) = 0.3;
  const auto rs = pipe.registerFrame(transformAll(box, straight.inverse()));

  // 大きく yaw 回転するフレーム。
  Eigen::Matrix4d turn = Eigen::Matrix4d::Identity();
  const double yaw = 0.15;  // ~8.6 deg
  turn.block<3, 3>(0, 0) << std::cos(yaw), -std::sin(yaw), 0, std::sin(yaw),
      std::cos(yaw), 0, 0, 0, 1;
  turn(0, 3) = 0.6;
  pipe.registerFrame(transformAll(box, turn.inverse()));

  // 旋回の次フレームは NHC 重みが直進時より小さい。
  const auto rafter = pipe.registerFrame(transformAll(box, turn.inverse()));
  EXPECT_LT(rafter.nhc_weight_used, rs.nhc_weight_used);
}

// (3) 横が弱拘束なシーンで NHC が横/上下 drift を抑える。
TEST(NhcLio, NhcReducesLateralDrift) {
  auto run = [](bool enable_nhc) {
    NhcLioParams p = baseParams();
    p.enable_nhc = enable_nhc;
    p.nhc_weight = 20.0;
    p.planarity_threshold = 0.3;
    NhcLioPipeline pipe(p);
    const auto scene = makeLateralAmbiguous();
    pipe.registerFrame(scene);
    // 前進しつつ、データに微小な横ずれを混ぜる (drift を誘発)。
    Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
    double y = 0.0;
    Eigen::Matrix4d res = Eigen::Matrix4d::Identity();
    for (int k = 1; k <= 5; k++) {
      move(0, 3) = 0.4 * k;
      move(1, 3) = 0.15 * k;  // 真の横ずれ (弱拘束なので NHC が抑える対象)
      res = pipe.registerFrame(transformAll(scene, move.inverse())).pose;
    }
    return std::abs(res(1, 3));
  };

  const double y_off = run(false);
  const double y_on = run(true);
  // NHC 有効時は横方向の動きがより抑制される。
  EXPECT_LT(y_on, y_off);
}
