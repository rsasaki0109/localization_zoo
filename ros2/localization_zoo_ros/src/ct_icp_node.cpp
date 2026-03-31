#include "ros_utils.h"
#include "ct_icp/ct_icp_registration.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>

using namespace localization_zoo;

class CTICPNode : public rclcpp::Node {
public:
  CTICPNode() : Node("ct_icp_node") {
    ct_icp::CTICPParams params;
    params.voxel_resolution = declare_parameter("voxel_resolution", 1.0);
    params.max_iterations = declare_parameter("max_iterations", 30);
    params.keypoint_voxel_size = declare_parameter("keypoint_voxel_size", 0.5);
    params.max_frames_in_map = declare_parameter("max_frames_in_map", 30);
    reg_ = std::make_unique<ct_icp::CTICPRegistration>(params);

    sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
        "points_raw", 10,
        std::bind(&CTICPNode::callback, this, std::placeholders::_1));
    pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    RCLCPP_INFO(get_logger(), "CT-ICP node started");
  }

private:
  void callback(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
    auto cloud = ros_utils::fromRosMsg(msg);

    // タイムスタンプ付き点群に変換
    std::vector<ct_icp::TimedPoint> timed_points;
    for (size_t i = 0; i < cloud->size(); i++) {
      const auto& p = cloud->points[i];
      if (!std::isfinite(p.x)) continue;
      ct_icp::TimedPoint tp;
      tp.raw_point = Eigen::Vector3d(p.x, p.y, p.z);
      tp.timestamp = static_cast<double>(i) / cloud->size();
      timed_points.push_back(tp);
    }

    if (timed_points.empty()) return;

    // 初期推定: 前フレームから等速仮定
    ct_icp::TrajectoryFrame init_frame;
    init_frame.begin_pose = current_pose_;
    init_frame.end_pose = current_pose_;

    auto result = reg_->registerFrame(
        timed_points, init_frame,
        frame_count_ > 0 ? &prev_frame_ : nullptr);

    // マップ更新
    std::vector<Eigen::Vector3d> world_points;
    for (const auto& tp : timed_points) {
      world_points.push_back(
          result.frame.transformPoint(tp.raw_point, tp.timestamp));
    }
    reg_->addPointsToMap(world_points);

    current_pose_ = result.frame.end_pose;
    prev_frame_ = result.frame;

    Eigen::Quaterniond q = current_pose_.quat;
    Eigen::Vector3d t = current_pose_.trans;
    auto stamp = msg->header.stamp;

    pub_->publish(ros_utils::toOdometryMsg(q, t, "map", "base_link", stamp));
    tf_broadcaster_->sendTransform(
        ros_utils::toTransformMsg(q, t, "map", "base_link", stamp));

    frame_count_++;
  }

  std::unique_ptr<ct_icp::CTICPRegistration> reg_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  ct_icp::SE3 current_pose_;
  ct_icp::TrajectoryFrame prev_frame_;
  int frame_count_ = 0;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CTICPNode>());
  rclcpp::shutdown();
}
