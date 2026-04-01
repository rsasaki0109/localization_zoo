#include "ros_utils.h"
#include "ndt/ndt_registration.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>

using namespace localization_zoo;

class NDTNode : public rclcpp::Node {
public:
  NDTNode() : Node("ndt_node") {
    ndt::NDTParams params;
    params.resolution = declare_parameter("resolution", 1.0);
    params.max_iterations = declare_parameter("max_iterations", 30);
    params.step_size = declare_parameter("step_size", 0.1);
    params.convergence_threshold =
        declare_parameter("convergence_threshold", 0.01);
    params.outlier_ratio = declare_parameter("outlier_ratio", 0.55);
    params.min_points_per_cell = declare_parameter("min_points_per_cell", 5);
    reg_ = std::make_unique<ndt::NDTRegistration>(params);

    map_frame_id_ = declare_parameter("map_frame_id", std::string("map"));
    base_frame_id_ = declare_parameter("base_frame_id", std::string("base_link"));

    sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
        "points_raw", 10,
        std::bind(&NDTNode::callback, this, std::placeholders::_1));
    pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    RCLCPP_INFO(get_logger(), "NDT node started");
  }

private:
  void publishPose(const rclcpp::Time& stamp) {
    const Eigen::Quaterniond q(accumulated_pose_.block<3, 3>(0, 0));
    const Eigen::Vector3d t = accumulated_pose_.block<3, 1>(0, 3);
    pub_->publish(
        ros_utils::toOdometryMsg(q, t, map_frame_id_, base_frame_id_, stamp));
    tf_broadcaster_->sendTransform(ros_utils::toTransformMsg(
        q, t, map_frame_id_, base_frame_id_, stamp));
  }

  void callback(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
    const auto cloud = ros_utils::fromRosMsg(msg);
    const auto points = ros_utils::pclToEigen(cloud);
    if (points.empty()) {
      return;
    }

    const auto stamp = rclcpp::Time(msg->header.stamp);
    if (!initialized_) {
      reg_->setTarget(points);
      initialized_ = true;
      publishPose(stamp);
      return;
    }

    const auto result = reg_->align(points);
    accumulated_pose_ = result.transformation * accumulated_pose_;
    publishPose(stamp);

    reg_->setTarget(points);
  }

  std::unique_ptr<ndt::NDTRegistration> reg_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  Eigen::Matrix4d accumulated_pose_ = Eigen::Matrix4d::Identity();
  bool initialized_ = false;
  std::string map_frame_id_ = "map";
  std::string base_frame_id_ = "base_link";
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<NDTNode>());
  rclcpp::shutdown();
}
