#pragma once

#include "aloam/laser_mapping.h"
#include "aloam/laser_odometry.h"
#include "aloam/types.h"

namespace localization_zoo {
namespace loam_livox {

struct LivoxScanRegistrationParams {
  int num_azimuth_sectors = 6;
  int num_subregions = 6;
  int neighbor_window = 5;
  float minimum_range = 1.0f;
  float maximum_range = 100.0f;
  float scan_period = 0.1f;
  float curvature_threshold = 0.05f;
  int max_corner_sharp = 4;
  int max_corner_less_sharp = 24;
  int max_surf_flat = 6;
  float less_flat_leaf_size = 0.2f;
};

class LivoxScanRegistration {
public:
  explicit LivoxScanRegistration(const LivoxScanRegistrationParams& params =
                                     LivoxScanRegistrationParams())
      : params_(params) {}

  aloam::FeatureCloud extract(const aloam::PointCloudConstPtr& cloud) const;

private:
  LivoxScanRegistrationParams params_;
};

struct LivoxLOAMParams {
  LivoxScanRegistrationParams scan_registration;
  aloam::LaserOdometryParams odometry;
  aloam::LaserMappingParams mapping;
};

struct LivoxLOAMResult {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  size_t num_corner_features = 0;
  size_t num_surface_features = 0;
  bool valid = false;
};

class LivoxLOAM {
public:
  explicit LivoxLOAM(const LivoxLOAMParams& params = LivoxLOAMParams());

  LivoxLOAMResult process(const aloam::PointCloudConstPtr& cloud);

private:
  LivoxScanRegistration scan_registration_;
  aloam::LaserOdometry odometry_;
  aloam::LaserMapping mapping_;
};

}  // namespace loam_livox
}  // namespace localization_zoo
