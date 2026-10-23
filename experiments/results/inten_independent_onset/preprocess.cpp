// Arithmetic copied from inten_facade_pca/native_loader.h for leaf=0,
// identity sensor pose and min-range flag=0. No PCL or estimator dependency.
#include <cmath>
#include <cstddef>
extern "C" std::size_t preprocess(float* xyzi, std::size_t size) {
  const double angle = 0.17 * (std::acos(-1.0) / 180.0);
  const double sin_angle = std::sin(angle), cos_angle = std::cos(angle);
  std::size_t kept = 0;
  for (std::size_t i = 0; i < size; ++i) {
    float x = xyzi[4*i], y = xyzi[4*i+1], z = xyzi[4*i+2];
    const float intensity = xyzi[4*i+3];
    const double horizontal = std::hypot(x, y);
    if (horizontal > 1e-12) {
      const double corrected_horizontal = horizontal * cos_angle - z * sin_angle;
      const double scale = corrected_horizontal / horizontal;
      z = static_cast<float>(z * cos_angle + horizontal * sin_angle);
      x = static_cast<float>(x * scale);
      y = static_cast<float>(y * scale);
    }
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
        !std::isfinite(intensity)) continue;
    const double r = std::sqrt(x*x + y*y + z*z);
    if (r <= 1.0 || r >= 120.0) continue;
    xyzi[4*kept] = x; xyzi[4*kept+1] = y;
    xyzi[4*kept+2] = z; xyzi[4*kept+3] = intensity;
    ++kept;
  }
  return kept;
}
