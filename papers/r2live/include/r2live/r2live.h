#pragma once

#include "fast_lio2/fast_lio2.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <vector>

namespace localization_zoo {
namespace r2live {

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

struct R2LiveParams {
  fast_lio2::FastLio2Params front_end;
  CameraIntrinsics camera;

  Eigen::Matrix3d body_R_camera = Eigen::Matrix3d::Identity();
  Eigen::Vector3d body_t_camera = Eigen::Vector3d::Zero();

  int keyframe_stride = 1;
  double keyframe_translation_threshold = 0.6;
  double keyframe_rotation_threshold_rad =
      8.0 * 3.14159265358979323846 / 180.0;
  int optimize_every_n_keyframes = 1;

  int min_visual_observations = 4;
  double visual_pixel_sigma = 2.0;
  double visual_huber_delta = 4.0;

  double odom_rotation_weight = 80.0;
  double odom_translation_weight = 40.0;
};

struct R2LiveResult {
  fast_lio2::FastLio2State front_end_state;
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  bool initialized = false;
  bool valid = false;
  bool imu_used = false;
  bool keyframe_added = false;
  bool visual_used = false;
  int num_keyframes = 0;
  int num_visual_factors = 0;
};

class R2Live {
public:
  explicit R2Live(const R2LiveParams& params = R2LiveParams());

  void setLandmarks(const std::vector<Landmark>& landmarks);
  void clearLandmarks();

  R2LiveResult process(
      const std::vector<Eigen::Vector3d>& raw_points,
      const std::vector<VisualObservation>& visual_observations = {},
      const std::vector<imu_preintegration::ImuSample>& imu_samples = {});

  int numKeyframes() const { return static_cast<int>(keyframes_.size()); }
  size_t numLandmarks() const { return landmarks_.size(); }
  bool initialized() const { return front_end_.initialized(); }
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
    std::vector<MatchedObservation> observations;
  };

  struct OdomEdge {
    int from = -1;
    int to = -1;
    PoseState relative_pose;
  };

  bool shouldCreateKeyframe(const PoseState& pose) const;
  std::vector<MatchedObservation> matchObservations(
      const PoseState& pose,
      const std::vector<VisualObservation>& visual_observations) const;
  bool projectLandmark(const PoseState& pose,
                       const Eigen::Vector3d& landmark_world,
                       Eigen::Vector2d* pixel) const;
  void optimizePoseGraph();

  static PoseState relativePose(const PoseState& from, const PoseState& to);
  static Eigen::Matrix4d poseToMatrix(const PoseState& pose);
  static double rotationAngle(const Eigen::Quaterniond& quat);

  R2LiveParams params_;
  fast_lio2::FastLio2 front_end_;

  std::vector<Landmark> landmarks_;
  std::unordered_map<int, size_t> landmark_lookup_;

  std::vector<Keyframe> keyframes_;
  std::vector<OdomEdge> odom_edges_;
  int valid_frame_count_ = 0;
};

}  // namespace r2live
}  // namespace localization_zoo
