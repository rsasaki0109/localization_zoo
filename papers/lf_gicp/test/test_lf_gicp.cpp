#include <gtest/gtest.h>

#include <cmath>
#include <random>

#include <Eigen/Eigenvalues>

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

namespace {

// Correspondences of a straight street seen from a sensor at `sensor`:
// two facades (normal y) and the ground (normal z), few points constraining x.
struct StreetSystem {
  std::vector<Eigen::Matrix<double, 3, 6>> jac;
  std::vector<Eigen::Matrix3d> info;
  Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
};

StreetSystem streetSystem(const Eigen::Vector3d& sensor) {
  StreetSystem s;
  auto add = [&](const Eigen::Vector3d& local, const Eigen::Vector3d& normal) {
    const Eigen::Vector3d q = sensor + local;
    Eigen::Matrix<double, 3, 6> J;
    J.block<3, 3>(0, 0) << 0.0, q.z(), -q.y(), -q.z(), 0.0, q.x(), q.y(), -q.x(), 0.0;
    J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
    // Regularized planar information: kappa_t I + (kappa_n - kappa_t) n n^T.
    const Eigen::Matrix3d omega =
        0.45 * Eigen::Matrix3d::Identity() + 1.5 * normal * normal.transpose();
    s.jac.push_back(J);
    s.info.push_back(omega);
    s.H += J.transpose() * omega * J;
  };
  for (int i = -20; i <= 20; ++i) {
    add(Eigen::Vector3d(1.5 * i, 8.0, 0.5 * (i % 3)), Eigen::Vector3d::UnitY());
    add(Eigen::Vector3d(1.5 * i, -7.0, 0.5 * (i % 4)), Eigen::Vector3d::UnitY());
    add(Eigen::Vector3d(1.5 * i, 0.3 * (i % 5), -1.7), Eigen::Vector3d::UnitZ());
  }
  add(Eigen::Vector3d(25.0, 2.0, 0.0), Eigen::Vector3d::UnitX());  // one pole
  return s;
}

}  // namespace

TEST(LFGICP, CenteredHessianEqualsSensorCentredJacobians) {
  const Eigen::Vector3d c(420.0, -130.0, 6.0);
  const auto s = streetSystem(c);
  Eigen::Matrix<double, 6, 6> Hc = Eigen::Matrix<double, 6, 6>::Zero();
  for (size_t m = 0; m < s.jac.size(); ++m) {
    Eigen::Matrix<double, 3, 6> Jc = s.jac[m];
    // [-[q]x + [c]x] = -[q - c]x
    Jc(0, 1) -= c.z(); Jc(0, 2) += c.y();
    Jc(1, 0) += c.z(); Jc(1, 2) -= c.x();
    Jc(2, 0) -= c.y(); Jc(2, 1) += c.x();
    Hc += Jc.transpose() * s.info[m] * Jc;
  }
  EXPECT_LT((centeredHessian(s.H, c) - Hc).norm(), 1e-6 * Hc.norm());
}

TEST(LFGICP, ZeroTangentOriginReproducesPaperWeights) {
  const auto s = streetSystem(Eigen::Vector3d(30.0, 5.0, 0.0));
  const auto paper = fisherWeights(s.jac, s.info, s.H);
  const auto centred = fisherWeights(s.jac, s.info, s.H, Eigen::Vector3d::Zero());
  ASSERT_EQ(paper.size(), centred.size());
  for (size_t m = 0; m < paper.size(); ++m) EXPECT_NEAR(paper[m], centred[m], 1e-9);
}

TEST(LFGICP, PaperWeakDirectionDependsOnWorldOriginSensorCentredDoesNot) {
  // The same street at the world origin and 500 m away from it.
  const auto near = streetSystem(Eigen::Vector3d::Zero());
  const Eigen::Vector3d far_sensor(400.0, 300.0, 0.0);
  const auto far = streetSystem(far_sensor);

  // Paper Eq. (2)/(7): at the origin the weakest direction is the street axis;
  // far away it turns into a rotation about the sensor, because a rotation
  // about the sensor needs |t| times more twist translation to express.
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 6, 6>> near_eig(near.H);
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 6, 6>> far_eig(far.H);
  const Eigen::Matrix<double, 6, 1> v_near = near_eig.eigenvectors().col(0);
  const Eigen::Matrix<double, 6, 1> v_far = far_eig.eigenvectors().col(0);
  EXPECT_GT(std::abs(v_near(3)), 0.99);  // translation along x
  // Far away, the same eigenvector expressed as sensor-local motion
  // (omega, rho - t x omega) is a rotation about the sensor, not a translation.
  const Eigen::Vector3d omega = v_far.head<3>();
  const Eigen::Vector3d local_rho = v_far.tail<3>() - far_sensor.cross(omega);
  // A point 10 m from the sensor moves 10 |omega| under the rotation, more
  // than ten times the sensor-local translation |local_rho|.
  const double lever_m = 10.0;
  EXPECT_GT(omega.norm(), 1e-4);
  EXPECT_GT(lever_m * omega.norm(), 10.0 * local_rho.norm());

  const auto w_near = fisherWeights(near.jac, near.info, near.H);
  const auto w_far = fisherWeights(far.jac, far.info, far.H);
  double paper_diff = 0.0;
  for (size_t m = 0; m < w_near.size(); ++m) paper_diff = std::max(paper_diff, std::abs(w_near[m] - w_far[m]));
  EXPECT_GT(paper_diff, 1.0);

  // Sensor-centred tangent: identical weights wherever the origin is, and the
  // pole (the only x constraint) carries the largest weight.
  Eigen::Matrix<double, 6, 1> v_world;
  const auto c_near = fisherWeights(near.jac, near.info, near.H, Eigen::Vector3d::Zero());
  const auto c_far = fisherWeights(far.jac, far.info, far.H, far_sensor, &v_world);
  for (size_t m = 0; m < c_near.size(); ++m) EXPECT_NEAR(c_near[m], c_far[m], 1e-6);
  EXPECT_EQ(std::max_element(c_far.begin(), c_far.end()) - c_far.begin(),
            static_cast<long>(c_far.size()) - 1);
  // Expressed in the world twist, the centred direction is still a pure
  // sensor-local translation along x: rho + t x omega = v_world.tail.
  EXPECT_GT(std::abs(v_world(3)), 0.99);
}

TEST(LFGICP, SensorCenteredFisherRecoversMotionWithTheGateForcedOn) {
  std::mt19937 rng(11);
  const auto room = roomScan(&rng);
  LFGICPParams params;
  params.sensor_centered_fisher = true;
  params.tau_on = 1.0;          // force the gate on for every frame
  params.tau_off = 2.0;
  params.tau_absence = 1e9;
  params.gate_window = 1;
  LFGICPOdometry odom(params);
  Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
  int degenerate = 0;
  for (int k = 0; k < 6; ++k) {
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
    const auto result = odom.registerFrame(scan);
    if (result.degenerate) {
      ++degenerate;
      EXPECT_TRUE(result.diagnostics.valid);
      EXPECT_GE(result.diagnostics.max_weight, 1.0);
    }
  }
  EXPECT_EQ(degenerate, 5);
  EXPECT_LT((odom.pose().block<3, 1>(0, 3) - truth.block<3, 1>(0, 3)).norm(), 0.05);
}
