#include "mb_icp/mb_icp.h"

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

namespace localization_zoo {
namespace mb_icp {

namespace {

Eigen::Matrix3d matrixFromIncrement(double tx, double ty, double dtheta) {
  Eigen::Matrix3d T = Eigen::Matrix3d::Identity();
  T(0, 0) = std::cos(dtheta);
  T(0, 1) = -std::sin(dtheta);
  T(1, 0) = std::sin(dtheta);
  T(1, 1) = std::cos(dtheta);
  T(0, 2) = tx;
  T(1, 2) = ty;
  return T;
}

double angleIncrement(const LaserScan& scan) {
  if (scan.angle_increment > 0.0) return scan.angle_increment;
  if (scan.size() > 1) {
    return (scan.angle_max - scan.angle_min) / static_cast<double>(scan.size() - 1);
  }
  return 0.0;
}

Eigen::Vector2d closestPointOnSegment(const Eigen::Vector2d& p, const Eigen::Vector2d& a,
                                      const Eigen::Vector2d& b) {
  const Eigen::Vector2d ab = b - a;
  const double denom = ab.squaredNorm();
  if (denom < 1e-12) return a;
  const double u = std::clamp((p - a).dot(ab) / denom, 0.0, 1.0);
  return a + u * ab;
}

}  // namespace

Eigen::Matrix3d pose2D(double x, double y, double yaw) {
  return matrixFromIncrement(x, y, yaw);
}

int64_t MbICPEstimator::voxelKey(double x, double y, double voxel_size) {
  const int ix = static_cast<int>(std::floor(x / voxel_size));
  const int iy = static_cast<int>(std::floor(y / voxel_size));
  return (static_cast<int64_t>(ix) << 32) ^ static_cast<uint32_t>(static_cast<uint32_t>(iy));
}

MbICPEstimator::LocalMapIndex MbICPEstimator::LocalMapIndex::build(
    const std::vector<Segment>& segments, const std::vector<RefPoint>& points,
    double cell_size, double query_radius) {
  LocalMapIndex index;
  index.cell_size = std::max(cell_size, 1e-3);
  index.query_radius = std::max(query_radius, index.cell_size);

  const auto key = [](int ix, int iy) {
    return (static_cast<int64_t>(ix) << 32) ^ static_cast<uint32_t>(iy);
  };

  for (size_t i = 0; i < segments.size(); ++i) {
    const auto& seg = segments[i];
    const Eigen::Vector2d mid = 0.5 * (seg.a + seg.b);
    const int ix = static_cast<int>(std::floor(mid.x() / index.cell_size));
    const int iy = static_cast<int>(std::floor(mid.y() / index.cell_size));
    index.segment_bins[key(ix, iy)].push_back(i);
  }

  for (size_t i = 0; i < points.size(); ++i) {
    if (!points[i].valid) continue;
    const int ix = static_cast<int>(std::floor(points[i].point.x() / index.cell_size));
    const int iy = static_cast<int>(std::floor(points[i].point.y() / index.cell_size));
    index.point_bins[key(ix, iy)].push_back(i);
  }
  return index;
}

void MbICPEstimator::LocalMapIndex::querySegments(const Eigen::Vector2d& p,
                                                  std::unordered_set<size_t>* visited) const {
  const int cx = static_cast<int>(std::floor(p.x() / cell_size));
  const int cy = static_cast<int>(std::floor(p.y() / cell_size));
  const int reach = static_cast<int>(std::ceil(query_radius / cell_size)) + 1;
  for (int dx = -reach; dx <= reach; ++dx) {
    for (int dy = -reach; dy <= reach; ++dy) {
      const int64_t k = (static_cast<int64_t>(cx + dx) << 32) ^ static_cast<uint32_t>(cy + dy);
      const auto it = segment_bins.find(k);
      if (it == segment_bins.end()) continue;
      visited->insert(it->second.begin(), it->second.end());
    }
  }
}

void MbICPEstimator::LocalMapIndex::queryPoints(const Eigen::Vector2d& p,
                                                std::unordered_set<size_t>* visited) const {
  const int cx = static_cast<int>(std::floor(p.x() / cell_size));
  const int cy = static_cast<int>(std::floor(p.y() / cell_size));
  const int reach = static_cast<int>(std::ceil(query_radius / cell_size)) + 1;
  for (int dx = -reach; dx <= reach; ++dx) {
    for (int dy = -reach; dy <= reach; ++dy) {
      const int64_t k = (static_cast<int64_t>(cx + dx) << 32) ^ static_cast<uint32_t>(cy + dy);
      const auto it = point_bins.find(k);
      if (it == point_bins.end()) continue;
      visited->insert(it->second.begin(), it->second.end());
    }
  }
}

MbICPEstimator::MbICPEstimator(const MbICPParams& params) : params_(params) {}

void MbICPEstimator::reset() {
  initialized_ = false;
  ref_points_.clear();
  ref_segments_.clear();
  local_points_.clear();
  local_segments_.clear();
  local_map_index_ = LocalMapIndex{};
  local_map_index_valid_ = false;
  pose_ = Eigen::Matrix3d::Identity();
  last_increment_ = Eigen::Matrix3d::Identity();
}

void MbICPEstimator::setInitialPose(const Eigen::Matrix3d& pose) { pose_ = pose; }

Eigen::Vector2d MbICPEstimator::transformPoint(const Eigen::Matrix3d& T,
                                               const Eigen::Vector2d& p) {
  return (T.block<2, 2>(0, 0) * p + T.block<2, 1>(0, 2)).eval();
}

std::vector<Eigen::Vector2d> MbICPEstimator::scanToPoints(const LaserScan& scan) const {
  std::vector<Eigen::Vector2d> points;
  points.reserve(scan.size());
  const double inc = angleIncrement(scan);
  for (size_t i = 0; i < scan.size(); ++i) {
    const double r = scan.ranges[i];
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    const double a = scan.angle_min + static_cast<double>(i) * inc;
    points.emplace_back(r * std::cos(a), r * std::sin(a));
  }
  return points;
}

void MbICPEstimator::buildReferenceModel(const LaserScan& scan) {
  ref_points_.assign(scan.size(), RefPoint{});
  ref_segments_.clear();

  const double inc = angleIncrement(scan);
  for (size_t i = 0; i < scan.size(); ++i) {
    const double r = scan.ranges[i];
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    const double a = scan.angle_min + static_cast<double>(i) * inc;
    ref_points_[i].point = Eigen::Vector2d(r * std::cos(a), r * std::sin(a));
    ref_points_[i].valid = true;
  }

  for (size_t i = 1; i < ref_points_.size(); ++i) {
    const auto& prev = ref_points_[i - 1];
    const auto& cur = ref_points_[i];
    if (!prev.valid || !cur.valid) continue;
    if ((cur.point - prev.point).norm() > params_.max_neighbor_gap) continue;
    ref_segments_.push_back({prev.point, cur.point});
  }
}

void MbICPEstimator::transformRobotMap(const Eigen::Matrix3d& inv_increment) {
  for (auto& ref : local_points_) {
    if (ref.valid) ref.point = transformPoint(inv_increment, ref.point);
  }
  for (auto& seg : local_segments_) {
    seg.a = transformPoint(inv_increment, seg.a);
    seg.b = transformPoint(inv_increment, seg.b);
  }
  local_map_index_valid_ = false;
}

void MbICPEstimator::addScanToLocalMap(const LaserScan& scan) {
  buildReferenceModel(scan);
  const double voxel = params_.local_map_voxel_size;

  std::unordered_map<int64_t, size_t> point_voxels;
  point_voxels.reserve(local_points_.size() + ref_points_.size());
  for (size_t i = 0; i < local_points_.size(); ++i) {
    if (!local_points_[i].valid) continue;
    point_voxels[voxelKey(local_points_[i].point.x(), local_points_[i].point.y(), voxel)] = i;
  }
  for (const auto& ref : ref_points_) {
    if (!ref.valid) continue;
    const int64_t k = voxelKey(ref.point.x(), ref.point.y(), voxel);
    const auto it = point_voxels.find(k);
    if (it != point_voxels.end()) {
      local_points_[it->second] = ref;
    } else {
      point_voxels[k] = local_points_.size();
      local_points_.push_back(ref);
    }
  }

  std::unordered_set<int64_t> segment_voxels;
  segment_voxels.reserve(local_segments_.size() + ref_segments_.size());
  for (const auto& seg : local_segments_) {
    const Eigen::Vector2d mid = 0.5 * (seg.a + seg.b);
    segment_voxels.insert(voxelKey(mid.x(), mid.y(), voxel));
  }
  for (const auto& seg : ref_segments_) {
    const Eigen::Vector2d mid = 0.5 * (seg.a + seg.b);
    const int64_t k = voxelKey(mid.x(), mid.y(), voxel);
    if (segment_voxels.insert(k).second) {
      local_segments_.push_back(seg);
    }
  }

  pruneLocalMap();
  rebuildLocalMapIndex();
}

void MbICPEstimator::pruneLocalMap() {
  if (params_.local_map_radius <= 0.0) return;
  const double r2 = params_.local_map_radius * params_.local_map_radius;
  local_points_.erase(std::remove_if(local_points_.begin(), local_points_.end(),
                                     [&](const RefPoint& ref) {
                                       return !ref.valid || ref.point.squaredNorm() > r2;
                                     }),
                      local_points_.end());
  local_segments_.erase(
      std::remove_if(local_segments_.begin(), local_segments_.end(),
                     [&](const Segment& seg) {
                       const Eigen::Vector2d mid = 0.5 * (seg.a + seg.b);
                       return mid.squaredNorm() > r2;
                     }),
      local_segments_.end());
  local_map_index_valid_ = false;
}

void MbICPEstimator::rebuildLocalMapIndex() {
  local_map_index_ =
      LocalMapIndex::build(local_segments_, local_points_,
                           std::max(params_.max_metric_distance, 0.5),
                           params_.max_metric_distance);
  local_map_index_valid_ = true;
}

double MbICPEstimator::metricDistanceSquared(const Eigen::Vector2d& delta,
                                             const Eigen::Vector2d& reference) const {
  const double L2 = params_.metric_radius * params_.metric_radius;
  const double k = reference.squaredNorm() + L2;
  if (k <= 1e-12) return delta.squaredNorm();
  const double cross = delta.x() * reference.y() - delta.y() * reference.x();
  return std::max(0.0, delta.squaredNorm() - (cross * cross) / k);
}

Eigen::Matrix2d MbICPEstimator::metricMatrix(const Eigen::Vector2d& reference) const {
  const double L2 = params_.metric_radius * params_.metric_radius;
  const double k = reference.squaredNorm() + L2;
  if (k <= 1e-12) return Eigen::Matrix2d::Identity();
  const Eigen::Vector2d a(reference.y(), -reference.x());
  return Eigen::Matrix2d::Identity() - (a * a.transpose()) / k;
}

bool MbICPEstimator::solveIncrement(const std::vector<Eigen::Vector2d>& current,
                                    const std::vector<RefPoint>& points,
                                    const std::vector<Segment>& segments,
                                    const Eigen::Matrix3d& transform,
                                    Eigen::Matrix3d* increment) const {
  struct Correspondence {
    Eigen::Vector2d current;
    Eigen::Vector2d reference;
    double metric_sq = 0.0;
  };

  std::vector<Correspondence> corr;
  corr.reserve(current.size());
  const double max_metric_sq = params_.max_metric_distance * params_.max_metric_distance;

  for (const auto& p_cur : current) {
    const Eigen::Vector2d p_tr = transformPoint(transform, p_cur);
    double best_metric_sq = max_metric_sq;
    Eigen::Vector2d best_ref = Eigen::Vector2d::Zero();
    bool found = false;

    for (const auto& seg : segments) {
      const Eigen::Vector2d ref = closestPointOnSegment(p_tr, seg.a, seg.b);
      const double d2 = metricDistanceSquared(p_tr - ref, ref);
      if (d2 < best_metric_sq) {
        best_metric_sq = d2;
        best_ref = ref;
        found = true;
      }
    }
    if (!found) {
      for (const auto& ref : points) {
        if (!ref.valid) continue;
        const double d2 = metricDistanceSquared(p_tr - ref.point, ref.point);
        if (d2 < best_metric_sq) {
          best_metric_sq = d2;
          best_ref = ref.point;
          found = true;
        }
      }
    }
    if (found) corr.push_back({p_tr, best_ref, best_metric_sq});
  }

  if (corr.size() < static_cast<size_t>(params_.min_correspondences)) return false;
  std::sort(corr.begin(), corr.end(), [](const auto& a, const auto& b) {
    return a.metric_sq < b.metric_sq;
  });
  size_t keep = corr.size();
  if (params_.trim_fraction > 0.0 && params_.trim_fraction < 1.0) {
    keep = static_cast<size_t>(std::ceil(params_.trim_fraction * static_cast<double>(corr.size())));
    keep = std::max(keep, static_cast<size_t>(params_.min_correspondences));
    keep = std::min(keep, corr.size());
  }

  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  for (size_t i = 0; i < keep; ++i) {
    const auto& c = corr[i];
    const Eigen::Vector2d e = c.current - c.reference;
    Eigen::Matrix<double, 2, 3> J;
    J << 1.0, 0.0, -c.current.y(), 0.0, 1.0, c.current.x();
    const Eigen::Matrix2d Q = metricMatrix(c.reference);
    A += J.transpose() * Q * J;
    b += J.transpose() * Q * e;
  }

  const Eigen::LDLT<Eigen::Matrix3d> ldlt(A);
  if (ldlt.info() != Eigen::Success) return false;
  const Eigen::Vector3d x = ldlt.solve(-b);
  if (!x.allFinite()) return false;
  *increment = matrixFromIncrement(x(0), x(1), x(2));
  return true;
}

bool MbICPEstimator::solveIncrementIndexed(const std::vector<Eigen::Vector2d>& current,
                                           const std::vector<RefPoint>& points,
                                           const std::vector<Segment>& segments,
                                           const LocalMapIndex& index,
                                           const Eigen::Matrix3d& transform,
                                           Eigen::Matrix3d* increment) const {
  struct Correspondence {
    Eigen::Vector2d current;
    Eigen::Vector2d reference;
    double metric_sq = 0.0;
  };

  std::vector<Correspondence> corr;
  corr.reserve(current.size());
  const double max_metric_sq = params_.max_metric_distance * params_.max_metric_distance;
  std::unordered_set<size_t> segment_ids;
  std::unordered_set<size_t> point_ids;
  segment_ids.reserve(32);
  point_ids.reserve(32);

  for (const auto& p_cur : current) {
    const Eigen::Vector2d p_tr = transformPoint(transform, p_cur);
    double best_metric_sq = max_metric_sq;
    Eigen::Vector2d best_ref = Eigen::Vector2d::Zero();
    bool found = false;

    segment_ids.clear();
    index.querySegments(p_tr, &segment_ids);
    for (const size_t sid : segment_ids) {
      const auto& seg = segments[sid];
      const Eigen::Vector2d ref = closestPointOnSegment(p_tr, seg.a, seg.b);
      const double d2 = metricDistanceSquared(p_tr - ref, ref);
      if (d2 < best_metric_sq) {
        best_metric_sq = d2;
        best_ref = ref;
        found = true;
      }
    }
    if (!found) {
      point_ids.clear();
      index.queryPoints(p_tr, &point_ids);
      for (const size_t pid : point_ids) {
        const auto& ref = points[pid];
        if (!ref.valid) continue;
        const double d2 = metricDistanceSquared(p_tr - ref.point, ref.point);
        if (d2 < best_metric_sq) {
          best_metric_sq = d2;
          best_ref = ref.point;
          found = true;
        }
      }
    }
    if (found) corr.push_back({p_tr, best_ref, best_metric_sq});
  }

  if (corr.size() < static_cast<size_t>(params_.min_correspondences)) return false;
  std::sort(corr.begin(), corr.end(), [](const auto& a, const auto& b) {
    return a.metric_sq < b.metric_sq;
  });
  size_t keep = corr.size();
  if (params_.trim_fraction > 0.0 && params_.trim_fraction < 1.0) {
    keep = static_cast<size_t>(std::ceil(params_.trim_fraction * static_cast<double>(corr.size())));
    keep = std::max(keep, static_cast<size_t>(params_.min_correspondences));
    keep = std::min(keep, corr.size());
  }

  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  for (size_t i = 0; i < keep; ++i) {
    const auto& c = corr[i];
    const Eigen::Vector2d e = c.current - c.reference;
    Eigen::Matrix<double, 2, 3> J;
    J << 1.0, 0.0, -c.current.y(), 0.0, 1.0, c.current.x();
    const Eigen::Matrix2d Q = metricMatrix(c.reference);
    A += J.transpose() * Q * J;
    b += J.transpose() * Q * e;
  }

  const Eigen::LDLT<Eigen::Matrix3d> ldlt(A);
  if (ldlt.info() != Eigen::Success) return false;
  const Eigen::Vector3d x = ldlt.solve(-b);
  if (!x.allFinite()) return false;
  *increment = matrixFromIncrement(x(0), x(1), x(2));
  return true;
}

MbICPResult MbICPEstimator::registerScan(const LaserScan& scan) {
  MbICPResult result;
  result.pose = pose_;

  const auto current = scanToPoints(scan);
  if (current.size() < static_cast<size_t>(params_.min_correspondences)) {
    return result;
  }

  if (!initialized_) {
    if (params_.use_local_map) {
      addScanToLocalMap(scan);
    } else {
      buildReferenceModel(scan);
    }
    initialized_ = true;
    result.valid = true;
    result.pose = pose_;
    return result;
  }

  Eigen::Matrix3d transform =
      params_.use_motion_prior ? last_increment_ : Eigen::Matrix3d::Identity();
  const std::vector<RefPoint>& map_points = params_.use_local_map ? local_points_ : ref_points_;
  const std::vector<Segment>& map_segments =
      params_.use_local_map ? local_segments_ : ref_segments_;
  if (params_.use_local_map && !local_map_index_valid_) {
    rebuildLocalMapIndex();
  }

  int iterations = 0;
  for (; iterations < params_.max_iterations; ++iterations) {
    Eigen::Matrix3d delta = Eigen::Matrix3d::Identity();
    const bool ok =
        params_.use_local_map
            ? solveIncrementIndexed(current, map_points, map_segments, local_map_index_,
                                    transform, &delta)
            : solveIncrement(current, map_points, map_segments, transform, &delta);
    if (!ok) break;
    transform = delta * transform;

    const double tx = delta(0, 2);
    const double ty = delta(1, 2);
    const double dyaw = std::atan2(delta(1, 0), delta(0, 0));
    if (std::hypot(tx, ty) < params_.convergence_translation &&
        std::abs(dyaw) < params_.convergence_rotation) {
      ++iterations;
      break;
    }
  }

  pose_ = pose_ * transform;
  last_increment_ = transform;
  if (params_.use_local_map) {
    transformRobotMap(transform.inverse());
    addScanToLocalMap(scan);
  } else {
    buildReferenceModel(scan);
  }
  result.increment = transform;
  result.pose = pose_;
  result.valid = true;
  result.iterations = iterations;
  return result;
}

}  // namespace mb_icp
}  // namespace localization_zoo
