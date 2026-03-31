#pragma once

#include "ct_lio/ct_lio_registration.h"

namespace localization_zoo {
namespace clins {

struct CLINSParams {
  CLINSParams();

  ct_lio::CTLIOParams registration;
  double scan_duration = 0.1;
  int keyframe_stride = 1;
  double keyframe_translation_threshold = 0.8;
  double keyframe_rotation_threshold_rad =
      8.0 * 3.14159265358979323846 / 180.0;
};

struct CLINSResult {
  ct_lio::CTLIOResult registration_result;
  ct_lio::CTLIOState state;
  bool initialized = false;
  bool valid = false;
  bool keyframe_added = false;
  int num_frames = 0;
  size_t map_size = 0;
};

class CLINS {
public:
  explicit CLINS(const CLINSParams& params = CLINSParams());

  CLINSResult process(
      const std::vector<ct_icp::TimedPoint>& timed_points,
      const std::vector<imu_preintegration::ImuSample>& imu_samples = {});
  void clear();

  const ct_lio::CTLIOState& state() const { return state_; }
  size_t mapSize() const { return registration_.mapSize(); }
  size_t numFramesInMap() const { return registration_.numFramesInMap(); }
  int numFrames() const { return frame_count_; }
  bool initialized() const { return initialized_; }

private:
  bool shouldAddKeyframe(const ct_icp::SE3& pose) const;
  std::vector<Eigen::Vector3d> transformPoints(
      const std::vector<ct_icp::TimedPoint>& timed_points,
      const ct_icp::TrajectoryFrame& frame) const;
  static double rotationAngle(const Eigen::Quaterniond& quat);

  CLINSParams params_;
  ct_lio::CTLIORegistration registration_;
  ct_lio::CTLIOState state_;
  ct_icp::SE3 last_keyframe_pose_;
  bool initialized_ = false;
  int frame_count_ = 0;
};

}  // namespace clins
}  // namespace localization_zoo
