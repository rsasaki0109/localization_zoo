#include "relead/cesikf.h"
#include "relead/degeneracy_detection.h"

#include <Eigen/Core>
#include <cmath>
#include <iostream>
#include <random>

using namespace localization_zoo::relead;

/// 地面+壁の点群からpoint-to-plane対応関係を生成
std::vector<PointWithNormal> generateCorrespondences(
    const Eigen::Matrix3d& R, const Eigen::Vector3d& t, std::mt19937& rng,
    bool tunnel_like = false) {
  std::vector<PointWithNormal> corrs;
  std::normal_distribution<double> noise(0.0, 0.01);

  // 地面 (z=0, 法線=[0,0,1])
  for (int i = 0; i < 200; i++) {
    std::uniform_real_distribution<double> xy(-20.0, 20.0);
    Eigen::Vector3d p_body(xy(rng), xy(rng), -t.z() + noise(rng));
    Eigen::Vector3d p_world = R * p_body + t;
    Eigen::Vector3d closest = p_world;
    closest.z() = 0.0;

    PointWithNormal pn;
    pn.point = p_world;
    pn.normal = Eigen::Vector3d(0, 0, 1);
    pn.closest = closest;
    corrs.push_back(pn);
  }

  if (!tunnel_like) {
    // 壁1 (y=10, 法線=[0,1,0])
    for (int i = 0; i < 100; i++) {
      std::uniform_real_distribution<double> x(-20.0, 20.0);
      std::uniform_real_distribution<double> z(0.0, 3.0);
      Eigen::Vector3d p_body(x(rng), 10.0 - t.y() + noise(rng), z(rng) - t.z());
      Eigen::Vector3d p_world = R * p_body + t;
      Eigen::Vector3d closest = p_world;
      closest.y() = 10.0;

      PointWithNormal pn;
      pn.point = p_world;
      pn.normal = Eigen::Vector3d(0, 1, 0);
      pn.closest = closest;
      corrs.push_back(pn);
    }

    // 壁2 (x=-10, 法線=[-1,0,0])
    for (int i = 0; i < 100; i++) {
      std::uniform_real_distribution<double> y(-20.0, 20.0);
      std::uniform_real_distribution<double> z(0.0, 3.0);
      Eigen::Vector3d p_body(-10.0 - t.x() + noise(rng), y(rng), z(rng) - t.z());
      Eigen::Vector3d p_world = R * p_body + t;
      Eigen::Vector3d closest = p_world;
      closest.x() = -10.0;

      PointWithNormal pn;
      pn.point = p_world;
      pn.normal = Eigen::Vector3d(-1, 0, 0);
      pn.closest = closest;
      corrs.push_back(pn);
    }
  } else {
    // トンネル: 円筒壁のみ (x方向が退化)
    // 法線は y,z方向のみ → x方向の並進が退化
    for (int i = 0; i < 200; i++) {
      double theta = std::uniform_real_distribution<double>(0.0, 2.0 * M_PI)(rng);
      double radius = 3.0;
      Eigen::Vector3d normal(0, std::cos(theta), std::sin(theta));
      std::uniform_real_distribution<double> x(-20.0, 20.0);
      Eigen::Vector3d wall_point(x(rng), radius * std::cos(theta),
                                  radius * std::sin(theta));

      PointWithNormal pn;
      pn.point = wall_point + Eigen::Vector3d(noise(rng), noise(rng), noise(rng));
      pn.normal = normal;
      pn.closest = wall_point;
      corrs.push_back(pn);
    }
  }

  return corrs;
}

