#pragma once

#include "aloam/types.h"
#include "gicp/gicp_registration.h"
#include "ndt/ndt_registration.h"
#include "scan_context/scan_context.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cmath>
#include <deque>
#include <vector>

namespace localization_zoo {
namespace hdl_graph_slam {

struct HdlGraphSlamParams {
  HdlGraphSlamParams();

  ndt::NDTParams ndt;
  scan_context::ScanContextParams scan_context;
  gicp::GICPParams loop_gicp;

  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.7;
  double map_voxel_size = 1.0;

  int keyframe_stride = 1;
  double keyframe_translation_threshold = 0.8;
  double keyframe_rotation_threshold_rad = 8.0 * 3.14159265358979323846 / 180.0;
  int max_keyframes_in_submap = 30;

  int min_loop_index_gap = 5;
  int optimize_every_n_keyframes = 1;
  bool enable_loop_closure = true;
  bool enable_floor_constraint = true;

  double odom_rotation_weight = 80.0;
  double odom_translation_weight = 40.0;
  double loop_rotation_weight = 100.0;
  double loop_translation_weight = 50.0;
  double floor_weight = 10.0;
  double floor_height = 0.0;

  double loop_fitness_threshold = 1.0;
  int min_loop_correspondences = 40;

  size_t descriptor_stride = 2;
  size_t loop_stride = 3;
};

struct HdlGraphSlamResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  ndt::NDTResult front_end_result;
  bool initialized = false;
  bool valid = false;
  bool converged = false;
  bool keyframe_added = false;
  bool loop_detected = false;
  int loop_index = -1;
  int num_keyframes = 0;
  int num_loop_edges = 0;
  size_t submap_points = 0;
};

class HdlGraphSlam {
public:
  explicit HdlGraphSlam(const HdlGraphSlamParams& params = HdlGraphSlamParams());

  HdlGraphSlamResult process(const aloam::PointCloudConstPtr& cloud);
  void clear();

  const Eigen::Matrix4d& pose() const { return pose_; }
  int numKeyframes() const { return static_cast<int>(keyframes_.size()); }
  int numLoopEdges() const { return static_cast<int>(loop_edges_.size()); }
  size_t submapSize() const { return local_map_points_.size(); }
  bool initialized() const { return initialized_; }

  Eigen::Matrix4d latestOptimizedPoseMatrix() const;

private:
  struct PoseState {
    Eigen::Quaterniond q = Eigen::Quaterniond::Identity();
    Eigen::Vector3d t = Eigen::Vector3d::Zero();
  };

  struct Keyframe {
    PoseState raw_pose;
    PoseState optimized_pose;
    std::vector<Eigen::Vector3d> world_points;
    std::vector<Eigen::Vector3d> descriptor_points;
    std::vector<Eigen::Vector3d> loop_points;
  };

  struct OdomEdge {
    int from = -1;
    int to = -1;
    PoseState relative_pose;
  };

  struct LoopEdge {
    int from = -1;
    int to = -1;
    PoseState relative_pose;
    double fitness = 0.0;
    int correspondences = 0;
  };

  std::vector<Eigen::Vector3d> preprocess(
      const aloam::PointCloudConstPtr& cloud, double voxel_size) const;
  void rebuildSubmap();
  bool shouldCreateKeyframe(const Eigen::Matrix4d& candidate_pose) const;
  void addKeyframe(const Eigen::Matrix4d& pose,
                   const std::vector<Eigen::Vector3d>& map_points,
                   const std::vector<Eigen::Vector3d>& descriptor_points,
                   const std::vector<Eigen::Vector3d>& loop_points);
  bool tryAddLoopEdge(int loop_index, int current_index);
  void optimizePoseGraph();

  static std::vector<Eigen::Vector3d> transformPoints(
      const std::vector<Eigen::Vector3d>& points, const Eigen::Matrix4d& pose);
  static double rotationAngle(const Eigen::Matrix4d& transform);
  static PoseState relativePose(const PoseState& from, const PoseState& to);
  static PoseState matrixToPose(const Eigen::Matrix4d& transform);
  static Eigen::Matrix4d poseToMatrix(const PoseState& pose);
  static std::vector<Eigen::Vector3d> stridePoints(
      const std::vector<Eigen::Vector3d>& points, size_t stride);

  HdlGraphSlamParams params_;
  ndt::NDTRegistration registration_;
  scan_context::ScanContextManager scan_context_;

  std::vector<Keyframe> keyframes_;
  std::deque<int> submap_indices_;
  std::vector<Eigen::Vector3d> local_map_points_;
  std::vector<OdomEdge> odom_edges_;
  std::vector<LoopEdge> loop_edges_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_keyframe_pose_ = Eigen::Matrix4d::Identity();
  bool initialized_ = false;
  int valid_frame_count_ = 0;
};

}  // namespace hdl_graph_slam
}  // namespace localization_zoo
