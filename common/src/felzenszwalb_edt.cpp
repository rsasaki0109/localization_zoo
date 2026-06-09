#include "common/felzenszwalb_edt.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace localization_zoo {
namespace common {

namespace {

void edt1d(const float* f, float* d, int n) {
  if (n <= 0) return;

  std::vector<int> v(static_cast<size_t>(n));
  std::vector<float> z(static_cast<size_t>(n) + 1);
  int k = 0;
  v[0] = 0;
  z[0] = -std::numeric_limits<float>::infinity();
  z[1] = +std::numeric_limits<float>::infinity();

  for (int q = 1; q < n; ++q) {
    float s = ((f[q] + static_cast<float>(q * q)) -
               (f[v[k]] + static_cast<float>(v[k] * v[k]))) /
              (2.f * static_cast<float>(q - v[k]));
    while (s <= z[k]) {
      --k;
      s = ((f[q] + static_cast<float>(q * q)) -
           (f[v[k]] + static_cast<float>(v[k] * v[k]))) /
          (2.f * static_cast<float>(q - v[k]));
    }
    ++k;
    v[k] = q;
    z[k] = s;
    z[k + 1] = +std::numeric_limits<float>::infinity();
  }

  k = 0;
  for (int q = 0; q < n; ++q) {
    while (z[k + 1] < static_cast<float>(q)) {
      ++k;
    }
    const float diff = static_cast<float>(q - v[k]);
    d[q] = diff * diff + f[v[k]];
  }
}

}  // namespace

void felzenszwalbDistanceTransformMeters(float* grid, int width, int height, float resolution,
                                         float feature_value, float background_value,
                                         float far_distance_m) {
  if (grid == nullptr || width <= 0 || height <= 0) return;

  const size_t n = static_cast<size_t>(width * height);
  std::vector<float> f(n);
  std::vector<float> tmp(n);

  for (size_t i = 0; i < n; ++i) {
    f[i] = grid[i] <= feature_value + 1e-6f ? 0.f : background_value;
  }

  for (int y = 0; y < height; ++y) {
    edt1d(f.data() + static_cast<size_t>(y * width), tmp.data() + static_cast<size_t>(y * width),
          width);
  }

  f.resize(static_cast<size_t>(height));
  std::vector<float> col(static_cast<size_t>(height));
  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      f[static_cast<size_t>(y)] = tmp[static_cast<size_t>(y * width + x)];
    }
    edt1d(f.data(), col.data(), height);
    for (int y = 0; y < height; ++y) {
      const float sq_px = col[static_cast<size_t>(y)];
      const float dist_m =
          sq_px >= background_value * 0.5f ? far_distance_m
                                           : std::sqrt(std::max(0.f, sq_px)) * resolution;
      grid[static_cast<size_t>(y * width + x)] = dist_m;
    }
  }
}

}  // namespace common
}  // namespace localization_zoo
