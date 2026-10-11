#include "ct_icp/ct_icp_registration.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <random>

using namespace localization_zoo::ct_icp;

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

std::vector<TimedPoint> simulateScan(const std::vector<Eigen::Vector3d>& env,
                                      const TrajectoryFrame& frame,
                                      std::mt19937& rng) {
  std::vector<TimedPoint> tps;
  std::normal_distribution<double> noise(0, 0.02);
  for (const auto& wp : env) {
    double t = std::uniform_real_distribution<double>(0, 1)(rng);
    SE3 pose = SE3::interpolate(frame.begin_pose, frame.end_pose, frame.getAlpha(t));
    Eigen::Vector3d ps = pose.inverse() * wp;
    if (ps.norm() > 30 || ps.norm() < 0.5) continue;
    TimedPoint tp;
    tp.raw_point = ps + Eigen::Vector3d(noise(rng), noise(rng), noise(rng));
    tp.timestamp = t;
    tps.push_back(tp);
    if (tps.size() > 5000) break;
  }
  return tps;
}

}  // namespace

TEST(CTICP, StaticRegistration) {
  std::mt19937 rng(42);
  auto env = generateEnv(rng);

  CTICPParams params;
  params.voxel_resolution = 1.0;
  params.max_iterations = 20;
  params.planarity_threshold = 0.1;
  CTICPRegistration reg(params);

  TrajectoryFrame f0;
  f0.begin_pose.trans = f0.end_pose.trans = Eigen::Vector3d(0, 0, 1);
  auto s0 = simulateScan(env, f0, rng);
  std::vector<Eigen::Vector3d> w0;
  for (auto& tp : s0) w0.push_back(f0.transformPoint(tp.raw_point, tp.timestamp));
  reg.addPointsToMap(w0);

  TrajectoryFrame f1_gt;
  f1_gt.begin_pose.trans = Eigen::Vector3d(0, 0, 1);
  f1_gt.end_pose.trans = Eigen::Vector3d(1, 0, 1);
  auto s1 = simulateScan(env, f1_gt, rng);

  TrajectoryFrame f1_init;
  f1_init.begin_pose = f1_init.end_pose = f0.end_pose;
  auto result = reg.registerFrame(s1, f1_init, &f0);

  double err = (result.frame.end_pose.trans - f1_gt.end_pose.trans).norm();
  EXPECT_LT(err, 2.0);
}

TEST(CTICP, VoxelMap) {
  CTICPRegistration reg;
  std::vector<Eigen::Vector3d> pts;
  for (int i = 0; i < 1000; i++) pts.emplace_back(i * 0.1, 0, 0);
  reg.addPointsToMap(pts);
  EXPECT_GT(reg.mapSize(), 0u);
  EXPECT_LT(reg.mapSize(), 1000u);
}

TEST(CTICP, SlidingWindowMap) {
  CTICPParams params;
  params.voxel_resolution = 1.0;
  params.max_frames_in_map = 2;
  CTICPRegistration reg(params);

  std::vector<Eigen::Vector3d> frame0;
  std::vector<Eigen::Vector3d> frame1;
  std::vector<Eigen::Vector3d> frame2;
  for (int i = 0; i < 10; i++) {
    frame0.emplace_back(0.01 * i, 0.0, 0.0);
    frame1.emplace_back(10.0 + 0.01 * i, 0.0, 0.0);
    frame2.emplace_back(20.0 + 0.01 * i, 0.0, 0.0);
  }

  reg.addPointsToMap(frame0);
  reg.addPointsToMap(frame1);
  EXPECT_EQ(reg.mapSize(), 2u);

  reg.addPointsToMap(frame2);
  EXPECT_EQ(reg.mapSize(), 2u);
  EXPECT_EQ(reg.map().find(Voxel{0, 0, 0}), reg.map().end());
  EXPECT_NE(reg.map().find(Voxel{10, 0, 0}), reg.map().end());
  EXPECT_NE(reg.map().find(Voxel{20, 0, 0}), reg.map().end());
}

TEST(CTICP, InsertPointsCapAndSpacing) {
  CTICPParams params;
  params.voxel_resolution = 1.0;
  params.max_points_per_voxel = 30;
  params.min_distance_between_points = 0.1;
  CTICPRegistration reg(params);

  // 46 points 0.021 m apart in one voxel: the 0.1 m spacing keeps every fifth.
  std::vector<Eigen::Vector3d> dense;
  for (int i = 0; i < 46; i++) dense.emplace_back(0.01 + 0.021 * i, 0.5, 0.5);
  reg.insertPoints(dense);
  ASSERT_EQ(reg.mapSize(), 1u);
  EXPECT_EQ(reg.map().at(Voxel{0, 0, 0}).num_points, 10);

  // 40 well-separated points in another voxel stop at the 30-point cap.
  std::vector<Eigen::Vector3d> spread;
  for (int i = 0; i < 40; i++)
    spread.emplace_back(5.05 + 0.15 * (i % 6), 0.05 + 0.15 * ((i / 6) % 6), 0.5 + 0.2 * (i / 36));
  reg.insertPoints(spread);
  EXPECT_EQ(reg.map().at(Voxel{5, 0, 0}).num_points, 30);
}

