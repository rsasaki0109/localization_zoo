#include "terrain_rbf_lio/terrain_rbf_lio.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace terrain_rbf_lio {

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

Eigen::Vector3d logSO3(const Eigen::Matrix3d& R) {
  const double c = std::clamp((R.trace() - 1.0) * 0.5, -1.0, 1.0);
  const double theta = std::acos(c);
  if (theta < 1e-9) return Eigen::Vector3d::Zero();
  const Eigen::Vector3d w(R(2, 1) - R(1, 2), R(0, 2) - R(2, 0),
                          R(1, 0) - R(0, 1));
  return w * (theta / (2.0 * std::sin(theta)));
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
    int normal_min_neighbors) const {
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
    c.point = best_point;
    c.found = found;
    if (!found) continue;
    if (static_cast<int>(neighbors.size()) >= normal_min_neighbors) {
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
        c.anchor = centroid;
        c.has_normal = true;
      }
    }
  }
  return correspondences;
}

// ============================================================
// TerrainRbfMap
// ============================================================

void TerrainRbfMap::update(const std::vector<Eigen::Vector3d>& world_points) {
  for (const auto& p : world_points) {
    auto key = toCell(p.x(), p.y());
    auto it = cells_.find(key);
    if (it == cells_.end()) {
      Cell c;
      c.cx = (key.x() + 0.5) * cell_size_;
      c.cy = (key.y() + 0.5) * cell_size_;
      c.h = p.z();
      c.n = 1.0;
      cells_.emplace(key, c);
      continue;
    }
    Cell& c = it->second;
    // ground 候補: 現推定より低い or band 内の点のみを地形面として取り込む。
    // 屋根や壁の高い点で地形高さが釣り上がるのを防ぐ。
    if (p.z() < c.h - ground_band_) {
      // より低い ground を発見 → 速やかに追従。
      c.h = p.z();
      c.n = 1.0;
    } else if (p.z() <= c.h + ground_band_) {
      c.n += 1.0;
      c.h += (p.z() - c.h) / c.n;  // RLS (走行平均) 更新
    }
  }
}

double TerrainRbfMap::evaluate(double x, double y, bool* valid, double* dfdx,
                               double* dfdy) const {
  const Eigen::Vector2i key = toCell(x, y);
  const int radius = std::max(1, static_cast<int>(std::ceil(2.0 * sigma_ / cell_size_)));
  const double inv2s2 = 1.0 / (2.0 * sigma_ * sigma_);
  double W = 0.0, S = 0.0;
  double dW_dx = 0.0, dW_dy = 0.0, dS_dx = 0.0, dS_dy = 0.0;
  for (int dx = -radius; dx <= radius; dx++)
    for (int dy = -radius; dy <= radius; dy++) {
      auto it = cells_.find(key + Eigen::Vector2i(dx, dy));
      if (it == cells_.end()) continue;
      const Cell& c = it->second;
      const double ex = x - c.cx;
      const double ey = y - c.cy;
      const double w = std::exp(-(ex * ex + ey * ey) * inv2s2);
      W += w;
      S += w * c.h;
      // ∂w/∂x = w * (-(x-cx)/σ²)
      const double dwdx = w * (-ex / (sigma_ * sigma_));
      const double dwdy = w * (-ey / (sigma_ * sigma_));
      dW_dx += dwdx;
      dW_dy += dwdy;
      dS_dx += dwdx * c.h;
      dS_dy += dwdy * c.h;
    }
  if (W < 1e-9) {
    if (valid) *valid = false;
    if (dfdx) *dfdx = 0.0;
    if (dfdy) *dfdy = 0.0;
    return 0.0;
  }
  const double f = S / W;
  if (valid) *valid = true;
  // ∂f/∂x = (∂S/∂x · W - S · ∂W/∂x) / W² = (∂S/∂x - f·∂W/∂x)/W
  if (dfdx) *dfdx = (dS_dx - f * dW_dx) / W;
  if (dfdy) *dfdy = (dS_dy - f * dW_dy) / W;
  return f;
}

// ============================================================
// TerrainRbfPipeline
// ============================================================

TerrainRbfPipeline::TerrainRbfPipeline(const TerrainRbfParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel),
      terrain_(params.terrain_cell_size, params.terrain_sigma,
               params.terrain_ground_band) {}

std::vector<Eigen::Vector3d> TerrainRbfPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> TerrainRbfPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d TerrainRbfPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  const Eigen::Matrix4d& d = delta_window_.back();
  return pose_ * d;
}

