// Diagnostic only: a cluster's full-point translation profile at the natural seed.
#pragma once
#include "inten_loam/inten_loam_paper.h"
#include <pcl/kdtree/kdtree_flann.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace diagnostic {
using Cloud = pcl::PointCloud<pcl::PointXYZ>;
using namespace localization_zoo::inten_loam;
struct Profile {
  std::vector<double> costs;
  int best = 60;
  bool excluded = false;
};

// One fixed rule. All constants and the absence of a semantic-motion guarantee
// are declared in protocol.json before running any KITTI profile.
inline Profile profile(const std::vector<Eigen::Vector3d>& source,
                       pcl::KdTreeFLANN<pcl::PointXYZ>& target,
                       const Eigen::Vector3d& direction) {
  Profile result;
  if (source.empty() || direction.squaredNorm() == 0.) return result;
  std::vector<float> distances(source.size());
  std::vector<int> ids(1);
  std::vector<float> ds(1);
  for (int step = -60; step <= 60; ++step) {
    for (size_t k = 0; k < source.size(); ++k) {
      const Eigen::Vector3d p = source[k] + (step * .05) * direction;
      if (target.nearestKSearch(pcl::PointXYZ(p.x(), p.y(), p.z()), 1, ids, ds) != 1)
        throw std::runtime_error("Full-point nearest neighbor failed");
      distances[k] = ds[0];
    }
    std::sort(distances.begin(), distances.end());
    const size_t kept = std::max<size_t>(1, distances.size() * 4 / 5);
    double sum = 0.;
    for (size_t k = 0; k < kept; ++k) sum += distances[k];
    result.costs.push_back(sum / kept);
  }
  // Prefer zero/smaller motion on exact ties; never use GT or a hand region.
  for (int k = 0; k < 121; ++k)
    if (result.costs[k] < result.costs[result.best] ||
        (result.costs[k] == result.costs[result.best] && std::abs(k - 60) < std::abs(result.best - 60)))
      result.best = k;
  const double shift = (result.best - 60) * .05;
  result.excluded = result.best > 0 && result.best < 120 && std::abs(shift) > 1. &&
                    result.costs[result.best] < .5 * result.costs[60];
  return result;
}
}  // namespace diagnostic
