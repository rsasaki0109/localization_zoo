#pragma once

#include "relead/degeneracy_detection.h"
#include "relead/types.h"

#include <Eigen/Core>
#include <vector>

namespace localization_zoo {
namespace relead {

struct CESIKFParams {
  /// ESIKF の最大反復回数
  int max_iterations = 5;
  /// 収束判定閾値
  double convergence_threshold = 1e-4;
  /// 退化検知パラメータ
  DegeneracyDetectionParams degeneracy_params;
  /// LiDAR計測ノイズ (point-to-plane残差の標準偏差)
  double lidar_noise_std = 0.01;
  /// IMUノイズ
  double gyro_noise_std = 0.01;
  double accel_noise_std = 0.1;
  double gyro_bias_noise_std = 0.001;
  double accel_bias_noise_std = 0.01;
};

/// 制約付きESIKF更新の結果
struct CESIKFResult {
  State state;
  Eigen::Matrix<double, 18, 18> covariance;
  DegeneracyInfo degeneracy_info;
  int iterations = 0;
  bool converged = false;
};

/// Constrained Error-State Iterated Kalman Filter
///
/// RELEADのフロントエンド: IMU伝播 + 退化検知 + 制約付きLiDAR更新
class CESIKF {
public:
  explicit CESIKF(const CESIKFParams& params = CESIKFParams());

  /// IMU計測値で状態を伝播
  void predict(const ImuMeasurement& imu, double dt);

  /// LiDAR scan-matching残差で状態を更新 (制約付き)
  /// @param correspondences Point-to-plane対応関係
  /// @return 更新結果 (退化情報含む)
  CESIKFResult update(const std::vector<PointWithNormal>& correspondences);

  /// 状態を取得
  const State& state() const { return state_; }
  State& state() { return state_; }

  /// 共分散を取得
  const Eigen::Matrix<double, 18, 18>& covariance() const { return P_; }

  /// リセット
  void reset();

private:
  /// 制約付き更新: 退化方向への更新を射影で除去
  Eigen::Matrix<double, 18, 1> constrainedUpdate(
      const Eigen::Matrix<double, 18, 1>& delta_x,
      const DegeneracyInfo& deg_info) const;

  CESIKFParams params_;
  State state_;
  Eigen::Matrix<double, 18, 18> P_;  // エラーステート共分散
  DegeneracyDetection degeneracy_detector_;
};

}  // namespace relead
}  // namespace localization_zoo