Eigen::Matrix4d TerrainRbfPipeline::runRegistration(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, TerrainRbfResult* result) {
  Eigen::Matrix4d T = initial_guess;
  const double kernel = params_.initial_threshold;

  const bool terrain_enabled =
      params_.terrain_weight > 0.0 && frame_count_ >= params_.terrain_warmup &&
      sensor_height_init_;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const auto src = transformPoints(source, T);
    const auto corr = local_map_.getCorrespondences(
        src, params_.initial_threshold, params_.normal_min_neighbors);

    Eigen::Matrix<double, 6, 6> A = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> b = Eigen::Matrix<double, 6, 1>::Zero();
    int used = 0;
    for (size_t i = 0; i < source.size(); i++) {
      const auto& c = corr[i];
      if (!c.found) continue;
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src[i]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      double e;
      Eigen::Matrix<double, 1, 6> Jr;
      if (c.has_normal && c.planarity >= params_.planarity_threshold) {
        e = c.normal.dot(src[i] - c.anchor);
        Jr = c.normal.transpose() * J;
      } else {
        const Eigen::Vector3d d = src[i] - c.point;
        const double dn = d.norm();
        if (dn < 1e-9) continue;
        Jr = (d / dn).transpose() * J;
        e = dn;
      }
      if (std::abs(e) > kernel) continue;
      const double w = std::exp(-0.5 * (e / kernel) * (e / kernel));
      A += w * Jr.transpose() * Jr;
      b -= w * Jr.transpose() * e;
      ++used;
    }
    if (used < 10) break;

    // --- RBF 地形 soft 拘束 (z-drift 抑制) ---
    // r_M = (t_z - h_sensor) - f(t_x, t_y),  ∂r_M/∂t = [-∂f/∂x, -∂f/∂y, 1]
    // ∂t/∂δ = [-skew(t) | I] (left perturbation T ← dT·T と整合)。
    if (terrain_enabled) {
      const Eigen::Vector3d t = T.block<3, 1>(0, 3);
      bool valid = false;
      double dfdx = 0.0, dfdy = 0.0;
      const double f = terrain_.evaluate(t.x(), t.y(), &valid, &dfdx, &dfdy);
      if (valid) {
        const double rM = (t.z() - sensor_height_) - f;
        Eigen::Matrix<double, 3, 6> Jt;
        Jt.block<3, 3>(0, 0) = -skew(t);
        Jt.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
        const Eigen::RowVector3d drdt(-dfdx, -dfdy, 1.0);
        const Eigen::Matrix<double, 1, 6> JrM = drdt * Jt;
        const double lam = params_.terrain_weight;
        A += lam * JrM.transpose() * JrM;
        b -= lam * JrM.transpose() * rM;
        if (result) {
          result->terrain_active = true;
          result->terrain_residual = rM;
        }
      }
    }

    const Eigen::Matrix<double, 6, 1> delta = A.ldlt().solve(b);
    if (!delta.allFinite()) break;
    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;

    if (result) result->iterations = iter + 1;
    if (delta.norm() < params_.convergence_criterion) break;
  }
  return T;
}

TerrainRbfResult TerrainRbfPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  TerrainRbfResult result;

  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg = voxelDownsample(downsampled, params_.voxel_size);
  if (reg.empty()) reg = downsampled;

  if (frame_count_ == 0) {
    const auto world = transformPoints(downsampled, pose_);
    local_map_.addPoints(world);
    terrain_.update(world);
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  const Eigen::Matrix4d prediction = predict();
  const Eigen::Matrix4d new_pose = runRegistration(reg, prediction, &result);

  const Eigen::Matrix4d delta = pose_.inverse() * new_pose;
  delta_window_.push_back(delta);
  if (static_cast<int>(delta_window_.size()) > 2) delta_window_.pop_front();

  pose_ = new_pose;

  const auto world = transformPoints(downsampled, pose_);
  local_map_.addPoints(world);
  terrain_.update(world);

  // h_sensor (地形面からのセンサ高) を EMA 更新。
  {
    const Eigen::Vector3d t = pose_.block<3, 1>(0, 3);
    bool valid = false;
    const double f = terrain_.evaluate(t.x(), t.y(), &valid);
    if (valid) {
      const double obs = t.z() - f;
      if (!sensor_height_init_) {
        sensor_height_ = obs;
        sensor_height_init_ = true;
      } else {
        sensor_height_ += params_.sensor_height_alpha * (obs - sensor_height_);
      }
    }
  }
  result.sensor_height = sensor_height_;

  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    local_map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.converged = (pose_.array().isFinite().all() &&
                      (new_pose - prediction).cwiseAbs().maxCoeff() < 1e3);
  frame_count_++;
  return result;
}

}  // namespace terrain_rbf_lio
}  // namespace localization_zoo
