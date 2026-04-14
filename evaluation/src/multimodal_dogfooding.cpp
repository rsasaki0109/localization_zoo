#include "fast_livo2/fast_livo2.h"
#include "lvi_sam/lvi_sam.h"
#include "okvis/okvis.h"
#include "orb_slam3/orb_slam3.h"
#include "r2live/r2live.h"
#include "vins_fusion/vins_fusion.h"

#define PCL_NO_PRECOMPILE
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::high_resolution_clock;

struct GTPose {
  double timestamp = 0.0;
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  double roll = 0.0;
  double pitch = 0.0;
  double yaw = 0.0;

  Eigen::Matrix4d toMatrix() const {
    Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();
    Eigen::AngleAxisd rx(roll, Eigen::Vector3d::UnitX());
    Eigen::AngleAxisd ry(pitch, Eigen::Vector3d::UnitY());
    Eigen::AngleAxisd rz(yaw, Eigen::Vector3d::UnitZ());
    transform.block<3, 3>(0, 0) = (rz * ry * rx).toRotationMatrix();
    transform(0, 3) = x;
    transform(1, 3) = y;
    transform(2, 3) = z;
    return transform;
  }
};

struct ImuSampleCsv {
  double timestamp = 0.0;
  Eigen::Vector3d gyro = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel = Eigen::Vector3d::Zero();
};

struct LandmarkRecord {
  int id = -1;
  Eigen::Vector3d position = Eigen::Vector3d::Zero();
};

struct FrameObservation {
  int landmark_id = -1;
  Eigen::Vector2d pixel = Eigen::Vector2d::Zero();
};

enum class FrameTimestampSource {
  kFrameTimestampCsv,
  kSampledGT,
};

struct MethodResult {
  std::string name;
  std::vector<Eigen::Matrix4d> poses;
  double time_ms = 0.0;
  double ate = 0.0;
  bool skipped = false;
  std::string note;
};

struct CameraOverrides {
  std::optional<double> fx;
  std::optional<double> fy;
  std::optional<double> cx;
  std::optional<double> cy;
  std::optional<int> width;
  std::optional<int> height;
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
  std::istringstream stream(csv);
  std::string token;
  while (std::getline(stream, token, ',')) {
    token = normalizeMethodId(token);
    if (!token.empty()) methods.push_back(token);
  }
  return methods;
}

bool isSupportedMethod(const std::string& method) {
  return method == "vins_fusion" || method == "okvis" ||
         method == "orb_slam3" || method == "lvi_sam" ||
         method == "fast_livo2" || method == "r2live";
}

