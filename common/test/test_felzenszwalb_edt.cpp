#include "common/felzenszwalb_edt.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using localization_zoo::common::felzenszwalbDistanceTransformMeters;

TEST(FelzenszwalbEdt, SingleFeatureCenter) {
  constexpr int w = 5;
  constexpr int h = 5;
  constexpr float res = 0.1f;
  std::vector<float> grid(static_cast<size_t>(w * h), 1e6f);
  grid[static_cast<size_t>(2 * w + 2)] = 0.f;

  felzenszwalbDistanceTransformMeters(grid.data(), w, h, res);

  EXPECT_NEAR(grid[static_cast<size_t>(2 * w + 2)], 0.0, 1e-6);
  EXPECT_NEAR(grid[static_cast<size_t>(2 * w + 3)], res, 1e-5);
  EXPECT_NEAR(grid[static_cast<size_t>(3 * w + 2)], res, 1e-5);
  EXPECT_NEAR(grid[static_cast<size_t>(3 * w + 3)], std::sqrt(2.f) * res, 1e-5);
}

TEST(FelzenszwalbEdt, TwoFeaturesRow) {
  constexpr int w = 7;
  constexpr int h = 3;
  constexpr float res = 0.05f;
  std::vector<float> grid(static_cast<size_t>(w * h), 1e6f);
  grid[static_cast<size_t>(1 * w + 1)] = 0.f;
  grid[static_cast<size_t>(1 * w + 5)] = 0.f;

  felzenszwalbDistanceTransformMeters(grid.data(), w, h, res);

  EXPECT_NEAR(grid[static_cast<size_t>(1 * w + 3)], 2.f * res, 1e-5);
  EXPECT_NEAR(grid[static_cast<size_t>(0 * w + 3)], std::sqrt(5.f) * res, 1e-5);
}
