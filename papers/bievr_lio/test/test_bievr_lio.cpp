#include "bievr_lio/bievr_lio.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::bievr_lio;

namespace {
// 密な室内 + 起伏のある地面 (bump 画像が情報を持つ)。
std::vector<Eigen::Vector3d> makeScene(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-12, 12);
  std::uniform_real_distribution<double> z(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  // 起伏地面: z = 0.15*sin(x)*cos(y) (bump)。
  for (int i = 0; i < 14000; i++) {
    double x = xy(rng), y = xy(rng);
    pts.emplace_back(x, y, 0.15 * std::sin(x) * std::cos(y) + n(rng));
  }
  for (int i = 0; i < 6000; i++) pts.emplace_back(xy(rng), 10 + n(rng), z(rng));
  for (int i = 0; i < 6000; i++) pts.emplace_back(-10 + n(rng), xy(rng), z(rng));
  for (int i = 0; i < 6000; i++) pts.emplace_back(xy(rng), -10 + n(rng), z(rng));
  return pts;
}
}  // namespace

TEST(BievrLIO, FirstFrameInitializesMap) {
  BievrLIOPipeline pipeline(BievrLIOParams{});
  const auto result = pipeline.registerFrame({});
  EXPECT_TRUE(result.converged);
}

TEST(BievrLIO, BumpImageStoresHeightDeviation) {
  // 起伏地面 1 ボクセルで bump 高さが場所により変わることを確認。
  BumpVoxelMap map(2.0, 8, 0.3, 0.2);
  std::mt19937 rng(3);
  std::normal_distribution<double> n(0, 0.005);
  std::uniform_real_distribution<double> a(-0.95, 0.95);
  std::vector<Eigen::Vector3d> pts;
  for (int i = 0; i < 600; i++) {
    double x = a(rng), y = a(rng);
    pts.emplace_back(x, y, 0.2 * x + n(rng));  // 傾いた面 (x 方向に高さ勾配)
  }
  map.addPoints(pts);
  const auto fa = map.query(Eigen::Vector3d(-0.6, 0, -0.12));
  const auto fb = map.query(Eigen::Vector3d(0.6, 0, 0.12));
  ASSERT_TRUE(fa.has_plane);
  ASSERT_TRUE(fb.has_plane);
  // 画像勾配が非ゼロ (面内に高さ変化がある = bump が効く)。
  EXPECT_GT(std::abs(fa.grad_u) + std::abs(fa.grad_v) +
                std::abs(fb.grad_u) + std::abs(fb.grad_v),
            0.0);
}

TEST(BievrLIO, MultiFrameTracksTranslation) {
  std::mt19937 rng(42);
  BievrLIOParams params;
  params.voxel_size = 1.0;
  params.pixel_res = 0.25;
  params.max_icp_iterations = 40;
  BievrLIOPipeline pipeline(params);

  auto scene = makeScene(rng);
  for (int f = 0; f < 4; f++) {
    const Eigen::Vector3d t(f * 0.5, 0, 0);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 1 && ps.norm() < 25) scan.push_back(ps);
    }
    const auto result = pipeline.registerFrame(scan);
    EXPECT_TRUE(result.converged);
  }
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(1.5, 0, 0)).norm();
  EXPECT_LT(err, 1.0);
}
