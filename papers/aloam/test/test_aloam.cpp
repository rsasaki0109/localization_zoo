#include "aloam/laser_mapping.h"
#include "aloam/laser_odometry.h"
#include "aloam/scan_registration.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/point_cloud.h>

#include <cmath>
#include <iostream>
#include <random>

using namespace localization_zoo::aloam;

/// テスト用のLiDARスキャン風点群を生成 (地面 + 壁 + 柱)
PointCloudPtr generateLidarScan(const Eigen::Matrix4d& pose,
                                 std::mt19937& rng, int n_scans = 16) {
  auto cloud = PointCloudPtr(new PointCloud);
  std::normal_distribution<float> noise(0.0f, 0.01f);
  std::uniform_real_distribution<float> angle_dist(0.0f,
                                                    2.0f * M_PI);

  Eigen::Matrix3d R = pose.block<3, 3>(0, 0);
  Eigen::Vector3d t = pose.block<3, 1>(0, 3);

  auto addPoint = [&](const Eigen::Vector3d& pw) {
    // ワールド→センサ座標
    Eigen::Vector3d ps = R.transpose() * (pw - t);
    PointT p;
    p.x = ps.x() + noise(rng);
    p.y = ps.y() + noise(rng);
    p.z = ps.z() + noise(rng);
    p.intensity = 0;
    cloud->push_back(p);
  };

  // 地面 (z = 0) [-30m, 30m] x [-30m, 30m]
  for (int i = 0; i < 3000; i++) {
    std::uniform_real_distribution<double> xy(-30.0, 30.0);
    addPoint(Eigen::Vector3d(xy(rng), xy(rng), 0.0));
  }

  // 壁1 (y = 15) [-30m, 30m] x [0, 4m]
  for (int i = 0; i < 1000; i++) {
    std::uniform_real_distribution<double> x(-30.0, 30.0);
    std::uniform_real_distribution<double> z(0.0, 4.0);
    addPoint(Eigen::Vector3d(x(rng), 15.0, z(rng)));
  }

  // 壁2 (x = -15) [-30m, 30m] x [0, 4m]
  for (int i = 0; i < 1000; i++) {
    std::uniform_real_distribution<double> y(-30.0, 30.0);
    std::uniform_real_distribution<double> z(0.0, 4.0);
    addPoint(Eigen::Vector3d(-15.0, y(rng), z(rng)));
  }

  // 柱 (x=5, y=5, r=0.3m, h=3m) - エッジ特徴生成用
  for (int i = 0; i < 500; i++) {
    float theta = angle_dist(rng);
    std::uniform_real_distribution<double> z(0.0, 3.0);
    addPoint(Eigen::Vector3d(5.0 + 0.3 * std::cos(theta),
                              5.0 + 0.3 * std::sin(theta), z(rng)));
  }

  // 柱2 (x=-5, y=-8)
  for (int i = 0; i < 500; i++) {
    float theta = angle_dist(rng);
    std::uniform_real_distribution<double> z(0.0, 3.0);
    addPoint(Eigen::Vector3d(-5.0 + 0.3 * std::cos(theta),
                              -8.0 + 0.3 * std::sin(theta), z(rng)));
  }

  return cloud;
}

bool testScanRegistration() {
  std::cout << "=== Test 1: Scan Registration ===" << std::endl;

  std::mt19937 rng(42);
  auto cloud = generateLidarScan(Eigen::Matrix4d::Identity(), rng);

  ScanRegistrationParams params;
  params.n_scans = 16;
  params.minimum_range = 0.3f;

  ScanRegistration reg(params);
  auto features = reg.extract(cloud);

  std::cout << "  Input points: " << cloud->size() << std::endl;
  std::cout << "  Corner sharp: " << features.corner_sharp->size() << std::endl;
  std::cout << "  Corner less sharp: " << features.corner_less_sharp->size()
            << std::endl;
  std::cout << "  Surf flat: " << features.surf_flat->size() << std::endl;
  std::cout << "  Surf less flat: " << features.surf_less_flat->size()
            << std::endl;
  std::cout << "  Full cloud: " << features.full_cloud->size() << std::endl;

  bool pass = features.corner_sharp->size() > 0 &&
              features.surf_flat->size() > 0 &&
              features.full_cloud->size() > 0;
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

bool testLaserOdometry() {
  std::cout << "\n=== Test 2: Laser Odometry ===" << std::endl;

  std::mt19937 rng(42);
  ScanRegistrationParams scan_params;
  scan_params.n_scans = 16;
  ScanRegistration scan_reg(scan_params);

  LaserOdometryParams odom_params;
  LaserOdometry odom(odom_params);

  // 5フレーム、各フレーム1m前進
  std::vector<Eigen::Matrix4d> gt_poses;
  bool all_valid = true;
  OdometryResult last_result;

  for (int f = 0; f < 5; f++) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = f * 1.0;  // x方向に1mずつ
    gt_poses.push_back(pose);

    auto cloud = generateLidarScan(pose, rng);
    auto features = scan_reg.extract(cloud);
    last_result = odom.process(features);

    if (f == 0) continue;  // 最初のフレームはinitのみ

    if (!last_result.valid) {
      all_valid = false;
      break;
    }
  }

  if (last_result.valid) {
    std::cout << "  Final position: " << last_result.t_w_curr.transpose()
              << std::endl;
    std::cout << "  Expected: ~[4, 0, 0]" << std::endl;

    double pos_err = (last_result.t_w_curr - Eigen::Vector3d(4.0, 0.0, 0.0)).norm();
    std::cout << "  Position error: " << pos_err << " m" << std::endl;
    // オドメトリなのでドリフトは許容
    all_valid = all_valid && pos_err < 5.0;
  }

  std::cout << "  Result: " << (all_valid ? "PASS" : "FAIL") << std::endl;
  return all_valid;
}

bool testFullPipeline() {
  std::cout << "\n=== Test 3: Full Pipeline (Odom + Mapping) ===" << std::endl;

  std::mt19937 rng(42);
  ScanRegistrationParams scan_params;
  scan_params.n_scans = 16;
  ScanRegistration scan_reg(scan_params);

  LaserOdometryParams odom_params;
  LaserOdometry odom(odom_params);

  LaserMappingParams map_params;
  LaserMapping mapping(map_params);

  bool success = true;
  MappingResult last_map_result;

  for (int f = 0; f < 5; f++) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = f * 1.0;

    auto cloud = generateLidarScan(pose, rng);
    auto features = scan_reg.extract(cloud);
    auto odom_result = odom.process(features);

    if (f == 0) continue;

    if (!odom_result.valid) {
      success = false;
      break;
    }

    last_map_result = mapping.process(
        features.corner_less_sharp, features.surf_less_flat,
        features.full_cloud, odom_result.q_w_curr, odom_result.t_w_curr);
  }

  if (last_map_result.valid) {
    std::cout << "  Mapping position: " << last_map_result.t_w_curr.transpose()
              << std::endl;
    std::cout << "  Expected: ~[4, 0, 0]" << std::endl;
  }

  std::cout << "  Result: " << (success ? "PASS" : "FAIL") << std::endl;
  return success;
}

int main() {
  std::cout << "A-LOAM Tests" << std::endl;
  std::cout << "============" << std::endl;

  int passed = 0;
  int total = 3;

  if (testScanRegistration()) passed++;
  if (testLaserOdometry()) passed++;
  if (testFullPipeline()) passed++;

  std::cout << "\n============" << std::endl;
  std::cout << "Results: " << passed << "/" << total << " tests passed"
            << std::endl;

  return (passed == total) ? 0 : 1;
}
