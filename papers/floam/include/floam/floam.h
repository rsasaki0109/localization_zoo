#pragma once

#include "aloam/laser_mapping.h"
#include "aloam/laser_odometry.h"
#include "aloam/scan_registration.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>

namespace localization_zoo {
namespace floam {

struct FLoamParams {
  FLoamParams();

  aloam::ScanRegistrationParams scan_registration;
  aloam::LaserOdometryParams odometry;
  aloam::LaserMappingParams mapping;

  size_t input_point_stride = 2;
  int mapping_update_interval = 2;
  bool enable_mapping = true;
};

struct FLoamResult {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  bool valid = false;
  bool odom_valid = false;
  bool mapping_updated = false;
  int frame_count = 0;
  int mapping_updates = 0;
  size_t num_input_points = 0;
  size_t num_processed_points = 0;
  size_t num_corner_less_sharp = 0;
  size_t num_surf_less_flat = 0;
};

class FLoam {
public:
  explicit FLoam(const FLoamParams& params = FLoamParams());

  FLoamResult process(const aloam::PointCloudConstPtr& cloud);
  void clear();

  int frameCount() const { return frame_count_; }
  int mappingUpdates() const { return mapping_updates_; }
  const Eigen::Quaterniond& orientation() const { return q_w_curr_; }
  const Eigen::Vector3d& position() const { return t_w_curr_; }

private:
  aloam::PointCloudPtr thinCloud(const aloam::PointCloudConstPtr& cloud) const;
  void updateMapOdomCorrection(const Eigen::Quaterniond& q_map,
                               const Eigen::Vector3d& t_map,
                               const Eigen::Quaterniond& q_odom,
                               const Eigen::Vector3d& t_odom);
  void applyMapOdomCorrection(const Eigen::Quaterniond& q_odom,
                              const Eigen::Vector3d& t_odom,
                              Eigen::Quaterniond* q_out,
                              Eigen::Vector3d* t_out) const;

  FLoamParams params_;
  aloam::ScanRegistration scan_registration_;
  aloam::LaserOdometry odometry_;
  aloam::LaserMapping mapping_;

  Eigen::Quaterniond q_w_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr_ = Eigen::Vector3d::Zero();
  Eigen::Quaterniond q_map_odom_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_map_odom_ = Eigen::Vector3d::Zero();
  int frame_count_ = 0;
  int mapping_updates_ = 0;
};

}  // namespace floam
}  // namespace localization_zoo
