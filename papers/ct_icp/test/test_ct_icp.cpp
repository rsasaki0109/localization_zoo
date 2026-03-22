#include "ct_icp/ct_icp_registration.h"
#include "ct_icp/types.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cmath>
#include <iostream>
#include <random>

using namespace localization_zoo::ct_icp;

/// 地面+壁+柱のワールド点群を生成
std::vector<Eigen::Vector3d> generateEnvironment(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> points;
  std::normal_distribution<double> noise(0.0, 0.01);

  // 地面 z=0
  for (int i = 0; i < 5000; i++) {
    std::uniform_real_distribution<double> xy(-30.0, 30.0);
    points.emplace_back(xy(rng), xy(rng), noise(rng));
  }
  // 壁 y=15
  for (int i = 0; i < 2000; i++) {
    std::uniform_real_distribution<double> x(-30.0, 30.0);
    std::uniform_real_distribution<double> z(0.0, 4.0);
    points.emplace_back(x(rng), 15.0 + noise(rng), z(rng));
  }
  // 壁 x=-15
  for (int i = 0; i < 2000; i++) {
    std::uniform_real_distribution<double> y(-30.0, 30.0);
    std::uniform_real_distribution<double> z(0.0, 4.0);
    points.emplace_back(-15.0 + noise(rng), y(rng), z(rng));
  }

  return points;
}

/// ポーズからLiDARスキャンをシミュレート (タイムスタンプ付き)
/// begin_poseとend_poseの間を補間してモーション歪みを含む点群を生成
std::vector<TimedPoint> simulateScan(const std::vector<Eigen::Vector3d>& env,
                                      const TrajectoryFrame& frame,
                                      std::mt19937& rng,
                                      double max_range = 30.0) {
  std::vector<TimedPoint> timed_points;
  std::normal_distribution<double> noise(0.0, 0.02);

  // 環境点をランダムな順序で観測 (各点に時刻を割り当て)
  std::vector<int> indices(env.size());
  std::iota(indices.begin(), indices.end(), 0);
  std::shuffle(indices.begin(), indices.end(), rng);

  for (int idx : indices) {
    // ランダムなタイムスタンプ
    double t = std::uniform_real_distribution<double>(0.0, 1.0)(rng);
    double alpha = frame.getAlpha(t);
    SE3 pose = SE3::interpolate(frame.begin_pose, frame.end_pose, alpha);

    // ワールド→センサ座標
    SE3 pose_inv = pose.inverse();
    Eigen::Vector3d ps = pose_inv * env[idx];

    double range = ps.norm();
    if (range > max_range || range < 0.5) continue;

    TimedPoint tp;
    tp.raw_point = ps + Eigen::Vector3d(noise(rng), noise(rng), noise(rng));
    tp.timestamp = t;
    tp.alpha = alpha;
    timed_points.push_back(tp);

    if (timed_points.size() > 5000) break;
  }

  return timed_points;
}

std::pair<double, double> poseError(const SE3& est, const SE3& gt) {
  Eigen::Matrix3d R_err =
      est.quat.toRotationMatrix() * gt.quat.toRotationMatrix().transpose();
  double angle =
      std::acos(std::min(1.0, std::max(-1.0, (R_err.trace() - 1.0) / 2.0)));
  double trans = (est.trans - gt.trans).norm();
  return {angle * 180.0 / M_PI, trans};
}

