#include "loam_livox/loam_livox.h"

#include <pcl/filters/voxel_grid.h>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace localization_zoo {
namespace loam_livox {

namespace {

float wrapAngle(float angle) {
  constexpr float kTwoPi = static_cast<float>(2.0 * M_PI);
  while (angle < 0.0f) {
    angle += kTwoPi;
  }
  while (angle >= kTwoPi) {
    angle -= kTwoPi;
  }
  return angle;
}

void appendPoint(const aloam::AnnotatedPoint& ap, aloam::PointCloudPtr cloud,
                 float scan_period) {
  aloam::PointT point;
  point.x = ap.x;
  point.y = ap.y;
  point.z = ap.z;
  point.intensity = ap.scan_id + scan_period * ap.rel_time;
  cloud->push_back(point);
}

}  // namespace

aloam::FeatureCloud LivoxScanRegistration::extract(
    const aloam::PointCloudConstPtr& cloud) const {
  aloam::FeatureCloud result;
  if (cloud == nullptr || cloud->empty()) {
    return result;
  }

  struct IndexedPoint {
    float yaw = 0.0f;
    int index = -1;
  };

  std::vector<aloam::AnnotatedPoint> annotated;
  std::vector<IndexedPoint> ordered_points;
  annotated.reserve(cloud->size());
  ordered_points.reserve(cloud->size());

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

    aloam::AnnotatedPoint ap;
    ap.x = point.x;
    ap.y = point.y;
    ap.z = point.z;
    ap.scan_id = 0;
    ap.rel_time = 0.0f;
    ap.curvature = 0.0f;
    annotated.push_back(ap);

    ordered_points.push_back(
        IndexedPoint{wrapAngle(std::atan2(point.y, point.x)),
                     static_cast<int>(annotated.size()) - 1});
  }

  if (annotated.size() < static_cast<size_t>(2 * params_.neighbor_window + 2)) {
    return result;
  }

  std::sort(ordered_points.begin(), ordered_points.end(),
            [](const IndexedPoint& lhs, const IndexedPoint& rhs) {
              return lhs.yaw < rhs.yaw;
            });

  const float min_yaw = ordered_points.front().yaw;
  const float max_yaw = ordered_points.back().yaw;
  const float yaw_span = std::max(max_yaw - min_yaw, 1e-3f);

  std::vector<std::vector<int>> sector_indices(params_.num_azimuth_sectors);
  for (size_t i = 0; i < ordered_points.size(); ++i) {
    const int idx = ordered_points[i].index;
    const int sector_id = std::min(
        params_.num_azimuth_sectors - 1,
        static_cast<int>(std::floor((ordered_points[i].yaw - min_yaw) / yaw_span *
                                    params_.num_azimuth_sectors)));
    annotated[idx].scan_id = std::max(0, sector_id);
    annotated[idx].rel_time =
        static_cast<float>(i) / static_cast<float>(ordered_points.size());
    sector_indices[sector_id].push_back(idx);
  }

  std::vector<int> label(annotated.size(), 0);
  std::vector<bool> neighbor_picked(annotated.size(), false);

  for (const auto& indices : sector_indices) {
    if (static_cast<int>(indices.size()) <= 2 * params_.neighbor_window + 1) {
      for (int idx : indices) {
        neighbor_picked[idx] = true;
      }
      continue;
    }

    for (int i = 0; i < params_.neighbor_window; ++i) {
      neighbor_picked[indices[i]] = true;
      neighbor_picked[indices[indices.size() - 1 - i]] = true;
    }

    for (size_t i = params_.neighbor_window;
         i + params_.neighbor_window < indices.size(); ++i) {
      float diff_x = 0.0f;
      float diff_y = 0.0f;
      float diff_z = 0.0f;
      const auto& center = annotated[indices[i]];
      for (int offset = -params_.neighbor_window; offset <= params_.neighbor_window;
           ++offset) {
        if (offset == 0) {
          continue;
        }
        const auto& neighbor = annotated[indices[i + offset]];
        diff_x += neighbor.x - center.x;
        diff_y += neighbor.y - center.y;
        diff_z += neighbor.z - center.z;
      }
      annotated[indices[i]].curvature =
          diff_x * diff_x + diff_y * diff_y + diff_z * diff_z;
    }
  }

