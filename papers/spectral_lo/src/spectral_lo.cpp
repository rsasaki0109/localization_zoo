#include "spectral_lo/spectral_lo.h"

#include <algorithm>
#include <cmath>

namespace localization_zoo {
namespace spectral_lo {

namespace {

constexpr double kPi = 3.14159265358979323846;

double signedShift(double shift, int n) {
  if (shift > n / 2) shift -= n;
  return shift;
}

// 2D FFT (in-place, row-major n×n)。inverse=true で 1/n² 正規化。
void fft2d(std::vector<std::complex<double>>& g, int n, bool inverse) {
  std::vector<std::complex<double>> line(n);
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < n; c++) line[c] = g[static_cast<size_t>(r) * n + c];
    SpectralLoPipeline::fft1d(line, inverse);
    for (int c = 0; c < n; c++) g[static_cast<size_t>(r) * n + c] = line[c];
  }
  for (int c = 0; c < n; c++) {
    for (int r = 0; r < n; r++) line[r] = g[static_cast<size_t>(r) * n + c];
    SpectralLoPipeline::fft1d(line, inverse);
    for (int r = 0; r < n; r++) g[static_cast<size_t>(r) * n + c] = line[r];
  }
}

std::vector<std::complex<double>> fft2dForward(const Image& img) {
  const int n = img.n;
  std::vector<std::complex<double>> g(static_cast<size_t>(n) * n);
  for (size_t i = 0; i < g.size(); i++) g[i] = std::complex<double>(img.data[i], 0.0);
  fft2d(g, n, false);
  return g;
}

double bilinear(const Image& img, double y, double x) {
  const int n = img.n;
  if (x < 0.0 || y < 0.0 || x > n - 1 || y > n - 1) return 0.0;
  const int x0 = static_cast<int>(std::floor(x));
  const int y0 = static_cast<int>(std::floor(y));
  const int x1 = std::min(x0 + 1, n - 1);
  const int y1 = std::min(y0 + 1, n - 1);
  const double fx = x - x0, fy = y - y0;
  return img.at(y0, x0) * (1 - fx) * (1 - fy) + img.at(y0, x1) * fx * (1 - fy) +
         img.at(y1, x0) * (1 - fx) * fy + img.at(y1, x1) * fx * fy;
}

double parabolicOffset(double m1, double p0, double p1) {
  const double denom = (m1 - 2.0 * p0 + p1);
  if (std::abs(denom) < 1e-12) return 0.0;
  double d = 0.5 * (m1 - p1) / denom;
  return std::clamp(d, -1.0, 1.0);
}

// 位相相関: a と b の正規化相互パワースペクトルを逆 FFT し、ピーク位置 (符号付き
// 行/列シフト) と正規化相関ピーク値を返す。b が a を (dr, dc) だけ巡回シフトした
// ものなら、ピークは (dr, dc) に立つ。
Eigen::Vector2d phaseCorrelation(const Image& a, const Image& b,
                                 double* peak_val) {
  const int n = a.n;
  auto Fa = fft2dForward(a);
  auto Fb = fft2dForward(b);
  std::vector<std::complex<double>> R(static_cast<size_t>(n) * n);
  for (size_t i = 0; i < R.size(); i++) {
    std::complex<double> cross = Fa[i] * std::conj(Fb[i]);
    const double m = std::abs(cross);
    R[i] = (m > 1e-12) ? cross / m : std::complex<double>(0.0, 0.0);
  }
  fft2d(R, n, true);

  int pr = 0, pc = 0;
  double best = -1.0;
  for (int r = 0; r < n; r++)
    for (int c = 0; c < n; c++) {
      const double v = R[static_cast<size_t>(r) * n + c].real();
      if (v > best) { best = v; pr = r; pc = c; }
    }

  if (peak_val) *peak_val = best;

  auto val = [&](int r, int c) {
    r = (r % n + n) % n;
    c = (c % n + n) % n;
    return R[static_cast<size_t>(r) * n + c].real();
  };
  const double dr = parabolicOffset(val(pr - 1, pc), val(pr, pc), val(pr + 1, pc));
  const double dc = parabolicOffset(val(pr, pc - 1), val(pr, pc), val(pr, pc + 1));

  double sr = signedShift(pr + dr, n);
  double sc = signedShift(pc + dc, n);
  return Eigen::Vector2d(sr, sc);
}

// 並進不変な振幅スペクトルを fftshift + 高域強調して返す。
Image magnitudeSpectrum(const Image& img) {
  const int n = img.n;
  auto F = fft2dForward(img);
  Image mag;
  mag.n = n;
  mag.data.assign(static_cast<size_t>(n) * n, 0.0);
  const int half = n / 2;
  for (int r = 0; r < n; r++)
    for (int c = 0; c < n; c++) {
      // fftshift: DC を中心へ。
      const int sr = (r + half) % n;
      const int sc = (c + half) % n;
      double m = std::log(1.0 + std::abs(F[static_cast<size_t>(r) * n + c]));
      // Fourier-Mellin 高域強調フィルタ H=(1-X)(2-X), X=cos(πξ)cos(πη)。
      const double xi = static_cast<double>(c <= half ? c : c - n) / n;
      const double eta = static_cast<double>(r <= half ? r : r - n) / n;
      const double X = std::cos(kPi * xi) * std::cos(kPi * eta);
      const double H = (1.0 - X) * (2.0 - X);
      mag.at(sr, sc) = m * H;
    }
  return mag;
}

}  // namespace

