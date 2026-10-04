#include <gtest/gtest.h>

#include <cmath>
#include <random>

#include "lf_gicp/lf_gicp.h"

using namespace localization_zoo::lf_gicp;

namespace {

VoxelGaussian planarVoxel(const Eigen::Vector3d& normal) {
  // Thin along the normal, wide in-plane.
  Eigen::Vector3d a = normal.unitOrthogonal();
  Eigen::Vector3d b = normal.cross(a);
  VoxelGaussian g;
  g.covariance = 1e-4 * normal * normal.transpose() + 0.08 * a * a.transpose() +
                 0.08 * b * b.transpose();
  g.count = 10;
  return g;
}

std::vector<Eigen::Vector3d> roomScan(std::mt19937* rng) {
  // Points on the inner walls, floor, and ceiling of a 30 x 20 x 6 m room
  // around the origin, plus a few pillars to make it non-symmetric.
  std::uniform_real_distribution<double> u(-1.0, 1.0);
  std::vector<Eigen::Vector3d> pts;
  for (int i = 0; i < 6000; ++i) {
    const double s = u(*rng), t = u(*rng);
    switch (i % 6) {
      case 0: pts.emplace_back(15.0, 10.0 * s, 3.0 * t); break;
      case 1: pts.emplace_back(-15.0, 10.0 * s, 3.0 * t); break;
      case 2: pts.emplace_back(15.0 * s, 10.0, 3.0 * t); break;
      case 3: pts.emplace_back(15.0 * s, -10.0, 3.0 * t); break;
      case 4: pts.emplace_back(15.0 * s, 10.0 * t, -2.0); break;
      default: pts.emplace_back(4.0 + 0.5 * s, -3.0 + 0.5 * t, 3.0 * u(*rng)); break;
    }
  }
  return pts;
}

}  // namespace

TEST(LFGICP, CorridorFieldIsDegenerateAlongTheCorridor) {
  std::vector<VoxelGaussian> corridor;
  for (int i = 0; i < 50; ++i) {
    corridor.push_back(planarVoxel(Eigen::Vector3d::UnitY()));
    corridor.push_back(planarVoxel(Eigen::Vector3d::UnitZ()));
  }
  const auto field = computeLocalizabilityField(corridor, 4096);
  EXPECT_LT(field.f0, 1e-3);
  EXPECT_GT(std::abs(field.weak_axis.x()), 0.99);  // along-corridor axis
  EXPECT_LT(field.lambda0, 1e-3);

  std::vector<VoxelGaussian> open = corridor;
  for (int i = 0; i < 50; ++i) open.push_back(planarVoxel(Eigen::Vector3d::UnitX()));
  const auto open_field = computeLocalizabilityField(open, 4096);
  EXPECT_NEAR(open_field.f0, 1.0 / 3.0, 1e-3);
  EXPECT_GT(open_field.lambda0, 0.2);

  // Regularization shrinks planarity (paper Table IV): f0 is unchanged for
  // identical voxels, but the absolute mass lambda0 drops.
  const auto regularized = computeLocalizabilityField(open, 4096, 2.0);
  EXPECT_NEAR(regularized.f0, 1.0 / 3.0, 1e-3);
  EXPECT_LT(regularized.lambda0, 0.02);
  EXPECT_GT(regularized.lambda0, 0.005);
}

TEST(LFGICP, FieldSubsamplesToMaxVoxels) {
  std::vector<VoxelGaussian> many(10000, planarVoxel(Eigen::Vector3d::UnitZ()));
  const auto field = computeLocalizabilityField(many, 4096);
  EXPECT_LE(field.sampled_voxels, 4096);
  EXPECT_GT(field.sampled_voxels, 3000);
}

TEST(LFGICP, FisherWeightsAreMeanNormalizedAndFavourTheWeakAxis) {
  // Two correspondences constrain y/z, one constrains x (the weak axis).
  std::vector<Eigen::Matrix<double, 3, 6>> jac(3);
  std::vector<Eigen::Matrix3d> info;
  for (auto& J : jac) {
    J.setZero();
    J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
    J.block<3, 3>(0, 0) = 1e-3 * Eigen::Matrix3d::Identity();
  }
  info.push_back(Eigen::Vector3d(0.01, 10.0, 0.01).asDiagonal());
  info.push_back(Eigen::Vector3d(0.01, 0.01, 10.0).asDiagonal());
  info.push_back(Eigen::Vector3d(1.0, 0.01, 0.01).asDiagonal());
  Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
  for (size_t m = 0; m < jac.size(); ++m) H += jac[m].transpose() * info[m] * jac[m];
  H.block<3, 3>(0, 0) += Eigen::Matrix3d::Identity();  // keep rotation well posed
  const auto w = fisherWeights(jac, info, H);
  EXPECT_NEAR((w[0] + w[1] + w[2]) / 3.0, 1.0, 1e-9);
  EXPECT_GT(w[2], w[0]);
  EXPECT_GT(w[2], w[1]);
}

TEST(LFGICP, GateUsesMediansWithHysteresis) {
  LFGICPParams params;
  params.gate_window = 3;
  DegeneracyGate gate(params);
  EXPECT_FALSE(gate.update(0.30, 0.03));
  EXPECT_FALSE(gate.update(0.10, 0.005));  // median still open
  EXPECT_TRUE(gate.update(0.10, 0.005));   // median now degenerate
  EXPECT_TRUE(gate.update(0.175, 0.005));  // inside the hysteresis band: stays on
  EXPECT_TRUE(gate.update(0.175, 0.005));
  EXPECT_TRUE(gate.update(0.30, 0.005));   // median 0.175: still inside the band
  EXPECT_FALSE(gate.update(0.30, 0.005));  // median 0.30 > tau_off: exits
  // Dilution (low f0 but large weak-axis mass) never enters.
  DegeneracyGate dilution(params);
  for (int i = 0; i < 5; ++i) EXPECT_FALSE(dilution.update(0.05, 0.03));
}

TEST(LFGICP, RecoversKnownMotionInARoom) {
  std::mt19937 rng(7);
  const auto room = roomScan(&rng);
  for (bool mitigation : {false, true}) {
    LFGICPParams params;
    params.enable_mitigation = mitigation;
    LFGICPOdometry odom(params);
    Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
    for (int k = 0; k < 6; ++k) {
      // The sensor moves +0.4 m in x and yaws 1 degree per frame.
      if (k > 0) {
        Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
        step.block<3, 3>(0, 0) =
            Eigen::AngleAxisd(M_PI / 180.0, Eigen::Vector3d::UnitZ()).toRotationMatrix();
        step(0, 3) = 0.4;
        truth = truth * step;
      }
      std::vector<Eigen::Vector3d> scan;
      const Eigen::Matrix4d inv = truth.inverse();
      for (const auto& p : room) scan.push_back(inv.block<3, 3>(0, 0) * p + inv.block<3, 1>(0, 3));
      odom.registerFrame(scan);
    }
    EXPECT_LT((odom.pose().block<3, 1>(0, 3) - truth.block<3, 1>(0, 3)).norm(), 0.05)
        << "mitigation=" << mitigation;
  }
}
