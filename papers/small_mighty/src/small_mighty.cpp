#include "small_mighty/small_mighty.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>

namespace localization_zoo {
namespace small_mighty {

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

std::vector<Eigen::Vector3d> transformPointsHelper(
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
        const Eigen::Vector3d ev = solver.eigenvalues();  // 昇順 λ0≤λ1≤λ2
        const double lambda2 = std::max(ev(2), 1e-12);
        c.planarity = std::clamp((ev(1) - ev(0)) / lambda2, 0.0, 1.0);
        c.linearity = std::clamp((ev(2) - ev(1)) / lambda2, 0.0, 1.0);
        c.normal = solver.eigenvectors().col(0).normalized();
        c.line_dir = solver.eigenvectors().col(2).normalized();
        c.mean = centroid;
        c.has_pca = true;
      }
    }
  }
  return correspondences;
}

// ============================================================
// SmallMightyPipeline
// ============================================================

SmallMightyPipeline::SmallMightyPipeline(const SmallMightyParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> SmallMightyPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> SmallMightyPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

std::vector<SourceFeature> SmallMightyPipeline::selectFeatures(
    const std::vector<Eigen::Vector3d>& source) const {
  // ソース走査の局所 PCA (kd-tree 不使用、ボクセル局所近傍)。
  VoxelHashMap src_map(params_.voxel_size, params_.max_points_per_voxel);
  src_map.addPoints(source);
  const auto pca = src_map.getCorrespondences(source, params_.voxel_size * 1.5,
                                              params_.normal_min_neighbors);

  // --- stability-aware selection: 平面性/線形性の分布 (μ, σ) を求める ---
  double sum_p = 0.0, sum_p2 = 0.0, sum_l = 0.0, sum_l2 = 0.0;
  long np = 0, nl = 0;
  for (size_t i = 0; i < source.size(); i++) {
    if (!pca[i].has_pca) continue;
    const double pl = pca[i].planarity, li = pca[i].linearity;
    if (pl >= params_.planar_min) { sum_p += pl; sum_p2 += pl * pl; ++np; }
    if (li >= params_.edge_min) { sum_l += li; sum_l2 += li * li; ++nl; }
  }
  const double mu_p = np > 0 ? sum_p / np : 0.0;
  const double mu_l = nl > 0 ? sum_l / nl : 0.0;
  const double sd_p =
      np > 0 ? std::sqrt(std::max(1e-9, sum_p2 / np - mu_p * mu_p)) : 1.0;
  const double sd_l =
      nl > 0 ? std::sqrt(std::max(1e-9, sum_l2 / nl - mu_l * mu_l)) : 1.0;

  // --- contribution weighting: 分布平均より上の安定特徴ほど強化 ---
  std::vector<SourceFeature> feats;
  feats.reserve(source.size());
  for (size_t i = 0; i < source.size(); i++) {
    if (!pca[i].has_pca) continue;
    const double pl = pca[i].planarity, li = pca[i].linearity;
    const bool planar_ok = pl >= params_.planar_min;
    const bool edge_ok = li >= params_.edge_min;
    if (!planar_ok && !edge_ok) continue;

    SourceFeature f;
    f.point = source[i];
    // 面/線は支配的な幾何で分類 (両方満たす場合は強い方)。
    if (planar_ok && (!edge_ok || pl >= li)) {
      f.is_edge = false;
      const double z = (pl - mu_p) / sd_p;
      f.weight = 1.0 + params_.contribution_gain *
                           std::clamp(z, 0.0, params_.contribution_cap);
    } else {
      f.is_edge = true;
      const double z = (li - mu_l) / sd_l;
      f.weight = 1.0 + params_.contribution_gain *
                           std::clamp(z, 0.0, params_.contribution_cap);
    }
    feats.push_back(f);
  }
  return feats;
}

Eigen::Matrix4d SmallMightyPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  return pose_ * delta_window_.back();
}

Eigen::Matrix4d SmallMightyPipeline::runRegistration(
    const std::vector<SourceFeature>& features, const Eigen::Matrix4d& base,
    SmallMightyResult* result) {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;
  const double kernel = params_.initial_threshold;

  std::vector<Eigen::Vector3d> pts(features.size());
  for (size_t i = 0; i < features.size(); i++) pts[i] = features[i].point;

  Eigen::Matrix4d T = base;
  int last_corr = 0, last_planar = 0, last_edge = 0;

  for (int it = 0; it < params_.max_iterations; it++) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);
    std::vector<Eigen::Vector3d> src(pts.size());
    for (size_t i = 0; i < pts.size(); i++) src[i] = R * pts[i] + t;
    const auto corr = local_map_.getCorrespondences(
        src, params_.initial_threshold, params_.normal_min_neighbors);

    Mat6 A = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    int used = 0, n_planar = 0, n_edge = 0;

    for (size_t k = 0; k < features.size(); k++) {
      const auto& c = corr[k];
      if (!c.found || !c.has_pca) continue;

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src[k]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      if (!features[k].is_edge) {
        // 面特徴: point-to-plane (スカラー残差)。map 平面性も重みに掛ける。
        const double e = c.normal.dot(src[k] - c.mean);
        if (std::abs(e) > kernel) continue;
        double w = params_.plane_weight * features[k].weight * c.planarity;
        if (params_.robust_scale > 0.0) {
          const double s = e / params_.robust_scale;
          w /= (1.0 + s * s);
        }
        const Eigen::Matrix<double, 1, 6> Jr = c.normal.transpose() * J;
        A += w * Jr.transpose() * Jr;
        b -= w * Jr.transpose() * e;
        ++n_planar;
        ++used;
      } else {
        // 線特徴: point-to-line。残差 = (I - d dᵀ)(p_w - μ) (線への垂直成分)。
        const Eigen::Vector3d d = c.line_dir;
        const Eigen::Matrix3d P =
            Eigen::Matrix3d::Identity() - d * d.transpose();
        const Eigen::Vector3d e = P * (src[k] - c.mean);
        if (e.norm() > kernel) continue;
        double w = params_.edge_weight * features[k].weight * c.linearity;
        if (params_.robust_scale > 0.0) {
          const double s = e.norm() / params_.robust_scale;
          w /= (1.0 + s * s);
        }
        const Eigen::Matrix<double, 3, 6> Je = P * J;
        A += w * Je.transpose() * Je;
        b -= w * Je.transpose() * e;
        ++n_edge;
        ++used;
      }
    }

    last_corr = used;
    last_planar = n_planar;
    last_edge = n_edge;
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
    result->num_planar = last_planar;
    result->num_edge = last_edge;
  }
  return T;
}

SmallMightyResult SmallMightyPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  SmallMightyResult result;

  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg = voxelDownsample(downsampled, params_.voxel_size);
  if (reg.empty()) reg = downsampled;

  if (frame_count_ == 0) {
    local_map_.addPoints(transformPointsHelper(downsampled, pose_));
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  const auto features = selectFeatures(reg);
  const Eigen::Matrix4d base = predict();
  const Eigen::Matrix4d new_pose = runRegistration(features, base, &result);

  const Eigen::Matrix4d delta = pose_.inverse() * new_pose;
  delta_window_.push_back(delta);
  if (static_cast<int>(delta_window_.size()) > 2) delta_window_.pop_front();
  pose_ = new_pose;

  local_map_.addPoints(transformPointsHelper(downsampled, pose_));
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

}  // namespace small_mighty
}  // namespace localization_zoo
