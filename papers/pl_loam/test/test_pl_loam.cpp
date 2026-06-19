#include "pl_loam/pl_loam.h"

#include <gtest/gtest.h>

#include <cmath>
#include <random>
#include <vector>

using namespace localization_zoo::pl_loam;

namespace {

std::vector<Eigen::Vector3d> makeWallCloud(double x_offset, double y0,
                                          double y1, int ny, double z0,
                                          double z1, int nz) {
  std::vector<Eigen::Vector3d> pts;
  for (int iy = 0; iy < ny; ++iy) {
    for (int iz = 0; iz < nz; ++iz) {
      const double y = y0 + (y1 - y0) * iy / std::max(1, ny - 1);
      const double z = z0 + (z1 - z0) * iz / std::max(1, nz - 1);
      pts.emplace_back(x_offset, y, z);
    }
  }
  return pts;
}

RangeImage filledDepthPatch(int width, int height, int cx, int cy,
                            float depth) {
  RangeImage img;
  img.width = width;
  img.height = height;
  img.depth.assign(static_cast<size_t>(width * height), 0.0f);
  img.intensity.assign(static_cast<size_t>(width * height), 0.5f);
  for (int y = cy - 5; y <= cy + 5; ++y) {
    for (int x = cx - 5; x <= cx + 5; ++x) {
      if (x < 0 || x >= width || y < 0 || y >= height) continue;
      img.depth[static_cast<size_t>(y * width + x)] = depth;
    }
  }
  return img;
}

}  // namespace

TEST(PlLoam, ExtractPointDepthUsesPatchMedian) {
  const RangeImage img = filledDepthPatch(200, 120, 100, 60, 12.5f);
  double depth = 0.0;
  ASSERT_TRUE(PlLoam::extractPointDepth(img, Eigen::Vector2d(100, 60), 4, 2.0,
                                         80.0, &depth));
  EXPECT_NEAR(depth, 12.5, 0.01);
}

TEST(PlLoam, BuildIntensityImageDensifiesSparseReturns) {
  RangeImage img;
  img.width = 24;
  img.height = 16;
  img.depth.assign(static_cast<size_t>(img.width * img.height), 0.0f);
  img.intensity.assign(static_cast<size_t>(img.width * img.height), 0.0f);
  const int low_idx = 8 * img.width + 8;
  const int high_idx = 8 * img.width + 14;
  img.depth[static_cast<size_t>(low_idx)] = 12.0f;
  img.intensity[static_cast<size_t>(low_idx)] = 0.1f;
  img.depth[static_cast<size_t>(high_idx)] = 12.0f;
  img.intensity[static_cast<size_t>(high_idx)] = 0.9f;

  const GrayscaleImage gray = PlLoam::buildIntensityImage(img, 1);
  ASSERT_EQ(gray.width, img.width);
  ASSERT_EQ(gray.height, img.height);
  ASSERT_EQ(gray.pixels.size(), img.depth.size());
  EXPECT_GT(gray.pixels[static_cast<size_t>(high_idx)],
            gray.pixels[static_cast<size_t>(low_idx)]);
  EXPECT_GT(gray.pixels[static_cast<size_t>(7 * img.width + 14)], 0);
  EXPECT_EQ(gray.pixels.front(), 0);
}

TEST(PlLoam, ScaleCorrectionMedianRatio) {
  const std::vector<double> lidar = {10.0, 20.0, 30.0};
  const std::vector<double> visual = {5.0, 10.0, 15.0};
  EXPECT_NEAR(PlLoam::scaleCorrectionFactor(lidar, visual), 2.0, 1e-6);
  EXPECT_DOUBLE_EQ(PlLoam::scaleCorrectionFactor({}, {}), 1.0);
}

TEST(PlLoam, OptimizeRelativePoseRecoversTranslation) {
  const CameraModel cam = CameraModel::kittiHalfRes();
  PlLoamParams params;
  params.use_scale_correction = false;
  params.use_line_features = false;

  std::vector<PointFeature> prev(12);
  std::vector<PointFeature> curr(12);
  for (int i = 0; i < 12; ++i) {
    const double u = 120.0 + 8.0 * i;
    const double v = 60.0 + (i % 3) * 5.0;
    const double d = 10.0 + 0.1 * i;
    prev[i].uv = Eigen::Vector2d(u, v);
    prev[i].depth_prior = d;
    prev[i].has_depth = true;
    curr[i].uv = Eigen::Vector2d(u + 2.0, v + 0.5);
    curr[i].depth_prior = d * 0.99;
    curr[i].has_depth = true;
  }

  std::vector<std::pair<int, int>> matches;
  matches.reserve(prev.size());
  for (size_t i = 0; i < prev.size(); ++i) matches.emplace_back(i, i);

  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  double scale = 1.0;
  size_t num_priors = 0;
  double mean_res = 0.0;
  ASSERT_TRUE(PlLoam::optimizeRelativePose(
      cam, params, prev, curr, {}, {}, matches, {}, &T, &scale, &num_priors,
      &mean_res));
  EXPECT_GT(num_priors, 5u);
  EXPECT_TRUE(std::isfinite(T(0, 3)));
}

TEST(PlLoam, TracksShortSequence) {
  std::mt19937 rng(21);
  PlLoam pipeline;
  PlLoamResult last;
  for (int frame = 0; frame < 8; ++frame) {
    std::vector<Eigen::Vector3d> pts =
        makeWallCloud(10.0 + 0.4 * frame, -5.0, 5.0, 35, -0.5, 2.5, 18);
    std::vector<float> intensity(pts.size());
    std::uniform_real_distribution<float> dist(0.3f, 0.9f);
    for (float& v : intensity) v = dist(rng);
    last = pipeline.process(pts, intensity);
  }
  EXPECT_GT(last.frame_count, 1);
  EXPECT_LT((last.t_w_curr - Eigen::Vector3d(2.8, 0.0, 0.0)).norm(), 4.0);
}

TEST(PlLoam, ClearResetsState) {
  PlLoam pipeline;
  std::vector<Eigen::Vector3d> pts = makeWallCloud(9.0, -3.0, 3.0, 20, 0.0, 2.0, 10);
  std::vector<float> intensity(pts.size(), 0.5f);
  pipeline.process(pts, intensity);
  pipeline.process(pts, intensity);
  EXPECT_GT(pipeline.frameCount(), 0);
  pipeline.clear();
  EXPECT_EQ(pipeline.frameCount(), 0);
  EXPECT_EQ(pipeline.position(), Eigen::Vector3d::Zero());
}
