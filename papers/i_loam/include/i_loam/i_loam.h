#pragma once

#include "aloam/scan_registration.h"
#include "aloam/types.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/kdtree/kdtree_flann.h>

#include <cstddef>
#include <vector>

namespace localization_zoo {
namespace i_loam {

// I-LOAM: Intensity Enhanced LiDAR Odometry and Mapping
// (Yeong-Sang Park, Hyesu Jang, Ayoung Kim, UR 2020)
//
// LOAM の scan-to-scan オドメトリに反射強度を 2 通りで注入する:
//   1) 強度拡張対応探索: 対応コスト = 幾何距離^2 + λ * Δ強度^2
//   2) 強度重み付き残差: w = exp(-Δ強度^2 / (2σ^2)) を各残差に乗じる
// 幾何特徴抽出は aloam::ScanRegistration を再利用し、抽出後に失われる
// 反射強度は生入力点群への最近傍参照で特徴点ごとに復元する。

struct ILoamParams {
  ILoamParams() = default;

  aloam::ScanRegistrationParams scan_registration;

  // --- オドメトリ (aloam::LaserOdometry 準拠) ---
  double distance_sq_threshold = 25.0;  // 対応点の最大距離^2 [m^2]
  double nearby_scan = 2.5;             // 異なるスキャンラインの最大差
  int num_optimization_iters = 2;       // 外側最適化ループ回数
  int ceres_max_iterations = 4;         // Ceres 内部反復回数
  double huber_loss_s = 0.1;            // Huber loss パラメータ
  bool enable_distortion = false;       // モーション補償
  size_t input_point_stride = 1;        // 入力点の間引き

  // --- I-LOAM 強度拡張 ---
  bool use_intensity_weight = true;          // 強度重み付き残差を使う
  bool use_intensity_correspondence = true;  // 強度拡張対応探索を使う
  double intensity_sigma = 0.15;             // 残差重みの σ (反射強度スケール)
  double intensity_corr_weight = 1.0;        // 対応探索の Δ強度^2 重み [m^2]
};

struct ILoamResult {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  bool valid = false;
  bool odom_valid = false;
  int frame_count = 0;
  size_t num_input_points = 0;
  size_t num_corner_sharp = 0;
  size_t num_surf_flat = 0;
  size_t num_edge_correspondences = 0;
  size_t num_plane_correspondences = 0;
  double mean_intensity_weight = 1.0;  // 当該フレームの平均残差重み(診断用)
};

class ILoam {
 public:
  explicit ILoam(const ILoamParams& params = ILoamParams());

  ILoamResult process(const aloam::PointCloudConstPtr& cloud);
  void clear();

  int frameCount() const { return frame_count_; }
  const Eigen::Quaterniond& orientation() const { return q_w_curr_; }
  const Eigen::Vector3d& position() const { return t_w_curr_; }

  // --- 単体テスト用の純関数ヘルパ ---
  /// 反射強度差から残差重み w = exp(-Δ^2 / (2σ^2)) を返す (w ∈ (0, 1])。
  static double intensityWeight(double delta_intensity, double sigma);
  /// 強度拡張対応コスト: 幾何距離^2 + λ * Δ強度^2。
  static double augmentedDistanceSq(double geom_dist_sq, double delta_intensity,
                                    double intensity_corr_weight);

 private:
  aloam::PointCloudPtr strideCloud(
      const aloam::PointCloudConstPtr& cloud) const;
  /// features の各点について working_cloud 内の最近傍点の反射強度を引く。
  std::vector<float> lookupReflectance(
      const aloam::PointCloudConstPtr& features,
      const pcl::KdTreeFLANN<aloam::PointT>& kd_raw,
      const aloam::PointCloudConstPtr& raw) const;
  void transformToStart(const aloam::PointT& pi, const Eigen::Quaterniond& q,
                        const Eigen::Vector3d& t, aloam::PointT& po) const;

  ILoamParams params_;
  aloam::ScanRegistration scan_registration_;

  // 前フレームの less-feature 点群 + その反射強度
  aloam::PointCloudPtr corner_last_;
  aloam::PointCloudPtr surf_last_;
  std::vector<float> corner_last_refl_;
  std::vector<float> surf_last_refl_;
  pcl::KdTreeFLANN<aloam::PointT> kd_corner_last_;
  pcl::KdTreeFLANN<aloam::PointT> kd_surf_last_;

  Eigen::Quaterniond q_w_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr_ = Eigen::Vector3d::Zero();
  Eigen::Quaterniond q_last_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_last_curr_ = Eigen::Vector3d::Zero();

  bool initialized_ = false;
  int frame_count_ = 0;
};

}  // namespace i_loam
}  // namespace localization_zoo
