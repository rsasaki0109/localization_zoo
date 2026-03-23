#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

namespace localization_zoo {

using PointT = pcl::PointXYZI;
using PointCloud = pcl::PointCloud<PointT>;
using PointCloudPtr = PointCloud::Ptr;

struct Pose {
  Eigen::Vector3d position = Eigen::Vector3d::Zero();
  Eigen::Quaterniond orientation = Eigen::Quaterniond::Identity();

  Eigen::Affine3d toAffine() const {
    Eigen::Affine3d affine = Eigen::Affine3d::Identity();
    affine.translation() = position;
    affine.linear() = orientation.toRotationMatrix();
    return affine;
  }

  static Pose fromAffine(const Eigen::Affine3d& affine) {
    Pose pose;
    pose.position = affine.translation();
    pose.orientation = Eigen::Quaterniond(affine.linear());
    return pose;
  }
};

struct ImuData {
  double timestamp = 0.0;
  Eigen::Vector3d angular_velocity = Eigen::Vector3d::Zero();
  Eigen::Vector3d linear_acceleration = Eigen::Vector3d::Zero();
};

struct GnssData {
  double timestamp = 0.0;
  double latitude = 0.0;
  double longitude = 0.0;
  double altitude = 0.0;
  Eigen::Matrix3d covariance = Eigen::Matrix3d::Identity();
};

}  // namespace localization_zoo
