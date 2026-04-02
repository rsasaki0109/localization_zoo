/// PCD連番データでの Dogfooding ツール
///
/// 使い方:
///   ./pcd_dogfooding <pcd_dir> <gt_csv> [max_frames] [--force-ct-lio]
///
/// pcd_dir: 00000000/cloud.pcd, 00000001/cloud.pcd, ... が並ぶディレクトリ
/// gt_csv:  lidar_pose.x,y,z,roll,pitch,yaw を含むCSV

#include "gicp/gicp_registration.h"
#include "kiss_icp/kiss_icp.h"
#include "litamin2/litamin2_registration.h"
#include "ndt/ndt_registration.h"
#include "ct_icp/ct_icp_registration.h"
#include "ct_lio/ct_lio_registration.h"
#include "xicp/xicp_registration.h"

#define PCL_NO_PRECOMPILE
#include <pcl/io/pcd_io.h>
#include <pcl/PCLPointCloud2.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/conversions.h>
#include <pcl/filters/voxel_grid.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include <algorithm>
#include <cctype>
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

struct ImuSampleCsv {
  double timestamp = 0.0;
  Eigen::Vector3d gyro = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel = Eigen::Vector3d::Zero();
};

enum class FrameTimestampSource {
  kFrameTimestampCsv,
  kGraphTrajectory,
  kSampledGT,
};

struct FrameGapStats {
  bool valid = false;
  double min_gap = 0.0;
  double max_gap = 0.0;
  double mean_gap = 0.0;
  double median_gap = 0.0;
};

