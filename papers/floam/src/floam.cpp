#include "floam/floam.h"

#include <algorithm>

namespace localization_zoo {
namespace floam {

FLoamParams::FLoamParams() {
  scan_registration.curvature_threshold = 0.08f;
  scan_registration.max_corner_sharp = 1;
  scan_registration.max_corner_less_sharp = 10;
  scan_registration.max_surf_flat = 2;
  scan_registration.less_flat_leaf_size = 0.3f;

  odometry.num_optimization_iters = 1;
  odometry.ceres_max_iterations = 3;
  odometry.distance_sq_threshold = 16.0;

  mapping.num_optimization_iters = 1;
  mapping.ceres_max_iterations = 3;
  mapping.line_resolution = 0.6;
  mapping.plane_resolution = 1.0;
  mapping.knn_max_dist_sq = 4.0;
}

FLoam::FLoam(const FLoamParams& params)
    : params_(params),
      scan_registration_(params.scan_registration),
      odometry_(params.odometry),
      mapping_(params.mapping) {}

aloam::PointCloudPtr FLoam::thinCloud(const aloam::PointCloudConstPtr& cloud) const {
  auto thinned = aloam::PointCloudPtr(new aloam::PointCloud);
  if (cloud == nullptr) {
    return thinned;
  }

  const size_t stride = std::max<size_t>(1, params_.input_point_stride);
  thinned->reserve(cloud->size() / stride + 1);
  for (size_t i = 0; i < cloud->size(); i += stride) {
    thinned->push_back(cloud->points[i]);
  }
  return thinned;
}

void FLoam::updateMapOdomCorrection(const Eigen::Quaterniond& q_map,
                                    const Eigen::Vector3d& t_map,
                                    const Eigen::Quaterniond& q_odom,
                                    const Eigen::Vector3d& t_odom) {
  q_map_odom_ = (q_map * q_odom.conjugate()).normalized();
  t_map_odom_ = t_map - q_map_odom_ * t_odom;
}

void FLoam::applyMapOdomCorrection(const Eigen::Quaterniond& q_odom,
                                   const Eigen::Vector3d& t_odom,
                                   Eigen::Quaterniond* q_out,
                                   Eigen::Vector3d* t_out) const {
  if (q_out == nullptr || t_out == nullptr) {
    return;
  }

  *q_out = (q_map_odom_ * q_odom).normalized();
  *t_out = q_map_odom_ * t_odom + t_map_odom_;
}

FLoamResult FLoam::process(const aloam::PointCloudConstPtr& cloud) {
  FLoamResult result;
  result.q_w_curr = q_w_curr_;
  result.t_w_curr = t_w_curr_;
  result.frame_count = frame_count_;
  result.mapping_updates = mapping_updates_;
  result.num_input_points = cloud != nullptr ? cloud->size() : 0u;

  const aloam::PointCloudPtr thinned_cloud = thinCloud(cloud);
  result.num_processed_points = thinned_cloud->size();

  const aloam::FeatureCloud features = scan_registration_.extract(thinned_cloud);
  result.num_corner_less_sharp = features.corner_less_sharp->size();
  result.num_surf_less_flat = features.surf_less_flat->size();

  const aloam::OdometryResult odom_result = odometry_.process(features);
  result.odom_valid = odom_result.valid;
  if (!odom_result.valid) {
    return result;
  }

  frame_count_++;
  const bool should_update_mapping =
      params_.enable_mapping &&
      (mapping_updates_ == 0 ||
       (params_.mapping_update_interval > 0 &&
        frame_count_ % params_.mapping_update_interval == 0));

  if (should_update_mapping) {
    const aloam::MappingResult mapping_result =
        mapping_.process(features.corner_less_sharp, features.surf_less_flat,
                         features.full_cloud, odom_result.q_w_curr,
                         odom_result.t_w_curr);
    if (mapping_result.valid) {
      q_w_curr_ = mapping_result.q_w_curr.normalized();
      t_w_curr_ = mapping_result.t_w_curr;
      updateMapOdomCorrection(q_w_curr_, t_w_curr_, odom_result.q_w_curr,
                              odom_result.t_w_curr);
      mapping_updates_++;
      result.mapping_updated = true;
      result.valid = true;
    }
  }

  if (!result.valid) {
    applyMapOdomCorrection(odom_result.q_w_curr, odom_result.t_w_curr, &q_w_curr_,
                           &t_w_curr_);
    result.valid = true;
  }

  result.q_w_curr = q_w_curr_;
  result.t_w_curr = t_w_curr_;
  result.frame_count = frame_count_;
  result.mapping_updates = mapping_updates_;
  return result;
}

void FLoam::clear() {
  scan_registration_ = aloam::ScanRegistration(params_.scan_registration);
  odometry_ = aloam::LaserOdometry(params_.odometry);
  mapping_ = aloam::LaserMapping(params_.mapping);
  q_w_curr_ = Eigen::Quaterniond::Identity();
  t_w_curr_.setZero();
  q_map_odom_ = Eigen::Quaterniond::Identity();
  t_map_odom_.setZero();
  frame_count_ = 0;
  mapping_updates_ = 0;
}

}  // namespace floam
}  // namespace localization_zoo
