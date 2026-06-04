#include "damm_loam/damm_loam.h"

#include <gtest/gtest.h>

#include <random>
#include <vector>

using namespace localization_zoo::damm_loam;

namespace {

// 地面 (z=0) + 4 壁の閉じた部屋を生成する。
std::vector<Eigen::Vector3d> makeRoom(double half = 10.0, double step = 0.25,
                                      bool with_walls = true) {
  std::vector<Eigen::Vector3d> pts;
  for (double x = -half; x <= half; x += step)
    for (double y = -half; y <= half; y += step)
      pts.emplace_back(x, y, 0.0);  // 地面
  if (with_walls) {
    for (double x = -half; x <= half; x += step)
      for (double z = 0.0; z <= 4.0; z += step) {
        pts.emplace_back(x, -half, z);
        pts.emplace_back(x, half, z);
      }
    for (double y = -half; y <= half; y += step)
      for (double z = 0.0; z <= 4.0; z += step) {
        pts.emplace_back(-half, y, z);
        pts.emplace_back(half, y, z);
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

// (1) 分類: 地面と壁を正しく ground / wall に分ける。
TEST(DammLoam, ClassifiesGroundAndWalls) {
  DammLoamParams params;
  params.voxel_size = 1.0;
  params.ground_z = 0.5;  // センサ高 ~ なので z<0.5 を地面候補に
  DammLoamPipeline pipe(params);

  const auto room = makeRoom();
  const auto labels = pipe.classifyScan(room);

  int ground = 0, wall = 0;
  for (auto l : labels) {
    if (l == PointClass::kGround) ++ground;
    if (l == PointClass::kWall) ++wall;
  }
  EXPECT_GT(ground, 0);
  EXPECT_GT(wall, 0);
  // 平らな部屋では地面点が支配的。
  EXPECT_GT(ground, wall);
}

// (2) 既知の並進を回復する。
TEST(DammLoam, RecoversKnownTranslation) {
  DammLoamParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;
  DammLoamPipeline pipe(params);

  const auto room = makeRoom();
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

// (3) 退化検出: 地面のみの平面シーンは並進 (XY) が拘束されず退化と判定。
//     閉じた部屋 (壁あり) は全方向拘束され退化しない。
TEST(DammLoam, DetectsDegenerateGroundOnlyScene) {
  auto runScene = [](bool with_walls) {
    DammLoamParams params;
    params.voxel_size = 1.0;
    params.max_icp_iterations = 30;
    params.degeneracy_ratio = 0.05;
    DammLoamPipeline pipe(params);
    const auto scene = makeRoom(10.0, 0.25, with_walls);
    pipe.registerFrame(scene);
    Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
    move(2, 3) = 0.1;  // わずかに上昇 (地面拘束は効く方向)
    const auto moved = transformAll(scene, move.inverse());
    return pipe.registerFrame(moved);
  };

  const auto ground_only = runScene(false);
  const auto closed_box = runScene(true);

  // 地面のみは並進条件数が大 (XY 退化)、閉じた部屋は小。
  EXPECT_GT(ground_only.degeneracy_factor_trans,
            closed_box.degeneracy_factor_trans);
  EXPECT_TRUE(ground_only.degenerate);
  EXPECT_FALSE(closed_box.degenerate);
}