bool isMethodEnabled(const std::vector<std::string>& methods,
                     const std::string& method) {
  return std::find(methods.begin(), methods.end(), method) != methods.end();
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

MethodResult makeSkippedResult(const std::string& name, const std::string& note) {
  MethodResult result;
  result.name = name;
  result.skipped = true;
  result.note = note;
  return result;
}

std::vector<GTPose> loadGTPoses(const std::string& csv_path) {
  std::vector<GTPose> poses;
  std::ifstream file(csv_path);
  if (!file.is_open()) return poses;

  std::string line;
  if (!std::getline(file, line)) return poses;

  std::istringstream header_stream(line);
  std::string column;
  std::vector<std::string> columns;
  while (std::getline(header_stream, column, ',')) {
    columns.push_back(trimCsvToken(column));
  }

  int idx_ts = -1;
  int idx_x = -1;
  int idx_y = -1;
  int idx_z = -1;
  int idx_roll = -1;
  int idx_pitch = -1;
  int idx_yaw = -1;
  for (size_t i = 0; i < columns.size(); i++) {
    if (columns[i] == "timestamp") idx_ts = static_cast<int>(i);
    if (columns[i] == "lidar_pose.x") idx_x = static_cast<int>(i);
    if (columns[i] == "lidar_pose.y") idx_y = static_cast<int>(i);
    if (columns[i] == "lidar_pose.z") idx_z = static_cast<int>(i);
    if (columns[i] == "lidar_pose.roll") idx_roll = static_cast<int>(i);
    if (columns[i] == "lidar_pose.pitch") idx_pitch = static_cast<int>(i);
    if (columns[i] == "lidar_pose.yaw") idx_yaw = static_cast<int>(i);
  }

  while (std::getline(file, line)) {
    std::istringstream row_stream(line);
    std::vector<std::string> values;
    std::string token;
    while (std::getline(row_stream, token, ',')) {
      values.push_back(trimCsvToken(token));
    }

    if (idx_x < 0 || idx_y < 0 || idx_z < 0 ||
        idx_roll < 0 || idx_pitch < 0 || idx_yaw < 0 ||
        idx_x >= static_cast<int>(values.size()) ||
        idx_y >= static_cast<int>(values.size()) ||
        idx_z >= static_cast<int>(values.size()) ||
        idx_roll >= static_cast<int>(values.size()) ||
        idx_pitch >= static_cast<int>(values.size()) ||
        idx_yaw >= static_cast<int>(values.size())) {
      continue;
    }

    GTPose pose;
    pose.timestamp =
        (idx_ts >= 0 && idx_ts < static_cast<int>(values.size()))
            ? std::stod(values[idx_ts])
            : static_cast<double>(poses.size());
    pose.x = std::stod(values[idx_x]);
    pose.y = std::stod(values[idx_y]);
    pose.z = std::stod(values[idx_z]);
    pose.roll = std::stod(values[idx_roll]);
    pose.pitch = std::stod(values[idx_pitch]);
    pose.yaw = std::stod(values[idx_yaw]);
    poses.push_back(pose);
  }

  return poses;
}

std::vector<ImuSampleCsv> loadImuCsv(const std::string& csv_path) {
  std::vector<ImuSampleCsv> samples;
  std::ifstream file(csv_path);
  if (!file.is_open()) return samples;

  std::string line;
  if (!std::getline(file, line)) return samples;

  std::istringstream header_stream(line);
  std::string column;
  std::vector<std::string> columns;
  while (std::getline(header_stream, column, ',')) {
    columns.push_back(trimCsvToken(column));
  }

  int idx_stamp = -1;
  int idx_gx = -1;
  int idx_gy = -1;
  int idx_gz = -1;
  int idx_ax = -1;
  int idx_ay = -1;
  int idx_az = -1;
  for (size_t i = 0; i < columns.size(); i++) {
    if (columns[i] == "stamp") idx_stamp = static_cast<int>(i);
    if (columns[i] == "angular_vel.x") idx_gx = static_cast<int>(i);
    if (columns[i] == "angular_vel.y") idx_gy = static_cast<int>(i);
    if (columns[i] == "angular_vel.z") idx_gz = static_cast<int>(i);
    if (columns[i] == "linear_acc.x") idx_ax = static_cast<int>(i);
    if (columns[i] == "linear_acc.y") idx_ay = static_cast<int>(i);
    if (columns[i] == "linear_acc.z") idx_az = static_cast<int>(i);
  }

  while (std::getline(file, line)) {
    std::istringstream row_stream(line);
    std::vector<std::string> values;
    std::string token;
    while (std::getline(row_stream, token, ',')) {
      values.push_back(trimCsvToken(token));
    }

    if (idx_stamp < 0 || idx_gx < 0 || idx_gy < 0 || idx_gz < 0 ||
        idx_ax < 0 || idx_ay < 0 || idx_az < 0 ||
        idx_az >= static_cast<int>(values.size())) {
      continue;
    }

    ImuSampleCsv sample;
    sample.timestamp = std::stod(values[idx_stamp]);
    sample.gyro = Eigen::Vector3d(std::stod(values[idx_gx]),
                                  std::stod(values[idx_gy]),
                                  std::stod(values[idx_gz]));
    sample.accel = Eigen::Vector3d(std::stod(values[idx_ax]),
                                   std::stod(values[idx_ay]),
                                   std::stod(values[idx_az]));
    samples.push_back(sample);
  }

  return samples;
}

std::vector<LandmarkRecord> loadLandmarksCsv(const std::string& csv_path) {
  std::vector<LandmarkRecord> landmarks;
  std::ifstream file(csv_path);
  if (!file.is_open()) return landmarks;

  std::string line;
  if (!std::getline(file, line)) return landmarks;

  std::istringstream header_stream(line);
  std::string column;
  std::vector<std::string> columns;
  while (std::getline(header_stream, column, ',')) {
    columns.push_back(normalizeMethodId(column));
  }

  int idx_id = -1;
  int idx_x = -1;
  int idx_y = -1;
  int idx_z = -1;
  for (size_t i = 0; i < columns.size(); i++) {
    if (columns[i] == "landmark_id" || columns[i] == "id") {
      idx_id = static_cast<int>(i);
    }
    if (columns[i] == "x") idx_x = static_cast<int>(i);
    if (columns[i] == "y") idx_y = static_cast<int>(i);
    if (columns[i] == "z") idx_z = static_cast<int>(i);
  }

  while (std::getline(file, line)) {
    std::istringstream row_stream(line);
    std::vector<std::string> values;
    std::string token;
    while (std::getline(row_stream, token, ',')) {
      values.push_back(trimCsvToken(token));
    }
    if (idx_id < 0 || idx_x < 0 || idx_y < 0 || idx_z < 0 ||
        idx_z >= static_cast<int>(values.size())) {
      continue;
    }
    LandmarkRecord landmark;
    landmark.id = std::stoi(values[idx_id]);
    landmark.position = Eigen::Vector3d(std::stod(values[idx_x]),
                                        std::stod(values[idx_y]),
                                        std::stod(values[idx_z]));
    landmarks.push_back(landmark);
  }

  return landmarks;
}

size_t nearestFrameIndex(const std::vector<double>& frame_timestamps,
                         double query_timestamp) {
  if (frame_timestamps.empty()) return 0;
  auto it = std::lower_bound(frame_timestamps.begin(), frame_timestamps.end(),
                             query_timestamp);
  if (it == frame_timestamps.end()) return frame_timestamps.size() - 1;
  if (it == frame_timestamps.begin()) return 0;
  const size_t hi = static_cast<size_t>(it - frame_timestamps.begin());
  const size_t lo = hi - 1;
  return std::abs(frame_timestamps[hi] - query_timestamp) <
                 std::abs(frame_timestamps[lo] - query_timestamp)
             ? hi
             : lo;
}

std::vector<std::vector<FrameObservation>> loadVisualObservationsCsv(
    const std::string& csv_path,
    const std::vector<double>& frame_timestamps) {
  std::vector<std::vector<FrameObservation>> observations;
  std::ifstream file(csv_path);
  if (!file.is_open()) return observations;

  std::string line;
  if (!std::getline(file, line)) return observations;

  std::istringstream header_stream(line);
  std::string column;
  std::vector<std::string> columns;
  while (std::getline(header_stream, column, ',')) {
    columns.push_back(normalizeMethodId(column));
  }

  int idx_frame = -1;
  int idx_timestamp = -1;
  int idx_landmark_id = -1;
  int idx_u = -1;
  int idx_v = -1;
  for (size_t i = 0; i < columns.size(); i++) {
    if (columns[i] == "frame" || columns[i] == "frame_index" ||
        columns[i] == "frame_idx" || columns[i] == "index") {
      idx_frame = static_cast<int>(i);
    }
    if (columns[i] == "timestamp" || columns[i] == "stamp") {
      idx_timestamp = static_cast<int>(i);
    }
    if (columns[i] == "landmark_id" || columns[i] == "id") {
      idx_landmark_id = static_cast<int>(i);
    }
    if (columns[i] == "u" || columns[i] == "pixel_x" || columns[i] == "x") {
      idx_u = static_cast<int>(i);
    }
    if (columns[i] == "v" || columns[i] == "pixel_y" || columns[i] == "y") {
      idx_v = static_cast<int>(i);
    }
  }

  while (std::getline(file, line)) {
    std::istringstream row_stream(line);
    std::vector<std::string> values;
    std::string token;
    while (std::getline(row_stream, token, ',')) {
      values.push_back(trimCsvToken(token));
    }
    if (idx_landmark_id < 0 || idx_u < 0 || idx_v < 0 ||
        idx_v >= static_cast<int>(values.size())) {
      continue;
    }

    size_t frame_index = std::numeric_limits<size_t>::max();
    if (idx_frame >= 0 && idx_frame < static_cast<int>(values.size())) {
      frame_index = static_cast<size_t>(std::stoll(values[idx_frame]));
    } else if (idx_timestamp >= 0 && idx_timestamp < static_cast<int>(values.size()) &&
               !frame_timestamps.empty()) {
      frame_index = nearestFrameIndex(frame_timestamps, std::stod(values[idx_timestamp]));
    } else {
      continue;
    }

    if (frame_index >= observations.size()) observations.resize(frame_index + 1);
    observations[frame_index].push_back(
        FrameObservation{std::stoi(values[idx_landmark_id]),
                         Eigen::Vector2d(std::stod(values[idx_u]),
                                         std::stod(values[idx_v]))});
  }

  return observations;
}

std::vector<std::string> listPCDDirs(const std::string& dir) {
  std::vector<std::string> directories;
  if (!fs::exists(dir)) return directories;

  for (const auto& entry : fs::directory_iterator(dir)) {
    if (!entry.is_directory()) continue;
    if (fs::exists(entry.path() / "cloud.pcd")) {
      directories.push_back(entry.path().string());
    }
  }
  std::sort(directories.begin(), directories.end());
  return directories;
}

std::vector<double> loadFrameTimestampsFromCsv(const std::string& dir) {
  std::vector<double> timestamps;
  const fs::path csv_path = fs::path(dir) / "frame_timestamps.csv";
  std::ifstream file(csv_path);
  if (!file.is_open()) return timestamps;

  std::string line;
  if (!std::getline(file, line)) return timestamps;

  std::istringstream header_stream(line);
  std::string column;
  std::vector<std::string> columns;
  while (std::getline(header_stream, column, ',')) {
    columns.push_back(normalizeMethodId(column));
  }

  int idx_timestamp = -1;
  for (size_t i = 0; i < columns.size(); i++) {
    if (columns[i] == "timestamp") {
      idx_timestamp = static_cast<int>(i);
      break;
    }
  }
  if (idx_timestamp < 0) idx_timestamp = columns.size() > 1 ? 1 : 0;

  while (std::getline(file, line)) {
    std::istringstream row_stream(line);
    std::vector<std::string> values;
    std::string token;
    while (std::getline(row_stream, token, ',')) {
      values.push_back(trimCsvToken(token));
    }
    if (idx_timestamp >= static_cast<int>(values.size())) continue;
    timestamps.push_back(std::stod(values[idx_timestamp]));
  }

  return timestamps;
}

std::vector<Eigen::Vector3d> loadPCDPoints(const std::string& path,
                                           double leaf_size,
                                           size_t max_points) {
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
  if (pcl::io::loadPCDFile<pcl::PointXYZ>(path, *cloud) == -1) return {};

  pcl::PointCloud<pcl::PointXYZ>::Ptr filtered(new pcl::PointCloud<pcl::PointXYZ>);
  if (leaf_size > 1e-9) {
    pcl::VoxelGrid<pcl::PointXYZ> voxel_grid;
    voxel_grid.setInputCloud(cloud);
    voxel_grid.setLeafSize(static_cast<float>(leaf_size),
                           static_cast<float>(leaf_size),
                           static_cast<float>(leaf_size));
    voxel_grid.filter(*filtered);
  } else {
    filtered = cloud;
  }

  std::vector<Eigen::Vector3d> points;
  points.reserve(filtered->size());
  for (const auto& point : filtered->points) {
    if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)) {
      continue;
    }
    const double range =
        std::sqrt(point.x * point.x + point.y * point.y + point.z * point.z);
    if (range <= 0.5 || range >= 80.0) continue;
    points.emplace_back(point.x, point.y, point.z);
  }

  if (max_points > 0 && points.size() > max_points) {
    std::vector<Eigen::Vector3d> limited;
    limited.reserve(max_points);
    const double step = static_cast<double>(points.size()) / max_points;
    for (size_t i = 0; i < max_points; i++) {
      const size_t idx =
          std::min(static_cast<size_t>(i * step), points.size() - 1);
      limited.push_back(points[idx]);
    }
    return limited;
  }
  return points;
}