struct EIGEN_ALIGN16 PointXYZITime {
  PCL_ADD_POINT4D;
  float intensity;
  float time;
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

POINT_CLOUD_REGISTER_POINT_STRUCT(
    PointXYZITime,
    (float, x, x)
    (float, y, y)
    (float, z, z)
    (float, intensity, intensity)
    (float, time, time))

struct LoadedScan {
  std::vector<Eigen::Vector3d> points;
  std::vector<double> relative_times;
  bool has_per_point_time = false;
};

std::string trimCsvToken(std::string token) {
  while (!token.empty() &&
         (token.back() == '\r' || token.back() == '\n' || token.back() == ' ' ||
          token.back() == '\t')) {
    token.pop_back();
  }
  size_t begin = 0;
  while (begin < token.size() &&
         (token[begin] == ' ' || token[begin] == '\t')) {
    begin++;
  }
  return token.substr(begin);
}

std::string normalizeMethodId(std::string method) {
  method = trimCsvToken(method);
  for (char& c : method) {
    if (c == '-') c = '_';
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return method;
}

std::vector<std::string> splitMethodList(const std::string& csv) {
  std::vector<std::string> methods;
  std::istringstream ss(csv);
  std::string token;
  while (std::getline(ss, token, ',')) {
    token = normalizeMethodId(token);
    if (!token.empty()) methods.push_back(token);
  }
  return methods;
}

bool isSupportedMethod(const std::string& method) {
  return method == "litamin2" || method == "gicp" || method == "ndt" ||
         method == "kiss_icp" || method == "ct_lio" || method == "ct_icp";
}

bool isMethodEnabled(const std::vector<std::string>& methods,
                     const std::string& method) {
  return std::find(methods.begin(), methods.end(), method) != methods.end();
}

std::vector<GTPose> loadGTPoses(const std::string& csv_path) {
  std::vector<GTPose> poses;
  std::ifstream file(csv_path);
  std::string line;
  std::getline(file, line);  // skip header

  // ヘッダからカラムインデックスを取得
  std::istringstream header_ss(line);
  std::string col;
  std::vector<std::string> cols;
  while (std::getline(header_ss, col, ',')) cols.push_back(trimCsvToken(col));

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
    while (std::getline(ss, token, ',')) vals.push_back(trimCsvToken(token));

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

std::vector<ImuSampleCsv> loadImuCsv(const std::string& csv_path) {
  std::vector<ImuSampleCsv> samples;
  std::ifstream file(csv_path);
  if (!file.is_open()) return samples;

  std::string line;
  if (!std::getline(file, line)) return samples;

  std::istringstream header_ss(line);
  std::string col;
  std::vector<std::string> cols;
  while (std::getline(header_ss, col, ',')) cols.push_back(trimCsvToken(col));

  int idx_stamp = -1;
  int idx_gx = -1, idx_gy = -1, idx_gz = -1;
  int idx_ax = -1, idx_ay = -1, idx_az = -1;
  for (size_t i = 0; i < cols.size(); i++) {
    if (cols[i] == "stamp") idx_stamp = i;
    if (cols[i] == "angular_vel.x") idx_gx = i;
    if (cols[i] == "angular_vel.y") idx_gy = i;
    if (cols[i] == "angular_vel.z") idx_gz = i;
    if (cols[i] == "linear_acc.x") idx_ax = i;
    if (cols[i] == "linear_acc.y") idx_ay = i;
    if (cols[i] == "linear_acc.z") idx_az = i;
  }

  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::vector<std::string> vals;
    std::string token;
    while (std::getline(ss, token, ',')) vals.push_back(trimCsvToken(token));

    if (idx_stamp < 0 || idx_az < 0 || idx_stamp >= static_cast<int>(vals.size()) ||
        idx_az >= static_cast<int>(vals.size())) {
      continue;
    }

    ImuSampleCsv sample;
    sample.timestamp = std::stod(vals[idx_stamp]);
    sample.gyro = Eigen::Vector3d(std::stod(vals[idx_gx]), std::stod(vals[idx_gy]),
                                  std::stod(vals[idx_gz]));
    sample.accel = Eigen::Vector3d(std::stod(vals[idx_ax]), std::stod(vals[idx_ay]),
                                   std::stod(vals[idx_az]));
    samples.push_back(sample);
  }

  return samples;
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

std::vector<Eigen::Vector3d> limitPoints(const std::vector<Eigen::Vector3d>& points,
                                         size_t max_points) {
  if (points.size() <= max_points) return points;
  std::vector<Eigen::Vector3d> limited;
  limited.reserve(max_points);
  double step = static_cast<double>(points.size()) / max_points;
  for (size_t i = 0; i < max_points; i++) {
    size_t idx = std::min(static_cast<size_t>(i * step), points.size() - 1);
    limited.push_back(points[idx]);
  }
  return limited;
}

void normalizeRelativeTimes(std::vector<double>& relative_times) {
  if (relative_times.empty()) return;

  auto [min_it, max_it] =
      std::minmax_element(relative_times.begin(), relative_times.end());
  double min_time = *min_it;
  double max_time = *max_it;
  if (max_time <= min_time + 1e-9) {
    const size_t denom = relative_times.size() > 1 ? relative_times.size() - 1 : 1;
    for (size_t i = 0; i < relative_times.size(); i++) {
      relative_times[i] = static_cast<double>(i) / denom;
    }
    return;
  }

  for (double& t : relative_times) {
    t = (t - min_time) / (max_time - min_time);
  }
}

LoadedScan limitLoadedScan(const LoadedScan& scan, size_t max_points) {
  if (scan.points.size() <= max_points) return scan;

  LoadedScan limited;
  limited.has_per_point_time = scan.has_per_point_time;
  limited.points.reserve(max_points);
  if (scan.has_per_point_time) limited.relative_times.reserve(max_points);

  double step = static_cast<double>(scan.points.size()) / max_points;
  for (size_t i = 0; i < max_points; i++) {
    size_t idx = std::min(static_cast<size_t>(i * step), scan.points.size() - 1);
    limited.points.push_back(scan.points[idx]);
    if (scan.has_per_point_time) {
      limited.relative_times.push_back(scan.relative_times[idx]);
    }
  }
  return limited;
}

LoadedScan loadTimedPCD(const std::string& path, double leaf = 0.5) {
  LoadedScan scan;

  pcl::PCLPointCloud2 raw_cloud;
  if (pcl::io::loadPCDFile(path, raw_cloud) == -1) return scan;

  bool has_time_field = false;
  for (const auto& field : raw_cloud.fields) {
    if (field.name == "time") {
      has_time_field = true;
      break;
    }
  }
  if (!has_time_field) {
    scan.points = loadPCD(path, leaf);
    return scan;
  }

  pcl::PointCloud<PointXYZITime>::Ptr cloud(new pcl::PointCloud<PointXYZITime>);
  pcl::fromPCLPointCloud2(raw_cloud, *cloud);

  pcl::PointCloud<PointXYZITime>::Ptr filtered(new pcl::PointCloud<PointXYZITime>);
  pcl::VoxelGrid<PointXYZITime> vg;
  vg.setInputCloud(cloud);
  vg.setLeafSize(leaf, leaf, leaf);
  vg.setDownsampleAllData(true);
  vg.filter(*filtered);

  scan.points.reserve(filtered->size());
  scan.relative_times.reserve(filtered->size());
  for (const auto& p : filtered->points) {
    if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z) ||
        !std::isfinite(p.time)) {
      continue;
    }
    double r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    if (r <= 1.0 || r >= 80.0) continue;

    scan.points.emplace_back(p.x, p.y, p.z);
    scan.relative_times.push_back(p.time);
  }

  scan.has_per_point_time = !scan.relative_times.empty();
  normalizeRelativeTimes(scan.relative_times);
  return scan;
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

std::vector<double> loadFrameTimestampsFromGraph(
    const std::vector<std::string>& pcd_dirs) {
  std::vector<double> timestamps;
  timestamps.reserve(pcd_dirs.size());

  for (const auto& dir : pcd_dirs) {
    fs::path trajectory_csv = fs::path(dir) / "trajectory.csv";
    if (!fs::exists(trajectory_csv)) {
      return {};
    }

    std::ifstream file(trajectory_csv);
    std::string line;
    if (!std::getline(file, line) || !std::getline(file, line)) {
      return {};
    }

    std::istringstream ss(line);
    std::vector<std::string> vals;
    std::string token;
    while (std::getline(ss, token, ',')) vals.push_back(trimCsvToken(token));
    if (vals.size() < 2) return {};
    timestamps.push_back(std::stod(vals[1]));
  }

  return timestamps;
}

std::vector<double> loadFrameTimestampsFromCsv(const std::string& dir,
                                               size_t expected_frames) {
  fs::path csv_path = fs::path(dir) / "frame_timestamps.csv";
  if (!fs::exists(csv_path)) return {};

  std::ifstream file(csv_path);
  std::string line;
  if (!std::getline(file, line)) return {};

  std::istringstream header_ss(line);
  std::string col;
  std::vector<std::string> cols;
  while (std::getline(header_ss, col, ',')) cols.push_back(trimCsvToken(col));

  int idx_timestamp = -1;
  for (size_t i = 0; i < cols.size(); i++) {
    if (cols[i] == "timestamp") {
      idx_timestamp = static_cast<int>(i);
      break;
    }
  }
  if (idx_timestamp < 0) idx_timestamp = cols.size() > 1 ? 1 : 0;

  std::vector<double> timestamps;
  timestamps.reserve(expected_frames);
  while (std::getline(file, line) && timestamps.size() < expected_frames) {
    std::istringstream ss(line);
    std::vector<std::string> vals;
    std::string token;
    while (std::getline(ss, token, ',')) vals.push_back(trimCsvToken(token));
    if (idx_timestamp >= static_cast<int>(vals.size())) continue;
    timestamps.push_back(std::stod(vals[idx_timestamp]));
  }

  if (timestamps.size() != expected_frames) return {};
  return timestamps;
}

FrameGapStats computeFrameGapStats(const std::vector<double>& frame_timestamps) {
  FrameGapStats stats;
  if (frame_timestamps.size() < 2) return stats;

  std::vector<double> gaps;
  gaps.reserve(frame_timestamps.size() - 1);
  for (size_t i = 1; i < frame_timestamps.size(); i++) {
    gaps.push_back(std::max(frame_timestamps[i] - frame_timestamps[i - 1], 0.0));
  }

  auto sorted_gaps = gaps;
  std::sort(sorted_gaps.begin(), sorted_gaps.end());

  stats.valid = true;
  stats.min_gap = *std::min_element(gaps.begin(), gaps.end());
  stats.max_gap = *std::max_element(gaps.begin(), gaps.end());
  double sum = 0.0;
  for (double gap : gaps) sum += gap;
  stats.mean_gap = sum / gaps.size();
  const size_t mid = sorted_gaps.size() / 2;
  if (sorted_gaps.size() % 2 == 0) {
    stats.median_gap = 0.5 * (sorted_gaps[mid - 1] + sorted_gaps[mid]);
  } else {
    stats.median_gap = sorted_gaps[mid];
  }
  return stats;
}

std::string frameTimestampSourceName(FrameTimestampSource source) {
  switch (source) {
    case FrameTimestampSource::kFrameTimestampCsv:
      return "frame_timestamps.csv";
    case FrameTimestampSource::kGraphTrajectory:
      return "graph/trajectory.csv";
    case FrameTimestampSource::kSampledGT:
      return "sampled GT timestamps";
  }
  return "unknown";
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

int frameToGTIndex(size_t frame_idx, size_t total_pcd_frames, size_t num_gt_poses) {
  if (num_gt_poses == 0) return 0;
  if (num_gt_poses == total_pcd_frames) {
    return std::min(static_cast<int>(frame_idx), static_cast<int>(num_gt_poses) - 1);
  }
  int gt_idx = static_cast<int>(
      static_cast<double>(frame_idx) / std::max<size_t>(total_pcd_frames, 1) * num_gt_poses);
  return std::min(gt_idx, static_cast<int>(num_gt_poses) - 1);
}

std::vector<double> sampleFrameTimestamps(const std::vector<GTPose>& gt_poses_raw,
                                          size_t num_frames,
                                          size_t total_pcd_frames) {
  std::vector<double> timestamps;
  timestamps.reserve(num_frames);
  for (size_t i = 0; i < num_frames; i++) {
    int gt_idx = frameToGTIndex(i, total_pcd_frames, gt_poses_raw.size());
    timestamps.push_back(gt_poses_raw[gt_idx].timestamp);
  }
  return timestamps;
}

std::vector<Eigen::Matrix4d> sampleGTPoseMatrices(
    const std::vector<GTPose>& gt_poses_raw,
    const std::vector<double>& frame_timestamps) {
  std::vector<Eigen::Matrix4d> gt;
  if (gt_poses_raw.empty()) return gt;

  std::vector<double> gt_times;
  gt_times.reserve(gt_poses_raw.size());
  for (const auto& gp : gt_poses_raw) gt_times.push_back(gp.timestamp);

  Eigen::Matrix4d T0_inv = gt_poses_raw.front().toMatrix().inverse();
  gt.reserve(frame_timestamps.size());
  for (double ts : frame_timestamps) {
    auto it = std::lower_bound(gt_times.begin(), gt_times.end(), ts);
    size_t idx = 0;
    if (it == gt_times.end()) {
      idx = gt_times.size() - 1;
    } else if (it == gt_times.begin()) {
      idx = 0;
    } else {
      size_t hi = static_cast<size_t>(it - gt_times.begin());
      size_t lo = hi - 1;
      idx = std::abs(gt_times[hi] - ts) < std::abs(gt_times[lo] - ts) ? hi : lo;
    }
    gt.push_back(T0_inv * gt_poses_raw[idx].toMatrix());
  }
  return gt;
}

std::vector<localization_zoo::imu_preintegration::ImuSample> selectImuWindow(
    const std::vector<ImuSampleCsv>& imu_samples, double start_time, double end_time) {
  std::vector<localization_zoo::imu_preintegration::ImuSample> out;
  for (const auto& sample : imu_samples) {
    if (sample.timestamp < start_time) continue;
    if (sample.timestamp > end_time) break;

    localization_zoo::imu_preintegration::ImuSample imu;
    imu.timestamp = sample.timestamp;
    imu.gyro = sample.gyro;
    imu.accel = sample.accel;
    out.push_back(imu);
  }
  return out;
}

// ============================================================

struct MethodResult {
  std::string name;
  std::vector<Eigen::Matrix4d> poses;
  double time_ms = 0;
  double ate = 0;
  bool skipped = false;
  std::string note;
};

MethodResult makeSkippedResult(const std::string& name, const std::string& note) {
  MethodResult res;
  res.name = name;
  res.skipped = true;
  res.note = note;
  return res;
}

/// ワールド座標に変換
std::vector<Eigen::Vector3d> transformPoints(
    const std::vector<Eigen::Vector3d>& pts, const Eigen::Matrix4d& T) {
  Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  Eigen::Vector3d t = T.block<3, 1>(0, 3);
  std::vector<Eigen::Vector3d> out;
  out.reserve(pts.size());
  for (auto& p : pts) out.push_back(R * p + t);
  return out;
}

void compactPointMap(std::vector<Eigen::Vector3d>& map_points,
                     size_t max_points) {
  if (map_points.size() <= max_points) return;
  map_points = limitPoints(map_points, max_points);
}

void addPointsToMap(std::vector<Eigen::Vector3d>& map_points,
                    const std::vector<Eigen::Vector3d>& local_points,
                    const Eigen::Matrix4d& pose,
                    size_t max_points) {
  auto points_world = transformPoints(local_points, pose);
  map_points.insert(map_points.end(), points_world.begin(), points_world.end());
  compactPointMap(map_points, max_points);
}

Eigen::Matrix4d anchorRelativePose(const Eigen::Matrix4d& anchor_pose,
                                   const Eigen::Matrix4d& relative_pose) {
  return anchor_pose * relative_pose;
}

bool shouldRefreshTargetMap(size_t frame_index, size_t refresh_interval) {
  return frame_index <= 1 || refresh_interval <= 1 ||
         (frame_index % refresh_interval) == 0;
}

MethodResult runLiTAMIN2(const std::vector<std::string>& pcd_dirs,
                          const std::vector<Eigen::Matrix4d>& gt) {
  using namespace localization_zoo::litamin2;
  MethodResult res;
  res.name = "LiTAMIN2";

  LiTAMIN2Params params;
  params.voxel_resolution = 1.0;
  params.min_points_per_voxel = 1;
  params.max_iterations = 20;
  params.use_cov_cost = true;
  LiTAMIN2Registration reg(params);
  constexpr size_t kLiTAMIN2MapMaxPoints = 60000;
  constexpr size_t kLiTAMIN2RefreshInterval = 4;

  // Scan-to-Map: 前フレームまでの点群をワールド座標で蓄積
  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];  // 初期推定にGTを使用
  res.poses.push_back(T_est);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.5), 2500);
    if (pts_local.empty()) continue;

