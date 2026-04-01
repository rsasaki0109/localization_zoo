#include "ros_utils.h"
#include "fast_lio2/fast_lio2.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>

#include <deque>
#include <memory>
#include <vector>

using namespace localization_zoo;

class FastLio2Node : public rclcpp::Node {
public:
  FastLio2Node() : Node("fast_lio2_node") {
    fast_lio2::FastLio2Params params;
    params.max_iterations = declare_parameter("max_iterations", 8);
    params.ceres_max_iterations = declare_parameter("ceres_max_iterations", 6);
    params.voxel_resolution = declare_parameter("voxel_resolution", 1.0);
    params.keypoint_voxel_size = declare_parameter("keypoint_voxel_size", 0.5);
    params.knn = declare_parameter("knn", 5);
    params.max_correspondence_distance =
        declare_parameter("max_correspondence_distance", 3.0);
    params.planarity_threshold = declare_parameter("planarity_threshold", 0.1);
    params.convergence_threshold =
        declare_parameter("convergence_threshold", 1e-3);
    params.cauchy_loss_param = declare_parameter("cauchy_loss_param", 0.3);
    params.imu_rotation_weight = declare_parameter("imu_rotation_weight", 4.0);
    params.imu_translation_weight =
        declare_parameter("imu_translation_weight", 0.5);
    params.imu_velocity_weight =
        declare_parameter("imu_velocity_weight", 0.5);
    params.velocity_prior_weight =
        declare_parameter("velocity_prior_weight", 0.2);
    params.max_frames_in_map = declare_parameter("max_frames_in_map", 30);

    scan_period_ = declare_parameter("scan_period", 0.1);
    imu_buffer_seconds_ = declare_parameter("imu_buffer_seconds", 2.0);
    map_frame_id_ = declare_parameter("map_frame_id", std::string("map"));
    base_frame_id_ = declare_parameter("base_frame_id", std::string("base_link"));

    pipeline_ = std::make_unique<fast_lio2::FastLio2>(params);

    cloud_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
        "points_raw", 10,
        std::bind(&FastLio2Node::cloudCallback, this, std::placeholders::_1));
    imu_sub_ = create_subscription<sensor_msgs::msg::Imu>(
        "imu_raw", 200,
        std::bind(&FastLio2Node::imuCallback, this, std::placeholders::_1));

    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    RCLCPP_INFO(get_logger(), "FAST-LIO2 node started");
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
    const auto cloud = ros_utils::fromRosMsg(msg);
    const auto points = ros_utils::pclToEigen(cloud);
    if (points.empty()) {
      return;
    }

    const double scan_end = rclcpp::Time(msg->header.stamp).seconds();
    const double scan_start =
        last_scan_end_time_ > 0.0 ? last_scan_end_time_ : scan_end - scan_period_;
    const auto imu_samples = collectImuSamples(scan_start, scan_end);
    const auto result = pipeline_->process(points, imu_samples);
    last_scan_end_time_ = scan_end;

    if (!result.initialized) {
      return;
    }

    auto odom = ros_utils::toOdometryMsg(result.state.quat, result.state.trans,
                                         map_frame_id_, base_frame_id_,
                                         msg->header.stamp);
    odom.twist.twist.linear.x = result.state.velocity.x();
    odom.twist.twist.linear.y = result.state.velocity.y();
    odom.twist.twist.linear.z = result.state.velocity.z();
    odom_pub_->publish(odom);

    tf_broadcaster_->sendTransform(
        ros_utils::toTransformMsg(result.state.quat, result.state.trans,
                                  map_frame_id_, base_frame_id_,
                                  msg->header.stamp));
  }

  std::unique_ptr<fast_lio2::FastLio2> pipeline_;
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
  rclcpp::spin(std::make_shared<FastLio2Node>());
  rclcpp::shutdown();
}
