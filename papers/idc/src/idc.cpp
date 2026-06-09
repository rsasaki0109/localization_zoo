#include "idc/idc.h"

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace idc {

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

double normalizeAngle(double a) {
  while (a > M_PI) a -= 2.0 * M_PI;
  while (a < -M_PI) a += 2.0 * M_PI;
  return a;
}

}  // namespace

Eigen::Matrix3d pose2D(double x, double y, double yaw) {
  return matrixFromIncrement(x, y, yaw);
}

int64_t IDCEstimator::voxelKey(double x, double y, double voxel_size) {
  const int ix = static_cast<int>(std::floor(x / voxel_size));
  const int iy = static_cast<int>(std::floor(y / voxel_size));
  return (static_cast<int64_t>(ix) << 32) ^ static_cast<uint32_t>(static_cast<uint32_t>(iy));
}

IDCEstimator::LocalMapIndex IDCEstimator::LocalMapIndex::build(
    const std::vector<RefPoint>& refs, double cell_size, double query_radius) {
  LocalMapIndex index;
  index.cell_size = std::max(cell_size, 1e-3);
  index.query_radius = std::max(query_radius, index.cell_size);
  const auto key = [](int ix, int iy) {
    return (static_cast<int64_t>(ix) << 32) ^ static_cast<uint32_t>(iy);
  };
  for (size_t i = 0; i < refs.size(); ++i) {
    if (!refs[i].valid) continue;
    const int ix = static_cast<int>(std::floor(refs[i].point.x() / index.cell_size));
    const int iy = static_cast<int>(std::floor(refs[i].point.y() / index.cell_size));
    index.bins[key(ix, iy)].push_back(i);
  }
  return index;
}

