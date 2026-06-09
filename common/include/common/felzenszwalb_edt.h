#pragma once

namespace localization_zoo {
namespace common {

/// Felzenszwalb & Huttenlocher 2D Euclidean distance transform.
///
/// Input grid: `feature_value` on occupied cells, `background_value` elsewhere.
/// Output grid: Euclidean distance to the nearest feature cell [meters].
void felzenszwalbDistanceTransformMeters(float* grid, int width, int height, float resolution,
                                         float feature_value = 0.f,
                                         float background_value = 1e6f,
                                         float far_distance_m = 5.f);

}  // namespace common
}  // namespace localization_zoo
