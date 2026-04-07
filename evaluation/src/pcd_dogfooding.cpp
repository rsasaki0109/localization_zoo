/// PCD連番データでの Dogfooding ツール
///
/// 使い方:
///   ./pcd_dogfooding <pcd_dir> <gt_csv> [max_frames] [--force-ct-lio]
///   Methods include litamin2,gicp,small_gicp,voxel_gicp,ndt,kiss_icp,dlo,dlio,aloam,floam,lego_loam,mulls,ct_icp,ct_lio,xicp,fast_lio2,hdl_graph_slam,vgicp_slam.
///
/// pcd_dir: 00000000/cloud.pcd, 00000001/cloud.pcd, ... が並ぶディレクトリ
/// gt_csv:  lidar_pose.x,y,z,roll,pitch,yaw を含むCSV

#include "gicp/gicp_registration.h"
#include "kiss_icp/kiss_icp.h"
#include "litamin2/litamin2_registration.h"
#include "ndt/ndt_registration.h"
#include "small_gicp/small_gicp_registration.h"
#include "voxel_gicp/voxel_gicp_registration.h"
#include "aloam/scan_registration.h"
#include "aloam/laser_odometry.h"
#include "aloam/laser_mapping.h"
#include "floam/floam.h"
#include "dlo/dlo.h"
#include "dlio/dlio.h"
#include "lego_loam/lego_loam.h"
#include "mulls/mulls.h"
#include "ct_icp/ct_icp_registration.h"
#include "ct_lio/ct_lio_registration.h"
#include "xicp/xicp_registration.h"
#include "fast_lio2/fast_lio2.h"
#include "hdl_graph_slam/hdl_graph_slam.h"
#include "vgicp_slam/vgicp_slam.h"

#define PCL_NO_PRECOMPILE
#include <pcl/io/pcd_io.h>
#include <pcl/PCLPointCloud2.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/conversions.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/kdtree/kdtree_flann.h>
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
#include <limits>
#include <sstream>
#include <string>
#include <thread>
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
         method == "kiss_icp" || method == "small_gicp" ||
         method == "voxel_gicp" || method == "aloam" || method == "floam" ||
         method == "dlo" || method == "dlio" || method == "lego_loam" ||
         method == "mulls" || method == "ct_lio" || method == "ct_icp" ||
         method == "xicp" || method == "fast_lio2" ||
         method == "hdl_graph_slam" || method == "vgicp_slam";
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

  if (!(leaf > 1e-9)) {
    std::vector<Eigen::Vector3d> points;
    points.reserve(cloud->size());
    for (auto& p : cloud->points) {
      double r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
      if (r > 1.0 && r < 80.0) points.emplace_back(p.x, p.y, p.z);
    }
    return points;
  }

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

pcl::PointCloud<pcl::PointXYZI>::Ptr toPclXYZICloud(
    const std::vector<Eigen::Vector3d>& points) {
  auto cloud = pcl::PointCloud<pcl::PointXYZI>::Ptr(
      new pcl::PointCloud<pcl::PointXYZI>);
  cloud->reserve(points.size());
  for (const auto& p : points) {
    pcl::PointXYZI pt;
    pt.x = static_cast<float>(p.x());
    pt.y = static_cast<float>(p.y());
    pt.z = static_cast<float>(p.z());
    pt.intensity = 0.0f;
    cloud->push_back(pt);
  }
  return cloud;
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

std::string jsonEscape(const std::string& input) {
  std::string escaped;
  escaped.reserve(input.size() + 8);
  for (char c : input) {
    switch (c) {
      case '\\':
        escaped += "\\\\";
        break;
      case '"':
        escaped += "\\\"";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      default:
        escaped += c;
        break;
    }
  }
  return escaped;
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

struct LiTAMIN2DogfoodingOptions {
  double voxel_resolution = 2.0;
  int min_points_per_voxel = 1;
  int max_iterations = 6;
  bool use_cov_cost = true;
  int num_threads =
      static_cast<int>(std::max(1u, std::thread::hardware_concurrency() / 2));
  size_t max_source_points = 2500;
  size_t map_max_points = 45000;
  size_t refresh_interval = 3;
  double map_radius = 45.0;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;
};

struct GICPDogfoodingOptions {
  double source_voxel_size = 1.0;
  size_t max_source_points = 2500;
  int k_neighbors = 8;
  double max_correspondence_distance = 2.5;
  int max_iterations = 8;
  size_t map_max_points = 40000;
  size_t refresh_interval = 4;
  double map_radius = 45.0;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;
};

struct SmallGICPDogfoodingOptions {
  double source_voxel_size = 1.0;
  size_t max_source_points = 2500;
  double voxel_resolution = 1.0;
  int k_neighbors = 12;
  double max_correspondence_distance = 4.0;
  int max_correspondences = 256;
  int max_iterations = 20;
  double covariance_regularization = 1e-3;
  double fitness_epsilon = 1e-6;
  double rotation_epsilon = 1e-4;
  double translation_epsilon = 1e-4;
  size_t map_max_points = 40000;
  size_t refresh_interval = 4;
  double map_radius = 45.0;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;
};

struct VoxelGICPDogfoodingOptions {
  double source_voxel_size = 1.0;
  size_t max_source_points = 2500;
  double voxel_resolution = 1.0;
  int min_points_per_voxel = 1;
  double max_correspondence_distance = 4.0;
  int max_iterations = 20;
  double covariance_regularization = 1e-3;
  double fitness_epsilon = 1e-6;
  double rotation_epsilon = 1e-4;
  double translation_epsilon = 1e-4;
  size_t map_max_points = 40000;
  size_t refresh_interval = 4;
  double map_radius = 45.0;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;
};

struct ALOAMDogfoodingOptions {
  int n_scans = 64;
  float scan_period = 0.1f;
  float minimum_range = 1.0f;
  float curvature_threshold = 0.1f;
  int max_corner_sharp = 2;
  int max_corner_less_sharp = 20;
  int max_surf_flat = 4;
  float less_flat_leaf_size = 0.2f;

  double odom_distance_sq_threshold = 25.0;
  double odom_nearby_scan = 2.5;
  int odom_outer_iters = 2;
  int odom_ceres_iters = 4;
  double odom_huber_loss_s = 0.1;

  double map_line_resolution = 0.4;
  double map_plane_resolution = 0.8;
  int map_outer_iters = 2;
  int map_ceres_iters = 4;
  double map_huber_loss_s = 0.1;
  int map_knn = 5;
  double map_knn_max_dist_sq = 1.0;
  double map_edge_eigenvalue_ratio = 3.0;
  double map_plane_threshold = 0.2;
};

struct FLOAMDogfoodingOptions {
  int n_scans = 64;
  float scan_period = 0.1f;
  float minimum_range = 1.0f;

  size_t input_point_stride = 2;
  int mapping_update_interval = 2;
  bool enable_mapping = true;

  float curvature_threshold = 0.08f;
  int max_corner_sharp = 1;
  int max_corner_less_sharp = 10;
  int max_surf_flat = 2;
  float less_flat_leaf_size = 0.3f;

  double odom_distance_sq_threshold = 16.0;
  int odom_outer_iters = 1;
  int odom_ceres_iters = 3;

  double map_line_resolution = 0.6;
  double map_plane_resolution = 1.0;
  int map_outer_iters = 1;
  int map_ceres_iters = 3;
  double map_knn_max_dist_sq = 4.0;
};

struct LeGOLOAMDogfoodingOptions {
  int n_scans = 64;
  int ground_scan_limit = 6;
  int num_subregions = 6;
  int neighbor_window = 5;
  float minimum_range = 1.0f;
  float maximum_range = 100.0f;
  float sensor_height = 1.8f;
  float ground_height_tolerance = 0.4f;
  float sensor_mount_angle_deg = 0.0f;
  float ground_angle_threshold_deg = 10.0f;
  float scan_period = 0.1f;
  float curvature_threshold = 0.1f;
  int max_corner_sharp = 2;
  int max_corner_less_sharp = 20;
  int max_surf_flat = 4;
  float less_flat_leaf_size = 0.2f;

  double odom_distance_sq_threshold = 25.0;
  double odom_nearby_scan = 2.5;
  int odom_outer_iters = 2;
  int odom_ceres_iters = 4;
  double odom_huber_loss_s = 0.1;

  double map_line_resolution = 0.4;
  double map_plane_resolution = 0.8;
  int map_outer_iters = 2;
  int map_ceres_iters = 4;
  double map_huber_loss_s = 0.1;
  int map_knn = 5;
  double map_knn_max_dist_sq = 1.0;
  double map_edge_eigenvalue_ratio = 3.0;
  double map_plane_threshold = 0.2;
};

struct MULLSDogfoodingOptions {
  int n_scans = 64;
  float scan_period = 0.1f;
  float minimum_range = 1.0f;
  float curvature_threshold = 0.1f;
  int max_corner_sharp = 2;
  int max_corner_less_sharp = 20;
  int max_surf_flat = 4;
  float less_flat_leaf_size = 0.2f;

  double odom_distance_sq_threshold = 25.0;
  double odom_nearby_scan = 2.5;
  int odom_outer_iters = 2;
  int odom_ceres_iters = 4;
  double odom_huber_loss_s = 0.1;

  double line_resolution = 0.4;
  double plane_resolution = 0.8;
  double point_resolution = 1.0;
  int mulls_map_iters = 2;
  int mulls_ceres_iters = 4;
  double mulls_huber_loss_s = 0.1;
  int mulls_knn = 5;
  double mulls_knn_max_dist_sq = 1.0;
  double mulls_edge_eigenvalue_ratio = 3.0;
  double mulls_plane_threshold = 0.2;
  double mulls_point_neighbor_max_dist_sq = 4.0;
  double mulls_line_weight = 1.0;
  double mulls_plane_weight = 1.0;
  double mulls_point_weight = 0.3;
  int mulls_full_downsample_rate = 5;
  int mulls_max_frames_in_map = 30;
};

struct NDTDogfoodingOptions {
  double source_voxel_size = 0.75;
  size_t max_source_points = 2000;
  double resolution = 1.5;
  int max_iterations = 5;
  double step_size = 0.2;
  double convergence_threshold = 1e-4;
  int min_points_per_cell = 1;
  size_t map_max_points = 22000;
  size_t refresh_interval = 8;
  double map_radius = 35.0;
  double max_seed_translation_delta = 1.5;
  double max_seed_rotation_delta_rad = 0.2;
};

struct KISSICPDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  double initial_threshold = 1.5;
  int max_points_per_voxel = 12;
  int max_icp_iterations = 30;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct CTICPDogfoodingOptions {
  double source_voxel_size = 0.75;
  size_t max_source_points = 500;
  double voxel_resolution = 1.5;
  int max_iterations = 8;
  int ceres_max_iterations = 1;
  double planarity_threshold = 0.08;
  double keypoint_voxel_size = 1.25;
  int max_frames_in_map = 8;
};

struct DLODofeedingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.5;
  double map_voxel_size = 0.8;
  double keyframe_translation_threshold = 0.6;
  double keyframe_rotation_threshold_rad = 8.0 * 3.14159265358979323846 / 180.0;
  int max_keyframes_in_map = 30;
  int gicp_k_neighbors = 15;
  double gicp_max_correspondence_distance = 4.0;
  int gicp_max_iterations = 25;
  double gicp_rotation_epsilon = 1e-4;
  double gicp_translation_epsilon = 1e-4;
  double gicp_covariance_regularization = 1e-3;
  double gicp_fitness_epsilon = 1e-6;
};

struct DLIODogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.5;
  double map_voxel_size = 0.8;
  double keyframe_translation_threshold = 0.6;
  double keyframe_rotation_threshold_rad = 8.0 * 3.14159265358979323846 / 180.0;
  int max_keyframes_in_map = 30;
  int gicp_k_neighbors = 15;
  double gicp_max_correspondence_distance = 4.0;
  int gicp_max_iterations = 25;
  double gicp_rotation_epsilon = 1e-4;
  double gicp_translation_epsilon = 1e-4;
  double gicp_covariance_regularization = 1e-3;
  double gicp_fitness_epsilon = 1e-6;
  double imu_rotation_fusion_weight = 0.15;
  double imu_translation_fusion_weight = 0.2;
  double imu_velocity_fusion_weight = 0.2;
  double lidar_confidence_correspondence_scale = 100.0;
};

struct XICPDogfoodingOptions {
  double source_voxel_size = 1.0;
  size_t max_source_points = 2500;
  int max_iterations = 30;
  double convergence_threshold = 1e-5;
  double kappa_f = 0.1736;
  double kappa_s = 0.707;
  int k_neighbors = 10;
  double max_correspondence_distance = 3.0;
  size_t map_max_points = 40000;
  size_t refresh_interval = 4;
  double map_radius = 45.0;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;
};

struct FastLio2DogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  int max_iterations = 8;
  double voxel_resolution = 1.0;
  double keypoint_voxel_size = 0.5;
  double max_correspondence_distance = 3.0;
  double planarity_threshold = 0.1;
  int max_frames_in_map = 30;
};

