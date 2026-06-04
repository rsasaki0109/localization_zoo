#include "svn_icp/svn_icp.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::svn_icp;

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

SvnIcpParams baseParams() {
  SvnIcpParams p;
  p.voxel_size = 1.0;
  p.num_particles = 16;
  p.svn_iterations = 6;
  p.lidar_sigma = 0.1;
  p.prior_precision = 0.1;
  return p;
}

}  // namespace

// (1) 良条件シーンで粒子平均が既知の並進を回復する。
TEST(SvnIcp, RecoversKnownTranslation) {
  SvnIcpPipeline pipe(baseParams());

  const auto room = makeScene(false);
  pipe.registerFrame(room);

  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.5;
  move(1, 3) = -0.3;
  const auto moved = transformAll(room, move.inverse());
  const auto res = pipe.registerFrame(moved);

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 0.5, 0.15);
  EXPECT_NEAR(res.pose(1, 3), -0.3, 0.15);
}

// (2) 不確かさ推定: 退化した廊下では粒子分散 (共分散) が閉じた箱より大きい。
TEST(SvnIcp, UncertaintyHigherInDegenerateCorridor) {
  auto runScene = [](bool corridor) {
    SvnIcpPipeline pipe(baseParams());
    const auto scene = makeScene(corridor);
    pipe.registerFrame(scene);
    Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
    move(1, 3) = 0.1;  // 両シーンで拘束される y 方向に微小移動
    const auto moved = transformAll(scene, move.inverse());
    return pipe.registerFrame(moved);
  };

  const auto corridor = runScene(true);
  const auto closed_box = runScene(false);

  // x が無拘束な廊下では粒子が広がり、共分散トレース・並進不確かさが大きい。
  EXPECT_GT(corridor.covariance_trace, closed_box.covariance_trace);
  EXPECT_GT(corridor.trans_std, closed_box.trans_std);
}

// (3) 決定的: 同入力で同じ姿勢を返す (固定シード)。
TEST(SvnIcp, Deterministic) {
  const auto room = makeScene(false);
  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.4;
  const auto moved = transformAll(room, move.inverse());

  SvnIcpPipeline a(baseParams());
  SvnIcpPipeline b(baseParams());
  a.registerFrame(room);
  b.registerFrame(room);
  const auto ra = a.registerFrame(moved);
  const auto rb = b.registerFrame(moved);
  EXPECT_NEAR(ra.pose(0, 3), rb.pose(0, 3), 1e-9);
  EXPECT_NEAR(ra.pose(1, 3), rb.pose(1, 3), 1e-9);
}
