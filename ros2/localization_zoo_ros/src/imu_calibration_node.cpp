#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/temperature.hpp>

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

class ImuCalibrationNode : public rclcpp::Node {
 public:
  ImuCalibrationNode() : Node("imu_calibration_node") {
    const auto input_topic = declare_parameter<std::string>("input_topic", "/imu_raw");
    const auto output_topic = declare_parameter<std::string>("output_topic", "/imu/calibrated");
    const auto temperature_topic = declare_parameter<std::string>("temperature_topic", "/imu/temperature");
    use_temperature_ = declare_parameter<bool>("use_temperature", true);
    reference_temperature_c_ = declare_parameter<double>("reference_temperature_c", 25.0);
    max_temperature_age_s_ = declare_parameter<double>("max_temperature_age_s", 2.0);
    if (max_temperature_age_s_ <= 0.0 || !std::isfinite(max_temperature_age_s_)) {
      throw std::invalid_argument("max_temperature_age_s must be positive and finite");
    }
    gyro_bias_ = vectorParameter("gyro_bias_at_reference");
    gyro_temperature_coefficient_ = vectorParameter("gyro_temperature_coefficient");
    accel_bias_ = vectorParameter("accel_bias");

    publisher_ = create_publisher<sensor_msgs::msg::Imu>(output_topic, rclcpp::SensorDataQoS());
    imu_subscription_ = create_subscription<sensor_msgs::msg::Imu>(
        input_topic, rclcpp::SensorDataQoS(),
        [this](sensor_msgs::msg::Imu::ConstSharedPtr message) { onImu(*message); });
    if (use_temperature_) {
      temperature_subscription_ = create_subscription<sensor_msgs::msg::Temperature>(
          temperature_topic, rclcpp::SensorDataQoS(),
          [this](sensor_msgs::msg::Temperature::ConstSharedPtr message) {
            if (std::isfinite(message->temperature)) {
              temperature_c_ = message->temperature;
              have_temperature_ = true;
              last_temperature_received_ = now();
            }
          });
    }
    RCLCPP_INFO(get_logger(), "IMU calibration: %s -> %s (temperature %s)",
                input_topic.c_str(), output_topic.c_str(), use_temperature_ ? "enabled" : "disabled");
  }

 private:
  std::vector<double> vectorParameter(const std::string& name) {
    const auto values = declare_parameter<std::vector<double>>(name, {0.0, 0.0, 0.0});
    if (values.size() != 3 || !std::isfinite(values[0]) || !std::isfinite(values[1]) ||
        !std::isfinite(values[2])) {
      throw std::invalid_argument(name + " must contain three finite values");
    }
    return values;
  }

  void onImu(const sensor_msgs::msg::Imu& input) {
    auto output = input;
    const bool temperature_fresh = use_temperature_ && have_temperature_ &&
        (now() - last_temperature_received_).seconds() <= max_temperature_age_s_;
    const double temperature = temperature_fresh ? temperature_c_ : reference_temperature_c_;
    const double delta = temperature - reference_temperature_c_;
    double* gyro[] = {&output.angular_velocity.x, &output.angular_velocity.y, &output.angular_velocity.z};
    double* accel[] = {&output.linear_acceleration.x, &output.linear_acceleration.y, &output.linear_acceleration.z};
    for (std::size_t axis = 0; axis < 3; ++axis) {
      *gyro[axis] -= gyro_bias_[axis] + gyro_temperature_coefficient_[axis] * delta;
      *accel[axis] -= accel_bias_[axis];
    }
    publisher_->publish(output);
  }

  bool use_temperature_ = true;
  bool have_temperature_ = false;
  double temperature_c_ = 25.0;
  double reference_temperature_c_ = 25.0;
  double max_temperature_age_s_ = 2.0;
  rclcpp::Time last_temperature_received_{0, 0, RCL_ROS_TIME};
  std::vector<double> gyro_bias_;
  std::vector<double> gyro_temperature_coefficient_;
  std::vector<double> accel_bias_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr publisher_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_subscription_;
  rclcpp::Subscription<sensor_msgs::msg::Temperature>::SharedPtr temperature_subscription_;
};

}  // namespace

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  try {
    rclcpp::spin(std::make_shared<ImuCalibrationNode>());
  } catch (const std::exception& error) {
    RCLCPP_FATAL(rclcpp::get_logger("imu_calibration_node"), "%s", error.what());
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
