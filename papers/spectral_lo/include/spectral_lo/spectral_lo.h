#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <complex>
#include <cstddef>
#include <vector>

namespace localization_zoo {
namespace spectral_lo {

/// 行優先で格納する正方 (N×N, N は 2 の冪) 実数画像。
struct Image {
  int n = 0;
  std::vector<double> data;  // size n*n, row-major
  double& at(int r, int c) { return data[static_cast<size_t>(r) * n + c]; }
  double at(int r, int c) const { return data[static_cast<size_t>(r) * n + c]; }
};

struct SpectralLoParams {
  /// BEV 画像サイズ (2 の冪)。
  int bev_size = 256;
  /// BEV の半径 [m] (中心からの片側スパン)。セル = 2*bev_range/bev_size。
  double bev_range = 40.0;
  double max_range = 80.0;
  double min_range = 1.0;
  /// この高さ範囲 [m] の点のみ BEV に投影 (地面/天井ノイズの抑制)。
  double z_min = -3.0;
  double z_max = 3.0;

  // --- Fourier-Mellin 回転推定 ---
  /// log-polar リサンプルの角度ビン数。
  int logpolar_angles = 256;
  /// log-polar リサンプルの動径ビン数。
  int logpolar_radii = 256;
  /// フレーム間 yaw の探索上限 [deg]。これを超える回転ピークは棄却 (誤マッチ防止)。
  double max_yaw_deg = 30.0;

  /// frame-to-keyframe にするキーフレーム並進しきい値 [m] (0=毎フレーム frame-to-frame)。
  double keyframe_translation = 0.0;
};

struct SpectralLoResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  /// このフレームで推定された相対 yaw [rad]。
  double yaw_rad = 0.0;
  /// 並進位相相関ピークの鋭さ (正規化相関値, 1 に近いほど信頼度高)。
  double translation_peak = 0.0;
};

/// Spectral-LO: 周波数領域 (位相相関) LiDAR オドメトリ。
///
///  SROM "Simple Real-time Odometry and Mapping" (Nagy & Benedek 系の
///  phase-only-correlation 手法, arXiv:2005.02042) の spectral マッチング機構を
///  論文記述から独自再構成したもの。著者コード未公開。
///
///  ICP / 分布ファクタを一切使わず、BEV 画像同士を **周波数領域** で位置合わせする:
///   1. スキャンを BEV (鳥瞰) 密度画像にラスタ化。
///   2. **Fourier-Mellin**: |FFT| 振幅スペクトル (並進不変) を log-polar 座標に
///      リサンプルし、その位相相関ピークの角度成分から相対 yaw を得る。
///   3. 推定 yaw で現在 BEV を de-rotate。
///   4. **Phase-Only Correlation (POC)**: 参照 BEV と de-rotate 後 BEV の
///      正規化相互パワースペクトルを逆 FFT し、ピーク位置から並進 (dx, dy) を得る。
///  地上車両 (KITTI) の 3-DoF (x, y, yaw) を仮定し、z/roll/pitch は据え置く
///  (BEV が z を捨てるため z drift は honest な限界)。
class SpectralLoPipeline {
public:
  explicit SpectralLoPipeline(const SpectralLoParams& params = SpectralLoParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  SpectralLoResult registerFrame(const std::vector<Eigen::Vector3d>& frame);
  const Eigen::Matrix4d& pose() const { return pose_; }

  /// 1 次元 FFT (in-place, radix-2 Cooley-Tukey)。テスト用に公開。
  static void fft1d(std::vector<std::complex<double>>& a, bool inverse);

private:
  Image rasterizeBev(const std::vector<Eigen::Vector3d>& frame) const;
  double estimateYaw(const Image& ref, const Image& cur) const;
  Eigen::Vector2d estimateTranslation(const Image& ref, const Image& cur,
                                       double* peak) const;

  SpectralLoParams params_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Image ref_bev_;
  Eigen::Matrix4d ref_pose_ = Eigen::Matrix4d::Identity();
  bool has_ref_ = false;
};

}  // namespace spectral_lo
}  // namespace localization_zoo