void IDCEstimator::LocalMapIndex::queryCandidates(const Eigen::Vector2d& p,
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

IDCEstimator::IDCEstimator(const IDCParams& params) : params_(params) {}

void IDCEstimator::reset() {
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

void IDCEstimator::setInitialPose(const Eigen::Matrix3d& pose) { pose_ = pose; }

double IDCEstimator::angleIncrement(const LaserScan& scan) const {
  if (scan.angle_increment > 0.0) return scan.angle_increment;
  if (scan.size() > 1) {
    return (scan.angle_max - scan.angle_min) / static_cast<double>(scan.size() - 1);
  }
  return 0.0;
}

Eigen::Vector2d IDCEstimator::transformPoint(const Eigen::Matrix3d& T,
                                             const Eigen::Vector2d& p) {
  return (T.block<2, 2>(0, 0) * p + T.block<2, 1>(0, 2)).eval();
}

std::vector<IDCEstimator::Sample> IDCEstimator::scanToSamples(const LaserScan& scan) const {
  std::vector<Sample> samples;
  samples.reserve(scan.size());
  const double inc = angleIncrement(scan);
  for (size_t i = 0; i < scan.size(); ++i) {
    const double r = scan.ranges[i];
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    const double a = scan.angle_min + static_cast<double>(i) * inc;
    samples.push_back({Eigen::Vector2d(r * std::cos(a), r * std::sin(a)), r, a,
                       static_cast<int>(i)});
  }
  return samples;
}

std::vector<IDCEstimator::RefPoint> IDCEstimator::buildReferenceModel(
    const LaserScan& scan) const {
  const double inc = angleIncrement(scan);
  std::vector<RefPoint> model(scan.size());
  for (size_t i = 0; i < scan.size(); ++i) {
    const double r = scan.ranges[i];
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    const double a = scan.angle_min + static_cast<double>(i) * inc;
    model[i].point = Eigen::Vector2d(r * std::cos(a), r * std::sin(a));
    model[i].range = r;
    model[i].bearing = a;
    model[i].beam = static_cast<int>(i);
    model[i].valid = true;
  }
  return model;
}

void IDCEstimator::transformRobotMap(const Eigen::Matrix3d& inv_increment) {
  for (auto& ref : local_refs_) {
    if (!ref.valid) continue;
    ref.point = transformPoint(inv_increment, ref.point);
    ref.range = ref.point.norm();
    ref.bearing = std::atan2(ref.point.y(), ref.point.x());
  }
  local_map_index_valid_ = false;
}

void IDCEstimator::rebuildPointVoxels() {
  point_voxels_.clear();
  const double voxel = params_.local_map_voxel_size;
  point_voxels_.reserve(local_refs_.size());
  for (size_t i = 0; i < local_refs_.size(); ++i) {
    if (!local_refs_[i].valid) continue;
    point_voxels_[voxelKey(local_refs_[i].point.x(), local_refs_[i].point.y(), voxel)] = i;
  }
}

void IDCEstimator::addScanToLocalMap(const LaserScan& scan) {
  const auto model = buildReferenceModel(scan);
  const double voxel = params_.local_map_voxel_size;

  rebuildPointVoxels();
  for (const auto& ref : model) {
    if (!ref.valid) continue;
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

void IDCEstimator::pruneLocalMap() {
  if (params_.local_map_radius <= 0.0) return;
  const double r2 = params_.local_map_radius * params_.local_map_radius;
  local_refs_.erase(std::remove_if(local_refs_.begin(), local_refs_.end(),
                                   [&](const RefPoint& ref) {
                                     return !ref.valid || ref.point.squaredNorm() > r2;
                                   }),
                    local_refs_.end());
  local_map_index_valid_ = false;
}

void IDCEstimator::rebuildLocalMapIndex() {
  local_map_index_ = LocalMapIndex::build(local_refs_, std::max(0.5, params_.local_map_voxel_size),
                                          params_.max_cp_distance);
  local_map_index_valid_ = true;
}

bool IDCEstimator::solveCP(const std::vector<Sample>& current, const std::vector<RefPoint>& refs,
                           const Eigen::Matrix3d& transform, double* tx, double* ty) const {
  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  int used = 0;

  for (const auto& sample : current) {
    const Eigen::Vector2d p_tr = transformPoint(transform, sample.point);
    double best_dist = params_.max_cp_distance;
    const RefPoint* best = nullptr;
    for (const auto& ref : refs) {
      if (!ref.valid) continue;
      const double d = (ref.point - p_tr).norm();
      if (d < best_dist) {
        best_dist = d;
        best = &ref;
      }
    }
    if (best == nullptr) continue;

    const Eigen::Vector2d err = p_tr - best->point;
    Eigen::Matrix<double, 2, 3> J;
    J << 1.0, 0.0, -p_tr.y(), 0.0, 1.0, p_tr.x();
    A += J.transpose() * J;
    b += J.transpose() * err;
    ++used;
  }

  if (used < params_.min_correspondences) return false;
  const Eigen::Vector3d x = A.ldlt().solve(-b);
  if (!x.allFinite()) return false;
  *tx = x(0);
  *ty = x(1);
  return true;
}

bool IDCEstimator::solveCPIndexed(const std::vector<Sample>& current,
                                  const std::vector<RefPoint>& refs, const LocalMapIndex& index,
                                  const Eigen::Matrix3d& transform, double* tx, double* ty) const {
  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  int used = 0;
  if (query_stamp_.size() < refs.size()) {
    query_stamp_.assign(refs.size(), 0);
  }
  const double max_dist = params_.max_cp_distance;
  const double max_dist2 = max_dist * max_dist;
  std::vector<size_t> candidates;
  candidates.reserve(32);

  for (const auto& sample : current) {
    if (++query_generation_ == 0) {
      std::fill(query_stamp_.begin(), query_stamp_.end(), 0);
      query_generation_ = 1;
    }
    const uint32_t generation = query_generation_;
    const Eigen::Vector2d p_tr = transformPoint(transform, sample.point);
    double best_dist2 = max_dist2;
    const RefPoint* best = nullptr;

    candidates.clear();
    index.queryCandidates(p_tr, query_stamp_, generation, &candidates);
    for (const size_t rid : candidates) {
      query_stamp_[rid] = generation;
      const auto& ref = refs[rid];
      if (!ref.valid) continue;
      const double d2 = (ref.point - p_tr).squaredNorm();
      if (d2 < best_dist2) {
        best_dist2 = d2;
        best = &ref;
      }
    }
    if (best == nullptr) continue;

    const Eigen::Vector2d err = p_tr - best->point;
    Eigen::Matrix<double, 2, 3> J;
    J << 1.0, 0.0, -p_tr.y(), 0.0, 1.0, p_tr.x();
    A += J.transpose() * J;
    b += J.transpose() * err;
    ++used;
  }

  if (used < params_.min_correspondences) return false;
  const Eigen::Vector3d x = A.ldlt().solve(-b);
  if (!x.allFinite()) return false;
  *tx = x(0);
  *ty = x(1);
  return true;
}

bool IDCEstimator::solveRRScanToScan(const std::vector<Sample>& current,
                                     const std::vector<RefPoint>& refs,
                                     const Eigen::Matrix3d& transform, double* yaw) const {
  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  int used = 0;

  for (const auto& sample : current) {
    const Eigen::Vector2d p_tr = transformPoint(transform, sample.point);
    const double range_tr = p_tr.norm();

    double best_diff = params_.max_range_diff;
    const RefPoint* best = nullptr;
    const int w = params_.range_match_beam_window;
    const int n = static_cast<int>(refs.size());
    for (int j = std::max(0, sample.beam - w); j < std::min(n, sample.beam + w + 1); ++j) {
      const auto& ref = refs[static_cast<size_t>(j)];
      if (!ref.valid) continue;
      const double diff = std::abs(range_tr - ref.range);
      if (diff < best_diff) {
        best_diff = diff;
        best = &ref;
      }
    }
    if (best == nullptr) continue;

    const Eigen::Vector2d err = p_tr - best->point;
    Eigen::Matrix<double, 2, 3> J;
    J << 1.0, 0.0, -p_tr.y(), 0.0, 1.0, p_tr.x();
    A += J.transpose() * J;
    b += J.transpose() * err;
    ++used;
  }

  if (used < params_.min_correspondences) return false;
  const Eigen::Vector3d x = A.ldlt().solve(-b);
  if (!x.allFinite()) return false;
  *yaw = x(2);
  return true;
}

bool IDCEstimator::solveRRLocalMap(const std::vector<Sample>& current,
                                   const std::vector<RefPoint>& refs, const LaserScan& scan,
                                   const Eigen::Matrix3d& transform, double* yaw) const {
  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  int used = 0;
  const double inc = angleIncrement(scan);
  const double angle_window = params_.range_match_beam_window * inc;

  for (const auto& sample : current) {
    const Eigen::Vector2d p_tr = transformPoint(transform, sample.point);
    const double range_tr = p_tr.norm();

    double best_diff = params_.max_range_diff;
    const RefPoint* best = nullptr;
    for (const auto& ref : refs) {
      if (!ref.valid) continue;
      if (std::abs(normalizeAngle(ref.bearing - sample.bearing)) > angle_window) continue;
      const double diff = std::abs(range_tr - ref.range);
      if (diff < best_diff) {
        best_diff = diff;
        best = &ref;
      }
    }
    if (best == nullptr) continue;

    const Eigen::Vector2d err = p_tr - best->point;
    Eigen::Matrix<double, 2, 3> J;
    J << 1.0, 0.0, -p_tr.y(), 0.0, 1.0, p_tr.x();
    A += J.transpose() * J;
    b += J.transpose() * err;
    ++used;
  }

  if (used < params_.min_correspondences) return false;
  const Eigen::Vector3d x = A.ldlt().solve(-b);
  if (!x.allFinite()) return false;
  *yaw = x(2);
  return true;
}

IDCResult IDCEstimator::registerScan(const LaserScan& scan) {
  IDCResult result;
  result.pose = pose_;

  const auto current = scanToSamples(scan);
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
  const std::vector<RefPoint>& refs = params_.use_local_map ? local_refs_ : ref_model_;
  if (params_.use_local_map && !local_map_index_valid_) {
    rebuildLocalMapIndex();
  }

  int iterations = 0;

  for (; iterations < params_.max_iterations; ++iterations) {
    double tx = 0.0;
    double ty = 0.0;
    double dyaw = 0.0;
    const bool cp_ok =
        params_.use_local_map
            ? solveCPIndexed(current, refs, local_map_index_, transform, &tx, &ty)
            : solveCP(current, refs, transform, &tx, &ty);
    const bool rr_ok = params_.use_local_map
                           ? solveRRLocalMap(current, refs, scan, transform, &dyaw)
                           : solveRRScanToScan(current, refs, transform, &dyaw);
    if (!cp_ok && !rr_ok) break;

    const double use_tx = cp_ok ? tx : 0.0;
    const double use_ty = cp_ok ? ty : 0.0;
    const double use_yaw = rr_ok ? dyaw : 0.0;
    const Eigen::Matrix3d delta = matrixFromIncrement(use_tx, use_ty, use_yaw);
    transform = delta * transform;

    if (std::hypot(use_tx, use_ty) < params_.convergence_translation &&
        std::abs(use_yaw) < params_.convergence_rotation) {
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

}  // namespace idc
}  // namespace localization_zoo
