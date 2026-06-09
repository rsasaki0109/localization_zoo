#include "pl_icp/pl_icp.h"

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace pl_icp {

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

}  // namespace

Eigen::Matrix3d pose2D(double x, double y, double yaw) {
  return matrixFromIncrement(x, y, yaw);
}

int64_t PLICPEstimator::voxelKey(double x, double y, double voxel_size) {
  const int ix = static_cast<int>(std::floor(x / voxel_size));
  const int iy = static_cast<int>(std::floor(y / voxel_size));
  return (static_cast<int64_t>(ix) << 32) ^ static_cast<uint32_t>(static_cast<uint32_t>(iy));
}

PLICPEstimator::LocalMapIndex PLICPEstimator::LocalMapIndex::build(
    const std::vector<RefPoint>& refs, double cell_size, double query_radius) {
  LocalMapIndex index;
  index.cell_size = std::max(cell_size, 1e-3);
  index.query_radius = std::max(query_radius, index.cell_size);
  const auto key = [](int ix, int iy) {
    return (static_cast<int64_t>(ix) << 32) ^ static_cast<uint32_t>(iy);
  };
  for (size_t i = 0; i < refs.size(); ++i) {
    if (!refs[i].valid_line) continue;
    const int ix = static_cast<int>(std::floor(refs[i].point.x() / index.cell_size));
    const int iy = static_cast<int>(std::floor(refs[i].point.y() / index.cell_size));
    index.bins[key(ix, iy)].push_back(i);
  }
  return index;
}

void PLICPEstimator::LocalMapIndex::queryCandidates(const Eigen::Vector2d& p,
                                                    const std::vector<uint32_t>& stamp,
                                                    uint32_t generation,
                                                    std::vector<size_t>* candidates) const {
  const int cx = static_cast<int>(std::floor(p.x() / cell_size));
  const int cy = static_cast<int>(std::floor(p.y() / cell_size));
  const int reach = static_cast<int>(std::ceil(query_radius / cell_size)) + 1;
  for (int dx = -reach; dx <= reach; ++dx) {
    for (int dy = -reach; dy <= reach; ++dy) {
      const int64_t k = (static_cast<int64_t>(cx + dx) << 32) ^ static_cast<uint32_t>(cy + dy);
      const auto it = bins.find(k);
      if (it == bins.end()) continue;
      for (const size_t rid : it->second) {
        if (rid >= stamp.size() || stamp[rid] == generation) continue;
        candidates->push_back(rid);
      }
    }
  }
}

PLICPEstimator::PLICPEstimator(const PLICPParams& params) : params_(params) {}

void PLICPEstimator::reset() {
  initialized_ = false;
  ref_model_.clear();
  local_refs_.clear();
  point_voxels_.clear();
  local_map_index_ = LocalMapIndex{};
  local_map_index_valid_ = false;
  query_stamp_.clear();
  query_generation_ = 1;
  pose_ = Eigen::Matrix3d::Identity();
  last_increment_ = Eigen::Matrix3d::Identity();
}

void PLICPEstimator::setInitialPose(const Eigen::Matrix3d& pose) { pose_ = pose; }

Eigen::Vector2d PLICPEstimator::transformPoint(const Eigen::Matrix3d& T,
                                               const Eigen::Vector2d& p) {
  return (T.block<2, 2>(0, 0) * p + T.block<2, 1>(0, 2)).eval();
}

std::vector<Eigen::Vector2d> PLICPEstimator::scanToPoints(const LaserScan& scan) const {
  std::vector<Eigen::Vector2d> points;
  points.reserve(scan.size());
  const double inc =
      scan.angle_increment > 0.0 ? scan.angle_increment
                                 : (scan.size() > 1 ? (scan.angle_max - scan.angle_min) /
                                                          static_cast<double>(scan.size() - 1)
                                                    : 0.0);
  for (size_t i = 0; i < scan.size(); ++i) {
    const double r = scan.ranges[i];
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    const double a = scan.angle_min + static_cast<double>(i) * inc;
    points.emplace_back(r * std::cos(a), r * std::sin(a));
  }
  return points;
}

