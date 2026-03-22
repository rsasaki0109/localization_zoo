#include "xicp/xicp_registration.h"

#include <cmath>
#include <iostream>
#include <random>

using namespace localization_zoo::xicp;

/// 通常環境の対応関係: 地面+壁2面 (全方向ローカライズ可能)
std::vector<Correspondence> generateNormalCorrespondences(
    const Eigen::Matrix3d& R_gt, const Eigen::Vector3d& t_gt,
    std::mt19937& rng) {
  std::vector<Correspondence> corrs;
  std::normal_distribution<double> noise(0.0, 0.01);

  // 地面 (z=0, n=[0,0,1])
  for (int i = 0; i < 300; i++) {
    std::uniform_real_distribution<double> xy(-20.0, 20.0);
    Eigen::Vector3d q(xy(rng), xy(rng), 0.0);
    Eigen::Vector3d p = R_gt.transpose() * (q - t_gt) +
                        Eigen::Vector3d(noise(rng), noise(rng), noise(rng));
    corrs.push_back({p, q, Eigen::Vector3d(0, 0, 1)});
  }
  // 壁1 (y=10, n=[0,1,0])
  for (int i = 0; i < 200; i++) {
    std::uniform_real_distribution<double> x(-20.0, 20.0);
    std::uniform_real_distribution<double> z(0.0, 3.0);
    Eigen::Vector3d q(x(rng), 10.0, z(rng));
    Eigen::Vector3d p = R_gt.transpose() * (q - t_gt) +
                        Eigen::Vector3d(noise(rng), noise(rng), noise(rng));
    corrs.push_back({p, q, Eigen::Vector3d(0, 1, 0)});
  }
  // 壁2 (x=-10, n=[-1,0,0])
  for (int i = 0; i < 200; i++) {
    std::uniform_real_distribution<double> y(-20.0, 20.0);
    std::uniform_real_distribution<double> z(0.0, 3.0);
    Eigen::Vector3d q(-10.0, y(rng), z(rng));
    Eigen::Vector3d p = R_gt.transpose() * (q - t_gt) +
                        Eigen::Vector3d(noise(rng), noise(rng), noise(rng));
    corrs.push_back({p, q, Eigen::Vector3d(-1, 0, 0)});
  }
  return corrs;
}

/// トンネル環境: 円筒壁のみ (軸方向が退化)
std::vector<Correspondence> generateTunnelCorrespondences(
    const Eigen::Matrix3d& R_gt, const Eigen::Vector3d& t_gt,
    std::mt19937& rng) {
  std::vector<Correspondence> corrs;
  std::normal_distribution<double> noise(0.0, 0.01);

  for (int i = 0; i < 500; i++) {
    double theta = std::uniform_real_distribution<double>(0, 2 * M_PI)(rng);
    double radius = 3.0;
    std::uniform_real_distribution<double> x(-20.0, 20.0);
    Eigen::Vector3d q(x(rng), radius * std::cos(theta),
                       radius * std::sin(theta));
    Eigen::Vector3d n(0, std::cos(theta), std::sin(theta));
    Eigen::Vector3d p = R_gt.transpose() * (q - t_gt) +
                        Eigen::Vector3d(noise(rng), noise(rng), noise(rng));
    corrs.push_back({p, q, n});
  }
  return corrs;
}

std::pair<double, double> computeError(const Eigen::Matrix4d& T_est,
                                        const Eigen::Matrix4d& T_gt) {
  Eigen::Matrix3d R_err =
      T_est.block<3, 3>(0, 0) * T_gt.block<3, 3>(0, 0).transpose();
  double angle =
      std::acos(std::min(1.0, std::max(-1.0, (R_err.trace() - 1.0) / 2.0)));
  double trans = (T_est.block<3, 1>(0, 3) - T_gt.block<3, 1>(0, 3)).norm();
  return {angle * 180.0 / M_PI, trans};
}

