#include "litamin2/litamin2_odometry.h"

#include <Eigen/Dense>

#include <cmath>
#include <limits>

namespace localization_zoo {
namespace litamin2 {

namespace {

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0, -v.z(), v.y(), v.z(), 0, -v.x(), -v.y(), v.x(), 0;
  return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& w) {
  const double theta = w.norm();
  if (theta < 1e-12) return Eigen::Matrix3d::Identity() + skew(w);
  const Eigen::Matrix3d K = skew(w / theta);
  return Eigen::Matrix3d::Identity() + std::sin(theta) * K + (1.0 - std::cos(theta)) * K * K;
}

// d tr([w]x M) / dw.
Eigen::Vector3d traceSkewGradient(const Eigen::Matrix3d& m) {
  return Eigen::Vector3d(m(1, 2) - m(2, 1), m(2, 0) - m(0, 2), m(0, 1) - m(1, 0));
}

// Hessian of f(w) = tr(e^[w]x M e^-[w]x C) at w = 0.
Eigen::Matrix3d rotationTraceHessian(const Eigen::Matrix3d& M, const Eigen::Matrix3d& C) {
  Eigen::Matrix3d G[3];
  for (int a = 0; a < 3; ++a) G[a] = skew(Eigen::Vector3d::Unit(a));
  Eigen::Matrix3d H;
  for (int a = 0; a < 3; ++a)
    for (int c = a; c < 3; ++c) {
      const Eigen::Matrix3d S = 0.5 * (G[a] * G[c] + G[c] * G[a]);
      H(a, c) = H(c, a) = ((S * M - G[a] * M * G[c] - G[c] * M * G[a] + M * S) * C).trace();
    }
  return H;
}

// Newton steps need a positive semi-definite contribution.
Eigen::Matrix3d projectPsd(const Eigen::Matrix3d& H) {
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(H);
  const Eigen::Vector3d ev = solver.eigenvalues().cwiseMax(0.0);
  return solver.eigenvectors() * ev.asDiagonal() * solver.eigenvectors().transpose();
}

Eigen::Matrix3d clampEigenvalues(const Eigen::Matrix3d& cov, double floor) {
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
  const Eigen::Vector3d ev = solver.eigenvalues().cwiseMax(floor);
  return solver.eigenvectors() * ev.asDiagonal() * solver.eigenvectors().transpose();
}

}  // namespace

LiTAMIN2Odometry::LiTAMIN2Odometry(const LiTAMIN2OdometryParams& params) : params_(params) {}

std::vector<GaussianVoxel> LiTAMIN2Odometry::voxelize(
    const std::vector<Eigen::Vector3d>& points) const {
  struct Acc {
    int n = 0;
    Eigen::Vector3d sum = Eigen::Vector3d::Zero();
    Eigen::Matrix3d sum_sq = Eigen::Matrix3d::Zero();
  };
  std::unordered_map<Eigen::Vector3i, Acc, Vector3iHash> cells;
  for (const auto& p : points) {
    const Eigen::Vector3i key = (p / params_.voxel_size).array().floor().cast<int>();
    Acc& a = cells[key];
    ++a.n;
    a.sum += p;
    a.sum_sq += p * p.transpose();
  }
  std::vector<GaussianVoxel> out;
  out.reserve(cells.size());
  for (const auto& [key, a] : cells) {
    (void)key;
    if (a.n < params_.min_points_per_voxel) continue;
    GaussianVoxel v;
    v.num_points = a.n;
    v.mean = a.sum / a.n;
    v.cov = clampEigenvalues(a.sum_sq / a.n - v.mean * v.mean.transpose(), params_.min_cov_eigenvalue);
    out.push_back(v);
  }
  return out;
}

Eigen::Vector3i LiTAMIN2Odometry::mapKey(const Eigen::Vector3d& p) const {
  return (p / params_.map_voxel_size).array().floor().cast<int>();
}

const GaussianVoxel* LiTAMIN2Odometry::nearest(const Eigen::Vector3d& q) const {
  const Eigen::Vector3i c = mapKey(q);
  const GaussianVoxel* best = nullptr;
  double best_d2 = std::numeric_limits<double>::infinity();
  for (int dx = -1; dx <= 1; ++dx)
    for (int dy = -1; dy <= 1; ++dy)
      for (int dz = -1; dz <= 1; ++dz) {
        auto it = map_.find(c + Eigen::Vector3i(dx, dy, dz));
        if (it == map_.end() || !it->second.valid) continue;
        const double d2 = (it->second.dist.mean - q).squaredNorm();
        if (d2 < best_d2) {
          best_d2 = d2;
          best = &it->second.dist;
        }
      }
  return best;
}

Eigen::Matrix4d LiTAMIN2Odometry::align(const std::vector<GaussianVoxel>& source,
                                        const Eigen::Matrix4d& guess, int* iterations,
                                        int* correspondences) const {
  Eigen::Matrix3d R = guess.block<3, 3>(0, 0);
  Eigen::Vector3d t = guess.block<3, 1>(0, 3);
  const double s_icp2 = params_.sigma_icp * params_.sigma_icp;
  const double s_cov2 = params_.sigma_cov * params_.sigma_cov;
  const Eigen::Matrix3d I3 = Eigen::Matrix3d::Identity();

  // Inverse source covariances do not change during the registration.
  std::vector<Eigen::Matrix3d> source_inv(source.size());
  for (size_t i = 0; i < source.size(); ++i) source_inv[i] = source[i].cov.inverse();

  int iter = 0;
  int pairs = 0;
  for (; iter < params_.max_iterations; ++iter) {
    Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> b = Eigen::Matrix<double, 6, 1>::Zero();
    pairs = 0;
    for (size_t i = 0; i < source.size(); ++i) {
      const Eigen::Vector3d x = R * source[i].mean + t;
      const GaussianVoxel* target = nearest(x);
      if (!target) continue;
      ++pairs;
      const Eigen::Matrix3d RCpRT = R * source[i].cov * R.transpose();
      const Eigen::Matrix3d C_inv = (target->cov + RCpRT + params_.lambda * I3).inverse();
      const Eigen::Matrix3d C_qp = C_inv / C_inv.norm();
      const Eigen::Vector3d e = target->mean - x;
      const double E_icp = e.dot(C_qp * e);
      const double w_icp = 1.0 - E_icp / (E_icp + s_icp2);

      // Left perturbation x -> (I + [w]x) x + v: de/dw = [x]x, de/dv = -I.
      Eigen::Matrix<double, 3, 6> J;
      J.leftCols<3>() = skew(x);
      J.rightCols<3>() = -I3;
      H += w_icp * J.transpose() * C_qp * J;
      b += w_icp * J.transpose() * C_qp * e;

      if (params_.use_cov_cost) {
        // Second term of Table I: w_Cov * r with r = tr(X) + tr(X^-1) - 6 >= 0,
        // X = R Cp^-1 R^T Cq. It is not a squared error, so Newton steps use
        // its Hessian. w_Cov comes from E_Cov = r^2 (eqs. 4, 6). H and b hold
        // half the gradient / Hessian (as for the ICP term), hence the 0.5.
        const Eigen::Matrix3d Cq_inv = target->cov.inverse();
        const Eigen::Matrix3d A = R * source_inv[i] * R.transpose();
        const double r = (A * target->cov).trace() + (Cq_inv * RCpRT).trace() - 6.0;
        const double w_cov = 1.0 - r * r / (r * r + s_cov2);
        const Eigen::Vector3d j = traceSkewGradient(A * target->cov - target->cov * A +
                                                    RCpRT * Cq_inv - Cq_inv * RCpRT);
        const Eigen::Matrix3d Hr = rotationTraceHessian(A, target->cov) +
                                   rotationTraceHessian(RCpRT, Cq_inv);
        H.topLeftCorner<3, 3>() += 0.5 * w_cov * projectPsd(Hr);
        b.head<3>() += 0.5 * w_cov * j;
      }
    }
    if (pairs < 6) break;
    const Eigen::Matrix<double, 6, 1> delta = H.ldlt().solve(-b);
    if (!delta.allFinite()) break;
    const Eigen::Matrix3d dR = expSO3(delta.head<3>());
    R = dR * R;
    t = dR * t + delta.tail<3>();
    if (delta.head<3>().norm() < params_.rotation_epsilon &&
        delta.tail<3>().norm() < params_.translation_epsilon) {
      ++iter;
      break;
    }
  }
  if (iterations) *iterations = iter;
  if (correspondences) *correspondences = pairs;
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = R;
  T.block<3, 1>(0, 3) = t;
  return T;
}

void LiTAMIN2Odometry::insert(const std::vector<Eigen::Vector3d>& points,
                              const std::vector<GaussianVoxel>& source,
                              const Eigen::Matrix4d& pose) {
  const Eigen::Matrix3d R = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d t = pose.block<3, 1>(0, 3);
  using Mode = LiTAMIN2OdometryParams::MapMode;
  if (params_.map_mode == Mode::kAggregate) {
    for (const auto& p : points) {
      const Eigen::Vector3d w = R * p + t;
      MapVoxel& v = map_[mapKey(w)];
      ++v.n;
      v.sum += w;
      v.sum_sq += w * w.transpose();
      v.valid = false;
    }
    for (auto& [key, v] : map_) {
      (void)key;
      if (v.valid || v.n < params_.min_points_per_voxel) continue;
      v.dist.num_points = v.n;
      v.dist.mean = v.sum / v.n;
      v.dist.cov = clampEigenvalues(v.sum_sq / v.n - v.dist.mean * v.dist.mean.transpose(),
                                    params_.min_cov_eigenvalue);
      v.valid = true;
    }
  } else {
    for (const auto& s : source) {
      GaussianVoxel g = s;
      g.mean = R * s.mean + t;
      g.cov = R * s.cov * R.transpose();
      MapVoxel& v = map_[mapKey(g.mean)];
      if (v.valid && params_.map_mode == Mode::kFirst) continue;
      v.dist = g;
      v.valid = true;
    }
  }

  // Keep the map local.
  const double r2 = params_.map_radius * params_.map_radius;
  for (auto it = map_.begin(); it != map_.end();) {
    const Eigen::Vector3d c = (it->first.cast<double>() + Eigen::Vector3d::Constant(0.5)) *
                              params_.map_voxel_size;
    if ((c - t).squaredNorm() > r2)
      it = map_.erase(it);
    else
      ++it;
  }
}

LiTAMIN2OdometryResult LiTAMIN2Odometry::process(const std::vector<Eigen::Vector3d>& points) {
  LiTAMIN2OdometryResult result;
  const std::vector<GaussianVoxel> source = voxelize(points);
  result.source_distributions = static_cast<int>(source.size());
  if (frame_ > 0 && !source.empty()) {
    const Eigen::Matrix4d guess = pose_ * velocity_;
    const Eigen::Matrix4d T = align(source, guess, &result.iterations, &result.correspondences);
    velocity_ = pose_.inverse() * T;
    pose_ = T;
  }
  insert(points, source, pose_);
  ++frame_;
  result.pose = pose_;
  result.map_distributions = map_.size();
  return result;
}

}  // namespace litamin2
}  // namespace localization_zoo
