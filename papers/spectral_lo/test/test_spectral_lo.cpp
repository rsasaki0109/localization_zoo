#include "spectral_lo/spectral_lo.h"

#include <gtest/gtest.h>

#include <cmath>
#include <complex>
#include <vector>

using namespace localization_zoo::spectral_lo;

namespace {

// 非対称な構造シーン (壁 + 対角線 + 散布)。回転/並進の周波数信号を持たせる。
std::vector<Eigen::Vector3d> makeScene() {
  std::vector<Eigen::Vector3d> pts;
  for (double a = -30; a <= 30; a += 0.3)
    for (double z = -1.0; z <= 1.0; z += 0.5) pts.emplace_back(a, 12.0, z);  // 壁A (x方向)
  for (double b = -20; b <= 20; b += 0.3)
    for (double z = -1.0; z <= 1.0; z += 0.5) pts.emplace_back(-18.0, b, z);  // 壁B (y方向)
  for (double d = -15; d <= 15; d += 0.3)
    pts.emplace_back(d, d + 5.0, 0.0);  // 対角線
  // 決定的な散布 (LCG)。
  unsigned int s = 12345u;
  auto rnd = [&s]() { s = s * 1103515245u + 12345u; return ((s >> 16) & 0x7fff) / 32768.0; };
  for (int i = 0; i < 1500; i++)
    pts.emplace_back(rnd() * 60.0 - 30.0, rnd() * 60.0 - 30.0, rnd() * 2.0 - 1.0);
  return pts;
}

std::vector<Eigen::Vector3d> transformAll(const std::vector<Eigen::Vector3d>& pts,
                                          const Eigen::Matrix4d& T) {
  std::vector<Eigen::Vector3d> out(pts.size());
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  for (size_t i = 0; i < pts.size(); i++) out[i] = R * pts[i] + t;
  return out;
}

SpectralLoParams testParams() {
  SpectralLoParams p;
  p.bev_size = 128;
  p.logpolar_angles = 128;
  p.logpolar_radii = 128;
  p.bev_range = 40.0;
  p.max_range = 40.0;
  return p;
}

}  // namespace

// (1) FFT の正当性: 既知 DFT と往復復元。
TEST(SpectralLo, FftCorrectness) {
  std::vector<std::complex<double>> a = {1, 1, 1, 1};
  SpectralLoPipeline::fft1d(a, false);
  EXPECT_NEAR(a[0].real(), 4.0, 1e-9);
  for (int i = 1; i < 4; i++) EXPECT_NEAR(std::abs(a[i]), 0.0, 1e-9);

  std::vector<std::complex<double>> b = {0.3, -1.2, 2.5, 0.7, -0.4, 1.1, 0.0, -2.0};
  auto orig = b;
  SpectralLoPipeline::fft1d(b, false);
  SpectralLoPipeline::fft1d(b, true);
  for (size_t i = 0; i < b.size(); i++) EXPECT_NEAR(b[i].real(), orig[i].real(), 1e-9);
}

// (2) 純並進をフレーム間位相相関で回復する。
TEST(SpectralLo, RecoversTranslation) {
  SpectralLoPipeline pipe(testParams());
  const auto scene = makeScene();
  pipe.registerFrame(scene);  // 基準フレーム

  Eigen::Matrix4d m = Eigen::Matrix4d::Identity();
  m(0, 3) = 3.0;  // センサ前進 3 m
  const auto res = pipe.registerFrame(transformAll(scene, m.inverse()));

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 3.0, 1.0);
  EXPECT_NEAR(res.pose(1, 3), 0.0, 1.0);
}

// (3) 純ヨー回転を Fourier-Mellin で回復する。
TEST(SpectralLo, RecoversYaw) {
  SpectralLoPipeline pipe(testParams());
  const auto scene = makeScene();
  pipe.registerFrame(scene);

  const double yaw = 6.0 * M_PI / 180.0;  // 6 deg
  Eigen::Matrix4d m = Eigen::Matrix4d::Identity();
  m.block<3, 3>(0, 0) << std::cos(yaw), -std::sin(yaw), 0, std::sin(yaw),
      std::cos(yaw), 0, 0, 0, 1;
  const auto res = pipe.registerFrame(transformAll(scene, m.inverse()));

  const double recovered = std::atan2(res.pose(1, 0), res.pose(0, 0));
  EXPECT_NEAR(recovered, yaw, 2.5 * M_PI / 180.0);
}

// (4) 実走に近い「前進 + yaw」を同時に含む motion で、de-rotation の符号を固定する。
TEST(SpectralLo, RecoversForwardTranslationWithYaw) {
  SpectralLoPipeline pipe(testParams());
  const auto scene = makeScene();
  pipe.registerFrame(scene);

  const double yaw = 3.0 * M_PI / 180.0;
  Eigen::Matrix4d m = Eigen::Matrix4d::Identity();
  m.block<3, 3>(0, 0) << std::cos(yaw), -std::sin(yaw), 0, std::sin(yaw),
      std::cos(yaw), 0, 0, 0, 1;
  m(0, 3) = 3.0;
  const auto res = pipe.registerFrame(transformAll(scene, m.inverse()));

  const double recovered_yaw = std::atan2(res.pose(1, 0), res.pose(0, 0));
  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(recovered_yaw, yaw, 2.5 * M_PI / 180.0);
  EXPECT_NEAR(res.pose(0, 3), 3.0, 1.0);
  EXPECT_NEAR(res.pose(1, 3), 0.0, 0.75);
}