struct HdlGraphSlamDogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.7;
  double map_voxel_size = 1.0;
  bool enable_loop_closure = true;
};

struct VgicpSlamDogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.4;
  double map_voxel_size = 0.6;
  bool enable_loop_closure = true;
};

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
  const size_t tail_keep = std::min(map_points.size(), max_points * 3 / 4);
  const size_t head_keep = max_points - tail_keep;

  std::vector<Eigen::Vector3d> compacted;
  compacted.reserve(max_points);
  if (head_keep > 0) {
    const size_t head_size = map_points.size() - tail_keep;
    if (head_size > 0) {
      const double step = static_cast<double>(head_size) / head_keep;
      for (size_t i = 0; i < head_keep; i++) {
        size_t idx = std::min(static_cast<size_t>(i * step), head_size - 1);
        compacted.push_back(map_points[idx]);
      }
    }
  }
  compacted.insert(compacted.end(), map_points.end() - tail_keep, map_points.end());
  map_points.swap(compacted);
}

void prunePointMapByRadius(std::vector<Eigen::Vector3d>& map_points,
                           const Eigen::Vector3d& center,
                           double radius) {
  if (radius <= 0.0 || map_points.empty()) return;

  const double radius_sq = radius * radius;
  std::vector<Eigen::Vector3d> filtered;
  filtered.reserve(map_points.size());
  for (const auto& point : map_points) {
    if ((point - center).squaredNorm() <= radius_sq) {
      filtered.push_back(point);
    }
  }
  map_points.swap(filtered);
}

void addPointsToMap(std::vector<Eigen::Vector3d>& map_points,
                    const std::vector<Eigen::Vector3d>& local_points,
                    const Eigen::Matrix4d& pose,
                    size_t max_points,
                    double local_map_radius = 0.0) {
  auto points_world = transformPoints(local_points, pose);
  map_points.insert(map_points.end(), points_world.begin(), points_world.end());
  prunePointMapByRadius(map_points, pose.block<3, 1>(0, 3), local_map_radius);
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

double poseTranslationDelta(const Eigen::Matrix4d& lhs,
                            const Eigen::Matrix4d& rhs) {
  return (lhs.block<3, 1>(0, 3) - rhs.block<3, 1>(0, 3)).norm();
}

double poseRotationDelta(const Eigen::Matrix4d& lhs,
                         const Eigen::Matrix4d& rhs) {
  const Eigen::Matrix3d dR =
      lhs.block<3, 3>(0, 0).transpose() * rhs.block<3, 3>(0, 0);
  Eigen::AngleAxisd aa(dR);
  if (!std::isfinite(aa.angle())) {
    return std::numeric_limits<double>::infinity();
  }
  return std::abs(aa.angle());
}

bool isReasonableRefinement(const Eigen::Matrix4d& refined_pose,
                            const Eigen::Matrix4d& seed_pose,
                            double max_translation_delta,
                            double max_rotation_delta_rad) {
  return poseTranslationDelta(refined_pose, seed_pose) <= max_translation_delta &&
         poseRotationDelta(refined_pose, seed_pose) <= max_rotation_delta_rad;
}

MethodResult runLiTAMIN2(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const LiTAMIN2DogfoodingOptions& options,
                         bool no_gt_seed = false) {
  using namespace localization_zoo::litamin2;
  MethodResult res;
  res.name = options.use_cov_cost ? "LiTAMIN2" : "LiTAMIN2-ICP";

  LiTAMIN2Params params;
  params.voxel_resolution = options.voxel_resolution;
  params.min_points_per_voxel = options.min_points_per_voxel;
  params.max_iterations = options.max_iterations;
  params.use_cov_cost = options.use_cov_cost;
  params.num_threads = options.num_threads;
  LiTAMIN2Registration reg(params);
  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];  // 初期推定にGTを使用
  res.poses.push_back(T_est);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.5),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      reg.setTarget(map_points);
      continue;
    }

    // scan-to-map: local scan を world map に対して初期値付きで最適化
    const Eigen::Matrix4d T_init_guess = no_gt_seed ? T_est : gt[i];
    const auto result = reg.align(pts_local, T_init_guess);
    if ((result.converged || result.num_iterations >= 3) &&
        isReasonableRefinement(result.transformation, T_init_guess,
                               options.max_seed_translation_delta,
                               options.max_seed_rotation_delta_rad)) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
    }
    res.poses.push_back(T_est);

    if (shouldRefreshTargetMap(i, options.refresh_interval)) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      reg.setTarget(map_points);
    }

    if (i % 10 == 0)
      std::cerr << "\r  [LiTAMIN2] " << i << "/" << pcd_dirs.size();
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = no_gt_seed
      ? "Uses odometry-chain scan-to-map initialization (no GT seed)."
      : "Uses GT-seeded scan-to-map initialization with weak-update fallback "
        "in this dogfooding tool.";
  if (!options.use_cov_cost) {
    res.note += " Covariance-shape term disabled.";
  }
  return res;
}

MethodResult runGICP(const std::vector<std::string>& pcd_dirs,
                     const std::vector<Eigen::Matrix4d>& gt,
                     const GICPDogfoodingOptions& options,
                     bool no_gt_seed = false) {
  using namespace localization_zoo::gicp;
  MethodResult res;
  res.name = "GICP";

  GICPParams params;
  params.k_neighbors = options.k_neighbors;
  params.max_correspondence_distance = options.max_correspondence_distance;
  params.max_iterations = options.max_iterations;
  GICPRegistration reg(params);

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];
  res.poses.push_back(T_est);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      reg.setTarget(map_points);
      continue;
    }

    const Eigen::Matrix4d T_init_guess = no_gt_seed ? T_est : gt[i];
    const auto result = reg.align(pts_local, T_init_guess);
    if ((result.converged || result.num_correspondences >= 128) &&
        isReasonableRefinement(result.transformation, T_init_guess,
                               options.max_seed_translation_delta,
                               options.max_seed_rotation_delta_rad)) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
    }
    res.poses.push_back(T_est);

    addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                   options.map_radius);
    if (shouldRefreshTargetMap(i, options.refresh_interval)) {
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
  res.note = no_gt_seed
      ? "Uses odometry-chain scan-to-map initialization (no GT seed)."
      : "Uses GT-seeded scan-to-map initialization with weak-update fallback "
        "in this dogfooding tool.";
  return res;
}

MethodResult runSmallGICP(const std::vector<std::string>& pcd_dirs,
                          const std::vector<Eigen::Matrix4d>& gt,
                          const SmallGICPDogfoodingOptions& options,
                          bool no_gt_seed = false) {
  using namespace localization_zoo::small_gicp;
  MethodResult res;
  res.name = "Small-GICP";

  SmallGICPParams params;
  params.voxel_resolution = options.voxel_resolution;
  params.k_neighbors = options.k_neighbors;
  params.max_correspondence_distance = options.max_correspondence_distance;
  params.max_correspondences = options.max_correspondences;
  params.max_iterations = options.max_iterations;
  params.rotation_epsilon = options.rotation_epsilon;
  params.translation_epsilon = options.translation_epsilon;
  params.covariance_regularization = options.covariance_regularization;
  params.fitness_epsilon = options.fitness_epsilon;
  SmallGICPRegistration reg(params);

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];
  res.poses.push_back(T_est);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      reg.setTarget(map_points);
      continue;
    }

    const Eigen::Matrix4d T_init_guess = no_gt_seed ? T_est : gt[i];
    const auto result = reg.align(pts_local, T_init_guess);
    if ((result.converged || result.num_correspondences >= 96) &&
        isReasonableRefinement(result.transformation, T_init_guess,
                               options.max_seed_translation_delta,
                               options.max_seed_rotation_delta_rad)) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
    }
    res.poses.push_back(T_est);

    addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                   options.map_radius);
    if (shouldRefreshTargetMap(i, options.refresh_interval)) {
      reg.setTarget(map_points);
    }

    if (i % 10 == 0) {
      std::cerr << "\r  [Small-GICP] " << i << "/" << pcd_dirs.size()
                << " corr=" << result.num_correspondences;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = no_gt_seed
      ? "Uses odometry-chain scan-to-map initialization (no GT seed)."
      : "Uses GT-seeded scan-to-map initialization with weak-update fallback "
        "in this dogfooding tool.";
  return res;
}

MethodResult runVoxelGICP(const std::vector<std::string>& pcd_dirs,
                          const std::vector<Eigen::Matrix4d>& gt,
                          const VoxelGICPDogfoodingOptions& options,
                          bool no_gt_seed = false) {
  using namespace localization_zoo::voxel_gicp;
  MethodResult res;
  res.name = "Voxel-GICP";

  VoxelGICPParams params;
  params.voxel_resolution = options.voxel_resolution;
  params.min_points_per_voxel = options.min_points_per_voxel;
  params.max_correspondence_distance = options.max_correspondence_distance;
  params.max_iterations = options.max_iterations;
  params.rotation_epsilon = options.rotation_epsilon;
  params.translation_epsilon = options.translation_epsilon;
  params.covariance_regularization = options.covariance_regularization;
  params.fitness_epsilon = options.fitness_epsilon;
  VoxelGICPRegistration reg(params);

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];
  res.poses.push_back(T_est);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      reg.setTarget(map_points);
      continue;
    }

    const Eigen::Matrix4d T_init_guess = no_gt_seed ? T_est : gt[i];
    const auto result = reg.align(pts_local, T_init_guess);
    if ((result.converged || result.num_correspondences >= 96) &&
        isReasonableRefinement(result.transformation, T_init_guess,
                               options.max_seed_translation_delta,
                               options.max_seed_rotation_delta_rad)) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
    }
    res.poses.push_back(T_est);

    addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                   options.map_radius);
    if (shouldRefreshTargetMap(i, options.refresh_interval)) {
      reg.setTarget(map_points);
    }

    if (i % 10 == 0) {
      std::cerr << "\r  [Voxel-GICP] " << i << "/" << pcd_dirs.size()
                << " corr=" << result.num_correspondences;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = no_gt_seed
      ? "Uses odometry-chain scan-to-map initialization (no GT seed)."
      : "Uses GT-seeded scan-to-map initialization with weak-update fallback "
        "in this dogfooding tool.";
  return res;
}

MethodResult runALOAM(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const ALOAMDogfoodingOptions& options) {
  using namespace localization_zoo::aloam;
  MethodResult res;
  res.name = "A-LOAM";

  ScanRegistrationParams scan_params;
  scan_params.n_scans = options.n_scans;
  scan_params.minimum_range = options.minimum_range;
  scan_params.scan_period = options.scan_period;
  scan_params.curvature_threshold = options.curvature_threshold;
  scan_params.max_corner_sharp = options.max_corner_sharp;
  scan_params.max_corner_less_sharp = options.max_corner_less_sharp;
  scan_params.max_surf_flat = options.max_surf_flat;
  scan_params.less_flat_leaf_size = options.less_flat_leaf_size;
  ScanRegistration scan_reg(scan_params);

  LaserOdometryParams odom_params;
  odom_params.distance_sq_threshold = options.odom_distance_sq_threshold;
  odom_params.nearby_scan = options.odom_nearby_scan;
  odom_params.num_optimization_iters = options.odom_outer_iters;
  odom_params.ceres_max_iterations = options.odom_ceres_iters;
  odom_params.huber_loss_s = options.odom_huber_loss_s;
  odom_params.enable_distortion = false;
  LaserOdometry laser_odom(odom_params);

  LaserMappingParams map_params;
  map_params.line_resolution = options.map_line_resolution;
  map_params.plane_resolution = options.map_plane_resolution;
  map_params.num_optimization_iters = options.map_outer_iters;
  map_params.ceres_max_iterations = options.map_ceres_iters;
  map_params.huber_loss_s = options.map_huber_loss_s;
  map_params.knn = options.map_knn;
  map_params.knn_max_dist_sq = options.map_knn_max_dist_sq;
  map_params.edge_eigenvalue_ratio = options.map_edge_eigenvalue_ratio;
  map_params.plane_threshold = options.map_plane_threshold;
  LaserMapping laser_map(map_params);

  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
  res.poses.push_back(world_anchor);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.0 /* no downsample */);
    if (pts.empty()) continue;

    auto cloud = toPclXYZICloud(pts);
    FeatureCloud features = scan_reg.extract(cloud);

    auto odom = laser_odom.process(features);
    if (!odom.valid) {
      if (i % 10 == 0) std::cerr << "\r  [A-LOAM] " << i << "/" << pcd_dirs.size();
      continue;
    }

    auto mapping = laser_map.process(features.corner_less_sharp,
                                     features.surf_less_flat,
                                     features.full_cloud,
                                     odom.q_w_curr,
                                     odom.t_w_curr);
    if (mapping.valid) {
      T_rel.setIdentity();
      T_rel.block<3, 3>(0, 0) = mapping.q_w_curr.toRotationMatrix();
      T_rel.block<3, 1>(0, 3) = mapping.t_w_curr;
    } else {
      // Fallback: use odometry pose when mapping didn't update.
      T_rel.setIdentity();
      T_rel.block<3, 3>(0, 0) = odom.q_w_curr.toRotationMatrix();
      T_rel.block<3, 1>(0, 3) = odom.t_w_curr;
    }
    res.poses.push_back(anchorRelativePose(world_anchor, T_rel));

    if (i % 10 == 0) std::cerr << "\r  [A-LOAM] " << i << "/" << pcd_dirs.size();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Odometry+mapping pipeline (no GT seed). KITTI assumes n_scans=64.";
  return res;
}

