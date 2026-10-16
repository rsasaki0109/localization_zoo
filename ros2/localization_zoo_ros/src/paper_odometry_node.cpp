#include "ros_utils.h"
#include "dilo/dilo_paper.h"
#include "imls_slam/imls_paper_odometry.h"
#include "litamin2/litamin2_odometry.h"
#include "suma/suma_paper.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <stdexcept>

using namespace localization_zoo;

namespace {
class PaperOdometryNode : public rclcpp::Node {
 public:
  explicit PaperOdometryNode(const std::string& method) : Node(method + "_node") {
    map_frame_ = declare_parameter("map_frame_id", std::string("map"));
    if (map_frame_.empty()) throw std::invalid_argument("map_frame_id must not be empty");
    const bool publish_tf = declare_parameter("publish_tf", true);
    const auto reliability = declare_parameter("input_reliability", std::string("best_effort"));
    auto input_qos = rclcpp::SensorDataQoS();
    if (reliability == "reliable") input_qos.reliable();
    else if (reliability != "best_effort")
      throw std::invalid_argument("input_reliability must be best_effort or reliable");
    if (method == "suma_paper") {
      suma::SuMaPaperParams p;
      p.width = declare_parameter("width", p.width);
      p.height = declare_parameter("height", p.height);
      p.fov_up_deg = declare_parameter("fov_up_deg", p.fov_up_deg);
      p.fov_down_deg = declare_parameter("fov_down_deg", p.fov_down_deg);
      p.min_depth = declare_parameter("min_depth", p.min_depth);
      p.max_depth = declare_parameter("max_depth", p.max_depth);
      p.model_min_depth = declare_parameter("model_min_depth", p.model_min_depth);
      p.model_max_depth = declare_parameter("model_max_depth", p.model_max_depth);
      p.max_iterations = declare_parameter("max_iterations", p.max_iterations);
      p.stop_delta = declare_parameter("stop_delta", p.stop_delta);
      p.stop_epsilon = declare_parameter("stop_epsilon", p.stop_epsilon);
      p.icp_max_distance = declare_parameter("icp_max_distance", p.icp_max_distance);
      p.icp_max_angle_deg = declare_parameter("icp_max_angle_deg", p.icp_max_angle_deg);
      p.huber_factor = declare_parameter("huber_factor", p.huber_factor);
      p.bilinear_sampling = declare_parameter("bilinear_sampling", p.bilinear_sampling);
      p.normalize_bilinear = declare_parameter("normalize_bilinear", p.normalize_bilinear);
      p.initialize_identity = declare_parameter("initialize_identity", p.initialize_identity);
      p.fallback_mode = declare_parameter("fallback_mode", p.fallback_mode);
      p.fallback_max_distance = declare_parameter("fallback_max_distance", p.fallback_max_distance);
      p.fallback_max_angle_deg = declare_parameter("fallback_max_angle_deg", p.fallback_max_angle_deg);
      p.fallback_trans_threshold = declare_parameter("fallback_trans_threshold", p.fallback_trans_threshold);
      p.fallback_rot_threshold = declare_parameter("fallback_rot_threshold", p.fallback_rot_threshold);
      p.min_radius = declare_parameter("min_radius", p.min_radius);
      p.max_radius = declare_parameter("max_radius", p.max_radius);
      p.max_angle_deg = declare_parameter("max_angle_deg", p.max_angle_deg);
      p.map_max_distance = declare_parameter("map_max_distance", p.map_max_distance);
      p.map_max_angle_deg = declare_parameter("map_max_angle_deg", p.map_max_angle_deg);
      p.unstable_age = declare_parameter("unstable_age", p.unstable_age);
      p.confidence_threshold = declare_parameter("confidence_threshold", p.confidence_threshold);
      p.p_stable = declare_parameter("p_stable", p.p_stable);
      p.p_prior = declare_parameter("p_prior", p.p_prior);
      p.sigma_angle = declare_parameter("sigma_angle", p.sigma_angle);
      p.sigma_distance = declare_parameter("sigma_distance", p.sigma_distance);
      p.use_stability = declare_parameter("use_stability", p.use_stability);
      p.gamma = declare_parameter("gamma", p.gamma);
      p.active_timestamps = declare_parameter("active_timestamps", p.active_timestamps);
      p.max_confidence = declare_parameter("max_confidence", p.max_confidence);
      p.time_init = declare_parameter("time_init", p.time_init);
      p.p_unstable_init = declare_parameter("p_unstable_init", p.p_unstable_init);
      p.map_extent = declare_parameter("map_extent", p.map_extent);
      p.keep_inactive = declare_parameter("keep_inactive", p.keep_inactive);
      auto core = std::make_shared<suma::SuMaPaper>(p);
      process_ = [core](const std::vector<Eigen::Vector3d>& points) {
        return core->process(points).pose;
      };
    }
    else if (method == "imls_slam_paper") {
      imls_slam::ImlsPaperParams p;
      p.h = declare_parameter("h", p.h);
      p.r = declare_parameter("r", p.r);
      p.samples_per_list = declare_parameter("samples_per_list", p.samples_per_list);
      p.window_scans = declare_parameter("window_scans", p.window_scans);
      p.iterations = declare_parameter("iterations", p.iterations);
      p.scan_normal_radius = declare_parameter("scan_normal_radius", p.scan_normal_radius);
      p.scan_normal_max_neighbors = declare_parameter("scan_normal_max_neighbors", p.scan_normal_max_neighbors);
      p.min_normal_points = declare_parameter("min_normal_points", p.min_normal_points);
      p.min_range = declare_parameter("min_range", p.min_range);
      p.max_range = declare_parameter("max_range", p.max_range);
      p.coarse_frames = declare_parameter("coarse_frames", p.coarse_frames);
      p.coarse_r = declare_parameter("coarse_r", p.coarse_r);
      p.coarse_h = declare_parameter("coarse_h", p.coarse_h);
      p.coarse_iterations = declare_parameter("coarse_iterations", p.coarse_iterations);
      auto core = std::make_shared<imls_slam::ImlsPaperOdometry>(p);
      process_ = [core](const std::vector<Eigen::Vector3d>& points) {
        return core->process(points).pose;
      };
    }
    else if (method == "dilo_paper") {
      dilo::DiloPaperParams p;
      p.width = declare_parameter("width", p.width);
      p.height = declare_parameter("height", p.height);
      p.top_elevation_deg = declare_parameter("top_elevation_deg", p.top_elevation_deg);
      p.min_range = declare_parameter("min_range", p.min_range);
      p.max_range = declare_parameter("max_range", p.max_range);
      p.fill_slope = declare_parameter("fill_slope", p.fill_slope);
      p.max_fill_gap = declare_parameter("max_fill_gap", p.max_fill_gap);
      p.keypoints_per_list = declare_parameter("keypoints_per_list", p.keypoints_per_list);
      p.min_planarity = declare_parameter("min_planarity", p.min_planarity);
      p.keypoint_cell_px = declare_parameter("keypoint_cell_px", p.keypoint_cell_px);
      p.window = declare_parameter("window", p.window);
      p.init_iterations = declare_parameter("init_iterations", p.init_iterations);
      p.update_iterations = declare_parameter("update_iterations", p.update_iterations);
      p.exact_points = declare_parameter("exact_points", p.exact_points);
      p.point_gradient_normals = declare_parameter("point_gradient_normals", p.point_gradient_normals);
      p.planar_prediction = declare_parameter("planar_prediction", p.planar_prediction);
      p.max_init_deviation_m = declare_parameter("max_init_deviation_m", p.max_init_deviation_m);
      p.max_init_deviation_rad = declare_parameter("max_init_deviation_rad", p.max_init_deviation_rad);
      p.max_pair_step_m = declare_parameter("max_pair_step_m", p.max_pair_step_m);
      p.max_pair_step_rad = declare_parameter("max_pair_step_rad", p.max_pair_step_rad);
      p.use_edges = declare_parameter("use_edges", p.use_edges);
      p.max_edge_points = declare_parameter("max_edge_points", p.max_edge_points);
      p.edge_search_px = declare_parameter("edge_search_px", p.edge_search_px);
      p.harris_k = declare_parameter("harris_k", p.harris_k);
      p.huber_delta = declare_parameter("huber_delta", p.huber_delta);
      p.degenerate_ratio = declare_parameter("degenerate_ratio", p.degenerate_ratio);
      p.tukey_c = declare_parameter("tukey_c", p.tukey_c);
      p.edge_detector = declare_parameter("edge_detector", p.edge_detector);
      p.edge_reference_points = declare_parameter("edge_reference_points", p.edge_reference_points);
      p.edge_subpixel = declare_parameter("edge_subpixel", p.edge_subpixel);
      p.edge_row_search = declare_parameter("edge_row_search", p.edge_row_search);
      p.edge_sum_normal_equations = declare_parameter("edge_sum_normal_equations", p.edge_sum_normal_equations);
      p.edge_weight = declare_parameter("edge_weight", p.edge_weight);
      p.edge_dofs = declare_parameter("edge_dofs", p.edge_dofs);
      p.edge_huber_delta = declare_parameter("edge_huber_delta", p.edge_huber_delta);
      p.edge_tukey_c = declare_parameter("edge_tukey_c", p.edge_tukey_c);
      auto core = std::make_shared<dilo::DiloPaperOdometry>(p);
      process_ = [core](const std::vector<Eigen::Vector3d>& points) {
        return core->process(points);
      };
    }
    else if (method == "litamin2_paper") {
      litamin2::LiTAMIN2OdometryParams p;
      p.voxel_size = declare_parameter("voxel_size", p.voxel_size);
      p.min_points_per_voxel = declare_parameter("min_points_per_voxel", p.min_points_per_voxel);
      p.min_cov_eigenvalue = declare_parameter("min_cov_eigenvalue", p.min_cov_eigenvalue);
      p.lambda = declare_parameter("lambda", p.lambda);
      p.sigma_icp = declare_parameter("sigma_icp", p.sigma_icp);
      p.sigma_cov = declare_parameter("sigma_cov", p.sigma_cov);
      p.use_cov_cost = declare_parameter("use_cov_cost", p.use_cov_cost);
      p.max_iterations = declare_parameter("max_iterations", p.max_iterations);
      p.rotation_epsilon = declare_parameter("rotation_epsilon", p.rotation_epsilon);
      p.translation_epsilon = declare_parameter("translation_epsilon", p.translation_epsilon);
      const auto mode = declare_parameter("map_mode", std::string("aggregate"));
      using Mode = litamin2::LiTAMIN2OdometryParams::MapMode;
      if (mode == "aggregate") p.map_mode = Mode::kAggregate;
      else if (mode == "first") p.map_mode = Mode::kFirst;
      else if (mode == "latest") p.map_mode = Mode::kLatest;
      else throw std::invalid_argument("map_mode must be aggregate, first or latest");
      p.map_voxel_size = declare_parameter("map_voxel_size", p.map_voxel_size);
      p.map_radius = declare_parameter("map_radius", p.map_radius);
      p.num_threads = declare_parameter("num_threads", p.num_threads);
      auto core = std::make_shared<litamin2::LiTAMIN2Odometry>(p);
      process_ = [core](const std::vector<Eigen::Vector3d>& points) {
        return core->process(points).pose;
      };
    }
    else throw std::invalid_argument("unknown paper pipeline: " + method);
    if (publish_tf) tf_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);
    odometry_ = create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    points_ = create_subscription<sensor_msgs::msg::PointCloud2>(
        "points_raw", input_qos,
        [this](sensor_msgs::msg::PointCloud2::ConstSharedPtr msg) { onScan(msg); });
  }

