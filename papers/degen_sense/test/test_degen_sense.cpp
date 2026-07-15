#include "degen_sense/degen_sense.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::degen_sense;
using localization_zoo::imu_preintegration::ImuSample;

namespace {
std::vector<Eigen::Vector3d> makeRoom(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-12, 12);
  std::uniform_real_distribution<double> z(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 12000; i++) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 6000; i++) pts.emplace_back(xy(rng), 10 + n(rng), z(rng));
  for (int i = 0; i < 6000; i++) pts.emplace_back(-10 + n(rng), xy(rng), z(rng));
  for (int i = 0; i < 6000; i++) pts.emplace_back(xy(rng), -10 + n(rng), z(rng));
  return pts;
}

std::vector<ImuSample> cvImu(int f) {
  std::vector<ImuSample> imu;
  for (int k = 0; k < 10; k++) {
    ImuSample s;
    s.timestamp = f + 0.1 * k;
    s.gyro = Eigen::Vector3d::Zero();
    s.accel = Eigen::Vector3d(0, 0, 9.81);
    imu.push_back(s);
  }
  return imu;
}
}  // namespace

TEST(DegenSense, FirstFrameInitializesMap) {
  DegenSensePipeline pipeline(DegenSenseParams{});
  const auto result = pipeline.registerFrame({}, {});
  EXPECT_TRUE(result.converged);
}

TEST(DegenSense, TracksTranslationAndReportsFactors) {
  std::mt19937 rng(42);
  DegenSenseParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 40;
  DegenSensePipeline pipeline(params);

  auto scene = makeRoom(rng);
  for (int f = 0; f < 4; f++) {
    const Eigen::Vector3d t(f * 0.5, 0, 0);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 1 && ps.norm() < 25) scan.push_back(ps);
    }
    const auto result = pipeline.registerFrame(scan, cvImu(f));
    EXPECT_TRUE(result.converged);
    if (f > 0) {
      EXPECT_GT(result.degeneracy_factor_trans, 0.0);  // 条件数は正
      EXPECT_GE(result.degeneracy_factor_trans, 1.0);  // λmax/λmin >= 1
    }
  }
  const double err =
      (pipeline.pose().block<3, 1>(0, 3) - Eigen::Vector3d(1.5, 0, 0)).norm();
  EXPECT_LT(err, 1.0);
}

namespace {
// 地面のみ (平面 z=0) → x,y 並進が完全に退化。
std::vector<Eigen::Vector3d> makeGroundOnly(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> a(-15, 15);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 20000; i++) pts.emplace_back(a(rng), a(rng), n(rng));
  return pts;
}

// 四方を壁で閉じた箱 → 全並進方向が拘束 (良条件)。
std::vector<Eigen::Vector3d> makeClosedBox(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> a(-15, 15);
  std::uniform_real_distribution<double> z(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 20000; i++) pts.emplace_back(a(rng), a(rng), n(rng));
  for (int i = 0; i < 5000; i++) {
    pts.emplace_back(a(rng), 15 + n(rng), z(rng));
    pts.emplace_back(a(rng), -15 + n(rng), z(rng));
    pts.emplace_back(15 + n(rng), a(rng), z(rng));
    pts.emplace_back(-15 + n(rng), a(rng), z(rng));
  }
  return pts;
}

double maxTransFactor(const std::vector<Eigen::Vector3d>& scene, double step) {
  DegenSenseParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 30;
  DegenSensePipeline pipeline(params);
  double mx = 0.0;
  for (int f = 0; f < 4; f++) {
    const Eigen::Vector3d t(f * step, 0, 0);
    std::vector<Eigen::Vector3d> scan;
    for (const auto& p : scene) {
      const Eigen::Vector3d ps = p - t;
      if (ps.norm() > 1 && ps.norm() < 35) scan.push_back(ps);
    }
    const auto r = pipeline.registerFrame(scan, {});
    if (f > 0) mx = std::max(mx, r.degeneracy_factor_trans);
  }
  return mx;
}
}  // namespace

TEST(DegenSense, DegeneracyFactorDiscriminatesDegenerateScene) {
  // 退化ファクタ (条件数) は、並進が退化した地面のみ平面で
  // 四方を閉じた箱より明確に大きくなるはず。
  std::mt19937 rng_a(7), rng_b(7);
  const double ground_max = maxTransFactor(makeGroundOnly(rng_a), 0.3);
  const double box_max = maxTransFactor(makeClosedBox(rng_b), 0.3);
  EXPECT_GT(ground_max, 1.0);
  EXPECT_GT(ground_max, box_max * 2.0);  // 退化シーンの条件数が明確に大
}

TEST(DegenSense, NoImuCompensationDoesNotUseConstantVelocityFallback) {
  std::mt19937 rng(42);
  const auto scene = makeRoom(rng);

  auto run = [&](double min_fusion_weight) {
    DegenSenseParams params;
    params.voxel_size = 1.0;
    params.max_icp_iterations = 40;
    params.warmup_frames = 1;
    params.mad_k = -1e9;  // force degeneracy after the warmup frame
    params.min_fusion_weight = min_fusion_weight;
    DegenSensePipeline pipeline(params);

    bool saw_degenerate = false;
    const std::vector<Eigen::Vector3d> path = {
        Eigen::Vector3d(0, 0, 0), Eigen::Vector3d(1, 0, 0),
        Eigen::Vector3d(1, 0, 0), Eigen::Vector3d(1, 0, 0)};
    for (const auto& t : path) {
      std::vector<Eigen::Vector3d> scan;
      for (const auto& p : scene) {
        const Eigen::Vector3d ps = p - t;
        if (ps.norm() > 1 && ps.norm() < 25) scan.push_back(ps);
      }
      const auto result = pipeline.registerFrame(scan, {});
      EXPECT_TRUE(result.converged);
      EXPECT_FALSE(result.used_imu);
      saw_degenerate = saw_degenerate || result.degenerate;
    }
    return std::make_pair(pipeline.pose(), saw_degenerate);
  };

  const auto low_weight = run(0.0);
  const auto high_weight = run(1.0);
  EXPECT_TRUE(low_weight.second);
  EXPECT_TRUE(high_weight.second);
  EXPECT_TRUE(low_weight.first.isApprox(high_weight.first, 1e-9));
}
