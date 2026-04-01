#include "ros_utils.h"
#include "kiss_icp/kiss_icp.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>

using namespace localization_zoo;

class KISSICPNode : public rclcpp::Node {
public:
  KISSICPNode() : Node("kiss_icp_node") {
    kiss_icp::KISSICPParams params;
    params.voxel_size = declare_parameter("voxel_size", 0.5);
    params.max_range = declare_parameter("max_range", 100.0);
    params.min_range = declare_parameter("min_range", 3.0);
    params.max_points_per_voxel =
        declare_parameter("max_points_per_voxel", 20);
    params.initial_threshold = declare_parameter("initial_threshold", 2.0);
    params.max_icp_iterations = declare_parameter("max_icp_iterations", 500);
    params.convergence_criterion =
        declare_parameter("convergence_criterion", 1e-3);

    map_frame_id_ = declare_parameter("map_frame_id", std::string("map"));
    base_frame_id_ = declare_parameter("base_frame_id", std::string("base_link"));

    pipeline_ = std::make_unique<kiss_icp::KISSICPPipeline>(params);

    sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
        "points_raw", 10,
        std::bind(&KISSICPNode::callback, this, std::placeholders::_1));
    pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    RCLCPP_INFO(get_logger(), "KISS-ICP node started");
  }

private:
  void callback(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
    const auto cloud = ros_utils::fromRosMsg(msg);
    const auto points = ros_utils::pclToEigen(cloud);
    if (points.empty()) {
      return;
    }

    const auto result = pipeline_->registerFrame(points);
    const Eigen::Quaterniond q(result.pose.block<3, 3>(0, 0));
    const Eigen::Vector3d t = result.pose.block<3, 1>(0, 3);
    const auto stamp = rclcpp::Time(msg->header.stamp);

    pub_->publish(
        ros_utils::toOdometryMsg(q, t, map_frame_id_, base_frame_id_, stamp));
    tf_broadcaster_->sendTransform(
        ros_utils::toTransformMsg(q, t, map_frame_id_, base_frame_id_, stamp));
  }

  std::unique_ptr<kiss_icp::KISSICPPipeline> pipeline_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  std::string map_frame_id_ = "map";
  std::string base_frame_id_ = "base_link";
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<KISSICPNode>());
  rclcpp::shutdown();
}
