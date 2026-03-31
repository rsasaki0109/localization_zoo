#pragma once

#include "imu_preintegration/imu_preintegration.h"

#include <ceres/ceres.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace localization_zoo {
namespace ct_lio {

namespace detail {

template <typename T>
Eigen::Quaternion<T> expQuaternion(const Eigen::Matrix<T, 3, 1>& omega) {
  const T theta = omega.norm();
  if (theta < T(1e-8)) {
    Eigen::Quaternion<T> q(T(1.0), T(0.5) * omega.x(), T(0.5) * omega.y(),
                           T(0.5) * omega.z());
    q.normalize();
    return q;
  }

  const T half_theta = T(0.5) * theta;
  const T imag_scale = ceres::sin(half_theta) / theta;
  return Eigen::Quaternion<T>(ceres::cos(half_theta), imag_scale * omega.x(),
                              imag_scale * omega.y(),
                              imag_scale * omega.z());
}

}  // namespace detail

struct IMURotationBiasPrior {
  IMURotationBiasPrior(const imu_preintegration::PreintegrationResult& preint,
                       const Eigen::Vector3d& reference_gyro_bias,
                       double weight)
      : delta_q_(preint.delta_R),
        J_R_bg_(preint.J_R_bg),
        reference_gyro_bias_(reference_gyro_bias),
        weight_(weight) {}

  template <typename T>
  bool operator()(const T* begin_quat, const T* end_quat,
                  const T* gyro_bias, T* residual) const {
    Eigen::Quaternion<T> q_begin{begin_quat[3], begin_quat[0], begin_quat[1],
                                 begin_quat[2]};
    Eigen::Quaternion<T> q_end{end_quat[3], end_quat[0], end_quat[1],
                               end_quat[2]};
    Eigen::Matrix<T, 3, 1> delta_bg{gyro_bias[0] - T(reference_gyro_bias_.x()),
                                    gyro_bias[1] - T(reference_gyro_bias_.y()),
                                    gyro_bias[2] - T(reference_gyro_bias_.z())};

    Eigen::Matrix<T, 3, 1> corrected_delta =
        J_R_bg_.cast<T>() * delta_bg;
    Eigen::Quaternion<T> delta_q{
        T(delta_q_.w()), T(delta_q_.x()), T(delta_q_.y()), T(delta_q_.z())};
    delta_q = delta_q * detail::expQuaternion(corrected_delta);

    Eigen::Quaternion<T> q_expected_end = q_begin * delta_q;
    Eigen::Quaternion<T> q_error = q_expected_end.conjugate() * q_end;

    residual[0] = T(weight_) * T(2.0) * q_error.x();
    residual[1] = T(weight_) * T(2.0) * q_error.y();
    residual[2] = T(weight_) * T(2.0) * q_error.z();
    return true;
  }

  static ceres::CostFunction* Create(
      const imu_preintegration::PreintegrationResult& preint,
      const Eigen::Vector3d& reference_gyro_bias,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<IMURotationBiasPrior, 3, 4, 4, 3>(
        new IMURotationBiasPrior(preint, reference_gyro_bias, weight));
  }

  Eigen::Quaterniond delta_q_;
  Eigen::Matrix3d J_R_bg_;
  Eigen::Vector3d reference_gyro_bias_;
  double weight_;
};

struct IMUTranslationBiasPrior {
  IMUTranslationBiasPrior(
      const imu_preintegration::PreintegrationResult& preint,
      const Eigen::Vector3d& reference_gyro_bias,
      const Eigen::Vector3d& reference_accel_bias, double delta_t,
      const Eigen::Vector3d& gravity, double weight)
      : delta_p_(preint.delta_p),
        J_p_bg_(preint.J_p_bg),
        J_p_ba_(preint.J_p_ba),
        reference_gyro_bias_(reference_gyro_bias),
        reference_accel_bias_(reference_accel_bias),
        delta_t_(delta_t),
        gravity_(gravity),
        weight_(weight) {}