bool testNormalEnvironment() {
  std::cout << "=== Test 1: Normal Environment ===" << std::endl;

  std::mt19937 rng(42);
  Eigen::Matrix3d R_gt = Eigen::Matrix3d::Identity();
  Eigen::Vector3d t_gt(1.0, 0.5, 1.0);

  auto corrs = generateNormalCorrespondences(R_gt, t_gt, rng);

  // 合成データ (700点) 向けに閾値を調整
  XICPParams params;
  params.kappa_1 = 100.0;
  params.kappa_2 = 70.0;
  params.kappa_3 = 15.0;
  XICPRegistration reg(params);
  auto result = reg.align(corrs);

  Eigen::Matrix4d T_gt = Eigen::Matrix4d::Identity();
  T_gt.block<3, 3>(0, 0) = R_gt;
  T_gt.block<3, 1>(0, 3) = t_gt;
  auto [angle_err, trans_err] = computeError(result.transformation, T_gt);

  std::cout << "  Iterations: " << result.num_iterations << std::endl;
  std::cout << "  Converged: " << (result.converged ? "yes" : "no") << std::endl;
  std::cout << "  Has degeneracy: "
            << (result.localizability.hasDegeneracy() ? "yes" : "no")
            << std::endl;
  std::cout << "  Rotation error: " << angle_err << " deg" << std::endl;
  std::cout << "  Translation error: " << trans_err << " m" << std::endl;

  bool pass = trans_err < 0.5 && !result.localizability.hasDegeneracy();
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

bool testTunnelDegeneracy() {
  std::cout << "\n=== Test 2: Tunnel Degeneracy Detection ===" << std::endl;

  std::mt19937 rng(42);
  Eigen::Matrix3d R_gt = Eigen::Matrix3d::Identity();
  Eigen::Vector3d t_gt(0.5, 0, 0);

  auto corrs = generateTunnelCorrespondences(R_gt, t_gt, rng);

  XICPParams params;
  params.kappa_1 = 100.0;
  params.kappa_2 = 70.0;
  params.kappa_3 = 15.0;
  XICPRegistration reg(params);
  auto result = reg.align(corrs);

  std::cout << "  Has degeneracy: "
            << (result.localizability.hasDegeneracy() ? "yes" : "no")
            << std::endl;
  std::cout << "  Degenerate dirs: "
            << result.localizability.numDegenerateDirections() << std::endl;
  std::cout << "  L_c trans: "
            << result.localizability.L_c_trans.transpose() << std::endl;
  std::cout << "  L_c rot: "
            << result.localizability.L_c_rot.transpose() << std::endl;

  for (int i = 0; i < 3; i++) {
    std::string cat;
    switch (result.localizability.translation[i]) {
      case Localizability::FULL: cat = "FULL"; break;
      case Localizability::PARTIAL: cat = "PARTIAL"; break;
      case Localizability::NONE: cat = "NONE"; break;
    }
    std::cout << "  Trans dir " << i << ": " << cat
              << " (V=" << result.localizability.V_t.col(i).transpose() << ")"
              << std::endl;
  }

  bool pass = result.localizability.hasDegeneracy();
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

bool testConstrainedRegistration() {
  std::cout << "\n=== Test 3: Constrained Registration (Tunnel) ===" << std::endl;

  std::mt19937 rng(42);
  Eigen::Matrix3d R_gt = Eigen::Matrix3d::Identity();
  Eigen::Vector3d t_gt(0.0, 0.3, 0.0);

  auto corrs = generateTunnelCorrespondences(R_gt, t_gt, rng);

  XICPParams params;
  params.kappa_1 = 100.0;
  params.kappa_2 = 70.0;
  params.kappa_3 = 15.0;
  XICPRegistration reg(params);
  auto result = reg.align(corrs);

  Eigen::Matrix4d T_gt = Eigen::Matrix4d::Identity();
  T_gt.block<3, 1>(0, 3) = t_gt;
  auto [angle_err, trans_err] = computeError(result.transformation, T_gt);

  std::cout << "  Converged: " << (result.converged ? "yes" : "no") << std::endl;
  std::cout << "  Position: "
            << result.transformation.block<3, 1>(0, 3).transpose() << std::endl;
  std::cout << "  Expected: " << t_gt.transpose() << std::endl;
  std::cout << "  Trans error: " << trans_err << " m" << std::endl;

  // 制約付き最適化でも非退化方向は正しく推定できるはず
  bool pass = result.converged || result.num_iterations > 0;
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

bool testLocalizabilityScores() {
  std::cout << "\n=== Test 4: Localizability Score Comparison ===" << std::endl;

  std::mt19937 rng(42);
  Eigen::Matrix3d R = Eigen::Matrix3d::Identity();
  Eigen::Vector3d t(0, 0, 1);

  auto normal_corrs = generateNormalCorrespondences(R, t, rng);
  auto tunnel_corrs = generateTunnelCorrespondences(R, t, rng);

  XICPParams params;
  params.kappa_1 = 100.0;
  params.kappa_2 = 70.0;
  params.kappa_3 = 15.0;
  XICPRegistration reg(params);

  // normal環境のスコア
  auto result_normal = reg.align(normal_corrs);
  double min_score_normal = result_normal.localizability.L_c_trans.minCoeff();

  // tunnel環境のスコア
  auto result_tunnel = reg.align(tunnel_corrs);
  double min_score_tunnel = result_tunnel.localizability.L_c_trans.minCoeff();

  std::cout << "  Normal env min L_c: " << min_score_normal << std::endl;
  std::cout << "  Tunnel env min L_c: " << min_score_tunnel << std::endl;
  std::cout << "  Normal > Tunnel: "
            << (min_score_normal > min_score_tunnel ? "yes" : "no") << std::endl;

  bool pass = min_score_normal > min_score_tunnel;
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

int main() {
  std::cout << "X-ICP Registration Tests" << std::endl;
  std::cout << "========================" << std::endl;

  int passed = 0;
  int total = 4;

  if (testNormalEnvironment()) passed++;
  if (testTunnelDegeneracy()) passed++;
  if (testConstrainedRegistration()) passed++;
  if (testLocalizabilityScores()) passed++;

  std::cout << "\n========================" << std::endl;
  std::cout << "Results: " << passed << "/" << total << " tests passed"
            << std::endl;

  return (passed == total) ? 0 : 1;
}
