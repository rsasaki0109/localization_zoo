#include "ros_utils.h"
#include "litamin2/litamin2_registration.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>

using namespace localization_zoo;

class LiTAMIN2Node : public rclcpp::Node {
public:
  LiTAMIN2Node() : Node("litamin2_node") {
    litamin2::LiTAMIN2Params params;
    params.voxel_resolution = declare_parameter("voxel_resolution", 3.0);
    params.use_cov_cost = declare_parameter("use_cov_cost", true);
    params.max_iterations = declare_parameter("max_iterations", 64);
    reg_ = std::make_unique<litamin2::LiTAMIN2Registration>(params);

    sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
        "points_raw", 10,
        std::bind(&LiTAMIN2Node::callback, this, std::placeholders::_1));
    pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    RCLCPP_INFO(get_logger(), "LiTAMIN2 node started");
  }

private:
  void callback(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
    auto cloud = ros_utils::fromRosMsg(msg);
    auto points = ros_utils::pclToEigen(cloud);
    if (points.empty()) return;

    if (!initialized_) {
      reg_->setTarget(points);
      initialized_ = true;
      RCLCPP_INFO(get_logger(), "Target set (%zu points)", points.size());
      return;
    }

    auto result = reg_->align(points);
    Eigen::Matrix3d R = result.transformation.block<3, 3>(0, 0);
    Eigen::Vector3d t = result.transformation.block<3, 1>(0, 3);
    T_accum_ = result.transformation * T_accum_;

    Eigen::Quaterniond q(T_accum_.block<3, 3>(0, 0));
    Eigen::Vector3d pos = T_accum_.block<3, 1>(0, 3);

    auto stamp = msg->header.stamp;
    pub_->publish(ros_utils::toOdometryMsg(q, pos, "map", "base_link", stamp));
    tf_broadcaster_->sendTransform(
        ros_utils::toTransformMsg(q, pos, "map", "base_link", stamp));

    reg_->setTarget(points);
  }

  std::unique_ptr<litamin2::LiTAMIN2Registration> reg_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  Eigen::Matrix4d T_accum_ = Eigen::Matrix4d::Identity();
  bool initialized_ = false;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<LiTAMIN2Node>());
  rclcpp::shutdown();
}