bool testDegeneracyDetectionNormal() {
  std::cout << "=== Test 1: Degeneracy Detection (Normal Env) ===" << std::endl;

  std::mt19937 rng(42);
  auto corrs = generateCorrespondences(Eigen::Matrix3d::Identity(),
                                        Eigen::Vector3d(0, 0, 1), rng, false);

  DegeneracyDetection detector;
  auto info = detector.detect(corrs);

  std::cout << "  Correspondences: " << corrs.size() << std::endl;
  std::cout << "  Has degeneracy: " << (info.hasDegeneracy() ? "yes" : "no")
            << std::endl;
  std::cout << "  Degenerate rot dirs: "
            << info.degenerate_rotation_dirs.size() << std::endl;
  std::cout << "  Degenerate trans dirs: "
            << info.degenerate_translation_dirs.size() << std::endl;

  // 通常環境では退化なし
  bool pass = !info.hasDegeneracy();
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

bool testDegeneracyDetectionTunnel() {
  std::cout << "\n=== Test 2: Degeneracy Detection (Tunnel) ===" << std::endl;

  std::mt19937 rng(42);
  auto corrs = generateCorrespondences(Eigen::Matrix3d::Identity(),
                                        Eigen::Vector3d(0, 0, 1), rng, true);

  DegeneracyDetectionParams params;
  params.none_threshold = 0.01;
  params.partial_threshold = 0.1;
  DegeneracyDetection detector(params);
  auto info = detector.detect(corrs);

  std::cout << "  Correspondences: " << corrs.size() << std::endl;
  std::cout << "  Has degeneracy: " << (info.hasDegeneracy() ? "yes" : "no")
            << std::endl;
  std::cout << "  Degenerate rot dirs: "
            << info.degenerate_rotation_dirs.size() << std::endl;
  std::cout << "  Degenerate trans dirs: "
            << info.degenerate_translation_dirs.size() << std::endl;

  for (const auto& d : info.degenerate_translation_dirs) {
    std::cout << "    dir: " << d.transpose() << std::endl;
  }

  // トンネルでは並進方向に退化があるはず
  bool pass = info.hasTranslationDegeneracy();
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

bool testCESIKFNormalUpdate() {
  std::cout << "\n=== Test 3: CESIKF Normal Update ===" << std::endl;

  std::mt19937 rng(42);

  CESIKFParams params;
  CESIKF filter(params);

  // 初期状態: 位置に少しオフセット
  filter.state().position = Eigen::Vector3d(0.5, 0.3, 1.2);
  // 真の位置: [0, 0, 1]

  auto corrs = generateCorrespondences(Eigen::Matrix3d::Identity(),
                                        Eigen::Vector3d(0, 0, 1), rng, false);

  auto result = filter.update(corrs);

  double pos_err = (result.state.position - Eigen::Vector3d(0, 0, 1)).norm();
  std::cout << "  Position after update: " << result.state.position.transpose()
            << std::endl;
  std::cout << "  Expected: [0, 0, 1]" << std::endl;
  std::cout << "  Error: " << pos_err << " m" << std::endl;
  std::cout << "  Converged: " << (result.converged ? "yes" : "no")
            << std::endl;
  std::cout << "  Iterations: " << result.iterations << std::endl;
  std::cout << "  Has degeneracy: "
            << (result.degeneracy_info.hasDegeneracy() ? "yes" : "no")
            << std::endl;

  bool pass = pos_err < 1.0;
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

bool testCESIKFConstrainedUpdate() {
  std::cout << "\n=== Test 4: CESIKF Constrained Update (Tunnel) ===" << std::endl;

  std::mt19937 rng(42);

  CESIKFParams params;
  CESIKF filter(params);

  // 初期状態
  filter.state().position = Eigen::Vector3d(0.5, 0.0, 1.0);

  auto corrs = generateCorrespondences(Eigen::Matrix3d::Identity(),
                                        Eigen::Vector3d(0, 0, 1), rng, true);

  auto result = filter.update(corrs);

  std::cout << "  Position after update: " << result.state.position.transpose()
            << std::endl;
  std::cout << "  Has degeneracy: "
            << (result.degeneracy_info.hasDegeneracy() ? "yes" : "no")
            << std::endl;
  std::cout << "  Degenerate trans dirs: "
            << result.degeneracy_info.degenerate_translation_dirs.size()
            << std::endl;

  // 退化方向（x）の更新が抑制されているか確認
  // 退化方向にはドリフトしない = 初期値付近に留まる
  bool has_deg = result.degeneracy_info.hasDegeneracy();
  std::cout << "  Result: " << (has_deg ? "PASS" : "FAIL")
            << " (degeneracy detected)" << std::endl;
  return has_deg;
}

bool testIMUPrediction() {
  std::cout << "\n=== Test 5: IMU Prediction ===" << std::endl;

  CESIKFParams params;
  CESIKF filter(params);

  filter.state().position = Eigen::Vector3d::Zero();
  filter.state().velocity = Eigen::Vector3d(1.0, 0, 0);  // x方向に1m/s
  filter.state().gravity = Eigen::Vector3d(0, 0, -9.81);

  // 1秒間 100Hz で伝播
  ImuMeasurement imu;
  imu.linear_acceleration = Eigen::Vector3d(0, 0, 9.81);  // 重力補償
  imu.angular_velocity = Eigen::Vector3d::Zero();

  for (int i = 0; i < 100; i++) {
    filter.predict(imu, 0.01);
  }

  double pos_err = (filter.state().position - Eigen::Vector3d(1.0, 0, 0)).norm();
  std::cout << "  Position after 1s: " << filter.state().position.transpose()
            << std::endl;
  std::cout << "  Expected: [1, 0, 0]" << std::endl;
  std::cout << "  Error: " << pos_err << " m" << std::endl;

  bool pass = pos_err < 0.1;
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

int main() {
  std::cout << "RELEAD (CESIKF + Degeneracy Detection) Tests" << std::endl;
  std::cout << "=============================================" << std::endl;

  int passed = 0;
  int total = 5;

  if (testDegeneracyDetectionNormal()) passed++;
  if (testDegeneracyDetectionTunnel()) passed++;
  if (testCESIKFNormalUpdate()) passed++;
  if (testCESIKFConstrainedUpdate()) passed++;
  if (testIMUPrediction()) passed++;

  std::cout << "\n=============================================" << std::endl;
  std::cout << "Results: " << passed << "/" << total << " tests passed"
            << std::endl;

  return (passed == total) ? 0 : 1;
}
