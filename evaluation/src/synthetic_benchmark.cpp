/// 自己完結型ベンチマーク: 合成データで全手法を比較
///
/// 外部データ不要。ビルド後すぐ実行可能:
///   ./synthetic_benchmark

#include "litamin2/litamin2_registration.h"
#include "aloam/scan_registration.h"
#include "aloam/laser_odometry.h"
#include "ct_icp/ct_icp_registration.h"
#include "xicp/xicp_registration.h"
#include "relead/cesikf.h"
#include "relead/degeneracy_detection.h"
#include "imu_preintegration/imu_preintegration.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::high_resolution_clock;

// ============================================================
// 環境生成: 都市 + トンネル
// ============================================================
struct Environment {
  std::vector<Eigen::Vector3d> points;
  std::vector<Eigen::Vector3d> normals;
};

Environment generateUrbanEnvironment(std::mt19937& rng) {
  Environment env;
  std::normal_distribution<double> noise(0.0, 0.02);
  std::uniform_real_distribution<double> xy(-50.0, 50.0);

  // 地面
  for (int i = 0; i < 10000; i++) {
    env.points.emplace_back(xy(rng), xy(rng), noise(rng));
    env.normals.emplace_back(0, 0, 1);
  }
  // 壁1 (y=20)
  for (int i = 0; i < 3000; i++) {
    std::uniform_real_distribution<double> z(0, 5);
    env.points.emplace_back(xy(rng), 20.0 + noise(rng), z(rng));
    env.normals.emplace_back(0, -1, 0);
  }
  // 壁2 (y=-20)
  for (int i = 0; i < 3000; i++) {
    std::uniform_real_distribution<double> z(0, 5);
    env.points.emplace_back(xy(rng), -20.0 + noise(rng), z(rng));
    env.normals.emplace_back(0, 1, 0);
  }
  // 壁3 (x=30)
  for (int i = 0; i < 2000; i++) {
    std::uniform_real_distribution<double> z(0, 5);
    env.points.emplace_back(30.0 + noise(rng), xy(rng), z(rng));
    env.normals.emplace_back(-1, 0, 0);
  }
  // ビル (x=-15, y=5, 5x5x8m)
  for (int i = 0; i < 2000; i++) {
    std::uniform_real_distribution<double> z(0, 8);
    int face = std::uniform_int_distribution<int>(0, 3)(rng);
    if (face == 0) {
      env.points.emplace_back(-15.0 + noise(rng),
                               std::uniform_real_distribution<double>(2.5, 7.5)(rng), z(rng));
      env.normals.emplace_back(1, 0, 0);
    } else if (face == 1) {
      env.points.emplace_back(-10.0 + noise(rng),
                               std::uniform_real_distribution<double>(2.5, 7.5)(rng), z(rng));
      env.normals.emplace_back(-1, 0, 0);
    } else if (face == 2) {
      env.points.emplace_back(std::uniform_real_distribution<double>(-15, -10)(rng),
                               2.5 + noise(rng), z(rng));
      env.normals.emplace_back(0, 1, 0);
    } else {
      env.points.emplace_back(std::uniform_real_distribution<double>(-15, -10)(rng),
                               7.5 + noise(rng), z(rng));
      env.normals.emplace_back(0, -1, 0);
    }
  }
  return env;
}

// ============================================================
// 軌跡生成: 直線+カーブ
// ============================================================
std::vector<Eigen::Matrix4d> generateTrajectory(int num_frames,
                                                  double step = 1.0) {
  std::vector<Eigen::Matrix4d> poses;
  double x = 0, y = 0, yaw = 0;

  for (int i = 0; i < num_frames; i++) {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) =
        Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    T(0, 3) = x;
    T(1, 3) = y;
    T(2, 3) = 1.0;  // 地面から1m
    poses.push_back(T);

    // 軌跡: 直線 → 右カーブ → 直線
    if (i < num_frames / 3) {
      x += step * std::cos(yaw);
      y += step * std::sin(yaw);
    } else if (i < 2 * num_frames / 3) {
      yaw -= 0.05;  // 右カーブ
      x += step * std::cos(yaw);
      y += step * std::sin(yaw);
    } else {
      x += step * std::cos(yaw);
      y += step * std::sin(yaw);
    }
  }
  return poses;
}

