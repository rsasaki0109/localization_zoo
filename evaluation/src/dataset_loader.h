#pragma once

#include <Eigen/Core>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace localization_zoo {
namespace evaluation {

namespace fs = std::filesystem;

/// 汎用点群フレーム
struct PointCloudFrame {
  double timestamp = 0.0;
  std::vector<Eigen::Vector3d> points;
};

/// GTポーズ
struct GTPose {
  double timestamp = 0.0;
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
};

// ============================================================
// KITTI形式 (.bin, 4 float: x,y,z,intensity)
// ============================================================
inline std::vector<Eigen::Vector3d> loadKittiBin(const std::string& path) {
  std::vector<Eigen::Vector3d> points;
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) return points;
  while (file.good()) {
    float buf[4];
    file.read(reinterpret_cast<char*>(buf), 4 * sizeof(float));
    if (file.good()) {
      double r = std::sqrt(buf[0] * buf[0] + buf[1] * buf[1] + buf[2] * buf[2]);
      if (r > 0.5 && r < 120.0)
        points.emplace_back(buf[0], buf[1], buf[2]);
    }
  }
  return points;
}

/// KITTI poses.txt (12 float/line = 3x4 matrix)
inline std::vector<GTPose> loadKittiPoses(const std::string& path) {
  std::vector<GTPose> poses;
  std::ifstream file(path);
  std::string line;
  int idx = 0;
  while (std::getline(file, line)) {
    std::istringstream ss(line);
    GTPose gp;
    gp.timestamp = idx++;
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 4; j++)
        ss >> gp.pose(i, j);
    poses.push_back(gp);
  }
  return poses;
}

// ============================================================
// MulRan形式 (.bin, Ouster OS1-64, 4 float: x,y,z,intensity)
// タイムスタンプはファイル名 (nanoseconds)
// ============================================================
inline std::vector<PointCloudFrame> loadMulRanSequence(
    const std::string& lidar_dir, int max_frames = -1) {
  std::vector<PointCloudFrame> frames;
  std::vector<std::string> files;
  for (auto& entry : fs::directory_iterator(lidar_dir)) {
    if (entry.path().extension() == ".bin")
      files.push_back(entry.path().string());
  }
  std::sort(files.begin(), files.end());

  for (size_t i = 0; i < files.size(); i++) {
    if (max_frames > 0 && static_cast<int>(i) >= max_frames) break;
    PointCloudFrame frame;
    // ファイル名からタイムスタンプ抽出 (nanoseconds)
    std::string stem = fs::path(files[i]).stem().string();
    frame.timestamp = std::stod(stem) * 1e-9;
    frame.points = loadKittiBin(files[i]);  // 同じ4-float形式
    frames.push_back(frame);
  }
  return frames;
}

/// MulRan GT: global_pose.csv (timestamp,x,y,z,qx,qy,qz,qw)
inline std::vector<GTPose> loadMulRanPoses(const std::string& path) {
  std::vector<GTPose> poses;
  std::ifstream file(path);
  std::string line;
  std::getline(file, line);  // skip header
  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string token;
    std::vector<double> vals;
    while (std::getline(ss, token, ','))
      vals.push_back(std::stod(token));
    if (vals.size() >= 8) {
      GTPose gp;
      gp.timestamp = vals[0] * 1e-9;
      Eigen::Quaterniond q(vals[7], vals[4], vals[5], vals[6]);
      gp.pose.block<3, 3>(0, 0) = q.toRotationMatrix();
      gp.pose.block<3, 1>(0, 3) = Eigen::Vector3d(vals[1], vals[2], vals[3]);
      poses.push_back(gp);
    }
  }
  return poses;
}

