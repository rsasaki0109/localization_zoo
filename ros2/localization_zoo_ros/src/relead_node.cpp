#include "ros_utils.h"
#include "relead/cesikf.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <std_msgs/msg/bool.hpp>
#include <tf2_ros/transform_broadcaster.h>

using namespace localization_zoo;

class RELEADNode : public rclcpp::Node {
public:
  RELEADNode() : Node("relead_node") {
    relead::CESIKFParams params;
    params.max_iterations = declare_parameter("max_iterations", 5);
    params.lidar_noise_std = declare_parameter("lidar_noise_std", 0.01);
    filter_ = std::make_unique<relead::CESIKF>(params);

    cloud_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
        "points_raw", 10,
        std::bind(&RELEADNode::cloudCallback, this, std::placeholders::_1));
    imu_sub_ = create_subscription<sensor_msgs::msg::Imu>(
        "imu_raw", 100,
        std::bind(&RELEADNode::imuCallback, this, std::placeholders::_1));

    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    deg_pub_ = create_publisher<std_msgs::msg::Bool>("degeneracy", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    RCLCPP_INFO(get_logger(), "RELEAD node started");
  }

private:
  void imuCallback(const sensor_msgs::msg::Imu::SharedPtr msg) {
    relead::ImuMeasurement imu;
    imu.timestamp = rclcpp::Time(msg->header.stamp).seconds();
    imu.angular_velocity = Eigen::Vector3d(
        msg->angular_velocity.x, msg->angular_velocity.y,
        msg->angular_velocity.z);
    imu.linear_acceleration = Eigen::Vector3d(
        msg->linear_acceleration.x, msg->linear_acceleration.y,
        msg->linear_acceleration.z);

    if (last_imu_time_ > 0) {
      double dt = imu.timestamp - last_imu_time_;
      if (dt > 0 && dt < 0.1) {
        filter_->predict(imu, dt);
      }
    }
    last_imu_time_ = imu.timestamp;
  }

  void cloudCallback(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
    // 簡易的なpoint-to-plane対応関係を生成
    // (実際にはikd-treeマップとのマッチングが必要)
    auto cloud = ros_utils::fromRosMsg(msg);

    std::vector<relead::PointWithNormal> correspondences;
    // ここではダミー: 各点の法線を(0,0,1)として地面マッチング
    for (const auto& p : cloud->points) {
      if (!std::isfinite(p.x)) continue;
      relead::PointWithNormal pn;
      pn.point = Eigen::Vector3d(p.x, p.y, p.z);
      pn.normal = Eigen::Vector3d(0, 0, 1);
      pn.closest = Eigen::Vector3d(p.x, p.y, 0);
      correspondences.push_back(pn);
      if (correspondences.size() > 500) break;
    }

    if (correspondences.empty()) return;

    auto result = filter_->update(correspondences);

    Eigen::Quaterniond q(result.state.rotation);
    Eigen::Vector3d t = result.state.position;
    auto stamp = msg->header.stamp;

    odom_pub_->publish(ros_utils::toOdometryMsg(q, t, "map", "base_link", stamp));
    tf_broadcaster_->sendTransform(
        ros_utils::toTransformMsg(q, t, "map", "base_link", stamp));

    // 退化情報をpublish
    std_msgs::msg::Bool deg_msg;
    deg_msg.data = result.degeneracy_info.hasDegeneracy();
    deg_pub_->publish(deg_msg);

    if (result.degeneracy_info.hasDegeneracy()) {
      RCLCPP_WARN(get_logger(), "Degeneracy detected! (rot:%zu, trans:%zu dirs)",
                  result.degeneracy_info.degenerate_rotation_dirs.size(),
                  result.degeneracy_info.degenerate_translation_dirs.size());
    }
  }

  std::unique_ptr<relead::CESIKF> filter_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr cloud_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr deg_pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  double last_imu_time_ = 0;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RELEADNode>());
  rclcpp::shutdown();
}
