#include "voxel_gicp/voxel_gicp_registration.h"

#include <gtest/gtest.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <algorithm>
#include <cmath>
#include <random>

using namespace localization_zoo::voxel_gicp;

namespace {

std::vector<Eigen::Vector3d> makeCloud(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> points;
  std::uniform_real_distribution<double> xy(-25.0, 25.0);
  std::uniform_real_distribution<double> z(0.0, 4.0);
  std::normal_distribution<double> noise(0.0, 0.01);

  for (int i = 0; i < 3000; i++) points.emplace_back(xy(rng), xy(rng), noise(rng));
  for (int i = 0; i < 1200; i++) points.emplace_back(xy(rng), 14.0 + noise(rng), z(rng));
  for (int i = 0; i < 1200; i++) points.emplace_back(-14.0 + noise(rng), xy(rng), z(rng));
  return points;
}

std::vector<Eigen::Vector3d> transform(const std::vector<Eigen::Vector3d>& pts,
                                       const Eigen::Matrix4d& T) {
  Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  Eigen::Vector3d t = T.block<3, 1>(0, 3);
  std::vector<Eigen::Vector3d> out(pts.size());
  for (size_t i = 0; i < pts.size(); i++) out[i] = R * pts[i] + t;
  return out;
}

std::pair<double, double> poseError(const Eigen::Matrix4d& est,
                                    const Eigen::Matrix4d& gt) {
  const Eigen::Matrix3d Re =
      est.block<3, 3>(0, 0) * gt.block<3, 3>(0, 0).transpose();
  const double angle =
      std::acos(std::clamp((Re.trace() - 1.0) / 2.0, -1.0, 1.0)) * 180.0 / M_PI;
  const double translation =
      (est.block<3, 1>(0, 3) - gt.block<3, 1>(0, 3)).norm();
  return {angle, translation};
}

}  // namespace

TEST(VoxelGICP, TargetSetup) {
  std::mt19937 rng(42);
  VoxelGICPRegistration reg;
  reg.setTarget(makeCloud(rng));
  EXPECT_GT(reg.targetSize(), 0u);
}

TEST(VoxelGICP, SmallTranslation) {
  std::mt19937 rng(42);
  const auto target = makeCloud(rng);

  Eigen::Matrix4d T_gt = Eigen::Matrix4d::Identity();
  T_gt(0, 3) = 0.6;
  T_gt(1, 3) = -0.2;
  auto source = transform(target, T_gt);

  VoxelGICPParams params;
  params.voxel_resolution = 1.0;
  params.max_iterations = 20;
  VoxelGICPRegistration reg(params);
  reg.setTarget(target);

  const auto result = reg.align(source);
  const auto [angle_err, trans_err] =
      poseError(result.transformation, T_gt.inverse());

  EXPECT_LT(angle_err, 1.0);
  EXPECT_LT(trans_err, 0.5);
  EXPECT_GT(result.num_correspondences, 50);
}

TEST(VoxelGICP, RotationAndTranslation) {
  std::mt19937 rng(7);
  const auto target = makeCloud(rng);

  Eigen::Matrix4d T_gt = Eigen::Matrix4d::Identity();
  T_gt.block<3, 3>(0, 0) =
      Eigen::AngleAxisd(4.0 * M_PI / 180.0, Eigen::Vector3d::UnitZ())
          .toRotationMatrix();
  T_gt(0, 3) = 1.0;
  T_gt(1, 3) = 0.5;
  auto source = transform(target, T_gt);

  VoxelGICPParams params;
  params.voxel_resolution = 1.0;
  params.max_iterations = 25;
  params.max_correspondence_distance = 5.0;
  VoxelGICPRegistration reg(params);
  reg.setTarget(target);

  const auto result = reg.align(source);
  const auto [angle_err, trans_err] =
      poseError(result.transformation, T_gt.inverse());

  EXPECT_LT(angle_err, 1.5);
  EXPECT_LT(trans_err, 0.8);
  EXPECT_GT(result.num_correspondences, 50);
}
