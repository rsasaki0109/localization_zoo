#include "relead/cesikf.h"

#include <Eigen/Dense>
#include <cmath>
#include <iostream>

namespace localization_zoo {
namespace relead {

namespace {

/// so(3) の歪対称行列
Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0, -v.z(), v.y(),
       v.z(), 0, -v.x(),
       -v.y(), v.x(), 0;
  return m;
}

/// so(3) → SO(3) 指数写像
Eigen::Matrix3d expSO3(const Eigen::Vector3d& omega) {
  double theta = omega.norm();
  if (theta < 1e-10) {
    return Eigen::Matrix3d::Identity() + skew(omega);
  }
  Eigen::Matrix3d K = skew(omega / theta);
  return Eigen::Matrix3d::Identity() + std::sin(theta) * K +
         (1.0 - std::cos(theta)) * K * K;
}

}  // namespace

CESIKF::CESIKF(const CESIKFParams& params)
    : params_(params), degeneracy_detector_(params.degeneracy_params) {
  P_.setIdentity();
  P_ *= 0.001;
}

void CESIKF::reset() {
  state_ = State();
  P_.setIdentity();
  P_ *= 0.001;
}

void CESIKF::predict(const ImuMeasurement& imu, double dt) {
  // IMU計測値からバイアスを除去
  Eigen::Vector3d omega = imu.angular_velocity - state_.bias_gyro;
  Eigen::Vector3d accel = imu.linear_acceleration - state_.bias_accel;

  // 状態伝播 (離散時間モデル)
  Eigen::Matrix3d R = state_.rotation;
  Eigen::Vector3d new_velocity =
      state_.velocity + (R * accel + state_.gravity) * dt;
  Eigen::Vector3d new_position =
      state_.position + state_.velocity * dt +
      0.5 * (R * accel + state_.gravity) * dt * dt;
  Eigen::Matrix3d new_rotation = R * expSO3(omega * dt);

  state_.rotation = new_rotation;
  state_.position = new_position;
  state_.velocity = new_velocity;
  // バイアスと重力はランダムウォークで不変

  // エラーステート遷移行列 F (18x18)
  Eigen::Matrix<double, 18, 18> F = Eigen::Matrix<double, 18, 18>::Identity();

  // δr → δr: I - [ω]× dt
  F.block<3, 3>(0, 0) = expSO3(-omega * dt);
  // δr → δb_g: -dt
  F.block<3, 3>(0, 9) = -Eigen::Matrix3d::Identity() * dt;
  // δt → δv: I * dt
  F.block<3, 3>(3, 6) = Eigen::Matrix3d::Identity() * dt;
  // δv → δr: -R [a]× dt
  F.block<3, 3>(6, 0) = -R * skew(accel) * dt;
  // δv → δb_a: -R * dt
  F.block<3, 3>(6, 12) = -R * dt;
  // δv → δg: I * dt
  F.block<3, 3>(6, 15) = Eigen::Matrix3d::Identity() * dt;

  // プロセスノイズ Q
  Eigen::Matrix<double, 18, 18> Q =
      Eigen::Matrix<double, 18, 18>::Zero();
  double gn = params_.gyro_noise_std;
  double an = params_.accel_noise_std;
  double gbn = params_.gyro_bias_noise_std;
  double abn = params_.accel_bias_noise_std;

  Q.block<3, 3>(0, 0) = gn * gn * dt * dt * Eigen::Matrix3d::Identity();
  Q.block<3, 3>(6, 6) = an * an * dt * dt * Eigen::Matrix3d::Identity();
  Q.block<3, 3>(9, 9) = gbn * gbn * dt * Eigen::Matrix3d::Identity();
  Q.block<3, 3>(12, 12) = abn * abn * dt * Eigen::Matrix3d::Identity();

  // 共分散伝播
  P_ = F * P_ * F.transpose() + Q;
}

Eigen::Matrix<double, 18, 1> CESIKF::constrainedUpdate(
    const Eigen::Matrix<double, 18, 1>& delta_x,
    const DegeneracyInfo& deg_info) const {
  if (!deg_info.hasDegeneracy()) {
    return delta_x;
  }

  // 制約行列 C と制約値 d を構築
  // C Δx = d (退化方向への更新をゼロに制約)
  int m_r = deg_info.degenerate_rotation_dirs.size();
  int m_t = deg_info.degenerate_translation_dirs.size();
  int m = m_r + m_t;

  Eigen::MatrixXd C = Eigen::MatrixXd::Zero(m, 18);
  Eigen::VectorXd d = Eigen::VectorXd::Zero(m);

  int row = 0;
  // 回転の退化方向: v_j^T δr = 0
  for (const auto& v : deg_info.degenerate_rotation_dirs) {
    C.block<1, 3>(row, 0) = v.transpose();
    d(row) = 0.0;  // 退化方向への更新はゼロ
    row++;
  }
  // 並進の退化方向: v_j^T δt = 0
  for (const auto& v : deg_info.degenerate_translation_dirs) {
    C.block<1, 3>(row, 3) = v.transpose();
    d(row) = 0.0;
    row++;
  }

  // 制約付き更新 (式8):
  // G = Σ C^T (C Σ C^T)^{-1}
  // Δx̄ = Δx - G(C Δx - d)
  Eigen::MatrixXd CPCT = C * P_ * C.transpose();
  Eigen::MatrixXd G = P_ * C.transpose() * CPCT.inverse();

  Eigen::Matrix<double, 18, 1> delta_x_constrained =
      delta_x - G * (C * delta_x - d);

  return delta_x_constrained;
}

CESIKFResult CESIKF::update(
    const std::vector<PointWithNormal>& correspondences) {
  CESIKFResult result;

  if (correspondences.empty()) {
    result.state = state_;
    result.covariance = P_;
    return result;
  }

  // 退化検知
  result.degeneracy_info = degeneracy_detector_.detect(correspondences);

  int n = correspondences.size();
  double R_lidar = params_.lidar_noise_std * params_.lidar_noise_std;

  // ESIKF反復
  State state_backup = state_;
  Eigen::Matrix<double, 18, 18> P_backup = P_;

  for (int iter = 0; iter < params_.max_iterations; iter++) {
    // 計測ヤコビアン H (Nx18) と残差 z (Nx1)
    Eigen::MatrixXd H = Eigen::MatrixXd::Zero(n, 18);
    Eigen::VectorXd z = Eigen::VectorXd::Zero(n);

    Eigen::Matrix3d R = state_.rotation;

    for (int i = 0; i < n; i++) {
      const auto& corr = correspondences[i];
      Eigen::Vector3d p_world = corr.point;
      Eigen::Vector3d u = corr.normal;
      Eigen::Vector3d q = corr.closest;

      // Point-to-plane残差: u^T (p_world - q)
      z(i) = -u.dot(p_world - q);

      // ヤコビアン (エラーステート)
      // ∂r/∂δr = u^T [Rp]×  (回転)
      // ∂r/∂δt = u^T          (並進)
      // 他の状態には直接依存しない
      Eigen::Vector3d Rp = p_world - state_.position;  // R * p_body
      H.block<1, 3>(i, 0) = u.transpose() * skew(Rp);
      H.block<1, 3>(i, 3) = u.transpose();
    }

    // 計測ノイズ行列
    Eigen::MatrixXd R_mat = R_lidar * Eigen::MatrixXd::Identity(n, n);

    // カルマンゲイン
    Eigen::MatrixXd S = H * P_ * H.transpose() + R_mat;
    Eigen::MatrixXd K = P_ * H.transpose() * S.inverse();

    // 状態更新
    Eigen::Matrix<double, 18, 1> delta_x = K * z;

    // 制約付き更新: 退化方向への更新を除去
    delta_x = constrainedUpdate(delta_x, result.degeneracy_info);

    // 状態に適用
    state_.rotation = state_.rotation * expSO3(delta_x.segment<3>(0));
    state_.position += delta_x.segment<3>(3);
    state_.velocity += delta_x.segment<3>(6);
    state_.bias_gyro += delta_x.segment<3>(9);
    state_.bias_accel += delta_x.segment<3>(12);
    state_.gravity += delta_x.segment<3>(15);

    // 共分散更新
    Eigen::Matrix<double, 18, 18> I_KH =
        Eigen::Matrix<double, 18, 18>::Identity() - K * H;
    P_ = I_KH * P_;

    result.iterations = iter + 1;

    // 収束判定
    if (delta_x.norm() < params_.convergence_threshold) {
      result.converged = true;
      break;
    }
  }

  result.state = state_;
  result.covariance = P_;
  return result;
}

}  // namespace relead
}  // namespace localization_zoo
