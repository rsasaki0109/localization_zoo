#pragma once

#include "aloam/laser_mapping.h"
#include "aloam/laser_odometry.h"
#include "aloam/types.h"

namespace localization_zoo {
namespace lego_loam {

struct GroundAwareScanRegistrationParams {
  int n_scans = 16;
  int ground_scan_limit = 5;
  int num_subregions = 6;
  int neighbor_window = 5;
  float minimum_range = 1.0f;
  float maximum_range = 100.0f;
  float sensor_height = 1.8f;
  float ground_height_tolerance = 0.4f;
  float sensor_mount_angle_deg = 0.0f;
  float ground_angle_threshold_deg = 10.0f;
  float scan_period = 0.1f;
  float curvature_threshold = 0.1f;
  int max_corner_sharp = 2;
  int max_corner_less_sharp = 20;
  int max_surf_flat = 4;
  float less_flat_leaf_size = 0.2f;
};

struct GroundFeatureCloud {
  aloam::FeatureCloud features;
  size_t num_ground_points = 0;
};

class GroundAwareScanRegistration {
public:
  explicit GroundAwareScanRegistration(
      const GroundAwareScanRegistrationParams& params =
          GroundAwareScanRegistrationParams())
      : params_(params) {}

  GroundFeatureCloud extract(const aloam::PointCloudConstPtr& cloud) const;

private:
  int computeScanId(float x, float y, float z) const;

  GroundAwareScanRegistrationParams params_;
};

struct LeGOLOAMParams {
  GroundAwareScanRegistrationParams scan_registration;
  aloam::LaserOdometryParams odometry;
  aloam::LaserMappingParams mapping;
};

struct LeGOLOAMResult {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  size_t num_ground_points = 0;
  size_t num_corner_features = 0;
  size_t num_surface_features = 0;
  bool valid = false;
};

class LeGOLOAM {
public:
  explicit LeGOLOAM(const LeGOLOAMParams& params = LeGOLOAMParams());

  LeGOLOAMResult process(const aloam::PointCloudConstPtr& cloud);

private:
  GroundAwareScanRegistration scan_registration_;
  aloam::LaserOdometry odometry_;
  aloam::LaserMapping mapping_;
};

}  // namespace lego_loam
}  // namespace localization_zoo
