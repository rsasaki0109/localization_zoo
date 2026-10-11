#pragma once

#include "mulls/mulls_lo.h"

#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/point_types.h>

namespace localization_zoo {
namespace mulls {

// Uniform random subset of k indices out of n, in input order.
std::vector<int> sampleIndices(size_t n, size_t k, std::mt19937* rng);
LOCloud sampleCloud(const LOCloud& cloud, size_t k, std::mt19937* rng);

class PointTree {
public:
  explicit PointTree(const LOCloud& cloud);
  // Neighbours within r, nearest first.
  int radius(const Eigen::Vector3d& q, double r, std::vector<int>* idx,
             std::vector<float>* d2) const;
  bool nearest(const Eigen::Vector3d& q, int* index, double* dist) const;

private:
  pcl::KdTreeFLANN<pcl::PointXYZ> tree_;
  bool empty_ = true;
};

}  // namespace mulls
}  // namespace localization_zoo
