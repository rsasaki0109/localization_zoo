#include "vlom/vlom.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace localization_zoo {
namespace vlom {

namespace {

double medianInPlace(std::vector<double>* values) {
  if (values->empty()) return 0.0;
  const size_t mid = values->size() / 2;
  std::nth_element(values->begin(), values->begin() + static_cast<long>(mid),
                   values->end());
  return (*values)[mid];
}

std::vector<std::pair<int, int>> matchPoints(
    const std::vector<pl_loam::PointFeature>& prev,
    const std::vector<pl_loam::PointFeature>& curr, double max_px) {
  std::vector<std::pair<int, int>> matches;
  const double max_sq = max_px * max_px;
  for (size_t i = 0; i < curr.size(); ++i) {
    int best = -1;
    double best_d = max_sq;
    for (size_t j = 0; j < prev.size(); ++j) {
      const double d = (curr[i].uv - prev[j].uv).squaredNorm();
      if (d < best_d) {
        best_d = d;
        best = static_cast<int>(j);
      }
    }
    if (best >= 0) matches.emplace_back(best, static_cast<int>(i));
  }
  return matches;
}

}  // namespace

bool VlomCore::triangulateDepth(const pl_loam::CameraModel& camera,
                                const Eigen::Vector2d& uv_prev,
                                const Eigen::Vector2d& uv_curr,
                                const Eigen::Matrix3d& R_prev_curr,
                                const Eigen::Vector3d& t_prev_curr,
                                double* depth_curr) {
  if (depth_curr == nullptr) return false;
  Eigen::Vector3d ray_prev = camera.backProject(uv_prev, 1.0);
  Eigen::Vector3d ray_curr = camera.backProject(uv_curr, 1.0);
  if (ray_prev.norm() < 1e-6 || ray_curr.norm() < 1e-6) return false;
  ray_prev.normalize();
  ray_curr.normalize();

  const Eigen::Vector3d ray_curr_in_prev = R_prev_curr.transpose() * ray_curr;
  const Eigen::Vector3d origin_curr_in_prev = -R_prev_curr.transpose() * t_prev_curr;

  const Eigen::Vector3d w = ray_prev.cross(ray_curr_in_prev);
  const double sin2 = w.squaredNorm();
  if (sin2 < 1e-8) return false;

  const double t0 =
      (origin_curr_in_prev.cross(ray_curr_in_prev)).dot(w) / sin2;
  const double t1 =
      ((origin_curr_in_prev - t0 * ray_prev).cross(ray_prev)).dot(w) / sin2;
  if (t0 < 0.05 || t1 < 0.05) return false;

  const Eigen::Vector3d p_prev = t0 * ray_prev;
  const Eigen::Vector3d p_curr =
      R_prev_curr * p_prev + t_prev_curr;
  if (p_curr.z() <= 0.1) return false;
  *depth_curr = p_curr.z();
  return true;
}

double VlomCore::scaleCorrectionFactor(
    const std::vector<double>& lidar_depths,
    const std::vector<double>& triangulated_depths, double mad_k) {
  if (lidar_depths.size() != triangulated_depths.size() ||
      lidar_depths.size() < 3) {
    return 1.0;
  }

  std::vector<double> ratios;
  ratios.reserve(lidar_depths.size());
  for (size_t i = 0; i < lidar_depths.size(); ++i) {
    if (lidar_depths[i] < 0.5 || triangulated_depths[i] < 0.3) continue;
    ratios.push_back(lidar_depths[i] / triangulated_depths[i]);
  }
  if (ratios.size() < 3) return 1.0;

  const double med = medianInPlace(&ratios);
  std::vector<double> devs;
  devs.reserve(ratios.size());
  for (double r : ratios) devs.push_back(std::abs(r - med));
  const double mad = std::max(medianInPlace(&devs), 0.02);
  const double thr = mad_k * mad;

  std::vector<double> inliers;
  inliers.reserve(ratios.size());
  for (double r : ratios) {
    if (std::abs(r - med) <= thr) inliers.push_back(r);
  }
  if (inliers.size() < 3) return med;
  return medianInPlace(&inliers);
}

Eigen::Matrix4d VlomCore::camMotionToVelo(const pl_loam::CameraModel& camera,
                                          const Eigen::Matrix4d& T_cam) {
  const Eigen::Matrix4d T_velo_cam = camera.T_velo_cam;
  const Eigen::Matrix4d T_cam_velo = T_velo_cam.inverse();
  return T_cam_velo * T_cam * T_velo_cam;
}

Vlom::Vlom(const VlomParams& params)
    : params_(params),
      scan_reg_(params.scan_registration),
      laser_odom_(params.odometry),
      laser_map_(params.mapping) {}

void Vlom::clear() {
  prev_points_.clear();
  prev_lines_.clear();
  q_visual_w_ = Eigen::Quaterniond::Identity();
  t_visual_w_ = Eigen::Vector3d::Zero();
  q_visual_last_curr_ = Eigen::Quaterniond::Identity();
  t_visual_last_curr_ = Eigen::Vector3d::Zero();
  cumulative_visual_scale_ = 1.0;
  q_w_curr_ = Eigen::Quaterniond::Identity();
  t_w_curr_ = Eigen::Vector3d::Zero();
  visual_initialized_ = false;
  frame_count_ = 0;
}

bool Vlom::processVisual(const std::vector<Eigen::Vector3d>& points,
                         const std::vector<float>& intensity,
                         const pl_loam::GrayscaleImage* gray,
                         Eigen::Matrix4d* T_velo_prev_curr, double* scale_factor,
                         size_t* num_matches, size_t* num_scale_samples,
                         double* mean_depth_residual) {
  *num_matches = 0;
  *num_scale_samples = 0;
  *mean_depth_residual = 0.0;
  *scale_factor = 1.0;
  T_velo_prev_curr->setIdentity();

  std::vector<Eigen::Vector3d> subsampled;
  std::vector<float> sub_intensity;
  subsampled.reserve(points.size() / params_.visual.input_stride + 1);
  sub_intensity.reserve(points.size() / params_.visual.input_stride + 1);
  for (size_t i = 0; i < points.size(); i += params_.visual.input_stride) {
    subsampled.push_back(points[i]);
    sub_intensity.push_back(i < intensity.size() ? intensity[i] : 0.5f);
  }

  const auto depth_image = pl_loam::PlLoam::buildRangeImage(
      subsampled, sub_intensity, params_.visual.camera);

  const bool use_rgb =
      gray != nullptr && !gray->empty() &&
      gray->width == params_.visual.camera.width &&
      gray->height == params_.visual.camera.height;
  pl_loam::GrayscaleImage intensity_image;
  if (!use_rgb && params_.visual.use_intensity_pseudo_image) {
    intensity_image = pl_loam::PlLoam::buildIntensityImage(
        depth_image, params_.visual.intensity_dilation_radius);
  }

  std::vector<pl_loam::PointFeature> points_feat;
  if (use_rgb) {
    pl_loam::PlLoam::detectPointFeaturesGrayscale(
        *gray, params_.visual.max_point_features, params_.visual.harris_block,
        params_.visual.harris_k, &points_feat);
  } else if (!intensity_image.empty()) {
    pl_loam::PlLoam::detectPointFeaturesGrayscale(
        intensity_image, params_.visual.max_point_features,
        params_.visual.harris_block, params_.visual.harris_k, &points_feat);
  } else {
    pl_loam::PlLoam::detectPointFeatures(
        depth_image, params_.visual.max_point_features,
        params_.visual.harris_block, params_.visual.harris_k, &points_feat);
  }
  for (auto& f : points_feat) {
    f.has_depth = pl_loam::PlLoam::extractPointDepth(
        depth_image, f.uv, params_.visual.patch_radius, params_.visual.min_depth,
        params_.visual.max_depth, &f.depth_prior);
  }

  std::vector<pl_loam::LineFeature> lines_feat;
  if (params_.visual.use_line_features) {
    if (use_rgb) {
      pl_loam::PlLoam::detectLineFeaturesGrayscale(
          *gray, params_.visual.max_line_features, &lines_feat);
    } else if (!intensity_image.empty()) {
      pl_loam::PlLoam::detectLineFeaturesGrayscale(
          intensity_image, params_.visual.max_line_features, &lines_feat);
    } else {
      pl_loam::PlLoam::detectLineFeatures(depth_image,
                                          params_.visual.max_line_features,
                                          &lines_feat);
    }
    for (auto& l : lines_feat) {
      pl_loam::PlLoam::extractLineDepths(depth_image, &l,
                                         params_.visual.patch_radius,
                                         params_.visual.min_depth,
                                         params_.visual.max_depth);
    }
  }

  if (!visual_initialized_) {
    prev_points_ = std::move(points_feat);
    prev_lines_ = std::move(lines_feat);
    visual_initialized_ = true;
    return false;
  }

  const auto point_matches = matchPoints(prev_points_, points_feat,
                                         params_.visual.match_max_px);
  std::vector<std::pair<int, int>> line_matches;
  if (params_.visual.use_line_features) {
    const double max_px = params_.visual.match_max_px * 2.0;
    for (size_t i = 0; i < lines_feat.size(); ++i) {
      int best = -1;
      double best_d = max_px * max_px;
      const Eigen::Vector2d c_curr = 0.5 * (lines_feat[i].start + lines_feat[i].end);
      for (size_t j = 0; j < prev_lines_.size(); ++j) {
        const Eigen::Vector2d c_prev = 0.5 * (prev_lines_[j].start + prev_lines_[j].end);
        const double d = (c_curr - c_prev).squaredNorm();
        if (d < best_d) {
          best_d = d;
          best = static_cast<int>(j);
        }
      }
      if (best >= 0) line_matches.emplace_back(best, static_cast<int>(i));
    }
  }

  *num_matches = point_matches.size() + line_matches.size();

  Eigen::Matrix4d T_cam_prev_curr = Eigen::Matrix4d::Identity();
  T_cam_prev_curr.block<3, 3>(0, 0) = q_visual_last_curr_.toRotationMatrix();
  T_cam_prev_curr.block<3, 1>(0, 3) = t_visual_last_curr_;

  double pl_scale = 1.0;
  size_t num_priors = 0;
  const bool ok = pl_loam::PlLoam::optimizeRelativePose(
      params_.visual.camera, params_.visual, prev_points_, points_feat,
      prev_lines_, lines_feat, point_matches, line_matches, &T_cam_prev_curr,
      &pl_scale, &num_priors, mean_depth_residual, T_cam_prev_curr);

  if (!ok) {
    prev_points_ = std::move(points_feat);
    prev_lines_ = std::move(lines_feat);
    return false;
  }

  Eigen::Matrix3d R_cam = T_cam_prev_curr.block<3, 3>(0, 0);
  Eigen::Vector3d t_cam = T_cam_prev_curr.block<3, 1>(0, 3);

  if (params_.enable_scale_correction &&
      (frame_count_ % params_.scale_correction_interval) == 0) {
    std::vector<double> lidar_depths;
    std::vector<double> tri_depths;
    lidar_depths.reserve(point_matches.size());
    tri_depths.reserve(point_matches.size());
    for (const auto& m : point_matches) {
      const auto& prev = prev_points_[m.first];
      const auto& curr = points_feat[m.second];
      if (!curr.has_depth) continue;
      double tri = 0.0;
      if (!VlomCore::triangulateDepth(params_.visual.camera, prev.uv, curr.uv,
                                      R_cam, t_cam, &tri)) {
        continue;
      }
      lidar_depths.push_back(curr.depth_prior);
      tri_depths.push_back(tri);
    }
    *num_scale_samples = lidar_depths.size();
    if (lidar_depths.size() >= 3) {
      double raw = VlomCore::scaleCorrectionFactor(
          lidar_depths, tri_depths, params_.mad_outlier_k);
      *scale_factor = std::clamp(raw, 0.85, 1.15);
      t_cam *= *scale_factor;
      cumulative_visual_scale_ =
          0.95 * cumulative_visual_scale_ + 0.05 * (*scale_factor);
      T_cam_prev_curr.block<3, 1>(0, 3) = t_cam;
    }
  }

  q_visual_last_curr_ = Eigen::Quaterniond(R_cam);
  t_visual_last_curr_ = t_cam;

  Eigen::Matrix4d T_visual_world = Eigen::Matrix4d::Identity();
  T_visual_world.block<3, 3>(0, 0) = q_visual_w_.toRotationMatrix();
  T_visual_world.block<3, 1>(0, 3) = t_visual_w_;
  T_visual_world = T_visual_world * T_cam_prev_curr;
  q_visual_w_ = Eigen::Quaterniond(T_visual_world.block<3, 3>(0, 0));
  t_visual_w_ = T_visual_world.block<3, 1>(0, 3);

  *T_velo_prev_curr =
      VlomCore::camMotionToVelo(params_.visual.camera, T_cam_prev_curr);

  prev_points_ = std::move(points_feat);
  prev_lines_ = std::move(lines_feat);
  return true;
}

VlomResult Vlom::process(const aloam::PointCloudConstPtr& cloud,
                         const std::vector<float>& intensity,
                         const pl_loam::GrayscaleImage* gray) {
  VlomResult result;
  result.frame_count = frame_count_;

  std::vector<Eigen::Vector3d> points;
  points.reserve(cloud->size());
  std::vector<float> intens;
  intens.reserve(cloud->size());
  for (size_t i = 0; i < cloud->size(); i += params_.input_point_stride) {
    const auto& p = cloud->points[i];
    points.emplace_back(p.x, p.y, p.z);
    intens.push_back(i < intensity.size()
                         ? intensity[i]
                         : static_cast<float>(p.intensity));
  }

  Eigen::Matrix4d T_velo_prev_curr = Eigen::Matrix4d::Identity();
  double scale_factor = 1.0;
  size_t num_matches = 0;
  size_t num_scale_samples = 0;
  double mean_depth_residual = 0.0;
  const bool visual_ok =
      processVisual(points, intens, gray, &T_velo_prev_curr, &scale_factor,
                    &num_matches, &num_scale_samples, &mean_depth_residual);

  result.num_visual_matches = num_matches;
  result.num_scale_samples = num_scale_samples;
  result.scale_factor = scale_factor;
  result.cumulative_scale = cumulative_visual_scale_;
  result.mean_depth_prior_residual = mean_depth_residual;

  if (params_.enable_visual_bootstrap && visual_ok &&
      T_velo_prev_curr.block<3, 1>(0, 3).norm() < 2.0 &&
      scale_factor > 0.8 && scale_factor < 1.2) {
    laser_odom_.setMotionPrior(
        Eigen::Quaterniond(T_velo_prev_curr.block<3, 3>(0, 0)),
        T_velo_prev_curr.block<3, 1>(0, 3));
    result.visual_bootstrap_used = true;
  }

  const aloam::FeatureCloud features = scan_reg_.extract(cloud);
  const aloam::OdometryResult odom = laser_odom_.process(features);
  result.odom_valid = odom.valid;

  if (!odom.valid) {
    frame_count_++;
    return result;
  }

  if (params_.enable_mapping) {
    const aloam::MappingResult mapping = laser_map_.process(
        features.corner_less_sharp, features.surf_less_flat,
        features.full_cloud, odom.q_w_curr, odom.t_w_curr);
    result.mapping_updated = mapping.valid;
    if (mapping.valid) {
      q_w_curr_ = mapping.q_w_curr;
      t_w_curr_ = mapping.t_w_curr;
    } else {
      q_w_curr_ = odom.q_w_curr;
      t_w_curr_ = odom.t_w_curr;
    }
  } else {
    q_w_curr_ = odom.q_w_curr;
    t_w_curr_ = odom.t_w_curr;
  }

  result.q_w_curr = q_w_curr_;
  result.t_w_curr = t_w_curr_;
  result.valid = true;
  frame_count_++;
  result.frame_count = frame_count_;
  return result;
}

}  // namespace vlom
}  // namespace localization_zoo
