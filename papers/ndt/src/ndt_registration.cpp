#include "ndt/ndt_registration.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <cmath>

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
  auto it = cells_.find(toVoxel(point));
  if (it != cells_.end() && it->second.valid) return &it->second;
  return nullptr;
}

// ============================================================
// NDTRegistration
// ============================================================

void NDTRegistration::setTarget(const std::vector<Eigen::Vector3d>& target) {
  target_map_ = std::make_unique<NDTMap>(params_.resolution, params_.min_points_per_cell);
  target_map_->setInputCloud(target);

  // ガウシアンパラメータ (外れ値比率から)
  gauss_d1_ = -std::log(params_.outlier_ratio);
  gauss_d2_ = -2.0 * std::log(params_.outlier_ratio *
                                 std::pow(2 * M_PI * params_.resolution, 1.5));
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

  for (auto& p : source) {
    Eigen::Vector3d tp = R * p + t;
    const NDTCell* cell = target_map_->lookup(tp);
    if (!cell) continue;

    Eigen::Vector3d diff = tp - cell->mean;
    double exp_val = diff.transpose() * cell->cov_inv * diff;

    if (exp_val > 10.0) continue;  // 外れ値スキップ

    double c = gauss_d1_ * std::exp(-gauss_d2_ * exp_val * 0.5);
    score += c;

    // ヤコビアン: ∂(Rp+t)/∂[ω,v] = [-skew(Rp), I]
    Eigen::Matrix<double, 3, 6> J;
    J.block<3, 3>(0, 0) = -skew(tp);
    J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

    Eigen::Vector3d q = cell->cov_inv * diff;

    // gradient += c * d2 * J^T Σ^{-1} diff
    gradient += c * gauss_d2_ * J.transpose() * q;

    // hessian += c * d2 * J^T Σ^{-1} J (ガウスニュートン近似)
    hessian += c * gauss_d2_ * J.transpose() * cell->cov_inv * J;
  }

  return score;
}

NDTResult NDTRegistration::align(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& initial_guess) {
  NDTResult result;
  if (!target_map_ || target_map_->size() == 0) return result;

  Eigen::Matrix3d R = initial_guess.block<3, 3>(0, 0);
  Eigen::Vector3d t = initial_guess.block<3, 1>(0, 3);

  for (int iter = 0; iter < params_.max_iterations; iter++) {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) = R;
    T.block<3, 1>(0, 3) = t;

    Eigen::Matrix<double, 6, 1> grad;
    Eigen::Matrix<double, 6, 6> H;
    double score = computeDerivatives(source, T, grad, H);

    // ニュートン更新: δ = -H^{-1} g
    Eigen::Matrix<double, 6, 1> delta = -H.ldlt().solve(grad);

    // ステップサイズ制限
    double step = std::min(1.0, params_.step_size / delta.norm());
    delta *= step;

    R = expSO3(delta.head<3>()) * R;
    t += delta.tail<3>();

    result.score = score;
    result.iterations = iter + 1;

    if (delta.norm() < params_.convergence_threshold) {
      result.converged = true;
      break;
    }
  }

  result.transformation.block<3, 3>(0, 0) = R;
  result.transformation.block<3, 1>(0, 3) = t;
  return result;
}

}  // namespace ndt
}  // namespace localization_zoo