// radix-2 Cooley-Tukey FFT (n は 2 の冪)。
void SpectralLoPipeline::fft1d(std::vector<std::complex<double>>& a, bool inverse) {
  const int n = static_cast<int>(a.size());
  if (n <= 1) return;
  for (int i = 1, j = 0; i < n; i++) {
    int bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) std::swap(a[i], a[j]);
  }
  for (int len = 2; len <= n; len <<= 1) {
    const double ang = 2.0 * kPi / len * (inverse ? 1.0 : -1.0);
    const std::complex<double> wlen(std::cos(ang), std::sin(ang));
    for (int i = 0; i < n; i += len) {
      std::complex<double> w(1.0, 0.0);
      for (int k = 0; k < len / 2; k++) {
        const std::complex<double> u = a[i + k];
        const std::complex<double> v = a[i + k + len / 2] * w;
        a[i + k] = u + v;
        a[i + k + len / 2] = u - v;
        w *= wlen;
      }
    }
  }
  if (inverse)
    for (auto& x : a) x /= static_cast<double>(n);
}

SpectralLoPipeline::SpectralLoPipeline(const SpectralLoParams& params)
    : params_(params) {}

Image SpectralLoPipeline::rasterizeBev(
    const std::vector<Eigen::Vector3d>& frame) const {
  const int n = params_.bev_size;
  Image img;
  img.n = n;
  img.data.assign(static_cast<size_t>(n) * n, 0.0);
  const double cell = 2.0 * params_.bev_range / n;
  const int center = n / 2;
  for (const auto& p : frame) {
    if (p.z() < params_.z_min || p.z() > params_.z_max) continue;
    const double r = std::hypot(p.x(), p.y());
    if (r < params_.min_range || r > params_.max_range) continue;
    const int c = static_cast<int>(std::lround(p.x() / cell)) + center;
    const int rr = static_cast<int>(std::lround(p.y() / cell)) + center;
    if (c < 0 || c >= n || rr < 0 || rr >= n) continue;
    img.at(rr, c) += 1.0;  // 密度 (点数) BEV
  }
  // 2D Hann 窓でスペクトル漏れを抑制。
  for (int r = 0; r < n; r++) {
    const double wr = 0.5 * (1.0 - std::cos(2.0 * kPi * r / (n - 1)));
    for (int c = 0; c < n; c++) {
      const double wc = 0.5 * (1.0 - std::cos(2.0 * kPi * c / (n - 1)));
      img.at(r, c) *= wr * wc;
    }
  }
  return img;
}

double SpectralLoPipeline::estimateYaw(const Image& ref, const Image& cur) const {
  const Image mref = magnitudeSpectrum(ref);
  const Image mcur = magnitudeSpectrum(cur);

  const int A = params_.logpolar_angles;
  const int R = params_.logpolar_radii;
  const int n = ref.n;
  const double center = n / 2.0;
  const double rmin = 1.0;
  const double rmax = n / 2.0 - 1.0;
  const double log_step = (std::log(rmax) - std::log(rmin)) / R;

  Image lpref, lpcur;
  lpref.n = A;  // 行=角度, 列=動径 (正方 A=R を要求)
  lpcur.n = A;
  lpref.data.assign(static_cast<size_t>(A) * R, 0.0);
  lpcur.data.assign(static_cast<size_t>(A) * R, 0.0);
  for (int a = 0; a < A; a++) {
    const double theta = a * kPi / A;  // 振幅は点対称なので [0, π)
    const double ct = std::cos(theta), st = std::sin(theta);
    for (int ri = 0; ri < R; ri++) {
      const double rad = std::exp(std::log(rmin) + ri * log_step);
      const double x = center + rad * ct;
      const double y = center + rad * st;
      lpref.data[static_cast<size_t>(a) * R + ri] = bilinear(mref, y, x);
      lpcur.data[static_cast<size_t>(a) * R + ri] = bilinear(mcur, y, x);
    }
  }

  double peak = 0.0;
  const Eigen::Vector2d sh = phaseCorrelation(lpref, lpcur, &peak);
  double yaw = sh(0) * (kPi / A);  // 角度ビンシフト → ラジアン
  if (yaw > kPi / 2) yaw -= kPi;
  if (yaw < -kPi / 2) yaw += kPi;
  const double max_yaw = params_.max_yaw_deg * kPi / 180.0;
  if (std::abs(yaw) > max_yaw) yaw = 0.0;  // 過大な回転ピークは棄却
  return yaw;
}

