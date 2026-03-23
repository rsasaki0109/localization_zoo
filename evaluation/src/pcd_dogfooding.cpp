/// PCD連番データでの Dogfooding ツール
///
/// 使い方:
///   ./pcd_dogfooding <pcd_dir> <gt_csv> [max_frames]
///
/// pcd_dir: 00000000/cloud.pcd, 00000001/cloud.pcd, ... が並ぶディレクトリ
/// gt_csv:  lidar_pose.x,y,z,roll,pitch,yaw を含むCSV

#include "litamin2/litamin2_registration.h"
#include "ct_icp/ct_icp_registration.h"
#include "xicp/xicp_registration.h"

#include <pcl/io/pcd_io.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::high_resolution_clock;

struct GTPose {
  double timestamp;
  double x, y, z, roll, pitch, yaw;

  Eigen::Matrix4d toMatrix() const {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    Eigen::AngleAxisd rx(roll, Eigen::Vector3d::UnitX());
    Eigen::AngleAxisd ry(pitch, Eigen::Vector3d::UnitY());
    Eigen::AngleAxisd rz(yaw, Eigen::Vector3d::UnitZ());
    T.block<3, 3>(0, 0) = (rz * ry * rx).toRotationMatrix();
    T(0, 3) = x; T(1, 3) = y; T(2, 3) = z;
    return T;
  }
};

std::vector<GTPose> loadGTPoses(const std::string& csv_path) {
  std::vector<GTPose> poses;
  std::ifstream file(csv_path);
  std::string line;
  std::getline(file, line);  // skip header

  // ヘッダからカラムインデックスを取得
  std::istringstream header_ss(line);
  std::string col;
  std::vector<std::string> cols;
  while (std::getline(header_ss, col, ',')) cols.push_back(col);

  int idx_ts = -1, idx_x = -1, idx_y = -1, idx_z = -1;
  int idx_roll = -1, idx_pitch = -1, idx_yaw = -1;
  for (size_t i = 0; i < cols.size(); i++) {
    if (cols[i] == "timestamp") idx_ts = i;
    if (cols[i] == "lidar_pose.x") idx_x = i;
    if (cols[i] == "lidar_pose.y") idx_y = i;
    if (cols[i] == "lidar_pose.z") idx_z = i;
    if (cols[i] == "lidar_pose.roll") idx_roll = i;
    if (cols[i] == "lidar_pose.pitch") idx_pitch = i;
    if (cols[i] == "lidar_pose.yaw") idx_yaw = i;
  }

  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::vector<std::string> vals;
    std::string token;
    while (std::getline(ss, token, ',')) vals.push_back(token);

    if (idx_x >= 0 && idx_x < (int)vals.size()) {
      GTPose gp;
      gp.timestamp = idx_ts >= 0 ? std::stod(vals[idx_ts]) : poses.size();
      gp.x = std::stod(vals[idx_x]);
      gp.y = std::stod(vals[idx_y]);
      gp.z = std::stod(vals[idx_z]);
      gp.roll = std::stod(vals[idx_roll]);
      gp.pitch = std::stod(vals[idx_pitch]);
      gp.yaw = std::stod(vals[idx_yaw]);
      poses.push_back(gp);
    }
  }
  return poses;
}

std::vector<Eigen::Vector3d> loadPCD(const std::string& path, double leaf = 0.5) {
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
  if (pcl::io::loadPCDFile<pcl::PointXYZ>(path, *cloud) == -1) return {};

  // ダウンサンプリング
  pcl::VoxelGrid<pcl::PointXYZ> vg;
  vg.setInputCloud(cloud);
  vg.setLeafSize(leaf, leaf, leaf);
  pcl::PointCloud<pcl::PointXYZ> filtered;
  vg.filter(filtered);

  std::vector<Eigen::Vector3d> points;
  for (auto& p : filtered) {
    double r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    if (r > 1.0 && r < 80.0)
      points.emplace_back(p.x, p.y, p.z);
  }
  return points;
}

std::vector<std::string> listPCDDirs(const std::string& dir) {
  std::vector<std::string> dirs;
  for (auto& entry : fs::directory_iterator(dir)) {
    if (entry.is_directory()) {
      auto pcd_path = entry.path() / "cloud.pcd";
      if (fs::exists(pcd_path)) dirs.push_back(entry.path().string());
    }
  }
  std::sort(dirs.begin(), dirs.end());
  return dirs;
}

double computeATE(const std::vector<Eigen::Matrix4d>& est,
                  const std::vector<Eigen::Matrix4d>& gt) {
  int n = std::min(est.size(), gt.size());
  if (n == 0) return 0;
  double sum = 0;
  for (int i = 0; i < n; i++) {
    double e = (est[i].block<3, 1>(0, 3) - gt[i].block<3, 1>(0, 3)).norm();
    sum += e * e;
  }
  return std::sqrt(sum / n);
}

// ============================================================

struct MethodResult {
  std::string name;
  std::vector<Eigen::Matrix4d> poses;
  double time_ms = 0;
  double ate = 0;
};

MethodResult runLiTAMIN2(const std::vector<std::string>& pcd_dirs,
                          const std::vector<Eigen::Matrix4d>& gt) {
  using namespace localization_zoo::litamin2;
  MethodResult res;
  res.name = "LiTAMIN2";

  LiTAMIN2Params params;
  params.voxel_resolution = 1.0;
  params.use_cov_cost = true;
  LiTAMIN2Registration reg(params);

  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  res.poses.push_back(T);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.5);
    if (pts.empty()) continue;
    if (i == 0) { reg.setTarget(pts); continue; }
    auto result = reg.align(pts);
    T = T * result.transformation.inverse();
    res.poses.push_back(T);
    reg.setTarget(pts);
    if (i % 10 == 0)
      std::cerr << "\r  [LiTAMIN2] " << i << "/" << pcd_dirs.size();
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  return res;
}

