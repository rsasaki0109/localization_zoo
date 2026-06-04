#include "pcr_dat/pcr_dat.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace pcr_dat {

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
  return Eigen::Matrix3d::Identity() + std::sin(t) * K +
         (1.0 - std::cos(t)) * K * K;
}

// Exp([w,v]) = [[expSO3(w), v],[0,1]] (小運動近似、本リポジトリの dT 構築と整合)。
Eigen::Matrix4d expSE3(const Eigen::Matrix<double, 6, 1>& xi) {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = expSO3(xi.head<3>());
  T.block<3, 1>(0, 3) = xi.tail<3>();
  return T;
}

std::vector<Eigen::Vector3d> transformPoints(
    const std::vector<Eigen::Vector3d>& pts, const Eigen::Matrix4d& T) {
  Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  Eigen::Vector3d t = T.block<3, 1>(0, 3);
  std::vector<Eigen::Vector3d> out(pts.size());
  for (size_t i = 0; i < pts.size(); i++) out[i] = R * pts[i] + t;
  return out;
}

}  // namespace

// ============================================================
// VoxelHashMap
// ============================================================

void VoxelHashMap::addPoints(const std::vector<Eigen::Vector3d>& points) {
  for (const auto& p : points) {
    auto key = toVoxel(p);
    auto& vb = map_[key];
    if (static_cast<int>(vb.points.size()) < max_points_) vb.points.push_back(p);
  }
}

void VoxelHashMap::pruneFarVoxels(const Eigen::Vector3d& center,
                                  double max_distance) {
  const double max_distance_sq = max_distance * max_distance;
  for (auto it = map_.begin(); it != map_.end();) {
    const Eigen::Vector3d c =
        (it->first.cast<double>() + Eigen::Vector3d::Constant(0.5)) * voxel_size_;
    if ((c - center).squaredNorm() > max_distance_sq) {
      it = map_.erase(it);
    } else {
      ++it;
    }
  }
}

std::vector<VoxelHashMap::Correspondence> VoxelHashMap::getCorrespondences(
    const std::vector<Eigen::Vector3d>& points, double max_dist,
    int min_neighbors) const {
  std::vector<Correspondence> correspondences(points.size());
  const double max_dist_sq = max_dist * max_dist;

  for (size_t i = 0; i < points.size(); i++) {
    const auto& query = points[i];
    auto key = toVoxel(query);
    double best_dist = max_dist_sq;
    Eigen::Vector3d best_point = Eigen::Vector3d::Zero();
    bool found = false;
    std::vector<Eigen::Vector3d> neighbors;
    neighbors.reserve(32);

    for (int dx = -1; dx <= 1; dx++)
      for (int dy = -1; dy <= 1; dy++)
        for (int dz = -1; dz <= 1; dz++) {
          auto it = map_.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == map_.end()) continue;
          for (const auto& mp : it->second.points) {
            const double d = (mp - query).squaredNorm();
            if (d < max_dist_sq) neighbors.push_back(mp);
            if (d < best_dist) {
              best_dist = d;
              best_point = mp;
              found = true;
            }
          }
        }

    Correspondence& c = correspondences[i];
    c.nearest = best_point;
    c.found = found;
    c.neighbor_count = static_cast<int>(neighbors.size());
    if (!found) continue;
    if (static_cast<int>(neighbors.size()) >= min_neighbors) {
      Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
      for (const auto& n : neighbors) centroid += n;
      centroid /= static_cast<double>(neighbors.size());
      Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
      for (const auto& n : neighbors) {
        const Eigen::Vector3d d = n - centroid;
        cov += d * d.transpose();
      }
      cov /= static_cast<double>(neighbors.size());
      Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
      if (solver.info() == Eigen::Success) {
        const Eigen::Vector3d ev = solver.eigenvalues();
        const double lambda2 = std::max(ev(2), 1e-12);
        c.planarity = std::clamp((ev(1) - ev(0)) / lambda2, 0.0, 1.0);
        c.normal = solver.eigenvectors().col(0).normalized();
        c.mean = centroid;
        c.cov = cov;
        c.has_distribution = true;
      }
    }
  }
  return correspondences;
}

// ============================================================
// PcrDatPipeline
// ============================================================

PcrDatPipeline::PcrDatPipeline(const PcrDatParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> PcrDatPipeline::voxelDownsample(
    const std::vector<Eigen::Vector3d>& points, double voxel_size) const {
  std::unordered_map<Eigen::Vector3i, Eigen::Vector3d, VoxelHash> grid;
  for (const auto& p : points) {
    Eigen::Vector3i key(static_cast<int>(std::floor(p.x() / voxel_size)),
                        static_cast<int>(std::floor(p.y() / voxel_size)),
                        static_cast<int>(std::floor(p.z() / voxel_size)));
    grid.emplace(key, p);
  }
  std::vector<Eigen::Vector3d> out;
  out.reserve(grid.size());
  for (const auto& kv : grid) out.push_back(kv.second);
  return out;
}

std::vector<Eigen::Vector3d> PcrDatPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d PcrDatPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  return pose_ * delta_window_.back();
}