  const int num_subregions = std::max(1, params_.num_subregions);
  for (const auto& indices : sector_indices) {
    const int sector_size = static_cast<int>(indices.size());
    if (sector_size <= 2 * params_.neighbor_window + 1) {
      continue;
    }

    for (int subregion = 0; subregion < num_subregions; ++subregion) {
      const int start = (sector_size * subregion) / num_subregions;
      const int end = (sector_size * (subregion + 1)) / num_subregions - 1;
      if (end - start < 2 * params_.neighbor_window) {
        continue;
      }

      std::vector<int> sorted_local_indices;
      for (int i = start; i <= end; ++i) {
        sorted_local_indices.push_back(indices[i]);
      }
      std::sort(sorted_local_indices.begin(), sorted_local_indices.end(),
                [&annotated](int lhs, int rhs) {
                  return annotated[lhs].curvature > annotated[rhs].curvature;
                });

      int corner_count = 0;
      for (int idx : sorted_local_indices) {
        if (neighbor_picked[idx] ||
            annotated[idx].curvature <= params_.curvature_threshold) {
          continue;
        }

        ++corner_count;
        if (corner_count <= params_.max_corner_sharp) {
          label[idx] = 2;
          appendPoint(annotated[idx], result.corner_sharp, params_.scan_period);
          appendPoint(annotated[idx], result.corner_less_sharp, params_.scan_period);
        } else if (corner_count <= params_.max_corner_less_sharp) {
          label[idx] = 1;
          appendPoint(annotated[idx], result.corner_less_sharp, params_.scan_period);
        } else {
          break;
        }

        neighbor_picked[idx] = true;
      }

      int flat_count = 0;
      for (auto it = sorted_local_indices.rbegin(); it != sorted_local_indices.rend();
           ++it) {
        const int idx = *it;
        if (neighbor_picked[idx] ||
            annotated[idx].curvature >= params_.curvature_threshold) {
          continue;
        }

        label[idx] = -1;
        appendPoint(annotated[idx], result.surf_flat, params_.scan_period);
        neighbor_picked[idx] = true;
        ++flat_count;
        if (flat_count >= params_.max_surf_flat) {
          break;
        }
      }
    }
  }

  aloam::PointCloudPtr surf_less_flat_scan(new aloam::PointCloud);
  for (const auto& indices : sector_indices) {
    for (int idx : indices) {
      if (label[idx] <= 0) {
        appendPoint(annotated[idx], surf_less_flat_scan, params_.scan_period);
      }
      appendPoint(annotated[idx], result.full_cloud, params_.scan_period);
    }
  }

  pcl::VoxelGrid<aloam::PointT> voxel_grid;
  voxel_grid.setInputCloud(surf_less_flat_scan);
  voxel_grid.setLeafSize(params_.less_flat_leaf_size, params_.less_flat_leaf_size,
                         params_.less_flat_leaf_size);
  voxel_grid.filter(*result.surf_less_flat);

  return result;
}

LivoxLOAM::LivoxLOAM(const LivoxLOAMParams& params)
    : scan_registration_(params.scan_registration),
      odometry_(params.odometry),
      mapping_(params.mapping) {}

LivoxLOAMResult LivoxLOAM::process(const aloam::PointCloudConstPtr& cloud) {
  LivoxLOAMResult result;

  const aloam::FeatureCloud features = scan_registration_.extract(cloud);
  result.num_corner_features = features.corner_less_sharp->size();
  result.num_surface_features = features.surf_less_flat->size();

  const auto odometry_result = odometry_.process(features);
  if (!odometry_result.valid) {
    return result;
  }

  const auto mapping_result = mapping_.process(features.corner_less_sharp,
                                               features.surf_less_flat,
                                               features.full_cloud,
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

}  // namespace loam_livox
}  // namespace localization_zoo