  template <typename T>
  bool operator()(const T* begin_quat, const T* begin_trans,
                  const T* end_trans, const T* begin_velocity,
                  const T* gyro_bias, const T* accel_bias,
                  T* residual) const {
    Eigen::Quaternion<T> q_begin{begin_quat[3], begin_quat[0], begin_quat[1],
                                 begin_quat[2]};
    Eigen::Matrix<T, 3, 1> t_begin{begin_trans[0], begin_trans[1],
                                   begin_trans[2]};
    Eigen::Matrix<T, 3, 1> t_end{end_trans[0], end_trans[1], end_trans[2]};
    Eigen::Matrix<T, 3, 1> v_begin{begin_velocity[0], begin_velocity[1],
                                   begin_velocity[2]};
    Eigen::Matrix<T, 3, 1> delta_bg{gyro_bias[0] - T(reference_gyro_bias_.x()),
                                    gyro_bias[1] - T(reference_gyro_bias_.y()),
                                    gyro_bias[2] - T(reference_gyro_bias_.z())};
    Eigen::Matrix<T, 3, 1> delta_ba{accel_bias[0] - T(reference_accel_bias_.x()),
                                    accel_bias[1] - T(reference_accel_bias_.y()),
                                    accel_bias[2] - T(reference_accel_bias_.z())};
    Eigen::Matrix<T, 3, 1> delta_p =
        delta_p_.cast<T>() + J_p_bg_.cast<T>() * delta_bg +
        J_p_ba_.cast<T>() * delta_ba;
    Eigen::Matrix<T, 3, 1> gravity{T(gravity_.x()), T(gravity_.y()),
                                   T(gravity_.z())};

    T dt = T(delta_t_);
    Eigen::Matrix<T, 3, 1> expected_end =
        t_begin + v_begin * dt + T(0.5) * gravity * dt * dt + q_begin * delta_p;
    Eigen::Matrix<T, 3, 1> error = t_end - expected_end;

    residual[0] = T(weight_) * error.x();
    residual[1] = T(weight_) * error.y();
    residual[2] = T(weight_) * error.z();
    return true;
  }

  static ceres::CostFunction* Create(
                                     const imu_preintegration::PreintegrationResult& preint,
                                     const Eigen::Vector3d& reference_gyro_bias,
                                     const Eigen::Vector3d& reference_accel_bias,
                                     double delta_t,
                                     const Eigen::Vector3d& gravity,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<
        IMUTranslationBiasPrior, 3, 4, 3, 3, 3, 3, 3>(
        new IMUTranslationBiasPrior(preint, reference_gyro_bias,
                                    reference_accel_bias, delta_t, gravity,
                                    weight));
  }

  Eigen::Vector3d delta_p_;
  Eigen::Matrix3d J_p_bg_;
  Eigen::Matrix3d J_p_ba_;
  Eigen::Vector3d reference_gyro_bias_;
  Eigen::Vector3d reference_accel_bias_;
  double delta_t_;
  Eigen::Vector3d gravity_;
  double weight_;
};

struct IMUVelocityBiasPrior {
  IMUVelocityBiasPrior(
      const imu_preintegration::PreintegrationResult& preint,
      const Eigen::Vector3d& reference_gyro_bias,
      const Eigen::Vector3d& reference_accel_bias, double delta_t,
      const Eigen::Vector3d& gravity, double weight)
      : delta_v_(preint.delta_v),
        J_v_bg_(preint.J_v_bg),
        J_v_ba_(preint.J_v_ba),
        reference_gyro_bias_(reference_gyro_bias),
        reference_accel_bias_(reference_accel_bias),
        delta_t_(delta_t),
        gravity_(gravity),
        weight_(weight) {}

  template <typename T>
  bool operator()(const T* begin_quat, const T* begin_velocity,
                  const T* next_begin_velocity, const T* gyro_bias,
                  const T* accel_bias, T* residual) const {
    Eigen::Quaternion<T> q_begin{begin_quat[3], begin_quat[0], begin_quat[1],
                                 begin_quat[2]};
    Eigen::Matrix<T, 3, 1> v_begin{begin_velocity[0], begin_velocity[1],
                                   begin_velocity[2]};
    Eigen::Matrix<T, 3, 1> v_next{next_begin_velocity[0], next_begin_velocity[1],
                                  next_begin_velocity[2]};
    Eigen::Matrix<T, 3, 1> delta_bg{gyro_bias[0] - T(reference_gyro_bias_.x()),
                                    gyro_bias[1] - T(reference_gyro_bias_.y()),
                                    gyro_bias[2] - T(reference_gyro_bias_.z())};
    Eigen::Matrix<T, 3, 1> delta_ba{accel_bias[0] - T(reference_accel_bias_.x()),
                                    accel_bias[1] - T(reference_accel_bias_.y()),
                                    accel_bias[2] - T(reference_accel_bias_.z())};
    Eigen::Matrix<T, 3, 1> corrected_delta_v =
        delta_v_.cast<T>() + J_v_bg_.cast<T>() * delta_bg +
        J_v_ba_.cast<T>() * delta_ba;
    Eigen::Matrix<T, 3, 1> gravity{T(gravity_.x()), T(gravity_.y()),
                                   T(gravity_.z())};
    const T dt = T(delta_t_);
    Eigen::Matrix<T, 3, 1> expected_next =
        v_begin + gravity * dt + q_begin * corrected_delta_v;
    Eigen::Matrix<T, 3, 1> error = v_next - expected_next;

    residual[0] = T(weight_) * error.x();
    residual[1] = T(weight_) * error.y();
    residual[2] = T(weight_) * error.z();
    return true;
  }

