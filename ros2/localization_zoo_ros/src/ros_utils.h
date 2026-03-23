#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <nav_msgs/msg/odometry.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <tf2_ros/transform_broadcaster.h>

namespace localization_zoo {
namespace ros_utils {

inline nav_msgs::msg::Odometry toOdometryMsg(
    const Eigen::Quaterniond& q, const Eigen::Vector3d& t,
    const std::string& frame_id, const std::string& child_frame_id,
    const rclcpp::Time& stamp) {
  nav_msgs::msg::Odometry msg;
  msg.header.stamp = stamp;
  msg.header.frame_id = frame_id;
  msg.child_frame_id = child_frame_id;
  msg.pose.pose.position.x = t.x();
  msg.pose.pose.position.y = t.y();
  msg.pose.pose.position.z = t.z();
  msg.pose.pose.orientation.x = q.x();
  msg.pose.pose.orientation.y = q.y();
  msg.pose.pose.orientation.z = q.z();
  msg.pose.pose.orientation.w = q.w();
  return msg;
}

inline geometry_msgs::msg::TransformStamped toTransformMsg(
    const Eigen::Quaterniond& q, const Eigen::Vector3d& t,
    const std::string& frame_id, const std::string& child_frame_id,
    const rclcpp::Time& stamp) {
  geometry_msgs::msg::TransformStamped tf;
  tf.header.stamp = stamp;
  tf.header.frame_id = frame_id;
  tf.child_frame_id = child_frame_id;
  tf.transform.translation.x = t.x();
  tf.transform.translation.y = t.y();
  tf.transform.translation.z = t.z();
  tf.transform.rotation.x = q.x();
  tf.transform.rotation.y = q.y();
  tf.transform.rotation.z = q.z();
  tf.transform.rotation.w = q.w();
  return tf;
}

inline pcl::PointCloud<pcl::PointXYZI>::Ptr fromRosMsg(
    const sensor_msgs::msg::PointCloud2::SharedPtr& msg) {
  auto cloud = pcl::PointCloud<pcl::PointXYZI>::Ptr(
      new pcl::PointCloud<pcl::PointXYZI>);
  pcl::fromROSMsg(*msg, *cloud);
  return cloud;
}

inline std::vector<Eigen::Vector3d> pclToEigen(
    const pcl::PointCloud<pcl::PointXYZI>::Ptr& cloud) {
  std::vector<Eigen::Vector3d> points;
  points.reserve(cloud->size());
  for (const auto& p : cloud->points) {
    if (std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z)) {
      points.emplace_back(p.x, p.y, p.z);
    }
  }
  return points;
}

}  // namespace ros_utils
}  // namespace localization_zoo