MethodResult runCTICP(const std::vector<std::string>& pcd_dirs,
                       const std::vector<Eigen::Matrix4d>& gt) {
  using namespace localization_zoo::ct_icp;
  MethodResult res;
  res.name = "CT-ICP";

  CTICPParams params;
  params.voxel_resolution = 1.0;
  params.max_iterations = 15;
  params.planarity_threshold = 0.1;
  CTICPRegistration reg(params);

  TrajectoryFrame prev;
  res.poses.push_back(Eigen::Matrix4d::Identity());

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.5);
    if (pts.empty()) continue;

    std::vector<TimedPoint> timed;
    for (size_t j = 0; j < pts.size(); j++) {
      TimedPoint tp;
      tp.raw_point = pts[j];
      tp.timestamp = static_cast<double>(j) / pts.size();
      timed.push_back(tp);
    }

    if (i == 0) {
      std::vector<Eigen::Vector3d> world;
      for (auto& tp : timed) world.push_back(prev.transformPoint(tp.raw_point, tp.timestamp));
      reg.addPointsToMap(world);
      continue;
    }

    TrajectoryFrame init;
    init.begin_pose = init.end_pose = prev.end_pose;
    auto result = reg.registerFrame(timed, init, &prev);

    std::vector<Eigen::Vector3d> world;
    for (auto& tp : timed)
      world.push_back(result.frame.transformPoint(tp.raw_point, tp.timestamp));
    reg.addPointsToMap(world);

    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) = result.frame.end_pose.quat.toRotationMatrix();
    T.block<3, 1>(0, 3) = result.frame.end_pose.trans;
    res.poses.push_back(T);
    prev = result.frame;

    if (i % 10 == 0)
      std::cerr << "\r  [CT-ICP] " << i << "/" << pcd_dirs.size();
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  return res;
}

void savePosesKITTI(const std::vector<Eigen::Matrix4d>& poses, const std::string& path) {
  std::ofstream f(path);
  for (auto& T : poses) {
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 4; j++)
        f << std::setprecision(9) << T(i, j) << (i == 2 && j == 3 ? "\n" : " ");
  }
}

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <pcd_dir> <gt_csv> [max_frames]" << std::endl;
    return 1;
  }

  std::string pcd_dir = argv[1];
  std::string gt_csv = argv[2];
  int max_frames = argc > 3 ? std::stoi(argv[3]) : -1;

  auto pcd_dirs = listPCDDirs(pcd_dir);
  if (max_frames > 0 && max_frames < (int)pcd_dirs.size())
    pcd_dirs.resize(max_frames);

  auto gt_poses_raw = loadGTPoses(gt_csv);
  std::vector<Eigen::Matrix4d> gt;
  Eigen::Matrix4d T0_inv = gt_poses_raw[0].toMatrix().inverse();
  for (auto& gp : gt_poses_raw)
    gt.push_back(T0_inv * gp.toMatrix());  // 原点を最初のフレームに

  std::cout << "========================================" << std::endl;
  std::cout << "  PCD Dogfooding" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "PCD frames: " << pcd_dirs.size() << std::endl;
  std::cout << "GT poses: " << gt.size() << std::endl;

  // 走行距離
  double dist = 0;
  for (size_t i = 1; i < gt.size(); i++)
    dist += (gt[i].block<3, 1>(0, 3) - gt[i-1].block<3, 1>(0, 3)).norm();
  std::cout << "Trajectory length: " << std::fixed << std::setprecision(1) << dist << " m" << std::endl;

  std::vector<MethodResult> results;

  std::cout << "\nRunning LiTAMIN2..." << std::endl;
  results.push_back(runLiTAMIN2(pcd_dirs, gt));

  // CT-ICPは実データだとボクセルマップが大きく遅い。必要時のみ有効化
  // std::cout << "Running CT-ICP..." << std::endl;
  // results.push_back(runCTICP(pcd_dirs, gt));

  // 結果表示
  std::cout << "\n========================================" << std::endl;
  std::cout << "  RESULTS (Real Data)" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << std::left
            << std::setw(15) << "Method"
            << std::setw(12) << "ATE [m]"
            << std::setw(12) << "Frames"
            << std::setw(15) << "Time [ms]"
            << std::setw(12) << "FPS"
            << std::endl;
  std::cout << std::string(66, '-') << std::endl;

  fs::create_directories("dogfooding_results");
  savePosesKITTI(gt, "dogfooding_results/gt.txt");

  for (auto& r : results) {
    r.ate = computeATE(r.poses, gt);
    double fps = r.poses.size() / (r.time_ms / 1000.0);
    std::cout << std::setw(15) << r.name
              << std::setw(12) << std::setprecision(3) << r.ate
              << std::setw(12) << r.poses.size()
              << std::setw(15) << std::setprecision(1) << r.time_ms
              << std::setw(12) << std::setprecision(1) << fps
              << std::endl;

    std::string fname = r.name;
    std::replace(fname.begin(), fname.end(), '-', '_');
    savePosesKITTI(r.poses, "dogfooding_results/" + fname + ".txt");
  }

  std::cout << "\nResults saved to dogfooding_results/" << std::endl;
  return 0;
}
