#pragma once

#include <ceres/ceres.h>
#include <ceres/rotation.h>
#include <Eigen/Core>

namespace localization_zoo {
namespace i_loam {

// I-LOAM (Park, Jang, Kim, UR 2020) の中核: 反射強度の類似度で
// 各対応の残差を重み付けする。aloam の LidarEdgeFactor / LidarPlaneFactor
// と同一の幾何残差に、スカラ重み w (>0) を乗じるだけ。最小二乗では
// コスト ∝ w^2 * r^2 になるため、w<1 で対応の寄与が弱まる。

/// 強度重み付きエッジ残差: 点と直線(2点定義)の距離 × w
struct IntensityWeightedEdgeFactor {
  IntensityWeightedEdgeFactor(Eigen::Vector3d curr_point,
                              Eigen::Vector3d last_point_a,
                              Eigen::Vector3d last_point_b, double s,
                              double weight)
      : curr_point_(curr_point),
        last_point_a_(last_point_a),
        last_point_b_(last_point_b),
        s_(s),
        weight_(weight) {}

  template <typename T>
  bool operator()(const T* q, const T* t, T* residual) const {
    Eigen::Matrix<T, 3, 1> cp{T(curr_point_.x()), T(curr_point_.y()),
                              T(curr_point_.z())};
    Eigen::Matrix<T, 3, 1> lpa{T(last_point_a_.x()), T(last_point_a_.y()),
                               T(last_point_a_.z())};
    Eigen::Matrix<T, 3, 1> lpb{T(last_point_b_.x()), T(last_point_b_.y()),
                               T(last_point_b_.z())};

    Eigen::Quaternion<T> q_curr{q[3], q[0], q[1], q[2]};
    Eigen::Quaternion<T> q_identity{T(1), T(0), T(0), T(0)};
    q_curr = q_identity.slerp(T(s_), q_curr);
    Eigen::Matrix<T, 3, 1> t_curr{T(s_) * t[0], T(s_) * t[1], T(s_) * t[2]};

    Eigen::Matrix<T, 3, 1> lp = q_curr * cp + t_curr;

    Eigen::Matrix<T, 3, 1> nu = (lp - lpa).cross(lp - lpb);
    Eigen::Matrix<T, 3, 1> de = lpa - lpb;
    T de_norm = de.norm();
    T w = T(weight_);

    residual[0] = w * nu.x() / de_norm;
    residual[1] = w * nu.y() / de_norm;
    residual[2] = w * nu.z() / de_norm;
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& curr_point,
                                     const Eigen::Vector3d& last_point_a,
                                     const Eigen::Vector3d& last_point_b,
                                     double s, double weight) {
    return new ceres::AutoDiffCostFunction<IntensityWeightedEdgeFactor, 3, 4, 3>(
        new IntensityWeightedEdgeFactor(curr_point, last_point_a, last_point_b,
                                        s, weight));
  }

  Eigen::Vector3d curr_point_, last_point_a_, last_point_b_;
  double s_;
  double weight_;
};

/// 強度重み付き平面残差: 点と平面(3点定義)の距離 × w
struct IntensityWeightedPlaneFactor {
  IntensityWeightedPlaneFactor(Eigen::Vector3d curr_point,
                               Eigen::Vector3d last_point_j,
                               Eigen::Vector3d last_point_l,
                               Eigen::Vector3d last_point_m, double s,
                               double weight)
      : curr_point_(curr_point),
        last_point_j_(last_point_j),
        s_(s),
        weight_(weight) {
    ljm_norm_ =
        (last_point_j - last_point_l).cross(last_point_j - last_point_m);
    ljm_norm_.normalize();
  }

  template <typename T>
  bool operator()(const T* q, const T* t, T* residual) const {
    Eigen::Matrix<T, 3, 1> cp{T(curr_point_.x()), T(curr_point_.y()),
                              T(curr_point_.z())};
    Eigen::Matrix<T, 3, 1> lpj{T(last_point_j_.x()), T(last_point_j_.y()),
                               T(last_point_j_.z())};
    Eigen::Matrix<T, 3, 1> ljm{T(ljm_norm_.x()), T(ljm_norm_.y()),
                               T(ljm_norm_.z())};

    Eigen::Quaternion<T> q_curr{q[3], q[0], q[1], q[2]};
    Eigen::Quaternion<T> q_identity{T(1), T(0), T(0), T(0)};
    q_curr = q_identity.slerp(T(s_), q_curr);
    Eigen::Matrix<T, 3, 1> t_curr{T(s_) * t[0], T(s_) * t[1], T(s_) * t[2]};

    Eigen::Matrix<T, 3, 1> lp = q_curr * cp + t_curr;

    residual[0] = T(weight_) * (lp - lpj).dot(ljm);
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& curr_point,
                                     const Eigen::Vector3d& last_point_j,
                                     const Eigen::Vector3d& last_point_l,
                                     const Eigen::Vector3d& last_point_m,
                                     double s, double weight) {
    return new ceres::AutoDiffCostFunction<IntensityWeightedPlaneFactor, 1, 4,
                                           3>(
        new IntensityWeightedPlaneFactor(curr_point, last_point_j, last_point_l,
                                         last_point_m, s, weight));
  }

  Eigen::Vector3d curr_point_, last_point_j_;
  Eigen::Vector3d ljm_norm_;
  double s_;
  double weight_;
};

}  // namespace i_loam
}  // namespace localization_zoo