  static ceres::CostFunction* Create(
      const imu_preintegration::PreintegrationResult& preint,
      const Eigen::Vector3d& reference_gyro_bias,
      const Eigen::Vector3d& reference_accel_bias, double delta_t,
      const Eigen::Vector3d& gravity, double weight) {
    return new ceres::AutoDiffCostFunction<
        IMUVelocityBiasPrior, 3, 4, 3, 3, 3, 3>(
        new IMUVelocityBiasPrior(preint, reference_gyro_bias,
                                 reference_accel_bias, delta_t, gravity, weight));
  }

  Eigen::Vector3d delta_v_;
  Eigen::Matrix3d J_v_bg_;
  Eigen::Matrix3d J_v_ba_;
  Eigen::Vector3d reference_gyro_bias_;
  Eigen::Vector3d reference_accel_bias_;
  double delta_t_;
  Eigen::Vector3d gravity_;
  double weight_;
};

struct VelocityPrior {
  VelocityPrior(const Eigen::Vector3d& target_velocity, double weight)
      : target_velocity_(target_velocity), weight_(weight) {}

  template <typename T>
  bool operator()(const T* velocity, T* residual) const {
    residual[0] = T(weight_) * (velocity[0] - T(target_velocity_.x()));
    residual[1] = T(weight_) * (velocity[1] - T(target_velocity_.y()));
    residual[2] = T(weight_) * (velocity[2] - T(target_velocity_.z()));
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& target_velocity,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<VelocityPrior, 3, 3>(
        new VelocityPrior(target_velocity, weight));
  }

  Eigen::Vector3d target_velocity_;
  double weight_;
};

struct BiasPrior {
  BiasPrior(const Eigen::Vector3d& target_bias, double weight)
      : target_bias_(target_bias), weight_(weight) {}

  template <typename T>
  bool operator()(const T* bias, T* residual) const {
    residual[0] = T(weight_) * (bias[0] - T(target_bias_.x()));
    residual[1] = T(weight_) * (bias[1] - T(target_bias_.y()));
    residual[2] = T(weight_) * (bias[2] - T(target_bias_.z()));
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& target_bias,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<BiasPrior, 3, 3>(
        new BiasPrior(target_bias, weight));
  }

  Eigen::Vector3d target_bias_;
  double weight_;
};

struct BiasDifferencePrior {
  explicit BiasDifferencePrior(double weight) : weight_(weight) {}

  template <typename T>
  bool operator()(const T* lhs_bias, const T* rhs_bias, T* residual) const {
    residual[0] = T(weight_) * (rhs_bias[0] - lhs_bias[0]);
    residual[1] = T(weight_) * (rhs_bias[1] - lhs_bias[1]);
    residual[2] = T(weight_) * (rhs_bias[2] - lhs_bias[2]);
    return true;
  }

  static ceres::CostFunction* Create(double weight) {
    return new ceres::AutoDiffCostFunction<BiasDifferencePrior, 3, 3, 3>(
        new BiasDifferencePrior(weight));
  }

  double weight_;
};

struct TranslationDifferencePrior {
  explicit TranslationDifferencePrior(double weight) : weight_(weight) {}

  template <typename T>
  bool operator()(const T* lhs_trans, const T* rhs_trans, T* residual) const {
    residual[0] = T(weight_) * (rhs_trans[0] - lhs_trans[0]);
    residual[1] = T(weight_) * (rhs_trans[1] - lhs_trans[1]);
    residual[2] = T(weight_) * (rhs_trans[2] - lhs_trans[2]);
    return true;
  }

  static ceres::CostFunction* Create(double weight) {
    return new ceres::AutoDiffCostFunction<TranslationDifferencePrior, 3, 3, 3>(
        new TranslationDifferencePrior(weight));
  }

  double weight_;
};

struct OrientationDifferencePrior {
  explicit OrientationDifferencePrior(double weight) : weight_(weight) {}

  template <typename T>
  bool operator()(const T* lhs_quat, const T* rhs_quat, T* residual) const {
    Eigen::Quaternion<T> q_lhs{lhs_quat[3], lhs_quat[0], lhs_quat[1], lhs_quat[2]};
    Eigen::Quaternion<T> q_rhs{rhs_quat[3], rhs_quat[0], rhs_quat[1], rhs_quat[2]};
    Eigen::Quaternion<T> q_error = q_lhs.conjugate() * q_rhs;
    residual[0] = T(weight_) * T(2.0) * q_error.x();
    residual[1] = T(weight_) * T(2.0) * q_error.y();
    residual[2] = T(weight_) * T(2.0) * q_error.z();
    return true;
  }

  static ceres::CostFunction* Create(double weight) {
    return new ceres::AutoDiffCostFunction<OrientationDifferencePrior, 3, 4, 4>(
        new OrientationDifferencePrior(weight));
  }

  double weight_;
};

}  // namespace ct_lio
}  // namespace localization_zoo
