#include "ros_utils.h"
#include "xicp/xicp_registration.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <std_msgs/msg/bool.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <pcl/kdtree/kdtree_flann.h>

using namespace localization_zoo;

class XICPNode : public rclcpp::Node {
public:
  XICPNode() : Node("xicp_node") {
    xicp::XICPParams params;
    params.max_iterations = declare_parameter("max_iterations", 30);
    params.kappa_1 = declare_parameter("kappa_1", 250.0);
    params.kappa_2 = declare_parameter("kappa_2", 180.0);
    params.kappa_3 = declare_parameter("kappa_3", 35.0);
    reg_ = std::make_unique<xicp::XICPRegistration>(params);

    sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
        "points_raw", 10,
        std::bind(&XICPNode::callback, this, std::placeholders::_1));
    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    deg_pub_ = create_publisher<std_msgs::msg::Bool>("degeneracy", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    RCLCPP_INFO(get_logger(), "X-ICP node started");
  }

private:
  void callback(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
    auto cloud = ros_utils::fromRosMsg(msg);
    if (cloud->empty()) return;

    if (!prev_cloud_) {
      prev_cloud_ = cloud;
      kd_tree_.setInputCloud(prev_cloud_);
      RCLCPP_INFO(get_logger(), "First frame set (%zu points)", cloud->size());
      return;
    }

    // 対応関係をKD-treeで検索
    std::vector<xicp::Correspondence> correspondences;
    std::vector<int> idx(1);
    std::vector<float> dist(1);

    for (const auto& p : cloud->points) {
      if (!std::isfinite(p.x)) continue;
      pcl::PointXYZI search_point = p;
      if (kd_tree_.nearestKSearch(search_point, 1, idx, dist) > 0 &&
          dist[0] < 4.0) {
        // 簡易法線: 近傍5点のPCA
        // ここでは簡略化してz方向法線
        xicp::Correspondence corr;
        corr.source = Eigen::Vector3d(p.x, p.y, p.z);
        corr.target = Eigen::Vector3d(prev_cloud_->points[idx[0]].x,
                                       prev_cloud_->points[idx[0]].y,
                                       prev_cloud_->points[idx[0]].z);
        corr.normal = Eigen::Vector3d(0, 0, 1);  // 簡易
        correspondences.push_back(corr);
        if (correspondences.size() > 1000) break;
      }
    }

    if (correspondences.empty()) return;

    auto result = reg_->align(correspondences, T_accum_);
    T_accum_ = result.transformation;

    Eigen::Quaterniond q(T_accum_.block<3, 3>(0, 0));
    Eigen::Vector3d t = T_accum_.block<3, 1>(0, 3);
    auto stamp = msg->header.stamp;

    odom_pub_->publish(ros_utils::toOdometryMsg(q, t, "map", "base_link", stamp));
    tf_broadcaster_->sendTransform(
        ros_utils::toTransformMsg(q, t, "map", "base_link", stamp));

    std_msgs::msg::Bool deg_msg;
    deg_msg.data = result.localizability.hasDegeneracy();
    deg_pub_->publish(deg_msg);

    prev_cloud_ = cloud;
    kd_tree_.setInputCloud(prev_cloud_);
  }

  std::unique_ptr<xicp::XICPRegistration> reg_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr deg_pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  pcl::PointCloud<pcl::PointXYZI>::Ptr prev_cloud_;
  pcl::KdTreeFLANN<pcl::PointXYZI> kd_tree_;
  Eigen::Matrix4d T_accum_ = Eigen::Matrix4d::Identity();
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<XICPNode>());
  rclcpp::shutdown();
}
