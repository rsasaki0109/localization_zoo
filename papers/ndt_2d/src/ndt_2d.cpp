#include "ndt_2d/ndt_2d.h"

#include <Eigen/Dense>
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
  ref_map_.clear();
  map_origin_ = Eigen::Vector2d::Zero();
  pose_ = Eigen::Matrix3d::Identity();
  last_increment_ = Eigen::Matrix3d::Identity();
}

void NDT2DEstimator::setInitialPose(const Eigen::Matrix3d& pose) { pose_ = pose; }

int64_t NDT2DEstimator::cellKey(int ix, int iy) const {
  return (static_cast<int64_t>(ix) << 32) ^
         static_cast<uint32_t>(static_cast<uint32_t>(iy));
}

void NDT2DEstimator::cellIndex(const Eigen::Vector2d& p, int* ix, int* iy) const {
  const Eigen::Vector2d q = p - map_origin_;
  *ix = static_cast<int>(std::floor(q.x() / params_.cell_size));
  *iy = static_cast<int>(std::floor(q.y() / params_.cell_size));
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

NDT2DEstimator::CellMap NDT2DEstimator::buildNDTMap(
    const std::vector<Eigen::Vector2d>& points) {
  if (points.empty()) return {};

  Eigen::Vector2d min_p = points.front();
  Eigen::Vector2d max_p = points.front();
  for (const auto& p : points) {
    min_p = min_p.cwiseMin(p);
    max_p = max_p.cwiseMax(p);
  }
  map_origin_ = min_p - Eigen::Vector2d::Constant(params_.cell_size);

  std::unordered_map<int64_t, CellAccum> accum;
  accum.reserve(points.size());
  for (const auto& p : points) {
    int ix = 0;
    int iy = 0;
    cellIndex(p, &ix, &iy);
    auto& cell = accum[cellKey(ix, iy)];
    cell.sum += p;
    cell.sum_outer += p * p.transpose();
    ++cell.count;
  }

  CellMap map;
  map.reserve(accum.size());
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
    map.emplace(key, cell);
  }
  return map;
}

const NDT2DEstimator::Cell* NDT2DEstimator::lookupCell(const CellMap& map,
                                                       const Eigen::Vector2d& p) const {
  int ix = 0;
  int iy = 0;
  cellIndex(p, &ix, &iy);
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
                                    Eigen::Matrix3d* increment, double* score) const {
  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  double total_score = 0.0;
  int used = 0;

  const double c = transform(0, 0);
  const double s = transform(1, 0);

  for (const auto& p : current) {
    const Eigen::Vector2d pw = transformPoint(transform, p);
    const Cell* cell = lookupCell(map, pw);
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

NDT2DResult NDT2DEstimator::registerScan(const LaserScan& scan) {
  NDT2DResult result;
  const auto points = scanToPoints(scan);
  if (points.size() < static_cast<size_t>(params_.min_points)) {
    return result;
  }

  if (!initialized_) {
    ref_map_ = buildNDTMap(points);
    if (ref_map_.size() < 3) return result;
    initialized_ = true;
    result.valid = true;
    result.pose = pose_;
    result.increment = Eigen::Matrix3d::Identity();
    return result;
  }

  Eigen::Matrix3d estimate = params_.use_motion_prior ? last_increment_ : Eigen::Matrix3d::Identity();
  double best_score = -1.0;

  for (int iter = 0; iter < params_.max_iterations; ++iter) {
    Eigen::Matrix3d increment = Eigen::Matrix3d::Identity();
    double score = 0.0;
    if (!solveIncrement(points, ref_map_, estimate, &increment, &score)) {
      break;
    }

    estimate = increment * estimate;
    result.iterations = iter + 1;
    best_score = score;

    const double dt = std::hypot(increment(0, 2), increment(1, 2));
    const double dyaw =
        std::atan2(increment(1, 0), increment(0, 0));
    if (dt < params_.convergence_translation &&
        std::abs(dyaw) < params_.convergence_rotation) {
      break;
    }
  }

  last_increment_ = estimate;
  pose_ = pose_ * estimate;
  ref_map_ = buildNDTMap(points);

  result.valid = true;
  result.increment = estimate;
  result.pose = pose_;
  result.score = best_score;
  return result;
}

}  // namespace ndt_2d
}  // namespace localization_zoo
