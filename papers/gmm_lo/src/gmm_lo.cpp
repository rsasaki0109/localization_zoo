#include "gmm_lo/gmm_lo.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>

namespace localization_zoo {
namespace gmm_lo {

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

std::vector<VoxelHashMap::SoftCorrespondence> VoxelHashMap::getSoftCorrespondences(
    const std::vector<Eigen::Vector3d>& points, double max_dist, double sigma,
    double outlier_weight, int normal_min_neighbors) const {
  std::vector<SoftCorrespondence> out(points.size());
  const double max_dist_sq = max_dist * max_dist;
  const double inv_two_sigma2 = 1.0 / (2.0 * sigma * sigma);

  for (size_t i = 0; i < points.size(); i++) {
    const auto& query = points[i];
    auto key = toVoxel(query);
    std::vector<Eigen::Vector3d> neighbors;
    std::vector<double> resp;
    neighbors.reserve(48);
    resp.reserve(48);
    double w_sum = 0.0;
    Eigen::Vector3d soft = Eigen::Vector3d::Zero();

    for (int dx = -1; dx <= 1; dx++)
      for (int dy = -1; dy <= 1; dy++)
        for (int dz = -1; dz <= 1; dz++) {
          auto it = map_.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == map_.end()) continue;
          for (const auto& mp : it->second.points) {
            const double d2 = (mp - query).squaredNorm();
            if (d2 >= max_dist_sq) continue;
            const double g = std::exp(-d2 * inv_two_sigma2);
            neighbors.push_back(mp);
            resp.push_back(g);
            w_sum += g;
            soft += g * mp;
          }
        }

    SoftCorrespondence& c = out[i];
    if (neighbors.empty() || w_sum < 1e-12) continue;
    c.found = true;
    c.centroid = soft / w_sum;
    c.weight = w_sum / (w_sum + outlier_weight);  // インライア信頼度

    if (static_cast<int>(neighbors.size()) >= normal_min_neighbors) {
      // responsibility 重み付き共分散から局所法線を推定。
      Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
      for (size_t k = 0; k < neighbors.size(); k++) {
        const Eigen::Vector3d d = neighbors[k] - c.centroid;
        cov += resp[k] * d * d.transpose();
      }
      cov /= w_sum;
      Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
      if (solver.info() == Eigen::Success) {
        const Eigen::Vector3d ev = solver.eigenvalues();
        const double lambda2 = std::max(ev(2), 1e-12);
        c.planarity = std::clamp((ev(1) - ev(0)) / lambda2, 0.0, 1.0);
        c.normal = solver.eigenvectors().col(0).normalized();
        c.has_normal = true;
      }
    }
  }
  return out;
}

// ============================================================
// GmmLoPipeline
// ============================================================

Eigen::Vector3d GmmLoPipeline::softCentroid(
    const Eigen::Vector3d& query, const std::vector<Eigen::Vector3d>& neighbors,
    double sigma) {
  const double inv_two_sigma2 = 1.0 / (2.0 * sigma * sigma);
  double w_sum = 0.0;
  Eigen::Vector3d soft = Eigen::Vector3d::Zero();
  for (const auto& q : neighbors) {
    const double g = std::exp(-(q - query).squaredNorm() * inv_two_sigma2);
    w_sum += g;
    soft += g * q;
  }
  if (w_sum < 1e-12) return query;
  return soft / w_sum;
}

GmmLoPipeline::GmmLoPipeline(const GmmLoParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> GmmLoPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> GmmLoPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d GmmLoPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  return pose_ * delta_window_.back();
}

Eigen::Matrix4d GmmLoPipeline::runRegistration(
    const std::vector<Eigen::Vector3d>& source, const Eigen::Matrix4d& base,
    GmmLoResult* result) {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;

  Eigen::Matrix4d T = base;
  int last_corr = 0;
  double mean_w = 0.0;
  const double anneal =
      params_.max_iterations > 1
          ? std::pow(params_.sigma_final / params_.sigma_init,
                     1.0 / (params_.max_iterations - 1))
          : 1.0;
  double sigma = params_.sigma_init;

  for (int it = 0; it < params_.max_iterations; it++) {
    const auto src = transformPoints(source, T);
    const double radius = std::max(params_.neighbor_radius_scale * sigma,
                                   params_.voxel_size);
    const auto corr = local_map_.getSoftCorrespondences(
        src, radius, sigma, params_.outlier_weight, params_.normal_min_neighbors);

    Mat6 A = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    int used = 0;
    double w_sum = 0.0;
    for (size_t k = 0; k < source.size(); k++) {
      const auto& c = corr[k];
      if (!c.found || !c.has_normal || c.planarity < params_.planarity_threshold)
        continue;
      const double e = c.normal.dot(src[k] - c.centroid);
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src[k]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const Eigen::Matrix<double, 1, 6> Jr = c.normal.transpose() * J;
      const double w = c.weight;  // EM インライア信頼度
      A += w * Jr.transpose() * Jr;
      b -= w * Jr.transpose() * e;
      w_sum += w;
      ++used;
    }
    if (used < 10) break;
    mean_w = w_sum / static_cast<double>(used);

    const Vec6 d = A.ldlt().solve(b);
    if (!d.allFinite()) break;
    T = expSE3(d) * T;
    last_corr = used;
    if (result) result->iterations = it + 1;
    sigma *= anneal;  // deterministic annealing
    if (d.norm() < params_.convergence_criterion && it > 2) break;
  }

  if (result) {
    result->num_correspondences = last_corr;
    result->mean_weight = mean_w;
  }
  return T;
}

GmmLoResult GmmLoPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  GmmLoResult result;

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

}  // namespace gmm_lo
}  // namespace localization_zoo
