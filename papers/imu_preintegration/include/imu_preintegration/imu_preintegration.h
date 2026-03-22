#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>

namespace localization_zoo {
namespace imu_preintegration {

struct ImuSample {
  double timestamp;
  Eigen::Vector3d gyro;   // 角速度 [rad/s]
  Eigen::Vector3d accel;  // 加速度 [m/s²]
};

struct ImuNoiseParams {
  double gyro_noise = 1.6968e-4;        // gyro noise density
  double accel_noise = 2.0000e-3;       // accel noise density
  double gyro_bias_noise = 1.9393e-5;   // gyro bias random walk
  double accel_bias_noise = 3.0000e-3;  // accel bias random walk
};

/// IMU事前積分の結果
struct PreintegrationResult {
  Eigen::Matrix3d delta_R = Eigen::Matrix3d::Identity();
  Eigen::Vector3d delta_v = Eigen::Vector3d::Zero();
  Eigen::Vector3d delta_p = Eigen::Vector3d::Zero();
  double delta_t = 0.0;

  /// 共分散 (9x9: δθ, δv, δp)
  Eigen::Matrix<double, 9, 9> covariance =
      Eigen::Matrix<double, 9, 9>::Zero();

  /// バイアス補正ヤコビアン (1次近似用)
  Eigen::Matrix3d J_R_bg = Eigen::Matrix3d::Zero();
  Eigen::Matrix3d J_v_bg = Eigen::Matrix3d::Zero();
  Eigen::Matrix3d J_v_ba = Eigen::Matrix3d::Zero();
  Eigen::Matrix3d J_p_bg = Eigen::Matrix3d::Zero();
  Eigen::Matrix3d J_p_ba = Eigen::Matrix3d::Zero();

  /// バイアス更新による補正 (再積分不要)
  PreintegrationResult correct(const Eigen::Vector3d& delta_bg,
                                const Eigen::Vector3d& delta_ba) const;
};

/// IMU Preintegration on Manifold
class ImuPreintegrator {
public:
  explicit ImuPreintegrator(const ImuNoiseParams& params = ImuNoiseParams(),
                            const Eigen::Vector3d& bias_gyro = Eigen::Vector3d::Zero(),
                            const Eigen::Vector3d& bias_accel = Eigen::Vector3d::Zero());

  /// IMUサンプルを1つ積分
  void integrate(const Eigen::Vector3d& gyro, const Eigen::Vector3d& accel,
                 double dt);

  /// 複数サンプルを一括積分
  void integrateBatch(const std::vector<ImuSample>& samples);

  /// 事前積分結果を取得
  const PreintegrationResult& result() const { return result_; }

  /// リセット
  void reset();

  /// バイアスを更新
  void setBias(const Eigen::Vector3d& bg, const Eigen::Vector3d& ba);

  /// 事前積分から絶対ポーズを計算
  /// @param R_i, v_i, p_i: キーフレームiの状態
  /// @param gravity: 重力ベクトル (世界座標系)
  void predict(const Eigen::Matrix3d& R_i, const Eigen::Vector3d& v_i,
               const Eigen::Vector3d& p_i, const Eigen::Vector3d& gravity,
               Eigen::Matrix3d& R_j, Eigen::Vector3d& v_j,
               Eigen::Vector3d& p_j) const;

private:
  ImuNoiseParams noise_params_;
  Eigen::Vector3d bias_gyro_;
  Eigen::Vector3d bias_accel_;
  PreintegrationResult result_;
};

}  // namespace imu_preintegration
}  // namespace localization_zoo
