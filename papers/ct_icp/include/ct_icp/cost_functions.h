#pragma once

#include <ceres/ceres.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace localization_zoo {
namespace ct_icp {

/// CT Point-to-Plane コスト関数
/// 4パラメータブロック: begin_quat(4), begin_trans(3), end_quat(4), end_trans(3)
/// 残差: 1次元 (点と平面の符号付き距離)
struct CTPointToPlane {
  CTPointToPlane(Eigen::Vector3d raw_point, Eigen::Vector3d reference_point,
                 Eigen::Vector3d normal, double alpha, double weight)
      : raw_point_(raw_point),
        reference_point_(reference_point),
        normal_(normal),
        alpha_(alpha),
        weight_(weight) {}

  template <typename T>
  bool operator()(const T* begin_quat, const T* begin_trans,
                  const T* end_quat, const T* end_trans,
                  T* residual) const {
    // begin pose
    Eigen::Quaternion<T> q_begin{begin_quat[3], begin_quat[0], begin_quat[1],
                                  begin_quat[2]};
    Eigen::Matrix<T, 3, 1> t_begin{begin_trans[0], begin_trans[1],
                                    begin_trans[2]};
    // end pose
    Eigen::Quaternion<T> q_end{end_quat[3], end_quat[0], end_quat[1],
                                end_quat[2]};
    Eigen::Matrix<T, 3, 1> t_end{end_trans[0], end_trans[1], end_trans[2]};

    // SLERP 補間
    T a = T(alpha_);
    Eigen::Quaternion<T> q_inter = q_begin.slerp(a, q_end);
    Eigen::Matrix<T, 3, 1> t_inter =
        (T(1.0) - a) * t_begin + a * t_end;

    // 変換
    Eigen::Matrix<T, 3, 1> raw{T(raw_point_.x()), T(raw_point_.y()),
                                T(raw_point_.z())};
    Eigen::Matrix<T, 3, 1> ref{T(reference_point_.x()),
                                T(reference_point_.y()),
                                T(reference_point_.z())};
    Eigen::Matrix<T, 3, 1> n{T(normal_.x()), T(normal_.y()), T(normal_.z())};

    Eigen::Matrix<T, 3, 1> transformed = q_inter * raw + t_inter;
    residual[0] = T(weight_) * n.dot(ref - transformed);

    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& raw_point,
                                     const Eigen::Vector3d& reference_point,
                                     const Eigen::Vector3d& normal,
                                     double alpha, double weight) {
    return new ceres::AutoDiffCostFunction<CTPointToPlane, 1, 4, 3, 4, 3>(
        new CTPointToPlane(raw_point, reference_point, normal, alpha, weight));
  }

  Eigen::Vector3d raw_point_, reference_point_, normal_;
  double alpha_, weight_;
};

/// 位置整合性制約: ||t_begin^k - t_end^{k-1}||
struct LocationConsistency {
  LocationConsistency(Eigen::Vector3d previous_end_trans, double weight)
      : previous_end_trans_(previous_end_trans), weight_(weight) {}

  template <typename T>
  bool operator()(const T* begin_trans, T* residual) const {
    residual[0] = T(weight_) * (begin_trans[0] - T(previous_end_trans_.x()));
    residual[1] = T(weight_) * (begin_trans[1] - T(previous_end_trans_.y()));
    residual[2] = T(weight_) * (begin_trans[2] - T(previous_end_trans_.z()));
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& prev_end_trans,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<LocationConsistency, 3, 3>(
        new LocationConsistency(prev_end_trans, weight));
  }

  Eigen::Vector3d previous_end_trans_;
  double weight_;
};

/// 等速制約: ||v_current - v_previous||
struct ConstantVelocity {
  ConstantVelocity(Eigen::Vector3d previous_velocity, double weight)
      : previous_velocity_(previous_velocity), weight_(weight) {}

  template <typename T>
  bool operator()(const T* begin_trans, const T* end_trans,
                  T* residual) const {
    residual[0] =
        T(weight_) * ((end_trans[0] - begin_trans[0]) - T(previous_velocity_.x()));
    residual[1] =
        T(weight_) * ((end_trans[1] - begin_trans[1]) - T(previous_velocity_.y()));
    residual[2] =
        T(weight_) * ((end_trans[2] - begin_trans[2]) - T(previous_velocity_.z()));
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& prev_velocity,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<ConstantVelocity, 3, 3, 3>(
        new ConstantVelocity(prev_velocity, weight));
  }

  Eigen::Vector3d previous_velocity_;
  double weight_;
};

/// 姿勢整合性制約: 1 - (q_begin · q_prev_end)^2
struct OrientationConsistency {
  OrientationConsistency(Eigen::Quaterniond previous_end_quat, double weight)
      : previous_end_quat_(previous_end_quat), weight_(weight) {}

  template <typename T>
  bool operator()(const T* begin_quat, T* residual) const {
    Eigen::Quaternion<T> q_begin{begin_quat[3], begin_quat[0], begin_quat[1],
                                  begin_quat[2]};
    Eigen::Quaternion<T> q_prev{T(previous_end_quat_.w()),
                                 T(previous_end_quat_.x()),
                                 T(previous_end_quat_.y()),
                                 T(previous_end_quat_.z())};
    T dot = q_begin.dot(q_prev);
    residual[0] = T(weight_) * (T(1.0) - dot * dot);
    return true;
  }

  static ceres::CostFunction* Create(
      const Eigen::Quaterniond& prev_end_quat, double weight) {
    return new ceres::AutoDiffCostFunction<OrientationConsistency, 1, 4>(
        new OrientationConsistency(prev_end_quat, weight));
  }

  Eigen::Quaterniond previous_end_quat_;
  double weight_;
};

}  // namespace ct_icp
}  // namespace localization_zoo
