#include "lidar_iba/lidar_iba.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::lidar_iba;

namespace {

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

// (1) ステレオ投影は単位法線を返し、n→u→n の往復で復元する (北極含む)。
TEST(LidarIba, StereoRoundTripAndUnitNorm) {
  std::vector<Eigen::Vector3d> normals = {
      {0, 0, -1}, {1, 0, 0},  {0, 1, 0},
      {0, 0, 1},  // 北極: 符号反転で扱う
      Eigen::Vector3d(1, 1, 1).normalized(),
      Eigen::Vector3d(-0.3, 0.5, -0.8).normalized()};
  for (const auto& n : normals) {
    const Eigen::Vector2d u = normalToStereo(n);
    const Eigen::Vector3d nr = stereoToNormal(u);
    EXPECT_NEAR(nr.norm(), 1.0, 1e-9);  // 単位ノルム
    // n と nr は同一平面の法線 (符号反転は許容)。
    EXPECT_NEAR(std::abs(nr.dot(n)), 1.0, 1e-6);
  }
}

// (2) 解析 Jacobian ∂n/∂u が有限差分と一致する。
TEST(LidarIba, StereoJacobianMatchesFiniteDiff) {
  const std::vector<Eigen::Vector2d> us = {
      {0.0, 0.0}, {0.3, -0.4}, {0.7, 0.2}, {-0.5, -0.5}};
  const double eps = 1e-6;
  for (const auto& u : us) {
    const Eigen::Matrix<double, 3, 2> H = stereoJacobian(u);
    for (int j = 0; j < 2; j++) {
      Eigen::Vector2d up = u, um = u;
      up(j) += eps;
      um(j) -= eps;
      const Eigen::Vector3d fd =
          (stereoToNormal(up) - stereoToNormal(um)) / (2.0 * eps);
      for (int i = 0; i < 3; i++) EXPECT_NEAR(H(i, j), fd(i), 1e-5);
    }
  }
}

// (3) BA ありで良条件シーンの既知並進を回復する。
TEST(LidarIba, RecoversKnownTranslation) {
  LidarIbaParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;
  params.enable_ba = true;
  params.keyframe_interval = 1;
  params.window_size = 4;
  LidarIbaPipeline pipe(params);

  const auto room = makeCube();
  pipe.registerFrame(room);

  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  for (int step = 1; step <= 3; step++) {
    Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
    move(0, 3) = 0.5 * step;
    move(1, 3) = -0.3 * step;
    const auto moved = transformAll(room, move.inverse());
    const auto res = pipe.registerFrame(moved);
    pose = res.pose;
  }

  EXPECT_NEAR(pose(0, 3), 1.5, 0.15);
  EXPECT_NEAR(pose(1, 3), -0.9, 0.15);
}
