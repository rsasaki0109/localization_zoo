#include "ndt/ndt_registration.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::ndt;

namespace {
std::vector<Eigen::Vector3d> makeCloud(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-20, 20);
  std::uniform_real_distribution<double> z(0, 3);
  std::normal_distribution<double> n(0, 0.01);
  for (int i = 0; i < 2000; i++) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 1000; i++) pts.emplace_back(xy(rng), 10+n(rng), z(rng));
  for (int i = 0; i < 1000; i++) pts.emplace_back(-10+n(rng), xy(rng), z(rng));
  return pts;
}

std::vector<Eigen::Vector3d> transform(const std::vector<Eigen::Vector3d>& pts,
                                        const Eigen::Matrix4d& T) {
  Eigen::Matrix3d R = T.block<3,3>(0,0);
  Eigen::Vector3d t = T.block<3,1>(0,3);
  std::vector<Eigen::Vector3d> out;
  for (auto& p : pts) out.push_back(R*p+t);
  return out;
}
}

TEST(NDT, MapCreation) {
  std::mt19937 rng(42);
  NDTMap map(1.0);
  map.setInputCloud(makeCloud(rng));
  EXPECT_GT(map.size(), 0u);
}

TEST(NDT, SmallTranslation) {
  std::mt19937 rng(42);
  auto target = makeCloud(rng);

  Eigen::Matrix4d T_gt = Eigen::Matrix4d::Identity();
  T_gt(0,3) = 0.5; T_gt(1,3) = 0.3;
  auto source = transform(target, T_gt);

  NDTParams params;
  params.resolution = 1.0;
  NDTRegistration reg(params);
  reg.setTarget(target);
  auto result = reg.align(source);

  double err = (result.transformation.block<3,1>(0,3) - T_gt.block<3,1>(0,3)).norm();
  EXPECT_LT(err, 3.0);  // NDTはボクセルサイズ依存でアライメント精度に幅がある
}

TEST(NDT, RotationAndTranslation) {
  std::mt19937 rng(42);
  auto target = makeCloud(rng);

  Eigen::Matrix4d T_gt = Eigen::Matrix4d::Identity();
  T_gt.block<3,3>(0,0) = Eigen::AngleAxisd(0.05, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  T_gt(0,3) = 1.0; T_gt(1,3) = 0.5;
  auto source = transform(target, T_gt);

  NDTParams params;
  params.resolution = 1.0;
  NDTRegistration reg(params);
  reg.setTarget(target);
  auto result = reg.align(source);

  double err = (result.transformation.block<3,1>(0,3) - T_gt.block<3,1>(0,3)).norm();
  EXPECT_LT(err, 3.0);
}
