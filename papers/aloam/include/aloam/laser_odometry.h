#pragma once

#include "aloam/types.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/kdtree/kdtree_flann.h>

namespace localization_zoo {
namespace aloam {

struct LaserOdometryParams {
  double distance_sq_threshold = 25.0;  // 対応点の最大距離² [m²]
  double nearby_scan = 2.5;             // 異なるスキャンラインの最大差
  int num_optimization_iters = 2;       // 外側最適化ループ回数
  int ceres_max_iterations = 4;         // Ceres内部反復回数
  double huber_loss_s = 0.1;            // Huber loss パラメータ
  bool enable_distortion = false;       // モーション補償
};

struct OdometryResult {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  bool valid = false;
};

/// Scan-to-Scan オドメトリ
class LaserOdometry {
public:
  explicit LaserOdometry(const LaserOdometryParams& params =
                             LaserOdometryParams())
      : params_(params) {}

  /// 新しい特徴点群を処理してオドメトリを更新
  OdometryResult process(const FeatureCloud& features);

  const Eigen::Quaterniond& orientation() const { return q_w_curr_; }
  const Eigen::Vector3d& position() const { return t_w_curr_; }

private:
  void transformToStart(const PointT& pi, PointT& po) const;

  LaserOdometryParams params_;

  // 前フレームの特徴点群
  PointCloudPtr corner_last_;
  PointCloudPtr surf_last_;
  pcl::KdTreeFLANN<PointT> kd_corner_last_;
  pcl::KdTreeFLANN<PointT> kd_surf_last_;

  // 累積ポーズ
  Eigen::Quaterniond q_w_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr_ = Eigen::Vector3d::Zero();

  // frame-to-frame 変換
  Eigen::Quaterniond q_last_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_last_curr_ = Eigen::Vector3d::Zero();

  bool initialized_ = false;
  int frame_count_ = 0;
};

}  // namespace aloam
}  // namespace localization_zoo