localization_zoo::aloam::PointCloudConstPtr toAloamCloud(
    const std::vector<Eigen::Vector3d>& points) {
  auto cloud = localization_zoo::aloam::PointCloudPtr(
      new localization_zoo::aloam::PointCloud);
  cloud->reserve(points.size());
  for (const auto& point : points) {
    localization_zoo::aloam::PointT pcl_point;
    pcl_point.x = static_cast<float>(point.x());
    pcl_point.y = static_cast<float>(point.y());
    pcl_point.z = static_cast<float>(point.z());
    pcl_point.intensity = 0.0f;
    cloud->push_back(pcl_point);
  }
  return cloud;
}

std::vector<localization_zoo::imu_preintegration::ImuSample> selectImuWindow(
    const std::vector<ImuSampleCsv>& imu_samples, double start_time, double end_time) {
  std::vector<localization_zoo::imu_preintegration::ImuSample> batch;
  for (const auto& sample : imu_samples) {
    if (sample.timestamp < start_time) continue;
    if (sample.timestamp > end_time) break;
    localization_zoo::imu_preintegration::ImuSample imu;
    imu.timestamp = sample.timestamp;
    imu.gyro = sample.gyro;
    imu.accel = sample.accel;
    batch.push_back(imu);
  }
  return batch;
}

double computeATE(const std::vector<Eigen::Matrix4d>& estimated,
                  const std::vector<Eigen::Matrix4d>& ground_truth) {
  const int num_poses =
      std::min(static_cast<int>(estimated.size()), static_cast<int>(ground_truth.size()));
  if (num_poses == 0) return 0.0;

  double squared_error_sum = 0.0;
  for (int i = 0; i < num_poses; i++) {
    const double error =
        (estimated[i].block<3, 1>(0, 3) - ground_truth[i].block<3, 1>(0, 3))
            .norm();
    squared_error_sum += error * error;
  }
  return std::sqrt(squared_error_sum / num_poses);
}

double computeTrajectoryLength(const std::vector<Eigen::Matrix4d>& ground_truth) {
  double trajectory_length = 0.0;
  for (size_t i = 1; i < ground_truth.size(); i++) {
    trajectory_length +=
        (ground_truth[i].block<3, 1>(0, 3) -
         ground_truth[i - 1].block<3, 1>(0, 3)).norm();
  }
  return trajectory_length;
}

int frameToGTIndex(size_t frame_index, size_t total_frames, size_t num_gt_poses) {
  if (num_gt_poses == 0) return 0;
  if (num_gt_poses == total_frames) {
    return std::min(static_cast<int>(frame_index), static_cast<int>(num_gt_poses) - 1);
  }
  const int gt_index = static_cast<int>(
      static_cast<double>(frame_index) /
      std::max<size_t>(total_frames, 1) * num_gt_poses);
  return std::min(gt_index, static_cast<int>(num_gt_poses) - 1);
}

std::vector<double> sampleFrameTimestamps(const std::vector<GTPose>& gt_poses,
                                          size_t num_frames,
                                          size_t total_frames) {
  std::vector<double> timestamps;
  timestamps.reserve(num_frames);
  for (size_t i = 0; i < num_frames; i++) {
    timestamps.push_back(
        gt_poses[frameToGTIndex(i, total_frames, gt_poses.size())].timestamp);
  }
  return timestamps;
}

