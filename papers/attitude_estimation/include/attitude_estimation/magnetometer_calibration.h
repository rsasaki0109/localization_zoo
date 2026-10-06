#pragma once

// Magnetometer hard/soft-iron calibration by ellipsoid fitting.
//
// Model: a reading is m = A h + b for the local field h (constant norm), a
// soft-iron matrix A, and a hard-iron offset b, so readings from a full
// rotation lie on an ellipsoid. The fit is the ellipsoid-specific algebraic
// least squares of Q. Li and J. G. Griffiths, "Least squares ellipsoid
// specific fitting", Geometric Modeling and Processing 2004 (constraint
// 4J - I^2 = 1, which admits only ellipsoids). The correction is
// m_cal = W (m - b) with W the symmetric square root of the fitted shape,
// scaled so the calibrated readings have the mean fitted radius.

#include <Eigen/Core>

#include <vector>

namespace localization_zoo {
namespace attitude_estimation {

struct MagnetometerCalibration {
  bool valid = false;
  Eigen::Vector3d offset = Eigen::Vector3d::Zero();      // hard iron b
  Eigen::Matrix3d matrix = Eigen::Matrix3d::Identity();  // soft-iron correction W
  double field_norm = 0.0;     // radius of the calibrated sphere
  double rms_residual = 0.0;   // RMS of |m_cal| - field_norm
  int direction_bins = 0;      // of 26 sphere directions visited (coverage)

  Eigen::Vector3d apply(const Eigen::Vector3d& m) const { return matrix * (m - offset); }
};

/// Fit hard and soft iron from readings of a full rotation (at least 10, but
/// cover as many directions as possible). valid is false when the fit is not
/// an ellipsoid (e.g. the readings span only a plane).
MagnetometerCalibration fitMagnetometerCalibration(const std::vector<Eigen::Vector3d>& readings);

}  // namespace attitude_estimation
}  // namespace localization_zoo
