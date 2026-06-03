#include "cube_lio/cube_lio.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::cube_lio;

namespace {
// 地面 + 2 壁。強度は壁/地面で別値、壁の境界に強度勾配を作る。
std::vector<IntensityPoint> makeScene(std::mt19937& rng) {
  std::vector<IntensityPoint> pts;
  std::uniform_real_distribution<double> xy(-30, 30);
  std::uniform_real_distribution<double> z(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 4000; i++) pts.push_back({{xy(rng), xy(rng), n(rng)}, 20.0});
  for (int i = 0; i < 1500; i++) pts.push_back({{xy(rng), 15 + n(rng), z(rng)}, 200.0});
  for (int i = 0; i < 1500; i++) pts.push_back({{-15 + n(rng), xy(rng), z(rng)}, 120.0});
  return pts;
}
}  // namespace

TEST(CubeLIO, FirstFrameInitializesMap) {
  CubeLIOPipeline pipeline(CubeLIOParams{});
  const auto result = pipeline.registerFrame({});
  EXPECT_TRUE(result.converged);
  EXPECT_TRUE(pipeline.pose().isApprox(result.pose));
}

TEST(CubeLIO, CubeMapDetectsIntensityGradient) {
  // 半球の半分だけ高強度にして、境界に IGM ピークが出ることを確認。
  CubeMap cube(128, 1.0);
  std::vector<IntensityPoint> pts;
  std::mt19937 rng(3);
  std::uniform_real_distribution<double> u(-1, 1);
  for (int i = 0; i < 20000; i++) {
    Eigen::Vector3d d(u(rng), u(rng), u(rng));
    if (d.norm() < 0.2) continue;
    d.normalize();
    const double intensity = d.x() > 0 ? 200.0 : 10.0;  // x=0 で段差
    pts.push_back({d * 10.0, intensity});
  }
  cube.build(pts);
  // 段差近傍 (x≈0, 強度境界) は勾配大、一様領域 (x>0 深部) は勾配小のはず。
  const double igm_edge = cube.sampleIGM(Eigen::Vector3d(0.05, 1, 0).normalized() * 10);
  const double igm_flat = cube.sampleIGM(Eigen::Vector3d(1, 0.05, 0).normalized() * 10);
  EXPECT_GE(igm_edge, igm_flat);
}

TEST(CubeLIO, MultiFrameTracksTranslation) {
  std::mt19937 rng(42);
  CubeLIOParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 40;
  params.cubemap_size = 128;
  CubeLIOPipeline pipeline(params);

  auto scene = makeScene(rng);
  for (int f = 0; f < 4; f++) {
    const Eigen::Vector3d t(f * 1.0, 0, 1);
    std::vector<IntensityPoint> scan;
    for (const auto& ip : scene) {
      const Eigen::Vector3d ps = ip.p - t;
      if (ps.norm() > 2 && ps.norm() < 30) scan.push_back({ps, ip.intensity});
    }
    const auto result = pipeline.registerFrame(scan);
    EXPECT_TRUE(result.converged);
    if (f > 0) EXPECT_GT(result.semi_dense_points, 0);
  }
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(3, 0, 0)).norm();
  EXPECT_LT(err, 2.0);
}
