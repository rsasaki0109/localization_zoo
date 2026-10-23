#pragma once
#include "kiss_icp/kiss_icp.h"
#include <cmath>

namespace localization_zoo::inten_loam::detail {
// The existing Zoo upstream-mechanics profile, with the input sensor range.
// Fixed upstream defaults; no sequence-specific tuning or motion guards.
inline kiss_icp::KISSICPParams geometricPriorParams(double max_range) {
  kiss_icp::KISSICPParams p;
  p.voxel_size = 1.0;
  p.min_range = 0.0;
  p.max_range = max_range;
  p.local_map_radius = max_range;
  p.map_cleanup_interval = 1;
  p.max_points_per_voxel = 20;
  p.initial_threshold = 2.0;
  p.max_icp_iterations = 500;
  p.convergence_criterion = 1e-4;
  p.registration_voxel_multiplier = 1.5;
  p.neighbor_voxel_radius = 1;
  p.geman_mcclure_kernel = true;
  p.map_min_point_distance = std::sqrt(1.0 / 20.0);
  p.use_model_deviation_threshold = true;
  return p;
}
}  // namespace localization_zoo::inten_loam::detail
