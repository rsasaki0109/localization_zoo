#pragma once

#include "fast_lio2/fast_lio2.h"

#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace fast_livo2 {

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

struct FastLivo2Params {
  fast_lio2::FastLio2Params front_end;
  CameraIntrinsics camera;

  Eigen::Matrix3d body_R_camera = Eigen::Matrix3d::Identity();
  Eigen::Vector3d body_t_camera = Eigen::Vector3d::Zero();

  double lidar_prior_translation_weight = 0.5;
  double lidar_prior_rotation_weight = 0.5;
  double visual_pixel_sigma = 2.0;
  double visual_huber_delta = 4.0;
  int min_visual_observations = 4;
  int max_visual_iterations = 8;
};

struct FastLivo2Result {
  fast_lio2::FastLio2State front_end_state;
  fast_lio2::FastLio2State state;
  bool initialized = false;
  bool valid = false;
  bool converged = false;
  bool imu_used = false;
  bool visual_used = false;
  int num_visual_factors = 0;
  size_t map_size = 0;
};

class FastLivo2 {
public:
  explicit FastLivo2(const FastLivo2Params& params = FastLivo2Params());

  void setLandmarks(const std::vector<Landmark>& landmarks);
  void clearLandmarks();

  FastLivo2Result process(
      const std::vector<Eigen::Vector3d>& raw_points,
      const std::vector<VisualObservation>& visual_observations = {},
      const std::vector<imu_preintegration::ImuSample>& imu_samples = {});
  void clear();

  const fast_lio2::FastLio2State& state() const { return front_end_.state(); }
  size_t mapSize() const { return front_end_.mapSize(); }
  size_t numLandmarks() const { return landmarks_.size(); }
  bool initialized() const { return front_end_.initialized(); }

private:
  struct PoseOptimizationSummary {
    fast_lio2::FastLio2State state;
    bool visual_used = false;
    bool converged = false;
    int num_factors = 0;
  };

  bool projectLandmark(const fast_lio2::FastLio2State& state,
                       const Eigen::Vector3d& landmark_world,
                       Eigen::Vector2d* pixel) const;
  PoseOptimizationSummary refineWithVision(
      const fast_lio2::FastLio2State& prior_state,
      const std::vector<VisualObservation>& visual_observations) const;

  FastLivo2Params params_;
  fast_lio2::FastLio2 front_end_;
  std::vector<Landmark> landmarks_;
  std::unordered_map<int, size_t> landmark_lookup_;
};

}  // namespace fast_livo2
}  // namespace localization_zoo
