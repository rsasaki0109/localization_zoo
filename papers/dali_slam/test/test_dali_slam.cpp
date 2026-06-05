#include "dali_slam/dali_slam.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::dali_slam;

namespace {

// 対称立方体 (corridor=true で ±x 面を外し x 軸を退化させる)。
std::vector<Eigen::Vector3d> makeScene(bool corridor = false, double half = 10.0,
                                       double step = 0.5) {
  std::vector<Eigen::Vector3d> pts;
  for (double a = -half; a <= half; a += step)
    for (double b = -half; b <= half; b += step) {
      pts.emplace_back(a, b, -half);
      pts.emplace_back(a, b, half);
      pts.emplace_back(a, -half, b);
      pts.emplace_back(a, half, b);
      if (!corridor) {
        pts.emplace_back(-half, a, b);
        pts.emplace_back(half, a, b);
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

// (1) 良条件シーンで既知の並進を回復する (deskew off, degeneracy on)。
TEST(DaliSlam, RecoversKnownTranslation) {
  DaliSlamParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;
  params.enable_deskew = false;
  params.degeneracy_ratio = 0.1;  // 良条件では全並進方向が floor を上回る
  DaliSlamPipeline pipe(params);

  const auto room = makeScene(false);
  pipe.registerFrame(room);

  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.5;
  move(1, 3) = -0.3;
  const auto moved = transformAll(room, move.inverse());
  const auto res = pipe.registerFrame(moved);

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 0.5, 0.1);
  EXPECT_NEAR(res.pose(1, 3), -0.3, 0.1);
  EXPECT_FALSE(res.degenerate);
}

// (2) degeneracy remap: 廊下 (x 退化) で退化方向を検出・除去する。
TEST(DaliSlam, DegeneracyDetectsCorridor) {
  auto runScene = [](bool corridor) {
    DaliSlamParams params;
    params.voxel_size = 1.0;
    params.max_icp_iterations = 30;
    params.enable_deskew = false;
    params.degeneracy_ratio = 0.1;  // 相対基準 λ_k < 0.1·λmax_t
    DaliSlamPipeline pipe(params);
    const auto scene = makeScene(corridor);
    pipe.registerFrame(scene);
    Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
    move(1, 3) = 0.1;  // 両シーンで拘束される y 方向に微小移動
    const auto moved = transformAll(scene, move.inverse());
    return pipe.registerFrame(moved);
  };

  const auto corridor = runScene(true);
  const auto closed_box = runScene(false);

  // 廊下は x 並進が無拘束 → 最小固有値が箱より顕著に小さく、相対 floor を下回る。
  EXPECT_LT(corridor.min_eigenvalue, closed_box.min_eigenvalue);
  EXPECT_TRUE(corridor.degenerate);
  EXPECT_GE(corridor.num_degenerate_dirs, 1);
  EXPECT_FALSE(closed_box.degenerate);  // 対称箱は全並進方向が拘束 → 非退化
}

// (3) deskew 有効でも移動シーケンスの並進を回復する (deskew が破綻しない)。
TEST(DaliSlam, DeskewEnabledStillRecoversMotion) {
  DaliSlamParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;
  params.enable_deskew = true;
  params.spline_quadratic = true;
  params.degeneracy_ratio = 0.1;
  DaliSlamPipeline pipe(params);

  const auto room = makeScene(false);
  pipe.registerFrame(room);

  // 合成の剛体箱には実 scan の歪みが無いため deskew は微小な擬似歪みを足す。
  // ここでは deskew 有効でも軌跡が破綻せず概ね回復することを確認する
  // (実 KITTI では歪みがあるため deskew が効く)。
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  for (int step = 1; step <= 4; step++) {
    Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
    move(0, 3) = 0.15 * step;
    const auto moved = transformAll(room, move.inverse());
    pose = pipe.registerFrame(moved).pose;
  }
  EXPECT_TRUE(std::isfinite(pose(0, 3)));
  EXPECT_NEAR(pose(0, 3), 0.6, 0.25);
}
