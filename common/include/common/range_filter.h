#pragma once

#include <pcl/point_cloud.h>

#include <algorithm>
#include <cstdint>

namespace localization_zoo {
namespace preprocessing {

/// Remove points strictly inside a sensor-centred sphere, preserving order and
/// every retained point field. Call after transforming into the sensor frame.
/// This optional non-paper preprocessing defaults off. The disabled path leaves
/// even organized-cloud metadata unchanged. Enabled filtering produces one row;
/// density, header and sensor pose metadata retain their input values.
///
/// Float arithmetic and the strict boundary match the historical runner filter.
/// Non-finite points are not sanitized here; consumers keep their own policy.
template <typename PointT>
void filterMinRange(pcl::PointCloud<PointT>& cloud, double min_range_m = 0.0) {
  if (!(min_range_m > 0.0)) return;
  const float min_sq = static_cast<float>(min_range_m * min_range_m);
  auto& points = cloud.points;
  points.erase(std::remove_if(points.begin(), points.end(), [min_sq](const PointT& point) {
                 return point.x * point.x + point.y * point.y + point.z * point.z < min_sq;
               }),
               points.end());
  cloud.width = static_cast<std::uint32_t>(points.size());
  cloud.height = 1;
}

}  // namespace preprocessing
}  // namespace localization_zoo