TEST(CTICP, RemoveVoxelsFarFrom) {
  CTICPParams params;
  params.voxel_resolution = 1.0;
  CTICPRegistration reg(params);
  reg.insertPoints({Eigen::Vector3d(0.5, 0.5, 0.5), Eigen::Vector3d(50.5, 0.5, 0.5),
                    Eigen::Vector3d(150.5, 0.5, 0.5)});
  ASSERT_EQ(reg.mapSize(), 3u);
  reg.removeVoxelsFarFrom(Eigen::Vector3d::Zero(), 100.0);
  EXPECT_EQ(reg.mapSize(), 2u);
  EXPECT_EQ(reg.map().find(Voxel{150, 0, 0}), reg.map().end());
}

TEST(CTICP, UpstreamMechanicsRegistration) {
  // Upstream driving-profile settings (runCTICPUpstream) on a scene dense
  // enough for 20 neighbours within 0.75 m: ground and two walls.
  std::mt19937 rng(7);
  std::normal_distribution<double> n(0.0, 0.01);
  std::uniform_real_distribution<double> xy(-12, 12), z(0, 4);
  std::vector<Eigen::Vector3d> env;
  for (int i = 0; i < 40000; i++) env.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 15000; i++) env.emplace_back(xy(rng), 8.0 + n(rng), z(rng));
  for (int i = 0; i < 15000; i++) env.emplace_back(-8.0 + n(rng), xy(rng), z(rng));

  auto scan = [&](const TrajectoryFrame& frame) {
    std::vector<TimedPoint> tps;
    std::uniform_real_distribution<double> u(0, 1);
    for (const auto& wp : env) {
      const double t = u(rng);
      const SE3 pose = SE3::interpolate(frame.begin_pose, frame.end_pose, frame.getAlpha(t));
      TimedPoint tp;
      tp.raw_point = pose.inverse() * wp;
      tp.timestamp = t;
      tps.push_back(tp);
    }
    return tps;
  };

  CTICPParams params;
  params.voxel_resolution = 0.8;
  params.max_points_per_voxel = 30;
  params.min_distance_between_points = 0.1;
  params.max_frames_in_map = 0;
  params.knn = 20;
  params.neighbor_search_radius = 0.75;
  params.max_correspondence_dist = 1e12;
  params.planarity_threshold = 0.0;
  params.a2d_from_std = true;
  params.power_planarity = 2.0;
  params.weight_alpha = 0.9;
  params.weight_neighborhood = 0.1;
  params.neighborhood_weight_scale = 6.0;
  params.farthest_neighbor_reference = true;
  params.max_num_residuals = 900;
  params.cauchy_loss_param = 0.1;
  params.orientation_consistency_weight = 0.0;
  params.max_iterations = 15;
  params.ceres_max_iterations = 5;
  params.upstream_convergence = true;
  params.keypoint_voxel_size = 1.0;
  params.truncate_voxel_coords = true;
  CTICPRegistration reg(params);

  TrajectoryFrame f0;
  f0.begin_pose.trans = f0.end_pose.trans = Eigen::Vector3d(0, 0, 1);
  std::vector<Eigen::Vector3d> w0;
  for (auto& tp : scan(f0)) w0.push_back(f0.transformPoint(tp.raw_point, tp.timestamp));
  reg.insertPoints(w0);

  TrajectoryFrame f1_gt;
  f1_gt.begin_pose.trans = Eigen::Vector3d(0, 0, 1);
  f1_gt.end_pose.trans = Eigen::Vector3d(0.3, 0, 1);
  auto s1 = scan(f1_gt);
  std::shuffle(s1.begin(), s1.end(), rng);

  TrajectoryFrame f1_init;
  f1_init.begin_pose = f1_init.end_pose = f0.end_pose;
  auto result = reg.registerFrame(s1, f1_init, &f0);

  EXPECT_GT(result.num_residuals, 20);
  EXPECT_LE(result.num_residuals, 900);
  EXPECT_LT((result.frame.end_pose.trans - f1_gt.end_pose.trans).norm(), 0.1);
}
