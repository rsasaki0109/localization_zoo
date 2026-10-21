#pragma once

// Repository extension, not in the paper. These constants/rules are inherited
// unchanged from the committed conditional consensus/fixed-rotation studies.
#include "inten_loam/inten_loam_paper.h"
#include <pcl/kdtree/kdtree_flann.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>

namespace localization_zoo::inten_loam::detail {
struct ClusterTranslationProfile {
  int cluster = 0;
  std::vector<double> costs;
};

inline std::vector<ClusterTranslationProfile> clusterTranslationProfiles(
    const std::vector<PaperFeature>& previous, const PaperFrame& current,
    const Eigen::Matrix4d& seed, bool accelerated) {
  std::map<int, std::vector<Eigen::Vector3d>> clusters;
  for (const auto& p : current.facade) if (p.cluster >= 2) clusters[p.cluster];
  for (const auto& p : current.all) {
    const auto found = clusters.find(p.cluster);
    if (found != clusters.end())
      found->second.push_back(seed.block<3, 3>(0, 0) * p.p + seed.block<3, 1>(0, 3));
  }
  std::vector<ClusterTranslationProfile> result;
  std::vector<const std::vector<Eigen::Vector3d>*> sources;
  for (const auto& entry : clusters) {
    result.push_back({entry.first, {}});
    sources.push_back(&entry.second);
  }
  Eigen::Vector3d direction = seed.block<3, 1>(0, 3);
  if (direction.norm() == 0.) return result;
  direction.normalize();
  using Cloud = pcl::PointCloud<pcl::PointXYZ>;
  Cloud::Ptr cloud(new Cloud);
  for (const auto& p : previous)
    if (p.cluster >= 2) cloud->emplace_back(p.p.x(), p.p.y(), p.p.z());
  if (cloud->empty()) return result;
  pcl::KdTreeFLANN<pcl::PointXYZ> tree;
  tree.setInputCloud(cloud);
  // Each profile is independent. Keep every point, sort and sum in the exact
  // original order. Concurrent immutable FLANN queries use thread-local buffers.
#pragma omp parallel for if(accelerated) schedule(static)
  for (long c = 0; c < static_cast<long>(result.size()); ++c) {
    const auto& source = *sources[c];
    if (source.empty()) continue;
    auto& costs = result[c].costs;
    std::vector<float> distances(source.size()), ds(1);
    std::vector<int> ids(1);
    for (int step = -60; step <= 60; ++step) {
      for (size_t k = 0; k < source.size(); ++k) {
        const Eigen::Vector3d p = source[k] + (step * .05) * direction;
        if (tree.nearestKSearch(pcl::PointXYZ(p.x(), p.y(), p.z()), 1, ids, ds) != 1)
          throw std::runtime_error("Consensus nearest-neighbour query failed");
        distances[k] = ds[0];
      }
      std::sort(distances.begin(), distances.end());
      const size_t kept = std::max<size_t>(1, distances.size() * 4 / 5);
      double sum = 0.;
      for (size_t k = 0; k < kept; ++k) sum += distances[k];
      costs.push_back(sum / kept);
    }
  }
  return result;
}

inline bool admitted(const PaperClusterAdmission& band, int step) {
  return (band.bits[step / 64] >> (step % 64)) & 1ULL;
}

inline PaperConsensusTrace selectTranslationConsensus(
    const std::vector<ClusterTranslationProfile>& profiles) {
  PaperConsensusTrace result;
  result.evaluated = true;
  for (const auto& p : profiles) {
    if (p.costs.empty()) continue;
    if (p.costs.size() != 121 || !std::all_of(p.costs.begin(), p.costs.end(),
        [](double v) { return std::isfinite(v) && v >= 0.; }))
      throw std::invalid_argument("Invalid consensus profile");
    const double minimum = *std::min_element(p.costs.begin(), p.costs.end());
    PaperClusterAdmission band;
    band.cluster = p.cluster;
    for (int k = 0; k < 121; ++k) {
      if (p.costs[k] - minimum <= .1 * .1) {
        band.bits[k / 64] |= 1ULL << (k % 64);
        ++result.votes[k];
      }
    }
    result.admissions.push_back(band);
  }
  result.valid_clusters = static_cast<int>(result.admissions.size());
  for (int k = 0; k < 121; ++k) {
    const int b = result.best_index;
    if (result.votes[k] > result.votes[b] ||
        (result.votes[k] == result.votes[b] &&
         (std::abs(k - 60) < std::abs(b - 60) ||
          (std::abs(k - 60) == std::abs(b - 60) && k < b)))) result.best_index = k;
  }
  result.usable = result.valid_clusters > 0 && result.best_index > 0 && result.best_index < 120 &&
                  2 * result.votes[result.best_index] > result.valid_clusters;
  if (result.usable)
    for (const auto& band : result.admissions)
      if (!admitted(band, result.best_index)) result.excluded_clusters.push_back(band.cluster);
  return result;
}
}  // namespace localization_zoo::inten_loam::detail
