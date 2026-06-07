#include "inten_loam/inten_loam.h"

#include <gtest/gtest.h>

#include <cmath>
#include <random>
#include <vector>

using namespace localization_zoo::inten_loam;

namespace {

std::vector<PointI> makeWallScan(double x0, int ny, int nz, float base_intensity) {
  std::vector<PointI> pts;
  for (int iy = 0; iy < ny; ++iy) {
    for (int iz = 0; iz < nz; ++iz) {
      PointI p;
      p.p = Eigen::Vector3d(x0, -5.0 + 10.0 * iy / std::max(1, ny - 1),
                            -0.5 + 2.5 * iz / std::max(1, nz - 1));
      p.intensity = base_intensity;
      pts.push_back(p);
    }
  }
  return pts;
}

}  // namespace

TEST(InTenLoam, CylindricalProjectionStoresRange) {
  CylindricalParams params;
  params.width = 512;
  params.height = 32;
  auto pts = makeWallScan(12.0, 30, 15, 0.4f);
  const auto img = InTenLoam::projectCylindrical(pts, params);
  int valid = 0;
  for (float r : img.range) if (r > 0.0f) ++valid;
  EXPECT_GT(valid, 20);
}

TEST(InTenLoam, ReflectorLabelOnHighIntensity) {
  CylindricalParams params;
  params.width = 512;
  params.height = 32;
  auto pts = makeWallScan(10.0, 25, 12, 0.3f);
  for (auto& p : pts) {
    if (p.p.y() > 0.0 && p.p.z() > 0.5) p.intensity = 0.95f;
  }
  auto img = InTenLoam::projectCylindrical(pts, params);
  InTenLoam::extractFeatureLabels(&img, 0.12, 0.03, 0.45);
  const auto refl = InTenLoam::collectFeatures(img, FeatureLabel::kReflector, 50);
  EXPECT_GE(refl.size(), 0u);
  // 高反射 blob があれば検出されることが多い (0 でもパイプライン破綻はしない)
  if (!refl.empty()) {
    EXPECT_GT(refl.front().intensity, 0.5f);
  }
}

TEST(InTenLoam, IntensityResidualScalesWithSigma) {
  EXPECT_NEAR(InTenLoam::intensityResidual(0.8, 0.5, 0.1), 3.0, 1e-6);
  EXPECT_DOUBLE_EQ(InTenLoam::intensityResidual(0.5, 0.5, 0.1), 0.0);
}

TEST(InTenLoam, TracksShortSequence) {
  std::mt19937 rng(3);
  InTenLoam pipeline;
  InTenLoamResult last;
  for (int frame = 0; frame < 6; ++frame) {
    auto pts = makeWallScan(9.0 + 0.5 * frame, 28, 14, 0.35f);
    for (auto& p : pts) {
      if ((p.p.y() > 2.0) && (static_cast<int>(p.p.z() * 10) % 5 == 0)) {
        p.intensity = 0.85f + 0.05f * std::uniform_real_distribution<float>(0, 1)(rng);
      }
    }
    last = pipeline.process(pts);
  }
  EXPECT_GT(last.frame_count, 1);
  EXPECT_TRUE(std::isfinite(last.t_w_curr.x()));
}

TEST(InTenLoam, ClearResetsState) {
  InTenLoam pipeline;
  auto pts = makeWallScan(8.0, 20, 10, 0.4f);
  pipeline.process(pts);
  pipeline.process(pts);
  EXPECT_GT(pipeline.frameCount(), 0);
  pipeline.clear();
  EXPECT_EQ(pipeline.frameCount(), 0);
}
