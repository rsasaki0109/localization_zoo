#pragma once

#include <ceres/ceres.h>
#include <ceres/rotation.h>
#include <Eigen/Core>

#include <array>

namespace localization_zoo {
namespace inten_loam {

template <typename T>
inline void EigenQuaternionRotatePoint(const T* eigen_q, const T* point,
                                       T* result) {
  const T ceres_q[4] = {eigen_q[3], eigen_q[0], eigen_q[1], eigen_q[2]};
  ceres::QuaternionRotatePoint(ceres_q, point, result);
}

struct EdgeFactor {
  EdgeFactor(const Eigen::Vector3d& curr, const Eigen::Vector3d& last_a,
             const Eigen::Vector3d& last_b, double s, double weight)
      : curr_(curr), last_a_(last_a), last_b_(last_b), s_(s), weight_(weight) {}

  template <typename T>
  bool operator()(const T* q, const T* t, T* residual) const {
    T cp[3] = {T(curr_.x()), T(curr_.y()), T(curr_.z())};
    T lpa[3] = {T(last_a_.x()), T(last_a_.y()), T(last_a_.z())};
    T lpb[3] = {T(last_b_.x()), T(last_b_.y()), T(last_b_.z())};
    T p[3];
    EigenQuaternionRotatePoint(q, cp, p);
    p[0] += T(s_) * t[0];
    p[1] += T(s_) * t[1];
    p[2] += T(s_) * t[2];
    T nu[3] = {(p[0] - lpa[0]) * (p[2] - lpb[2]) - (p[2] - lpa[2]) * (p[1] - lpb[1]),
               (p[1] - lpa[1]) * (p[0] - lpb[0]) - (p[0] - lpa[0]) * (p[1] - lpb[1]),
               (p[2] - lpa[2]) * (p[1] - lpb[1]) - (p[1] - lpa[1]) * (p[2] - lpb[2])};
    T de[3] = {lpa[0] - lpb[0], lpa[1] - lpb[1], lpa[2] - lpb[2]};
    T de_norm = ceres::sqrt(de[0] * de[0] + de[1] * de[1] + de[2] * de[2]);
    residual[0] = T(weight_) * nu[0] / de_norm;
    residual[1] = T(weight_) * nu[1] / de_norm;
    residual[2] = T(weight_) * nu[2] / de_norm;
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& curr,
                                     const Eigen::Vector3d& last_a,
                                     const Eigen::Vector3d& last_b, double s,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<EdgeFactor, 3, 4, 3>(
        new EdgeFactor(curr, last_a, last_b, s, weight));
  }

  Eigen::Vector3d curr_, last_a_, last_b_;
  double s_, weight_;
};

struct PlaneFactor {
  PlaneFactor(const Eigen::Vector3d& curr, const Eigen::Vector3d& last_j,
              const Eigen::Vector3d& last_l, const Eigen::Vector3d& last_m,
              double s, double weight)
      : curr_(curr), last_j_(last_j), s_(s), weight_(weight) {
    ljm_norm_ = (last_j - last_l).cross(last_j - last_m);
    ljm_norm_.normalize();
  }

  template <typename T>
  bool operator()(const T* q, const T* t, T* residual) const {
    T cp[3] = {T(curr_.x()), T(curr_.y()), T(curr_.z())};
    T p[3];
    EigenQuaternionRotatePoint(q, cp, p);
    p[0] += T(s_) * t[0];
    p[1] += T(s_) * t[1];
    p[2] += T(s_) * t[2];
    T lj[3] = {T(last_j_.x()), T(last_j_.y()), T(last_j_.z())};
    T n[3] = {T(ljm_norm_.x()), T(ljm_norm_.y()), T(ljm_norm_.z())};
    residual[0] = T(weight_) * ((p[0] - lj[0]) * n[0] + (p[1] - lj[1]) * n[1] +
                                (p[2] - lj[2]) * n[2]);
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& curr,
                                     const Eigen::Vector3d& last_j,
                                     const Eigen::Vector3d& last_l,
                                     const Eigen::Vector3d& last_m, double s,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<PlaneFactor, 1, 4, 3>(
        new PlaneFactor(curr, last_j, last_l, last_m, s, weight));
  }

  Eigen::Vector3d curr_, last_j_;
  Eigen::Vector3d ljm_norm_;
  double s_, weight_;
};

/// B-spline 曲面の簡約版: 局所 8 ボクセル強度の三線形補間残差。
struct IntensityBsplineFactor {
  IntensityBsplineFactor(const Eigen::Vector3d& source, double observed,
                         const Eigen::Vector3d& box_min,
                         const Eigen::Vector3d& box_max,
                         const std::array<double, 8>& corner_intensity,
                         double weight)
      : source_(source),
        observed_(observed),
        box_min_(box_min),
        box_max_(box_max),
        corner_intensity_(corner_intensity),
        weight_(weight) {}

  template <typename T>
  bool operator()(const T* q, const T* t, T* residual) const {
    T p[3] = {T(source_.x()), T(source_.y()), T(source_.z())};
    T pt[3];
    EigenQuaternionRotatePoint(q, p, pt);
    pt[0] += t[0];
    pt[1] += t[1];
    pt[2] += t[2];

    const T eps = T(1e-6);
    const T sx = T(box_max_.x()) - T(box_min_.x());
    const T sy = T(box_max_.y()) - T(box_min_.y());
    const T sz = T(box_max_.z()) - T(box_min_.z());
    const T u = (pt[0] - T(box_min_.x())) / (sx + eps);
    const T v = (pt[1] - T(box_min_.y())) / (sy + eps);
    const T w = (pt[2] - T(box_min_.z())) / (sz + eps);

    const T c000 = T(corner_intensity_[0]);
    const T c100 = T(corner_intensity_[1]);
    const T c010 = T(corner_intensity_[2]);
    const T c110 = T(corner_intensity_[3]);
    const T c001 = T(corner_intensity_[4]);
    const T c101 = T(corner_intensity_[5]);
    const T c011 = T(corner_intensity_[6]);
    const T c111 = T(corner_intensity_[7]);

    const T c00 = c000 * (T(1) - u) + c100 * u;
    const T c10 = c010 * (T(1) - u) + c110 * u;
    const T c01 = c001 * (T(1) - u) + c101 * u;
    const T c11 = c011 * (T(1) - u) + c111 * u;
    const T c0 = c00 * (T(1) - v) + c10 * v;
    const T c1 = c01 * (T(1) - v) + c11 * v;
    const T pred = c0 * (T(1) - w) + c1 * w;

    residual[0] = T(weight_) * (pred - T(observed_));
    return true;
  }

  static ceres::CostFunction* Create(
      const Eigen::Vector3d& source, double observed,
      const Eigen::Vector3d& box_min, const Eigen::Vector3d& box_max,
      const std::array<double, 8>& corner_intensity, double weight) {
    return new ceres::AutoDiffCostFunction<IntensityBsplineFactor, 1, 4, 3>(
        new IntensityBsplineFactor(source, observed, box_min, box_max,
                                 corner_intensity, weight));
  }

  Eigen::Vector3d source_;
  double observed_;
  Eigen::Vector3d box_min_, box_max_;
  std::array<double, 8> corner_intensity_;
  double weight_;
};

}  // namespace inten_loam
}  // namespace localization_zoo