std::vector<Eigen::Matrix4d> sampleGTPoseMatrices(
    const std::vector<GTPose>& gt_poses,
    const std::vector<double>& frame_timestamps) {
  std::vector<Eigen::Matrix4d> sampled;
  if (gt_poses.empty()) return sampled;

  std::vector<double> gt_timestamps;
  gt_timestamps.reserve(gt_poses.size());
  for (const auto& pose : gt_poses) gt_timestamps.push_back(pose.timestamp);

  const Eigen::Matrix4d initial_inverse = gt_poses.front().toMatrix().inverse();
  sampled.reserve(frame_timestamps.size());
  for (double timestamp : frame_timestamps) {
    auto it = std::lower_bound(gt_timestamps.begin(), gt_timestamps.end(), timestamp);
    size_t index = 0;
    if (it == gt_timestamps.end()) {
      index = gt_timestamps.size() - 1;
    } else if (it == gt_timestamps.begin()) {
      index = 0;
    } else {
      const size_t hi = static_cast<size_t>(it - gt_timestamps.begin());
      const size_t lo = hi - 1;
      index = std::abs(gt_timestamps[hi] - timestamp) <
                      std::abs(gt_timestamps[lo] - timestamp)
                  ? hi
                  : lo;
    }
    sampled.push_back(initial_inverse * gt_poses[index].toMatrix());
  }
  return sampled;
}

std::string frameTimestampSourceName(FrameTimestampSource source) {
  switch (source) {
    case FrameTimestampSource::kFrameTimestampCsv:
      return "frame_timestamps.csv";
    case FrameTimestampSource::kSampledGT:
      return "sampled GT timestamps";
  }
  return "unknown";
}

template <typename CameraT>
void applyCameraOverrides(CameraT* camera, const CameraOverrides& overrides) {
  if (overrides.fx) camera->fx = *overrides.fx;
  if (overrides.fy) camera->fy = *overrides.fy;
  if (overrides.cx) camera->cx = *overrides.cx;
  if (overrides.cy) camera->cy = *overrides.cy;
  if (overrides.width) camera->width = *overrides.width;
  if (overrides.height) camera->height = *overrides.height;
}

template <typename LandmarkT>
std::vector<LandmarkT> convertLandmarks(
    const std::vector<LandmarkRecord>& landmarks) {
  std::vector<LandmarkT> converted;
  converted.reserve(landmarks.size());
  for (const auto& landmark : landmarks) {
    LandmarkT item;
    item.id = landmark.id;
    item.position = landmark.position;
    converted.push_back(item);
  }
  return converted;
}

template <typename ObservationT>
std::vector<ObservationT> convertObservations(
    const std::vector<FrameObservation>& observations) {
  std::vector<ObservationT> converted;
  converted.reserve(observations.size());
  for (const auto& observation : observations) {
    ObservationT item;
    item.landmark_id = observation.landmark_id;
    item.pixel = observation.pixel;
    converted.push_back(item);
  }
  return converted;
}

Eigen::Matrix4d poseFromQuaternionTranslation(const Eigen::Quaterniond& quat,
                                              const Eigen::Vector3d& trans) {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  pose.block<3, 3>(0, 0) = quat.toRotationMatrix();
  pose.block<3, 1>(0, 3) = trans;
  return pose;
}

MethodResult runVinsFusion(
    const std::vector<std::vector<FrameObservation>>& observations,
    const std::vector<double>& frame_timestamps,
    const std::vector<ImuSampleCsv>& imu_samples,
    const std::vector<LandmarkRecord>& landmarks,
    const localization_zoo::vins_fusion::VinsFusionParams& params) {
  using namespace localization_zoo::vins_fusion;
  MethodResult result;
  result.name = "VINS-Fusion";

  VinsFusion pipeline(params);
  pipeline.setLandmarks(convertLandmarks<Landmark>(landmarks));

  const auto start = Clock::now();
  for (size_t i = 0; i < observations.size(); i++) {
    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && frame_timestamps.size() == observations.size() && !imu_samples.empty()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }
    const auto frame_result =
        pipeline.process(convertObservations<VisualObservation>(observations[i]),
                         imu_batch);
    result.poses.push_back(
        poseFromQuaternionTranslation(frame_result.q_w_curr, frame_result.t_w_curr));
    if (i % 10 == 0) {
      std::cerr << "\r  [VINS-Fusion] " << i << "/" << observations.size()
                << " imu=" << (frame_result.imu_used ? "y" : "n")
                << " vis=" << frame_result.num_visual_factors;
    }
  }
  std::cerr << std::endl;
  result.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - start).count();
  result.note =
      "Compact visual-inertial odometry with landmark reprojection and IMU "
      "preintegration.";
  return result;
}

MethodResult runOkvis(
    const std::vector<std::vector<FrameObservation>>& observations,
    const std::vector<double>& frame_timestamps,
    const std::vector<ImuSampleCsv>& imu_samples,
    const std::vector<LandmarkRecord>& landmarks,
    const localization_zoo::okvis::OkvisParams& params) {
  using namespace localization_zoo::okvis;
  MethodResult result;
  result.name = "OKVIS";

  Okvis pipeline(params);
  pipeline.setLandmarks(convertLandmarks<Landmark>(landmarks));

  const auto start = Clock::now();
  for (size_t i = 0; i < observations.size(); i++) {
    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && frame_timestamps.size() == observations.size() && !imu_samples.empty()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }
    const auto frame_result =
        pipeline.process(convertObservations<VisualObservation>(observations[i]),
                         imu_batch);
    result.poses.push_back(
        poseFromQuaternionTranslation(frame_result.q_w_curr, frame_result.t_w_curr));
    if (i % 10 == 0) {
      std::cerr << "\r  [OKVIS] " << i << "/" << observations.size()
                << " imu=" << (frame_result.imu_used ? "y" : "n")
                << " vis=" << frame_result.num_visual_factors;
    }
  }
  std::cerr << std::endl;
  result.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - start).count();
  result.note =
      "Compact fixed-window visual-inertial odometry with landmark "
      "reprojection and IMU priors.";
  return result;
}

MethodResult runOrbSlam3(
    const std::vector<std::vector<FrameObservation>>& observations,
    const std::vector<double>& frame_timestamps,
    const std::vector<ImuSampleCsv>& imu_samples,
    const std::vector<LandmarkRecord>& landmarks,
    const localization_zoo::orb_slam3::OrbSlam3Params& params) {
  using namespace localization_zoo::orb_slam3;
  MethodResult result;
  result.name = "ORB-SLAM3";

  OrbSlam3 pipeline(params);
  pipeline.setLandmarks(convertLandmarks<Landmark>(landmarks));

  const auto start = Clock::now();
  for (size_t i = 0; i < observations.size(); i++) {
    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && frame_timestamps.size() == observations.size() && !imu_samples.empty()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }
    const auto frame_result =
        pipeline.process(convertObservations<VisualObservation>(observations[i]),
                         imu_batch);
    result.poses.push_back(
        poseFromQuaternionTranslation(frame_result.q_w_curr, frame_result.t_w_curr));
    if (i % 10 == 0) {
      std::cerr << "\r  [ORB-SLAM3] " << i << "/" << observations.size()
                << " imu=" << (frame_result.imu_used ? "y" : "n")
                << " loops=" << frame_result.num_loop_edges
                << " vis=" << frame_result.num_visual_factors;
    }
  }
  std::cerr << std::endl;
  result.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - start).count();
  result.note =
      "Compact visual-inertial SLAM with loop-closure graph and landmark "
      "reprojection factors.";
  return result;
}

