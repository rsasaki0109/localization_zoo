#include "ros_utils.h"
#include "ct_lio/ct_lio_registration.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>

#include <deque>
#include <memory>
#include <vector>

using namespace localization_zoo;

class CTLIONode : public rclcpp::Node {
public:
  CTLIONode() : Node("ct_lio_node") {
    ct_lio::CTLIOParams params;
    params.voxel_resolution = declare_parameter("voxel_resolution", 1.0);
    params.max_iterations = declare_parameter("max_iterations", 20);
    params.keypoint_voxel_size = declare_parameter("keypoint_voxel_size", 0.5);
    params.max_frames_in_map = declare_parameter("max_frames_in_map", 30);
    params.imu_rotation_weight = declare_parameter("imu_rotation_weight", 2.0);
    params.imu_translation_weight =
        declare_parameter("imu_translation_weight", 2.0);
    params.velocity_prior_weight =
        declare_parameter("velocity_prior_weight", 0.2);
    params.estimate_imu_bias =
        declare_parameter("estimate_imu_bias", false);
    params.fixed_lag_state_window =
        declare_parameter("fixed_lag_state_window", 1);
    params.fixed_lag_velocity_prior_weight =
        declare_parameter("fixed_lag_velocity_prior_weight", 0.0);
    params.fixed_lag_gyro_bias_weight_scale =
        declare_parameter("fixed_lag_gyro_bias_weight_scale", 0.0);
    params.fixed_lag_accel_bias_weight_scale =
        declare_parameter("fixed_lag_accel_bias_weight_scale", 0.0);
    params.fixed_lag_history_decay =
        declare_parameter("fixed_lag_history_decay", 1.0);
    params.fixed_lag_outer_iterations =
        declare_parameter("fixed_lag_outer_iterations", 3);
    params.fixed_lag_optimize_history =
        declare_parameter("fixed_lag_optimize_history", false);

    scan_period_ = declare_parameter("scan_period", 0.1);
    imu_buffer_seconds_ = declare_parameter("imu_buffer_seconds", 2.0);
    map_frame_id_ = declare_parameter("map_frame_id", std::string("map"));
    base_frame_id_ = declare_parameter("base_frame_id", std::string("base_link"));

    reg_ = std::make_unique<ct_lio::CTLIORegistration>(params);

    cloud_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
        "points_raw", 10,
        std::bind(&CTLIONode::cloudCallback, this, std::placeholders::_1));
    imu_sub_ = create_subscription<sensor_msgs::msg::Imu>(
        "imu_raw", 200,
        std::bind(&CTLIONode::imuCallback, this, std::placeholders::_1));

    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    RCLCPP_INFO(get_logger(), "CT-LIO node started");
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
    auto cloud = ros_utils::fromRosMsg(msg);

    std::vector<ct_icp::TimedPoint> timed_points;
    timed_points.reserve(cloud->size());
    const size_t denom = cloud->size() > 1 ? cloud->size() - 1 : 1;
    for (size_t i = 0; i < cloud->size(); ++i) {
      const auto& point = cloud->points[i];
      if (!std::isfinite(point.x) || !std::isfinite(point.y) ||
          !std::isfinite(point.z)) {
        continue;
      }

      ct_icp::TimedPoint timed_point;
      timed_point.raw_point = Eigen::Vector3d(point.x, point.y, point.z);
      timed_point.timestamp = static_cast<double>(i) / static_cast<double>(denom);
      timed_points.push_back(timed_point);
    }

    if (timed_points.empty()) {
      return;
    }

    const double scan_end = rclcpp::Time(msg->header.stamp).seconds();
    const double scan_start = scan_end - scan_period_;
    auto imu_samples = collectImuSamples(scan_start, scan_end);

    ct_lio::CTLIOState initial_state = current_state_;
    initial_state.frame.begin_pose = current_state_.frame.end_pose;
    initial_state.frame.end_pose = current_state_.frame.end_pose;
    initial_state.frame.begin_timestamp = 0.0;
    initial_state.frame.end_timestamp = 1.0;
    initial_state.begin_velocity = current_state_.end_velocity;
    initial_state.end_velocity = current_state_.end_velocity;

    auto result = reg_->registerFrame(timed_points, initial_state, imu_samples);

    std::vector<Eigen::Vector3d> world_points;
    world_points.reserve(timed_points.size());
    for (const auto& point : timed_points) {
      world_points.push_back(
          result.state.frame.transformPoint(point.raw_point, point.timestamp));
    }
    reg_->addPointsToMap(world_points);
    current_state_ = result.state;

    auto odom = ros_utils::toOdometryMsg(
        current_state_.frame.end_pose.quat, current_state_.frame.end_pose.trans,
        map_frame_id_, base_frame_id_, msg->header.stamp);
    odom.twist.twist.linear.x = current_state_.end_velocity.x();
    odom.twist.twist.linear.y = current_state_.end_velocity.y();
    odom.twist.twist.linear.z = current_state_.end_velocity.z();
    odom_pub_->publish(odom);

    tf_broadcaster_->sendTransform(
        ros_utils::toTransformMsg(current_state_.frame.end_pose.quat,
                                  current_state_.frame.end_pose.trans,
                                  map_frame_id_, base_frame_id_,
                                  msg->header.stamp));
  }

  std::unique_ptr<ct_lio::CTLIORegistration> reg_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr cloud_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  std::deque<imu_preintegration::ImuSample> imu_buffer_;
  ct_lio::CTLIOState current_state_;
  double scan_period_ = 0.1;
  double imu_buffer_seconds_ = 2.0;
  std::string map_frame_id_ = "map";
  std::string base_frame_id_ = "base_link";
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CTLIONode>());
  rclcpp::shutdown();
}
