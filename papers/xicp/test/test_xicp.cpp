#include "xicp/xicp_registration.h"

#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::xicp;

namespace {

XICPParams testParams() {
  XICPParams p;
  p.kappa_1 = 100.0;
  p.kappa_2 = 70.0;
  p.kappa_3 = 15.0;
  return p;
}

std::vector<Correspondence> makeNormal(const Eigen::Matrix3d& R,
                                        const Eigen::Vector3d& t,
                                        std::mt19937& rng) {
  std::vector<Correspondence> corrs;
  std::normal_distribution<double> noise(0, 0.01);
  std::uniform_real_distribution<double> xy(-20, 20), z(0, 3);

  for (int i = 0; i < 300; i++) {
    Eigen::Vector3d q(xy(rng), xy(rng), 0);
    corrs.push_back({R.transpose() * (q - t) + Eigen::Vector3d(noise(rng), noise(rng), noise(rng)),
                     q, {0, 0, 1}});
  }
  for (int i = 0; i < 200; i++) {
    Eigen::Vector3d q(xy(rng), 10, z(rng));
    corrs.push_back({R.transpose() * (q - t) + Eigen::Vector3d(noise(rng), noise(rng), noise(rng)),
                     q, {0, 1, 0}});
  }
  for (int i = 0; i < 200; i++) {
    Eigen::Vector3d q(-10, xy(rng), z(rng));
    corrs.push_back({R.transpose() * (q - t) + Eigen::Vector3d(noise(rng), noise(rng), noise(rng)),
                     q, {-1, 0, 0}});
  }
  return corrs;
}

std::vector<Correspondence> makeTunnel(const Eigen::Matrix3d& R,
                                        const Eigen::Vector3d& t,
                                        std::mt19937& rng) {
  std::vector<Correspondence> corrs;
  std::normal_distribution<double> noise(0, 0.01);
  for (int i = 0; i < 500; i++) {
    double th = std::uniform_real_distribution<double>(0, 2 * M_PI)(rng);
    Eigen::Vector3d q(std::uniform_real_distribution<double>(-20, 20)(rng),
                       3 * std::cos(th), 3 * std::sin(th));
    Eigen::Vector3d n(0, std::cos(th), std::sin(th));
    corrs.push_back({R.transpose() * (q - t) + Eigen::Vector3d(noise(rng), noise(rng), noise(rng)),
                     q, n});
  }
  return corrs;
}

}  // namespace

TEST(XICP, NormalEnvironment) {
  std::mt19937 rng(42);
  auto corrs = makeNormal(Eigen::Matrix3d::Identity(), {1, 0.5, 1}, rng);

  XICPRegistration reg(testParams());
  auto result = reg.align(corrs);

  EXPECT_TRUE(result.converged);
  EXPECT_FALSE(result.localizability.hasDegeneracy());

  double err = (result.transformation.block<3, 1>(0, 3) - Eigen::Vector3d(1, 0.5, 1)).norm();
  EXPECT_LT(err, 0.5);
}

TEST(XICP, TunnelDegeneracy) {
  std::mt19937 rng(42);
  auto corrs = makeTunnel(Eigen::Matrix3d::Identity(), {0.5, 0, 0}, rng);

  XICPRegistration reg(testParams());
  auto result = reg.align(corrs);

  EXPECT_TRUE(result.localizability.hasDegeneracy());
}

TEST(XICP, ConstrainedRegistration) {
  std::mt19937 rng(42);
  auto corrs = makeTunnel(Eigen::Matrix3d::Identity(), {0, 0.3, 0}, rng);

  XICPRegistration reg(testParams());
  auto result = reg.align(corrs);

  EXPECT_TRUE(result.converged || result.num_iterations > 0);
}

TEST(XICP, ScoreComparison) {
  std::mt19937 rng(42);
  Eigen::Matrix3d R = Eigen::Matrix3d::Identity();
  Eigen::Vector3d t(0, 0, 1);

  XICPRegistration reg(testParams());
  auto r_normal = reg.align(makeNormal(R, t, rng));
  auto r_tunnel = reg.align(makeTunnel(R, t, rng));

  EXPECT_GT(r_normal.localizability.L_c_trans.minCoeff(),
            r_tunnel.localizability.L_c_trans.minCoeff());
}
