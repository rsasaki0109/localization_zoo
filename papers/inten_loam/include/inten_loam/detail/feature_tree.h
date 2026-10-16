#pragma once

#include "inten_loam/inten_loam_paper.h"

#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <flann/algorithms/dist.h>
#include <flann/algorithms/kdtree_single_index.h>
#include <flann/util/result_set.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <vector>

namespace localization_zoo {
namespace inten_loam {
namespace detail {

// The accelerated query uses the same float coordinates, KDTreeSingleIndex
// (15 points/leaf), exact SearchParams and small-k result set as PCL. Calling
// findNeighbors directly avoids allocating a query matrix and starting an
// OpenMP region for each individual query inside the outer feature loop.
class FeatureTree {
 public:
  std::vector<PaperFeature> pts;

  void build(bool accelerated = false) {
    ok_ = false;
    native_.reset();
    coordinates_.clear();
    if (pts.empty()) return;
    bool use_native = accelerated;
    if (accelerated) {
      coordinates_.reserve(pts.size() * 3);
      for (const auto& f : pts)
        for (int axis = 0; axis < 3; ++axis) {
          const float value = static_cast<float>(f.p(axis));
          coordinates_.push_back(value);
          use_native = use_native && std::isfinite(value);
        }
    }
    if (use_native) {
      native_ = std::make_unique<NativeIndex>(
          flann::Matrix<float>(coordinates_.data(), pts.size(), 3),
          flann::KDTreeSingleIndexParams(15));
      native_->buildIndex();
    } else {
      pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
      cloud->reserve(pts.size());
      for (const auto& f : pts)
        cloud->push_back(pcl::PointXYZ(static_cast<float>(f.p.x()), static_cast<float>(f.p.y()),
                                       static_cast<float>(f.p.z())));
      kd_.setInputCloud(cloud);
    }
    ok_ = true;
  }

  int knn(const Eigen::Vector3d& x, int k, std::vector<int>* idx, std::vector<float>* d2) const {
    if (!ok_) return 0;
    const int count = std::min<int>(k, static_cast<int>(pts.size()));
    if (!native_) {
      const pcl::PointXYZ q(static_cast<float>(x.x()), static_cast<float>(x.y()), static_cast<float>(x.z()));
      return kd_.nearestKSearch(q, count, *idx, *d2);
    }
    idx->resize(count);
    d2->resize(count);
    if (!count) return 0;
    const std::array<float, 3> query{{static_cast<float>(x.x()), static_cast<float>(x.y()), static_cast<float>(x.z())}};
    flann::KNNSimpleResultSet<float> result(count);
    native_->findNeighbors(result, query.data(), flann::SearchParams(-1, 0.0f, true));
    std::vector<size_t> indices(count);
    result.copy(indices.data(), d2->data(), count, true);
    std::copy(indices.begin(), indices.end(), idx->begin());
    return count;
  }

 private:
  using NativeIndex = flann::KDTreeSingleIndex<flann::L2_Simple<float>>;
  pcl::KdTreeFLANN<pcl::PointXYZ> kd_;
  bool ok_ = false;
  std::vector<float> coordinates_;
  std::unique_ptr<NativeIndex> native_;
};

}  // namespace detail
}  // namespace inten_loam
}  // namespace localization_zoo
