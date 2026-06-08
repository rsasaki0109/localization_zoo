#pragma once

#include "aloam/laser_mapping.h"
#include "aloam/laser_odometry.h"
#include "aloam/scan_registration.h"
#include "aloam/types.h"
#include "pl_loam/pl_loam.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <vector>

namespace localization_zoo {
namespace vlom {

/// Scale-correction VLOM (arXiv:2304.08978): loosely coupled monocular VO +
/// A-LOAM LiDAR odometry/mapping with LiDAR depth scale correction and visual
/// motion bootstrapping.
///
/// KITTI Odometry には RGB が無いため、PL-LOAM と同様に LiDAR 投影疑似画像上で
/// 視覚特徴を抽出する (depth-gradient pseudo-image)。

struct VlomParams {
  pl_loam::PlLoamParams visual;
  aloam::ScanRegistrationParams scan_registration;
  aloam::LaserOdometryParams odometry;
  aloam::LaserMappingParams mapping;
  bool enable_visual_bootstrap = true;
  bool enable_scale_correction = true;
  int scale_correction_interval = 5;
  double mad_outlier_k = 2.5;
  bool enable_mapping = true;
  size_t input_point_stride = 1;
};

struct VlomResult {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  bool valid = false;
  bool odom_valid = false;
  bool mapping_updated = false;
  bool visual_bootstrap_used = false;
  int frame_count = 0;
  size_t num_visual_matches = 0;
  size_t num_scale_samples = 0;
  double scale_factor = 1.0;
  double cumulative_scale = 1.0;
  double mean_depth_prior_residual = 0.0;
};

class VlomCore {
 public:
  /// 2 視線の最近接点から cam0 座標系での三角測量深度を推定。
  static bool triangulateDepth(const pl_loam::CameraModel& camera,
                               const Eigen::Vector2d& uv_prev,
                               const Eigen::Vector2d& uv_curr,
                               const Eigen::Matrix3d& R_prev_curr,
                               const Eigen::Vector3d& t_prev_curr,
                               double* depth_curr);
  /// LiDAR 深度 / 三角測量深度 の比のロバスト中央値 (MAD 外れ値除去)。
  static double scaleCorrectionFactor(
      const std::vector<double>& lidar_depths,
      const std::vector<double>& triangulated_depths, double mad_k = 2.5);
  /// カメラ相対姿勢を Velodyne frame-to-frame 変換へ。
  static Eigen::Matrix4d camMotionToVelo(const pl_loam::CameraModel& camera,
                                           const Eigen::Matrix4d& T_cam);
};

class Vlom {
 public:
  explicit Vlom(const VlomParams& params = VlomParams());

  VlomResult process(const aloam::PointCloudConstPtr& cloud,
                     const std::vector<float>& intensity);
  void clear();

  int frameCount() const { return frame_count_; }
  const Eigen::Quaterniond& orientation() const { return q_w_curr_; }
  const Eigen::Vector3d& position() const { return t_w_curr_; }

 private:
  bool processVisual(const std::vector<Eigen::Vector3d>& points,
                     const std::vector<float>& intensity,
                     Eigen::Matrix4d* T_velo_prev_curr, double* scale_factor,
                     size_t* num_matches, size_t* num_scale_samples,
                     double* mean_depth_residual);

  VlomParams params_;
  aloam::ScanRegistration scan_reg_;
  aloam::LaserOdometry laser_odom_;
  aloam::LaserMapping laser_map_;

  std::vector<pl_loam::PointFeature> prev_points_;
  std::vector<pl_loam::LineFeature> prev_lines_;
  Eigen::Quaterniond q_visual_w_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_visual_w_ = Eigen::Vector3d::Zero();
  Eigen::Quaterniond q_visual_last_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_visual_last_curr_ = Eigen::Vector3d::Zero();
  double cumulative_visual_scale_ = 1.0;

  Eigen::Quaterniond q_w_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr_ = Eigen::Vector3d::Zero();
  bool visual_initialized_ = false;
  int frame_count_ = 0;
};

}  // namespace vlom
}  // namespace localization_zoo
