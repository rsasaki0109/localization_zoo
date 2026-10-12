#include "imls_slam/imls_paper_odometry.h"

#include <gtest/gtest.h>
#include <Eigen/Geometry>

#include <cmath>
#include <random>

using namespace localization_zoo::imls_slam;

namespace {

// Dense room: floor, three walls and a few boxes, sampled every `step` metres
// with a little noise (the paper's h and r need dense surfaces).
std::vector<Eigen::Vector3d> room(double step, std::mt19937& rng) {
  std::normal_distribution<double> noise(0.0, 0.005);
  std::vector<Eigen::Vector3d> pts;
  for (double a = -8.0; a <= 8.0; a += step)
    for (double b = -6.0; b <= 6.0; b += step) pts.emplace_back(a, b, -1.7 + noise(rng));
  for (double a = -8.0; a <= 8.0; a += step)
    for (double z = -1.7; z <= 2.0; z += step) {
      pts.emplace_back(a, 6.0 + noise(rng), z);
      pts.emplace_back(a, -6.0 + noise(rng), z);
    }
  for (double b = -6.0; b <= 6.0; b += step)
    for (double z = -1.7; z <= 2.0; z += step) pts.emplace_back(8.0 + noise(rng), b, z);
  for (const Eigen::Vector3d c : {Eigen::Vector3d(2.0, 3.0, -1.2), Eigen::Vector3d(-3.0, -2.5, -1.0)})
    for (double a = -0.5; a <= 0.5; a += step)
      for (double z = -0.5; z <= 0.5; z += step) {
        pts.push_back(c + Eigen::Vector3d(a, 0.5, z));
        pts.push_back(c + Eigen::Vector3d(a, -0.5, z));
        pts.push_back(c + Eigen::Vector3d(0.5, a, z));
        pts.push_back(c + Eigen::Vector3d(-0.5, a, z));
      }
  return pts;
}

std::vector<Eigen::Vector3d> seenFrom(const std::vector<Eigen::Vector3d>& world, const Eigen::Matrix4d& pose) {
  const Eigen::Matrix4d inv = pose.inverse();
  std::vector<Eigen::Vector3d> out;
  out.reserve(world.size());
  for (const auto& p : world) out.push_back(inv.block<3, 3>(0, 0) * p + inv.block<3, 1>(0, 3));
  return out;
}

}  // namespace

TEST(ImlsPaperOdometry, ImlsDistanceToAPlane) {
  ImlsPaperParams params;
  params.min_range = 0.0;
  ImlsPaperOdometry odometry(params);
  std::vector<Eigen::Vector3d> plane;
  for (double a = -1.0; a <= 1.0; a += 0.02)
    for (double b = -1.0; b <= 1.0; b += 0.02) plane.emplace_back(a, b, 0.0);
  odometry.insertScan(plane, Eigen::Matrix4d::Identity());
  Eigen::Vector3d n;
  double I;
  ASSERT_TRUE(odometry.imls(Eigen::Vector3d(0.1, -0.2, 0.05), &n, &I));
  EXPECT_GT(std::abs(n.z()), 0.999);
  EXPECT_NEAR(I * n.z(), 0.05, 1e-3);  // signed distance along the normal
  EXPECT_FALSE(odometry.imls(Eigen::Vector3d(0.0, 0.0, 0.5), &n, &I));  // nothing within r
}

TEST(ImlsPaperOdometry, ListsRankPlanarPointsByTheirConstraint) {
  ImlsPaperOdometry odometry;
  std::vector<ImlsPaperOdometry::ScanPoint> pts(3);
  pts[0] = {Eigen::Vector3d(5, 0, 0), Eigen::Vector3d(1, 0, 0), 1.0};    // constrains x
  pts[1] = {Eigen::Vector3d(0, 5, 0), Eigen::Vector3d(0, 1, 0), 1.0};    // constrains y
  pts[2] = {Eigen::Vector3d(5, 0, 0), Eigen::Vector3d(1, 0, 0), 0.1};    // weakly planar
  const auto lists = odometry.sampleLists(pts);
  ASSERT_EQ(lists.size(), 9u);
  EXPECT_EQ(lists[6].front(), 0);  // |n.x| list: the planar x point first
  EXPECT_EQ(lists[7].front(), 1);  // |n.y| list
  EXPECT_EQ(lists[6].back(), 1);
}

TEST(ImlsPaperOdometry, TracksMotionInADenseRoom) {
  std::mt19937 rng(5);
  const auto world = room(0.04, rng);
  ImlsPaperParams params;
  params.window_scans = 5;
  params.samples_per_list = 50;
  params.min_range = 0.0;
  ImlsPaperOdometry odometry(params);
  Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
  for (int k = 0; k < 6; ++k) {
    if (k > 0) {
      Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
      step.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.02, Eigen::Vector3d::UnitZ()).toRotationMatrix();
      step(0, 3) = 0.3;
      step(1, 3) = 0.05;
      truth = truth * step;
    }
    odometry.process(seenFrom(world, truth));
  }
  EXPECT_LT((odometry.pose().block<3, 1>(0, 3) - truth.block<3, 1>(0, 3)).norm(), 0.03);
  const Eigen::Matrix3d dR = odometry.pose().block<3, 3>(0, 0) * truth.block<3, 3>(0, 0).transpose();
  EXPECT_LT(Eigen::AngleAxisd(dR).angle(), 0.003);
}
