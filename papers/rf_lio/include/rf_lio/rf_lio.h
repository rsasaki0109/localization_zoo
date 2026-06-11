#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <vector>

#include "id_lio/id_lio.h"
#include "imu_preintegration/imu_preintegration.h"

namespace localization_zoo {
namespace rf_lio {

struct RfLioParams {
  id_lio::IDLIOParams backend;

  int range_image_width = 1024;
  int range_image_height = 64;
  double fov_up_deg = 3.0;
  double fov_down_deg = -25.0;
  double min_range = 1.0;
  double max_range = 100.0;

  double foreground_margin = 0.8;
  int min_foreground_votes = 2;
  double max_removal_fraction = 0.35;
  int min_keep_points = 800;
};

struct RfLioResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  id_lio::IDLIOResult backend;
  int removal_first_points = 0;
  int kept_points = 0;
  int candidate_points = 0;
};

class RfLioPipeline {
 public:
  explicit RfLioPipeline(const RfLioParams& params = RfLioParams());

  void setInitialPose(const Eigen::Matrix4d& pose);

  std::vector<Eigen::Vector3d> removalFirstFilter(
      const std::vector<Eigen::Vector3d>& frame, int* removed_points = nullptr,
      int* candidate_points = nullptr) const;

  RfLioResult registerFrame(
      const std::vector<Eigen::Vector3d>& frame,
      const std::vector<imu_preintegration::ImuSample>& imu);

  const Eigen::Matrix4d& pose() const { return backend_.pose(); }
  size_t mapSize() const { return backend_.mapSize(); }
  size_t dynamicMapPoints() const { return backend_.dynamicMapPoints(); }

 private:
  struct RangeImage {
    int width = 0;
    int height = 0;
    std::vector<double> ranges;
    std::vector<bool> valid;
  };

  bool projectToRangeImage(const Eigen::Vector3d& p_sensor, int width,
                           int height, int* idx, double* range = nullptr) const;
  RangeImage buildRangeImage(const std::vector<Eigen::Vector3d>& points,
                             int width, int height) const;
  int foregroundVotes(const Eigen::Vector3d& p_sensor,
                      const std::vector<RangeImage>& predicted) const;

  RfLioParams params_;
  id_lio::IDLIOPipeline backend_;
  std::vector<Eigen::Vector3d> previous_static_scan_;
  Eigen::Matrix4d previous_pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  bool has_previous_ = false;
};

}  // namespace rf_lio
}  // namespace localization_zoo
