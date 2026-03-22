#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <vector>

namespace localization_zoo {
namespace relead {

/// IMU計測値
struct ImuMeasurement {
  double timestamp = 0.0;
  Eigen::Vector3d angular_velocity = Eigen::Vector3d::Zero();
  Eigen::Vector3d linear_acceleration = Eigen::Vector3d::Zero();
};

/// 点群の点 (法線付き)
struct PointWithNormal {
  Eigen::Vector3d point = Eigen::Vector3d::Zero();     // マップ座標系
  Eigen::Vector3d normal = Eigen::Vector3d::Zero();     // 法線
  Eigen::Vector3d closest = Eigen::Vector3d::Zero();    // マップ上の最近傍点
};

/// 状態ベクトル: R, t, v, b_g, b_a, g  (18状態, エラーステートは18次元)
struct State {
  Eigen::Matrix3d rotation = Eigen::Matrix3d::Identity();
  Eigen::Vector3d position = Eigen::Vector3d::Zero();
  Eigen::Vector3d velocity = Eigen::Vector3d::Zero();
  Eigen::Vector3d bias_gyro = Eigen::Vector3d::Zero();
  Eigen::Vector3d bias_accel = Eigen::Vector3d::Zero();
  Eigen::Vector3d gravity = Eigen::Vector3d(0, 0, -9.81);

  static constexpr int kErrorStateDim = 18;  // δr(3)+δt(3)+δv(3)+δbg(3)+δba(3)+δg(3)
};

/// 各方向のローカライズ可能性カテゴリ
enum class Localizability {
  FULL,       // 完全にローカライズ可能
  PARTIAL,    // 部分的にローカライズ可能
  NONE        // ローカライズ不可 (退化)
};

/// 退化検知結果
struct DegeneracyInfo {
  /// 回転の退化方向 (V_r のうち退化と判定された列)
  std::vector<Eigen::Vector3d> degenerate_rotation_dirs;
  /// 並進の退化方向 (V_t のうち退化と判定された列)
  std::vector<Eigen::Vector3d> degenerate_translation_dirs;

  /// 各方向のローカライズ可能性
  std::array<Localizability, 3> rotation_localizability = {
      Localizability::FULL, Localizability::FULL, Localizability::FULL};
  std::array<Localizability, 3> translation_localizability = {
      Localizability::FULL, Localizability::FULL, Localizability::FULL};

  bool hasRotationDegeneracy() const {
    return !degenerate_rotation_dirs.empty();
  }
  bool hasTranslationDegeneracy() const {
    return !degenerate_translation_dirs.empty();
  }
  bool hasDegeneracy() const {
    return hasRotationDegeneracy() || hasTranslationDegeneracy();
  }
};

}  // namespace relead
}  // namespace localization_zoo
