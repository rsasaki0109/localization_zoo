#pragma once

#include "aloam/types.h"
#include "floam/floam.h"
#include "gicp/gicp_registration.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cmath>
#include <vector>

namespace localization_zoo {
namespace isc_loam {

struct IntensityScanContextParams {
  int num_rings = 20;
  int num_sectors = 60;
  double max_radius = 80.0;
  int exclude_recent_frames = 30;
  int num_candidates = 10;
  double search_ratio = 0.1;
  double distance_threshold = 0.18;
  double intensity_scale = 1.0;
};

struct IntensityLoopCandidate {
  bool valid = false;
  int index = -1;
  double distance = 1.0;
  int yaw_shift = 0;
  double yaw_offset_rad = 0.0;
};

class IntensityScanContextManager {
public:
  explicit IntensityScanContextManager(
      const IntensityScanContextParams& params = IntensityScanContextParams())
      : params_(params) {}

  Eigen::MatrixXd makeDescriptor(const aloam::PointCloudConstPtr& cloud) const;
  Eigen::VectorXd makeRingKey(const Eigen::MatrixXd& descriptor) const;
  Eigen::VectorXd makeSectorKey(const Eigen::MatrixXd& descriptor) const;

  int addScan(const aloam::PointCloudConstPtr& cloud);
  IntensityLoopCandidate detectLoop(const aloam::PointCloudConstPtr& cloud) const;
  void clear();

  int numScans() const { return static_cast<int>(contexts_.size()); }

private:
  struct StoredContext {
    Eigen::MatrixXd descriptor;
    Eigen::VectorXd ring_key;
    Eigen::VectorXd sector_key;
  };

  int alignSectorKeys(const Eigen::VectorXd& query,
                      const Eigen::VectorXd& candidate) const;
  double compareDescriptors(const Eigen::MatrixXd& query,
                            const Eigen::VectorXd& query_sector_key,
                            const StoredContext& candidate,
                            int* best_shift) const;
  static Eigen::MatrixXd circularShiftColumns(const Eigen::MatrixXd& matrix,
                                              int shift);

  IntensityScanContextParams params_;
  std::vector<StoredContext> contexts_;
};

struct IscLoamParams {
  IscLoamParams();

  floam::FLoamParams front_end;
  IntensityScanContextParams scan_context;
  gicp::GICPParams loop_gicp;

  int keyframe_stride = 1;
  double keyframe_translation_threshold = 0.8;
  double keyframe_rotation_threshold_rad =
      8.0 * 3.14159265358979323846 / 180.0;
  int min_loop_index_gap = 5;
  int optimize_every_n_keyframes = 1;

  bool enable_loop_closure = true;

  double odom_rotation_weight = 60.0;
  double odom_translation_weight = 25.0;
  double loop_rotation_weight = 80.0;
  double loop_translation_weight = 40.0;

  double min_range = 1.0;
  double max_range = 100.0;
  size_t loop_stride = 3;
  double loop_fitness_threshold = 1.0;
  int min_loop_correspondences = 40;
};

struct IscLoamResult {
  floam::FLoamResult front_end_result;
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  bool valid = false;
  bool keyframe_added = false;
  bool loop_detected = false;
  int loop_index = -1;
  int num_keyframes = 0;
  int num_loop_edges = 0;
};

class IscLoam {
public:
  explicit IscLoam(const IscLoamParams& params = IscLoamParams());

  IscLoamResult process(const aloam::PointCloudConstPtr& cloud);
  void clear();

  int numKeyframes() const { return static_cast<int>(keyframes_.size()); }
  int numLoopEdges() const { return static_cast<int>(loop_edges_.size()); }

  Eigen::Matrix4d latestOptimizedPoseMatrix() const;

private:
  struct PoseState {
    Eigen::Quaterniond q = Eigen::Quaterniond::Identity();
    Eigen::Vector3d t = Eigen::Vector3d::Zero();
  };

  struct Keyframe {
    PoseState raw_pose;
    PoseState optimized_pose;
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
  std::vector<Eigen::Vector3d> makeLoopPoints(
      const aloam::PointCloudConstPtr& cloud, size_t stride) const;

  static PoseState relativePose(const PoseState& from, const PoseState& to);
  static PoseState matrixToPose(const Eigen::Matrix4d& transform);
  static Eigen::Matrix4d poseToMatrix(const PoseState& pose);

  IscLoamParams params_;
  floam::FLoam front_end_;
  IntensityScanContextManager scan_context_;

  std::vector<Keyframe> keyframes_;
  std::vector<OdomEdge> odom_edges_;
  std::vector<LoopEdge> loop_edges_;
  int valid_frame_count_ = 0;
};

}  // namespace isc_loam
}  // namespace localization_zoo