// ============================================================
// LiDARスキャンシミュレーション
// ============================================================
std::vector<Eigen::Vector3d> simulateLidar(const Environment& env,
                                            const Eigen::Matrix4d& pose,
                                            std::mt19937& rng,
                                            double max_range = 30.0) {
  Eigen::Matrix3d R = pose.block<3, 3>(0, 0);
  Eigen::Vector3d t = pose.block<3, 1>(0, 3);
  std::normal_distribution<double> noise(0.0, 0.01);
  std::vector<Eigen::Vector3d> scan;

  for (const auto& wp : env.points) {
    Eigen::Vector3d ps = R.transpose() * (wp - t);
    double range = ps.norm();
    if (range > 1.0 && range < max_range) {
      scan.push_back(ps + Eigen::Vector3d(noise(rng), noise(rng), noise(rng)));
    }
  }
  return scan;
}

// ============================================================
// 各手法の実行
// ============================================================
struct BenchmarkResult {
  std::string method;
  std::vector<Eigen::Matrix4d> estimated_poses;
  double total_time_ms = 0;
  double ate = 0;
};

double computeATE(const std::vector<Eigen::Matrix4d>& est,
                  const std::vector<Eigen::Matrix4d>& gt) {
  int n = std::min(est.size(), gt.size());
  double sum = 0;
  for (int i = 0; i < n; i++) {
    double e = (est[i].block<3, 1>(0, 3) - gt[i].block<3, 1>(0, 3)).norm();
    sum += e * e;
  }
  return std::sqrt(sum / n);
}

BenchmarkResult runLiTAMIN2(const Environment& env,
                             const std::vector<Eigen::Matrix4d>& gt_poses,
                             std::mt19937& rng) {
  using namespace localization_zoo::litamin2;
  BenchmarkResult result;
  result.method = "LiTAMIN2";

  LiTAMIN2Params params;
  params.voxel_resolution = 3.0;
  params.use_cov_cost = true;
  LiTAMIN2Registration reg(params);

  Eigen::Matrix4d T_accum = Eigen::Matrix4d::Identity();
  result.estimated_poses.push_back(T_accum);

  auto t0 = Clock::now();
  for (size_t i = 0; i < gt_poses.size(); i++) {
    auto scan = simulateLidar(env, gt_poses[i], rng);
    if (i == 0) { reg.setTarget(scan); continue; }
    auto res = reg.align(scan);
    T_accum = T_accum * res.transformation.inverse();
    result.estimated_poses.push_back(T_accum);
    reg.setTarget(scan);
  }
  result.total_time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  return result;
}

BenchmarkResult runALOAM(const Environment& env,
                          const std::vector<Eigen::Matrix4d>& gt_poses,
                          std::mt19937& rng) {
  using namespace localization_zoo::aloam;
  BenchmarkResult result;
  result.method = "A-LOAM";

  ScanRegistration scan_reg;
  LaserOdometry odom;
  result.estimated_poses.push_back(Eigen::Matrix4d::Identity());

  auto t0 = Clock::now();
  for (size_t i = 0; i < gt_poses.size(); i++) {
    auto raw = simulateLidar(env, gt_poses[i], rng);
    auto cloud = PointCloudPtr(new PointCloud);
    for (auto& p : raw) {
      PointT pt; pt.x = p.x(); pt.y = p.y(); pt.z = p.z(); pt.intensity = 0;
      cloud->push_back(pt);
    }
    auto features = scan_reg.extract(cloud);
    auto odom_result = odom.process(features);
    if (odom_result.valid) {
      Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
      T.block<3, 3>(0, 0) = odom_result.q_w_curr.toRotationMatrix();
      T.block<3, 1>(0, 3) = odom_result.t_w_curr;
      result.estimated_poses.push_back(T);
    }
  }
  result.total_time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  return result;
}