 private:
  static std::vector<Eigen::Vector3d> readPoints(const sensor_msgs::msg::PointCloud2& msg) {
    using Field = sensor_msgs::msg::PointField;
    if (!msg.width || !msg.height || !msg.point_step ||
        uint64_t(msg.row_step) < uint64_t(msg.width) * msg.point_step ||
        uint64_t(msg.data.size()) < uint64_t(msg.row_step) * msg.height)
      throw std::invalid_argument("invalid PointCloud2 dimensions or data length");
    std::array<Field, 3> xyz;
    for (size_t axis = 0; axis < xyz.size(); ++axis) {
      const std::string name(1, "xyz"[axis]);
      auto found = std::find_if(msg.fields.begin(), msg.fields.end(),
                                [&](const Field& f) { return f.name == name; });
      if (found == msg.fields.end() || found->count != 1 ||
          (found->datatype != Field::FLOAT32 && found->datatype != Field::FLOAT64) ||
          uint64_t(found->offset) + (found->datatype == Field::FLOAT32 ? 4 : 8) > msg.point_step)
        throw std::invalid_argument("PointCloud2 needs scalar float32/float64 XYZ fields");
      xyz[axis] = *found;
    }
    std::vector<Eigen::Vector3d> points;
    points.reserve(size_t(msg.width) * msg.height);
    for (size_t row = 0; row < msg.height; ++row)
      for (size_t col = 0; col < msg.width; ++col) {
        Eigen::Vector3d point;
        for (size_t axis = 0; axis < xyz.size(); ++axis) {
          const auto& f = xyz[axis];
          const size_t count = f.datatype == Field::FLOAT32 ? 4 : 8;
          const auto* bytes = msg.data.data() + row * msg.row_step + col * msg.point_step + f.offset;
          uint64_t bits = 0;
          for (size_t i = 0; i < count; ++i)
            bits |= uint64_t(bytes[i]) << (8 * (msg.is_bigendian ? count - i - 1 : i));
          if (count == 4) {
            const uint32_t small = static_cast<uint32_t>(bits);
            float value;
            std::memcpy(&value, &small, sizeof(value));
            point[axis] = value;
          } else {
            std::memcpy(&point[axis], &bits, sizeof(double));
          }
        }
        if (point.allFinite()) points.push_back(point);
      }
    return points;
  }

