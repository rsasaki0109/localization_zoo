#include "dilo/dilo.h"

#include <gtest/gtest.h>

#include <cmath>
#include <random>
#include <vector>

using namespace localization_zoo::dilo;

namespace {

// 原点を囲む密な点群 (壁/床/天井)。SRI 投影が密に埋まるよう細かく生成。
std::vector<Eigen::Vector3d> makeScene(double half = 12.0, double step = 0.25) {
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

DiloParams baseParams() {
  DiloParams p;
  p.sri_height = 32;
  p.sri_width = 360;
  p.fov_up_deg = 85.0;  // 合成シーンは全方位に広がるため広い FOV
  p.fov_down_deg = -85.0;
  p.initial_threshold = 1.0;
  p.keyframe_translation = 5.0;
  p.keyframe_rotation_deg = 30.0;
  return p;
}

}  // namespace

// (1) SRI: 点を投影して対応・法線が取れる。
TEST(Dilo, SphericalRangeImageProjectsAndNormals) {
  SphericalRangeImage sri;
  SphericalRangeImage::Params sp;
  sp.height = 32;
  sp.width = 360;
  sp.fov_up_deg = 85.0;
  sp.fov_down_deg = -85.0;
  const auto room = makeScene();
  sri.build(room, sp);

  // 壁面 (+x 面 x=12) の点は密にサンプルされ、有効画素・単位法線が得られる。
  const auto px = sri.lookup(Eigen::Vector3d(12.0, 2.0, 1.0));
  EXPECT_TRUE(px.valid);
  EXPECT_TRUE(px.has_normal);
  EXPECT_NEAR(px.normal.norm(), 1.0, 1e-6);
  // 壁法線はセンサ (原点) 方向、すなわち -x 寄り。
  EXPECT_LT(px.normal.x(), -0.5);
}

TEST(Dilo, SphericalRangeImageLookupRadiusRecoversNeighborPixel) {
  const auto pointAt = [](double yaw_deg, double pitch_deg, double range) {
    const double yaw = yaw_deg * M_PI / 180.0;
    const double pitch = pitch_deg * M_PI / 180.0;
    return Eigen::Vector3d(range * std::cos(pitch) * std::cos(yaw),
                           range * std::cos(pitch) * std::sin(yaw),
                           range * std::sin(pitch));
  };

  std::vector<Eigen::Vector3d> patch = {
      pointAt(0.0, 0.0, 10.0), pointAt(1.0, 0.0, 10.0),
      pointAt(0.0, -1.0, 10.0)};

  SphericalRangeImage::Params exact_params;
  exact_params.height = 90;
  exact_params.width = 360;
  exact_params.fov_up_deg = 45.0;
  exact_params.fov_down_deg = -45.0;
  exact_params.lookup_radius = 0;

  SphericalRangeImage exact;
  exact.build(patch, exact_params);
  EXPECT_FALSE(exact.lookup(pointAt(-0.6, 0.0, 10.0)).valid);

  SphericalRangeImage::Params neighbor_params = exact_params;
  neighbor_params.lookup_radius = 1;
  neighbor_params.max_lookup_point_distance = 1.5;

  SphericalRangeImage neighbor;
  neighbor.build(patch, neighbor_params);
  const auto px = neighbor.lookup(pointAt(-0.6, 0.0, 10.0));
  EXPECT_TRUE(px.valid);
  EXPECT_TRUE(px.has_normal);
}

// (2) projective data association で既知の並進を direct alignment 回復する。
TEST(Dilo, RecoversKnownTranslation) {
  DiloPipeline pipe(baseParams());

  const auto room = makeScene();
  pipe.registerFrame(room);

  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.3;
  move(1, 3) = -0.2;
  const auto moved = transformAll(room, move.inverse());
  const auto res = pipe.registerFrame(moved);

  EXPECT_TRUE(res.converged);
  EXPECT_GT(res.num_correspondences, 50);
  EXPECT_NEAR(res.pose(0, 3), 0.3, 0.12);
  EXPECT_NEAR(res.pose(1, 3), -0.2, 0.12);
}

// (3) scan-to-local-map: 連続する複数フレームの並進を累積追従する。
TEST(Dilo, TracksConsecutiveMotion) {
  DiloPipeline pipe(baseParams());
  const auto room = makeScene();
  pipe.registerFrame(room);  // frame 0 = 初期マップ

  // 各フレーム +0.2m ずつ x 方向に動かす (累積 0.6m)。
  Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
  DiloResult res;
  for (int k = 1; k <= 3; k++) {
    step(0, 3) = 0.2 * k;
    const auto moved = transformAll(room, step.inverse());
    res = pipe.registerFrame(moved);
  }

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 0.6, 0.12);
}
