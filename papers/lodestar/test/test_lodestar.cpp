#include "lodestar/lodestar.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::lodestar;

namespace {

// 対称な立方体 (6 面)。corridor=true なら ±x 面を外し、x 軸が退化する
// 開放トンネルにする (±y, ±z 面のみ → x 並進が不拘束)。
// 対称な立方体は H_tt = 2N·I で χ≈1 となり非退化。
std::vector<Eigen::Vector3d> makeScene(bool corridor = false, double half = 10.0,
                                       double step = 0.5) {
  std::vector<Eigen::Vector3d> pts;
  for (double a = -half; a <= half; a += step)
    for (double b = -half; b <= half; b += step) {
      pts.emplace_back(a, b, -half);  // -z 面
      pts.emplace_back(a, b, half);   // +z 面
      pts.emplace_back(a, -half, b);  // -y 面
      pts.emplace_back(a, half, b);   // +y 面
      if (!corridor) {
        pts.emplace_back(-half, a, b);  // -x 面 (廊下では開放)
        pts.emplace_back(half, a, b);   // +x 面
      }
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
TEST(Lodestar, RecoversKnownTranslation) {
  LodestarParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;
  LodestarPipeline pipe(params);

  const auto room = makeScene(/*corridor=*/false);
  pipe.registerFrame(room);  // 初期フレームで地図構築

  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.5;
  move(1, 3) = -0.3;
  const auto moved = transformAll(room, move.inverse());  // センサが move 進む
  const auto res = pipe.registerFrame(moved);

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 0.5, 0.1);
  EXPECT_NEAR(res.pose(1, 3), -0.3, 0.1);
}

// (2) 退化検出: 廊下 (x 軸退化) は退化判定 + anchor 注入、対称立方体は非退化。
TEST(Lodestar, DetectsDegenerateCorridorNotClosedBox) {
  auto runScene = [](bool corridor) {
    LodestarParams params;
    params.voxel_size = 1.0;
    params.max_icp_iterations = 30;
    params.t_chi = 1.5;
    LodestarPipeline pipe(params);
    const auto scene = makeScene(corridor);
    pipe.registerFrame(scene);
    Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
    move(1, 3) = 0.1;  // y 方向 (両シーンで拘束される方向) にわずかに動く
    const auto moved = transformAll(scene, move.inverse());
    return pipe.registerFrame(moved);
  };

  const auto corridor = runScene(true);
  const auto closed_box = runScene(false);

  // 廊下は並進条件数が大 (x 退化)、対称立方体は χ≈1 で小。
  EXPECT_GT(corridor.condition_number_trans,
            closed_box.condition_number_trans);
  EXPECT_TRUE(corridor.degenerate);
  EXPECT_GE(corridor.num_degenerate_dirs, 1);
  EXPECT_FALSE(closed_box.degenerate);
  EXPECT_EQ(closed_box.num_degenerate_dirs, 0);
}

// (3) DA-ASKF anchor: 廊下では x 軸が退化し LiDAR は拘束できない。純 LiDAR
//     scope では事前速度なしに退化軸の急な運動は原理的に復元できないが
//     (本来 IMU が担う)、anchor は退化軸の解を予測 (≈ゼロ) へ固定して発散を
//     防ぎ、観測軸 (y) は通常どおり復元する。
TEST(Lodestar, AnchorsStabilizesDegenerateAxis) {
  LodestarParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 40;
  params.t_chi = 1.5;
  params.anchor_strength = 1.0;
  LodestarPipeline pipe(params);

  const auto corridor = makeScene(/*corridor=*/true);
  pipe.registerFrame(corridor);  // frame 0: 地図

  // 真の運動: x+0.4 (退化軸, 復元不可) かつ y+0.2 (観測軸)。
  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.4;
  move(1, 3) = 0.2;
  const auto moved = transformAll(corridor, move.inverse());
  const auto res = pipe.registerFrame(moved);

  EXPECT_TRUE(res.degenerate);
  EXPECT_GE(res.num_degenerate_dirs, 1);
  // 観測軸 y は復元される。
  EXPECT_NEAR(res.pose(1, 3), 0.2, 0.1);
  // 退化軸 x は予測 (≈0) に固定され発散しない (anchor の安定化効果)。
  EXPECT_LT(std::abs(res.pose(0, 3)), 0.15);
}
