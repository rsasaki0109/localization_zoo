#include "imu_preintegration/imu_preintegration.h"

#include <cmath>

namespace localization_zoo {
namespace imu_preintegration {

namespace {

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0, -v.z(), v.y(), v.z(), 0, -v.x(), -v.y(), v.x(), 0;
  return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& omega) {
  double theta = omega.norm();
  if (theta < 1e-10) return Eigen::Matrix3d::Identity() + skew(omega);
  Eigen::Matrix3d K = skew(omega / theta);
  return Eigen::Matrix3d::Identity() + std::sin(theta) * K +
         (1.0 - std::cos(theta)) * K * K;
}

/// 右ヤコビアン Jr(ω)
Eigen::Matrix3d rightJacobian(const Eigen::Vector3d& omega) {
  double theta = omega.norm();
  if (theta < 1e-10) return Eigen::Matrix3d::Identity();
  Eigen::Matrix3d K = skew(omega / theta);
  return Eigen::Matrix3d::Identity() -
         (1.0 - std::cos(theta)) / (theta * theta) * K +
         (theta - std::sin(theta)) / (theta * theta * theta) * K * K;
}

}  // namespace

PreintegrationResult PreintegrationResult::correct(
    const Eigen::Vector3d& delta_bg,
    const Eigen::Vector3d& delta_ba) const {
  PreintegrationResult corrected = *this;
  corrected.delta_R = delta_R * expSO3(J_R_bg * delta_bg);
  corrected.delta_v = delta_v + J_v_bg * delta_bg + J_v_ba * delta_ba;
  corrected.delta_p = delta_p + J_p_bg * delta_bg + J_p_ba * delta_ba;
  return corrected;
}

ImuPreintegrator::ImuPreintegrator(const ImuNoiseParams& params,
                                    const Eigen::Vector3d& bias_gyro,
                                    const Eigen::Vector3d& bias_accel)
    : noise_params_(params), bias_gyro_(bias_gyro), bias_accel_(bias_accel) {}

void ImuPreintegrator::reset() {
  result_ = PreintegrationResult();
}

void ImuPreintegrator::setBias(const Eigen::Vector3d& bg,
                                const Eigen::Vector3d& ba) {
  bias_gyro_ = bg;
  bias_accel_ = ba;
}

void ImuPreintegrator::integrate(const Eigen::Vector3d& gyro,
                                  const Eigen::Vector3d& accel, double dt) {
  // バイアス除去
  Eigen::Vector3d omega = gyro - bias_gyro_;
  Eigen::Vector3d acc = accel - bias_accel_;

  // 増分回転
  Eigen::Vector3d omega_dt = omega * dt;
  Eigen::Matrix3d dR = expSO3(omega_dt);
  Eigen::Matrix3d Jr = rightJacobian(omega_dt);

  // 事前積分量を更新
  // Δp += Δv dt + 0.5 ΔR acc dt²
  result_.delta_p += result_.delta_v * dt +
                     0.5 * result_.delta_R * acc * dt * dt;
  // Δv += ΔR acc dt
  result_.delta_v += result_.delta_R * acc * dt;
  // ΔR = ΔR · Exp(ω dt)
  Eigen::Matrix3d R_prev = result_.delta_R;
  result_.delta_R = result_.delta_R * dR;

  result_.delta_t += dt;

  // ヤコビアン更新 (バイアス補正用)
  // J_R_bg
  result_.J_R_bg = dR.transpose() * result_.J_R_bg -
                   Jr * dt;

  // J_v_ba
  result_.J_v_ba += -R_prev * dt;
  // J_v_bg
  result_.J_v_bg += -R_prev * skew(acc) * result_.J_R_bg * dt;

  // J_p_ba
  result_.J_p_ba += result_.J_v_ba * dt - 0.5 * R_prev * dt * dt;
  // J_p_bg
  result_.J_p_bg += result_.J_v_bg * dt -
                    0.5 * R_prev * skew(acc) * result_.J_R_bg * dt * dt;

  // 共分散伝播
  Eigen::Matrix<double, 9, 9> F = Eigen::Matrix<double, 9, 9>::Identity();
  // δθ
  F.block<3, 3>(0, 0) = dR.transpose();
  // δv ← δθ
  F.block<3, 3>(3, 0) = -R_prev * skew(acc) * dt;
  // δp ← δv
  F.block<3, 3>(6, 3) = Eigen::Matrix3d::Identity() * dt;
  // δp ← δθ
  F.block<3, 3>(6, 0) = -0.5 * R_prev * skew(acc) * dt * dt;

  // ノイズ
  Eigen::Matrix<double, 9, 6> G = Eigen::Matrix<double, 9, 6>::Zero();
  G.block<3, 3>(0, 0) = -Jr * dt;
  G.block<3, 3>(3, 3) = -R_prev * dt;
  G.block<3, 3>(6, 3) = -0.5 * R_prev * dt * dt;

  Eigen::Matrix<double, 6, 6> Q = Eigen::Matrix<double, 6, 6>::Zero();
  double ng2 = noise_params_.gyro_noise * noise_params_.gyro_noise;
  double na2 = noise_params_.accel_noise * noise_params_.accel_noise;
  Q.block<3, 3>(0, 0) = ng2 * Eigen::Matrix3d::Identity();
  Q.block<3, 3>(3, 3) = na2 * Eigen::Matrix3d::Identity();

  result_.covariance = F * result_.covariance * F.transpose() +
                       G * Q * G.transpose();
}

void ImuPreintegrator::integrateBatch(const std::vector<ImuSample>& samples) {
  for (size_t i = 1; i < samples.size(); i++) {
    double dt = samples[i].timestamp - samples[i - 1].timestamp;
    if (dt > 0 && dt < 0.1) {
      // 中点法
      Eigen::Vector3d gyro = 0.5 * (samples[i - 1].gyro + samples[i].gyro);
      Eigen::Vector3d accel = 0.5 * (samples[i - 1].accel + samples[i].accel);
      integrate(gyro, accel, dt);
    }
  }
}

void ImuPreintegrator::predict(const Eigen::Matrix3d& R_i,
                                const Eigen::Vector3d& v_i,
                                const Eigen::Vector3d& p_i,
                                const Eigen::Vector3d& gravity,
                                Eigen::Matrix3d& R_j, Eigen::Vector3d& v_j,
                                Eigen::Vector3d& p_j) const {
  double dt = result_.delta_t;
  R_j = R_i * result_.delta_R;
  v_j = v_i + gravity * dt + R_i * result_.delta_v;
  p_j = p_i + v_i * dt + 0.5 * gravity * dt * dt + R_i * result_.delta_p;
}

}  // namespace imu_preintegration
}  // namespace localization_zoo