  void onScan(const sensor_msgs::msg::PointCloud2::ConstSharedPtr& msg) {
    if (msg->header.stamp.sec < 0 || msg->header.stamp.nanosec >= 1000000000) {
      RCLCPP_WARN(get_logger(), "Skipping scan: invalid acquisition timestamp");
      return;
    }
    const rclcpp::Time stamp(msg->header.stamp);
    if (stamp.nanoseconds() <= last_stamp_ || msg->header.frame_id.empty() ||
        msg->header.frame_id == map_frame_ ||
        (!sensor_frame_.empty() && msg->header.frame_id != sensor_frame_)) {
      RCLCPP_WARN(get_logger(), "Skipping scan: require increasing positive stamps and a stable sensor frame");
      return;
    }
    std::vector<Eigen::Vector3d> points;
    try {
      points = readPoints(*msg);
    } catch (const std::exception& error) {
      RCLCPP_WARN(get_logger(), "Cannot read scan: %s", error.what());
      return;
    }
    if (points.empty()) return;
    const Eigen::Matrix4d pose = process_(points);
    if (!pose.allFinite()) throw std::runtime_error("paper pipeline returned a nonfinite pose");
    last_stamp_ = stamp.nanoseconds();
    sensor_frame_ = msg->header.frame_id;
    const Eigen::Quaterniond q(pose.block<3, 3>(0, 0));
    const Eigen::Vector3d t = pose.block<3, 1>(0, 3);
    odometry_->publish(ros_utils::toOdometryMsg(q, t, map_frame_, sensor_frame_, stamp));
    if (tf_) tf_->sendTransform(ros_utils::toTransformMsg(q, t, map_frame_, sensor_frame_, stamp));
  }

  std::function<Eigen::Matrix4d(const std::vector<Eigen::Vector3d>&)> process_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr points_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odometry_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_;
  std::string map_frame_, sensor_frame_;
  int64_t last_stamp_ = 0;
};
}  // namespace

int runPaperOdometryNode(const std::string& method, int argc, char** argv) {
  rclcpp::init(argc, argv);
  int code = 0;
  try {
    rclcpp::spin(std::make_shared<PaperOdometryNode>(method));
  } catch (const std::exception& error) {
    RCLCPP_ERROR(rclcpp::get_logger("paper_odometry"), "%s", error.what());
    code = 1;
  }
  rclcpp::shutdown();
  return code;
}