std::vector<PLICPEstimator::RefPoint> PLICPEstimator::buildReferenceModel(
    const LaserScan& scan) const {
  const double inc =
      scan.angle_increment > 0.0 ? scan.angle_increment
                                 : (scan.size() > 1 ? (scan.angle_max - scan.angle_min) /
                                                          static_cast<double>(scan.size() - 1)
                                                    : 0.0);
  std::vector<RefPoint> model(scan.size());
  for (size_t i = 0; i < scan.size(); ++i) {
    const double r = scan.ranges[i];
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    const double a = scan.angle_min + static_cast<double>(i) * inc;
    model[i].point = Eigen::Vector2d(r * std::cos(a), r * std::sin(a));
    model[i].valid_point = true;
  }

  for (size_t i = 0; i < scan.size(); ++i) {
    if (!model[i].valid_point) continue;
    int prev = static_cast<int>(i) - 1;
    int next = static_cast<int>(i) + 1;
    while (prev >= 0 && !model[static_cast<size_t>(prev)].valid_point) --prev;
    while (next < static_cast<int>(scan.size()) && !model[static_cast<size_t>(next)].valid_point) {
      ++next;
    }
    if (prev < 0 || next >= static_cast<int>(scan.size())) continue;
    const Eigen::Vector2d tangent =
        model[static_cast<size_t>(next)].point - model[static_cast<size_t>(prev)].point;
    const double len = tangent.norm();
    if (len < 1e-3 || len > params_.max_neighbor_gap) continue;
    model[i].normal = Eigen::Vector2d(-tangent.y(), tangent.x()) / len;
    model[i].valid_line = true;
  }
  return model;
}

void PLICPEstimator::transformRobotMap(const Eigen::Matrix3d& inv_increment) {
  const Eigen::Matrix2d R = inv_increment.block<2, 2>(0, 0);
  for (auto& ref : local_refs_) {
    ref.point = transformPoint(inv_increment, ref.point);
    ref.normal = (R * ref.normal).normalized();
  }
  local_map_index_valid_ = false;
}

void PLICPEstimator::rebuildPointVoxels() {
  point_voxels_.clear();
  const double voxel = params_.local_map_voxel_size;
  point_voxels_.reserve(local_refs_.size());
  for (size_t i = 0; i < local_refs_.size(); ++i) {
    if (!local_refs_[i].valid_line) continue;
    point_voxels_[voxelKey(local_refs_[i].point.x(), local_refs_[i].point.y(), voxel)] = i;
  }
}

void PLICPEstimator::addScanToLocalMap(const LaserScan& scan) {
  const auto model = buildReferenceModel(scan);
  const double voxel = params_.local_map_voxel_size;

  rebuildPointVoxels();
  for (const auto& ref : model) {
    if (!ref.valid_line) continue;
    const int64_t k = voxelKey(ref.point.x(), ref.point.y(), voxel);
    const auto it = point_voxels_.find(k);
    if (it != point_voxels_.end()) {
      local_refs_[it->second] = ref;
    } else {
      point_voxels_[k] = local_refs_.size();
      local_refs_.push_back(ref);
    }
  }

  const size_t before = local_refs_.size();
  pruneLocalMap();
  if (local_refs_.size() != before) {
    rebuildPointVoxels();
  }
  rebuildLocalMapIndex();
}

void PLICPEstimator::pruneLocalMap() {
  if (params_.local_map_radius <= 0.0) return;
  const double r2 = params_.local_map_radius * params_.local_map_radius;
  local_refs_.erase(std::remove_if(local_refs_.begin(), local_refs_.end(),
                                   [&](const RefPoint& ref) {
                                     return !ref.valid_line || ref.point.squaredNorm() > r2;
                                   }),
                    local_refs_.end());
  local_map_index_valid_ = false;
}

void PLICPEstimator::rebuildLocalMapIndex() {
  local_map_index_ = LocalMapIndex::build(local_refs_, std::max(0.5, params_.local_map_voxel_size),
                                          params_.max_correspondence_distance);
  local_map_index_valid_ = true;
}

