#include "ndt/ndt_registration.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <array>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace ndt {

namespace {

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0, -v.z(), v.y(), v.z(), 0, -v.x(), -v.y(), v.x(), 0;
  return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& w) {
  double t = w.norm();
  if (t < 1e-10) return Eigen::Matrix3d::Identity() + skew(w);
  Eigen::Matrix3d K = skew(w / t);
  return Eigen::Matrix3d::Identity() + std::sin(t) * K + (1.0 - std::cos(t)) * K * K;
}

std::pair<Eigen::Vector3d, double> computeCentroidAndYaw(
    const std::vector<Eigen::Vector3d>& points) {
  if (points.empty()) {
    return {Eigen::Vector3d::Zero(), 0.0};
  }

  Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
  for (const auto& point : points) {
    centroid += point;
  }
  centroid /= static_cast<double>(points.size());

  Eigen::Matrix2d covariance = Eigen::Matrix2d::Zero();
  for (const auto& point : points) {
    const Eigen::Vector2d diff = point.head<2>() - centroid.head<2>();
    covariance += diff * diff.transpose();
  }
  covariance /= static_cast<double>(points.size());

  Eigen::SelfAdjointEigenSolver<Eigen::Matrix2d> solver(covariance);
  if (solver.info() != Eigen::Success) {
    return {centroid, 0.0};
  }

  const Eigen::Vector2d principal_axis = solver.eigenvectors().col(1);
  return {centroid, std::atan2(principal_axis.y(), principal_axis.x())};
}

}  // namespace

// ============================================================
// NDTMap
// ============================================================

NDTMap::NDTMap(double resolution, int min_points)
    : resolution_(resolution), min_points_(min_points) {}

Eigen::Vector3i NDTMap::toVoxel(const Eigen::Vector3d& p) const {
  return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / resolution_)),
                          static_cast<int>(std::floor(p.y() / resolution_)),
                          static_cast<int>(std::floor(p.z() / resolution_)));
}

void NDTMap::setInputCloud(const std::vector<Eigen::Vector3d>& points) {
  cells_.clear();

  // 各ボクセルに点を分配
  std::unordered_map<Eigen::Vector3i, std::vector<Eigen::Vector3d>, VoxelHash> buckets;
  for (auto& p : points) {
    buckets[toVoxel(p)].push_back(p);
  }

  for (auto& [key, pts] : buckets) {
    if (static_cast<int>(pts.size()) < min_points_) continue;

    NDTCell cell;
    cell.num_points = pts.size();

    // 平均
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    for (auto& p : pts) mean += p;
    mean /= pts.size();
    cell.mean = mean;

    // 共分散
    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    for (auto& p : pts) {
      Eigen::Vector3d d = p - mean;
      cov += d * d.transpose();
    }
    cov /= pts.size();

    // 正則化
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
    Eigen::Vector3d evals = solver.eigenvalues();
    double min_eval = evals.maxCoeff() * 0.01;
    for (int i = 0; i < 3; i++) {
      if (evals(i) < min_eval) evals(i) = min_eval;
    }
    cov = solver.eigenvectors() * evals.asDiagonal() *
          solver.eigenvectors().transpose();

    cell.cov = cov;
    cell.cov_inv = cov.inverse();
    cell.valid = true;
    cells_[key] = cell;
  }
}

const NDTCell* NDTMap::lookup(const Eigen::Vector3d& point) const {
  const Eigen::Vector3i center = toVoxel(point);
  const NDTCell* best_cell = nullptr;
  double best_mahalanobis = std::numeric_limits<double>::infinity();

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        const Eigen::Vector3i key = center + Eigen::Vector3i(dx, dy, dz);
        const auto it = cells_.find(key);
        if (it == cells_.end() || !it->second.valid) {
          continue;
        }

        const Eigen::Vector3d diff = point - it->second.mean;
        const double mahalanobis = diff.transpose() * it->second.cov_inv * diff;
        if (mahalanobis < best_mahalanobis) {
          best_mahalanobis = mahalanobis;
          best_cell = &it->second;
        }
      }
    }
  }

  return best_cell;
}

// ============================================================
// NDTRegistration
// ============================================================

void NDTRegistration::setTarget(const std::vector<Eigen::Vector3d>& target) {
  target_map_ = std::make_unique<NDTMap>(params_.resolution, params_.min_points_per_cell);
  target_map_->setInputCloud(target);
  const auto [centroid, yaw] = computeCentroidAndYaw(target);
  target_centroid_ = centroid;
  target_xy_yaw_ = yaw;
}

