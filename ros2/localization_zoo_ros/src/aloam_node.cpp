#include "ros_utils.h"
#include "aloam/scan_registration.h"
#include "aloam/laser_odometry.h"
#include "aloam/laser_mapping.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>

using namespace localization_zoo;

class ALOAMNode : public rclcpp::Node {
public:
  ALOAMNode() : Node("aloam_node") {
    aloam::ScanRegistrationParams scan_params;
    scan_params.n_scans = declare_parameter("n_scans", 16);
    scan_params.minimum_range = declare_parameter("minimum_range", 0.3);
    scan_reg_ = std::make_unique<aloam::ScanRegistration>(scan_params);

    aloam::LaserOdometryParams odom_params;
    odom_ = std::make_unique<aloam::LaserOdometry>(odom_params);

    aloam::LaserMappingParams map_params;
    map_params.line_resolution = declare_parameter("line_resolution", 0.4);
    map_params.plane_resolution = declare_parameter("plane_resolution", 0.8);
    mapping_ = std::make_unique<aloam::LaserMapping>(map_params);

    sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
        "points_raw", 10,
        std::bind(&ALOAMNode::callback, this, std::placeholders::_1));
    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    map_pub_ = create_publisher<nav_msgs::msg::Odometry>("map_odom", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    RCLCPP_INFO(get_logger(), "A-LOAM node started (n_scans=%d)", scan_params.n_scans);
  }

private:
  void callback(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
    auto cloud = ros_utils::fromRosMsg(msg);
    auto features = scan_reg_->extract(cloud);
    auto odom_result = odom_->process(features);

    auto stamp = msg->header.stamp;

    if (odom_result.valid) {
      odom_pub_->publish(ros_utils::toOdometryMsg(
          odom_result.q_w_curr, odom_result.t_w_curr, "odom", "base_link", stamp));

      auto map_result = mapping_->process(
          features.corner_less_sharp, features.surf_less_flat,
          features.full_cloud, odom_result.q_w_curr, odom_result.t_w_curr);

      if (map_result.valid) {
        map_pub_->publish(ros_utils::toOdometryMsg(
            map_result.q_w_curr, map_result.t_w_curr, "map", "base_link", stamp));
        tf_broadcaster_->sendTransform(ros_utils::toTransformMsg(
            map_result.q_w_curr, map_result.t_w_curr, "map", "base_link", stamp));
      }
    }
  }

  std::unique_ptr<aloam::ScanRegistration> scan_reg_;
  std::unique_ptr<aloam::LaserOdometry> odom_;
  std::unique_ptr<aloam::LaserMapping> mapping_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_, map_pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ALOAMNode>());
  rclcpp::shutdown();
}