    if (i == 0) {
      // 最初のフレームはマップに追加
      addPointsToMap(map_points, pts_local, T_est, kLiTAMIN2MapMaxPoints);
      reg.setTarget(map_points);
      continue;
    }

    // scan-to-map: local scan を world map に対して初期値付きで最適化
    const Eigen::Matrix4d T_init_guess = gt[i];
    const auto result = reg.align(pts_local, T_init_guess);
    if (result.converged || result.num_iterations > 0) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
    }
    res.poses.push_back(T_est);

    // マップ更新 (推定ポーズで変換した点群を追加)
    addPointsToMap(map_points, pts_local, T_est, kLiTAMIN2MapMaxPoints);
    if (shouldRefreshTargetMap(i, kLiTAMIN2RefreshInterval)) {
      reg.setTarget(map_points);
    }

    if (i % 10 == 0)
      std::cerr << "\r  [LiTAMIN2] " << i << "/" << pcd_dirs.size();
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  return res;
}

MethodResult runGICP(const std::vector<std::string>& pcd_dirs,
                     const std::vector<Eigen::Matrix4d>& gt) {
  using namespace localization_zoo::gicp;
  MethodResult res;
  res.name = "GICP";

  GICPParams params;
  params.k_neighbors = 10;
  params.max_correspondence_distance = 2.5;
  params.max_iterations = 10;
  GICPRegistration reg(params);
  constexpr size_t kGICPMapMaxPoints = 60000;
  constexpr size_t kGICPRefreshInterval = 5;

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];
  res.poses.push_back(T_est);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local =
        limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd", 1.0), 2500);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, kGICPMapMaxPoints);
      reg.setTarget(map_points);
      continue;
    }

    const Eigen::Matrix4d T_init_guess = gt[i];
    const auto result = reg.align(pts_local, T_init_guess);
    if (result.converged || result.num_correspondences > 0) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
    }
    res.poses.push_back(T_est);

    addPointsToMap(map_points, pts_local, T_est, kGICPMapMaxPoints);
    if (shouldRefreshTargetMap(i, kGICPRefreshInterval)) {
      reg.setTarget(map_points);
    }

    if (i % 10 == 0) {
      std::cerr << "\r  [GICP] " << i << "/" << pcd_dirs.size()
                << " corr=" << result.num_correspondences;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Uses GT-seeded scan-to-map initialization in this dogfooding tool.";
  return res;
}

