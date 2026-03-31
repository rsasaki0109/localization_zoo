#pragma once

#include "fast_lio2/fast_lio2.h"
#include "gicp/gicp_registration.h"
#include "scan_context/scan_context.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cmath>
#include <vector>

namespace localization_zoo {
namespace fast_lio_slam {

struct FastLioSlamParams {
  FastLioSlamParams();

  fast_lio2::FastLio2Params front_end;
  scan_context::ScanContextParams scan_context;
  gicp::GICPParams loop_gicp;

  int keyframe_stride = 1;
  double keyframe_translation_threshold = 0.8;
  double keyframe_rotation_threshold_rad = 8.0 * 3.14159265358979323846 / 180.0;
  int min_loop_index_gap = 5;
  int optimize_every_n_keyframes = 1;

  bool enable_loop_closure = true;

  double odom_rotation_weight = 80.0;
  double odom_translation_weight = 40.0;
  double loop_rotation_weight = 100.0;
  double loop_translation_weight = 50.0;

  double loop_fitness_threshold = 1.0;
  int min_loop_correspondences = 40;

  size_t descriptor_stride = 2;
  size_t loop_stride = 4;
};

struct FastLioSlamResult {
  fast_lio2::FastLio2State front_end_state;
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  bool initialized = false;
  bool valid = false;
  bool imu_used = false;
  bool keyframe_added = false;
  bool loop_detected = false;
  int loop_index = -1;
  int num_keyframes = 0;
  int num_loop_edges = 0;
};

class FastLioSlam {
public:
  explicit FastLioSlam(const FastLioSlamParams& params = FastLioSlamParams());

  FastLioSlamResult process(
      const std::vector<Eigen::Vector3d>& raw_points,
      const std::vector<imu_preintegration::ImuSample>& imu_samples = {});

  int numKeyframes() const { return static_cast<int>(keyframes_.size()); }
  int numLoopEdges() const { return static_cast<int>(loop_edges_.size()); }
  bool initialized() const { return front_end_.initialized(); }

  Eigen::Matrix4d latestOptimizedPoseMatrix() const;
  void clear();

private:
  struct PoseState {
    Eigen::Quaterniond q = Eigen::Quaterniond::Identity();
    Eigen::Vector3d t = Eigen::Vector3d::Zero();
  };

  struct Keyframe {
    PoseState raw_pose;
    PoseState optimized_pose;
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

  bool shouldCreateKeyframe(const PoseState& pose) const;
  bool tryAddLoopEdge(int loop_index, int current_index);
  void optimizePoseGraph();

  static PoseState relativePose(const PoseState& from, const PoseState& to);
  static PoseState matrixToPose(const Eigen::Matrix4d& transform);
  static Eigen::Matrix4d poseToMatrix(const PoseState& pose);
  static std::vector<Eigen::Vector3d> stridePoints(
      const std::vector<Eigen::Vector3d>& points, size_t stride);

  FastLioSlamParams params_;
  fast_lio2::FastLio2 front_end_;
  scan_context::ScanContextManager scan_context_;

  std::vector<Keyframe> keyframes_;
  std::vector<OdomEdge> odom_edges_;
  std::vector<LoopEdge> loop_edges_;
  int valid_frame_count_ = 0;
};

}  // namespace fast_lio_slam
}  // namespace localization_zoo
