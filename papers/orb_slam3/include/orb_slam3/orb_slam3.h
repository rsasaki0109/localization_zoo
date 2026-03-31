#pragma once

#include "imu_preintegration/imu_preintegration.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace orb_slam3 {

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

struct OrbSlam3Params {
  CameraIntrinsics camera;
  imu_preintegration::ImuNoiseParams imu_noise;

  Eigen::Matrix3d body_R_camera = Eigen::Matrix3d::Identity();
  Eigen::Vector3d body_t_camera = Eigen::Vector3d::Zero();
  Eigen::Vector3d gravity = Eigen::Vector3d::Zero();
  Eigen::Vector3d initial_velocity = Eigen::Vector3d::Zero();

  int keyframe_stride = 1;
  int optimize_every_n_keyframes = 1;
  double keyframe_translation_threshold = 0.2;
  double keyframe_rotation_threshold_rad =
      8.0 * 3.14159265358979323846 / 180.0;

  bool enable_imu_factors = true;
  bool enable_visual_factors = true;
  bool enable_loop_closure = true;

  double imu_rotation_weight = 80.0;
  double imu_translation_weight = 25.0;
  double loop_rotation_weight = 60.0;
  double loop_translation_weight = 30.0;
  double visual_pixel_sigma = 2.0;
  double visual_huber_delta = 4.0;
  double visual_prior_rotation_weight = 0.2;
  double visual_prior_translation_weight = 0.2;
  int min_visual_observations = 4;
  int min_loop_index_gap = 5;
  int min_shared_landmarks = 8;
};

struct OrbSlam3Result {
  Eigen::Quaterniond raw_q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d raw_t_w_curr = Eigen::Vector3d::Zero();
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  Eigen::Vector3d v_w_curr = Eigen::Vector3d::Zero();
  bool initialized = false;
  bool valid = false;
  bool keyframe_added = false;
  bool imu_used = false;
  bool visual_used = false;
  bool loop_detected = false;
  int loop_index = -1;
  int num_keyframes = 0;
  int num_imu_edges = 0;
  int num_loop_edges = 0;
  int num_visual_factors = 0;
};

class OrbSlam3 {
public:
  explicit OrbSlam3(const OrbSlam3Params& params = OrbSlam3Params());

  void setLandmarks(const std::vector<Landmark>& landmarks);
  void clearLandmarks();

  OrbSlam3Result process(
      const std::vector<VisualObservation>& visual_observations,
      const std::vector<imu_preintegration::ImuSample>& imu_samples = {});

  int numKeyframes() const { return static_cast<int>(keyframes_.size()); }
  int numImuEdges() const { return static_cast<int>(imu_edges_.size()); }
  int numLoopEdges() const { return static_cast<int>(loop_edges_.size()); }
  size_t numLandmarks() const { return landmarks_.size(); }
  bool initialized() const { return initialized_; }

  Eigen::Matrix4d latestOptimizedPoseMatrix() const;
  void clear();

private:
  struct PoseState {
    Eigen::Quaterniond q = Eigen::Quaterniond::Identity();
    Eigen::Vector3d t = Eigen::Vector3d::Zero();
    Eigen::Vector3d v = Eigen::Vector3d::Zero();
  };

  struct MatchedObservation {
    int landmark_index = -1;
    Eigen::Vector2d pixel = Eigen::Vector2d::Zero();
  };

  struct Keyframe {
    PoseState raw_pose;
    PoseState optimized_pose;
    std::vector<MatchedObservation> observations;
  };

  struct ImuEdge {
    int from = -1;
    int to = -1;
    Eigen::Quaterniond delta_q = Eigen::Quaterniond::Identity();
    Eigen::Vector3d delta_t = Eigen::Vector3d::Zero();
    double delta_t_sec = 0.0;
  };

  struct LoopEdge {
    int from = -1;
    int to = -1;
    Eigen::Quaterniond delta_q = Eigen::Quaterniond::Identity();
    Eigen::Vector3d delta_t = Eigen::Vector3d::Zero();
    int shared_landmarks = 0;
  };

  struct VisualPoseSummary {
    PoseState pose;
    bool valid = false;
    int num_factors = 0;
  };

  bool shouldCreateKeyframe(const PoseState& pose) const;
  PoseState predictPose(
      const std::vector<imu_preintegration::ImuSample>& imu_samples,
      bool* imu_used) const;
  std::vector<MatchedObservation> matchObservations(
      const PoseState& pose,
      const std::vector<VisualObservation>& visual_observations) const;
  bool projectLandmark(const PoseState& pose,
                       const Eigen::Vector3d& landmark_world,
                       Eigen::Vector2d* pixel) const;
  VisualPoseSummary estimateVisualPose(
      const PoseState& prior_pose,
      const std::vector<MatchedObservation>& observations) const;
  bool tryAddLoopEdge(int current_index, int* loop_index);
  void optimizePoseGraph();

  static Eigen::Quaterniond relativeQuaternion(const PoseState& from,
                                               const PoseState& to);
  static Eigen::Vector3d relativeTranslation(const PoseState& from,
                                             const PoseState& to);
  static Eigen::Matrix4d poseToMatrix(const PoseState& pose);
  static double rotationAngle(const Eigen::Quaterniond& q);

  OrbSlam3Params params_;
  std::vector<Landmark> landmarks_;
  std::unordered_map<int, size_t> landmark_lookup_;
  std::vector<Keyframe> keyframes_;
  std::vector<ImuEdge> imu_edges_;
  std::vector<LoopEdge> loop_edges_;
  PoseState current_state_;
  bool initialized_ = false;
  int valid_frame_count_ = 0;
};

}  // namespace orb_slam3
}  // namespace localization_zoo