MethodResult runNDT(const std::vector<std::string>& pcd_dirs,
                    const std::vector<Eigen::Matrix4d>& gt) {
  using namespace localization_zoo::ndt;
  MethodResult res;
  res.name = "NDT";

  NDTParams params;
  params.resolution = 1.5;
  params.max_iterations = 6;
  params.step_size = 0.2;
  params.convergence_threshold = 1e-4;
  params.min_points_per_cell = 1;
  NDTRegistration reg(params);
  constexpr size_t kNDTMapMaxPoints = 30000;
  constexpr size_t kNDTRefreshInterval = 10;

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];
  res.poses.push_back(T_est);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local =
        limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.75), 2000);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, kNDTMapMaxPoints);
      reg.setTarget(map_points);
      continue;
    }

    const Eigen::Matrix4d T_init_guess = gt[i];
    const auto result = reg.align(pts_local, T_init_guess);
    if (result.converged || result.iterations > 0) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
    }
    res.poses.push_back(T_est);

    addPointsToMap(map_points, pts_local, T_est, kNDTMapMaxPoints);
    if (shouldRefreshTargetMap(i, kNDTRefreshInterval)) {
      reg.setTarget(map_points);
    }

    if (i % 10 == 0) {
      std::cerr << "\r  [NDT] " << i << "/" << pcd_dirs.size()
                << " score=" << std::fixed << std::setprecision(3)
                << result.score;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Uses GT-seeded scan-to-map initialization in this dogfooding tool.";
  return res;
}

