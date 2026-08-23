#include "imu_motion_health/imu_motion_health.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <std_msgs/msg/string.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_broadcaster.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

namespace localization_zoo_ros {

namespace {

using localization_zoo::imu_motion_health::HealthState;
using localization_zoo::imu_motion_health::ImuMotionEvent;
using localization_zoo::imu_motion_health::ImuMotionHealth;
using localization_zoo::imu_motion_health::ImuMotionHealthParams;
using localization_zoo::imu_motion_health::ImuSample;

std::string lower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char character) {
                   return static_cast<char>(std::tolower(character));
                 });
  return value;
}

}  // namespace

class ImuMotionHealthNode final : public rclcpp::Node {
 public:
  ImuMotionHealthNode() : Node("imu_motion_health_node") {
    const std::string profile = declare_parameter<std::string>(
        "profile", "default");
    const std::string profile_file = declare_parameter<std::string>(
        "profile_file", "");

    ImuMotionHealthParams params;
    std::string error;
    if (!localization_zoo::imu_motion_health::applyProfile(
            profile, &params, &error)) {
      RCLCPP_FATAL(get_logger(), "Invalid IMU profile '%s': %s",
                   profile.c_str(), error.c_str());
      throw std::invalid_argument(error);
    }
    if (!profile_file.empty() &&
        !localization_zoo::imu_motion_health::loadProfileFile(
            profile_file, &params, &error)) {
      RCLCPP_FATAL(get_logger(), "Cannot load IMU profile file '%s': %s",
                   profile_file.c_str(), error.c_str());
      throw std::invalid_argument(error);
    }

    // ROS parameters are declared after applying the preset/file.  A value
    // supplied by a ROS parameter YAML file therefore overrides the profile,
    // while an omitted parameter retains the profile value.
    declareCoreParameters(&params);
    params_ = params;
    pipeline_ = std::make_unique<ImuMotionHealth>(params_);

    imu_topic_ = declare_parameter<std::string>("imu_topic", "imu_raw");
    odom_topic_ = declare_parameter<std::string>("odom_topic", "imu/odom");
    health_topic_ = declare_parameter<std::string>(
        "health_topic", "imu/health_json");
    events_topic_ = declare_parameter<std::string>(
        "events_topic", "imu/events_json");
    world_frame_id_ = declare_parameter<std::string>(
        "world_frame_id", "imu_world");
    base_frame_id_ = declare_parameter<std::string>(
        "base_frame_id", "base_link");
    publish_tf_ = declare_parameter<bool>("publish_tf", true);
    const std::string zero_stamp_policy = declare_parameter<std::string>(
        "zero_stamp_policy", "reject");
    receive_time_fallback_ = declare_parameter<bool>(
        "receive_time_fallback", false);
    const std::string normalized_policy = lower(zero_stamp_policy);
    if (normalized_policy == "receive_time" ||
        normalized_policy == "receive-time" ||
        normalized_policy == "fallback" || receive_time_fallback_) {
      receive_time_fallback_ = true;
    } else if (normalized_policy != "reject") {
      RCLCPP_FATAL(
          get_logger(),
          "zero_stamp_policy must be 'reject' or 'receive_time' (got '%s')",
          zero_stamp_policy.c_str());
      throw std::invalid_argument("invalid zero_stamp_policy");
    }

    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);
    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>(odom_topic_, 10);
    health_pub_ = create_publisher<std_msgs::msg::String>(health_topic_, 10);
    events_pub_ = create_publisher<std_msgs::msg::String>(events_topic_, 10);
    imu_sub_ = create_subscription<sensor_msgs::msg::Imu>(
        imu_topic_, rclcpp::SensorDataQoS(),
        std::bind(&ImuMotionHealthNode::imuCallback, this,
                  std::placeholders::_1));