MethodResult runFLOAM(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const FLOAMDogfoodingOptions& options) {
  using namespace localization_zoo::floam;
  MethodResult res;
  res.name = "F-LOAM";

  FLoamParams params;
  params.scan_registration.n_scans = options.n_scans;
  params.scan_registration.scan_period = options.scan_period;
  params.scan_registration.minimum_range = options.minimum_range;
  params.scan_registration.curvature_threshold = options.curvature_threshold;
  params.scan_registration.max_corner_sharp = options.max_corner_sharp;
  params.scan_registration.max_corner_less_sharp = options.max_corner_less_sharp;
  params.scan_registration.max_surf_flat = options.max_surf_flat;
  params.scan_registration.less_flat_leaf_size = options.less_flat_leaf_size;

  params.odometry.distance_sq_threshold = options.odom_distance_sq_threshold;
  params.odometry.num_optimization_iters = options.odom_outer_iters;
  params.odometry.ceres_max_iterations = options.odom_ceres_iters;
  params.odometry.enable_distortion = false;

  params.mapping.line_resolution = options.map_line_resolution;
  params.mapping.plane_resolution = options.map_plane_resolution;
  params.mapping.num_optimization_iters = options.map_outer_iters;
  params.mapping.ceres_max_iterations = options.map_ceres_iters;
  params.mapping.knn_max_dist_sq = options.map_knn_max_dist_sq;

  params.input_point_stride = options.input_point_stride;
  params.mapping_update_interval = options.mapping_update_interval;
  params.enable_mapping = options.enable_mapping;

  FLoam pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  res.poses.push_back(world_anchor);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.0 /* no downsample */);
    if (pts.empty()) continue;
    auto cloud = toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    if (!result.valid) {
      if (i % 10 == 0) std::cerr << "\r  [F-LOAM] " << i << "/" << pcd_dirs.size();
      continue;
    }

    Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
    T_rel.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    T_rel.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, T_rel));

    if (i % 10 == 0) {
      std::cerr << "\r  [F-LOAM] " << i << "/" << pcd_dirs.size()
                << " map_updates=" << result.mapping_updates
                << " stride=" << result.num_input_points << "->"
                << result.num_processed_points;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Fast feature-based odometry+mapping with thinned input and throttled mapping updates (no GT seed).";
  return res;
}

MethodResult runLeGOLOAM(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const LeGOLOAMDogfoodingOptions& options) {
  using namespace localization_zoo::lego_loam;
  MethodResult res;
  res.name = "LeGO-LOAM";

  LeGOLOAMParams params;
  params.scan_registration.n_scans = options.n_scans;
  params.scan_registration.ground_scan_limit = options.ground_scan_limit;
  params.scan_registration.num_subregions = options.num_subregions;
  params.scan_registration.neighbor_window = options.neighbor_window;
  params.scan_registration.minimum_range = options.minimum_range;
  params.scan_registration.maximum_range = options.maximum_range;
  params.scan_registration.sensor_height = options.sensor_height;
  params.scan_registration.ground_height_tolerance =
      options.ground_height_tolerance;
  params.scan_registration.sensor_mount_angle_deg =
      options.sensor_mount_angle_deg;
  params.scan_registration.ground_angle_threshold_deg =
      options.ground_angle_threshold_deg;
  params.scan_registration.scan_period = options.scan_period;
  params.scan_registration.curvature_threshold = options.curvature_threshold;
  params.scan_registration.max_corner_sharp = options.max_corner_sharp;
  params.scan_registration.max_corner_less_sharp =
      options.max_corner_less_sharp;
  params.scan_registration.max_surf_flat = options.max_surf_flat;
  params.scan_registration.less_flat_leaf_size = options.less_flat_leaf_size;

  params.odometry.distance_sq_threshold = options.odom_distance_sq_threshold;
  params.odometry.nearby_scan = options.odom_nearby_scan;
  params.odometry.num_optimization_iters = options.odom_outer_iters;
  params.odometry.ceres_max_iterations = options.odom_ceres_iters;
  params.odometry.huber_loss_s = options.odom_huber_loss_s;
  params.odometry.enable_distortion = false;

  params.mapping.line_resolution = options.map_line_resolution;
  params.mapping.plane_resolution = options.map_plane_resolution;
  params.mapping.num_optimization_iters = options.map_outer_iters;
  params.mapping.ceres_max_iterations = options.map_ceres_iters;
  params.mapping.huber_loss_s = options.map_huber_loss_s;
  params.mapping.knn = options.map_knn;
  params.mapping.knn_max_dist_sq = options.map_knn_max_dist_sq;
  params.mapping.edge_eigenvalue_ratio = options.map_edge_eigenvalue_ratio;
  params.mapping.plane_threshold = options.map_plane_threshold;

  LeGOLOAM pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  res.poses.push_back(world_anchor);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.0 /* no downsample */);
    if (pts.empty()) continue;
    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud =
        toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    if (!result.valid) {
      if (i % 10 == 0) {
        std::cerr << "\r  [LeGO-LOAM] " << i << "/" << pcd_dirs.size();
      }
      continue;
    }

    Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
    T_rel.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    T_rel.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, T_rel));

    if (i % 10 == 0) {
      std::cerr << "\r  [LeGO-LOAM] " << i << "/" << pcd_dirs.size()
                << " ground=" << result.num_ground_points
                << " corner=" << result.num_corner_features
                << " surf=" << result.num_surface_features;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Ground-aware feature odometry+mapping (no GT seed). n_scans=64 matches Velodyne HDL-64E layout.";
  return res;
}

MethodResult runMULLS(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const MULLSDogfoodingOptions& options) {
  using namespace localization_zoo::mulls;
  MethodResult res;
  res.name = "MULLS";

  MULLSParams params;
  params.scan_registration.n_scans = options.n_scans;
  params.scan_registration.minimum_range = options.minimum_range;
  params.scan_registration.scan_period = options.scan_period;
  params.scan_registration.curvature_threshold = options.curvature_threshold;
  params.scan_registration.max_corner_sharp = options.max_corner_sharp;
  params.scan_registration.max_corner_less_sharp = options.max_corner_less_sharp;
  params.scan_registration.max_surf_flat = options.max_surf_flat;
  params.scan_registration.less_flat_leaf_size = options.less_flat_leaf_size;

  params.odometry.distance_sq_threshold = options.odom_distance_sq_threshold;
  params.odometry.nearby_scan = options.odom_nearby_scan;
  params.odometry.num_optimization_iters = options.odom_outer_iters;
  params.odometry.ceres_max_iterations = options.odom_ceres_iters;
  params.odometry.huber_loss_s = options.odom_huber_loss_s;
  params.odometry.enable_distortion = false;

  params.mapping.line_resolution = options.line_resolution;
  params.mapping.plane_resolution = options.plane_resolution;
  params.mapping.point_resolution = options.point_resolution;
  params.mapping.num_optimization_iters = options.mulls_map_iters;
  params.mapping.ceres_max_iterations = options.mulls_ceres_iters;
  params.mapping.huber_loss_s = options.mulls_huber_loss_s;
  params.mapping.knn = options.mulls_knn;
  params.mapping.knn_max_dist_sq = options.mulls_knn_max_dist_sq;
  params.mapping.edge_eigenvalue_ratio = options.mulls_edge_eigenvalue_ratio;
  params.mapping.plane_threshold = options.mulls_plane_threshold;
  params.mapping.point_neighbor_max_dist_sq =
      options.mulls_point_neighbor_max_dist_sq;
  params.mapping.line_weight = options.mulls_line_weight;
  params.mapping.plane_weight = options.mulls_plane_weight;
  params.mapping.point_weight = options.mulls_point_weight;
  params.mapping.full_downsample_rate = options.mulls_full_downsample_rate;
  params.mapping.max_frames_in_map = options.mulls_max_frames_in_map;

  MULLS pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  res.poses.push_back(world_anchor);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.0 /* no downsample */);
    if (pts.empty()) continue;
    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud =
        toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    if (!result.valid) {
      if (i % 10 == 0) {
        std::cerr << "\r  [MULLS] " << i << "/" << pcd_dirs.size();
      }
      continue;
    }

    Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
    T_rel.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    T_rel.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, T_rel));

    if (i % 10 == 0) {
      std::cerr << "\r  [MULLS] " << i << "/" << pcd_dirs.size()
                << " L=" << result.num_line_constraints
                << " P=" << result.num_plane_constraints
                << " Pt=" << result.num_point_constraints
                << " map_frames=" << result.num_frames_in_map;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Multi-metric scan-to-map (line/plane/point Ceres) after A-LOAM-style feature odometry (no GT seed).";
  return res;
}

MethodResult runNDT(const std::vector<std::string>& pcd_dirs,
                    const std::vector<Eigen::Matrix4d>& gt,
                    const NDTDogfoodingOptions& options,
                    bool no_gt_seed = false) {
  using namespace localization_zoo::ndt;
  MethodResult res;
  res.name = "NDT";

  NDTParams params;
  params.resolution = options.resolution;
  params.max_iterations = options.max_iterations;
  params.step_size = options.step_size;
  params.convergence_threshold = options.convergence_threshold;
  params.min_points_per_cell = options.min_points_per_cell;
  NDTRegistration reg(params);

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];
  res.poses.push_back(T_est);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      reg.setTarget(map_points);
      continue;
    }

    const Eigen::Matrix4d T_init_guess = no_gt_seed ? T_est : gt[i];
    const auto result = reg.align(pts_local, T_init_guess);
    if ((result.converged || result.iterations >= 2) &&
        isReasonableRefinement(result.transformation, T_init_guess,
                               options.max_seed_translation_delta,
                               options.max_seed_rotation_delta_rad)) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
    }
    res.poses.push_back(T_est);

    addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                   options.map_radius);
    if (shouldRefreshTargetMap(i, options.refresh_interval)) {
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
  res.note = no_gt_seed
      ? "Uses odometry-chain scan-to-map initialization (no GT seed)."
      : "Uses GT-seeded scan-to-map initialization with weak-update fallback "
        "in this dogfooding tool.";
  return res;
}

