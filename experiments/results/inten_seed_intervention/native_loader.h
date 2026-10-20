// Verbatim runner loader fragments; provenance and boundaries in protocol.json.
#pragma once
#include <pcl/io/pcd_io.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/point_types.h>
#include <Eigen/Geometry>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "common/range_filter.h"
namespace fs = std::filesystem;
Eigen::Matrix4d g_input_sensor_pose = Eigen::Matrix4d::Identity();
bool g_input_sensor_pose_set = false;
double g_input_vertical_angle_correction_rad = 0.17 * (std::acos(-1.0) / 180.0);
double g_input_max_range_m = 120.0;
double g_input_min_range_m = 0.0;
struct LoadedXYZI {
  Eigen::Vector3d point = Eigen::Vector3d::Zero();
  float intensity = 0.0f;
};

fs::path resolvePointCloudPath(const std::string& requested_path) {
  const fs::path requested(requested_path);
  const fs::path parent = requested.parent_path();
  if (requested.filename() == "cloud.pcd" && parent.extension() == ".bin" &&
      fs::is_regular_file(parent)) {
    return parent;
  }
  return requested;
}

bool loadKittiBin(const fs::path& path,
                  pcl::PointCloud<pcl::PointXYZI>* cloud) {
  std::error_code size_error;
  const auto byte_count = fs::file_size(path, size_error);
  constexpr std::uintmax_t kBytesPerPoint = 4 * sizeof(float);
  if (size_error || byte_count == 0 || byte_count % kBytesPerPoint != 0) {
    std::cerr << "Invalid KITTI point-cloud file: " << path << "\n";
    return false;
  }

  std::ifstream input(path, std::ios::binary);
  if (!input) return false;

  cloud->clear();
  cloud->reserve(static_cast<std::size_t>(byte_count / kBytesPerPoint));
  std::array<float, 4> values{};
  while (input.read(reinterpret_cast<char*>(values.data()),
                    static_cast<std::streamsize>(kBytesPerPoint))) {
    pcl::PointXYZI point;
    point.x = values[0];
    point.y = values[1];
    point.z = values[2];
    point.intensity = values[3];
    cloud->push_back(point);
  }
  if (!input.eof()) return false;
  cloud->width = static_cast<std::uint32_t>(cloud->size());
  cloud->height = 1;
  cloud->is_dense = false;
  return true;
}


template <typename PointT>
void applyInputSensorPose(pcl::PointCloud<PointT>* cloud) {
  if (!g_input_sensor_pose_set) return;
  const Eigen::Matrix3d Rt = g_input_sensor_pose.block<3, 3>(0, 0).transpose();
  const Eigen::Vector3d t = g_input_sensor_pose.block<3, 1>(0, 3);
  for (auto& point : cloud->points) {
    const Eigen::Vector3d q = Rt * (Eigen::Vector3d(point.x, point.y, point.z) - t);
    point.x = static_cast<float>(q.x());
    point.y = static_cast<float>(q.y());
    point.z = static_cast<float>(q.z());
  }
}

template <typename PointT>
void applyInputVerticalAngleCorrection(pcl::PointCloud<PointT>* cloud) {
  applyInputSensorPose(cloud);
  localization_zoo::preprocessing::filterMinRange(*cloud, g_input_min_range_m);
  const double angle = g_input_vertical_angle_correction_rad;
  if (angle == 0.0) return;
  const double sin_angle = std::sin(angle);
  const double cos_angle = std::cos(angle);
  for (auto& point : cloud->points) {
    const double horizontal = std::hypot(point.x, point.y);
    if (!(horizontal > 1e-12)) continue;
    const double corrected_horizontal =
        horizontal * cos_angle - point.z * sin_angle;
    const double scale = corrected_horizontal / horizontal;
    point.z = static_cast<float>(point.z * cos_angle + horizontal * sin_angle);
    point.x = static_cast<float>(point.x * scale);
    point.y = static_cast<float>(point.y * scale);
  }
}


std::vector<LoadedXYZI> loadPCDXYZI(const std::string& path, double leaf = 0.5) {
  const fs::path resolved_path = resolvePointCloudPath(path);
  pcl::PointCloud<pcl::PointXYZI>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZI>);
  if (resolved_path.extension() == ".bin") {
    if (!loadKittiBin(resolved_path, cloud.get())) return {};
  } else if (pcl::io::loadPCDFile<pcl::PointXYZI>(
                 resolved_path.string(), *cloud) == -1) {
    return {};
  }
  applyInputVerticalAngleCorrection(cloud.get());

  pcl::PointCloud<pcl::PointXYZI>::Ptr selected = cloud;
  pcl::PointCloud<pcl::PointXYZI> filtered;
  if (leaf > 1e-9) {
    pcl::VoxelGrid<pcl::PointXYZI> vg;
    vg.setInputCloud(cloud);
    vg.setLeafSize(leaf, leaf, leaf);
    vg.setDownsampleAllData(true);
    vg.filter(filtered);
    selected.reset(new pcl::PointCloud<pcl::PointXYZI>(filtered));
  }

  std::vector<LoadedXYZI> points;
  points.reserve(selected->size());
  for (const auto& p : selected->points) {
    if (!std::isfinite(p.x) || !std::isfinite(p.y) ||
        !std::isfinite(p.z) || !std::isfinite(p.intensity)) {
      continue;
    }
    const double r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    if (r <= 1.0 || r >= g_input_max_range_m) continue;
    points.push_back({Eigen::Vector3d(p.x, p.y, p.z), p.intensity});
  }
  return points;
}

