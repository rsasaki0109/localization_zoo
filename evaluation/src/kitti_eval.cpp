/// KITTI Odometry Benchmark 評価ツール
///
/// 使い方:
///   ./kitti_eval --method litamin2 --sequence 00 --data_dir /path/to/kitti/sequences
///
/// KITTI形式:
///   sequences/00/velodyne/*.bin  (点群)
///   sequences/00/calib.txt       (キャリブレーション)
///   poses/00.txt                 (GT: 12値/行 = 3x4変換行列)

#include "litamin2/litamin2_registration.h"
#include "aloam/scan_registration.h"
#include "aloam/laser_odometry.h"
#include "ct_icp/ct_icp_registration.h"
#include "xicp/xicp_registration.h"

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <Eigen/Core>

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

/// KITTI .bin 点群を読み込み
std::vector<Eigen::Vector3d> loadKittiBin(const std::string& path) {
  std::vector<Eigen::Vector3d> points;
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) return points;

  while (file.good()) {
    float x, y, z, intensity;
    file.read(reinterpret_cast<char*>(&x), sizeof(float));
    file.read(reinterpret_cast<char*>(&y), sizeof(float));
    file.read(reinterpret_cast<char*>(&z), sizeof(float));
    file.read(reinterpret_cast<char*>(&intensity), sizeof(float));
    if (file.good()) {
      double range = std::sqrt(x * x + y * y + z * z);
      if (range > 0.5 && range < 100.0) {
        points.emplace_back(x, y, z);
      }
    }
  }
  return points;
}

/// KITTI poses.txt を読み込み (各行: 12値 = 3x4変換行列)
std::vector<Eigen::Matrix4d> loadKittiPoses(const std::string& path) {
  std::vector<Eigen::Matrix4d> poses;
  std::ifstream file(path);
  if (!file.is_open()) return poses;

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream ss(line);
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 4; j++)
        ss >> T(i, j);
    poses.push_back(T);
  }
  return poses;
}

/// ボクセルダウンサンプリング
std::vector<Eigen::Vector3d> downsample(const std::vector<Eigen::Vector3d>& pts,
                                         double leaf) {
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
  for (auto& p : pts) cloud->emplace_back(p.x(), p.y(), p.z());

  pcl::VoxelGrid<pcl::PointXYZ> vg;
  vg.setInputCloud(cloud);
  vg.setLeafSize(leaf, leaf, leaf);
  pcl::PointCloud<pcl::PointXYZ> filtered;
  vg.filter(filtered);

  std::vector<Eigen::Vector3d> result;
  for (auto& p : filtered) result.emplace_back(p.x, p.y, p.z);
  return result;
}

/// 相対ポーズ誤差 (RPE) を計算
struct RPE {
  double trans_err;  // [m]
  double rot_err;    // [deg]
};

RPE computeRPE(const Eigen::Matrix4d& T_est, const Eigen::Matrix4d& T_gt) {
  Eigen::Matrix4d T_err = T_gt.inverse() * T_est;
  double trans = T_err.block<3, 1>(0, 3).norm();
  Eigen::Matrix3d R_err = T_err.block<3, 3>(0, 0);
  double angle = std::acos(std::clamp((R_err.trace() - 1.0) / 2.0, -1.0, 1.0));
  return {trans, angle * 180.0 / M_PI};
}

/// LiTAMIN2でオドメトリ実行
std::vector<Eigen::Matrix4d> runLiTAMIN2(
    const std::vector<std::string>& bin_files, double voxel_res) {
  using namespace localization_zoo::litamin2;

  LiTAMIN2Params params;
  params.voxel_resolution = voxel_res;
  params.use_cov_cost = true;
  LiTAMIN2Registration reg(params);

  std::vector<Eigen::Matrix4d> poses;
  Eigen::Matrix4d T_accum = Eigen::Matrix4d::Identity();
  poses.push_back(T_accum);

  for (size_t i = 0; i < bin_files.size(); i++) {
    auto pts = downsample(loadKittiBin(bin_files[i]), 0.5);
    if (pts.empty()) continue;

    if (i == 0) {
      reg.setTarget(pts);
      continue;
    }

    auto result = reg.align(pts);
    T_accum = T_accum * result.transformation.inverse();
    poses.push_back(T_accum);
    reg.setTarget(pts);

    if (i % 100 == 0)
      std::cerr << "\r  [LiTAMIN2] Frame " << i << "/" << bin_files.size();
  }
  std::cerr << std::endl;
  return poses;
}

