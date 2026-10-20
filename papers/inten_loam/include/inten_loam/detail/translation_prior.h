#pragma once

#include <Eigen/Dense>
#include <algorithm>

namespace localization_zoo {
namespace inten_loam {
namespace detail {

inline Eigen::Matrix3d crossMatrix(const Eigen::Vector3d& t) {
  Eigen::Matrix3d K;
  K << 0, -t.z(), t.y(), t.z(), 0, -t.x(), -t.y(), t.x(), 0;
  return K;
}

// Extra quadratic information only in weak translation directions, after
// marginalizing rotation at the sensor origin. Translation and rotation are
// never compared as eigenvalues of a mixed-unit 6 x 6 matrix.
inline Eigen::Matrix3d translationPriorInformation(
    const Eigen::Matrix<double, 6, 6>& H, const Eigen::Vector3d& t, double ratio) {
  Eigen::Matrix<double, 6, 6> B = Eigen::Matrix<double, 6, 6>::Identity();
  B.block<3, 3>(0, 3) = crossMatrix(t);
  const Eigen::Matrix<double, 6, 6> centered = B.transpose() * H * B;
  const Eigen::Matrix3d rotation = centered.block<3, 3>(3, 3) + 1e-6 * Eigen::Matrix3d::Identity();
  const Eigen::Matrix3d S = centered.block<3, 3>(0, 0) -
      centered.block<3, 3>(0, 3) * rotation.ldlt().solve(centered.block<3, 3>(3, 0));
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eigen((S + S.transpose()) * 0.5);
  if (eigen.info() != Eigen::Success || !eigen.eigenvalues().allFinite())
    return Eigen::Matrix3d::Zero();
  const double floor = ratio * std::max(0.0, eigen.eigenvalues().maxCoeff());
  const Eigen::Vector3d extra = (Eigen::Vector3d::Constant(floor) - eigen.eigenvalues()).cwiseMax(0.0);
  return eigen.eigenvectors() * extra.asDiagonal() * eigen.eigenvectors().transpose();
}

// Prior on the actual sensor position in the target frame, anchored at the
// initial prediction for this solve. For a left increment, dt/d[v,w]=[I,-[t]x].
inline double addTranslationPrior(const Eigen::Matrix3d& information,
                                 const Eigen::Vector3d& prediction,
                                 const Eigen::Vector3d& t,
                                 Eigen::Matrix<double, 6, 6>* H,
                                 Eigen::Matrix<double, 6, 1>* g) {
  Eigen::Matrix<double, 3, 6> J;
  J.leftCols<3>().setIdentity();
  J.rightCols<3>() = -crossMatrix(t);
  const Eigen::Vector3d residual = t - prediction;
  *H += J.transpose() * information * J;
  *g += J.transpose() * information * residual;
  return residual.dot(information * residual);
}

}  // namespace detail
}  // namespace inten_loam
}  // namespace localization_zoo
