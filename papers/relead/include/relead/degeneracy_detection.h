#pragma once

#include "relead/types.h"

#include <Eigen/Core>
#include <vector>

namespace localization_zoo {
namespace relead {

struct DegeneracyDetectionParams {
  /// ローカライズ可能性の閾値
  /// 固有値比がこれ以下なら NONE (退化)
  double none_threshold = 0.01;
  /// 固有値比がこれ以下なら PARTIAL
  double partial_threshold = 0.1;
};

/// 退化検知モジュール
///
/// Point-to-plane残差のヘシアン行列をSVDで分解し、
/// 各方向のローカライズ可能性を判定する
class DegeneracyDetection {
public:
  explicit DegeneracyDetection(
      const DegeneracyDetectionParams& params = DegeneracyDetectionParams())
      : params_(params) {}

  /// Point-to-plane対応関係からヘシアンを構築し退化を検知
  /// @param correspondences (変換済み点, 法線) のペア
  /// @return 退化検知結果
  DegeneracyInfo detect(
      const std::vector<PointWithNormal>& correspondences) const;

private:
  DegeneracyDetectionParams params_;
};

}  // namespace relead
}  // namespace localization_zoo