/// A-LOAMでオドメトリ実行
std::vector<Eigen::Matrix4d> runALOAM(
    const std::vector<std::string>& bin_files) {
  using namespace localization_zoo::aloam;

  ScanRegistration scan_reg;
  LaserOdometry odom;

  std::vector<Eigen::Matrix4d> poses;
  poses.push_back(Eigen::Matrix4d::Identity());

  for (size_t i = 0; i < bin_files.size(); i++) {
    auto raw_pts = loadKittiBin(bin_files[i]);
    if (raw_pts.empty()) continue;

    // PCL PointCloud に変換
    auto cloud = PointCloudPtr(new PointCloud);
    for (auto& p : raw_pts) {
      PointT pt;
      pt.x = p.x(); pt.y = p.y(); pt.z = p.z(); pt.intensity = 0;
      cloud->push_back(pt);
    }

    auto features = scan_reg.extract(cloud);
    auto result = odom.process(features);

    if (result.valid) {
      Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
      T.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
      T.block<3, 1>(0, 3) = result.t_w_curr;
      poses.push_back(T);
    }

    if (i % 100 == 0)
      std::cerr << "\r  [A-LOAM] Frame " << i << "/" << bin_files.size();
  }
  std::cerr << std::endl;
  return poses;
}

/// ポーズをKITTI形式で保存
void savePoses(const std::vector<Eigen::Matrix4d>& poses,
               const std::string& path) {
  std::ofstream file(path);
  for (auto& T : poses) {
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 4; j++)
        file << std::setprecision(9) << T(i, j) << (j == 3 && i == 2 ? "\n" : " ");
  }
}

/// ATE (Absolute Trajectory Error) を計算
double computeATE(const std::vector<Eigen::Matrix4d>& est,
                  const std::vector<Eigen::Matrix4d>& gt) {
  int n = std::min(est.size(), gt.size());
  double sum_sq = 0;
  for (int i = 0; i < n; i++) {
    double err = (est[i].block<3, 1>(0, 3) - gt[i].block<3, 1>(0, 3)).norm();
    sum_sq += err * err;
  }
  return std::sqrt(sum_sq / n);
}

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0]
              << " <method> <sequence_dir> <gt_poses_file> [output_file]"
              << std::endl;
    std::cerr << "Methods: litamin2, aloam" << std::endl;
    return 1;
  }

  std::string method = argv[1];
  std::string seq_dir = argv[2];
  std::string gt_file = argv[3];
  std::string output_file = argc > 4 ? argv[4] : "";

  // bin ファイルを列挙
  std::string velodyne_dir = seq_dir + "/velodyne";
  std::vector<std::string> bin_files;
  for (auto& entry : fs::directory_iterator(velodyne_dir)) {
    if (entry.path().extension() == ".bin")
      bin_files.push_back(entry.path().string());
  }
  std::sort(bin_files.begin(), bin_files.end());

  std::cout << "Sequence: " << seq_dir << std::endl;
  std::cout << "Method: " << method << std::endl;
  std::cout << "Frames: " << bin_files.size() << std::endl;

  // GT読み込み
  auto gt_poses = loadKittiPoses(gt_file);
  std::cout << "GT poses: " << gt_poses.size() << std::endl;

  // 実行
  auto t_start = std::chrono::high_resolution_clock::now();
  std::vector<Eigen::Matrix4d> est_poses;

  if (method == "litamin2") {
    est_poses = runLiTAMIN2(bin_files, 3.0);
  } else if (method == "aloam") {
    est_poses = runALOAM(bin_files);
  } else {
    std::cerr << "Unknown method: " << method << std::endl;
    return 1;
  }

  auto t_end = std::chrono::high_resolution_clock::now();
  double elapsed =
      std::chrono::duration<double>(t_end - t_start).count();

  // 評価
  double ate = computeATE(est_poses, gt_poses);
  double fps = bin_files.size() / elapsed;

  std::cout << "=== Results ===" << std::endl;
  std::cout << "ATE (RMSE): " << std::fixed << std::setprecision(3) << ate
            << " m" << std::endl;
  std::cout << "Total time: " << std::setprecision(1) << elapsed << " s"
            << std::endl;
  std::cout << "FPS: " << std::setprecision(1) << fps << std::endl;

  // ポーズ保存
  if (!output_file.empty()) {
    savePoses(est_poses, output_file);
    std::cout << "Saved to: " << output_file << std::endl;
  }

  return 0;
}