    RCLCPP_INFO(
        get_logger(),
        "IMU Motion & Health node started: profile=%s imu=%s odom=%s "
        "health=%s events=%s zero_stamp_policy=%s",
        profile.c_str(), imu_topic_.c_str(), odom_topic_.c_str(),
        health_topic_.c_str(), events_topic_.c_str(),
        receive_time_fallback_ ? "receive_time" : "reject");
  }

 private:
  void declareCoreParameters(ImuMotionHealthParams* params) {
    params->startup_duration_s = declare_parameter<double>(
        "startup_duration_s", params->startup_duration_s);
    params->startup_min_samples = positiveSizeParameter(
        "startup_min_samples", params->startup_min_samples);
    params->estimate_gyro_bias = declare_parameter<bool>(
        "estimate_gyro_bias", params->estimate_gyro_bias);
    params->estimate_accel_bias = declare_parameter<bool>(
        "estimate_accel_bias", params->estimate_accel_bias);
    params->gravity_magnitude = declare_parameter<double>(
        "gravity_magnitude", params->gravity_magnitude);
    params->startup_max_gyro_std = declare_parameter<double>(
        "startup_max_gyro_std", params->startup_max_gyro_std);
    params->startup_max_accel_norm_std = declare_parameter<double>(
        "startup_max_accel_norm_std", params->startup_max_accel_norm_std);
    params->startup_gravity_tolerance = declare_parameter<double>(
        "startup_gravity_tolerance", params->startup_gravity_tolerance);

    params->max_gap_s = declare_parameter<double>(
        "max_gap_s", params->max_gap_s);
    params->min_dt_s = declare_parameter<double>(
        "min_dt_s", params->min_dt_s);
    params->gyro_saturation_rad_s = declare_parameter<double>(
        "gyro_saturation_rad_s", params->gyro_saturation_rad_s);
    params->accel_saturation_mps2 = declare_parameter<double>(
        "accel_saturation_mps2", params->accel_saturation_mps2);

    params->stationary_gyro_threshold = declare_parameter<double>(
        "stationary_gyro_threshold", params->stationary_gyro_threshold);
    params->stationary_accel_tolerance = declare_parameter<double>(
        "stationary_accel_tolerance", params->stationary_accel_tolerance);
    params->moving_gyro_threshold = declare_parameter<double>(
        "moving_gyro_threshold", params->moving_gyro_threshold);
    params->moving_accel_threshold = declare_parameter<double>(
        "moving_accel_threshold", params->moving_accel_threshold);
    params->impact_accel_threshold = declare_parameter<double>(
        "impact_accel_threshold", params->impact_accel_threshold);
    params->impact_gyro_threshold = declare_parameter<double>(
        "impact_gyro_threshold", params->impact_gyro_threshold);
    params->fall_freefall_threshold = declare_parameter<double>(
        "fall_freefall_threshold", params->fall_freefall_threshold);
    params->fall_min_duration_s = declare_parameter<double>(
        "fall_min_duration_s", params->fall_min_duration_s);
    params->event_hold_duration_s = declare_parameter<double>(
        "event_hold_duration_s", params->event_hold_duration_s);
    params->tilt_angle_threshold_deg = declare_parameter<double>(
        "tilt_angle_threshold_deg", params->tilt_angle_threshold_deg);
    params->tilt_min_duration_s = declare_parameter<double>(
        "tilt_min_duration_s", params->tilt_min_duration_s);
    params->vibration_rms_threshold = declare_parameter<double>(
        "vibration_rms_threshold", params->vibration_rms_threshold);
    params->gyro_bias_jump_threshold = declare_parameter<double>(
        "gyro_bias_jump_threshold", params->gyro_bias_jump_threshold);
    params->gyro_bias_jump_min_duration_s = declare_parameter<double>(
        "gyro_bias_jump_min_duration_s",
        params->gyro_bias_jump_min_duration_s);
    params->motion_window_samples = positiveSizeParameter(
        "motion_window_samples", params->motion_window_samples);

    params->stationary_bias_gain = declare_parameter<double>(
        "stationary_bias_gain", params->stationary_bias_gain);
    params->leveling_gain = declare_parameter<double>(
        "leveling_gain", params->leveling_gain);
    params->max_integration_dt_s = declare_parameter<double>(
        "max_integration_dt_s", params->max_integration_dt_s);
    params->zero_velocity_when_stationary = declare_parameter<bool>(
        "zero_velocity_when_stationary", params->zero_velocity_when_stationary);
    params->event_queue_capacity = positiveSizeParameter(
        "event_queue_capacity", params->event_queue_capacity);
    params->emit_moving_events = declare_parameter<bool>(
        "emit_moving_events", params->emit_moving_events);
  }

  std::size_t positiveSizeParameter(const char* name,
                                    std::size_t fallback) {
    const std::int64_t value = declare_parameter<std::int64_t>(
        name, static_cast<std::int64_t>(fallback));
    if (value <= 0) {
      RCLCPP_WARN(get_logger(), "%s must be positive; using %zu", name,
                  fallback);
      return fallback;
    }
    return static_cast<std::size_t>(value);
  }

  bool messageStampIsZero(const sensor_msgs::msg::Imu& message) const {
    return message.header.stamp.sec == 0 &&
           message.header.stamp.nanosec == 0;
  }

  void imuCallback(const sensor_msgs::msg::Imu::SharedPtr message) {
    if (!message) return;

    rclcpp::Time output_stamp(message->header.stamp);
    if (messageStampIsZero(*message)) {
      if (!receive_time_fallback_) {
        RCLCPP_WARN_THROTTLE(
            get_logger(), *get_clock(), 5000,
            "Rejecting IMU sample with zero header stamp; set "
            "zero_stamp_policy:=receive_time to use node receive time");
        return;
      }
      output_stamp = now();
    }

    ImuSample sample;
    sample.timestamp = output_stamp.seconds();
    sample.gyro = Eigen::Vector3d(message->angular_velocity.x,
                                  message->angular_velocity.y,
                                  message->angular_velocity.z);
    sample.accel = Eigen::Vector3d(message->linear_acceleration.x,
                                   message->linear_acceleration.y,
                                   message->linear_acceleration.z);
    const auto state = pipeline_->process(sample);

    nav_msgs::msg::Odometry odometry;
    odometry.header.stamp = output_stamp;
    odometry.header.frame_id = world_frame_id_;
    odometry.child_frame_id = base_frame_id_;
    odometry.pose.pose.position.x = state.relative_position.x();
    odometry.pose.pose.position.y = state.relative_position.y();
    odometry.pose.pose.position.z = state.relative_position.z();
    const Eigen::Quaterniond orientation = state.orientation.normalized();
    odometry.pose.pose.orientation.x = orientation.x();
    odometry.pose.pose.orientation.y = orientation.y();
    odometry.pose.pose.orientation.z = orientation.z();
    odometry.pose.pose.orientation.w = orientation.w();
    odometry.twist.twist.linear.x = state.relative_velocity.x();
    odometry.twist.twist.linear.y = state.relative_velocity.y();
    odometry.twist.twist.linear.z = state.relative_velocity.z();
    // The core state intentionally does not expose a corrected angular-rate
    // vector.  Preserve the incoming rate in the odometry twist rather than
    // publishing a made-up zero value.
    odometry.twist.twist.angular = message->angular_velocity;
    odom_pub_->publish(odometry);

    std_msgs::msg::String health;
    health.data = pipeline_->toJson();
    health_pub_->publish(health);

    if (publish_tf_) {
      geometry_msgs::msg::TransformStamped transform;
      transform.header = odometry.header;
      transform.child_frame_id = base_frame_id_;
      transform.transform.translation.x = state.relative_position.x();
      transform.transform.translation.y = state.relative_position.y();
      transform.transform.translation.z = state.relative_position.z();
      transform.transform.rotation = odometry.pose.pose.orientation;
      tf_broadcaster_->sendTransform(transform);
    }

    for (const ImuMotionEvent& event : pipeline_->drainEvents()) {
      std_msgs::msg::String event_message;
      event_message.data = localization_zoo::imu_motion_health::toJson(event);
      events_pub_->publish(event_message);
    }

    if (state.health_state == HealthState::kInvalid) {
      RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                           "IMU stream is invalid: %s",
                           state.diagnostic.c_str());
    }
  }

  ImuMotionHealthParams params_;
  std::unique_ptr<ImuMotionHealth> pipeline_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr health_pub_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr events_pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  std::string imu_topic_ = "imu_raw";
  std::string odom_topic_ = "imu/odom";
  std::string health_topic_ = "imu/health_json";
  std::string events_topic_ = "imu/events_json";
  std::string world_frame_id_ = "imu_world";
  std::string base_frame_id_ = "base_link";
  bool publish_tf_ = true;
  bool receive_time_fallback_ = false;
};

}  // namespace localization_zoo_ros

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<localization_zoo_ros::ImuMotionHealthNode>());
  rclcpp::shutdown();
  return 0;
}
