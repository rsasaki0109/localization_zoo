#include "litamin2/litamin2_odometry.h"

#include <gtest/gtest.h>
#include <Eigen/Geometry>

#include <cmath>
#include <random>

using namespace localization_zoo::litamin2;

namespace {

// Static street: ground, buildings with gaps on both sides (front and side
// walls), poles, and box-shaped objects.
std::vector<Eigen::Vector3d> street(std::mt19937& rng) {
  std::uniform_real_distribution<double> u(0.0, 1.0);
  std::normal_distribution<double> noise(0.0, 0.02);
  std::vector<Eigen::Vector3d> world;
  for (int i = 0; i < 60000; ++i) world.emplace_back(-40 + 120 * u(rng), -14 + 28 * u(rng), noise(rng));
  for (int side : {-1, 1}) {
    double x = -40.0 + (side > 0 ? 3.0 : 0.0);
    while (x < 80.0) {
      const double length = 8.0 + 6.0 * u(rng);
      const double front = side * (10.0 + 2.0 * u(rng));
      for (int i = 0; i < static_cast<int>(length * 400); ++i)
        world.emplace_back(x + length * u(rng), front + noise(rng), 8 * u(rng));
      for (double wall_x : {x, x + length})
        for (int i = 0; i < 1200; ++i)
          world.emplace_back(wall_x + noise(rng), front + side * 4.0 * u(rng), 8 * u(rng));
      x += length + 3.0 + 3.0 * u(rng);
    }
  }
  for (int k = 0; k < 17; ++k)
    for (int side : {-1, 1})
      for (int i = 0; i < 300; ++i) {
        const double a = 2 * M_PI * u(rng);
        world.emplace_back(-38 + 7.0 * k + side * 2.0 + 0.15 * std::cos(a), side * 7.0 + 0.15 * std::sin(a),
                           5 * u(rng));
      }
  for (int b = 0; b < 25; ++b) {
    const Eigen::Vector3d c(-35 + 4.5 * b, (b % 2 ? 6.0 : -6.0) + 2 * u(rng), 0.0);
    const Eigen::Vector3d half(0.5 + u(rng), 0.4 + 0.6 * u(rng), 0.6 + u(rng));
    for (int i = 0; i < 2000; ++i) {
      Eigen::Vector3d p = c + Eigen::Vector3d((2 * u(rng) - 1) * half.x(), (2 * u(rng) - 1) * half.y(),
                                              half.z() * (1 + (2 * u(rng) - 1)));
      if (i % 4 == 0) p.x() = c.x() + half.x();
      if (i % 4 == 1) p.x() = c.x() - half.x();
      if (i % 4 == 2) p.y() = c.y() + half.y();
      if (i % 4 == 3) p.y() = c.y() - half.y();
      world.push_back(p);
    }
  }
  return world;
}

std::vector<Eigen::Vector3d> scanFrom(const std::vector<Eigen::Vector3d>& world, const Eigen::Matrix4d& pose) {
  const Eigen::Matrix4d inv = pose.inverse();
  std::vector<Eigen::Vector3d> scan;
  for (const auto& p : world) {
    const Eigen::Vector3d q = inv.block<3, 3>(0, 0) * p + inv.block<3, 1>(0, 3);
    if (q.head<2>().norm() < 40.0) scan.push_back(q);
  }
  return scan;
}

double trackStreet(const LiTAMIN2OdometryParams& params) {
  std::mt19937 rng(7);
  const auto world = street(rng);
  LiTAMIN2Odometry odometry(params);
  Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
  for (int k = 0; k < 15; ++k) {
    if (k > 0) {
      Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
      step.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.01, Eigen::Vector3d::UnitZ()).toRotationMatrix();
      step(0, 3) = 1.2;
      truth = truth * step;
    }
    odometry.process(scanFrom(world, truth));
  }
  return (odometry.pose().block<3, 1>(0, 3) - truth.block<3, 1>(0, 3)).norm();
}

}  // namespace

TEST(LiTAMIN2Odometry, ThreeMetreVoxelsGiveFewFlatDistributions) {
  std::mt19937 rng(1);
  std::uniform_real_distribution<double> u(0.0, 1.0);
  std::normal_distribution<double> noise(0.0, 0.01);
  std::vector<Eigen::Vector3d> plane;
  for (int i = 0; i < 9000; ++i) plane.emplace_back(0.1 + 2.8 * u(rng), 0.1 + 2.8 * u(rng), 1.0 + noise(rng));
  LiTAMIN2Odometry odometry;
  const auto dists = odometry.voxelize(plane);
  ASSERT_EQ(dists.size(), 1u);
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(dists[0].cov);
  EXPECT_GT(std::abs(solver.eigenvectors().col(0).z()), 0.99);  // normal of the plane
  EXPECT_LT(solver.eigenvalues()(0), 2e-3);
  EXPECT_NEAR(dists[0].mean.z(), 1.0, 0.01);
}

TEST(LiTAMIN2Odometry, IcpCovWithScanDistributionMapTracksAStreet) {
  LiTAMIN2OdometryParams params;
  params.map_mode = LiTAMIN2OdometryParams::MapMode::kFirst;
  EXPECT_LT(trackStreet(params), 0.15);
}

TEST(LiTAMIN2Odometry, IcpOnlyWithAggregatedMapTracksAStreet) {
  LiTAMIN2OdometryParams params;
  params.use_cov_cost = false;
  EXPECT_LT(trackStreet(params), 0.15);
}

TEST(LiTAMIN2Odometry, AlignRecoversARotatedOffset) {
  // One scan in the map, the next scan rotated by 2 deg and shifted by 0.8 m:
  // a single registration from identity has to close most of it (the two
  // scans are voxelized in their own frames, so 3 m cells cut the surfaces
  // differently and a few cm of aliasing remain).
  std::mt19937 rng(3);
  const auto world = street(rng);
  for (bool cov : {false, true}) {
    LiTAMIN2OdometryParams params;
    params.use_cov_cost = cov;
    params.map_mode = LiTAMIN2OdometryParams::MapMode::kFirst;
    LiTAMIN2Odometry odometry(params);
    odometry.process(scanFrom(world, Eigen::Matrix4d::Identity()));
    Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
    truth.block<3, 3>(0, 0) = Eigen::AngleAxisd(2.0 * M_PI / 180.0, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    truth(0, 3) = 0.8;
    const auto source = odometry.voxelize(scanFrom(world, truth));
    const Eigen::Matrix4d T = odometry.align(source, Eigen::Matrix4d::Identity(), nullptr, nullptr);
    EXPECT_LT((T.block<3, 1>(0, 3) - truth.block<3, 1>(0, 3)).norm(), 0.2) << "cov=" << cov;
    const Eigen::Matrix3d dR = T.block<3, 3>(0, 0) * truth.block<3, 3>(0, 0).transpose();
    EXPECT_LT(Eigen::AngleAxisd(dR).angle(), 0.2 * M_PI / 180.0) << "cov=" << cov;
  }
}
