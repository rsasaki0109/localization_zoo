#include "m_gclo/m_gclo.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>

namespace localization_zoo {
namespace m_gclo {

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
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
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
        c.has_pca = true;
      }
    }
  }
  return correspondences;
}

// ============================================================
// MGcloPipeline
// ============================================================

MGcloPipeline::MGcloPipeline(const MGcloParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> MGcloPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> MGcloPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d MGcloPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  return pose_ * delta_window_.back();
}

Eigen::Matrix4d MGcloPipeline::runRegistration(
    const std::vector<Eigen::Vector3d>& source, const Eigen::Matrix4d& base,
    MGcloResult* result) {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;
  const double kernel = params_.initial_threshold;
  const double ref2 =
      params_.uncertainty_range_ref * params_.uncertainty_range_ref;

  // 各点の距離依存不確かさ重み w_unc = ref²/(ref²+range²) (body 系で一度算出)。
  std::vector<double> w_unc(source.size(), 1.0);
  for (size_t k = 0; k < source.size(); k++) {
    const double r2 = source[k].squaredNorm();
    w_unc[k] = ref2 / (ref2 + r2);
  }

  Eigen::Matrix4d T = base;
  int last_corr = 0, last_ground = 0, last_nonground = 0;

  for (int it = 0; it < params_.max_iterations; it++) {
    const auto src = transformPoints(source, T);
    const auto corr = local_map_.getCorrespondences(
        src, params_.initial_threshold, params_.normal_min_neighbors);
    const double sensor_z = T(2, 3);

    Mat6 A = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    int used = 0, n_ground = 0, n_nonground = 0;

    for (size_t k = 0; k < source.size(); k++) {
      const auto& c = corr[k];
      if (!c.found || !c.has_pca) continue;

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src[k]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      // --- 地面/非地面分類: 対応平面の法線鉛直性 + 高さ ---
      const bool is_ground =
          c.planarity >= params_.ground_planarity &&
          std::abs(c.normal.z()) >= params_.ground_normal_threshold &&
          c.mean.z() < sensor_z + params_.ground_height_offset;

      if (is_ground) {
        ++n_ground;
        if (params_.ground_weight <= 0.0) continue;

        // 地面 point-to-plane (鉛直精度を拘束する複数地面平面)。
        const double e = c.normal.dot(src[k] - c.mean);
        if (std::abs(e) > kernel) continue;
        double w = params_.ground_weight * w_unc[k];
        if (params_.robust_scale > 0.0) {
          const double s = e / params_.robust_scale;
          w /= (1.0 + s * s);
        }
        const Eigen::Matrix<double, 1, 6> Jr = c.normal.transpose() * J;
        A += w * Jr.transpose() * Jr;
        b -= w * Jr.transpose() * e;
        ++used;
      } else {
        // 非地面 point-to-distribution (NDT Mahalanobis)。
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(c.cov);
        Eigen::Vector3d ev = es.eigenvalues();
        const double lmax = std::max(ev(2), 1e-9);
        const double floor_v = params_.distribution_regularization * lmax;
        for (int d = 0; d < 3; d++) ev(d) = std::max(ev(d), floor_v);
        const Eigen::Matrix3d V = es.eigenvectors();
        const Eigen::Matrix3d info =
            V * ev.cwiseInverse().asDiagonal() * V.transpose();

        const Eigen::Vector3d e = src[k] - c.mean;
        if (e.norm() > kernel) continue;
        double w = params_.nonground_weight * w_unc[k];
        if (params_.robust_scale > 0.0) {
          const double m2 = e.transpose() * info * e;
          w /= (1.0 + m2 / (params_.robust_scale * params_.robust_scale));
        }
        A += w * J.transpose() * info * J;
        b -= w * J.transpose() * info * e;
        ++n_nonground;
        ++used;
      }
    }

    last_corr = used;
    last_ground = n_ground;
    last_nonground = n_nonground;
    if (used < 10) break;

    if (params_.prior_precision > 0.0) {
      const Eigen::Vector3d dt = T.block<3, 1>(0, 3) - base.block<3, 1>(0, 3);
      Mat6 Pm = Mat6::Zero();
      Pm.block<3, 3>(3, 3) =
          params_.prior_precision * Eigen::Matrix3d::Identity();
      Vec6 pr = Vec6::Zero();
      pr.tail<3>() = params_.prior_precision * dt;
      A += Pm;
      b -= pr;
    }

    const Vec6 dx = A.ldlt().solve(b);
    if (!dx.allFinite()) break;
    T = expSE3(dx) * T;
    if (result) result->iterations = it + 1;
    if (dx.norm() < params_.convergence_criterion) break;
  }

  if (result) {
    result->num_correspondences = last_corr;
    result->num_ground = last_ground;
    result->num_nonground = last_nonground;
  }
  return T;
}

MGcloResult MGcloPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame,
    const Eigen::Matrix4d* init_guess) {
  MGcloResult result;

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

  const Eigen::Matrix4d base = init_guess ? *init_guess : predict();
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

}  // namespace m_gclo
}  // namespace localization_zoo
