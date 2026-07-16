#pragma once

#include <ceres/ceres.h>

namespace localization_zoo {

/// Set the quaternion manifold using the API available in the detected Ceres version.
/// SetManifold() was introduced in Ceres 2.1; Ceres 2.0.x (e.g. Ubuntu 22.04's
/// libceres-dev) is major version 2 but still only has SetParameterization().
inline void SetEigenQuaternionManifold(ceres::Problem& problem, double* q) {
#if CERES_VERSION_MAJOR > 2 || (CERES_VERSION_MAJOR == 2 && CERES_VERSION_MINOR >= 1)
  problem.SetManifold(q, new ceres::EigenQuaternionManifold());
#else
  problem.SetParameterization(q, new ceres::EigenQuaternionParameterization());
#endif
}

}  // namespace localization_zoo
