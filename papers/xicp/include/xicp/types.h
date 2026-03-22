#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>
#include <array>

namespace localization_zoo {
namespace xicp {

/// Point-to-plane対応関係
struct Correspondence {
  Eigen::Vector3d source;   // ソース点 (センサ座標系)
  Eigen::Vector3d target;   // ターゲット点 (マップ座標系)
  Eigen::Vector3d normal;   // ターゲット法線
};

/// ローカライズ可能性カテゴリ
enum class Localizability { FULL, PARTIAL, NONE };

/// 各方向のローカライズ可能性情報
struct LocalizabilityInfo {
  /// 回転/並進それぞれ3方向のカテゴリ
  std::array<Localizability, 3> rotation = {
      Localizability::FULL, Localizability::FULL, Localizability::FULL};
  std::array<Localizability, 3> translation = {
      Localizability::FULL, Localizability::FULL, Localizability::FULL};

  /// 固有ベクトル (主方向)
  Eigen::Matrix3d V_r = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d V_t = Eigen::Matrix3d::Identity();

  /// 固有値
  Eigen::Vector3d sigma_r = Eigen::Vector3d::Ones();
  Eigen::Vector3d sigma_t = Eigen::Vector3d::Ones();

  /// ローカライズ可能性スコア (集計値)
  Eigen::Vector3d L_c_rot = Eigen::Vector3d::Zero();
  Eigen::Vector3d L_c_trans = Eigen::Vector3d::Zero();
  Eigen::Vector3d L_s_rot = Eigen::Vector3d::Zero();
  Eigen::Vector3d L_s_trans = Eigen::Vector3d::Zero();

  /// partial方向の制約値
  Eigen::Vector3d r0 = Eigen::Vector3d::Zero();
  Eigen::Vector3d t0 = Eigen::Vector3d::Zero();

  bool hasDegeneracy() const {
    for (int i = 0; i < 3; i++) {
      if (rotation[i] != Localizability::FULL) return true;
      if (translation[i] != Localizability::FULL) return true;
    }
    return false;
  }

  int numDegenerateDirections() const {
    int count = 0;
    for (int i = 0; i < 3; i++) {
      if (rotation[i] != Localizability::FULL) count++;
      if (translation[i] != Localizability::FULL) count++;
    }
    return count;
  }
};

}  // namespace xicp
}  // namespace localization_zoo
