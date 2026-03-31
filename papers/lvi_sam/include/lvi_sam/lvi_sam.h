#pragma once

#include "aloam/laser_mapping.h"
#include "aloam/laser_odometry.h"
#include "aloam/scan_registration.h"
#include "gicp/gicp_registration.h"
#include "imu_preintegration/imu_preintegration.h"
#include "scan_context/scan_context.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cmath>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace lvi_sam {

struct CameraIntrinsics {
  double fx = 160.0;
  double fy = 160.0;
  double cx = 320.0;
  double cy = 240.0;
  int width = 640;
  int height = 480;
};

struct Landmark {
  int id = -1;
  Eigen::Vector3d position = Eigen::Vector3d::Zero();
};

struct VisualObservation {
  int landmark_id = -1;
  Eigen::Vector2d pixel = Eigen::Vector2d::Zero();
};

struct LviSamParams {
  LviSamParams();

  aloam::ScanRegistrationParams scan_registration;
  aloam::LaserOdometryParams odometry;
  aloam::LaserMappingParams mapping;
  scan_context::ScanContextParams scan_context;
  gicp::GICPParams loop_gicp;
  imu_preintegration::ImuNoiseParams imu_noise;
  CameraIntrinsics camera;

  Eigen::Matrix3d body_R_camera = Eigen::Matrix3d::Identity();
  Eigen::Vector3d body_t_camera = Eigen::Vector3d::Zero();

  int keyframe_stride = 1;
  double keyframe_translation_threshold = 0.8;
  double keyframe_rotation_threshold_rad = 8.0 * 3.14159265358979323846 / 180.0;
  int min_loop_index_gap = 5;
  int optimize_every_n_keyframes = 1;

  bool enable_loop_closure = true;
  bool enable_imu_rotation_prior = true;
  bool enable_visual_factors = true;

  double odom_rotation_weight = 60.0;
  double odom_translation_weight = 25.0;
  double loop_rotation_weight = 80.0;
  double loop_translation_weight = 40.0;
  double imu_rotation_weight = 10.0;

  double visual_pixel_sigma = 2.0;
  double visual_huber_delta = 4.0;
  int min_visual_observations = 4;

  double loop_fitness_threshold = 1.0;
  int min_loop_correspondences = 40;
};

struct LviSamResult {
  Eigen::Quaterniond raw_q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d raw_t_w_curr = Eigen::Vector3d::Zero();
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  bool valid = false;
  bool keyframe_added = false;
  bool loop_detected = false;
  bool imu_used = false;
  bool visual_used = false;
  int loop_index = -1;
  int num_keyframes = 0;
  int num_loop_edges = 0;
  int num_visual_factors = 0;
};

class LviSam {
public:
  explicit LviSam(const LviSamParams& params = LviSamParams());

  void setLandmarks(const std::vector<Landmark>& landmarks);
  void clearLandmarks();

  LviSamResult process(
      const aloam::PointCloudConstPtr& cloud,
      const std::vector<VisualObservation>& visual_observations = {},
      const std::vector<imu_preintegration::ImuSample>& imu_samples = {});

  int numKeyframes() const { return static_cast<int>(keyframes_.size()); }
  int numLoopEdges() const { return static_cast<int>(loop_edges_.size()); }
  int numImuEdges() const { return static_cast<int>(imu_edges_.size()); }
  size_t numLandmarks() const { return landmarks_.size(); }
  bool initialized() const { return !keyframes_.empty(); }

  Eigen::Matrix4d latestOptimizedPoseMatrix() const;
  void clear();

private:
  struct PoseState {
    Eigen::Quaterniond q = Eigen::Quaterniond::Identity();
    Eigen::Vector3d t = Eigen::Vector3d::Zero();
  };

  struct MatchedObservation {
    int landmark_index = -1;
    Eigen::Vector2d pixel = Eigen::Vector2d::Zero();
  };

  struct Keyframe {
    PoseState raw_pose;
    PoseState optimized_pose;
    std::vector<Eigen::Vector3d> descriptor_points;
    std::vector<Eigen::Vector3d> loop_points;
    std::vector<MatchedObservation> observations;
  };

  struct OdomEdge {
    int from = -1;
    int to = -1;
    PoseState relative_pose;
  };

  struct ImuEdge {
    int from = -1;
    int to = -1;
    Eigen::Quaterniond delta_q = Eigen::Quaterniond::Identity();
  };

  struct LoopEdge {
    int from = -1;
    int to = -1;
    PoseState relative_pose;
    double fitness = 0.0;
    int correspondences = 0;
  };

  bool shouldCreateKeyframe(const PoseState& pose) const;
  std::vector<MatchedObservation> matchObservations(
      const PoseState& pose,
      const std::vector<VisualObservation>& visual_observations) const;
  bool projectLandmark(const PoseState& pose,
                       const Eigen::Vector3d& landmark_world,
                       Eigen::Vector2d* pixel) const;
  bool tryAddLoopEdge(int loop_index, int current_index);
  void optimizePoseGraph();

  static PoseState relativePose(const PoseState& from, const PoseState& to);
  static PoseState matrixToPose(const Eigen::Matrix4d& transform);
  static Eigen::Matrix4d poseToMatrix(const PoseState& pose);
  static double rotationAngle(const Eigen::Quaterniond& q);

  LviSamParams params_;
  aloam::ScanRegistration scan_registration_;
  aloam::LaserOdometry odometry_;
  aloam::LaserMapping mapping_;
  scan_context::ScanContextManager scan_context_;

  std::vector<Landmark> landmarks_;
  std::unordered_map<int, size_t> landmark_lookup_;
  std::vector<Keyframe> keyframes_;
  std::vector<OdomEdge> odom_edges_;
  std::vector<ImuEdge> imu_edges_;
  std::vector<LoopEdge> loop_edges_;
  std::vector<imu_preintegration::ImuSample> pending_imu_samples_;
  int valid_frame_count_ = 0;
};

}  // namespace lvi_sam
}  // namespace localization_zoo
