#include "ros_utils.h"
#include "dlo/dlo.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>

using namespace localization_zoo;

class DLONode : public rclcpp::Node {
public:
  DLONode() : Node("dlo_node") {
    dlo::DLOParams params;
    params.min_range = declare_parameter("min_range", 1.0);
    params.max_range = declare_parameter("max_range", 100.0);
    params.registration_voxel_size =
        declare_parameter("registration_voxel_size", 0.5);
    params.map_voxel_size = declare_parameter("map_voxel_size", 0.8);
    params.keyframe_translation_threshold =
        declare_parameter("keyframe_translation_threshold", 0.6);
    params.keyframe_rotation_threshold_rad =
        declare_parameter("keyframe_rotation_threshold_rad",
                          8.0 * 3.14159265358979323846 / 180.0);
    params.max_keyframes_in_map =
        declare_parameter("max_keyframes_in_map", 30);
    params.gicp.max_correspondence_distance =
        declare_parameter("max_correspondence_distance", 3.0);
    params.gicp.max_iterations = declare_parameter("max_iterations", 30);
    params.gicp.k_neighbors = declare_parameter("k_neighbors", 20);

    map_frame_id_ = declare_parameter("map_frame_id", std::string("map"));
    base_frame_id_ = declare_parameter("base_frame_id", std::string("base_link"));

    pipeline_ = std::make_unique<dlo::DLO>(params);

    sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
        "points_raw", 10,
        std::bind(&DLONode::callback, this, std::placeholders::_1));
    pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    RCLCPP_INFO(get_logger(), "DLO node started");
  }

private:
  void callback(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
    const aloam::PointCloudConstPtr cloud = ros_utils::fromRosMsg(msg);
    const auto result = pipeline_->process(cloud);
    if (!result.initialized) {
      return;
    }

    const Eigen::Quaterniond q(result.pose.block<3, 3>(0, 0));
    const Eigen::Vector3d t = result.pose.block<3, 1>(0, 3);
    const auto stamp = rclcpp::Time(msg->header.stamp);

    pub_->publish(
        ros_utils::toOdometryMsg(q, t, map_frame_id_, base_frame_id_, stamp));
    tf_broadcaster_->sendTransform(
        ros_utils::toTransformMsg(q, t, map_frame_id_, base_frame_id_, stamp));
  }

  std::unique_ptr<dlo::DLO> pipeline_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  std::string map_frame_id_ = "map";
  std::string base_frame_id_ = "base_link";
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DLONode>());
  rclcpp::shutdown();
}
