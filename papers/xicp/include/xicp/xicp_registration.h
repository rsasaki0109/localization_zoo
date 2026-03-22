#pragma once

#include "xicp/types.h"

#include <Eigen/Core>
#include <vector>

namespace localization_zoo {
namespace xicp {

struct XICPParams {
  int max_iterations = 30;
  double convergence_threshold = 1e-5;

  // ローカライズ可能性分類パラメータ
  double kappa_f = 0.1736;     // cos(80°), 低寄与フィルタ閾値
  double kappa_s = 0.707;      // cos(45°), 強寄与フィルタ閾値
  double kappa_1 = 250.0;      // full判定閾値
  double kappa_2 = 180.0;      // partial判定閾値
  double kappa_3 = 35.0;       // minimum情報閾値
};

struct XICPResult {
  Eigen::Matrix4d transformation = Eigen::Matrix4d::Identity();
  LocalizabilityInfo localizability;
  int num_iterations = 0;
  bool converged = false;
};

/// X-ICP: Localizability-Aware Point Cloud Registration
class XICPRegistration {
public:
  explicit XICPRegistration(const XICPParams& params = XICPParams())
      : params_(params) {}

  /// レジストレーション実行
  /// @param correspondences 対応関係 (source=センサ, target=マップ, normal)
  /// @param initial_guess 初期変換
  XICPResult align(const std::vector<Correspondence>& correspondences,
                   const Eigen::Matrix4d& initial_guess =
                       Eigen::Matrix4d::Identity());

private:
  /// ローカライズ可能性解析
  LocalizabilityInfo analyzeLocalizability(
      const std::vector<Correspondence>& correspondences,
      const Eigen::Matrix3d& R, const Eigen::Vector3d& t) const;

  /// 制約なしICP更新 (6x6正規方程式)
  Eigen::Matrix<double, 6, 1> solveUnconstrained(
      const std::vector<Correspondence>& correspondences,
      const Eigen::Matrix3d& R, const Eigen::Vector3d& t,
      Eigen::Matrix<double, 6, 6>& H, Eigen::Matrix<double, 6, 1>& b) const;

  /// 制約付きICP更新 (Lagrange乗数法)
  Eigen::Matrix<double, 6, 1> solveConstrained(
      const Eigen::Matrix<double, 6, 6>& H,
      const Eigen::Matrix<double, 6, 1>& b,
      const LocalizabilityInfo& loc_info) const;

  XICPParams params_;
};

}  // namespace xicp
}  // namespace localization_zoo