bool testStaticRegistration() {
  std::cout << "=== Test 1: Static Registration ===" << std::endl;

  std::mt19937 rng(42);
  auto env = generateEnvironment(rng);

  // フレーム0: 原点
  TrajectoryFrame frame0;
  frame0.begin_pose.trans = Eigen::Vector3d(0, 0, 1);
  frame0.end_pose.trans = Eigen::Vector3d(0, 0, 1);

  CTICPParams params;
  params.voxel_resolution = 1.0;
  params.max_iterations = 20;
  params.planarity_threshold = 0.1;

  CTICPRegistration reg(params);

  // フレーム0の点をマップに追加
  auto scan0 = simulateScan(env, frame0, rng);
  std::vector<Eigen::Vector3d> world0;
  for (auto& tp : scan0) {
    world0.push_back(frame0.transformPoint(tp.raw_point, tp.timestamp));
  }
  reg.addPointsToMap(world0);

  // フレーム1: 1m前進
  TrajectoryFrame frame1_gt;
  frame1_gt.begin_pose.trans = Eigen::Vector3d(0, 0, 1);
  frame1_gt.end_pose.trans = Eigen::Vector3d(1, 0, 1);

  auto scan1 = simulateScan(env, frame1_gt, rng);

  // 初期推定 = 前フレームのポーズをそのまま
  TrajectoryFrame frame1_init;
  frame1_init.begin_pose = frame0.end_pose;
  frame1_init.end_pose = frame0.end_pose;

  auto result = reg.registerFrame(scan1, frame1_init, &frame0);

  auto [angle_err, trans_err] = poseError(result.frame.end_pose, frame1_gt.end_pose);

  std::cout << "  Iterations: " << result.num_iterations << std::endl;
  std::cout << "  Converged: " << (result.converged ? "yes" : "no")
            << std::endl;
  std::cout << "  End pose translation: "
            << result.frame.end_pose.trans.transpose() << std::endl;
  std::cout << "  Expected: " << frame1_gt.end_pose.trans.transpose()
            << std::endl;
  std::cout << "  Rotation error: " << angle_err << " deg" << std::endl;
  std::cout << "  Translation error: " << trans_err << " m" << std::endl;

  bool pass = trans_err < 2.0 && angle_err < 5.0;
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

bool testContinuousTimeMotion() {
  std::cout << "\n=== Test 2: Continuous-Time Motion Compensation ===" << std::endl;

  std::mt19937 rng(123);
  auto env = generateEnvironment(rng);

  CTICPParams params;
  params.voxel_resolution = 1.0;
  params.max_iterations = 20;
  params.planarity_threshold = 0.1;

  CTICPRegistration reg(params);

  // フレーム0: 静止
  TrajectoryFrame frame0;
  frame0.begin_pose.trans = Eigen::Vector3d(0, 0, 1);
  frame0.end_pose.trans = Eigen::Vector3d(0, 0, 1);

  auto scan0 = simulateScan(env, frame0, rng);
  std::vector<Eigen::Vector3d> world0;
  for (auto& tp : scan0) {
    world0.push_back(frame0.transformPoint(tp.raw_point, tp.timestamp));
  }
  reg.addPointsToMap(world0);

  // フレーム1: 回転しながら前進 (begin→endで変化)
  TrajectoryFrame frame1_gt;
  frame1_gt.begin_pose.trans = Eigen::Vector3d(0, 0, 1);
  frame1_gt.end_pose.trans = Eigen::Vector3d(2, 0.5, 1);
  double yaw = 3.0 * M_PI / 180.0;
  frame1_gt.end_pose.quat =
      Eigen::Quaterniond(Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()));

  auto scan1 = simulateScan(env, frame1_gt, rng);

  // 初期推定: 前フレームの速度から等速仮定
  TrajectoryFrame frame1_init;
  frame1_init.begin_pose = frame0.end_pose;
  frame1_init.end_pose = frame0.end_pose;

  auto result = reg.registerFrame(scan1, frame1_init, &frame0);

  auto [angle_err_begin, trans_err_begin] =
      poseError(result.frame.begin_pose, frame1_gt.begin_pose);
  auto [angle_err_end, trans_err_end] =
      poseError(result.frame.end_pose, frame1_gt.end_pose);

  std::cout << "  Iterations: " << result.num_iterations << std::endl;
  std::cout << "  Begin pose error: rot=" << angle_err_begin
            << "deg, trans=" << trans_err_begin << "m" << std::endl;
  std::cout << "  End pose error: rot=" << angle_err_end
            << "deg, trans=" << trans_err_end << "m" << std::endl;

  // CT-ICPなので begin と end 両方が推定される
  bool pass = trans_err_end < 3.0 && angle_err_end < 10.0;
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

bool testVoxelMap() {
  std::cout << "\n=== Test 3: Voxel Map ===" << std::endl;

  CTICPParams params;
  params.voxel_resolution = 1.0;
  CTICPRegistration reg(params);

  std::vector<Eigen::Vector3d> points;
  for (int i = 0; i < 1000; i++) {
    points.emplace_back(i * 0.1, 0, 0);
  }
  reg.addPointsToMap(points);

  std::cout << "  Points: 1000" << std::endl;
  std::cout << "  Voxels: " << reg.mapSize() << std::endl;

  bool pass = reg.mapSize() > 0 && reg.mapSize() < 1000;
  std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << std::endl;
  return pass;
}

int main() {
  std::cout << "CT-ICP Registration Tests" << std::endl;
  std::cout << "=========================" << std::endl;

  int passed = 0;
  int total = 3;

  if (testStaticRegistration()) passed++;
  if (testContinuousTimeMotion()) passed++;
  if (testVoxelMap()) passed++;

  std::cout << "\n=========================" << std::endl;
  std::cout << "Results: " << passed << "/" << total << " tests passed"
            << std::endl;

  return (passed == total) ? 0 : 1;
}
