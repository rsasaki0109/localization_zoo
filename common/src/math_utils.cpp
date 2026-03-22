#include "common/math_utils.h"

#include <cmath>

namespace localization_zoo {
namespace math {

Eigen::Vector3d toRPY(const Eigen::Matrix3d& rotation) {
  return rotation.eulerAngles(0, 1, 2);
}

Eigen::Matrix3d fromRPY(double roll, double pitch, double yaw) {
  Eigen::AngleAxisd r(roll, Eigen::Vector3d::UnitX());
  Eigen::AngleAxisd p(pitch, Eigen::Vector3d::UnitY());
  Eigen::AngleAxisd y(yaw, Eigen::Vector3d::UnitZ());
  return (y * p * r).toRotationMatrix();
}

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0, -v.z(), v.y(),
       v.z(), 0, -v.x(),
       -v.y(), v.x(), 0;
  return m;
}

double normalizeAngle(double angle) {
  while (angle > M_PI) angle -= 2.0 * M_PI;
  while (angle < -M_PI) angle += 2.0 * M_PI;
  return angle;
}

}  // namespace math
}  // namespace localization_zoo