// ============================================================
// nuScenes形式 (.pcd.bin, 5 float: x,y,z,intensity,ring)
// ============================================================
inline std::vector<Eigen::Vector3d> loadNuScenesBin(const std::string& path) {
  std::vector<Eigen::Vector3d> points;
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) return points;
  while (file.good()) {
    float buf[5];
    file.read(reinterpret_cast<char*>(buf), 5 * sizeof(float));
    if (file.good()) {
      double r = std::sqrt(buf[0] * buf[0] + buf[1] * buf[1] + buf[2] * buf[2]);
      if (r > 0.5 && r < 100.0)
        points.emplace_back(buf[0], buf[1], buf[2]);
    }
  }
  return points;
}

// ============================================================
// Newer College: PCD形式 (PCLで読み込み) またはrosbag
// ============================================================

// ============================================================
// 汎用: ディレクトリ内の.binファイルを自動検出
// ============================================================
inline std::vector<std::string> listBinFiles(const std::string& dir) {
  std::vector<std::string> files;
  if (!fs::exists(dir)) return files;
  for (auto& entry : fs::directory_iterator(dir)) {
    if (entry.path().extension() == ".bin")
      files.push_back(entry.path().string());
  }
  std::sort(files.begin(), files.end());
  return files;
}

// ============================================================
// 評価メトリクス
// ============================================================

/// ATE (RMSE)
inline double computeATE(const std::vector<Eigen::Matrix4d>& est,
                          const std::vector<GTPose>& gt) {
  int n = std::min(est.size(), gt.size());
  if (n == 0) return 0.0;
  double sum = 0;
  for (int i = 0; i < n; i++) {
    double e = (est[i].block<3, 1>(0, 3) - gt[i].pose.block<3, 1>(0, 3)).norm();
    sum += e * e;
  }
  return std::sqrt(sum / n);
}

/// RPE (100m区間の平均)
inline std::pair<double, double> computeRPE(
    const std::vector<Eigen::Matrix4d>& est,
    const std::vector<GTPose>& gt, double segment_length = 100.0) {
  int n = std::min(est.size(), gt.size());
  std::vector<double> trans_errs, rot_errs;

  for (int i = 0; i < n; i++) {
    // segment_lengthだけ進んだフレームを探す
    double dist = 0;
    int j = i + 1;
    while (j < n) {
      dist += (gt[j].pose.block<3, 1>(0, 3) -
               gt[j - 1].pose.block<3, 1>(0, 3)).norm();
      if (dist >= segment_length) break;
      j++;
    }
    if (j >= n) break;

    Eigen::Matrix4d dT_est = est[i].inverse() * est[j];
    Eigen::Matrix4d dT_gt = gt[i].pose.inverse() * gt[j].pose;
    Eigen::Matrix4d T_err = dT_gt.inverse() * dT_est;

    double t_err = T_err.block<3, 1>(0, 3).norm();
    Eigen::Matrix3d R_err = T_err.block<3, 3>(0, 0);
    double r_err = std::acos(std::clamp((R_err.trace() - 1.0) / 2.0, -1.0, 1.0));

    trans_errs.push_back(t_err / dist * 100.0);  // [%]
    rot_errs.push_back(r_err / dist * 180.0 / M_PI);  // [deg/m]
  }

  if (trans_errs.empty()) return {0, 0};
  double avg_t = 0, avg_r = 0;
  for (auto& e : trans_errs) avg_t += e;
  for (auto& e : rot_errs) avg_r += e;
  return {avg_t / trans_errs.size(), avg_r / rot_errs.size()};
}

/// 結果をCSVに保存
inline void saveResultsCSV(const std::string& path,
                            const std::string& method,
                            const std::string& dataset,
                            const std::string& sequence,
                            double ate, double rpe_trans, double rpe_rot,
                            double fps) {
  bool exists = fs::exists(path);
  std::ofstream file(path, std::ios::app);
  if (!exists) {
    file << "method,dataset,sequence,ATE_m,RPE_trans_pct,RPE_rot_deg_per_m,FPS\n";
  }
  file << method << "," << dataset << "," << sequence << ","
       << ate << "," << rpe_trans << "," << rpe_rot << "," << fps << "\n";
}

}  // namespace evaluation
}  // namespace localization_zoo