Eigen::Vector2d SpectralLoPipeline::estimateTranslation(const Image& ref,
                                                        const Image& cur,
                                                        double* peak) const {
  const double cell = 2.0 * params_.bev_range / params_.bev_size;
  const Eigen::Vector2d sh = phaseCorrelation(ref, cur, peak);
  // sh = (行=y シフト, 列=x シフト)。位相相関ピークは ref→cur の BEV シフトを表し、
  // それが ref フレームでのセンサ並進に対応する。並進 = sh*cell。
  const double tx = sh(1) * cell;
  const double ty = sh(0) * cell;
  return Eigen::Vector2d(tx, ty);
}

SpectralLoResult SpectralLoPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  SpectralLoResult result;
  Image bev = rasterizeBev(frame);

  if (!has_ref_) {
    ref_bev_ = bev;
    ref_pose_ = pose_;
    has_ref_ = true;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  const double yaw = estimateYaw(ref_bev_, bev);

  // 推定 yaw で current 点群を reference 向きへ回してから並進を推定
  // (画像補間でなく点群回転)。T_rel は cur->ref なので +yaw を使う。
  const double cz = std::cos(yaw), sz = std::sin(yaw);
  std::vector<Eigen::Vector3d> derot(frame.size());
  for (size_t i = 0; i < frame.size(); i++) {
    const auto& p = frame[i];
    derot[i] = Eigen::Vector3d(cz * p.x() - sz * p.y(), sz * p.x() + cz * p.y(), p.z());
  }
  const Image bev_rot = rasterizeBev(derot);
  double tpeak = 0.0;
  Eigen::Vector2d t =
      estimateTranslation(ref_bev_, bev_rot, &tpeak);
  for (int iter = 0; iter < params_.translation_refinement_iterations; ++iter) {
    std::vector<Eigen::Vector3d> aligned(frame.size());
    for (size_t i = 0; i < frame.size(); i++) {
      const auto& p = frame[i];
      aligned[i] = Eigen::Vector3d(cz * p.x() - sz * p.y() + t.x(),
                                   sz * p.x() + cz * p.y() + t.y(), p.z());
    }
    const Image bev_aligned = rasterizeBev(aligned);
    double residual_peak = 0.0;
    const Eigen::Vector2d residual =
        estimateTranslation(ref_bev_, bev_aligned, &residual_peak);
    if (!residual.allFinite() ||
        residual.norm() > params_.max_translation_refinement_m) {
      break;
    }
    t += residual;
    tpeak = residual_peak;
  }

  // ref フレームから見た相対変換 T_rel (cur 座標 → ref 座標)。
  Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
  const double cy = std::cos(yaw), sy = std::sin(yaw);
  T_rel(0, 0) = cy; T_rel(0, 1) = -sy;
  T_rel(1, 0) = sy; T_rel(1, 1) = cy;
  T_rel(0, 3) = t.x();
  T_rel(1, 3) = t.y();

  pose_ = ref_pose_ * T_rel;

  result.pose = pose_;
  result.yaw_rad = yaw;
  result.translation_peak = tpeak;
  result.converged = pose_.array().isFinite().all();

  // frame-to-frame (既定) では毎フレーム ref を更新。keyframe_translation>0 なら
  // 並進が閾値を超えたときのみ更新 (frame-to-keyframe)。
  const double moved = T_rel.block<3, 1>(0, 3).norm();
  if (params_.keyframe_translation <= 0.0 || moved >= params_.keyframe_translation) {
    ref_bev_ = bev;
    ref_pose_ = pose_;
  }
  return result;
}

}  // namespace spectral_lo
}  // namespace localization_zoo
