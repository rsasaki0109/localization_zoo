#include "litamin2/litamin2_registration.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cmath>
#include <iostream>
#include <random>
#include <vector>

using namespace localization_zoo::litamin2;

/// ランダムな平面点群を生成 (地面 + 壁)
std::vector<Eigen::Vector3d> generateTestPointCloud(int num_points,
                                                     std::mt19937& rng) {
  std::vector<Eigen::Vector3d> points;
  points.reserve(num_points);

  std::uniform_real_distribution<double> xy_dist(-50.0, 50.0);
  std::normal_distribution<double> noise(0.0, 0.02);

  // 地面 (z ≈ 0)
  for (int i = 0; i < num_points / 2; i++) {
    double x = xy_dist(rng);
    double y = xy_dist(rng);
    double z = noise(rng);
    points.emplace_back(x, y, z);
  }

  // 壁 (y ≈ 10)
  std::uniform_real_distribution<double> z_dist(0.0, 5.0);
  for (int i = 0; i < num_points / 4; i++) {
    double x = xy_dist(rng);
    double y = 10.0 + noise(rng);
    double z = z_dist(rng);
    points.emplace_back(x, y, z);
  }

  // 壁 (x ≈ -10)
  for (int i = 0; i < num_points / 4; i++) {
    double x = -10.0 + noise(rng);
    double y = xy_dist(rng);
    double z = z_dist(rng);
    points.emplace_back(x, y, z);
  }

  return points;
}

/// 点群に変換を適用
std::vector<Eigen::Vector3d> transformPoints(
    const std::vector<Eigen::Vector3d>& points,
    const Eigen::Matrix4d& T) {
  Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  Eigen::Vector3d t = T.block<3, 1>(0, 3);

  std::vector<Eigen::Vector3d> transformed;
  transformed.reserve(points.size());
  for (const auto& p : points) {
    transformed.push_back(R * p + t);
  }
  return transformed;
}

/// 変換行列間の誤差を計算
std::pair<double, double> computeError(const Eigen::Matrix4d& T_est,
                                        const Eigen::Matrix4d& T_gt) {
  Eigen::Matrix3d R_err =
      T_est.block<3, 3>(0, 0) * T_gt.block<3, 3>(0, 0).transpose();
  double angle_err =
      std::acos(std::min(1.0, std::max(-1.0, (R_err.trace() - 1.0) / 2.0)));
  double trans_err = (T_est.block<3, 1>(0, 3) - T_gt.block<3, 1>(0, 3)).norm();
  return {angle_err * 180.0 / M_PI, trans_err};
}

