#pragma once

#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#include <cmath>

namespace localization_zoo {
namespace math {

// Remove translation updates along eigenvectors whose eigenvalue is strictly
// below ratio * largest. The symmetric translation normal-equation block and
// update must use the same coordinates. The caller chooses the translation
// block, independently of its six-vector's rotation/translation ordering.
//
// This is the existing DiLO opt-in remap, not a paper/default correction. It
// projects the local solver update, not the accumulated SE(3) pose; rotation
// and its coupling through pose composition can still change translation.
// Non-positive/non-finite ratios and invalid inputs leave the update untouched.
// A zero normal matrix retains the legacy no-op behaviour. Returns the number
// of projected directions, for callers that need diagnostics.
inline int remapDegenerateTranslation(const Eigen::Matrix3d& normal, double ratio,
                                      Eigen::Ref<Eigen::Vector3d> update) {
  if (!(ratio > 0.0) || !std::isfinite(ratio) || !normal.allFinite() || !update.allFinite())
    return 0;
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eig(normal);
  if (eig.info() != Eigen::Success) return 0;
  const Eigen::Vector3d lambda = eig.eigenvalues();  // ascending
  int removed = 0;
  for (int k = 0; k < 3; ++k) {
    if (lambda(k) >= ratio * lambda(2)) break;
    const Eigen::Vector3d v = eig.eigenvectors().col(k);
    update -= v.dot(update) * v;
    ++removed;
  }
  return removed;
}

}  // namespace math
}  // namespace localization_zoo
