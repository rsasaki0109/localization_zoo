#pragma once

#include "aloam/types.h"

namespace localization_zoo {
namespace aloam {

struct ScanRegistrationParams {
  int n_scans = 16;              // スキャンライン数 (16, 32, 64)
  float minimum_range = 0.3f;    // 最小距離 [m]
  float scan_period = 0.1f;      // スキャン周期 [s]
  float curvature_threshold = 0.1f;  // エッジ判定の曲率閾値
  int max_corner_sharp = 2;      // サブリージョンあたりのsharpエッジ数
  int max_corner_less_sharp = 20;
  int max_surf_flat = 4;         // サブリージョンあたりのflat平面数
  float less_flat_leaf_size = 0.2f;  // LessFlat点のダウンサンプリング [m]
};

/// LiDAR点群からエッジ・平面特徴を抽出
class ScanRegistration {
public:
  explicit ScanRegistration(const ScanRegistrationParams& params =
                                ScanRegistrationParams())
      : params_(params) {}

  /// 特徴抽出を実行
  /// @param cloud 入力点群 (PointXYZI, intensity未使用)
  /// @return 特徴点群
  FeatureCloud extract(const PointCloudConstPtr& cloud);

private:
  /// 各点のスキャンラインIDを計算
  int computeScanId(float x, float y, float z) const;

  ScanRegistrationParams params_;
};

}  // namespace aloam
}  // namespace localization_zoo