MethodResult runKISSICP(const std::vector<std::string>& pcd_dirs,
                        const std::vector<Eigen::Matrix4d>& gt) {
  using namespace localization_zoo::kiss_icp;
  MethodResult res;
  res.name = "KISS-ICP";

  KISSICPParams params;
  params.voxel_size = 1.5;
  params.initial_threshold = 1.5;
  params.max_points_per_voxel = 10;
  params.max_icp_iterations = 20;
  KISSICPPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local =
        limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.75), 3000);
    if (pts_local.empty()) continue;

    const auto result = pipeline.registerFrame(pts_local);
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [KISS-ICP] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  return res;
}

MethodResult runCTICP(const std::vector<std::string>& pcd_dirs,
                       const std::vector<Eigen::Matrix4d>& gt) {
  using namespace localization_zoo::ct_icp;
  MethodResult res;
  res.name = "CT-ICP";

  CTICPParams params;
  params.voxel_resolution = 1.5;
  params.max_iterations = 8;
  params.ceres_max_iterations = 1;
  params.planarity_threshold = 0.08;
  params.keypoint_voxel_size = 1.25;
  params.max_frames_in_map = 8;
  CTICPRegistration reg(params);

  TrajectoryFrame prev;
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  prev.begin_pose.trans = world_anchor.block<3, 1>(0, 3);
  prev.begin_pose.quat = Eigen::Quaterniond(world_anchor.block<3, 3>(0, 0));
  prev.end_pose = prev.begin_pose;
  res.poses.push_back(world_anchor);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto scan = limitLoadedScan(loadTimedPCD(pcd_dirs[i] + "/cloud.pcd", 0.75), 500);
    if (scan.points.empty()) continue;

    std::vector<TimedPoint> timed;
    const size_t denom = scan.points.size() > 1 ? scan.points.size() - 1 : 1;
    for (size_t j = 0; j < scan.points.size(); j++) {
      TimedPoint tp;
      tp.raw_point = scan.points[j];
      tp.timestamp = scan.has_per_point_time ? scan.relative_times[j]
                                             : static_cast<double>(j) / denom;
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

MethodResult runCTLIO(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const std::vector<double>& frame_timestamps,
                      const std::vector<ImuSampleCsv>& imu_samples,
                      bool estimate_imu_bias,
                      int fixed_lag_state_window,
                      double fixed_lag_velocity_prior_weight,
                      double fixed_lag_gyro_bias_weight_scale,
                      double fixed_lag_accel_bias_weight_scale,
                      double fixed_lag_history_decay,
                      int fixed_lag_outer_iterations,
                      bool fixed_lag_optimize_history) {
  using namespace localization_zoo::ct_lio;
  MethodResult res;
  res.name = "CT-LIO";
  if (estimate_imu_bias) {
    res.name += "+bias";
  }
  if (fixed_lag_state_window > 1) {
    res.name += "+lag";
  }
  if (fixed_lag_optimize_history) {
    res.name += "+smooth";
  }

  CTLIOParams params;
  params.voxel_resolution = 1.0;
  params.max_iterations = 6;
  params.ceres_max_iterations = 2;
  params.planarity_threshold = 0.1;
  params.keypoint_voxel_size = 1.0;
  params.max_frames_in_map = 10;
  params.estimate_imu_bias = estimate_imu_bias;
  if (fixed_lag_state_window > 1) {
    params.fixed_lag_state_window = fixed_lag_state_window;
    params.fixed_lag_velocity_prior_weight = fixed_lag_velocity_prior_weight;
    params.fixed_lag_gyro_bias_weight_scale = fixed_lag_gyro_bias_weight_scale;
    params.fixed_lag_accel_bias_weight_scale = fixed_lag_accel_bias_weight_scale;
    params.fixed_lag_history_decay = fixed_lag_history_decay;
    params.fixed_lag_outer_iterations = fixed_lag_outer_iterations;
    params.fixed_lag_optimize_history = fixed_lag_optimize_history;
  }
  CTLIORegistration reg(params);

  CTLIOState state;
  state.frame.begin_pose.trans = gt[0].block<3, 1>(0, 3);
  state.frame.begin_pose.quat = Eigen::Quaterniond(gt[0].block<3, 3>(0, 0));
  state.frame.end_pose = state.frame.begin_pose;
  res.poses.push_back(gt[0]);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto scan = limitLoadedScan(loadTimedPCD(pcd_dirs[i] + "/cloud.pcd", 1.0), 1200);
    if (scan.points.empty()) continue;

    std::vector<localization_zoo::ct_icp::TimedPoint> timed;
    timed.reserve(scan.points.size());
    const size_t denom = scan.points.size() > 1 ? scan.points.size() - 1 : 1;
    for (size_t j = 0; j < scan.points.size(); j++) {
      localization_zoo::ct_icp::TimedPoint tp;
      tp.raw_point = scan.points[j];
      tp.timestamp = scan.has_per_point_time ? scan.relative_times[j]
                                             : static_cast<double>(j) / denom;
      timed.push_back(tp);
    }

    if (i == 0) {
      std::vector<Eigen::Vector3d> world;
      world.reserve(timed.size());
      for (const auto& tp : timed) {
        world.push_back(state.frame.transformPoint(tp.raw_point, tp.timestamp));
      }
      reg.addPointsToMap(world);
      continue;
    }

    double dt = std::max(frame_timestamps[i] - frame_timestamps[i - 1], 1e-3);
    Eigen::Vector3d gt_velocity =
        (gt[i].block<3, 1>(0, 3) - gt[i - 1].block<3, 1>(0, 3)) / dt;
    CTLIOState init = state;
    init.frame.begin_pose = state.frame.end_pose;
    init.frame.end_pose.trans = gt[i].block<3, 1>(0, 3);
    init.frame.end_pose.quat = Eigen::Quaterniond(gt[i].block<3, 3>(0, 0));
    init.begin_velocity = gt_velocity;
    init.end_velocity = gt_velocity;

    auto imu_window =
        selectImuWindow(imu_samples, frame_timestamps[i - 1], frame_timestamps[i]);
    auto result = reg.registerFrame(timed, init, imu_window);

    std::vector<Eigen::Vector3d> world;
    world.reserve(timed.size());
    for (const auto& tp : timed) {
      world.push_back(
          result.state.frame.transformPoint(tp.raw_point, tp.timestamp));
    }
    reg.addPointsToMap(world);

    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) = result.state.frame.end_pose.quat.toRotationMatrix();
    T.block<3, 1>(0, 3) = result.state.frame.end_pose.trans;
    res.poses.push_back(T);
    state = result.state;

    if (i % 1 == 0) {
      std::cerr << "\r  [CT-LIO] " << i << "/" << pcd_dirs.size()
                << " corr=" << result.num_correspondences;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
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
    std::cerr << "Usage: " << argv[0]
              << " <pcd_dir> <gt_csv> [max_frames] [--force-ct-lio]"
              << " [--methods litamin2,gicp,ndt,kiss_icp,ct_lio,ct_icp]"
              << " [--ct-lio-estimate-bias]"
              << " [--ct-lio-fixed-lag-window N]"
              << " [--ct-lio-fixed-lag-velocity-weight W]"
              << " [--ct-lio-fixed-lag-gyro-bias-scale W]"
              << " [--ct-lio-fixed-lag-accel-bias-scale W]"
              << " [--ct-lio-fixed-lag-history-decay W]"
              << " [--ct-lio-fixed-lag-outer-iterations N]"
              << " [--ct-lio-fixed-lag-smoother]" << std::endl;
    return 1;
  }

  std::string pcd_dir = argv[1];
  std::string gt_csv = argv[2];
  int max_frames = -1;
  bool force_ct_lio = false;
  bool ct_lio_estimate_bias = false;
  int ct_lio_fixed_lag_window = 1;
  double ct_lio_fixed_lag_velocity_weight = 0.0;
  double ct_lio_fixed_lag_gyro_bias_scale = 0.25;
  double ct_lio_fixed_lag_accel_bias_scale = 0.25;
  double ct_lio_fixed_lag_history_decay = 1.0;
  int ct_lio_fixed_lag_outer_iterations = 3;
  bool ct_lio_fixed_lag_smoother = false;
  std::vector<std::string> selected_methods = {
      "litamin2", "gicp", "ndt", "kiss_icp", "ct_lio"};
  for (int i = 3; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--force-ct-lio") {
      force_ct_lio = true;
      continue;
    }
    if (arg == "--ct-lio-estimate-bias") {
      ct_lio_estimate_bias = true;
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-window") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-lio-fixed-lag-window requires an integer value"
                  << std::endl;
        return 1;
      }
      ct_lio_fixed_lag_window = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ct-lio-fixed-lag-window=", 0) == 0) {
      ct_lio_fixed_lag_window = std::max(
          1, std::stoi(arg.substr(std::string("--ct-lio-fixed-lag-window=").size())));
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-velocity-weight") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-lio-fixed-lag-velocity-weight requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_lio_fixed_lag_velocity_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-lio-fixed-lag-velocity-weight=", 0) == 0) {
      ct_lio_fixed_lag_velocity_weight = std::stod(
          arg.substr(std::string("--ct-lio-fixed-lag-velocity-weight=").size()));
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-gyro-bias-scale") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-lio-fixed-lag-gyro-bias-scale requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_lio_fixed_lag_gyro_bias_scale = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-lio-fixed-lag-gyro-bias-scale=", 0) == 0) {
      ct_lio_fixed_lag_gyro_bias_scale = std::stod(
          arg.substr(std::string("--ct-lio-fixed-lag-gyro-bias-scale=").size()));
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-accel-bias-scale") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-lio-fixed-lag-accel-bias-scale requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_lio_fixed_lag_accel_bias_scale = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-lio-fixed-lag-accel-bias-scale=", 0) == 0) {
      ct_lio_fixed_lag_accel_bias_scale = std::stod(
          arg.substr(std::string("--ct-lio-fixed-lag-accel-bias-scale=").size()));
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-history-decay") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-lio-fixed-lag-history-decay requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_lio_fixed_lag_history_decay = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-lio-fixed-lag-history-decay=", 0) == 0) {
      ct_lio_fixed_lag_history_decay =
          std::stod(arg.substr(std::string("--ct-lio-fixed-lag-history-decay=").size()));
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-outer-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-lio-fixed-lag-outer-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      ct_lio_fixed_lag_outer_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ct-lio-fixed-lag-outer-iterations=", 0) == 0) {
      ct_lio_fixed_lag_outer_iterations = std::max(
          1, std::stoi(arg.substr(
                 std::string("--ct-lio-fixed-lag-outer-iterations=").size())));
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-smoother") {
      ct_lio_fixed_lag_smoother = true;
      continue;
    }
    if (arg == "--methods") {
      if (i + 1 >= argc) {
        std::cerr << "--methods requires a comma-separated value" << std::endl;
        return 1;
      }
      selected_methods = splitMethodList(argv[++i]);
      continue;
    }
    if (arg.rfind("--methods=", 0) == 0) {
      selected_methods = splitMethodList(arg.substr(std::string("--methods=").size()));
      continue;
    }
    max_frames = std::stoi(arg);
  }

  if (selected_methods.empty()) {
    std::cerr
        << "No methods selected. Supported methods: litamin2, gicp, ndt, "
        << "kiss_icp, ct_lio, ct_icp"
        << std::endl;
    return 1;
  }
  for (const auto& method : selected_methods) {
    if (!isSupportedMethod(method)) {
      std::cerr << "Unsupported method: " << method
                << " (supported: litamin2, gicp, ndt, kiss_icp, ct_lio, ct_icp)"
                << std::endl;
      return 1;
    }
  }

  auto pcd_dirs = listPCDDirs(pcd_dir);
  size_t total_pcd_frames = pcd_dirs.size();
  if (max_frames > 0 && max_frames < (int)pcd_dirs.size())
    pcd_dirs.resize(max_frames);

  auto gt_poses_raw = loadGTPoses(gt_csv);
  auto frame_timestamps = loadFrameTimestampsFromCsv(pcd_dir, pcd_dirs.size());
  FrameTimestampSource frame_timestamp_source =
      FrameTimestampSource::kFrameTimestampCsv;
  if (frame_timestamps.empty()) {
    frame_timestamps = loadFrameTimestampsFromGraph(pcd_dirs);
    frame_timestamp_source = FrameTimestampSource::kGraphTrajectory;
  }
  if (frame_timestamps.empty()) {
    frame_timestamps =
        sampleFrameTimestamps(gt_poses_raw, pcd_dirs.size(), total_pcd_frames);
    frame_timestamp_source = FrameTimestampSource::kSampledGT;
  }
  auto frame_gap_stats = computeFrameGapStats(frame_timestamps);
  auto gt = sampleGTPoseMatrices(gt_poses_raw, frame_timestamps);

  std::cout << "========================================" << std::endl;
  std::cout << "  PCD Dogfooding" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "PCD frames: " << pcd_dirs.size() << std::endl;
  std::cout << "GT poses: " << gt.size() << std::endl;

  fs::path imu_csv_path = fs::path(pcd_dir) / "imu.csv";
  if (!fs::exists(imu_csv_path)) {
    imu_csv_path = fs::path(pcd_dir).parent_path() / "imu.csv";
  }
  std::vector<ImuSampleCsv> imu_samples;
  if (fs::exists(imu_csv_path)) {
    imu_samples = loadImuCsv(imu_csv_path.string());
    std::cout << "IMU samples: " << imu_samples.size() << std::endl;
  } else {
    std::cout << "IMU samples: 0 (imu.csv not found)" << std::endl;
  }

  std::cout << "Timestamp source: " << frameTimestampSourceName(frame_timestamp_source)
            << std::endl;
  if (frame_gap_stats.valid) {
    std::cout << "Frame gap [s]: min=" << std::fixed << std::setprecision(3)
              << frame_gap_stats.min_gap
              << " median=" << frame_gap_stats.median_gap
              << " mean=" << frame_gap_stats.mean_gap
              << " max=" << frame_gap_stats.max_gap << std::endl;
  } else {
    std::cout << "Frame gap [s]: unavailable" << std::endl;
  }

  // 走行距離
  double dist = 0;
  for (size_t i = 1; i < gt.size(); i++)
    dist += (gt[i].block<3, 1>(0, 3) - gt[i-1].block<3, 1>(0, 3)).norm();
  std::cout << "Trajectory length: " << std::fixed << std::setprecision(1) << dist << " m" << std::endl;

  std::vector<MethodResult> results;

  if (isMethodEnabled(selected_methods, "litamin2")) {
    std::cout << "\nRunning LiTAMIN2..." << std::endl;
    results.push_back(runLiTAMIN2(pcd_dirs, gt));
  }

  if (isMethodEnabled(selected_methods, "gicp")) {
    std::cout << "Running GICP..." << std::endl;
    results.push_back(runGICP(pcd_dirs, gt));
  }

  if (isMethodEnabled(selected_methods, "ndt")) {
    std::cout << "Running NDT..." << std::endl;
    results.push_back(runNDT(pcd_dirs, gt));
  }

  if (isMethodEnabled(selected_methods, "kiss_icp")) {
    std::cout << "Running KISS-ICP..." << std::endl;
    results.push_back(runKISSICP(pcd_dirs, gt));
  }

  constexpr double kCTLIORecommendedMedianGapSec = 0.5;
  if (isMethodEnabled(selected_methods, "ct_lio")) {
    if (imu_samples.empty()) {
      results.push_back(makeSkippedResult(
          "CT-LIO", "imu.csv not found. CT-LIO requires synchronized IMU samples."));
    } else if (!force_ct_lio &&
               frame_timestamp_source != FrameTimestampSource::kSampledGT &&
               frame_gap_stats.valid &&
               frame_gap_stats.median_gap > kCTLIORecommendedMedianGapSec) {
      std::ostringstream oss;
      oss << "median frame gap " << std::fixed << std::setprecision(3)
          << frame_gap_stats.median_gap << " s exceeds CT-LIO threshold "
          << kCTLIORecommendedMedianGapSec
          << " s. Input looks like sparse graph keyframes/submaps, not raw scans. "
          << "Use a dense scan sequence or pass --force-ct-lio to override.";
      results.push_back(makeSkippedResult("CT-LIO", oss.str()));
      std::cout << "Skipping CT-LIO: " << oss.str() << std::endl;
    } else {
      std::cout << "Running CT-LIO..." << std::endl;
      if (ct_lio_fixed_lag_window > 1) {
        std::cout << "  fixed-lag window=" << ct_lio_fixed_lag_window
                  << " velocity_weight=" << ct_lio_fixed_lag_velocity_weight
                  << " gyro_bias_scale=" << ct_lio_fixed_lag_gyro_bias_scale
                  << " accel_bias_scale=" << ct_lio_fixed_lag_accel_bias_scale
                  << " history_decay=" << ct_lio_fixed_lag_history_decay
                  << " outer_iterations=" << ct_lio_fixed_lag_outer_iterations
                  << " smoother="
                  << (ct_lio_fixed_lag_smoother ? "on" : "off")
                  << std::endl;
      }
      results.push_back(
          runCTLIO(pcd_dirs, gt, frame_timestamps, imu_samples,
                   ct_lio_estimate_bias, ct_lio_fixed_lag_window,
                   ct_lio_fixed_lag_velocity_weight,
                   ct_lio_fixed_lag_gyro_bias_scale,
                   ct_lio_fixed_lag_accel_bias_scale,
                   ct_lio_fixed_lag_history_decay,
                   ct_lio_fixed_lag_outer_iterations,
                   ct_lio_fixed_lag_smoother));
    }
  }

  if (isMethodEnabled(selected_methods, "ct_icp")) {
    std::cout << "Running CT-ICP..." << std::endl;
    results.push_back(runCTICP(pcd_dirs, gt));
  }

  // 結果表示
  std::cout << "\n========================================" << std::endl;
  std::cout << "  RESULTS (Real Data)" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << std::left
            << std::setw(24) << "Method"
            << std::setw(12) << "Status"
            << std::setw(12) << "ATE [m]"
            << std::setw(12) << "Frames"
            << std::setw(15) << "Time [ms]"
            << std::setw(12) << "FPS"
            << std::endl;
  std::cout << std::string(87, '-') << std::endl;

  fs::create_directories("dogfooding_results");
  savePosesKITTI(gt, "dogfooding_results/gt.txt");

  for (auto& r : results) {
    if (r.skipped) {
      std::cout << std::setw(24) << r.name
                << std::setw(12) << "SKIPPED"
                << std::setw(12) << "-"
                << std::setw(12) << "-"
                << std::setw(15) << "-"
                << std::setw(12) << "-"
                << std::endl;
      continue;
    }

    r.ate = computeATE(r.poses, gt);
    double fps = r.time_ms > 0.0 ? r.poses.size() / (r.time_ms / 1000.0) : 0.0;
    std::cout << std::setw(24) << r.name
              << std::setw(12) << "OK"
              << std::setw(12) << std::setprecision(3) << r.ate
              << std::setw(12) << r.poses.size()
              << std::setw(15) << std::setprecision(1) << r.time_ms
              << std::setw(12) << std::setprecision(1) << fps
              << std::endl;

    std::string fname = r.name;
    std::replace(fname.begin(), fname.end(), '-', '_');
    savePosesKITTI(r.poses, "dogfooding_results/" + fname + ".txt");
  }

  bool printed_notes = false;
  for (const auto& r : results) {
    if (!r.note.empty()) {
      if (!printed_notes) {
        std::cout << "\nNotes:" << std::endl;
        printed_notes = true;
      }
      std::cout << "- " << r.name << ": " << r.note << std::endl;
    }
  }

  std::cout << "\nResults saved to dogfooding_results/" << std::endl;
  return 0;
}
