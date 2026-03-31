#include "lego_loam/lego_loam.h"

#include <pcl/filters/voxel_grid.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace localization_zoo {
namespace lego_loam {

namespace {

struct IndexedPoint {
  float yaw = 0.0f;
  int index = -1;
};

void appendPoint(const aloam::AnnotatedPoint& point, aloam::PointCloudPtr cloud,
                 float scan_period) {
  aloam::PointT output;
  output.x = point.x;
  output.y = point.y;
  output.z = point.z;
  output.intensity = point.scan_id + scan_period * point.rel_time;
  cloud->push_back(output);
}

int nearestYawIndex(const std::vector<IndexedPoint>& points, float yaw) {
  if (points.empty()) {
    return -1;
  }

  auto it = std::lower_bound(
      points.begin(), points.end(), yaw,
      [](const IndexedPoint& lhs, float rhs) { return lhs.yaw < rhs; });
  if (it == points.begin()) {
    return it->index;
  }
  if (it == points.end()) {
    return points.back().index;
  }

  const auto previous = std::prev(it);
  return std::abs(previous->yaw - yaw) < std::abs(it->yaw - yaw) ? previous->index
                                                                  : it->index;
}

}  // namespace

int GroundAwareScanRegistration::computeScanId(float x, float y, float z) const {
  const float vertical_angle =
      std::atan2(z, std::sqrt(x * x + y * y)) * 180.0f / static_cast<float>(M_PI);

  if (params_.n_scans == 16) {
    const int scan_id = static_cast<int>(vertical_angle + 15.0f) / 2 + 0.5f;
    return (scan_id >= 0 && scan_id < params_.n_scans) ? scan_id : -1;
  }
  if (params_.n_scans == 32) {
    const int scan_id =
        static_cast<int>((vertical_angle + 92.0f / 3.0f) * 3.0f / 4.0f);
    return (scan_id >= 0 && scan_id < params_.n_scans) ? scan_id : -1;
  }
  if (params_.n_scans == 64) {
    int scan_id = 0;
    if (vertical_angle >= -8.83f) {
      scan_id = static_cast<int>((2.0f - vertical_angle) * 3.0f + 0.5f);
    } else {
      scan_id = params_.n_scans / 2 +
                static_cast<int>((-8.83f - vertical_angle) * 2.0f + 0.5f);
    }
    return (scan_id >= 0 && scan_id < params_.n_scans) ? scan_id : -1;
  }

  const float fov = 30.0f;
  const int scan_id = static_cast<int>((vertical_angle + fov / 2.0f) /
                                       (fov / params_.n_scans));
  return (scan_id >= 0 && scan_id < params_.n_scans) ? scan_id : -1;
}

GroundFeatureCloud GroundAwareScanRegistration::extract(
    const aloam::PointCloudConstPtr& cloud) const {
  GroundFeatureCloud result;
  if (cloud == nullptr || cloud->empty()) {
    return result;
  }

  std::vector<aloam::AnnotatedPoint> annotated;
  std::vector<std::vector<IndexedPoint>> ordered_scans(params_.n_scans);
  annotated.reserve(cloud->size());

  for (size_t i = 0; i < cloud->size(); ++i) {
    const auto& point = cloud->points[i];
    if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)) {
      continue;
    }

    const float range =
        std::sqrt(point.x * point.x + point.y * point.y + point.z * point.z);
    if (range < params_.minimum_range || range > params_.maximum_range) {
      continue;
    }

    const int scan_id = computeScanId(point.x, point.y, point.z);
    if (scan_id < 0) {
      continue;
    }

    aloam::AnnotatedPoint annotated_point;
    annotated_point.x = point.x;
    annotated_point.y = point.y;
    annotated_point.z = point.z;
    annotated_point.scan_id = scan_id;
    annotated_point.rel_time = 0.0f;
    annotated_point.curvature = 0.0f;
    annotated.push_back(annotated_point);