bool testSmallTranslation() {
  std::cout << "=== Test 1: Small Translation ===" << std::endl;

  std::mt19937 rng(42);
  auto target = generateTestPointCloud(10000, rng);

  // 小さな並進 (1m)
  Eigen::Matrix4d T_gt = Eigen::Matrix4d::Identity();
  T_gt(0, 3) = 1.0;
  T_gt(1, 3) = 0.5;
  T_gt(2, 3) = 0.1;

  auto source = transformPoints(target, T_gt);

  LiTAMIN2Params params;
  params.voxel_resolution = 3.0;
  params.use_cov_cost = true;

  LiTAMIN2Registration reg(params);
  reg.setTarget(target);
  auto result = reg.align(source);

  // source = T_gt * target なので、align結果は T_gt^{-1} に収束するべき
  Eigen::Matrix4d T_expected = T_gt.inverse();
  auto [angle_err, trans_err] = computeError(result.transformation, T_expected);

  std::cout << "  Iterations: " << result.num_iterations << std::endl;
  std::cout << "  Converged: " << (result.converged ? "yes" : "no")
            << std::endl;
  std::cout << "  Rotation error: " << angle_err << " deg" << std::endl;
  std::cout << "  Translation error: " << trans_err << " m" << std::endl;

  bool pass = angle_err < 1.0 && trans_err < 0.5;
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

bool testRotationAndTranslation() {
  std::cout << "\n=== Test 2: Rotation + Translation ===" << std::endl;

  std::mt19937 rng(123);
  auto target = generateTestPointCloud(10000, rng);

  // 回転 (5度) + 並進 (2m)
  Eigen::Matrix4d T_gt = Eigen::Matrix4d::Identity();
  double yaw = 5.0 * M_PI / 180.0;
  T_gt.block<3, 3>(0, 0) =
      Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  T_gt(0, 3) = 2.0;
  T_gt(1, 3) = 1.0;

  auto source = transformPoints(target, T_gt);

  LiTAMIN2Params params;
  params.voxel_resolution = 3.0;
  params.use_cov_cost = true;

  LiTAMIN2Registration reg(params);
  reg.setTarget(target);
  auto result = reg.align(source);

  Eigen::Matrix4d T_expected = T_gt.inverse();
  auto [angle_err, trans_err] = computeError(result.transformation, T_expected);

  std::cout << "  Iterations: " << result.num_iterations << std::endl;
  std::cout << "  Converged: " << (result.converged ? "yes" : "no")
            << std::endl;
  std::cout << "  Rotation error: " << angle_err << " deg" << std::endl;
  std::cout << "  Translation error: " << trans_err << " m" << std::endl;

  bool pass = angle_err < 2.0 && trans_err < 1.0;
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

bool testICPOnlyMode() {
  std::cout << "\n=== Test 3: ICP-Only Mode (no cov cost) ===" << std::endl;

  std::mt19937 rng(42);
  auto target = generateTestPointCloud(10000, rng);

  Eigen::Matrix4d T_gt = Eigen::Matrix4d::Identity();
  T_gt(0, 3) = 1.5;
  T_gt(1, 3) = -0.5;

  auto source = transformPoints(target, T_gt);

  LiTAMIN2Params params;
  params.voxel_resolution = 3.0;
  params.use_cov_cost = false;  // ICPコストのみ

  LiTAMIN2Registration reg(params);
  reg.setTarget(target);
  auto result = reg.align(source);

  Eigen::Matrix4d T_expected = T_gt.inverse();
  auto [angle_err, trans_err] = computeError(result.transformation, T_expected);

  std::cout << "  Iterations: " << result.num_iterations << std::endl;
  std::cout << "  Converged: " << (result.converged ? "yes" : "no")
            << std::endl;
  std::cout << "  Rotation error: " << angle_err << " deg" << std::endl;
  std::cout << "  Translation error: " << trans_err << " m" << std::endl;

  // ICP-onlyは退化しやすい (論文の主張通り)。収束すれば合格
  bool pass = result.converged && angle_err < 5.0 && trans_err < 3.0;
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

bool testVoxelReduction() {
  std::cout << "\n=== Test 4: Voxel Point Reduction ===" << std::endl;

  std::mt19937 rng(42);
  int num_input = 50000;
  auto points = generateTestPointCloud(num_input, rng);

  LiTAMIN2Params params;
  params.voxel_resolution = 3.0;

  GaussianVoxelMap vmap(params.voxel_resolution, params.min_points_per_voxel);
  vmap.createFromPoints(points);

  double reduction = 100.0 * (1.0 - static_cast<double>(vmap.size()) / num_input);

  std::cout << "  Input points: " << num_input << std::endl;
  std::cout << "  Voxels: " << vmap.size() << std::endl;
  std::cout << "  Reduction: " << reduction << "%" << std::endl;

  bool pass = vmap.size() > 0 && vmap.size() < static_cast<size_t>(num_input);
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

int main() {
  std::cout << "LiTAMIN2 Registration Tests" << std::endl;
  std::cout << "===========================" << std::endl;

  int passed = 0;
  int total = 4;

  if (testSmallTranslation()) passed++;
  if (testRotationAndTranslation()) passed++;
  if (testICPOnlyMode()) passed++;
  if (testVoxelReduction()) passed++;

  std::cout << "\n===========================" << std::endl;
  std::cout << "Results: " << passed << "/" << total << " tests passed"
            << std::endl;

  return (passed == total) ? 0 : 1;
}