MethodResult runLviSam(
    const std::vector<std::string>& pcd_dirs,
    const std::vector<std::vector<FrameObservation>>& observations,
    const std::vector<double>& frame_timestamps,
    const std::vector<ImuSampleCsv>& imu_samples,
    const std::vector<LandmarkRecord>& landmarks,
    const localization_zoo::lvi_sam::LviSamParams& params,
    double cloud_voxel_size,
    size_t max_lidar_points) {
  using namespace localization_zoo::lvi_sam;
  MethodResult result;
  result.name = "LVI-SAM";

  LviSam pipeline(params);
  pipeline.setLandmarks(convertLandmarks<Landmark>(landmarks));

  const auto start = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size() && i < observations.size(); i++) {
    const auto points =
        loadPCDPoints(pcd_dirs[i] + "/cloud.pcd", cloud_voxel_size, max_lidar_points);
    if (points.empty()) continue;

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && frame_timestamps.size() >= pcd_dirs.size() && !imu_samples.empty()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }
    const auto frame_result =
        pipeline.process(toAloamCloud(points),
                         convertObservations<VisualObservation>(observations[i]),
                         imu_batch);
    result.poses.push_back(
        poseFromQuaternionTranslation(frame_result.q_w_curr, frame_result.t_w_curr));
    if (i % 10 == 0) {
      std::cerr << "\r  [LVI-SAM] " << i << "/" << pcd_dirs.size()
                << " imu=" << (frame_result.imu_used ? "y" : "n")
                << " loops=" << frame_result.num_loop_edges
                << " vis=" << frame_result.num_visual_factors;
    }
  }
  std::cerr << std::endl;
  result.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - start).count();
  result.note =
      "Compact visual-lidar-inertial SLAM with A-LOAM front-end, loop closure, "
      "and landmark reprojection factors.";
  return result;
}

MethodResult runFastLivo2(
    const std::vector<std::string>& pcd_dirs,
    const std::vector<std::vector<FrameObservation>>& observations,
    const std::vector<double>& frame_timestamps,
    const std::vector<ImuSampleCsv>& imu_samples,
    const std::vector<LandmarkRecord>& landmarks,
    const localization_zoo::fast_livo2::FastLivo2Params& params,
    double cloud_voxel_size,
    size_t max_lidar_points) {
  using namespace localization_zoo::fast_livo2;
  MethodResult result;
  result.name = "FAST-LIVO2";

  FastLivo2 pipeline(params);
  pipeline.setLandmarks(convertLandmarks<Landmark>(landmarks));

  const auto start = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size() && i < observations.size(); i++) {
    const auto points =
        loadPCDPoints(pcd_dirs[i] + "/cloud.pcd", cloud_voxel_size, max_lidar_points);
    if (points.empty()) continue;

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && frame_timestamps.size() >= pcd_dirs.size() && !imu_samples.empty()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }
    const auto frame_result =
        pipeline.process(points, convertObservations<VisualObservation>(observations[i]),
                         imu_batch);
    result.poses.push_back(
        poseFromQuaternionTranslation(frame_result.state.quat, frame_result.state.trans));
    if (i % 10 == 0) {
      std::cerr << "\r  [FAST-LIVO2] " << i << "/" << pcd_dirs.size()
                << " imu=" << (frame_result.imu_used ? "y" : "n")
                << " vis=" << frame_result.num_visual_factors
                << " map=" << frame_result.map_size;
    }
  }
  std::cerr << std::endl;
  result.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - start).count();
  result.note =
      "FAST-LIO2 front-end with landmark reprojection refinement for "
      "visual-lidar-inertial odometry.";
  return result;
}

MethodResult runR2Live(
    const std::vector<std::string>& pcd_dirs,
    const std::vector<std::vector<FrameObservation>>& observations,
    const std::vector<double>& frame_timestamps,
    const std::vector<ImuSampleCsv>& imu_samples,
    const std::vector<LandmarkRecord>& landmarks,
    const localization_zoo::r2live::R2LiveParams& params,
    double cloud_voxel_size,
    size_t max_lidar_points) {
  using namespace localization_zoo::r2live;
  MethodResult result;
  result.name = "R2LIVE";

  R2Live pipeline(params);
  pipeline.setLandmarks(convertLandmarks<Landmark>(landmarks));

  const auto start = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size() && i < observations.size(); i++) {
    const auto points =
        loadPCDPoints(pcd_dirs[i] + "/cloud.pcd", cloud_voxel_size, max_lidar_points);
    if (points.empty()) continue;

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && frame_timestamps.size() >= pcd_dirs.size() && !imu_samples.empty()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }
    const auto frame_result =
        pipeline.process(points, convertObservations<VisualObservation>(observations[i]),
                         imu_batch);
    result.poses.push_back(
        poseFromQuaternionTranslation(frame_result.q_w_curr, frame_result.t_w_curr));
    if (i % 10 == 0) {
      std::cerr << "\r  [R2LIVE] " << i << "/" << pcd_dirs.size()
                << " imu=" << (frame_result.imu_used ? "y" : "n")
                << " kf=" << frame_result.num_keyframes
                << " vis=" << frame_result.num_visual_factors;
    }
  }
  std::cerr << std::endl;
  result.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - start).count();
  result.note =
      "FAST-LIO2-based visual-lidar-inertial SLAM with landmark reprojection "
      "factors on keyframes.";
  return result;
}

