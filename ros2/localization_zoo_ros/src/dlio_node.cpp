#include "ros_utils.h"
#include "dlio/dlio.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>

#include <deque>
#include <memory>
#include <vector>

using namespace localization_zoo;

class DLIONode : public rclcpp::Node {
public:
  DLIONode() : Node("dlio_node") {
    dlio::DLIOParams params;
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
    params.imu_rotation_fusion_weight =
        declare_parameter("imu_rotation_fusion_weight", 0.15);
    params.imu_translation_fusion_weight =
        declare_parameter("imu_translation_fusion_weight", 0.2);
    params.imu_velocity_fusion_weight =
        declare_parameter("imu_velocity_fusion_weight", 0.2);
    params.lidar_confidence_correspondence_scale =
        declare_parameter("lidar_confidence_correspondence_scale", 100.0);

    scan_period_ = declare_parameter("scan_period", 0.1);
    imu_buffer_seconds_ = declare_parameter("imu_buffer_seconds", 2.0);
    map_frame_id_ = declare_parameter("map_frame_id", std::string("map"));
    base_frame_id_ = declare_parameter("base_frame_id", std::string("base_link"));

    pipeline_ = std::make_unique<dlio::DLIO>(params);

    cloud_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
        "points_raw", 10,
        std::bind(&DLIONode::cloudCallback, this, std::placeholders::_1));
    imu_sub_ = create_subscription<sensor_msgs::msg::Imu>(
        "imu_raw", 200,
        std::bind(&DLIONode::imuCallback, this, std::placeholders::_1));

    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    RCLCPP_INFO(get_logger(), "DLIO node started");
  }

private:
  void imuCallback(const sensor_msgs::msg::Imu::SharedPtr msg) {
    imu_preintegration::ImuSample sample;
    sample.timestamp = rclcpp::Time(msg->header.stamp).seconds();
    sample.gyro = Eigen::Vector3d(msg->angular_velocity.x,
                                  msg->angular_velocity.y,
                                  msg->angular_velocity.z);
    sample.accel = Eigen::Vector3d(msg->linear_acceleration.x,
                                   msg->linear_acceleration.y,
                                   msg->linear_acceleration.z);
    imu_buffer_.push_back(sample);

    const double cutoff = sample.timestamp - imu_buffer_seconds_;
    while (!imu_buffer_.empty() && imu_buffer_.front().timestamp < cutoff) {
      imu_buffer_.pop_front();
    }
  }

  std::vector<imu_preintegration::ImuSample> collectImuSamples(
      double start_time, double end_time) const {
    std::vector<imu_preintegration::ImuSample> samples;
    samples.reserve(imu_buffer_.size());
    for (const auto& sample : imu_buffer_) {
      if (sample.timestamp < start_time) {
        continue;
      }
      if (sample.timestamp > end_time) {
        break;
      }
      samples.push_back(sample);
    }
    return samples;
  }

  void cloudCallback(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
    const aloam::PointCloudConstPtr cloud = ros_utils::fromRosMsg(msg);
    const double scan_end = rclcpp::Time(msg->header.stamp).seconds();
    const double scan_start =
        last_scan_end_time_ > 0.0 ? last_scan_end_time_ : scan_end - scan_period_;
    const auto imu_samples = collectImuSamples(scan_start, scan_end);
    const auto result = pipeline_->process(cloud, imu_samples);
    last_scan_end_time_ = scan_end;

    if (!result.initialized) {
      return;
    }

    const Eigen::Quaterniond q(result.state.pose.block<3, 3>(0, 0));
    const Eigen::Vector3d t = result.state.pose.block<3, 1>(0, 3);
    auto odom = ros_utils::toOdometryMsg(q, t, map_frame_id_, base_frame_id_,
                                         msg->header.stamp);
    odom.twist.twist.linear.x = result.state.velocity.x();
    odom.twist.twist.linear.y = result.state.velocity.y();
    odom.twist.twist.linear.z = result.state.velocity.z();
    odom_pub_->publish(odom);

    tf_broadcaster_->sendTransform(
        ros_utils::toTransformMsg(q, t, map_frame_id_, base_frame_id_,
                                  msg->header.stamp));
  }

  std::unique_ptr<dlio::DLIO> pipeline_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr cloud_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  std::deque<imu_preintegration::ImuSample> imu_buffer_;
  double last_scan_end_time_ = -1.0;
  double scan_period_ = 0.1;
  double imu_buffer_seconds_ = 2.0;
  std::string map_frame_id_ = "map";
  std::string base_frame_id_ = "base_link";
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DLIONode>());
  rclcpp::shutdown();
}