MethodResult runKISSICP(const std::vector<std::string>& pcd_dirs,
                        const std::vector<Eigen::Matrix4d>& gt,
                        const KISSICPDogfoodingOptions& options) {
  using namespace localization_zoo::kiss_icp;
  MethodResult res;
  res.name = "KISS-ICP";

  KISSICPParams params;
  params.voxel_size = options.voxel_size;
  params.initial_threshold = options.initial_threshold;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_icp_iterations = options.max_icp_iterations;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  KISSICPPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
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

MethodResult runDLO(const std::vector<std::string>& pcd_dirs,
                    const std::vector<Eigen::Matrix4d>& gt,
                    const DLODofeedingOptions& options) {
  using namespace localization_zoo::dlo;
  MethodResult res;
  res.name = "DLO";

  DLOParams params;
  params.min_range = options.min_range;
  params.max_range = options.max_range;
  params.registration_voxel_size = options.registration_voxel_size;
  params.map_voxel_size = options.map_voxel_size;
  params.keyframe_translation_threshold = options.keyframe_translation_threshold;
  params.keyframe_rotation_threshold_rad = options.keyframe_rotation_threshold_rad;
  params.max_keyframes_in_map = options.max_keyframes_in_map;
  params.gicp.k_neighbors = options.gicp_k_neighbors;
  params.gicp.max_correspondence_distance = options.gicp_max_correspondence_distance;
  params.gicp.max_iterations = options.gicp_max_iterations;
  params.gicp.rotation_epsilon = options.gicp_rotation_epsilon;
  params.gicp.translation_epsilon = options.gicp_translation_epsilon;
  params.gicp.covariance_regularization = options.gicp_covariance_regularization;
  params.gicp.fitness_epsilon = options.gicp_fitness_epsilon;

  DLO pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;

    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud =
        toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [DLO] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " map_pts=" << result.map_points;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Keyframe scan-to-map odometry with GICP alignment (no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runDLIO(const std::vector<std::string>& pcd_dirs,
                     const std::vector<Eigen::Matrix4d>& gt,
                     const std::vector<double>& frame_timestamps,
                     const std::vector<ImuSampleCsv>& imu_samples,
                     const DLIODogfoodingOptions& options) {
  using namespace localization_zoo::dlio;
  MethodResult res;
  res.name = "DLIO";

  DLIOParams params;
  params.min_range = options.min_range;
  params.max_range = options.max_range;
  params.registration_voxel_size = options.registration_voxel_size;
  params.map_voxel_size = options.map_voxel_size;
  params.keyframe_translation_threshold = options.keyframe_translation_threshold;
  params.keyframe_rotation_threshold_rad = options.keyframe_rotation_threshold_rad;
  params.max_keyframes_in_map = options.max_keyframes_in_map;
  params.gicp.k_neighbors = options.gicp_k_neighbors;
  params.gicp.max_correspondence_distance = options.gicp_max_correspondence_distance;
  params.gicp.max_iterations = options.gicp_max_iterations;
  params.gicp.rotation_epsilon = options.gicp_rotation_epsilon;
  params.gicp.translation_epsilon = options.gicp_translation_epsilon;
  params.gicp.covariance_regularization = options.gicp_covariance_regularization;
  params.gicp.fitness_epsilon = options.gicp_fitness_epsilon;
  params.imu_rotation_fusion_weight = options.imu_rotation_fusion_weight;
  params.imu_translation_fusion_weight = options.imu_translation_fusion_weight;
  params.imu_velocity_fusion_weight = options.imu_velocity_fusion_weight;
  params.lidar_confidence_correspondence_scale =
      options.lidar_confidence_correspondence_scale;

  DLIO pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;

    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud =
        toPclXYZICloud(pts);

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() && frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.process(cloud, imu_batch);
    res.poses.push_back(anchorRelativePose(world_anchor, result.state.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [DLIO] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " imu=" << (result.imu_used ? "y" : "n")
                << " corr=" << result.num_correspondences;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Direct LiDAR-inertial odometry: GICP scan-to-map plus IMU preintegration "
      "prior between scans when imu.csv and per-frame timestamps align; otherwise "
      "LiDAR motion prior only (no GT seed).";
  return res;
}

MethodResult runCTICP(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const CTICPDogfoodingOptions& options) {
  using namespace localization_zoo::ct_icp;
  MethodResult res;
  res.name = "CT-ICP";

  CTICPParams params;
  params.voxel_resolution = options.voxel_resolution;
  params.max_iterations = options.max_iterations;
  params.ceres_max_iterations = options.ceres_max_iterations;
  params.planarity_threshold = options.planarity_threshold;
  params.keypoint_voxel_size = options.keypoint_voxel_size;
  params.max_frames_in_map = options.max_frames_in_map;
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
    auto scan = limitLoadedScan(loadTimedPCD(pcd_dirs[i] + "/cloud.pcd",
                                             options.source_voxel_size),
                                options.max_source_points);
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

MethodResult runXICP(const std::vector<std::string>& pcd_dirs,
                     const std::vector<Eigen::Matrix4d>& gt,
                     const XICPDogfoodingOptions& options,
                     bool no_gt_seed = false) {
  using namespace localization_zoo::xicp;
  MethodResult res;
  res.name = "X-ICP";

  XICPParams params;
  params.max_iterations = options.max_iterations;
  params.convergence_threshold = options.convergence_threshold;
  params.kappa_f = options.kappa_f;
  params.kappa_s = options.kappa_s;
  XICPRegistration reg(params);

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];
  res.poses.push_back(T_est);

  // KdTree for NN search in map
  pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;
  pcl::PointCloud<pcl::PointXYZ>::Ptr map_cloud(new pcl::PointCloud<pcl::PointXYZ>);

  auto rebuildKdTree = [&]() {
    map_cloud->clear();
    map_cloud->reserve(map_points.size());
    for (const auto& p : map_points) {
      pcl::PointXYZ pt;
      pt.x = static_cast<float>(p.x());
      pt.y = static_cast<float>(p.y());
      pt.z = static_cast<float>(p.z());
      map_cloud->push_back(pt);
    }
    if (!map_cloud->empty()) {
      kdtree.setInputCloud(map_cloud);
    }
  };

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      rebuildKdTree();
      continue;
    }

    const Eigen::Matrix4d T_init_guess = no_gt_seed ? T_est : gt[i];

    // Transform source points to world frame using initial guess
    Eigen::Matrix3d R_init = T_init_guess.block<3, 3>(0, 0);
    Eigen::Vector3d t_init = T_init_guess.block<3, 1>(0, 3);

    // Build correspondences: for each source point, find nearest map point
    // and compute normal from neighborhood
    std::vector<Correspondence> correspondences;
    if (!map_cloud->empty()) {
      const int K = options.k_neighbors;
      std::vector<int> nn_indices(K);
      std::vector<float> nn_dists(K);
      const double max_dist_sq =
          options.max_correspondence_distance * options.max_correspondence_distance;

      for (const auto& p_local : pts_local) {
        Eigen::Vector3d p_world = R_init * p_local + t_init;
        pcl::PointXYZ query;
        query.x = static_cast<float>(p_world.x());
        query.y = static_cast<float>(p_world.y());
        query.z = static_cast<float>(p_world.z());

        if (kdtree.nearestKSearch(query, K, nn_indices, nn_dists) < 3) continue;
        if (nn_dists[0] > max_dist_sq) continue;

        // Compute normal from neighborhood covariance
        Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
        int valid_nn = std::min(K, static_cast<int>(nn_indices.size()));
        for (int k = 0; k < valid_nn; k++) {
          centroid += map_points[nn_indices[k]];
        }
        centroid /= valid_nn;

        Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
        for (int k = 0; k < valid_nn; k++) {
          Eigen::Vector3d d = map_points[nn_indices[k]] - centroid;
          cov += d * d.transpose();
        }
        cov /= valid_nn;

        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
        Eigen::Vector3d normal = solver.eigenvectors().col(0);  // smallest eigenvalue

        // Ensure consistent normal orientation (pointing toward sensor)
        if (normal.dot(p_world - map_points[nn_indices[0]]) > 0) {
          normal = -normal;
        }

        Correspondence corr;
        corr.source = p_local;
        corr.target = map_points[nn_indices[0]];
        corr.normal = normal;
        correspondences.push_back(corr);
      }
    }

    bool accepted = false;
    if (correspondences.size() >= 64) {
      const auto result = reg.align(correspondences, T_init_guess);
      if ((result.converged || correspondences.size() >= 128) &&
          isReasonableRefinement(result.transformation, T_init_guess,
                                 options.max_seed_translation_delta,
                                 options.max_seed_rotation_delta_rad)) {
        T_est = result.transformation;
        accepted = true;
      }
    }
    if (!accepted) {
      T_est = T_init_guess;
    }
    res.poses.push_back(T_est);

    addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                   options.map_radius);
    if (shouldRefreshTargetMap(i, options.refresh_interval)) {
      rebuildKdTree();
    }

    if (i % 10 == 0) {
      std::cerr << "\r  [X-ICP] " << i << "/" << pcd_dirs.size()
                << " corr=" << correspondences.size();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = no_gt_seed
      ? "Uses odometry-chain scan-to-map initialization (no GT seed)."
      : "Uses GT-seeded scan-to-map initialization with weak-update fallback "
        "in this dogfooding tool.";
  return res;
}

MethodResult runFastLio2(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const std::vector<double>& frame_timestamps,
                         const std::vector<ImuSampleCsv>& imu_samples,
                         const FastLio2DogfoodingOptions& options) {
  using namespace localization_zoo::fast_lio2;
  MethodResult res;
  res.name = "FAST-LIO2";

  FastLio2Params params;
  params.max_iterations = options.max_iterations;
  params.voxel_resolution = options.voxel_resolution;
  params.keypoint_voxel_size = options.keypoint_voxel_size;
  params.max_correspondence_distance = options.max_correspondence_distance;
  params.planarity_threshold = options.planarity_threshold;
  params.max_frames_in_map = options.max_frames_in_map;

  FastLio2 pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;

    // Filter by range
    std::vector<Eigen::Vector3d> filtered;
    filtered.reserve(pts.size());
    for (const auto& p : pts) {
      double r = p.norm();
      if (r >= options.min_range && r <= options.max_range) {
        filtered.push_back(p);
      }
    }

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() && frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.process(filtered, imu_batch);

    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose.block<3, 3>(0, 0) = result.state.quat.toRotationMatrix();
    pose.block<3, 1>(0, 3) = result.state.trans;
    res.poses.push_back(anchorRelativePose(world_anchor, pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [FAST-LIO2] " << i << "/" << pcd_dirs.size()
                << " imu=" << (result.imu_used ? "y" : "n")
                << " corr=" << result.num_correspondences
                << " map=" << result.map_size;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "LiDAR-inertial odometry with iterated Kalman filter and voxel map "
      "(no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runHdlGraphSlam(const std::vector<std::string>& pcd_dirs,
                             const std::vector<Eigen::Matrix4d>& gt,
                             const HdlGraphSlamDogfoodingOptions& options) {
  using namespace localization_zoo::hdl_graph_slam;
  MethodResult res;
  res.name = "HDL-Graph-SLAM";

  HdlGraphSlamParams params;
  params.min_range = options.min_range;
  params.max_range = options.max_range;
  params.registration_voxel_size = options.registration_voxel_size;
  params.map_voxel_size = options.map_voxel_size;
  params.enable_loop_closure = options.enable_loop_closure;

  HdlGraphSlam pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;

    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud =
        toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [HDL-Graph-SLAM] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " loops=" << result.num_loop_edges
                << " submap=" << result.submap_points;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "NDT odometry with pose-graph optimization and ScanContext loop closure "
      "(no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runVgicpSlam(const std::vector<std::string>& pcd_dirs,
                          const std::vector<Eigen::Matrix4d>& gt,
                          const VgicpSlamDogfoodingOptions& options) {
  using namespace localization_zoo::vgicp_slam;
  MethodResult res;
  res.name = "VGICP-SLAM";

  VgicpSlamParams params;
  params.min_range = options.min_range;
  params.max_range = options.max_range;
  params.registration_voxel_size = options.registration_voxel_size;
  params.map_voxel_size = options.map_voxel_size;
  params.enable_loop_closure = options.enable_loop_closure;

  VgicpSlam pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;

    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud =
        toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [VGICP-SLAM] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " loops=" << result.num_loop_edges
                << " submap=" << result.submap_points;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure "
      "(no GT seed; anchor matches first GT pose).";
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

void writeSummaryJson(const std::string& path,
                      const std::string& pcd_dir,
                      const std::string& gt_csv,
                      size_t num_frames,
                      double trajectory_length_m,
                      FrameTimestampSource frame_timestamp_source,
                      const std::vector<MethodResult>& results) {
  std::ofstream out(path);
  out << "{\n";
  out << "  \"pcd_dir\": \"" << jsonEscape(pcd_dir) << "\",\n";
  out << "  \"gt_csv\": \"" << jsonEscape(gt_csv) << "\",\n";
  out << "  \"num_frames\": " << num_frames << ",\n";
  out << "  \"trajectory_length_m\": " << std::fixed << std::setprecision(6)
      << trajectory_length_m << ",\n";
  out << "  \"timestamp_source\": \""
      << jsonEscape(frameTimestampSourceName(frame_timestamp_source)) << "\",\n";
  out << "  \"methods\": [\n";
  for (size_t i = 0; i < results.size(); i++) {
    const auto& r = results[i];
    out << "    {\n";
    out << "      \"name\": \"" << jsonEscape(r.name) << "\",\n";
    out << "      \"status\": \"" << (r.skipped ? "SKIPPED" : "OK") << "\",\n";
    if (r.skipped) {
      out << "      \"ate_m\": null,\n";
      out << "      \"frames\": 0,\n";
      out << "      \"time_ms\": null,\n";
      out << "      \"fps\": null,\n";
    } else {
      const double fps =
          r.time_ms > 0.0 ? r.poses.size() / (r.time_ms / 1000.0) : 0.0;
      out << "      \"ate_m\": " << std::fixed << std::setprecision(6) << r.ate
          << ",\n";
      out << "      \"frames\": " << r.poses.size() << ",\n";
      out << "      \"time_ms\": " << std::fixed << std::setprecision(6)
          << r.time_ms << ",\n";
      out << "      \"fps\": " << std::fixed << std::setprecision(6) << fps
          << ",\n";
    }
    out << "      \"note\": \"" << jsonEscape(r.note) << "\"\n";
    out << "    }" << (i + 1 == results.size() ? "\n" : ",\n");
  }
  out << "  ]\n";
  out << "}\n";
}

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << " <pcd_dir> <gt_csv> [max_frames] [--force-ct-lio]"
              << " [--methods litamin2,gicp,small_gicp,voxel_gicp,ndt,kiss_icp,dlo,dlio,aloam,floam,"
              << "lego_loam,mulls,ct_lio,ct_icp]"
              << " [--summary-json path]"
              << " [--litamin2-paper-profile]"
              << " [--litamin2-icp-only]"
              << " [--litamin2-voxel-resolution X]"
              << " [--litamin2-max-iterations N]"
              << " [--litamin2-max-source-points N]"
              << " [--litamin2-num-threads N]"
              << " [--gicp-fast-profile]"
              << " [--gicp-dense-profile]"
              << " [--gicp-k-neighbors N]"
              << " [--gicp-max-iterations N]"
              << " [--aloam-kitti-profile]"
              << " [--aloam-fast-profile]"
              << " [--aloam-dense-profile]"
              << " [--floam-kitti-profile]"
              << " [--floam-fast-profile]"
              << " [--floam-dense-profile]"
              << " [--lego-loam-kitti-profile]"
              << " [--lego-loam-fast-profile]"
              << " [--lego-loam-dense-profile]"
              << " [--mulls-kitti-profile]"
              << " [--mulls-fast-profile]"
              << " [--mulls-dense-profile]"
              << " [--dlo-kitti-profile]"
              << " [--dlo-fast-profile]"
              << " [--dlo-dense-profile]"
              << " [--dlio-kitti-profile]"
              << " [--dlio-fast-profile]"
              << " [--dlio-dense-profile]"
              << " [--small-gicp-fast-profile]"
              << " [--small-gicp-dense-profile]"
              << " [--voxel-gicp-fast-profile]"
              << " [--voxel-gicp-dense-profile]"
              << " [--voxel-gicp-voxel-resolution X]"
              << " [--voxel-gicp-min-points-per-voxel N]"
              << " [--voxel-gicp-max-correspondence-distance X]"
              << " [--voxel-gicp-max-iterations N]"
              << " [--small-gicp-voxel-resolution X]"
              << " [--small-gicp-k-neighbors N]"
              << " [--small-gicp-max-correspondence-distance X]"
              << " [--small-gicp-max-correspondences N]"
              << " [--small-gicp-max-iterations N]"
              << " [--ndt-fast-profile]"
              << " [--ndt-dense-profile]"
              << " [--ndt-resolution X]"
              << " [--ndt-max-iterations N]"
              << " [--kiss-fast-profile]"
              << " [--kiss-dense-profile]"
              << " [--kiss-voxel-size X]"
              << " [--kiss-max-iterations N]"
              << " [--ct-icp-fast-profile]"
              << " [--ct-icp-dense-profile]"
              << " [--ct-icp-voxel-resolution X]"
              << " [--ct-icp-max-iterations N]"
              << " [--ct-lio-estimate-bias]"
              << " [--ct-lio-fixed-lag-window N]"
              << " [--ct-lio-fixed-lag-velocity-weight W]"
              << " [--ct-lio-fixed-lag-gyro-bias-scale W]"
              << " [--ct-lio-fixed-lag-accel-bias-scale W]"
              << " [--ct-lio-fixed-lag-history-decay W]"
              << " [--ct-lio-fixed-lag-outer-iterations N]"
              << " [--ct-lio-fixed-lag-smoother]"
              << " [--no-gt-seed]" << std::endl;
    return 1;
  }

  std::string pcd_dir = argv[1];
  std::string gt_csv = argv[2];
  int max_frames = -1;
  bool force_ct_lio = false;
  bool no_gt_seed = false;
  bool ct_lio_estimate_bias = false;
  std::string summary_json_path;
  int ct_lio_fixed_lag_window = 1;
  double ct_lio_fixed_lag_velocity_weight = 0.0;
  double ct_lio_fixed_lag_gyro_bias_scale = 0.25;
  double ct_lio_fixed_lag_accel_bias_scale = 0.25;
  double ct_lio_fixed_lag_history_decay = 1.0;
  int ct_lio_fixed_lag_outer_iterations = 3;
  bool ct_lio_fixed_lag_smoother = false;
  LiTAMIN2DogfoodingOptions litamin2_options;
  GICPDogfoodingOptions gicp_options;
  SmallGICPDogfoodingOptions small_gicp_options;
  VoxelGICPDogfoodingOptions voxel_gicp_options;
  ALOAMDogfoodingOptions aloam_options;
  FLOAMDogfoodingOptions floam_options;
  LeGOLOAMDogfoodingOptions lego_loam_options;
  MULLSDogfoodingOptions mulls_options;
  NDTDogfoodingOptions ndt_options;
  KISSICPDogfoodingOptions kiss_icp_options;
  CTICPDogfoodingOptions ct_icp_options;
  DLODofeedingOptions dlo_options;
  DLIODogfoodingOptions dlio_options;
  XICPDogfoodingOptions xicp_options;
  FastLio2DogfoodingOptions fast_lio2_options;
  HdlGraphSlamDogfoodingOptions hdl_graph_slam_options;
  VgicpSlamDogfoodingOptions vgicp_slam_options;
  std::vector<std::string> selected_methods = {
      "litamin2", "gicp", "small_gicp", "aloam", "floam", "ndt", "kiss_icp",
      "ct_lio"};
  for (int i = 3; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--force-ct-lio") {
      force_ct_lio = true;
      continue;
    }
    if (arg == "--no-gt-seed") {
      no_gt_seed = true;
      continue;
    }
    if (arg == "--ct-lio-estimate-bias") {
      ct_lio_estimate_bias = true;
      continue;
    }
    if (arg == "--summary-json") {
      if (i + 1 >= argc) {
        std::cerr << "--summary-json requires a path" << std::endl;
        return 1;
      }
      summary_json_path = argv[++i];
      continue;
    }
    if (arg.rfind("--summary-json=", 0) == 0) {
      summary_json_path = arg.substr(std::string("--summary-json=").size());
      continue;
    }
    if (arg == "--litamin2-paper-profile") {
      litamin2_options.voxel_resolution = 3.0;
      litamin2_options.max_iterations = 8;
      litamin2_options.use_cov_cost = true;
      litamin2_options.map_max_points = 25000;
      litamin2_options.refresh_interval = 2;
      litamin2_options.map_radius = 35.0;
      continue;
    }
    if (arg == "--litamin2-icp-only") {
      litamin2_options.use_cov_cost = false;
      continue;
    }
    if (arg == "--litamin2-voxel-resolution") {
      if (i + 1 >= argc) {
        std::cerr << "--litamin2-voxel-resolution requires a numeric value"
                  << std::endl;
        return 1;
      }
      litamin2_options.voxel_resolution = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--litamin2-voxel-resolution=", 0) == 0) {
      litamin2_options.voxel_resolution = std::stod(
          arg.substr(std::string("--litamin2-voxel-resolution=").size()));
      continue;
    }
    if (arg == "--litamin2-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--litamin2-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      litamin2_options.max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--litamin2-max-iterations=", 0) == 0) {
      litamin2_options.max_iterations = std::max(
          1, std::stoi(arg.substr(std::string("--litamin2-max-iterations=").size())));
      continue;
    }
    if (arg == "--litamin2-max-source-points") {
      if (i + 1 >= argc) {
        std::cerr << "--litamin2-max-source-points requires an integer value"
                  << std::endl;
        return 1;
      }
      litamin2_options.max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--litamin2-max-source-points=", 0) == 0) {
      litamin2_options.max_source_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--litamin2-max-source-points=").size()))));
      continue;
    }
    if (arg == "--litamin2-num-threads") {
      if (i + 1 >= argc) {
        std::cerr << "--litamin2-num-threads requires an integer value"
                  << std::endl;
        return 1;
      }
      litamin2_options.num_threads = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--litamin2-num-threads=", 0) == 0) {
      litamin2_options.num_threads = std::max(
          1, std::stoi(arg.substr(std::string("--litamin2-num-threads=").size())));
      continue;
    }
    if (arg == "--gicp-fast-profile") {
      gicp_options.source_voxel_size = 1.25;
      gicp_options.max_source_points = 1800;
      gicp_options.k_neighbors = 6;
      gicp_options.max_correspondence_distance = 2.0;
      gicp_options.max_iterations = 6;
      gicp_options.map_max_points = 30000;
      gicp_options.refresh_interval = 6;
      gicp_options.map_radius = 35.0;
      continue;
    }
    if (arg == "--gicp-dense-profile") {
      gicp_options.source_voxel_size = 0.75;
      gicp_options.max_source_points = 3200;
      gicp_options.k_neighbors = 12;
      gicp_options.max_correspondence_distance = 3.0;
      gicp_options.max_iterations = 10;
      gicp_options.map_max_points = 55000;
      gicp_options.refresh_interval = 3;
      gicp_options.map_radius = 55.0;
      continue;
    }
    if (arg == "--gicp-source-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-source-voxel-size requires a numeric value"
                  << std::endl;
        return 1;
      }
      gicp_options.source_voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--gicp-source-voxel-size=", 0) == 0) {
      gicp_options.source_voxel_size =
          std::stod(arg.substr(std::string("--gicp-source-voxel-size=").size()));
      continue;
    }
    if (arg == "--gicp-max-source-points") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-max-source-points requires an integer value"
                  << std::endl;
        return 1;
      }
      gicp_options.max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--gicp-max-source-points=", 0) == 0) {
      gicp_options.max_source_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--gicp-max-source-points=").size()))));
      continue;
    }
    if (arg == "--gicp-k-neighbors") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-k-neighbors requires an integer value"
                  << std::endl;
        return 1;
      }
      gicp_options.k_neighbors = std::max(3, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--gicp-k-neighbors=", 0) == 0) {
      gicp_options.k_neighbors =
          std::max(3, std::stoi(arg.substr(std::string("--gicp-k-neighbors=").size())));
      continue;
    }
    if (arg == "--gicp-max-correspondence-distance") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-max-correspondence-distance requires a numeric value"
                  << std::endl;
        return 1;
      }
      gicp_options.max_correspondence_distance = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--gicp-max-correspondence-distance=", 0) == 0) {
      gicp_options.max_correspondence_distance = std::stod(
          arg.substr(std::string("--gicp-max-correspondence-distance=").size()));
      continue;
    }
    if (arg == "--gicp-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      gicp_options.max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--gicp-max-iterations=", 0) == 0) {
      gicp_options.max_iterations = std::max(
          1, std::stoi(arg.substr(std::string("--gicp-max-iterations=").size())));
      continue;
    }
    if (arg == "--gicp-map-max-points") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-map-max-points requires an integer value"
                  << std::endl;
        return 1;
      }
      gicp_options.map_max_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--gicp-map-max-points=", 0) == 0) {
      gicp_options.map_max_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(std::string("--gicp-map-max-points=").size()))));
      continue;
    }
    if (arg == "--gicp-refresh-interval") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-refresh-interval requires an integer value"
                  << std::endl;
        return 1;
      }
      gicp_options.refresh_interval =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--gicp-refresh-interval=", 0) == 0) {
      gicp_options.refresh_interval = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(std::string("--gicp-refresh-interval=").size()))));
      continue;
    }
    if (arg == "--gicp-map-radius") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-map-radius requires a numeric value" << std::endl;
        return 1;
      }
      gicp_options.map_radius = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--gicp-map-radius=", 0) == 0) {
      gicp_options.map_radius =
          std::stod(arg.substr(std::string("--gicp-map-radius=").size()));
      continue;
    }
    if (arg == "--aloam-kitti-profile") {
      aloam_options.n_scans = 64;
      aloam_options.scan_period = 0.1f;
      aloam_options.minimum_range = 1.0f;
      aloam_options.curvature_threshold = 0.1f;
      aloam_options.less_flat_leaf_size = 0.2f;
      aloam_options.odom_outer_iters = 2;
      aloam_options.odom_ceres_iters = 4;
      aloam_options.map_outer_iters = 2;
      aloam_options.map_ceres_iters = 4;
      aloam_options.map_line_resolution = 0.4;
      aloam_options.map_plane_resolution = 0.8;
      aloam_options.map_knn = 5;
      aloam_options.map_knn_max_dist_sq = 1.0;
      aloam_options.map_edge_eigenvalue_ratio = 3.0;
      aloam_options.map_plane_threshold = 0.2;
      continue;
    }
    if (arg == "--aloam-fast-profile") {
      aloam_options.curvature_threshold = 0.15f;
      aloam_options.less_flat_leaf_size = 0.35f;
      aloam_options.odom_outer_iters = 1;
      aloam_options.odom_ceres_iters = 3;
      aloam_options.map_outer_iters = 1;
      aloam_options.map_ceres_iters = 3;
      aloam_options.map_line_resolution = 0.6;
      aloam_options.map_plane_resolution = 1.2;
      aloam_options.map_knn = 5;
      aloam_options.map_knn_max_dist_sq = 1.5;
      aloam_options.map_plane_threshold = 0.25;
      continue;
    }
    if (arg == "--aloam-dense-profile") {
      aloam_options.curvature_threshold = 0.08f;
      aloam_options.less_flat_leaf_size = 0.15f;
      aloam_options.odom_outer_iters = 2;
      aloam_options.odom_ceres_iters = 6;
      aloam_options.map_outer_iters = 2;
      aloam_options.map_ceres_iters = 6;
      aloam_options.map_line_resolution = 0.3;
      aloam_options.map_plane_resolution = 0.6;
      aloam_options.map_knn = 7;
      aloam_options.map_knn_max_dist_sq = 1.0;
      aloam_options.map_plane_threshold = 0.18;
      continue;
    }
    if (arg == "--floam-kitti-profile") {
      floam_options.n_scans = 64;
      floam_options.scan_period = 0.1f;
      floam_options.minimum_range = 1.0f;
      floam_options.input_point_stride = 2;
      floam_options.mapping_update_interval = 2;
      floam_options.enable_mapping = true;
      continue;
    }
    if (arg == "--floam-fast-profile") {
      floam_options.input_point_stride = 4;
      floam_options.mapping_update_interval = 3;
      floam_options.curvature_threshold = 0.12f;
      floam_options.less_flat_leaf_size = 0.4f;
      floam_options.odom_outer_iters = 1;
      floam_options.odom_ceres_iters = 2;
      floam_options.map_outer_iters = 1;
      floam_options.map_ceres_iters = 2;
      floam_options.map_line_resolution = 0.8;
      floam_options.map_plane_resolution = 1.4;
      floam_options.map_knn_max_dist_sq = 5.0;
      continue;
    }
    if (arg == "--floam-dense-profile") {
      floam_options.input_point_stride = 1;
      floam_options.mapping_update_interval = 1;
      floam_options.curvature_threshold = 0.06f;
      floam_options.less_flat_leaf_size = 0.2f;
      floam_options.odom_outer_iters = 2;
      floam_options.odom_ceres_iters = 4;
      floam_options.map_outer_iters = 2;
      floam_options.map_ceres_iters = 4;
      floam_options.map_line_resolution = 0.5;
      floam_options.map_plane_resolution = 0.9;
      floam_options.map_knn_max_dist_sq = 3.0;
      continue;
    }
    if (arg == "--lego-loam-kitti-profile") {
      lego_loam_options.n_scans = 64;
      lego_loam_options.scan_period = 0.1f;
      lego_loam_options.minimum_range = 1.0f;
      lego_loam_options.maximum_range = 100.0f;
      lego_loam_options.ground_scan_limit = 6;
      lego_loam_options.sensor_height = 1.73f;
      lego_loam_options.ground_height_tolerance = 0.4f;
      lego_loam_options.curvature_threshold = 0.1f;
      lego_loam_options.less_flat_leaf_size = 0.2f;
      lego_loam_options.odom_distance_sq_threshold = 25.0;
      lego_loam_options.odom_outer_iters = 2;
      lego_loam_options.odom_ceres_iters = 4;
      lego_loam_options.map_outer_iters = 2;
      lego_loam_options.map_ceres_iters = 4;
      lego_loam_options.map_line_resolution = 0.4;
      lego_loam_options.map_plane_resolution = 0.8;
      lego_loam_options.map_knn = 5;
      lego_loam_options.map_knn_max_dist_sq = 1.0;
      lego_loam_options.map_edge_eigenvalue_ratio = 3.0;
      lego_loam_options.map_plane_threshold = 0.2;
      continue;
    }
    if (arg == "--lego-loam-fast-profile") {
      lego_loam_options.curvature_threshold = 0.15f;
      lego_loam_options.less_flat_leaf_size = 0.35f;
      lego_loam_options.odom_outer_iters = 1;
      lego_loam_options.odom_ceres_iters = 3;
      lego_loam_options.map_outer_iters = 1;
      lego_loam_options.map_ceres_iters = 3;
      lego_loam_options.map_line_resolution = 0.6;
      lego_loam_options.map_plane_resolution = 1.2;
      lego_loam_options.map_knn_max_dist_sq = 1.5;
      lego_loam_options.map_plane_threshold = 0.25;
      continue;
    }
    if (arg == "--lego-loam-dense-profile") {
      lego_loam_options.curvature_threshold = 0.08f;
      lego_loam_options.less_flat_leaf_size = 0.15f;
      lego_loam_options.odom_outer_iters = 2;
      lego_loam_options.odom_ceres_iters = 6;
      lego_loam_options.map_outer_iters = 2;
      lego_loam_options.map_ceres_iters = 6;
      lego_loam_options.map_line_resolution = 0.3;
      lego_loam_options.map_plane_resolution = 0.6;
      lego_loam_options.map_knn = 7;
      lego_loam_options.map_knn_max_dist_sq = 1.0;
      lego_loam_options.map_plane_threshold = 0.18;
      continue;
    }
    if (arg == "--mulls-kitti-profile") {
      mulls_options.n_scans = 64;
      mulls_options.scan_period = 0.1f;
      mulls_options.minimum_range = 1.0f;
      mulls_options.curvature_threshold = 0.1f;
      mulls_options.less_flat_leaf_size = 0.2f;
      mulls_options.odom_distance_sq_threshold = 25.0;
      mulls_options.odom_outer_iters = 2;
      mulls_options.odom_ceres_iters = 4;
      mulls_options.line_resolution = 0.4;
      mulls_options.plane_resolution = 0.8;
      mulls_options.point_resolution = 1.0;
      mulls_options.mulls_map_iters = 2;
      mulls_options.mulls_ceres_iters = 4;
      mulls_options.mulls_knn = 5;
      mulls_options.mulls_knn_max_dist_sq = 1.0;
      mulls_options.mulls_point_neighbor_max_dist_sq = 4.0;
      mulls_options.mulls_point_weight = 0.3;
      mulls_options.mulls_full_downsample_rate = 5;
      mulls_options.mulls_max_frames_in_map = 30;
      continue;
    }
    if (arg == "--mulls-fast-profile") {
      mulls_options.curvature_threshold = 0.15f;
      mulls_options.less_flat_leaf_size = 0.35f;
      mulls_options.odom_outer_iters = 1;
      mulls_options.odom_ceres_iters = 3;
      mulls_options.line_resolution = 0.55;
      mulls_options.plane_resolution = 1.0;
      mulls_options.point_resolution = 1.2;
      mulls_options.mulls_map_iters = 1;
      mulls_options.mulls_ceres_iters = 3;
      mulls_options.mulls_knn_max_dist_sq = 1.5;
      mulls_options.mulls_point_neighbor_max_dist_sq = 5.0;
      mulls_options.mulls_point_weight = 0.2;
      mulls_options.mulls_full_downsample_rate = 8;
      mulls_options.mulls_max_frames_in_map = 20;
      continue;
    }
    if (arg == "--mulls-dense-profile") {
      mulls_options.curvature_threshold = 0.08f;
      mulls_options.less_flat_leaf_size = 0.15f;
      mulls_options.odom_outer_iters = 2;
      mulls_options.odom_ceres_iters = 6;
      mulls_options.line_resolution = 0.3;
      mulls_options.plane_resolution = 0.6;
      mulls_options.point_resolution = 0.85;
      mulls_options.mulls_map_iters = 2;
      mulls_options.mulls_ceres_iters = 6;
      mulls_options.mulls_knn = 7;
      mulls_options.mulls_knn_max_dist_sq = 1.0;
      mulls_options.mulls_plane_threshold = 0.18;
      mulls_options.mulls_max_frames_in_map = 40;
      continue;
    }
    if (arg == "--dlo-kitti-profile") {
      dlo_options.input_voxel_size = 0.5;
      dlo_options.max_input_points = 6000;
      dlo_options.min_range = 1.0;
      dlo_options.max_range = 100.0;
      dlo_options.registration_voxel_size = 0.5;
      dlo_options.map_voxel_size = 0.8;
      dlo_options.keyframe_translation_threshold = 0.6;
      dlo_options.keyframe_rotation_threshold_rad =
          8.0 * 3.14159265358979323846 / 180.0;
      dlo_options.max_keyframes_in_map = 30;
      dlo_options.gicp_k_neighbors = 15;
      dlo_options.gicp_max_correspondence_distance = 4.0;
      dlo_options.gicp_max_iterations = 25;
      continue;
    }
    if (arg == "--dlo-fast-profile") {
      dlo_options.input_voxel_size = 0.65;
      dlo_options.max_input_points = 4000;
      dlo_options.registration_voxel_size = 0.85;
      dlo_options.map_voxel_size = 1.1;
      dlo_options.keyframe_translation_threshold = 0.75;
      dlo_options.max_keyframes_in_map = 20;
      dlo_options.gicp_k_neighbors = 12;
      dlo_options.gicp_max_correspondence_distance = 5.5;
      dlo_options.gicp_max_iterations = 14;
      continue;
    }
    if (arg == "--dlo-dense-profile") {
      dlo_options.input_voxel_size = 0.35;
      dlo_options.max_input_points = 9000;
      dlo_options.registration_voxel_size = 0.35;
      dlo_options.map_voxel_size = 0.55;
      dlo_options.keyframe_translation_threshold = 0.45;
      dlo_options.max_keyframes_in_map = 40;
      dlo_options.gicp_k_neighbors = 18;
      dlo_options.gicp_max_correspondence_distance = 3.5;
      dlo_options.gicp_max_iterations = 35;
      continue;
    }
    if (arg == "--dlio-kitti-profile") {
      dlio_options.input_voxel_size = 0.5;
      dlio_options.max_input_points = 6000;
      dlio_options.min_range = 1.0;
      dlio_options.max_range = 100.0;
      dlio_options.registration_voxel_size = 0.5;
      dlio_options.map_voxel_size = 0.8;
      dlio_options.keyframe_translation_threshold = 0.6;
      dlio_options.keyframe_rotation_threshold_rad =
          8.0 * 3.14159265358979323846 / 180.0;
      dlio_options.max_keyframes_in_map = 30;
      dlio_options.gicp_k_neighbors = 15;
      dlio_options.gicp_max_correspondence_distance = 4.0;
      dlio_options.gicp_max_iterations = 25;
      dlio_options.imu_rotation_fusion_weight = 0.15;
      dlio_options.imu_translation_fusion_weight = 0.2;
      dlio_options.imu_velocity_fusion_weight = 0.2;
      dlio_options.lidar_confidence_correspondence_scale = 100.0;
      continue;
    }
    if (arg == "--dlio-fast-profile") {
      dlio_options.input_voxel_size = 0.65;
      dlio_options.max_input_points = 4000;
      dlio_options.registration_voxel_size = 0.85;
      dlio_options.map_voxel_size = 1.1;
      dlio_options.keyframe_translation_threshold = 0.75;
      dlio_options.max_keyframes_in_map = 20;
      dlio_options.gicp_k_neighbors = 12;
      dlio_options.gicp_max_correspondence_distance = 5.5;
      dlio_options.gicp_max_iterations = 14;
      dlio_options.imu_rotation_fusion_weight = 0.08;
      dlio_options.imu_translation_fusion_weight = 0.12;
      dlio_options.imu_velocity_fusion_weight = 0.12;
      dlio_options.lidar_confidence_correspondence_scale = 80.0;
      continue;
    }
    if (arg == "--dlio-dense-profile") {
      dlio_options.input_voxel_size = 0.35;
      dlio_options.max_input_points = 9000;
      dlio_options.registration_voxel_size = 0.35;
      dlio_options.map_voxel_size = 0.55;
      dlio_options.keyframe_translation_threshold = 0.45;
      dlio_options.max_keyframes_in_map = 40;
      dlio_options.gicp_k_neighbors = 18;
      dlio_options.gicp_max_correspondence_distance = 3.5;
      dlio_options.gicp_max_iterations = 35;
      dlio_options.imu_rotation_fusion_weight = 0.2;
      dlio_options.imu_translation_fusion_weight = 0.25;
      dlio_options.imu_velocity_fusion_weight = 0.25;
      dlio_options.lidar_confidence_correspondence_scale = 120.0;
      continue;
    }
    if (arg == "--small-gicp-fast-profile") {
      small_gicp_options.source_voxel_size = 1.5;
      small_gicp_options.max_source_points = 1600;
      small_gicp_options.voxel_resolution = 1.5;
      small_gicp_options.k_neighbors = 10;
      small_gicp_options.max_correspondence_distance = 3.0;
      small_gicp_options.max_correspondences = 160;
      small_gicp_options.max_iterations = 10;
      small_gicp_options.map_max_points = 25000;
      small_gicp_options.refresh_interval = 6;
      small_gicp_options.map_radius = 35.0;
      continue;
    }
    if (arg == "--small-gicp-dense-profile") {
      small_gicp_options.source_voxel_size = 0.75;
      small_gicp_options.max_source_points = 3200;
      small_gicp_options.voxel_resolution = 0.9;
      small_gicp_options.k_neighbors = 14;
      small_gicp_options.max_correspondence_distance = 4.5;
      small_gicp_options.max_correspondences = 320;
      small_gicp_options.max_iterations = 24;
      small_gicp_options.map_max_points = 50000;
      small_gicp_options.refresh_interval = 3;
      small_gicp_options.map_radius = 55.0;
      continue;
    }
    if (arg == "--small-gicp-source-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-source-voxel-size requires a numeric value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.source_voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--small-gicp-source-voxel-size=", 0) == 0) {
      small_gicp_options.source_voxel_size = std::stod(
          arg.substr(std::string("--small-gicp-source-voxel-size=").size()));
      continue;
    }
    if (arg == "--small-gicp-max-source-points") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-max-source-points requires an integer value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--small-gicp-max-source-points=", 0) == 0) {
      small_gicp_options.max_source_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--small-gicp-max-source-points=").size()))));
      continue;
    }
    if (arg == "--small-gicp-voxel-resolution") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-voxel-resolution requires a numeric value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.voxel_resolution = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--small-gicp-voxel-resolution=", 0) == 0) {
      small_gicp_options.voxel_resolution = std::stod(
          arg.substr(std::string("--small-gicp-voxel-resolution=").size()));
      continue;
    }
    if (arg == "--small-gicp-k-neighbors") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-k-neighbors requires an integer value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.k_neighbors = std::max(3, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--small-gicp-k-neighbors=", 0) == 0) {
      small_gicp_options.k_neighbors = std::max(
          3, std::stoi(arg.substr(std::string("--small-gicp-k-neighbors=").size())));
      continue;
    }
    if (arg == "--small-gicp-max-correspondence-distance") {
      if (i + 1 >= argc) {
        std::cerr
            << "--small-gicp-max-correspondence-distance requires a numeric value"
            << std::endl;
        return 1;
      }
      small_gicp_options.max_correspondence_distance = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--small-gicp-max-correspondence-distance=", 0) == 0) {
      small_gicp_options.max_correspondence_distance = std::stod(
          arg.substr(
              std::string("--small-gicp-max-correspondence-distance=").size()));
      continue;
    }
    if (arg == "--small-gicp-max-correspondences") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-max-correspondences requires an integer value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.max_correspondences = std::max(16, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--small-gicp-max-correspondences=", 0) == 0) {
      small_gicp_options.max_correspondences = std::max(
          16, std::stoi(arg.substr(
                  std::string("--small-gicp-max-correspondences=").size())));
      continue;
    }
    if (arg == "--small-gicp-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--small-gicp-max-iterations=", 0) == 0) {
      small_gicp_options.max_iterations = std::max(
          1, std::stoi(arg.substr(std::string("--small-gicp-max-iterations=").size())));
      continue;
    }
    if (arg == "--small-gicp-map-max-points") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-map-max-points requires an integer value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.map_max_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--small-gicp-map-max-points=", 0) == 0) {
      small_gicp_options.map_max_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--small-gicp-map-max-points=").size()))));
      continue;
    }
    if (arg == "--small-gicp-refresh-interval") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-refresh-interval requires an integer value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.refresh_interval =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--small-gicp-refresh-interval=", 0) == 0) {
      small_gicp_options.refresh_interval = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--small-gicp-refresh-interval=").size()))));
      continue;
    }
    if (arg == "--small-gicp-map-radius") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-map-radius requires a numeric value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.map_radius = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--small-gicp-map-radius=", 0) == 0) {
      small_gicp_options.map_radius = std::stod(
          arg.substr(std::string("--small-gicp-map-radius=").size()));
      continue;
    }
    if (arg == "--voxel-gicp-fast-profile") {
      voxel_gicp_options.source_voxel_size = 1.5;
      voxel_gicp_options.max_source_points = 1600;
      voxel_gicp_options.voxel_resolution = 1.5;
      voxel_gicp_options.min_points_per_voxel = 1;
      voxel_gicp_options.max_correspondence_distance = 3.0;
      voxel_gicp_options.max_iterations = 10;
      voxel_gicp_options.map_max_points = 25000;
      voxel_gicp_options.refresh_interval = 6;
      voxel_gicp_options.map_radius = 35.0;
      continue;
    }
    if (arg == "--voxel-gicp-dense-profile") {
      voxel_gicp_options.source_voxel_size = 0.75;
      voxel_gicp_options.max_source_points = 3200;
      voxel_gicp_options.voxel_resolution = 0.9;
      voxel_gicp_options.min_points_per_voxel = 3;
      voxel_gicp_options.max_correspondence_distance = 4.5;
      voxel_gicp_options.max_iterations = 24;
      voxel_gicp_options.map_max_points = 50000;
      voxel_gicp_options.refresh_interval = 3;
      voxel_gicp_options.map_radius = 55.0;
      continue;
    }
    if (arg == "--voxel-gicp-voxel-resolution") {
      if (i + 1 >= argc) {
        std::cerr << "--voxel-gicp-voxel-resolution requires a numeric value"
                  << std::endl;
        return 1;
      }
      voxel_gicp_options.voxel_resolution = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--voxel-gicp-voxel-resolution=", 0) == 0) {
      voxel_gicp_options.voxel_resolution = std::stod(
          arg.substr(std::string("--voxel-gicp-voxel-resolution=").size()));
      continue;
    }
    if (arg == "--voxel-gicp-min-points-per-voxel") {
      if (i + 1 >= argc) {
        std::cerr << "--voxel-gicp-min-points-per-voxel requires an integer value"
                  << std::endl;
        return 1;
      }
      voxel_gicp_options.min_points_per_voxel =
          std::max(2, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--voxel-gicp-min-points-per-voxel=", 0) == 0) {
      voxel_gicp_options.min_points_per_voxel = std::max(
          2, std::stoi(
                 arg.substr(std::string("--voxel-gicp-min-points-per-voxel=").size())));
      continue;
    }
    if (arg == "--voxel-gicp-max-correspondence-distance") {
      if (i + 1 >= argc) {
        std::cerr << "--voxel-gicp-max-correspondence-distance requires a value"
                  << std::endl;
        return 1;
      }
      voxel_gicp_options.max_correspondence_distance = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--voxel-gicp-max-correspondence-distance=", 0) == 0) {
      voxel_gicp_options.max_correspondence_distance = std::stod(
          arg.substr(
              std::string("--voxel-gicp-max-correspondence-distance=").size()));
      continue;
    }
    if (arg == "--voxel-gicp-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--voxel-gicp-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      voxel_gicp_options.max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--voxel-gicp-max-iterations=", 0) == 0) {
      voxel_gicp_options.max_iterations = std::max(
          1, std::stoi(
                 arg.substr(std::string("--voxel-gicp-max-iterations=").size())));
      continue;
    }
    if (arg == "--ndt-fast-profile") {
      ndt_options.source_voxel_size = 1.0;
      ndt_options.max_source_points = 1600;
      ndt_options.resolution = 2.0;
      ndt_options.max_iterations = 4;
      ndt_options.step_size = 0.25;
      ndt_options.map_max_points = 18000;
      ndt_options.refresh_interval = 10;
      ndt_options.map_radius = 30.0;
      continue;
    }
    if (arg == "--ndt-dense-profile") {
      ndt_options.source_voxel_size = 0.6;
      ndt_options.max_source_points = 2600;
      ndt_options.resolution = 1.2;
      ndt_options.max_iterations = 7;
      ndt_options.step_size = 0.15;
      ndt_options.map_max_points = 30000;
      ndt_options.refresh_interval = 6;
      ndt_options.map_radius = 40.0;
      continue;
    }
    if (arg == "--ndt-source-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-source-voxel-size requires a numeric value"
                  << std::endl;
        return 1;
      }
      ndt_options.source_voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ndt-source-voxel-size=", 0) == 0) {
      ndt_options.source_voxel_size =
          std::stod(arg.substr(std::string("--ndt-source-voxel-size=").size()));
      continue;
    }
    if (arg == "--ndt-max-source-points") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-max-source-points requires an integer value"
                  << std::endl;
        return 1;
      }
      ndt_options.max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--ndt-max-source-points=", 0) == 0) {
      ndt_options.max_source_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--ndt-max-source-points=").size()))));
      continue;
    }
    if (arg == "--ndt-resolution") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-resolution requires a numeric value" << std::endl;
        return 1;
      }
      ndt_options.resolution = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ndt-resolution=", 0) == 0) {
      ndt_options.resolution =
          std::stod(arg.substr(std::string("--ndt-resolution=").size()));
      continue;
    }
    if (arg == "--ndt-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      ndt_options.max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ndt-max-iterations=", 0) == 0) {
      ndt_options.max_iterations = std::max(
          1, std::stoi(arg.substr(std::string("--ndt-max-iterations=").size())));
      continue;
    }
    if (arg == "--ndt-step-size") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-step-size requires a numeric value" << std::endl;
        return 1;
      }
      ndt_options.step_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ndt-step-size=", 0) == 0) {
      ndt_options.step_size =
          std::stod(arg.substr(std::string("--ndt-step-size=").size()));
      continue;
    }
    if (arg == "--ndt-map-max-points") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-map-max-points requires an integer value"
                  << std::endl;
        return 1;
      }
      ndt_options.map_max_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--ndt-map-max-points=", 0) == 0) {
      ndt_options.map_max_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(std::string("--ndt-map-max-points=").size()))));
      continue;
    }
    if (arg == "--ndt-refresh-interval") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-refresh-interval requires an integer value"
                  << std::endl;
        return 1;
      }
      ndt_options.refresh_interval =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--ndt-refresh-interval=", 0) == 0) {
      ndt_options.refresh_interval = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(std::string("--ndt-refresh-interval=").size()))));
      continue;
    }
    if (arg == "--ndt-map-radius") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-map-radius requires a numeric value" << std::endl;
        return 1;
      }
      ndt_options.map_radius = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ndt-map-radius=", 0) == 0) {
      ndt_options.map_radius =
          std::stod(arg.substr(std::string("--ndt-map-radius=").size()));
      continue;
    }
    if (arg == "--kiss-fast-profile") {
      kiss_icp_options.source_voxel_size = 0.75;
      kiss_icp_options.max_source_points = 2500;
      kiss_icp_options.voxel_size = 1.25;
      kiss_icp_options.initial_threshold = 1.75;
      kiss_icp_options.max_points_per_voxel = 10;
      kiss_icp_options.max_icp_iterations = 20;
      kiss_icp_options.local_map_radius = 45.0;
      kiss_icp_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--kiss-dense-profile") {
      kiss_icp_options.source_voxel_size = 0.35;
      kiss_icp_options.max_source_points = 6000;
      kiss_icp_options.voxel_size = 0.8;
      kiss_icp_options.initial_threshold = 1.25;
      kiss_icp_options.max_points_per_voxel = 16;
      kiss_icp_options.max_icp_iterations = 40;
      kiss_icp_options.local_map_radius = 80.0;
      kiss_icp_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--kiss-source-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-source-voxel-size requires a numeric value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.source_voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--kiss-source-voxel-size=", 0) == 0) {
      kiss_icp_options.source_voxel_size =
          std::stod(arg.substr(std::string("--kiss-source-voxel-size=").size()));
      continue;
    }
    if (arg == "--kiss-max-source-points") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-max-source-points requires an integer value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--kiss-max-source-points=", 0) == 0) {
      kiss_icp_options.max_source_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--kiss-max-source-points=").size()))));
      continue;
    }
    if (arg == "--kiss-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-voxel-size requires a numeric value" << std::endl;
        return 1;
      }
      kiss_icp_options.voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--kiss-voxel-size=", 0) == 0) {
      kiss_icp_options.voxel_size =
          std::stod(arg.substr(std::string("--kiss-voxel-size=").size()));
      continue;
    }
    if (arg == "--kiss-initial-threshold") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-initial-threshold requires a numeric value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.initial_threshold = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--kiss-initial-threshold=", 0) == 0) {
      kiss_icp_options.initial_threshold = std::stod(
          arg.substr(std::string("--kiss-initial-threshold=").size()));
      continue;
    }
    if (arg == "--kiss-max-points-per-voxel") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-max-points-per-voxel requires an integer value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.max_points_per_voxel = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--kiss-max-points-per-voxel=", 0) == 0) {
      kiss_icp_options.max_points_per_voxel = std::max(
          1, std::stoi(arg.substr(std::string("--kiss-max-points-per-voxel=").size())));
      continue;
    }
    if (arg == "--kiss-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.max_icp_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--kiss-max-iterations=", 0) == 0) {
      kiss_icp_options.max_icp_iterations = std::max(
          1, std::stoi(arg.substr(std::string("--kiss-max-iterations=").size())));
      continue;
    }
    if (arg == "--kiss-local-map-radius") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-local-map-radius requires a numeric value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.local_map_radius = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--kiss-local-map-radius=", 0) == 0) {
      kiss_icp_options.local_map_radius = std::stod(
          arg.substr(std::string("--kiss-local-map-radius=").size()));
      continue;
    }
    if (arg == "--kiss-map-cleanup-interval") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-map-cleanup-interval requires an integer value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.map_cleanup_interval = std::max(0, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--kiss-map-cleanup-interval=", 0) == 0) {
      kiss_icp_options.map_cleanup_interval = std::max(
          0, std::stoi(arg.substr(std::string("--kiss-map-cleanup-interval=").size())));
      continue;
    }
    if (arg == "--ct-icp-fast-profile") {
      ct_icp_options.source_voxel_size = 1.0;
      ct_icp_options.max_source_points = 350;
      ct_icp_options.voxel_resolution = 1.8;
      ct_icp_options.max_iterations = 6;
      ct_icp_options.ceres_max_iterations = 1;
      ct_icp_options.planarity_threshold = 0.10;
      ct_icp_options.keypoint_voxel_size = 1.5;
      ct_icp_options.max_frames_in_map = 6;
      continue;
    }
    if (arg == "--ct-icp-dense-profile") {
      ct_icp_options.source_voxel_size = 0.6;
      ct_icp_options.max_source_points = 700;
      ct_icp_options.voxel_resolution = 1.2;
      ct_icp_options.max_iterations = 10;
      ct_icp_options.ceres_max_iterations = 2;
      ct_icp_options.planarity_threshold = 0.06;
      ct_icp_options.keypoint_voxel_size = 1.0;
      ct_icp_options.max_frames_in_map = 10;
      continue;
    }
    if (arg == "--ct-icp-source-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-source-voxel-size requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.source_voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-source-voxel-size=", 0) == 0) {
      ct_icp_options.source_voxel_size = std::stod(
          arg.substr(std::string("--ct-icp-source-voxel-size=").size()));
      continue;
    }
    if (arg == "--ct-icp-max-source-points") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-max-source-points requires an integer value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--ct-icp-max-source-points=", 0) == 0) {
      ct_icp_options.max_source_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--ct-icp-max-source-points=").size()))));
      continue;
    }
    if (arg == "--ct-icp-voxel-resolution") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-voxel-resolution requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.voxel_resolution = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-voxel-resolution=", 0) == 0) {
      ct_icp_options.voxel_resolution = std::stod(
          arg.substr(std::string("--ct-icp-voxel-resolution=").size()));
      continue;
    }
    if (arg == "--ct-icp-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ct-icp-max-iterations=", 0) == 0) {
      ct_icp_options.max_iterations = std::max(
          1, std::stoi(arg.substr(std::string("--ct-icp-max-iterations=").size())));
      continue;
    }
    if (arg == "--ct-icp-ceres-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-ceres-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.ceres_max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ct-icp-ceres-max-iterations=", 0) == 0) {
      ct_icp_options.ceres_max_iterations = std::max(
          1, std::stoi(arg.substr(
                 std::string("--ct-icp-ceres-max-iterations=").size())));
      continue;
    }
    if (arg == "--ct-icp-planarity-threshold") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-planarity-threshold requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.planarity_threshold = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-planarity-threshold=", 0) == 0) {
      ct_icp_options.planarity_threshold = std::stod(
          arg.substr(std::string("--ct-icp-planarity-threshold=").size()));
      continue;
    }
    if (arg == "--ct-icp-keypoint-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-keypoint-voxel-size requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.keypoint_voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-keypoint-voxel-size=", 0) == 0) {
      ct_icp_options.keypoint_voxel_size = std::stod(
          arg.substr(std::string("--ct-icp-keypoint-voxel-size=").size()));
      continue;
    }
    if (arg == "--ct-icp-max-frames-in-map") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-max-frames-in-map requires an integer value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.max_frames_in_map = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ct-icp-max-frames-in-map=", 0) == 0) {
      ct_icp_options.max_frames_in_map = std::max(
          1, std::stoi(arg.substr(std::string("--ct-icp-max-frames-in-map=").size())));
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
    if (!arg.empty() && arg[0] == '-') {
      std::cerr << "Unknown option: " << arg << std::endl;
      return 1;
    }
    max_frames = std::stoi(arg);
  }

  if (selected_methods.empty()) {
    std::cerr
        << "No methods selected. Supported methods: litamin2, gicp, small_gicp, "
        << "voxel_gicp, ndt, kiss_icp, dlo, dlio, aloam, floam, lego_loam, mulls, "
        << "ct_lio, ct_icp, xicp, fast_lio2, hdl_graph_slam, vgicp_slam"
        << std::endl;
    return 1;
  }
  for (const auto& method : selected_methods) {
    if (!isSupportedMethod(method)) {
      std::cerr << "Unsupported method: " << method
                << " (supported: litamin2, gicp, small_gicp, voxel_gicp, ndt, "
                   "kiss_icp, dlo, dlio, aloam, floam, lego_loam, mulls, ct_lio, "
                   "ct_icp, xicp, fast_lio2, hdl_graph_slam, vgicp_slam)"
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
  if (frame_timestamp_source == FrameTimestampSource::kFrameTimestampCsv &&
      frame_gap_stats.valid &&
      frame_gap_stats.median_gap > 0.5) {
    // Some preprocessed sequences provide sparse/keyframe timestamps that do not
    // line up with the dense scan sequence expected by most methods. When that
    // happens, fall back to index-based GT sampling to keep evaluation aligned.
    frame_timestamps =
        sampleFrameTimestamps(gt_poses_raw, pcd_dirs.size(), total_pcd_frames);
    frame_timestamp_source = FrameTimestampSource::kSampledGT;
    frame_gap_stats = computeFrameGapStats(frame_timestamps);
  }
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

  if (no_gt_seed) {
    std::cout << "[no-gt-seed] Scan-to-map methods will use odometry chain instead of GT initialization." << std::endl;
  }

  if (isMethodEnabled(selected_methods, "litamin2")) {
    std::cout << "\nRunning LiTAMIN2..." << std::endl;
    std::cout << "  voxel_resolution=" << litamin2_options.voxel_resolution
              << " max_iterations=" << litamin2_options.max_iterations
              << " max_source_points=" << litamin2_options.max_source_points
              << " use_cov_cost=" << (litamin2_options.use_cov_cost ? "on" : "off")
              << " num_threads=" << litamin2_options.num_threads << std::endl;
    results.push_back(runLiTAMIN2(pcd_dirs, gt, litamin2_options, no_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "gicp")) {
    std::cout << "Running GICP..." << std::endl;
    std::cout << "  source_voxel_size=" << gicp_options.source_voxel_size
              << " max_source_points=" << gicp_options.max_source_points
              << " k_neighbors=" << gicp_options.k_neighbors
              << " max_iterations=" << gicp_options.max_iterations
              << " map_max_points=" << gicp_options.map_max_points << std::endl;
    results.push_back(runGICP(pcd_dirs, gt, gicp_options, no_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "small_gicp")) {
    std::cout << "Running Small-GICP..." << std::endl;
    std::cout << "  source_voxel_size=" << small_gicp_options.source_voxel_size
              << " max_source_points=" << small_gicp_options.max_source_points
              << " voxel_resolution=" << small_gicp_options.voxel_resolution
              << " k_neighbors=" << small_gicp_options.k_neighbors
              << " max_iterations=" << small_gicp_options.max_iterations
              << " max_correspondences=" << small_gicp_options.max_correspondences
              << " map_max_points=" << small_gicp_options.map_max_points
              << std::endl;
    results.push_back(
        runSmallGICP(pcd_dirs, gt, small_gicp_options, no_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "voxel_gicp")) {
    std::cout << "Running Voxel-GICP..." << std::endl;
    std::cout << "  source_voxel_size=" << voxel_gicp_options.source_voxel_size
              << " max_source_points=" << voxel_gicp_options.max_source_points
              << " voxel_resolution=" << voxel_gicp_options.voxel_resolution
              << " min_points_per_voxel="
              << voxel_gicp_options.min_points_per_voxel
              << " max_iterations=" << voxel_gicp_options.max_iterations
              << " map_max_points=" << voxel_gicp_options.map_max_points
              << std::endl;
    results.push_back(
        runVoxelGICP(pcd_dirs, gt, voxel_gicp_options, no_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "aloam")) {
    std::cout << "Running A-LOAM..." << std::endl;
    std::cout << "  n_scans=" << aloam_options.n_scans
              << " scan_period=" << aloam_options.scan_period
              << " curvature_threshold=" << aloam_options.curvature_threshold
              << " less_flat_leaf_size=" << aloam_options.less_flat_leaf_size
              << " odom_iters=" << aloam_options.odom_outer_iters
              << " map_iters=" << aloam_options.map_outer_iters
              << std::endl;
    results.push_back(runALOAM(pcd_dirs, gt, aloam_options));
  }

  if (isMethodEnabled(selected_methods, "floam")) {
    std::cout << "Running F-LOAM..." << std::endl;
    std::cout << "  n_scans=" << floam_options.n_scans
              << " stride=" << floam_options.input_point_stride
              << " map_interval=" << floam_options.mapping_update_interval
              << " curvature_threshold=" << floam_options.curvature_threshold
              << " odom_iters=" << floam_options.odom_outer_iters
              << " map_iters=" << floam_options.map_outer_iters
              << std::endl;
    results.push_back(runFLOAM(pcd_dirs, gt, floam_options));
  }

  if (isMethodEnabled(selected_methods, "lego_loam")) {
    std::cout << "Running LeGO-LOAM..." << std::endl;
    std::cout << "  n_scans=" << lego_loam_options.n_scans
              << " sensor_height=" << lego_loam_options.sensor_height
              << " ground_scan_limit=" << lego_loam_options.ground_scan_limit
              << " curvature_threshold=" << lego_loam_options.curvature_threshold
              << " odom_iters=" << lego_loam_options.odom_outer_iters
              << " map_iters=" << lego_loam_options.map_outer_iters
              << std::endl;
    results.push_back(runLeGOLOAM(pcd_dirs, gt, lego_loam_options));
  }

  if (isMethodEnabled(selected_methods, "mulls")) {
    std::cout << "Running MULLS..." << std::endl;
    std::cout << "  n_scans=" << mulls_options.n_scans
              << " mulls_map_iters=" << mulls_options.mulls_map_iters
              << " mulls_ceres_iters=" << mulls_options.mulls_ceres_iters
              << " max_frames_in_map=" << mulls_options.mulls_max_frames_in_map
              << " point_w=" << mulls_options.mulls_point_weight << std::endl;
    results.push_back(runMULLS(pcd_dirs, gt, mulls_options));
  }

  if (isMethodEnabled(selected_methods, "ndt")) {
    std::cout << "Running NDT..." << std::endl;
    std::cout << "  source_voxel_size=" << ndt_options.source_voxel_size
              << " max_source_points=" << ndt_options.max_source_points
              << " resolution=" << ndt_options.resolution
              << " max_iterations=" << ndt_options.max_iterations
              << " map_max_points=" << ndt_options.map_max_points << std::endl;
    results.push_back(runNDT(pcd_dirs, gt, ndt_options, no_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "kiss_icp")) {
    std::cout << "Running KISS-ICP..." << std::endl;
    std::cout << "  source_voxel_size=" << kiss_icp_options.source_voxel_size
              << " max_source_points=" << kiss_icp_options.max_source_points
              << " voxel_size=" << kiss_icp_options.voxel_size
              << " max_iterations=" << kiss_icp_options.max_icp_iterations
              << " local_map_radius=" << kiss_icp_options.local_map_radius
              << std::endl;
    results.push_back(runKISSICP(pcd_dirs, gt, kiss_icp_options));
  }

  if (isMethodEnabled(selected_methods, "dlo")) {
    std::cout << "Running DLO..." << std::endl;
    std::cout << "  input_voxel_size=" << dlo_options.input_voxel_size
              << " max_input_points=" << dlo_options.max_input_points
              << " reg_voxel=" << dlo_options.registration_voxel_size
              << " map_voxel=" << dlo_options.map_voxel_size
              << " max_keyframes=" << dlo_options.max_keyframes_in_map
              << " gicp_iters=" << dlo_options.gicp_max_iterations << std::endl;
    results.push_back(runDLO(pcd_dirs, gt, dlo_options));
  }

  if (isMethodEnabled(selected_methods, "dlio")) {
    std::cout << "Running DLIO..." << std::endl;
    std::cout << "  input_voxel_size=" << dlio_options.input_voxel_size
              << " max_input_points=" << dlio_options.max_input_points
              << " reg_voxel=" << dlio_options.registration_voxel_size
              << " map_voxel=" << dlio_options.map_voxel_size
              << " imu_rot_w=" << dlio_options.imu_rotation_fusion_weight
              << " imu_trans_w=" << dlio_options.imu_translation_fusion_weight
              << std::endl;
    results.push_back(runDLIO(pcd_dirs, gt, frame_timestamps, imu_samples,
                              dlio_options));
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
    std::cout << "  source_voxel_size=" << ct_icp_options.source_voxel_size
              << " max_source_points=" << ct_icp_options.max_source_points
              << " voxel_resolution=" << ct_icp_options.voxel_resolution
              << " max_iterations=" << ct_icp_options.max_iterations
              << " max_frames_in_map=" << ct_icp_options.max_frames_in_map
              << std::endl;
    results.push_back(runCTICP(pcd_dirs, gt, ct_icp_options));
  }

  if (isMethodEnabled(selected_methods, "xicp")) {
    std::cout << "Running X-ICP..." << std::endl;
    results.push_back(runXICP(pcd_dirs, gt, xicp_options, no_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "fast_lio2")) {
    std::cout << "Running FAST-LIO2..." << std::endl;
    results.push_back(runFastLio2(pcd_dirs, gt, frame_timestamps, imu_samples, fast_lio2_options));
  }

  if (isMethodEnabled(selected_methods, "hdl_graph_slam")) {
    std::cout << "Running HDL-Graph-SLAM..." << std::endl;
    results.push_back(runHdlGraphSlam(pcd_dirs, gt, hdl_graph_slam_options));
  }

  if (isMethodEnabled(selected_methods, "vgicp_slam")) {
    std::cout << "Running VGICP-SLAM..." << std::endl;
    results.push_back(runVgicpSlam(pcd_dirs, gt, vgicp_slam_options));
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

  if (!summary_json_path.empty()) {
    writeSummaryJson(summary_json_path, pcd_dir, gt_csv, pcd_dirs.size(), dist,
                     frame_timestamp_source, results);
  }

  std::cout << "\nResults saved to dogfooding_results/" << std::endl;
  return 0;
}
