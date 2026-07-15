#pragma once

#include <ceres/ceres.h>
#include <ceres/rotation.h>
#include <Eigen/Core>

namespace localization_zoo {
namespace pl_loam {

template <typename T>
inline void EigenQuaternionRotatePoint(const T* eigen_q, const T* point,
                                       T* result) {
  const T ceres_q[4] = {eigen_q[3], eigen_q[0], eigen_q[1], eigen_q[2]};
  ceres::QuaternionRotatePoint(ceres_q, point, result);
}

/// 3D 点 (prev カメラ) の再投影残差。
struct PointReprojFactor {
  PointReprojFactor(const Eigen::Vector3d& point_prev_cam,
                    const Eigen::Vector2d& obs_uv, double fx, double fy,
                    double cx, double cy, double weight)
      : point_(point_prev_cam),
        obs_(obs_uv),
        fx_(fx),
        fy_(fy),
        cx_(cx),
        cy_(cy),
        weight_(weight) {}

  template <typename T>
  bool operator()(const T* q, const T* t, T* residual) const {
    T p[3] = {T(point_.x()), T(point_.y()), T(point_.z())};
    T p_cam[3];
    EigenQuaternionRotatePoint(q, p, p_cam);
    p_cam[0] += t[0];
    p_cam[1] += t[1];
    p_cam[2] += t[2];
    if (p_cam[2] < T(0.1)) {
      residual[0] = T(0);
      residual[1] = T(0);
      return true;
    }
    const T u = fx_ * p_cam[0] / p_cam[2] + cx_;
    const T v = fy_ * p_cam[1] / p_cam[2] + cy_;
    residual[0] = T(weight_) * (u - T(obs_.x()));
    residual[1] = T(weight_) * (v - T(obs_.y()));
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& point_prev_cam,
                                     const Eigen::Vector2d& obs_uv, double fx,
                                     double fy, double cx, double cy,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<PointReprojFactor, 2, 4, 3>(
        new PointReprojFactor(point_prev_cam, obs_uv, fx, fy, cx, cy, weight));
  }

  Eigen::Vector3d point_;
  Eigen::Vector2d obs_;
  double fx_, fy_, cx_, cy_, weight_;
};

/// LiDAR 深度 prior: 変換後カメラ Z と prior の差。
struct PointDepthPriorFactor {
  PointDepthPriorFactor(const Eigen::Vector3d& point_prev_cam,
                          double depth_prior, double weight)
      : point_(point_prev_cam), depth_prior_(depth_prior), weight_(weight) {}

  template <typename T>
  bool operator()(const T* q, const T* t, T* residual) const {
    T p[3] = {T(point_.x()), T(point_.y()), T(point_.z())};
    T p_cam[3];
    EigenQuaternionRotatePoint(q, p, p_cam);
    p_cam[2] += t[2];
    if (p_cam[2] < T(0.1)) {
      residual[0] = T(0);
      return true;
    }
    residual[0] = T(weight_) * (p_cam[2] - T(depth_prior_));
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& point_prev_cam,
                                     double depth_prior, double weight) {
    return new ceres::AutoDiffCostFunction<PointDepthPriorFactor, 1, 4, 3>(
        new PointDepthPriorFactor(point_prev_cam, depth_prior, weight));
  }

  Eigen::Vector3d point_;
  double depth_prior_, weight_;
};

/// 線分端点 (prev カメラ) の再投影残差。
struct LineEndpointReprojFactor {
  LineEndpointReprojFactor(const Eigen::Vector3d& endpoint_prev_cam,
                           const Eigen::Vector2d& obs_uv, double fx,
                           double fy, double cx, double cy, double weight)
      : point_(endpoint_prev_cam),
        obs_(obs_uv),
        fx_(fx),
        fy_(fy),
        cx_(cx),
        cy_(cy),
        weight_(weight) {}

  template <typename T>
  bool operator()(const T* q, const T* t, T* residual) const {
    T p[3] = {T(point_.x()), T(point_.y()), T(point_.z())};
    T p_cam[3];
    EigenQuaternionRotatePoint(q, p, p_cam);
    p_cam[0] += t[0];
    p_cam[1] += t[1];
    p_cam[2] += t[2];
    if (p_cam[2] < T(0.1)) {
      residual[0] = T(0);
      residual[1] = T(0);
      return true;
    }
    const T u = fx_ * p_cam[0] / p_cam[2] + cx_;
    const T v = fy_ * p_cam[1] / p_cam[2] + cy_;
    residual[0] = T(weight_) * (u - T(obs_.x()));
    residual[1] = T(weight_) * (v - T(obs_.y()));
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& endpoint_prev_cam,
                                     const Eigen::Vector2d& obs_uv, double fx,
                                     double fy, double cx, double cy,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<LineEndpointReprojFactor, 2, 4, 3>(
        new LineEndpointReprojFactor(endpoint_prev_cam, obs_uv, fx, fy, cx, cy,
                                     weight));
  }

  Eigen::Vector3d point_;
  Eigen::Vector2d obs_;
  double fx_, fy_, cx_, cy_, weight_;
};

}  // namespace pl_loam
}  // namespace localization_zoo
