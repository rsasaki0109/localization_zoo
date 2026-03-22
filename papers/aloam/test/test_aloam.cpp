#include "aloam/scan_registration.h"
#include "aloam/laser_odometry.h"
#include "aloam/laser_mapping.h"

#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::aloam;

namespace {

PointCloudPtr generateLidarScan(const Eigen::Matrix4d& pose, std::mt19937& rng) {
  auto cloud = PointCloudPtr(new PointCloud);
  std::normal_distribution<float> noise(0.0f, 0.01f);
  std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * M_PI);
  Eigen::Matrix3d R = pose.block<3, 3>(0, 0);
  Eigen::Vector3d t = pose.block<3, 1>(0, 3);

  auto add = [&](const Eigen::Vector3d& pw) {
    Eigen::Vector3d ps = R.transpose() * (pw - t);
    PointT p;
    p.x = ps.x() + noise(rng);
    p.y = ps.y() + noise(rng);
    p.z = ps.z() + noise(rng);
    p.intensity = 0;
    cloud->push_back(p);
  };

  std::uniform_real_distribution<double> xy(-30.0, 30.0);
  std::uniform_real_distribution<double> z(0.0, 4.0);
  for (int i = 0; i < 3000; i++) add({xy(rng), xy(rng), 0.0});
  for (int i = 0; i < 1000; i++) add({xy(rng), 15.0, z(rng)});
  for (int i = 0; i < 1000; i++) add({-15.0, xy(rng), z(rng)});
  for (int i = 0; i < 500; i++) {
    float th = angle_dist(rng);
    add({5.0 + 0.3 * std::cos(th), 5.0 + 0.3 * std::sin(th), z(rng)});
  }
  return cloud;
}

}  // namespace

TEST(ALOAM, ScanRegistration) {
  std::mt19937 rng(42);
  auto cloud = generateLidarScan(Eigen::Matrix4d::Identity(), rng);

  ScanRegistration reg;
  auto features = reg.extract(cloud);

  EXPECT_GT(features.corner_sharp->size(), 0u);
  EXPECT_GT(features.surf_flat->size(), 0u);
  EXPECT_GT(features.full_cloud->size(), 0u);
}

TEST(ALOAM, LaserOdometry) {
  std::mt19937 rng(42);
  ScanRegistration scan_reg;
  LaserOdometry odom;
  OdometryResult last;

  for (int f = 0; f < 5; f++) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = f * 1.0;
    auto cloud = generateLidarScan(pose, rng);
    auto features = scan_reg.extract(cloud);
    last = odom.process(features);
  }

  EXPECT_TRUE(last.valid);
  double err = (last.t_w_curr - Eigen::Vector3d(4.0, 0, 0)).norm();
  EXPECT_LT(err, 5.0);
}

TEST(ALOAM, FullPipeline) {
  std::mt19937 rng(42);
  ScanRegistration scan_reg;
  LaserOdometry odom;
  LaserMapping mapping;
  MappingResult last_map;

  for (int f = 0; f < 5; f++) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = f * 1.0;
    auto cloud = generateLidarScan(pose, rng);
    auto features = scan_reg.extract(cloud);
    auto odom_result = odom.process(features);
    if (!odom_result.valid) continue;
    last_map = mapping.process(features.corner_less_sharp, features.surf_less_flat,
                               features.full_cloud, odom_result.q_w_curr, odom_result.t_w_curr);
  }
  EXPECT_TRUE(last_map.valid);
}
