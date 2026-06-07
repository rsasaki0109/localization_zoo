#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <vector>

namespace localization_zoo {
namespace pl_loam {

/// PL-LOAM (Huang et al., ICRA 2020): LiDAR-monocular visual odometry with
/// point + line features and LiDAR depth priors in point-line bundle adjustment.
///
/// KITTI Odometry 評価では RGB が無いため、Velodyne を KITTI カメラへ投影した
/// 深度+反射強度画像上で Harris コーナー / 勾配線分を抽出し、論文の深度抽出と
/// depth-prior 付き PL-BA 機構を再現する (LiDAR-rendered pseudo-image)。

struct CameraModel {
  int width = 621;
  int height = 188;
  double fx = 359.428;
  double fy = 359.428;
  double cx = 303.596;
  double cy = 92.6079;
  /// Velodyne → camera (KITTI seq00 標準 calib, 半解像度)。
  Eigen::Matrix4d T_velo_cam = Eigen::Matrix4d::Identity();

  static CameraModel kittiHalfRes();
  Eigen::Vector3d veloToCam(const Eigen::Vector3d& p_velo) const;
  bool projectVelo(const Eigen::Vector3d& p_velo, Eigen::Vector2d* uv,
                   double* depth_cam) const;
  Eigen::Vector3d backProject(const Eigen::Vector2d& uv, double depth) const;
};

struct RangeImage {
  int width = 0;
  int height = 0;
  std::vector<float> depth;      // camera Z [m], 0 = invalid
  std::vector<float> intensity;  // [0,1]
};

struct PointFeature {
  Eigen::Vector2d uv = Eigen::Vector2d::Zero();
  double depth_prior = 0.0;
  bool has_depth = false;
  float score = 0.0f;
};

struct LineFeature {
  Eigen::Vector2d start = Eigen::Vector2d::Zero();
  Eigen::Vector2d end = Eigen::Vector2d::Zero();
  double depth_start = 0.0;
  double depth_end = 0.0;
  bool has_depth = false;
  float score = 0.0f;
};

struct PlLoamParams {
  CameraModel camera = CameraModel::kittiHalfRes();
  size_t input_stride = 2;
  int patch_radius = 4;
  int max_point_features = 280;
  int max_line_features = 64;
  int harris_block = 3;
  double harris_k = 0.04;
  double min_depth = 2.0;
  double max_depth = 80.0;
  bool use_depth_prior = true;
  bool use_line_features = true;
  bool use_scale_correction = true;
  double depth_prior_weight = 1.0;
  double reproj_sigma_px = 2.0;
  double line_sigma_px = 2.5;
  int ceres_max_iterations = 12;
  double match_max_px = 18.0;
};

struct PlLoamResult {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  bool valid = false;
  int frame_count = 0;
  size_t num_points = 0;
  size_t num_lines = 0;
  size_t num_point_matches = 0;
  size_t num_line_matches = 0;
  size_t num_depth_priors = 0;
  double scale_correction = 1.0;
  double mean_depth_prior_residual = 0.0;
};

class PlLoam {
 public:
  explicit PlLoam(const PlLoamParams& params = PlLoamParams());

  PlLoamResult process(const std::vector<Eigen::Vector3d>& points,
                       const std::vector<float>& intensity);
  void clear();

  int frameCount() const { return frame_count_; }
  const Eigen::Quaterniond& orientation() const { return q_w_curr_; }
  const Eigen::Vector3d& position() const { return t_w_curr_; }

  // --- 機構ヘルパ (ユニットテスト用) ---

  static RangeImage buildRangeImage(const std::vector<Eigen::Vector3d>& points,
                                    const std::vector<float>& intensity,
                                    const CameraModel& camera);
  /// 近傍パッチのロバスト中央値深度 (論文の点深度抽出の簡約版)。
  static bool extractPointDepth(const RangeImage& image,
                                const Eigen::Vector2d& uv, int patch_radius,
                                double min_depth, double max_depth,
                                double* depth_out);
  /// 端点ごとにパッチ深度を取り、線分上の LiDAR 点列で最小二乗平面交差を補強。
  static bool extractLineDepths(const RangeImage& image, LineFeature* line,
                                int patch_radius, double min_depth,
                                double max_depth);
  static void detectPointFeatures(const RangeImage& image,
                                  int max_features, int block_size,
                                  double harris_k,
                                  std::vector<PointFeature>* features);
  static void detectLineFeatures(const RangeImage& image, int max_features,
                                 std::vector<LineFeature>* features);
  /// 単眼三角測量深度と LiDAR 深度の比からスケール補正係数 (論文 scheme)。
  static double scaleCorrectionFactor(
      const std::vector<double>& lidar_depths,
      const std::vector<double>& visual_depths);
  /// depth-prior 付き point-line BA で相対姿勢を推定。
  static bool optimizeRelativePose(
      const CameraModel& camera, const PlLoamParams& params,
      const std::vector<PointFeature>& prev_points,
      const std::vector<PointFeature>& curr_points,
      const std::vector<LineFeature>& prev_lines,
      const std::vector<LineFeature>& curr_lines,
      const std::vector<std::pair<int, int>>& point_matches,
      const std::vector<std::pair<int, int>>& line_matches,
      Eigen::Matrix4d* T_prev_curr, double* scale_correction,
      size_t* num_depth_priors, double* mean_depth_prior_residual,
      const Eigen::Matrix4d& init_T = Eigen::Matrix4d::Identity());

 private:
  std::vector<std::pair<int, int>> matchPoints(
      const std::vector<PointFeature>& prev,
      const std::vector<PointFeature>& curr) const;
  std::vector<std::pair<int, int>> matchLines(
      const std::vector<LineFeature>& prev,
      const std::vector<LineFeature>& curr) const;

  PlLoamParams params_;
  std::vector<PointFeature> prev_points_;
  std::vector<LineFeature> prev_lines_;
  Eigen::Quaterniond q_w_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr_ = Eigen::Vector3d::Zero();
  Eigen::Quaterniond q_last_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_last_curr_ = Eigen::Vector3d::Zero();
  bool initialized_ = false;
  int frame_count_ = 0;
};

}  // namespace pl_loam
}  // namespace localization_zoo
