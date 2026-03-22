#include "litamin2/litamin2_registration.h"

#include <gtest/gtest.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <random>

using namespace localization_zoo::litamin2;

namespace {

std::vector<Eigen::Vector3d> generateTestCloud(int n, std::mt19937& rng) {
  std::vector<Eigen::Vector3d> points;
  std::uniform_real_distribution<double> xy(-50.0, 50.0);
  std::uniform_real_distribution<double> z_dist(0.0, 5.0);
  std::normal_distribution<double> noise(0.0, 0.02);

  for (int i = 0; i < n / 2; i++)
    points.emplace_back(xy(rng), xy(rng), noise(rng));
  for (int i = 0; i < n / 4; i++)
    points.emplace_back(xy(rng), 10.0 + noise(rng), z_dist(rng));
  for (int i = 0; i < n / 4; i++)
    points.emplace_back(-10.0 + noise(rng), xy(rng), z_dist(rng));
  return points;
}

std::vector<Eigen::Vector3d> transform(const std::vector<Eigen::Vector3d>& pts,
                                        const Eigen::Matrix4d& T) {
  Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  Eigen::Vector3d t = T.block<3, 1>(0, 3);
  std::vector<Eigen::Vector3d> out;
  for (auto& p : pts) out.push_back(R * p + t);
  return out;
}

std::pair<double, double> poseError(const Eigen::Matrix4d& est,
                                     const Eigen::Matrix4d& gt) {
  Eigen::Matrix3d Re = est.block<3, 3>(0, 0) * gt.block<3, 3>(0, 0).transpose();
  double a = std::acos(std::clamp((Re.trace() - 1.0) / 2.0, -1.0, 1.0));
  double t = (est.block<3, 1>(0, 3) - gt.block<3, 1>(0, 3)).norm();
  return {a * 180.0 / M_PI, t};
}

}  // namespace

TEST(LiTAMIN2, SmallTranslation) {
  std::mt19937 rng(42);
  auto target = generateTestCloud(10000, rng);

  Eigen::Matrix4d T_gt = Eigen::Matrix4d::Identity();
  T_gt(0, 3) = 1.0; T_gt(1, 3) = 0.5; T_gt(2, 3) = 0.1;
  auto source = transform(target, T_gt);

  LiTAMIN2Params params;
  params.voxel_resolution = 3.0;
  params.use_cov_cost = true;

  LiTAMIN2Registration reg(params);
  reg.setTarget(target);
  auto result = reg.align(source);

  auto [angle_err, trans_err] = poseError(result.transformation, T_gt.inverse());
  EXPECT_LT(angle_err, 1.0);
  EXPECT_LT(trans_err, 0.5);
  EXPECT_TRUE(result.converged);
}

TEST(LiTAMIN2, RotationAndTranslation) {
  std::mt19937 rng(123);
  auto target = generateTestCloud(10000, rng);

  Eigen::Matrix4d T_gt = Eigen::Matrix4d::Identity();
  double yaw = 5.0 * M_PI / 180.0;
  T_gt.block<3, 3>(0, 0) = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  T_gt(0, 3) = 2.0; T_gt(1, 3) = 1.0;
  auto source = transform(target, T_gt);

  LiTAMIN2Params params;
  params.voxel_resolution = 3.0;
  params.use_cov_cost = true;

  LiTAMIN2Registration reg(params);
  reg.setTarget(target);
  auto result = reg.align(source);

  auto [angle_err, trans_err] = poseError(result.transformation, T_gt.inverse());
  EXPECT_LT(angle_err, 2.0);
  EXPECT_LT(trans_err, 1.0);
}

TEST(LiTAMIN2, VoxelReduction) {
  std::mt19937 rng(42);
  int n = 50000;
  auto points = generateTestCloud(n, rng);

  GaussianVoxelMap vmap(3.0, 3);
  vmap.createFromPoints(points);

  EXPECT_GT(vmap.size(), 0u);
  EXPECT_LT(vmap.size(), static_cast<size_t>(n));
  double reduction = 100.0 * (1.0 - static_cast<double>(vmap.size()) / n);
  EXPECT_GT(reduction, 80.0);
}