double NDTRegistration::computeDerivatives(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& T,
    Eigen::Matrix<double, 6, 1>& gradient,
    Eigen::Matrix<double, 6, 6>& hessian) const {
  gradient.setZero();
  hessian.setZero();

  Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  Eigen::Vector3d t = T.block<3, 1>(0, 3);

  double score = 0.0;
  int used = 0;

  for (auto& p : source) {
    const Eigen::Vector3d rotated = R * p;
    const Eigen::Vector3d tp = rotated + t;
    const NDTCell* cell = target_map_->lookup(tp);
    if (!cell) continue;

    const Eigen::Vector3d diff = tp - cell->mean;
    const double mahalanobis = diff.transpose() * cell->cov_inv * diff;

    if (mahalanobis > 10.0) continue;  // 外れ値スキップ

    const Eigen::Vector3d q = cell->cov_inv * diff;
    score += 0.5 * diff.dot(q);
    ++used;

    // Left perturbation on SE(3): x' = exp(ξ^) x
    Eigen::Matrix<double, 3, 6> J;
    J.block<3, 3>(0, 0) = -skew(tp);
    J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

    gradient += J.transpose() * q;
    hessian += J.transpose() * cell->cov_inv * J;
  }

  if (used == 0) {
    return std::numeric_limits<double>::infinity();
  }

  gradient /= static_cast<double>(used);
  hessian /= static_cast<double>(used);
  return score / static_cast<double>(used);
}

double NDTRegistration::computeScore(const std::vector<Eigen::Vector3d>& source,
                                     const Eigen::Matrix4d& T) const {
  Eigen::Matrix<double, 6, 1> gradient;
  Eigen::Matrix<double, 6, 6> hessian;
  return computeDerivatives(source, T, gradient, hessian);
}

double NDTRegistration::lineSearchStep(
    const std::vector<Eigen::Vector3d>& source, const Eigen::Matrix4d& T,
    const Eigen::Matrix<double, 6, 1>& delta, double current_score) const {
  const double delta_norm = delta.norm();
  if (delta_norm < 1e-12) {
    return 0.0;
  }

  double step = std::min(1.0, params_.step_size / delta_norm);
  for (int iter = 0; iter < 10; ++iter) {
    const Eigen::Matrix3d dR = expSO3(step * delta.head<3>());
    Eigen::Matrix4d candidate = T;
    candidate.block<3, 3>(0, 0) = dR * T.block<3, 3>(0, 0);
    candidate.block<3, 1>(0, 3) = dR * T.block<3, 1>(0, 3) + step * delta.tail<3>();
    const double candidate_score = computeScore(source, candidate);
    if (candidate_score < current_score) {
      return step;
    }
    step *= 0.5;
  }
  return 0.0;
}

Eigen::Matrix4d NDTRegistration::makeInitialGuess(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess) const {
  if ((initial_guess - Eigen::Matrix4d::Identity()).cwiseAbs().maxCoeff() > 1e-12 ||
      source.empty()) {
    return initial_guess;
  }

  const auto [source_centroid, source_yaw] = computeCentroidAndYaw(source);
  const double base_yaw = target_xy_yaw_ - source_yaw;
  const std::array<double, 3> yaw_candidates = {
      base_yaw, base_yaw + M_PI, base_yaw - M_PI};

  Eigen::Matrix4d best_guess = initial_guess;
  double best_score = std::numeric_limits<double>::infinity();

  for (const double yaw : yaw_candidates) {
    Eigen::Matrix4d candidate = Eigen::Matrix4d::Identity();
    candidate.block<3, 3>(0, 0) =
        Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    candidate.block<3, 1>(0, 3) =
        target_centroid_ - candidate.block<3, 3>(0, 0) * source_centroid;

    const double candidate_score = computeScore(source, candidate);
    if (candidate_score < best_score) {
      best_score = candidate_score;
      best_guess = candidate;
    }
  }

  return best_guess;
}

NDTResult NDTRegistration::align(const std::vector<Eigen::Vector3d>& source,
                                 const Eigen::Matrix4d& initial_guess) {
  NDTResult result;
  if (!target_map_ || target_map_->size() == 0) return result;

  const Eigen::Matrix4d guess = makeInitialGuess(source, initial_guess);
  Eigen::Matrix3d R = guess.block<3, 3>(0, 0);
  Eigen::Vector3d t = guess.block<3, 1>(0, 3);

  for (int iter = 0; iter < params_.max_iterations; iter++) {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) = R;
    T.block<3, 1>(0, 3) = t;

    Eigen::Matrix<double, 6, 1> grad;
    Eigen::Matrix<double, 6, 6> H;
    const double score = computeDerivatives(source, T, grad, H);
    if (!std::isfinite(score)) {
      break;
    }

    H += 1e-6 * Eigen::Matrix<double, 6, 6>::Identity();
    const Eigen::Matrix<double, 6, 1> delta = -H.ldlt().solve(grad);
    if (!delta.allFinite()) {
      break;
    }

    const double step = lineSearchStep(source, T, delta, score);
    if (step <= 0.0) {
      break;
    }

    const Eigen::Matrix<double, 6, 1> applied_delta = step * delta;
    const Eigen::Matrix3d dR = expSO3(applied_delta.head<3>());
    R = dR * R;
    t = dR * t + applied_delta.tail<3>();

    Eigen::Matrix4d updated = Eigen::Matrix4d::Identity();
    updated.block<3, 3>(0, 0) = R;
    updated.block<3, 1>(0, 3) = t;
    result.score = computeScore(source, updated);
    result.iterations = iter + 1;

    if (applied_delta.norm() < params_.convergence_threshold) {
      result.converged = true;
      break;
    }
  }

  result.transformation = Eigen::Matrix4d::Identity();
  result.transformation.block<3, 3>(0, 0) = R;
  result.transformation.block<3, 1>(0, 3) = t;
  return result;
}

}  // namespace ndt
}  // namespace localization_zoo
