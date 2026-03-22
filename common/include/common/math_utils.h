#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace localization_zoo {
namespace math {

/// 回転行列からロール・ピッチ・ヨーを取得
Eigen::Vector3d toRPY(const Eigen::Matrix3d& rotation);

/// ロール・ピッチ・ヨーから回転行列を生成
Eigen::Matrix3d fromRPY(double roll, double pitch, double yaw);

/// スキューシンメトリック行列
Eigen::Matrix3d skew(const Eigen::Vector3d& v);

/// 角度を [-pi, pi] に正規化
double normalizeAngle(double angle);

}  // namespace math
}  // namespace localization_zoo
