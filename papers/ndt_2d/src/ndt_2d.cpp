#include "ndt_2d/ndt_2d.h"

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace ndt_2d {

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

struct CellAccum {
  Eigen::Vector2d sum = Eigen::Vector2d::Zero();
  Eigen::Matrix2d sum_outer = Eigen::Matrix2d::Zero();
  int count = 0;
};

}  // namespace

Eigen::Matrix3d pose2D(double x, double y, double yaw) {
  return matrixFromIncrement(x, y, yaw);
}

NDT2DEstimator::NDT2DEstimator(const NDT2DParams& params) : params_(params) {}

void NDT2DEstimator::reset() {
  initialized_ = false;
  local_points_.clear();
  ref_points_.clear();
  point_voxels_.clear();
  ref_map_ = {};
  pose_ = Eigen::Matrix3d::Identity();
  last_increment_ = Eigen::Matrix3d::Identity();
}

void NDT2DEstimator::setInitialPose(const Eigen::Matrix3d& pose) { pose_ = pose; }

int64_t NDT2DEstimator::cellKey(int ix, int iy) const {
  return (static_cast<int64_t>(ix) << 32) ^
         static_cast<uint32_t>(static_cast<uint32_t>(iy));
}

void NDT2DEstimator::cellIndex(const Eigen::Vector2d& p, const Eigen::Vector2d& map_origin,
                               double cell_size, int* ix, int* iy) const {
  const Eigen::Vector2d q = p - map_origin;
  *ix = static_cast<int>(std::floor(q.x() / cell_size));
  *iy = static_cast<int>(std::floor(q.y() / cell_size));
}

int64_t NDT2DEstimator::voxelKey(double x, double y, double voxel_size) {
  const int ix = static_cast<int>(std::floor(x / voxel_size));
  const int iy = static_cast<int>(std::floor(y / voxel_size));
  return (static_cast<int64_t>(ix) << 32) ^ static_cast<uint32_t>(static_cast<uint32_t>(iy));
}

void NDT2DEstimator::rebuildPointVoxels() {
  point_voxels_.clear();
  const double voxel = params_.local_map_voxel_size;
  point_voxels_.reserve(local_points_.size());
  for (size_t i = 0; i < local_points_.size(); ++i) {
    point_voxels_[voxelKey(local_points_[i].x(), local_points_[i].y(), voxel)] = i;
  }
}

void NDT2DEstimator::addScanToLocalMap(const std::vector<Eigen::Vector2d>& points) {
  const double voxel = params_.local_map_voxel_size;
  rebuildPointVoxels();
  for (const auto& p : points) {
    const int64_t k = voxelKey(p.x(), p.y(), voxel);
    const auto it = point_voxels_.find(k);
    if (it != point_voxels_.end()) {
      local_points_[it->second] = p;
    } else {
      point_voxels_[k] = local_points_.size();
      local_points_.push_back(p);
    }
  }
  const size_t before = local_points_.size();
  pruneLocalMap();
  if (local_points_.size() != before) {
    rebuildPointVoxels();
  }
}

void NDT2DEstimator::transformRobotMap(const Eigen::Matrix3d& inv_increment) {
  for (auto& p : local_points_) {
    p = transformPoint(inv_increment, p);
  }
}

void NDT2DEstimator::pruneLocalMap() {
  if (params_.local_map_radius <= 0.0) return;
  const double r2 = params_.local_map_radius * params_.local_map_radius;
  local_points_.erase(std::remove_if(local_points_.begin(), local_points_.end(),
                                     [&](const Eigen::Vector2d& p) { return p.squaredNorm() > r2; }),
                      local_points_.end());
}