bool PLICPEstimator::solveIncrement(const std::vector<Eigen::Vector2d>& current,
                                    const std::vector<RefPoint>& references,
                                    const Eigen::Matrix3d& transform,
                                    Eigen::Matrix3d* increment) const {
  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  int used = 0;

  for (const auto& p_cur : current) {
    const Eigen::Vector2d p_tr = transformPoint(transform, p_cur);
    double best_dist = params_.max_correspondence_distance;
    const RefPoint* best = nullptr;
    for (const auto& ref : references) {
      if (!ref.valid_line) continue;
      const double d = (ref.point - p_tr).norm();
      if (d < best_dist) {
        best_dist = d;
        best = &ref;
      }
    }
    if (best == nullptr) continue;

    const Eigen::Vector2d n = best->normal;
    const double err = n.dot(p_tr - best->point);
    Eigen::Vector3d row;
    row(0) = n.x();
    row(1) = n.y();
    row(2) = -n.x() * p_tr.y() + n.y() * p_tr.x();
    A += row * row.transpose();
    b += -err * row;
    ++used;
  }

  if (used < params_.min_correspondences) return false;
  const Eigen::Vector3d x = A.ldlt().solve(b);
  if (!x.allFinite()) return false;
  *increment = matrixFromIncrement(x(0), x(1), x(2));
  return true;
}

bool PLICPEstimator::solveIncrementIndexed(const std::vector<Eigen::Vector2d>& current,
                                           const std::vector<RefPoint>& references,
                                           const LocalMapIndex& index,
                                           const Eigen::Matrix3d& transform,
                                           Eigen::Matrix3d* increment) const {
  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  int used = 0;
  if (query_stamp_.size() < references.size()) {
    query_stamp_.assign(references.size(), 0);
  }
  const double max_dist = params_.max_correspondence_distance;
  const double max_dist2 = max_dist * max_dist;
  std::vector<size_t> candidates;
  candidates.reserve(32);

  for (const auto& p_cur : current) {
    if (++query_generation_ == 0) {
      std::fill(query_stamp_.begin(), query_stamp_.end(), 0);
      query_generation_ = 1;
    }
    const uint32_t generation = query_generation_;
    const Eigen::Vector2d p_tr = transformPoint(transform, p_cur);
    double best_dist2 = max_dist2;
    const RefPoint* best = nullptr;

    candidates.clear();
    index.queryCandidates(p_tr, query_stamp_, generation, &candidates);
    for (const size_t rid : candidates) {
      query_stamp_[rid] = generation;
      const auto& ref = references[rid];
      if (!ref.valid_line) continue;
      const double d2 = (ref.point - p_tr).squaredNorm();
      if (d2 < best_dist2) {
        best_dist2 = d2;
        best = &ref;
      }
    }
    if (best == nullptr) continue;

    const Eigen::Vector2d n = best->normal;
    const double err = n.dot(p_tr - best->point);
    Eigen::Vector3d row;
    row(0) = n.x();
    row(1) = n.y();
    row(2) = -n.x() * p_tr.y() + n.y() * p_tr.x();
    A += row * row.transpose();
    b += -err * row;
    ++used;
  }

  if (used < params_.min_correspondences) return false;
  const Eigen::Vector3d x = A.ldlt().solve(b);
  if (!x.allFinite()) return false;
  *increment = matrixFromIncrement(x(0), x(1), x(2));
  return true;
}

PLICPResult PLICPEstimator::registerScan(const LaserScan& scan) {
  PLICPResult result;
  result.pose = pose_;

  const auto current = scanToPoints(scan);
  if (current.size() < static_cast<size_t>(params_.min_correspondences)) {
    return result;
  }

  if (!initialized_) {
    if (params_.use_local_map) {
      addScanToLocalMap(scan);
    } else {
      ref_model_ = buildReferenceModel(scan);
    }
    initialized_ = true;
    result.valid = true;
    result.pose = pose_;
    return result;
  }

  Eigen::Matrix3d transform =
      params_.use_motion_prior ? last_increment_ : Eigen::Matrix3d::Identity();
  const std::vector<RefPoint>& references = params_.use_local_map ? local_refs_ : ref_model_;
  if (params_.use_local_map && !local_map_index_valid_) {
    rebuildLocalMapIndex();
  }

  int iterations = 0;
  for (; iterations < params_.max_iterations; ++iterations) {
    Eigen::Matrix3d delta = Eigen::Matrix3d::Identity();
    const bool ok =
        params_.use_local_map
            ? solveIncrementIndexed(current, references, local_map_index_, transform, &delta)
            : solveIncrement(current, references, transform, &delta);
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
    ref_model_ = buildReferenceModel(scan);
  }
  result.increment = transform;
  result.pose = pose_;
  result.valid = true;
  result.iterations = iterations;
  return result;
}

}  // namespace pl_icp
}  // namespace localization_zoo
