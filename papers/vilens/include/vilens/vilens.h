#pragma once

#include "imu_preintegration/imu_preintegration.h"
#include "relead/cesikf.h"

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace vilens {

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

struct VILENSParams {
  relead::CESIKFParams filter;
  CameraIntrinsics camera;

  Eigen::Matrix3d body_R_camera = Eigen::Matrix3d::Identity();
  Eigen::Vector3d body_t_camera = Eigen::Vector3d::Zero();

  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.5;
  double map_voxel_size = 0.8;
  int knn = 5;
  double max_correspondence_distance = 3.0;
  double planarity_threshold = 0.08;
  int max_correspondences = 200;

  double keyframe_translation_threshold = 0.6;
  double keyframe_rotation_threshold_rad = 8.0 * 3.14159265358979323846 / 180.0;
  int max_keyframes_in_map = 30;

  double lidar_prior_translation_weight = 0.5;
  double lidar_prior_rotation_weight = 0.5;
  double visual_pixel_sigma = 2.0;
  double visual_huber_delta = 4.0;
  int min_visual_observations = 4;
  int max_visual_iterations = 8;
};

struct VILENSResult {
  relead::State state;
  relead::DegeneracyInfo degeneracy_info;
  bool initialized = false;
  bool valid = false;
  bool converged = false;
  bool keyframe_added = false;
  bool imu_used = false;
  bool visual_used = false;
  int num_keyframes = 0;
  int num_correspondences = 0;
  int num_visual_factors = 0;
  int num_iterations = 0;
  size_t map_points = 0;
};

class VILENS {
public:
  explicit VILENS(const VILENSParams& params = VILENSParams());

  void setLandmarks(const std::vector<Landmark>& landmarks);
  void clearLandmarks();

  VILENSResult process(
      const std::vector<Eigen::Vector3d>& raw_points,
      const std::vector<VisualObservation>& visual_observations = {},
      const std::vector<imu_preintegration::ImuSample>& imu_samples = {});
  void clear();

  const relead::State& state() const { return filter_.state(); }
  int numKeyframes() const { return static_cast<int>(keyframes_.size()); }
  size_t mapSize() const { return map_points_.size(); }
  size_t numLandmarks() const { return landmarks_.size(); }

private:
  struct PoseOptimizationSummary {
    relead::State state;
    bool visual_used = false;
    bool converged = false;
    int num_factors = 0;
  };

  std::vector<Eigen::Vector3d> preprocess(
      const std::vector<Eigen::Vector3d>& raw_points, double voxel_size) const;
  void propagateImu(const std::vector<imu_preintegration::ImuSample>& imu_samples);
  std::vector<relead::PointWithNormal> buildCorrespondences(
      const std::vector<Eigen::Vector3d>& points_world) const;
  void rebuildMap();
  bool shouldAddKeyframe(const relead::State& candidate_state) const;
  void addKeyframe(const std::vector<Eigen::Vector3d>& points_world,
                   const relead::State& state);
  bool projectPoint(const relead::State& state,
                    const Eigen::Vector3d& landmark_world,
                    Eigen::Vector2d* pixel) const;
  PoseOptimizationSummary refineWithVision(
      const relead::State& prior_state,
      const std::vector<VisualObservation>& visual_observations) const;

  static std::vector<Eigen::Vector3d> transformPoints(
      const std::vector<Eigen::Vector3d>& points, const relead::State& state);
  static double rotationAngle(const Eigen::Matrix3d& rotation);

  VILENSParams params_;
  relead::CESIKF filter_;
  std::deque<std::vector<Eigen::Vector3d>> keyframes_;
  std::vector<Eigen::Vector3d> map_points_;
  pcl::PointCloud<pcl::PointXYZ>::Ptr map_cloud_;
  relead::State last_keyframe_state_;
  std::vector<Landmark> landmarks_;
  std::unordered_map<int, size_t> landmark_lookup_;
  bool initialized_ = false;
};

}  // namespace vilens
}  // namespace localization_zoo
