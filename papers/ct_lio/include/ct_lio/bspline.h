#pragma once

// Cumulative cubic SE3 B-spline (Sommer et al., "Efficient Derivative Computation
// for Cumulative B-Splines on Lie Groups", CVPR 2020).
//
// Stage 1 scope (this file):
//   - 4 SE3 control points P0, P1, P2, P3 per scan.
//   - Local time u = (t - t_begin) / (t_end - t_begin) in [0, 1].
//   - Cubic basis B_j(u) and cumulative basis B_tilde_i(u) = sum_{j=i}^{3} B_j(u).
//   - Pose at time u:
//       T(u) = P0
//              * Exp(B_tilde_1(u) * log(P0^-1 * P1))
//              * Exp(B_tilde_2(u) * log(P1^-1 * P2))
//              * Exp(B_tilde_3(u) * log(P2^-1 * P3))
//
// We expose a templated helper so Ceres autodiff can differentiate through the
// spline. The control points are passed as raw pointers (4D quat XYZW + 3D
// trans per CP), matching the existing CTPointToPlane parameter-block layout.

#include <ceres/ceres.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace localization_zoo {
namespace ct_lio {
namespace bspline {

// Cubic B-spline basis functions (uniform knots, t in [0,1] per segment).
//   B0(u) = (1-u)^3 / 6
//   B1(u) = (3u^3 - 6u^2 + 4) / 6
//   B2(u) = (-3u^3 + 3u^2 + 3u + 1) / 6
//   B3(u) = u^3 / 6
template <typename T>
inline void cubicBasis(const T& u, T b[4]) {
  const T one = T(1.0);
  const T u2 = u * u;
  const T u3 = u2 * u;
  const T inv6 = T(1.0 / 6.0);
  b[0] = inv6 * (one - u) * (one - u) * (one - u);
  b[1] = inv6 * (T(3.0) * u3 - T(6.0) * u2 + T(4.0));
  b[2] = inv6 * (T(-3.0) * u3 + T(3.0) * u2 + T(3.0) * u + T(1.0));
  b[3] = inv6 * u3;
}

// Cumulative basis: B_tilde_i(u) = sum_{j=i}^{3} B_j(u). B_tilde_0 == 1 always.
template <typename T>
inline void cumulativeBasis(const T& u, T b_tilde[4]) {
  T b[4];
  cubicBasis<T>(u, b);
  b_tilde[3] = b[3];
  b_tilde[2] = b[2] + b_tilde[3];
  b_tilde[1] = b[1] + b_tilde[2];
  b_tilde[0] = b[0] + b_tilde[1];  // == 1
}

// SO3 log map: q -> omega in R^3.
template <typename T>
inline Eigen::Matrix<T, 3, 1> logQuaternion(const Eigen::Quaternion<T>& q) {
  const T w = q.w();
  Eigen::Matrix<T, 3, 1> v(q.x(), q.y(), q.z());
  const T n2 = v.squaredNorm();
  if (n2 < T(1e-20)) {
    return T(2.0) * v;  // small-angle approximation
  }
  const T n = ceres::sqrt(n2);
  const T abs_w = ceres::abs(w);
  T theta;
  if (abs_w < T(0.999999)) {
    theta = T(2.0) * ceres::atan2(n, w);
  } else {
    // Fallback when w is close to +/-1 (very small rotation).
    theta = T(2.0) * n / w;
  }
  return (theta / n) * v;
}

// SO3 exp map: omega in R^3 -> quaternion.
template <typename T>
inline Eigen::Quaternion<T> expQuaternion(const Eigen::Matrix<T, 3, 1>& omega) {
  const T theta2 = omega.squaredNorm();
  if (theta2 < T(1e-20)) {
    Eigen::Quaternion<T> q(T(1.0), T(0.5) * omega.x(), T(0.5) * omega.y(),
                           T(0.5) * omega.z());
    q.normalize();
    return q;
  }
  const T theta = ceres::sqrt(theta2);
  const T half = T(0.5) * theta;
  const T s = ceres::sin(half) / theta;
  return Eigen::Quaternion<T>(ceres::cos(half), s * omega.x(), s * omega.y(),
                              s * omega.z());
}

// Build an SE3 pose (q, t) at local time u from 4 control points (q_i, t_i).
// The control point arrays are XYZW for quaternion, XYZ for translation —
// matching the existing CT-LIO param block layout.
//
// We define rotation via cumulative quaternion product:
//   q(u) = q0 * Exp(B_tilde_1 * log(q0^-1 q1))
//             * Exp(B_tilde_2 * log(q1^-1 q2))
//             * Exp(B_tilde_3 * log(q2^-1 q3))
//
// For translation, cumulative chain in R^3 (additive group) reduces to the
// standard cumulative B-spline:
//   t(u) = t0 + B_tilde_1*(t1-t0) + B_tilde_2*(t2-t1) + B_tilde_3*(t3-t2)
template <typename T>
inline void interpolateSe3(const T* const q0_ptr, const T* const t0_ptr,
                           const T* const q1_ptr, const T* const t1_ptr,
                           const T* const q2_ptr, const T* const t2_ptr,
                           const T* const q3_ptr, const T* const t3_ptr,
                           const T& u, Eigen::Quaternion<T>* q_out,
                           Eigen::Matrix<T, 3, 1>* t_out) {
  const Eigen::Quaternion<T> q0(q0_ptr[3], q0_ptr[0], q0_ptr[1], q0_ptr[2]);
  const Eigen::Quaternion<T> q1(q1_ptr[3], q1_ptr[0], q1_ptr[1], q1_ptr[2]);
  const Eigen::Quaternion<T> q2(q2_ptr[3], q2_ptr[0], q2_ptr[1], q2_ptr[2]);
  const Eigen::Quaternion<T> q3(q3_ptr[3], q3_ptr[0], q3_ptr[1], q3_ptr[2]);
  const Eigen::Matrix<T, 3, 1> t0(t0_ptr[0], t0_ptr[1], t0_ptr[2]);
  const Eigen::Matrix<T, 3, 1> t1(t1_ptr[0], t1_ptr[1], t1_ptr[2]);
  const Eigen::Matrix<T, 3, 1> t2(t2_ptr[0], t2_ptr[1], t2_ptr[2]);
  const Eigen::Matrix<T, 3, 1> t3(t3_ptr[0], t3_ptr[1], t3_ptr[2]);

  T b_tilde[4];
  cumulativeBasis<T>(u, b_tilde);

  const Eigen::Matrix<T, 3, 1> omega1 = logQuaternion<T>(q0.conjugate() * q1);
  const Eigen::Matrix<T, 3, 1> omega2 = logQuaternion<T>(q1.conjugate() * q2);
  const Eigen::Matrix<T, 3, 1> omega3 = logQuaternion<T>(q2.conjugate() * q3);

  const Eigen::Quaternion<T> A1 = expQuaternion<T>(b_tilde[1] * omega1);
  const Eigen::Quaternion<T> A2 = expQuaternion<T>(b_tilde[2] * omega2);
  const Eigen::Quaternion<T> A3 = expQuaternion<T>(b_tilde[3] * omega3);

  *q_out = q0 * A1 * A2 * A3;
  (*q_out).normalize();

  *t_out = t0 + b_tilde[1] * (t1 - t0) + b_tilde[2] * (t2 - t1) +
           b_tilde[3] * (t3 - t2);
}

}  // namespace bspline
}  // namespace ct_lio
}  // namespace localization_zoo