    ordered_scans[scan_id].push_back(
        IndexedPoint{-std::atan2(point.y, point.x),
                     static_cast<int>(annotated.size()) - 1});
  }

  if (annotated.size() < static_cast<size_t>(2 * params_.neighbor_window + 2)) {
    return result;
  }

  for (auto& scan : ordered_scans) {
    std::sort(scan.begin(), scan.end(),
              [](const IndexedPoint& lhs, const IndexedPoint& rhs) {
                return lhs.yaw < rhs.yaw;
              });
    for (size_t i = 0; i < scan.size(); ++i) {
      annotated[scan[i].index].rel_time =
          static_cast<float>(i) / std::max<size_t>(1, scan.size());
    }
  }

  std::vector<bool> is_ground(annotated.size(), false);
  for (int scan_id = 0;
       scan_id < std::min(params_.ground_scan_limit, params_.n_scans - 1);
       ++scan_id) {
    const auto& lower_scan = ordered_scans[scan_id];
    const auto& upper_scan = ordered_scans[scan_id + 1];
    if (lower_scan.empty() || upper_scan.empty()) {
      continue;
    }

    for (const auto& lower_point : lower_scan) {
      const int lower_index = lower_point.index;
      const int upper_index = nearestYawIndex(upper_scan, lower_point.yaw);
      if (upper_index < 0) {
        continue;
      }

      const auto& lower = annotated[lower_index];
      const auto& upper = annotated[upper_index];
      const float dx = upper.x - lower.x;
      const float dy = upper.y - lower.y;
      const float dz = upper.z - lower.z;
      const float angle_deg =
          std::atan2(dz, std::sqrt(dx * dx + dy * dy)) * 180.0f /
          static_cast<float>(M_PI);

      const bool height_ground =
          std::abs(lower.z + params_.sensor_height) <=
          params_.ground_height_tolerance;
      if (height_ground ||
          std::abs(angle_deg - params_.sensor_mount_angle_deg) <=
              params_.ground_angle_threshold_deg) {
        is_ground[lower_index] = true;
        is_ground[upper_index] = true;
      }
    }
  }

  for (size_t i = 0; i < annotated.size(); ++i) {
    if (std::abs(annotated[i].z + params_.sensor_height) <=
        params_.ground_height_tolerance) {
      is_ground[i] = true;
    }
  }

  result.num_ground_points = std::count(is_ground.begin(), is_ground.end(), true);

  std::vector<int> label(annotated.size(), 0);
  std::vector<bool> neighbor_picked(annotated.size(), false);

  for (const auto& scan : ordered_scans) {
    if (static_cast<int>(scan.size()) <= 2 * params_.neighbor_window + 1) {
      for (const auto& point : scan) {
        neighbor_picked[point.index] = true;
      }
      continue;
    }

    for (int i = 0; i < params_.neighbor_window; ++i) {
      neighbor_picked[scan[i].index] = true;
      neighbor_picked[scan[scan.size() - 1 - i].index] = true;
    }

    for (size_t i = params_.neighbor_window;
         i + params_.neighbor_window < scan.size(); ++i) {
      float diff_x = 0.0f;
      float diff_y = 0.0f;
      float diff_z = 0.0f;
      const auto& center = annotated[scan[i].index];
      for (int offset = -params_.neighbor_window; offset <= params_.neighbor_window;
           ++offset) {
        if (offset == 0) {
          continue;
        }
        const auto& neighbor = annotated[scan[i + offset].index];
        diff_x += neighbor.x - center.x;
        diff_y += neighbor.y - center.y;
        diff_z += neighbor.z - center.z;
      }
      annotated[scan[i].index].curvature =
          diff_x * diff_x + diff_y * diff_y + diff_z * diff_z;
    }
  }

  const int num_subregions = std::max(1, params_.num_subregions);
  for (const auto& scan : ordered_scans) {
    const int scan_size = static_cast<int>(scan.size());
    if (scan_size <= 2 * params_.neighbor_window + 1) {
      continue;
    }

    for (int subregion = 0; subregion < num_subregions; ++subregion) {
      const int start = (scan_size * subregion) / num_subregions;
      const int end = (scan_size * (subregion + 1)) / num_subregions - 1;
      if (end - start < 2 * params_.neighbor_window) {
        continue;
      }

      std::vector<int> sorted_indices;
      for (int i = start; i <= end; ++i) {
        sorted_indices.push_back(scan[i].index);
      }
      std::sort(sorted_indices.begin(), sorted_indices.end(),
                [&annotated](int lhs, int rhs) {
                  return annotated[lhs].curvature > annotated[rhs].curvature;
                });

      int corner_count = 0;
      for (int index : sorted_indices) {
        if (neighbor_picked[index] || is_ground[index] ||
            annotated[index].curvature <= params_.curvature_threshold) {
          continue;
        }

        ++corner_count;
        if (corner_count <= params_.max_corner_sharp) {
          label[index] = 2;
          appendPoint(annotated[index], result.features.corner_sharp,
                      params_.scan_period);
          appendPoint(annotated[index], result.features.corner_less_sharp,
                      params_.scan_period);
        } else if (corner_count <= params_.max_corner_less_sharp) {
          label[index] = 1;
          appendPoint(annotated[index], result.features.corner_less_sharp,
                      params_.scan_period);
        } else {
          break;
        }

        neighbor_picked[index] = true;
      }

      int flat_count = 0;
      for (auto it = sorted_indices.rbegin(); it != sorted_indices.rend(); ++it) {
        const int index = *it;
        if (neighbor_picked[index] ||
            annotated[index].curvature >= params_.curvature_threshold) {
          continue;
        }

        label[index] = -1;
        appendPoint(annotated[index], result.features.surf_flat,
                    params_.scan_period);
        neighbor_picked[index] = true;
        ++flat_count;
        if (flat_count >= params_.max_surf_flat) {
          break;
        }
      }
    }
  }

  aloam::PointCloudPtr surf_less_flat_scan(new aloam::PointCloud);
  for (const auto& scan : ordered_scans) {
    for (const auto& indexed_point : scan) {
      const int index = indexed_point.index;
      if (label[index] <= 0) {
        appendPoint(annotated[index], surf_less_flat_scan, params_.scan_period);
      }
      appendPoint(annotated[index], result.features.full_cloud, params_.scan_period);
    }
  }

  pcl::VoxelGrid<aloam::PointT> voxel_grid;
  voxel_grid.setInputCloud(surf_less_flat_scan);
  voxel_grid.setLeafSize(params_.less_flat_leaf_size, params_.less_flat_leaf_size,
                         params_.less_flat_leaf_size);
  voxel_grid.filter(*result.features.surf_less_flat);

  return result;
}

LeGOLOAM::LeGOLOAM(const LeGOLOAMParams& params)
    : scan_registration_(params.scan_registration),
      odometry_(params.odometry),
      mapping_(params.mapping) {}

LeGOLOAMResult LeGOLOAM::process(const aloam::PointCloudConstPtr& cloud) {
  LeGOLOAMResult result;

  const GroundFeatureCloud features = scan_registration_.extract(cloud);
  result.num_ground_points = features.num_ground_points;
  result.num_corner_features = features.features.corner_less_sharp->size();
  result.num_surface_features = features.features.surf_less_flat->size();

  const auto odometry_result = odometry_.process(features.features);
  if (!odometry_result.valid) {
    return result;
  }

  const auto mapping_result = mapping_.process(features.features.corner_less_sharp,
                                               features.features.surf_less_flat,
                                               features.features.full_cloud,
                                               odometry_result.q_w_curr,
                                               odometry_result.t_w_curr);
  if (!mapping_result.valid) {
    return result;
  }

  result.q_w_curr = mapping_result.q_w_curr;
  result.t_w_curr = mapping_result.t_w_curr;
  result.valid = true;
  return result;
}

}  // namespace lego_loam
}  // namespace localization_zoo
