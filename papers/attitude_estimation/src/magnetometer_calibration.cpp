#include "attitude_estimation/magnetometer_calibration.h"

#include <Eigen/Eigenvalues>
#include <Eigen/LU>

#include <cmath>
#include <set>
#include <tuple>

namespace localization_zoo {
namespace attitude_estimation {

MagnetometerCalibration fitMagnetometerCalibration(const std::vector<Eigen::Vector3d>& readings) {
  MagnetometerCalibration out;
  if (readings.size() < 10) return out;

  // Centre and scale the data first for conditioning; undone at the end.
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  for (const auto& m : readings) mean += m;
  mean /= static_cast<double>(readings.size());
  double scale = 0.0;
  for (const auto& m : readings) scale += (m - mean).norm();
  scale /= static_cast<double>(readings.size());
  if (!(scale > 0)) return out;

  // Design matrix rows [x^2, y^2, z^2, 2yz, 2xz, 2xy, 2x, 2y, 2z, 1].
  Eigen::Matrix<double, 10, 10> S = Eigen::Matrix<double, 10, 10>::Zero();
  for (const auto& m : readings) {
    const Eigen::Vector3d p = (m - mean) / scale;
    Eigen::Matrix<double, 10, 1> d;
    d << p.x() * p.x(), p.y() * p.y(), p.z() * p.z(), 2 * p.y() * p.z(), 2 * p.x() * p.z(),
        2 * p.x() * p.y(), 2 * p.x(), 2 * p.y(), 2 * p.z(), 1.0;
    S += d * d.transpose();
  }
  const Eigen::Matrix<double, 6, 6> S11 = S.topLeftCorner<6, 6>();
  const Eigen::Matrix<double, 6, 4> S12 = S.topRightCorner<6, 4>();
  const Eigen::Matrix<double, 4, 4> S22 = S.bottomRightCorner<4, 4>();

  // Li and Griffiths constraint matrix with k = 4.
  const double k = 4.0;
  Eigen::Matrix<double, 6, 6> C = Eigen::Matrix<double, 6, 6>::Zero();
  C.topLeftCorner<3, 3>().setConstant(k / 2 - 1);
  C.topLeftCorner<3, 3>().diagonal().setConstant(-1);
  C.bottomRightCorner<3, 3>().diagonal().setConstant(-k);

  const Eigen::Matrix<double, 4, 6> T = -S22.inverse() * S12.transpose();
  const Eigen::Matrix<double, 6, 6> M = C.inverse() * (S11 + S12 * T);
  Eigen::EigenSolver<Eigen::Matrix<double, 6, 6>> eig(M);
  int best = -1;
  for (int i = 0; i < 6; ++i) {
    const double re = eig.eigenvalues()[i].real();
    if (std::abs(eig.eigenvalues()[i].imag()) < 1e-12 && re > 0 &&
        (best < 0 || re > eig.eigenvalues()[best].real())) {
      best = i;
    }
  }
  if (best < 0) return out;
  Eigen::Matrix<double, 6, 1> v1 = eig.eigenvectors().col(best).real();
  if (v1(0) < 0) v1 = -v1;
  const Eigen::Matrix<double, 4, 1> v2 = T * v1;

  // Quadric x^T Q x + 2 u^T x + d = 0 in the normalised coordinates.
  Eigen::Matrix3d Q;
  Q << v1(0), v1(5), v1(4),
       v1(5), v1(1), v1(3),
       v1(4), v1(3), v1(2);
  const Eigen::Vector3d u(v2(0), v2(1), v2(2));
  const double d = v2(3);
  const Eigen::Vector3d centre = -Q.inverse() * u;
  const double rhs = centre.dot(Q * centre) - d;
  const Eigen::Matrix3d shape = Q / rhs;  // (x - c)^T shape (x - c) = 1
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> sym(shape);
  if (sym.info() != Eigen::Success || sym.eigenvalues().minCoeff() <= 0) return out;

  // root = sqrt(shape) maps the normalised ellipsoid onto the unit sphere;
  // radius rescales it to the equal-volume sphere. With p - c =
  // (m - offset) / scale, W = radius * root gives |W (m - offset)| =
  // radius * scale for readings on the ellipsoid.
  const Eigen::Matrix3d root = sym.eigenvectors() *
                               sym.eigenvalues().cwiseSqrt().asDiagonal() *
                               sym.eigenvectors().transpose();
  const double radius = std::pow(sym.eigenvalues().prod(), -1.0 / 6.0);
  out.offset = mean + scale * centre;
  out.matrix = radius * root;
  out.field_norm = radius * scale;
  out.valid = true;

  double sq = 0.0;
  std::set<std::tuple<int, int, int>> bins;
  for (const auto& m : readings) {
    const Eigen::Vector3d c = out.apply(m);
    sq += std::pow(c.norm() - out.field_norm, 2);
    const Eigen::Vector3d dir = c.normalized();
    bins.insert({static_cast<int>(std::lround(dir.x() * 1.5)), static_cast<int>(std::lround(dir.y() * 1.5)),
                 static_cast<int>(std::lround(dir.z() * 1.5))});
  }
  bins.erase({0, 0, 0});
  out.rms_residual = std::sqrt(sq / readings.size());
  out.direction_bins = static_cast<int>(bins.size());
  return out;
}

}  // namespace attitude_estimation
}  // namespace localization_zoo