Eigen::Matrix4d PcrDatPipeline::runRegistration(
    const std::vector<Eigen::Vector3d>& source, const Eigen::Matrix4d& base,
    PcrDatResult* result) {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;
  const double inv_dist_sigma2 =
      1.0 / (params_.distance_sigma * params_.distance_sigma);
  const double kernel = params_.initial_threshold;

  Eigen::Matrix4d T = base;
  int last_corr = 0, last_dist = 0, last_distrib = 0;

  for (int it = 0; it < params_.max_iterations; it++) {
    const auto src = transformPoints(source, T);
    const auto corr = local_map_.getCorrespondences(
        src, params_.initial_threshold, params_.normal_min_neighbors);

    Mat6 A = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    int used = 0, n_dist = 0, n_distrib = 0;

    for (size_t k = 0; k < source.size(); k++) {
      const auto& c = corr[k];
      if (!c.found) continue;

      // 並進ヤコビアン J = [-skew(p_w), I] (左摂動)。
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src[k]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      // --- PCR-DAT のコア: 局所点密度でファクタを切り替える ---
      const bool rich = c.has_distribution &&
                        c.neighbor_count >= params_.distribution_min_points &&
                        c.planarity >= params_.distribution_planarity;

      if (rich) {
        // 分布(Gauss)ファクタ: Mahalanobis 残差 e = p_w - μ, 情報 = Σ⁻¹。
        // Σ を NDT 流に正則化 (最小固有値を λmax の比で下限化)。
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(c.cov);
        Eigen::Vector3d ev = es.eigenvalues();
        const double lmax = std::max(ev(2), 1e-9);
        const double floor_v = params_.distribution_regularization * lmax;
        for (int d = 0; d < 3; d++) ev(d) = std::max(ev(d), floor_v);
        const Eigen::Matrix3d V = es.eigenvectors();
        const Eigen::Matrix3d info =
            V * ev.cwiseInverse().asDiagonal() * V.transpose();

        const Eigen::Vector3d e = src[k] - c.mean;
        // Cauchy ロバスト重み (Mahalanobis ノルムベース)。
        double w = params_.distribution_weight;
        if (params_.robust_scale > 0.0) {
          const double m2 = e.transpose() * info * e;
          w /= (1.0 + m2 / (params_.robust_scale * params_.robust_scale));
        }
        A += w * J.transpose() * info * J;
        b -= w * J.transpose() * info * e;
        ++n_distrib;
        ++used;
      } else {
        // 距離(distance)ファクタ: point-to-plane (法線が立てば) か point-to-point。
        double e;
        Eigen::Matrix<double, 1, 6> Jr;
        if (c.has_distribution && c.planarity >= params_.distance_planarity) {
          e = c.normal.dot(src[k] - c.mean);
          Jr = c.normal.transpose() * J;
        } else {
          // point-to-point: 最近傍方向の 1 次元残差。
          const Eigen::Vector3d diff = src[k] - c.nearest;
          const double n = diff.norm();
          if (n < 1e-9) continue;
          const Eigen::Vector3d dir = diff / n;
          e = n;
          Jr = dir.transpose() * J;
        }
        if (std::abs(e) > kernel) continue;
        double w = params_.distance_weight * inv_dist_sigma2;
        if (params_.robust_scale > 0.0) {
          const double s = e / params_.robust_scale;
          w /= (1.0 + s * s);
        }
        A += w * Jr.transpose() * Jr;
        b -= w * Jr.transpose() * e;
        ++n_dist;
        ++used;
      }
    }

    last_corr = used;
    last_dist = n_dist;
    last_distrib = n_distrib;
    if (used < 10) break;

    // 予測 (等速、IMU 事前積分ファクタの KITTI 代替) への弱い prior。
    if (params_.prior_precision > 0.0) {
      // base からの逸脱 θ = log(base⁻¹ T) を 0 へ引く (並進近似のみ、左接)。
      const Eigen::Vector3d dt = T.block<3, 1>(0, 3) - base.block<3, 1>(0, 3);
      Mat6 P = Mat6::Zero();
      P.block<3, 3>(3, 3) =
          params_.prior_precision * Eigen::Matrix3d::Identity();
      Vec6 pr = Vec6::Zero();
      pr.tail<3>() = params_.prior_precision * dt;
      A += P;
      b -= pr;
    }

    const Vec6 d = A.ldlt().solve(b);
    if (!d.allFinite()) break;
    T = expSE3(d) * T;
    if (result) result->iterations = it + 1;
    if (d.norm() < params_.convergence_criterion) break;
  }

  if (result) {
    result->num_correspondences = last_corr;
    result->num_distance = last_dist;
    result->num_distribution = last_distrib;
  }
  return T;
}

PcrDatResult PcrDatPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  PcrDatResult result;

  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg = voxelDownsample(downsampled, params_.voxel_size);
  if (reg.empty()) reg = downsampled;

  if (frame_count_ == 0) {
    local_map_.addPoints(transformPoints(downsampled, pose_));
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  const Eigen::Matrix4d base = predict();
  const Eigen::Matrix4d new_pose = runRegistration(reg, base, &result);

  const Eigen::Matrix4d delta = pose_.inverse() * new_pose;
  delta_window_.push_back(delta);
  if (static_cast<int>(delta_window_.size()) > 2) delta_window_.pop_front();
  pose_ = new_pose;

  local_map_.addPoints(transformPoints(downsampled, pose_));
  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    local_map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.converged = (pose_.array().isFinite().all() &&
                      (new_pose - base).cwiseAbs().maxCoeff() < 1e3);
  frame_count_++;
  return result;
}

}  // namespace pcr_dat
}  // namespace localization_zoo
