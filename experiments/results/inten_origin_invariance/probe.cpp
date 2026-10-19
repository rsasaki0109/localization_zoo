// Equation-level origin-shift experiment, not an odometry implementation.
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include "native_fragments.h"

using native::Mat6;
using native::Vec6;
using Mat4 = Eigen::Matrix4d;

Eigen::Matrix3d skew(const Eigen::Vector3d& a) {
  Eigen::Matrix3d s;
  s << 0., -a.z(), a.y(), a.z(), 0., -a.x(), -a.y(), a.x(), 0.;
  return s;
}

// Independent exact SE(3) exponential control. The native update is retained
// verbatim in native_fragments.h; no estimator uses this control.
Mat4 exactExp(const Vec6& d) {
  const double theta = d.tail<3>().norm();
  const double x = theta * theta;
  const Eigen::Matrix3d W = skew(d.tail<3>());
  const double b = theta < 1e-4 ? .5 - x / 24. + x*x / 720.
                               : (1. - std::cos(theta)) / x;
  const double c = theta < 1e-4 ? 1./6. - x / 120. + x*x / 5040.
                               : (theta - std::sin(theta)) / (x * theta);
  Mat4 result = Mat4::Identity();
  if (theta > 0.)
    result.topLeftCorner<3,3>() = Eigen::AngleAxisd(theta, d.tail<3>() / theta).toRotationMatrix();
  result.topRightCorner<3,1>() =
      (Eigen::Matrix3d::Identity() + b * W + c * W * W) * d.head<3>();
  return result;
}

Eigen::Vector3d sensorTranslation(const Mat4& T, const Vec6& d) {
  return T.topLeftCorner<3,3>().transpose() *
      (d.head<3>() - T.topRightCorner<3,1>().cross(d.tail<3>()));
}

int main() {
  std::cout << std::setprecision(17);
  std::cout << "id,observed_delta_linf,normal_covariance_relative_error,gradient_covariance_relative_error,"
               "transported_native_position_error_m,transport_formula_error_m,exact_exp_position_error_m,"
               "shifted_native_position_error_m,raw_solve_sensor_translation_error_m,raw_solve_rotation_error_rad,"
               "covariant_damping_sensor_translation_error_m,covariant_damping_rotation_error_rad,"
               "raw_solve_finite,covariant_solve_finite\n";
  std::string id;
  while (std::cin >> id) {
    Eigen::Vector3d c;
    Mat6 H;
    Vec6 g, observed;
    Mat4 T;
    for (int i=0;i<3;++i) std::cin >> c[i];
    for (int i=0;i<6;++i) for (int j=0;j<6;++j) std::cin >> H(i,j);
    for (int i=0;i<6;++i) std::cin >> g[i];
    for (int i=0;i<6;++i) std::cin >> observed[i];
    for (int i=0;i<4;++i) for (int j=0;j<4;++j) std::cin >> T(i,j);
    if (!std::cin || !H.allFinite() || !g.allFinite() || !observed.allFinite() || !T.allFinite()) return 2;
    Mat6 B = Mat6::Identity(), inverse = Mat6::Identity();
    B.topRightCorner<3,3>() = skew(c);
    inverse.topRightCorner<3,3>() = -skew(c);
    const Mat6 shifted_H = inverse.transpose() * H * inverse;
    const Vec6 shifted_g = inverse.transpose() * g;
    const Vec6 d = native::solve(H,g);
    const Vec6 transported = B*d;
    const Vec6 raw = native::solve(shifted_H,shifted_g);
    // Covariantly transport the same existing regularizer as a numerical
    // control; raw coordinate damping and this control have different priors.
    const Mat6 penalty = 1e-6 * inverse.transpose() * inverse;
    const Vec6 covariant = (shifted_H + penalty).ldlt().solve(-shifted_g);
    Mat4 C=Mat4::Identity(), undo=Mat4::Identity();
    C.topRightCorner<3,1>()=c;
    undo.topRightCorner<3,1>()=-c;
    const Mat4 shifted_T=C*T;
    const Mat4 original=native::expSe3(d)*T;
    const Mat4 transported_pose=undo*native::expSe3(transported)*shifted_T;
    const Mat4 raw_pose=undo*native::expSe3(raw)*shifted_T;
    const Eigen::Vector3d difference=transported_pose.topRightCorner<3,1>()-original.topRightCorner<3,1>();
    const Eigen::Vector3d formula=(native::expSe3(d).topLeftCorner<3,3>()-Eigen::Matrix3d::Identity())*c
                               +c.cross(d.tail<3>());
    const Mat4 exact_original=exactExp(d)*T;
    const Mat4 exact_transported=undo*exactExp(transported)*shifted_T;
    std::cout << id << ',' << (d-observed).cwiseAbs().maxCoeff()
      << ',' << (B.transpose()*shifted_H*B-H).norm()/std::max(1.,H.norm())
      << ',' << (B.transpose()*shifted_g-g).norm()/std::max(1.,g.norm())
      << ',' << difference.norm() << ',' << (difference-formula).norm()
      << ',' << (exact_original.topRightCorner<3,1>()-exact_transported.topRightCorner<3,1>()).norm()
      << ',' << (original.topRightCorner<3,1>()-raw_pose.topRightCorner<3,1>()).norm()
      << ',' << (sensorTranslation(shifted_T,raw)-sensorTranslation(T,d)).norm()
      << ',' << (raw.tail<3>()-d.tail<3>()).norm()
      << ',' << (sensorTranslation(shifted_T,covariant)-sensorTranslation(T,d)).norm()
      << ',' << (covariant.tail<3>()-d.tail<3>()).norm()
      << ',' << raw.allFinite() << ',' << covariant.allFinite() << '\n';
  }
  return std::cin.eof() ? 0 : 2;
}