void writeSummaryJson(const std::string& path,
                      const std::string& sequence_dir,
                      const std::string& gt_csv,
                      const std::string& landmarks_csv,
                      const std::string& observations_csv,
                      size_t num_frames,
                      size_t num_landmarks,
                      size_t num_observations,
                      double trajectory_length_m,
                      FrameTimestampSource timestamp_source,
                      const std::vector<MethodResult>& results) {
  std::ofstream out(path);
  out << "{\n";
  out << "  \"sequence_dir\": \"" << jsonEscape(sequence_dir) << "\",\n";
  out << "  \"pcd_dir\": \"" << jsonEscape(sequence_dir) << "\",\n";
  out << "  \"gt_csv\": \"" << jsonEscape(gt_csv) << "\",\n";
  out << "  \"landmarks_csv\": \"" << jsonEscape(landmarks_csv) << "\",\n";
  out << "  \"visual_observations_csv\": \"" << jsonEscape(observations_csv) << "\",\n";
  out << "  \"num_frames\": " << num_frames << ",\n";
  out << "  \"num_landmarks\": " << num_landmarks << ",\n";
  out << "  \"num_observations\": " << num_observations << ",\n";
  out << "  \"trajectory_length_m\": " << std::fixed << std::setprecision(6)
      << trajectory_length_m << ",\n";
  out << "  \"timestamp_source\": \""
      << jsonEscape(frameTimestampSourceName(timestamp_source)) << "\",\n";
  out << "  \"methods\": [\n";
  for (size_t i = 0; i < results.size(); i++) {
    const auto& result = results[i];
    out << "    {\n";
    out << "      \"name\": \"" << jsonEscape(result.name) << "\",\n";
    out << "      \"status\": \"" << (result.skipped ? "SKIPPED" : "OK") << "\",\n";
    if (result.skipped) {
      out << "      \"ate_m\": null,\n";
      out << "      \"frames\": 0,\n";
      out << "      \"time_ms\": null,\n";
      out << "      \"fps\": null,\n";
    } else {
      const double fps = result.time_ms > 0.0
                             ? result.poses.size() / (result.time_ms / 1000.0)
                             : 0.0;
      out << "      \"ate_m\": " << std::fixed << std::setprecision(6)
          << result.ate << ",\n";
      out << "      \"frames\": " << result.poses.size() << ",\n";
      out << "      \"time_ms\": " << std::fixed << std::setprecision(6)
          << result.time_ms << ",\n";
      out << "      \"fps\": " << std::fixed << std::setprecision(6) << fps
          << ",\n";
    }
    out << "      \"note\": \"" << jsonEscape(result.note) << "\"\n";
    out << "    }" << (i + 1 == results.size() ? "\n" : ",\n");
  }
  out << "  ]\n";
  out << "}\n";
}

