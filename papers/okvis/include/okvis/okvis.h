#pragma once

#include "imu_preintegration/imu_preintegration.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace okvis {

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

struct OkvisParams {
  CameraIntrinsics camera;
  imu_preintegration::ImuNoiseParams imu_noise;

  Eigen::Matrix3d body_R_camera = Eigen::Matrix3d::Identity();
  Eigen::Vector3d body_t_camera = Eigen::Vector3d::Zero();
  Eigen::Vector3d gravity = Eigen::Vector3d::Zero();
  Eigen::Vector3d initial_velocity = Eigen::Vector3d::Zero();

  int keyframe_stride = 1;
  int optimize_every_n_keyframes = 1;
  int max_keyframes = 5;
  double keyframe_translation_threshold = 0.2;
  double keyframe_rotation_threshold_rad =
      8.0 * 3.14159265358979323846 / 180.0;

  bool enable_imu_factors = true;
  bool enable_visual_factors = true;

  double imu_rotation_weight = 80.0;
  double imu_translation_weight = 25.0;
  double visual_pixel_sigma = 2.0;
  double visual_huber_delta = 4.0;
  int min_visual_observations = 4;
};

struct OkvisResult {
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
  int num_keyframes = 0;
  int num_imu_edges = 0;
  int num_visual_factors = 0;
};

class Okvis {
public:
  explicit Okvis(const OkvisParams& params = OkvisParams());

  void setLandmarks(const std::vector<Landmark>& landmarks);
  void clearLandmarks();

  OkvisResult process(
      const std::vector<VisualObservation>& visual_observations,
      const std::vector<imu_preintegration::ImuSample>& imu_samples = {});

  int numKeyframes() const { return static_cast<int>(window_frames_.size()); }
  int numImuEdges() const { return static_cast<int>(imu_edges_.size()); }
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

  struct FrameState {
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
  void optimizeWindow();
  void pruneWindow();

  static Eigen::Matrix4d poseToMatrix(const PoseState& pose);
  static double rotationAngle(const Eigen::Quaterniond& q);

  OkvisParams params_;
  std::vector<Landmark> landmarks_;
  std::unordered_map<int, size_t> landmark_lookup_;
  std::vector<FrameState> window_frames_;
  std::vector<ImuEdge> imu_edges_;
  PoseState current_state_;
  bool initialized_ = false;
  int valid_frame_count_ = 0;
};

}  // namespace okvis
}  // namespace localization_zoo
