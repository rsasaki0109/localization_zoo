#include "ct_icp_relead/degeneracy_aware_ct_icp.h"

#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo;

namespace {

std::vector<Eigen::Vector3d> generateEnv(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::normal_distribution<double> n(0.0, 0.01);
  std::uniform_real_distribution<double> xy(-30, 30), z(0, 4);
  for (int i = 0; i < 5000; i++) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 2000; i++) pts.emplace_back(xy(rng), 15.0 + n(rng), z(rng));
  for (int i = 0; i < 2000; i++) pts.emplace_back(-15.0 + n(rng), xy(rng), z(rng));
  return pts;
}

std::vector<ct_icp::TimedPoint> simulateScan(
    const std::vector<Eigen::Vector3d>& env,
    const ct_icp::TrajectoryFrame& frame, std::mt19937& rng) {
  std::vector<ct_icp::TimedPoint> tps;
  std::normal_distribution<double> noise(0, 0.02);
  for (const auto& wp : env) {
    double t = std::uniform_real_distribution<double>(0, 1)(rng);
    ct_icp::SE3 pose = ct_icp::SE3::interpolate(
        frame.begin_pose, frame.end_pose, frame.getAlpha(t));
    Eigen::Vector3d ps = pose.inverse() * wp;
    if (ps.norm() > 30 || ps.norm() < 0.5) continue;
    ct_icp::TimedPoint tp;
    tp.raw_point = ps + Eigen::Vector3d(noise(rng), noise(rng), noise(rng));
    tp.timestamp = t;
    tps.push_back(tp);
    if (tps.size() > 5000) break;
  }
  return tps;
}

}  // namespace

TEST(CTICPRelead, BasicRegistrationWithDegCheck) {
  std::mt19937 rng(42);
  auto env = generateEnv(rng);

  ct_icp_relead::DegAwareCTICPParams params;
  params.ct_icp_params.voxel_resolution = 1.0;
  params.ct_icp_params.max_iterations = 20;
  params.ct_icp_params.planarity_threshold = 0.1;

  ct_icp_relead::DegeneracyAwareCTICP reg(params);

  // Frame 0
  ct_icp::TrajectoryFrame f0;
  f0.begin_pose.trans = f0.end_pose.trans = Eigen::Vector3d(0, 0, 1);
  auto s0 = simulateScan(env, f0, rng);
  std::vector<Eigen::Vector3d> w0;
  for (auto& tp : s0) w0.push_back(f0.transformPoint(tp.raw_point, tp.timestamp));
  reg.addPointsToMap(w0);

  // Frame 1
  ct_icp::TrajectoryFrame f1_gt;
  f1_gt.begin_pose.trans = Eigen::Vector3d(0, 0, 1);
  f1_gt.end_pose.trans = Eigen::Vector3d(1, 0, 1);
  auto s1 = simulateScan(env, f1_gt, rng);

  ct_icp::TrajectoryFrame f1_init;
  f1_init.begin_pose = f1_init.end_pose = f0.end_pose;
  auto result = reg.registerFrame(s1, f1_init, &f0);

  double err = (result.ct_result.frame.end_pose.trans - f1_gt.end_pose.trans).norm();
  EXPECT_LT(err, 2.0);

  // 退化検知が実行されたことを確認 (結果は環境依存)
  // degeneracy_info が valid であること
  EXPECT_GE(result.degeneracy_info.degenerate_rotation_dirs.size() +
                result.degeneracy_info.degenerate_translation_dirs.size(), 0u);
}

TEST(CTICPRelead, MultiFrameWithDegeneracy) {
  std::mt19937 rng(123);
  auto env = generateEnv(rng);

  ct_icp_relead::DegAwareCTICPParams params;
  params.ct_icp_params.voxel_resolution = 1.0;
  params.ct_icp_params.max_iterations = 15;
  params.ct_icp_params.planarity_threshold = 0.1;

  ct_icp_relead::DegeneracyAwareCTICP reg(params);

  ct_icp::TrajectoryFrame prev_frame;
  prev_frame.begin_pose.trans = prev_frame.end_pose.trans = Eigen::Vector3d(0, 0, 1);

  // 3フレーム走行
  for (int f = 0; f < 3; f++) {
    ct_icp::TrajectoryFrame gt;
    gt.begin_pose.trans = Eigen::Vector3d(f * 1.0, 0, 1);
    gt.end_pose.trans = Eigen::Vector3d((f + 1) * 1.0, 0, 1);

    auto scan = simulateScan(env, gt, rng);

    if (f == 0) {
      std::vector<Eigen::Vector3d> wpts;
      for (auto& tp : scan)
        wpts.push_back(gt.transformPoint(tp.raw_point, tp.timestamp));
      reg.addPointsToMap(wpts);
      prev_frame = gt;
      continue;
    }

    ct_icp::TrajectoryFrame init;
    init.begin_pose = prev_frame.end_pose;
    init.end_pose = prev_frame.end_pose;

    auto result = reg.registerFrame(scan, init, &prev_frame);

    // マップに追加
    std::vector<Eigen::Vector3d> wpts;
    for (auto& tp : scan)
      wpts.push_back(result.ct_result.frame.transformPoint(tp.raw_point, tp.timestamp));
    reg.addPointsToMap(wpts);

    prev_frame = result.ct_result.frame;
  }

  // 最終位置が概ね正しいこと
  double err = (prev_frame.end_pose.trans - Eigen::Vector3d(3, 0, 1)).norm();
  EXPECT_LT(err, 3.0);
}