BenchmarkResult runCTICP(const Environment& env,
                          const std::vector<Eigen::Matrix4d>& gt_poses,
                          std::mt19937& rng) {
  using namespace localization_zoo::ct_icp;
  BenchmarkResult result;
  result.method = "CT-ICP";

  CTICPParams params;
  params.voxel_resolution = 1.0;
  params.max_iterations = 15;
  params.planarity_threshold = 0.1;
  CTICPRegistration reg(params);

  TrajectoryFrame prev_frame;
  prev_frame.begin_pose.trans = prev_frame.end_pose.trans =
      gt_poses[0].block<3, 1>(0, 3);
  result.estimated_poses.push_back(gt_poses[0]);

  auto t0 = Clock::now();
  for (size_t i = 0; i < gt_poses.size(); i++) {
    auto scan_pts = simulateLidar(env, gt_poses[i], rng);
    std::vector<TimedPoint> timed;
    for (size_t j = 0; j < scan_pts.size(); j++) {
      TimedPoint tp;
      tp.raw_point = scan_pts[j];
      tp.timestamp = static_cast<double>(j) / scan_pts.size();
      timed.push_back(tp);
    }

    if (i == 0) {
      std::vector<Eigen::Vector3d> world;
      for (auto& tp : timed)
        world.push_back(prev_frame.transformPoint(tp.raw_point, tp.timestamp));
      reg.addPointsToMap(world);
      continue;
    }

    TrajectoryFrame init;
    init.begin_pose = init.end_pose = prev_frame.end_pose;
    auto res = reg.registerFrame(timed, init, &prev_frame);

    std::vector<Eigen::Vector3d> world;
    for (auto& tp : timed)
      world.push_back(res.frame.transformPoint(tp.raw_point, tp.timestamp));
    reg.addPointsToMap(world);

    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) = res.frame.end_pose.quat.toRotationMatrix();
    T.block<3, 1>(0, 3) = res.frame.end_pose.trans;
    result.estimated_poses.push_back(T);
    prev_frame = res.frame;
  }
  result.total_time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  return result;
}

BenchmarkResult runXICP(const Environment& env,
                         const std::vector<Eigen::Matrix4d>& gt_poses,
                         std::mt19937& rng) {
  using namespace localization_zoo::xicp;
  BenchmarkResult result;
  result.method = "X-ICP";

  XICPParams params;
  params.kappa_1 = 100;
  params.kappa_2 = 70;
  params.kappa_3 = 15;
  XICPRegistration reg(params);

  result.estimated_poses.push_back(Eigen::Matrix4d::Identity());
  std::vector<Eigen::Vector3d> prev_scan;

  auto t0 = Clock::now();
  for (size_t i = 0; i < gt_poses.size(); i++) {
    auto scan = simulateLidar(env, gt_poses[i], rng);
    if (i == 0) { prev_scan = scan; continue; }

    // 簡易対応関係: 各点の最近傍
    std::vector<Correspondence> corrs;
    for (size_t j = 0; j < std::min(scan.size(), (size_t)500); j++) {
      double best_d = 1e9;
      int best_k = -1;
      for (size_t k = 0; k < prev_scan.size(); k += 10) {
        double d = (scan[j] - prev_scan[k]).squaredNorm();
        if (d < best_d) { best_d = d; best_k = k; }
      }
      if (best_d < 4.0 && best_k >= 0) {
        corrs.push_back({scan[j], prev_scan[best_k], Eigen::Vector3d(0, 0, 1)});
      }
    }

    if (!corrs.empty()) {
      auto res = reg.align(corrs);
      result.estimated_poses.push_back(res.transformation);
    }
    prev_scan = scan;
  }
  result.total_time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  return result;
}

