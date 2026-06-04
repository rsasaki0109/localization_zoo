#include "d2lio/d2lio.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::d2lio;
using localization_zoo::imu_preintegration::ImuSample;

namespace {
std::vector<Eigen::Vector3d> makeScene(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-30, 30);
  std::uniform_real_distribution<double> z(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 3000; i++) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 1000; i++) pts.emplace_back(xy(rng), 15 + n(rng), z(rng));
  for (int i = 0; i < 1000; i++) pts.emplace_back(-15 + n(rng), xy(rng), z(rng));
  return pts;
}
}  // namespace

TEST(D2LIO, FirstFrameInitializesMap) {
  D2LIOPipeline pipeline(D2LIOParams{});
  const auto result = pipeline.registerFrame({}, {});
  EXPECT_TRUE(result.converged);
  EXPECT_TRUE(pipeline.pose().isApprox(result.pose));
}

TEST(D2LIO, TracksTranslationNoImu) {
  std::mt19937 rng(42);
  D2LIOParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;
  D2LIOPipeline pipeline(params);

  auto scene = makeScene(rng);
  for (int f = 0; f < 4; f++) {
    const Eigen::Vector3d t(f * 1.0, 0, 1);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 3 && ps.norm() < 30) scan.push_back(ps);
    }
    const auto result = pipeline.registerFrame(scan, {});
    EXPECT_TRUE(result.converged);
  }
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(3, 0, 0)).norm();
  EXPECT_LT(err, 2.0);
}

TEST(D2LIO, DegeneracyRegularizationStabilizesTunnelLikeScene) {
  // 地面 + 2 枚の縦壁 (x 方向に拘束が弱いトンネル様) で前進する。
  // IMU prior があれば x 並進が破綻しないことを確認する。
  std::mt19937 rng(7);
  std::vector<Eigen::Vector3d> scene;
  std::uniform_real_distribution<double> x(-40, 40);
  std::uniform_real_distribution<double> zd(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 4000; i++) scene.emplace_back(x(rng), n(rng) + 6, zd(rng));   // 右壁 y=6
  for (int i = 0; i < 4000; i++) scene.emplace_back(x(rng), n(rng) - 6, zd(rng));   // 左壁 y=-6
  for (int i = 0; i < 4000; i++) scene.emplace_back(x(rng), n(rng) * 600, 0.0);     // 地面

  D2LIOParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 40;
  D2LIOPipeline pipeline(params);  // デフォルト比 0.05: x は point-to-point で
                                   // 部分的に拘束されるので退化発火せず追従する。

  for (int f = 0; f < 4; f++) {
    const Eigen::Vector3d t(f * 1.0, 0, 0);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 2 && ps.norm() < 40) scan.push_back(ps);
    }
    // 等速 (1 m/frame) を表す軽い IMU (回転ゼロ)。
    std::vector<ImuSample> imu;
    for (int k = 0; k < 10; k++) {
      ImuSample s;
      s.timestamp = f + 0.1 * k;
      s.gyro = Eigen::Vector3d::Zero();
      s.accel = Eigen::Vector3d(0, 0, 9.81);
      imu.push_back(s);
    }
    const auto result = pipeline.registerFrame(scan, imu);
    EXPECT_TRUE(result.converged);
    if (f > 0) EXPECT_TRUE(result.used_imu);  // f==0 は初期化フレーム
  }
  // x 方向に大きく破綻していないこと。
  EXPECT_LT(std::abs(pipeline.pose()(0, 3) - 3.0), 2.5);
}

TEST(D2LIO, DegeneracyDetectorFiresOnUnconstrainedDirection) {
  // 完全に平面のみのシーン (y=±6 の壁 + z=0 の地面、planarity=0 で全点 point-to-plane)。
  // 並進拘束は y と z のみ → x 方向は厳密に無拘束で検出器が退化を拾うはず。
  std::mt19937 rng(11);
  std::vector<Eigen::Vector3d> scene;
  std::uniform_real_distribution<double> x(-40, 40);
  std::uniform_real_distribution<double> zd(0, 4);
  std::uniform_real_distribution<double> yg(-18, 18);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 5000; i++) scene.emplace_back(x(rng), n(rng) + 6, zd(rng));
  for (int i = 0; i < 5000; i++) scene.emplace_back(x(rng), n(rng) - 6, zd(rng));
  for (int i = 0; i < 5000; i++) scene.emplace_back(x(rng), yg(rng), n(rng));

  D2LIOParams params;
  params.voxel_size = 1.0;
  params.planarity_threshold = 0.0;  // 全対応を point-to-plane にし x 漏れを排除
  params.max_icp_iterations = 30;
  D2LIOPipeline pipeline(params);

  bool fired = false;
  for (int f = 0; f < 3; f++) {
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      if (p.norm() > 2 && p.norm() < 40) scan.push_back(p);
    }
    const auto result = pipeline.registerFrame(scan, {});
    EXPECT_TRUE(result.converged);
    if (result.degenerate_trans_dirs > 0) fired = true;
  }
  EXPECT_TRUE(fired);  // x 無拘束方向で退化正則化が発火する。
}