std::vector<Eigen::Vector2d> NDT2DEstimator::scanToPoints(const LaserScan& scan) const {
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

NDT2DEstimator::NDTMap NDT2DEstimator::buildNDTMap(
    const std::vector<Eigen::Vector2d>& points, double cell_size) const {
  NDTMap out;
  if (points.empty()) return out;

  Eigen::Vector2d min_p = points.front();
  Eigen::Vector2d max_p = points.front();
  for (const auto& p : points) {
    min_p = min_p.cwiseMin(p);
    max_p = max_p.cwiseMax(p);
  }
  out.origin = min_p - Eigen::Vector2d::Constant(cell_size);

  std::unordered_map<int64_t, CellAccum> accum;
  accum.reserve(points.size());
  for (const auto& p : points) {
    int ix = 0;
    int iy = 0;
    cellIndex(p, out.origin, cell_size, &ix, &iy);
    auto& cell = accum[cellKey(ix, iy)];
    cell.sum += p;
    cell.sum_outer += p * p.transpose();
    ++cell.count;
  }

  out.cells.reserve(accum.size());
  for (const auto& [key, acc] : accum) {
    if (acc.count < params_.min_points_per_cell) continue;
    Cell cell;
    cell.count = acc.count;
    cell.mean = acc.sum / static_cast<double>(acc.count);
    Eigen::Matrix2d cov =
        acc.sum_outer / static_cast<double>(acc.count) - cell.mean * cell.mean.transpose();
    cov += Eigen::Matrix2d::Identity() * params_.cov_regularization;
    const double det = cov.determinant();
    if (det < 1e-9) continue;
    cell.inv_cov = cov.inverse();
    cell.valid = true;
    out.cells.emplace(key, cell);
  }
  return out;
}

const NDT2DEstimator::Cell* NDT2DEstimator::lookupCell(const CellMap& map,
                                                       const Eigen::Vector2d& p,
                                                       const Eigen::Vector2d& map_origin,
                                                       double cell_size) const {
  int ix = 0;
  int iy = 0;
  cellIndex(p, map_origin, cell_size, &ix, &iy);
  const Cell* best = nullptr;
  double best_mahal = std::numeric_limits<double>::infinity();
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      const auto it = map.find(cellKey(ix + dx, iy + dy));
      if (it == map.end() || !it->second.valid) continue;
      const Eigen::Vector2d r = p - it->second.mean;
      const double mahal = r.transpose() * it->second.inv_cov * r;
      if (mahal < best_mahal) {
        best_mahal = mahal;
        best = &it->second;
      }
    }
  }
  return best;
}

Eigen::Vector2d NDT2DEstimator::transformPoint(const Eigen::Matrix3d& T,
                                               const Eigen::Vector2d& p) {
  return (T.block<2, 2>(0, 0) * p + T.block<2, 1>(0, 2)).eval();
}

bool NDT2DEstimator::solveIncrement(const std::vector<Eigen::Vector2d>& current,
                                    const CellMap& map, const Eigen::Matrix3d& transform,
                                    const Eigen::Vector2d& map_origin, double cell_size,
                                    Eigen::Matrix3d* increment, double* score) const {
  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  double total_score = 0.0;
  int used = 0;

  const double c = transform(0, 0);
  const double s = transform(1, 0);

  for (const auto& p : current) {
    const Eigen::Vector2d pw = transformPoint(transform, p);
    const Cell* cell = lookupCell(map, pw, map_origin, cell_size);
    if (cell == nullptr) continue;

    const Eigen::Vector2d r = pw - cell->mean;
    const double mahal = r.transpose() * cell->inv_cov * r;
    total_score += std::exp(-0.5 * mahal);

    Eigen::Matrix<double, 2, 3> J = Eigen::Matrix<double, 2, 3>::Zero();
    J(0, 0) = 1.0;
    J(1, 1) = 1.0;
    J(0, 2) = -s * p.x() - c * p.y();
    J(1, 2) = c * p.x() - s * p.y();

    const Eigen::Matrix<double, 3, 3> H = J.transpose() * cell->inv_cov * J;
    const Eigen::Vector3d g = J.transpose() * cell->inv_cov * r;
    A += H;
    b += g;
    ++used;
  }

  if (used < params_.min_points) return false;

  const Eigen::Vector3d delta = A.ldlt().solve(-b);
  if (!delta.allFinite()) return false;

  *increment = matrixFromIncrement(delta(0), delta(1), delta(2));
  if (score != nullptr) *score = total_score;
  return true;
}

