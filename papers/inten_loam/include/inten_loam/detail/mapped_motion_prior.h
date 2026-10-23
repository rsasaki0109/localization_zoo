#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU>

namespace localization_zoo {
namespace inten_loam {
namespace detail {

// Used only by the opt-in recursive mapped-motion seed. Matrix products of
// finite near-rigid poses carry roundoff outside SO(3); rigid left updates
// cannot remove that component. Keep the translation and restore a proper
// rotation before feeding this motion back into S2S. No fitted tolerance.
inline Eigen::Matrix4d mappedMotionPrior(const Eigen::Matrix4d& previous,
                                       const Eigen::Matrix4d& current) {
  Eigen::Matrix4d relative = previous.inverse() * current;
  relative.block<3, 3>(0, 0) =
      Eigen::Quaterniond(Eigen::Matrix3d(relative.block<3, 3>(0, 0))).normalized().toRotationMatrix();
  relative.row(3) << 0.0, 0.0, 0.0, 1.0;
  return relative;
}

}  // namespace detail
}  // namespace inten_loam
}  // namespace localization_zoo
