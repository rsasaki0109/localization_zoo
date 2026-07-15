#pragma once

#include <ceres/ceres.h>

namespace localization_zoo {

/// Set the quaternion manifold using the API available in the detected Ceres major version.
inline void SetEigenQuaternionManifold(ceres::Problem& problem, double* q) {
#if CERES_VERSION_MAJOR >= 2
  problem.SetManifold(q, new ceres::EigenQuaternionManifold());
#else
  problem.SetParameterization(q, new ceres::EigenQuaternionParameterization());
#endif
}

}  // namespace localization_zoo
