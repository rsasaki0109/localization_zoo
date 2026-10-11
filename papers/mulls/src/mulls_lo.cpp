#include "mulls/mulls_lo.h"
#include "mulls_lo_internal.h"

#include <Eigen/Eigenvalues>

#include <cmath>

namespace localization_zoo {
namespace mulls {

namespace {

constexpr int kMapClasses[] = {kLOGround, kLOPillar, kLOFacade};

void transformCloud(const Eigen::Isometry3d& T, LOCloud* cloud) {
  for (auto& pt : *cloud) {
    pt.p = T * pt.p;
    pt.dir = T.linear() * pt.dir;
  }
}

}  // namespace

MullsLO::MullsLO(const MullsLOParams& params) : params_(params), rng_(params.seed) {}

void MullsLO::updateMap() {
  // The map follows the newest registered scan: re-express it there, then
  // add that scan's sparse features.
  const Eigen::Isometry3d to_previous = last_relative_.inverse();
  size_t total = 0;
  for (int c : kMapClasses) {
    transformCloud(to_previous, &map_[c]);
    total += map_[c].size();
  }

  // Drop new pillar / facade points close to the sensor whose nearest map
  // point is either a duplicate or displaced by about one frame's motion
  // (moving objects).
  double speed = 0.0;
  for (double d : recent_motion_) speed += d;
  if (!recent_motion_.empty()) speed = speed / recent_motion_.size() * params_.frame_rate;
  const double upper = std::max(0.15 * speed, 0.55);
  const bool remove_dynamic =
      params_.use_dynamic_removal && frame_index_ >= 4 &&
      static_cast<int>(total) > params_.dynamic_min_map_points;
  for (int c : kMapClasses) {
    LOCloud incoming = previous_.sparse[c];
    if (remove_dynamic && (c == kLOPillar || c == kLOFacade) && !map_[c].empty()) {
      const PointTree tree(map_[c]);
      LOCloud kept;
      for (const auto& pt : incoming) {
        int j;
        double d;
        if (std::hypot(pt.p.x(), pt.p.y()) < params_.dynamic_radius && tree.nearest(pt.p, &j, &d) &&
            (d <= params_.dynamic_same_dist || (d >= params_.dynamic_min_dist && d <= upper)))
          continue;
        kept.push_back(pt);
      }
      incoming.swap(kept);
    }
    map_[c].insert(map_[c].end(), incoming.begin(), incoming.end());
  }

  total = 0;
  for (int c : kMapClasses) {
    LOCloud kept;
    for (const auto& pt : map_[c])
      if (std::hypot(pt.p.x(), pt.p.y()) < params_.map_radius) kept.push_back(pt);
    map_[c].swap(kept);
    total += map_[c].size();
  }
  // Over the cap, every class shrinks in proportion, so old points decay.
  if (static_cast<int>(total) > params_.map_max_points) {
    for (int c : kMapClasses) {
      const size_t keep = static_cast<size_t>(
          std::floor(static_cast<double>(params_.map_max_points) / total * map_[c].size() + 1.0));
      map_[c] = sampleCloud(map_[c], keep, &rng_);
    }
  }

  if (params_.use_pillar_refresh && params_.pillar_refresh_period > 0 &&
      frame_index_ % params_.pillar_refresh_period == 0 &&
      !map_[kLOPillar].empty()) {
    // Re-fit pillar directions on the accumulated map; keep clean poles only.
    const LOCloud& pillars = map_[kLOPillar];
    const PointTree tree(pillars);
    LOCloud kept;
    std::vector<int> idx;
    std::vector<float> d2;
    for (const auto& pt : pillars) {
      tree.radius(pt.p, params_.pillar_refresh_radius, &idx, &d2);
      const size_t n = std::min(idx.size(), static_cast<size_t>(params_.pillar_refresh_max_neighbors));
      if (static_cast<int>(n) < params_.gf_min_points) continue;
      Eigen::Vector3d mean = Eigen::Vector3d::Zero();
      for (size_t k = 0; k < n; ++k) mean += pillars[idx[k]].p;
      mean /= static_cast<double>(n);
      Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
      for (size_t k = 0; k < n; ++k) {
        const Eigen::Vector3d d = pillars[idx[k]].p - mean;
        cov += d * d.transpose();
      }
      Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov / static_cast<double>(n));
      const double l1 = solver.eigenvalues()(2), l2 = solver.eigenvalues()(1);
      const Eigen::Vector3d e1 = solver.eigenvectors().col(2);
      if (l1 <= 0.0 || (l1 - l2) / l1 <= params_.pillar_refresh_linearity ||
          std::abs(e1.z()) <= params_.pillar_refresh_min_z)
        continue;
      LOPoint refreshed = pt;
      refreshed.dir = e1;
      kept.push_back(refreshed);
    }
    map_[kLOPillar].swap(kept);
  }

  map_bound_.setEmpty();
  for (int c : kMapClasses)
    for (const auto& pt : map_[c]) map_bound_.extend(pt.p);
}

MullsLOFrameResult MullsLO::process(const std::vector<Eigen::Vector4d>& xyzi) {
  MullsLOFrameResult result;
  LOFeatures features = extractFeatures(xyzi, params_, &rng_);
  for (int c = 0; c < kLONumClasses; ++c)
    result.feature_counts[c] = static_cast<int>(features.sparse[c].size());

  if (frame_index_ == 0) {
    poses_.push_back(Eigen::Isometry3d::Identity());
    guess_ = Eigen::Isometry3d::Identity();
    guess_.translation() = Eigen::Vector3d(0.5, 0.0, 0.0);
    previous_ = std::move(features);
    ++frame_index_;
    result.pose = poses_.back();
    return result;
  }

  updateMap();
  const double gate =
      params_.gate_initial + (frame_index_ <= 3 ? params_.gate_startup_bonus : 0.0);
  result.scan_to_map = params_.use_scan_to_map && frame_index_ > 2;
  if (result.scan_to_map) {
    result.icp = multiMetricICP(features.sparse, map_, map_bound_, guess_, gate, params_);
  } else {
    result.icp = multiMetricICP(features.sparse, previous_.dense, previous_.bound, guess_, gate,
                                params_);
  }

  // Constant velocity: the next guess repeats this frame's motion.
  last_relative_ = result.icp.T;
  guess_ = result.icp.T;
  recent_motion_.push_back(result.icp.T.translation().norm());
  if (recent_motion_.size() > 20) recent_motion_.pop_front();
  poses_.push_back(poses_.back() * result.icp.T);
  previous_ = std::move(features);
  ++frame_index_;

  result.pose = poses_.back();
  for (int c : kMapClasses) result.map_points += map_[c].size();
  return result;
}

}  // namespace mulls
}  // namespace localization_zoo
