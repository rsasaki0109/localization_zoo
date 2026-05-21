#include "kiss_icp/kiss_icp.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::kiss_icp;

namespace {
std::vector<Eigen::Vector3d> makeScene(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-30, 30);
  std::uniform_real_distribution<double> z(0, 4);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 3000; i++) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 1000; i++) pts.emplace_back(xy(rng), 15+n(rng), z(rng));
  for (int i = 0; i < 1000; i++) pts.emplace_back(-15+n(rng), xy(rng), z(rng));
  return pts;
}
}

TEST(KISSICP, VoxelHashMap) {
  VoxelHashMap map(1.0);
  std::mt19937 rng(42);
  auto pts = makeScene(rng);
  map.addPoints(pts);
  EXPECT_GT(map.size(), 0u);
  EXPECT_LT(map.size(), pts.size());
}

TEST(KISSICP, PairMatcher) {
  std::mt19937 rng(42);
  auto scene = makeScene(rng);

  const Eigen::Matrix3d R =
      Eigen::AngleAxisd(0.03, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  const Eigen::Vector3d t(0.8, -0.2, 0.0);

  std::vector<Eigen::Vector3d> target;
  std::vector<Eigen::Vector3d> source;
  for (const auto& p : scene) {
    if (p.norm() > 3 && p.norm() < 30) target.push_back(p);
    const Eigen::Vector3d ps = R.transpose() * (p - t);
    if (ps.norm() > 3 && ps.norm() < 30) source.push_back(ps);
  }

  KISSMatcherParams params;
  params.target_voxel_size = 0.5;
  params.source_voxel_size = 0.5;
  params.max_correspondence_distance = 1.5;
  params.max_icp_iterations = 40;
  params.min_correspondences = 50;
  KISSMatcher matcher(params);
  matcher.setTarget(target);

  const auto result = matcher.align(source);
  EXPECT_TRUE(result.converged);
  EXPECT_GT(result.num_correspondences, 50);
  EXPECT_LT((result.transform.block<3, 1>(0, 3) - t).norm(), 0.15);
  EXPECT_LT((result.transform.block<3, 3>(0, 0) - R).norm(), 0.15);
}

TEST(KISSICP, MultiFrame) {
  std::mt19937 rng(42);
  KISSICPParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;  // テスト用に制限
  KISSICPPipeline pipeline(params);

  auto scene = makeScene(rng);
  for (int f = 0; f < 3; f++) {
    // センサ座標に変換
    Eigen::Matrix3d R = Eigen::AngleAxisd(f * 0.02, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    Eigen::Vector3d t(f * 1.0, 0, 1);
    std::vector<Eigen::Vector3d> scan;
    for (auto& p : scene) {
      Eigen::Vector3d ps = R.transpose() * (p - t);
      if (ps.norm() > 3 && ps.norm() < 30) scan.push_back(ps);
    }
    auto result = pipeline.registerFrame(scan);
    EXPECT_TRUE(result.converged);
  }
  // The pipeline reports odometry relative to the first frame, whose
  // synthetic sensor height is treated as the origin here.
  double err =
      (pipeline.pose().block<3,1>(0,3) - Eigen::Vector3d(2,0,0)).norm();
  EXPECT_LT(err, 2.0);
}