// ============================================================
// メイン
// ============================================================
int main() {
  std::cout << "========================================" << std::endl;
  std::cout << "  Localization Zoo - Synthetic Benchmark" << std::endl;
  std::cout << "========================================" << std::endl;

  std::mt19937 rng(42);
  int num_frames = 30;

  std::cout << "\nGenerating environment..." << std::endl;
  auto env = generateUrbanEnvironment(rng);
  std::cout << "  Environment points: " << env.points.size() << std::endl;

  std::cout << "Generating trajectory (" << num_frames << " frames)..." << std::endl;
  auto gt_poses = generateTrajectory(num_frames, 1.0);
  double traj_length = 0;
  for (size_t i = 1; i < gt_poses.size(); i++)
    traj_length += (gt_poses[i].block<3, 1>(0, 3) -
                    gt_poses[i - 1].block<3, 1>(0, 3)).norm();
  std::cout << "  Trajectory length: " << std::fixed << std::setprecision(1)
            << traj_length << " m" << std::endl;

  // 各手法を実行
  std::vector<BenchmarkResult> results;

  std::cout << "\nRunning LiTAMIN2..." << std::endl;
  auto rng1 = rng;
  results.push_back(runLiTAMIN2(env, gt_poses, rng1));

  std::cout << "Running A-LOAM..." << std::endl;
  auto rng2 = rng;
  results.push_back(runALOAM(env, gt_poses, rng2));

  std::cout << "Running CT-ICP..." << std::endl;
  auto rng3 = rng;
  results.push_back(runCTICP(env, gt_poses, rng3));

  std::cout << "Running X-ICP..." << std::endl;
  auto rng4 = rng;
  results.push_back(runXICP(env, gt_poses, rng4));

  // 結果表示
  std::cout << "\n========================================" << std::endl;
  std::cout << "  RESULTS" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << std::left
            << std::setw(15) << "Method"
            << std::setw(12) << "ATE [m]"
            << std::setw(12) << "Frames"
            << std::setw(15) << "Time [ms]"
            << std::setw(12) << "FPS"
            << std::endl;
  std::cout << std::string(66, '-') << std::endl;

  for (auto& r : results) {
    r.ate = computeATE(r.estimated_poses, gt_poses);
    double fps = (r.estimated_poses.size() > 1)
                     ? r.estimated_poses.size() / (r.total_time_ms / 1000.0)
                     : 0;
    std::cout << std::setw(15) << r.method
              << std::setw(12) << std::fixed << std::setprecision(3) << r.ate
              << std::setw(12) << r.estimated_poses.size()
              << std::setw(15) << std::setprecision(1) << r.total_time_ms
              << std::setw(12) << std::setprecision(1) << fps
              << std::endl;
  }

  // ポーズ保存 (plot_trajectory.py で可視化可能)
  std::string output_dir = "benchmark_results";
  fs::create_directories(output_dir);

  // GT保存
  {
    std::ofstream f(output_dir + "/gt.txt");
    for (auto& T : gt_poses)
      for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
          f << T(i, j) << (i == 2 && j == 3 ? "\n" : " ");
  }

  for (auto& r : results) {
    std::string name = r.method;
    std::replace(name.begin(), name.end(), '-', '_');
    std::ofstream f(output_dir + "/" + name + ".txt");
    for (auto& T : r.estimated_poses)
      for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
          f << T(i, j) << (i == 2 && j == 3 ? "\n" : " ");
  }

  std::cout << "\nResults saved to " << output_dir << "/" << std::endl;
  std::cout << "Visualize: python3 ../evaluation/scripts/plot_trajectory.py "
            << "--gt " << output_dir << "/gt.txt "
            << "--est ";
  for (auto& r : results) {
    std::string name = r.method;
    std::replace(name.begin(), name.end(), '-', '_');
    std::cout << r.method << ":" << output_dir << "/" << name << ".txt ";
  }
  std::cout << std::endl;

  return 0;
}