bool NDT2DEstimator::solveGaussNewton(const std::vector<Eigen::Vector2d>& current,
                                      const NDTMap& map, double cell_size,
                                      Eigen::Matrix3d* estimate, int max_iterations,
                                      int* iterations, double* score) const {
  if (estimate == nullptr) return false;
  double best_score = -1.0;
  int total_iters = iterations != nullptr ? *iterations : 0;

  for (int iter = 0; iter < max_iterations; ++iter) {
    Eigen::Matrix3d increment = Eigen::Matrix3d::Identity();
    double iter_score = 0.0;
    if (!solveIncrement(current, map.cells, *estimate, map.origin, cell_size, &increment,
                        &iter_score)) {
      break;
    }

    *estimate = increment * (*estimate);
    ++total_iters;
    best_score = iter_score;

    const double dt = std::hypot(increment(0, 2), increment(1, 2));
    const double dyaw = std::atan2(increment(1, 0), increment(0, 0));
    if (dt < params_.convergence_translation &&
        std::abs(dyaw) < params_.convergence_rotation) {
      break;
    }
  }

  if (iterations != nullptr) *iterations = total_iters;
  if (score != nullptr) *score = best_score;
  return total_iters > 0;
}

bool NDT2DEstimator::solveWithPyramid(const std::vector<Eigen::Vector2d>& current,
                                      const std::vector<Eigen::Vector2d>& ref_points,
                                      Eigen::Matrix3d* estimate, int* iterations,
                                      double* score) const {
  if (estimate == nullptr || ref_points.empty()) return false;

  const int levels = std::max(1, params_.pyramid_levels);
  int total_iters = 0;
  double best_score = -1.0;

  for (int level = 0; level < levels; ++level) {
    const double cell_size = params_.cell_size *
                             std::pow(params_.pyramid_scale,
                                      static_cast<double>(levels - 1 - level));
    const NDTMap map = buildNDTMap(ref_points, cell_size);
    if (map.cells.size() < 3) continue;

    const bool finest = level == levels - 1;
    const int level_iters =
        finest ? params_.max_iterations : std::max(5, params_.max_iterations / 2);
    if (!solveGaussNewton(current, map, cell_size, estimate, level_iters, &total_iters,
                          &best_score)) {
      return false;
    }
  }

  if (iterations != nullptr) *iterations = total_iters;
  if (score != nullptr) *score = best_score;
  return total_iters > 0;
}

NDT2DResult NDT2DEstimator::registerScan(const LaserScan& scan) {
  NDT2DResult result;
  const auto points = scanToPoints(scan);
  if (points.size() < static_cast<size_t>(params_.min_points)) {
    return result;
  }

  if (!initialized_) {
    if (params_.use_local_map) {
      addScanToLocalMap(points);
      ref_points_ = local_points_;
      ref_map_ = buildNDTMap(ref_points_, params_.cell_size);
    } else {
      ref_points_ = points;
      ref_map_ = buildNDTMap(ref_points_, params_.cell_size);
    }
    if (ref_map_.cells.size() < 3) return result;
    initialized_ = true;
    result.valid = true;
    result.pose = pose_;
    result.increment = Eigen::Matrix3d::Identity();
    return result;
  }

  Eigen::Matrix3d estimate =
      params_.use_motion_prior ? last_increment_ : Eigen::Matrix3d::Identity();
  double best_score = -1.0;
  int iterations = 0;

  if (params_.pyramid_levels > 1) {
    if (!solveWithPyramid(points, ref_points_, &estimate, &iterations, &best_score)) {
      return result;
    }
    result.iterations = iterations;
  } else {
    if (!solveGaussNewton(points, ref_map_, params_.cell_size, &estimate, params_.max_iterations,
                          &iterations, &best_score)) {
      return result;
    }
    result.iterations = iterations;
  }

  last_increment_ = estimate;
  pose_ = pose_ * estimate;
  if (params_.use_local_map) {
    transformRobotMap(estimate.inverse());
    addScanToLocalMap(points);
    ref_points_ = local_points_;
    ref_map_ = buildNDTMap(ref_points_, params_.cell_size);
  } else {
    ref_points_ = points;
    ref_map_ = buildNDTMap(ref_points_, params_.cell_size);
  }

  result.valid = true;
  result.increment = estimate;
  result.pose = pose_;
  result.score = best_score;
  return result;
}

}  // namespace ndt_2d
}  // namespace localization_zoo