void printUsage(const char* argv0) {
  std::cerr
      << "Usage: " << argv0
      << " <sequence_dir> <gt_csv> [max_frames]"
      << " [--methods vins_fusion,okvis,orb_slam3,lvi_sam,fast_livo2,r2live]"
      << " [--summary-json path]"
      << " [--landmarks-csv path]"
      << " [--visual-observations-csv path]"
      << " [--camera-fx value] [--camera-fy value]"
      << " [--camera-cx value] [--camera-cy value]"
      << " [--camera-width value] [--camera-height value]"
      << " [--cloud-voxel-size value] [--max-lidar-points value]"
      << " [--vins-fusion-fast-profile] [--vins-fusion-dense-profile]"
      << " [--okvis-fast-profile] [--okvis-dense-profile]"
      << " [--orb-slam3-fast-profile] [--orb-slam3-dense-profile]"
      << " [--lvi-sam-fast-profile] [--lvi-sam-dense-profile]"
      << " [--fast-livo2-fast-profile] [--fast-livo2-dense-profile]"
      << " [--r2live-fast-profile] [--r2live-dense-profile]"
      << std::endl;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    printUsage(argv[0]);
    return 1;
  }

  const std::string sequence_dir = argv[1];
  const std::string gt_csv = argv[2];
  int arg_index = 3;
  int max_frames = -1;
  if (arg_index < argc && std::string(argv[arg_index]).rfind("--", 0) != 0) {
    max_frames = std::stoi(argv[arg_index]);
    arg_index++;
  }

  std::string methods_csv =
      "vins_fusion,okvis,orb_slam3,lvi_sam,fast_livo2,r2live";
  std::string summary_json_path;
  std::string landmarks_csv_path;
  std::string observations_csv_path;
  double cloud_voxel_size = 0.4;
  size_t max_lidar_points = 8000;
  CameraOverrides camera_overrides;

  localization_zoo::vins_fusion::VinsFusionParams vins_params;
  localization_zoo::okvis::OkvisParams okvis_params;
  localization_zoo::orb_slam3::OrbSlam3Params orb_params;
  localization_zoo::lvi_sam::LviSamParams lvi_params;
  localization_zoo::fast_livo2::FastLivo2Params fast_livo2_params;
  localization_zoo::r2live::R2LiveParams r2live_params;

  const auto requireValue = [&](const std::string& flag) -> const char* {
    if (arg_index + 1 >= argc) {
      std::cerr << flag << " requires a value" << std::endl;
      std::exit(1);
    }
    return argv[++arg_index];
  };

  for (; arg_index < argc; arg_index++) {
    const std::string arg = argv[arg_index];
    if (arg == "--methods") {
      methods_csv = requireValue(arg);
      continue;
    }
    if (arg == "--summary-json") {
      summary_json_path = requireValue(arg);
      continue;
    }
    if (arg == "--landmarks-csv") {
      landmarks_csv_path = requireValue(arg);
      continue;
    }
    if (arg == "--visual-observations-csv") {
      observations_csv_path = requireValue(arg);
      continue;
    }
    if (arg == "--camera-fx") {
      camera_overrides.fx = std::stod(requireValue(arg));
      continue;
    }
    if (arg == "--camera-fy") {
      camera_overrides.fy = std::stod(requireValue(arg));
      continue;
    }
    if (arg == "--camera-cx") {
      camera_overrides.cx = std::stod(requireValue(arg));
      continue;
    }
    if (arg == "--camera-cy") {
      camera_overrides.cy = std::stod(requireValue(arg));
      continue;
    }
    if (arg == "--camera-width") {
      camera_overrides.width = std::stoi(requireValue(arg));
      continue;
    }
    if (arg == "--camera-height") {
      camera_overrides.height = std::stoi(requireValue(arg));
      continue;
    }
    if (arg == "--cloud-voxel-size") {
      cloud_voxel_size = std::stod(requireValue(arg));
      continue;
    }
    if (arg == "--max-lidar-points") {
      max_lidar_points = static_cast<size_t>(std::stoll(requireValue(arg)));
      continue;
    }

    if (arg == "--vins-fusion-fast-profile") {
      vins_params.keyframe_stride = 2;
      vins_params.optimize_every_n_keyframes = 2;
      vins_params.max_keyframes = 8;
      vins_params.min_visual_observations = 6;
      continue;
    }
    if (arg == "--vins-fusion-dense-profile") {
      vins_params.keyframe_stride = 1;
      vins_params.optimize_every_n_keyframes = 1;
      vins_params.max_keyframes = 24;
      vins_params.min_visual_observations = 4;
      vins_params.visual_pixel_sigma = 1.5;
      continue;
    }
    if (arg == "--okvis-fast-profile") {
      okvis_params.max_keyframes = 3;
      okvis_params.optimize_every_n_keyframes = 2;
      okvis_params.keyframe_translation_threshold = 0.3;
      okvis_params.min_visual_observations = 6;
      continue;
    }
    if (arg == "--okvis-dense-profile") {
      okvis_params.max_keyframes = 8;
      okvis_params.optimize_every_n_keyframes = 1;
      okvis_params.keyframe_translation_threshold = 0.12;
      okvis_params.min_visual_observations = 4;
      okvis_params.visual_pixel_sigma = 1.5;
      continue;
    }
    if (arg == "--orb-slam3-fast-profile") {
      orb_params.keyframe_translation_threshold = 0.3;
      orb_params.optimize_every_n_keyframes = 2;
      orb_params.enable_loop_closure = false;
      orb_params.min_visual_observations = 6;
      continue;
    }
    if (arg == "--orb-slam3-dense-profile") {
      orb_params.keyframe_translation_threshold = 0.12;
      orb_params.optimize_every_n_keyframes = 1;
      orb_params.enable_loop_closure = true;
      orb_params.min_visual_observations = 4;
      orb_params.min_shared_landmarks = 6;
      continue;
    }
    if (arg == "--lvi-sam-fast-profile") {
      lvi_params.scan_registration.less_flat_leaf_size = 0.4f;
      lvi_params.odometry.num_optimization_iters = 1;
      lvi_params.mapping.num_optimization_iters = 1;
      lvi_params.mapping.knn_max_dist_sq = 2.0;
      lvi_params.keyframe_translation_threshold = 0.5;
      lvi_params.optimize_every_n_keyframes = 2;
      lvi_params.enable_loop_closure = false;
      continue;
    }
    if (arg == "--lvi-sam-dense-profile") {
      lvi_params.scan_registration.less_flat_leaf_size = 0.15f;
      lvi_params.odometry.num_optimization_iters = 3;
      lvi_params.mapping.num_optimization_iters = 3;
      lvi_params.mapping.knn_max_dist_sq = 1.0;
      lvi_params.keyframe_translation_threshold = 0.2;
      lvi_params.optimize_every_n_keyframes = 1;
      lvi_params.enable_loop_closure = true;
      lvi_params.min_visual_observations = 4;
      continue;
    }
    if (arg == "--fast-livo2-fast-profile") {
      fast_livo2_params.front_end.max_iterations = 4;
      fast_livo2_params.front_end.ceres_max_iterations = 3;
      fast_livo2_params.front_end.keypoint_voxel_size = 0.8;
      fast_livo2_params.front_end.max_frames_in_map = 20;
      fast_livo2_params.max_visual_iterations = 4;
      continue;
    }
    if (arg == "--fast-livo2-dense-profile") {
      fast_livo2_params.front_end.max_iterations = 8;
      fast_livo2_params.front_end.ceres_max_iterations = 6;
      fast_livo2_params.front_end.keypoint_voxel_size = 0.4;
      fast_livo2_params.front_end.max_frames_in_map = 40;
      fast_livo2_params.max_visual_iterations = 10;
      fast_livo2_params.min_visual_observations = 4;
      continue;
    }
    if (arg == "--r2live-fast-profile") {
      r2live_params.front_end.max_iterations = 4;
      r2live_params.front_end.ceres_max_iterations = 3;
      r2live_params.front_end.keypoint_voxel_size = 0.8;
      r2live_params.keyframe_translation_threshold = 0.5;
      r2live_params.optimize_every_n_keyframes = 2;
      r2live_params.min_visual_observations = 6;
      continue;
    }
    if (arg == "--r2live-dense-profile") {
      r2live_params.front_end.max_iterations = 8;
      r2live_params.front_end.ceres_max_iterations = 6;
      r2live_params.front_end.keypoint_voxel_size = 0.4;
      r2live_params.keyframe_translation_threshold = 0.2;
      r2live_params.optimize_every_n_keyframes = 1;
      r2live_params.min_visual_observations = 4;
      continue;
    }

    std::cerr << "Unknown argument: " << arg << std::endl;
    return 1;
  }

  const auto methods = splitMethodList(methods_csv);
  if (methods.empty()) {
    std::cerr << "No methods selected." << std::endl;
    return 1;
  }
  for (const auto& method : methods) {
    if (!isSupportedMethod(method)) {
      std::cerr << "Unsupported method in --methods: " << method << std::endl;
      return 1;
    }
  }

  if (landmarks_csv_path.empty()) {
    landmarks_csv_path = (fs::path(sequence_dir) / "landmarks.csv").string();
  }
  if (observations_csv_path.empty()) {
    observations_csv_path =
        (fs::path(sequence_dir) / "visual_observations.csv").string();
  }

  applyCameraOverrides(&vins_params.camera, camera_overrides);
  applyCameraOverrides(&okvis_params.camera, camera_overrides);
  applyCameraOverrides(&orb_params.camera, camera_overrides);
  applyCameraOverrides(&lvi_params.camera, camera_overrides);
  applyCameraOverrides(&fast_livo2_params.camera, camera_overrides);
  applyCameraOverrides(&r2live_params.camera, camera_overrides);

  const auto gt_poses_raw = loadGTPoses(gt_csv);
  if (gt_poses_raw.empty()) {
    std::cerr << "Failed to load GT poses from " << gt_csv << std::endl;
    return 1;
  }

  auto pcd_dirs = listPCDDirs(sequence_dir);
  auto frame_timestamps = loadFrameTimestampsFromCsv(sequence_dir);
  auto observations =
      loadVisualObservationsCsv(observations_csv_path, frame_timestamps);
  const auto landmarks = loadLandmarksCsv(landmarks_csv_path);

  std::vector<size_t> frame_counts;
  if (!pcd_dirs.empty()) frame_counts.push_back(pcd_dirs.size());
  if (!frame_timestamps.empty()) frame_counts.push_back(frame_timestamps.size());
  if (!observations.empty()) frame_counts.push_back(observations.size());
  if (frame_counts.empty()) {
    std::cerr << "No frame-aligned input found under " << sequence_dir
              << ". Expected cloud.pcd frames, frame_timestamps.csv, or "
              << "visual_observations.csv." << std::endl;
    return 1;
  }

  size_t total_frames =
      *std::min_element(frame_counts.begin(), frame_counts.end());
  if (max_frames > 0) {
    total_frames = std::min(total_frames, static_cast<size_t>(max_frames));
  }
  if (total_frames == 0) {
    std::cerr << "No frames available after alignment." << std::endl;
    return 1;
  }

  FrameTimestampSource timestamp_source = FrameTimestampSource::kFrameTimestampCsv;
  if (frame_timestamps.size() < total_frames) {
    frame_timestamps = sampleFrameTimestamps(
        gt_poses_raw, total_frames,
        std::max(total_frames, std::max(pcd_dirs.size(), observations.size())));
    timestamp_source = FrameTimestampSource::kSampledGT;
  } else {
    frame_timestamps.resize(total_frames);
  }

  if (pcd_dirs.size() > total_frames) pcd_dirs.resize(total_frames);
  if (observations.size() < total_frames) observations.resize(total_frames);
  if (observations.size() > total_frames) observations.resize(total_frames);

  fs::path imu_csv_path = fs::path(sequence_dir) / "imu.csv";
  std::vector<ImuSampleCsv> imu_samples;
  if (fs::exists(imu_csv_path)) {
    imu_samples = loadImuCsv(imu_csv_path.string());
  }

  const auto gt = sampleGTPoseMatrices(gt_poses_raw, frame_timestamps);
  const double trajectory_length_m = computeTrajectoryLength(gt);
  size_t num_observations = 0;
  for (const auto& frame_observations : observations) {
    num_observations += frame_observations.size();
  }

  std::cout << "Sequence dir: " << sequence_dir << std::endl;
  std::cout << "GT poses: " << gt_poses_raw.size() << std::endl;
  std::cout << "Frames: " << total_frames << std::endl;
  std::cout << "PCD frames: " << pcd_dirs.size() << std::endl;
  std::cout << "Landmarks: " << landmarks.size() << std::endl;
  std::cout << "Visual observations: " << num_observations << std::endl;
  std::cout << "IMU samples: " << imu_samples.size() << std::endl;
  std::cout << "Timestamp source: " << frameTimestampSourceName(timestamp_source)
            << std::endl;

  std::vector<MethodResult> results;
  results.reserve(methods.size());

  const bool has_landmarks = fs::exists(landmarks_csv_path) && !landmarks.empty();
  const bool has_observations =
      fs::exists(observations_csv_path) && num_observations > 0;

  if (isMethodEnabled(methods, "vins_fusion")) {
    if (!has_landmarks || !has_observations) {
      results.push_back(makeSkippedResult(
          "VINS-Fusion",
          "landmarks.csv or visual_observations.csv missing/empty."));
    } else {
      results.push_back(
          runVinsFusion(observations, frame_timestamps, imu_samples, landmarks,
                        vins_params));
    }
  }

  if (isMethodEnabled(methods, "okvis")) {
    if (!has_landmarks || !has_observations) {
      results.push_back(makeSkippedResult(
          "OKVIS", "landmarks.csv or visual_observations.csv missing/empty."));
    } else {
      results.push_back(
          runOkvis(observations, frame_timestamps, imu_samples, landmarks,
                   okvis_params));
    }
  }

  if (isMethodEnabled(methods, "orb_slam3")) {
    if (!has_landmarks || !has_observations) {
      results.push_back(makeSkippedResult(
          "ORB-SLAM3",
          "landmarks.csv or visual_observations.csv missing/empty."));
    } else {
      results.push_back(
          runOrbSlam3(observations, frame_timestamps, imu_samples, landmarks,
                      orb_params));
    }
  }

  if (isMethodEnabled(methods, "lvi_sam")) {
    if (!has_landmarks || !has_observations) {
      results.push_back(makeSkippedResult(
          "LVI-SAM", "landmarks.csv or visual_observations.csv missing/empty."));
    } else if (pcd_dirs.empty()) {
      results.push_back(makeSkippedResult(
          "LVI-SAM", "cloud.pcd frames not found under sequence_dir."));
    } else {
      results.push_back(runLviSam(pcd_dirs, observations, frame_timestamps,
                                  imu_samples, landmarks, lvi_params,
                                  cloud_voxel_size, max_lidar_points));
    }
  }

  if (isMethodEnabled(methods, "fast_livo2")) {
    if (!has_landmarks || !has_observations) {
      results.push_back(makeSkippedResult(
          "FAST-LIVO2",
          "landmarks.csv or visual_observations.csv missing/empty."));
    } else if (pcd_dirs.empty()) {
      results.push_back(makeSkippedResult(
          "FAST-LIVO2", "cloud.pcd frames not found under sequence_dir."));
    } else {
      results.push_back(runFastLivo2(pcd_dirs, observations, frame_timestamps,
                                     imu_samples, landmarks, fast_livo2_params,
                                     cloud_voxel_size, max_lidar_points));
    }
  }

  if (isMethodEnabled(methods, "r2live")) {
    if (!has_landmarks || !has_observations) {
      results.push_back(makeSkippedResult(
          "R2LIVE", "landmarks.csv or visual_observations.csv missing/empty."));
    } else if (pcd_dirs.empty()) {
      results.push_back(makeSkippedResult(
          "R2LIVE", "cloud.pcd frames not found under sequence_dir."));
    } else {
      results.push_back(runR2Live(pcd_dirs, observations, frame_timestamps,
                                  imu_samples, landmarks, r2live_params,
                                  cloud_voxel_size, max_lidar_points));
    }
  }

  for (auto& result : results) {
    if (!result.skipped) result.ate = computeATE(result.poses, gt);
  }

  std::cout << "\n=== Summary ===" << std::endl;
  std::cout << std::left << std::setw(15) << "Method"
            << std::setw(10) << "Status"
            << std::setw(12) << "Frames"
            << std::setw(12) << "ATE[m]"
            << std::setw(12) << "Time[ms]"
            << std::setw(12) << "FPS" << std::endl;
  for (const auto& result : results) {
    const double fps = (!result.skipped && result.time_ms > 0.0)
                           ? result.poses.size() / (result.time_ms / 1000.0)
                           : 0.0;
    std::cout << std::left << std::setw(15) << result.name
              << std::setw(10) << (result.skipped ? "SKIPPED" : "OK")
              << std::setw(12) << result.poses.size();
    if (result.skipped) {
      std::cout << std::setw(12) << "-" << std::setw(12) << "-"
                << std::setw(12) << "-" << std::endl;
    } else {
      std::cout << std::setw(12) << std::fixed << std::setprecision(3)
                << result.ate
                << std::setw(12) << std::fixed << std::setprecision(1)
                << result.time_ms
                << std::setw(12) << std::fixed << std::setprecision(1)
                << fps << std::endl;
    }
  }

  for (const auto& result : results) {
    if (!result.note.empty()) {
      std::cout << "- " << result.name << ": " << result.note << std::endl;
    }
  }

  if (!summary_json_path.empty()) {
    writeSummaryJson(summary_json_path, sequence_dir, gt_csv, landmarks_csv_path,
                     observations_csv_path, total_frames, landmarks.size(),
                     num_observations, trajectory_length_m, timestamp_source,
                     results);
  }

  return 0;
}
