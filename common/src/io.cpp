#include "common/io.h"

#include <pcl/io/pcd_io.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace localization_zoo {
namespace io {

PointCloudPtr loadPointCloud(const std::string& path) {
  auto cloud = std::make_shared<PointCloud>();
  if (pcl::io::loadPCDFile<PointT>(path, *cloud) == -1) {
    throw std::runtime_error("Failed to load PCD file: " + path);
  }
  return cloud;
}

std::vector<std::pair<double, Pose>> loadPosesCsv(const std::string& path) {
  std::vector<std::pair<double, Pose>> poses;
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + path);
  }

  std::string line;
  std::getline(file, line);  // skip header

  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string token;
    std::vector<double> values;

    while (std::getline(ss, token, ',')) {
      values.push_back(std::stod(token));
    }

    if (values.size() >= 8) {
      double timestamp = values[0];
      Pose pose;
      pose.position = Eigen::Vector3d(values[1], values[2], values[3]);
      pose.orientation = Eigen::Quaterniond(values[7], values[4], values[5], values[6]);
      poses.emplace_back(timestamp, pose);
    }
  }
  return poses;
}

std::vector<ImuData> loadImuCsv(const std::string& path) {
  std::vector<ImuData> data;
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + path);
  }

  std::string line;
  std::getline(file, line);  // skip header

  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string token;
    std::vector<double> values;

    while (std::getline(ss, token, ',')) {
      values.push_back(std::stod(token));
    }

    if (values.size() >= 7) {
      ImuData imu;
      imu.timestamp = values[0];
      imu.angular_velocity = Eigen::Vector3d(values[1], values[2], values[3]);
      imu.linear_acceleration = Eigen::Vector3d(values[4], values[5], values[6]);
      data.push_back(imu);
    }
  }
  return data;
}

}  // namespace io
}  // namespace localization_zoo
