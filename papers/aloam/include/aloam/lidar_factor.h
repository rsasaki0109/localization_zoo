#pragma once

#include <ceres/ceres.h>
#include <ceres/rotation.h>
#include <Eigen/Core>

namespace localization_zoo {
namespace aloam {

/// エッジ残差: 点と直線(2点定義)の距離
/// 残差次元: 3, パラメータ: q(4), t(3)
struct LidarEdgeFactor {
  LidarEdgeFactor(Eigen::Vector3d curr_point, Eigen::Vector3d last_point_a,
                  Eigen::Vector3d last_point_b, double s)
      : curr_point_(curr_point),
        last_point_a_(last_point_a),
        last_point_b_(last_point_b),
        s_(s) {}

  template <typename T>
  bool operator()(const T* q, const T* t, T* residual) const {
    Eigen::Matrix<T, 3, 1> cp{T(curr_point_.x()), T(curr_point_.y()),
                               T(curr_point_.z())};
    Eigen::Matrix<T, 3, 1> lpa{T(last_point_a_.x()), T(last_point_a_.y()),
                                T(last_point_a_.z())};
    Eigen::Matrix<T, 3, 1> lpb{T(last_point_b_.x()), T(last_point_b_.y()),
                                T(last_point_b_.z())};

    // 変換: lp = q * cp + t
    Eigen::Quaternion<T> q_curr{q[3], q[0], q[1], q[2]};
    Eigen::Quaternion<T> q_identity{T(1), T(0), T(0), T(0)};
    q_curr = q_identity.slerp(T(s_), q_curr);
    Eigen::Matrix<T, 3, 1> t_curr{T(s_) * t[0], T(s_) * t[1], T(s_) * t[2]};

    Eigen::Matrix<T, 3, 1> lp = q_curr * cp + t_curr;

    // 残差: 外積 / 線分長 = 点と直線の距離
    Eigen::Matrix<T, 3, 1> nu = (lp - lpa).cross(lp - lpb);
    Eigen::Matrix<T, 3, 1> de = lpa - lpb;
    T de_norm = de.norm();

    residual[0] = nu.x() / de_norm;
    residual[1] = nu.y() / de_norm;
    residual[2] = nu.z() / de_norm;

    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& curr_point,
                                     const Eigen::Vector3d& last_point_a,
                                     const Eigen::Vector3d& last_point_b,
                                     double s) {
    return new ceres::AutoDiffCostFunction<LidarEdgeFactor, 3, 4, 3>(
        new LidarEdgeFactor(curr_point, last_point_a, last_point_b, s));
  }

  Eigen::Vector3d curr_point_, last_point_a_, last_point_b_;
  double s_;
};

/// 平面残差: 点と平面(3点定義)の距離 (scan-to-scan用)
/// 残差次元: 1, パラメータ: q(4), t(3)
struct LidarPlaneFactor {
  LidarPlaneFactor(Eigen::Vector3d curr_point, Eigen::Vector3d last_point_j,
                   Eigen::Vector3d last_point_l, Eigen::Vector3d last_point_m,
                   double s)
      : curr_point_(curr_point),
        last_point_j_(last_point_j),
        last_point_l_(last_point_l),
        last_point_m_(last_point_m),
        s_(s) {
    // 法線を事前計算
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

    residual[0] = (lp - lpj).dot(ljm);

    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& curr_point,
                                     const Eigen::Vector3d& last_point_j,
                                     const Eigen::Vector3d& last_point_l,
                                     const Eigen::Vector3d& last_point_m,
                                     double s) {
    return new ceres::AutoDiffCostFunction<LidarPlaneFactor, 1, 4, 3>(
        new LidarPlaneFactor(curr_point, last_point_j, last_point_l,
                             last_point_m, s));
  }

  Eigen::Vector3d curr_point_, last_point_j_, last_point_l_, last_point_m_;
  Eigen::Vector3d ljm_norm_;
  double s_;
};

/// 平面残差: 法線+距離による定義 (scan-to-map用)
/// 残差次元: 1, パラメータ: q(4), t(3)
struct LidarPlaneNormFactor {
  LidarPlaneNormFactor(Eigen::Vector3d curr_point, Eigen::Vector3d plane_norm,
                       double negative_oa_dot_norm)
      : curr_point_(curr_point),
        plane_norm_(plane_norm),
        negative_oa_dot_norm_(negative_oa_dot_norm) {}

  template <typename T>
  bool operator()(const T* q, const T* t, T* residual) const {
    Eigen::Quaternion<T> q_w{q[3], q[0], q[1], q[2]};
    Eigen::Matrix<T, 3, 1> t_w{t[0], t[1], t[2]};
    Eigen::Matrix<T, 3, 1> cp{T(curr_point_.x()), T(curr_point_.y()),
                               T(curr_point_.z())};
    Eigen::Matrix<T, 3, 1> norm{T(plane_norm_.x()), T(plane_norm_.y()),
                                 T(plane_norm_.z())};

    Eigen::Matrix<T, 3, 1> point_w = q_w * cp + t_w;
    residual[0] = norm.dot(point_w) + T(negative_oa_dot_norm_);

    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& curr_point,
                                     const Eigen::Vector3d& plane_norm,
                                     double negative_oa_dot_norm) {
    return new ceres::AutoDiffCostFunction<LidarPlaneNormFactor, 1, 4, 3>(
        new LidarPlaneNormFactor(curr_point, plane_norm,
                                 negative_oa_dot_norm));
  }

  Eigen::Vector3d curr_point_, plane_norm_;
  double negative_oa_dot_norm_;
};

}  // namespace aloam
}  // namespace localization_zoo
