#pragma once

#include <ceres/ceres.h>

namespace localization_zoo {

/// Set quaternion manifold on a Ceres problem, compatible with both Ceres 2.1 and 2.2+.
inline void SetEigenQuaternionManifold(ceres::Problem& problem, double* q) {
#if CERES_VERSION_MAJOR > 2 || (CERES_VERSION_MAJOR == 2 && CERES_VERSION_MINOR >= 2)
  problem.SetManifold(q, new ceres::EigenQuaternionManifold());
#else
  problem.SetParameterization(q, new ceres::EigenQuaternionParameterization());
#endif
}

}  // namespace localization_zoo
