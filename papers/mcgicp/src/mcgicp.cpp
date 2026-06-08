#include "mcgicp/mcgicp.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

#include <algorithm>
#include <cmath>

namespace localization_zoo {
namespace mcgicp {

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

std::vector<PointI> transformPoints(const std::vector<PointI>& pts,
                                    const Eigen::Matrix4d& T) {
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  std::vector<PointI> out(pts.size());
  for (size_t i = 0; i < pts.size(); i++) {
    out[i].p = R * pts[i].p + t;
    out[i].intensity = pts[i].intensity;
  }
  return out;
}

}  // namespace

// ============================================================
// McGicpCore
// ============================================================

Vec4 McGicpCore::toVec4(const PointI& pt, double intensity_scale) {
  Vec4 v;
  v << pt.p.x(), pt.p.y(), pt.p.z(), intensity_scale * pt.intensity;
  return v;
}

Mat4 McGicpCore::computeCovariance(const std::vector<PointI>& neighbors,
                                   double intensity_scale,
                                   double normal_epsilon,
                                   double intensity_epsilon) {
  Mat4 cov = Mat4::Zero();
  if (neighbors.size() < 4) {
    cov(0, 0) = cov(1, 1) = cov(2, 2) = 1.0;
    cov(3, 3) = intensity_epsilon;
    return cov;
  }

  Vec4 mean = Vec4::Zero();
  for (const auto& n : neighbors) mean += toVec4(n, intensity_scale);
  mean /= static_cast<double>(neighbors.size());

  for (const auto& n : neighbors) {
    const Vec4 d = toVec4(n, intensity_scale) - mean;
    cov += d * d.transpose();
  }
  cov /= static_cast<double>(neighbors.size());

  Eigen::SelfAdjointEigenSolver<Mat4> solver(cov);
  if (solver.info() != Eigen::Success) {
    cov.setIdentity();
    cov(3, 3) = intensity_epsilon;
    return cov;
  }

  const Mat4 U = solver.eigenvectors();
  Eigen::Matrix<double, 4, 1> lambda = solver.eigenvalues();
  Eigen::Matrix<double, 4, 1> reg;
  reg(0) = normal_epsilon;
  reg(1) = 1.0;
  reg(2) = 1.0;
  reg(3) = std::max(lambda(3), intensity_epsilon);
  return U * reg.asDiagonal() * U.transpose();
}

double McGicpCore::mahalanobisSquared(const Vec4& a, const Vec4& b,
                                      const Mat4& cov_a, const Mat4& cov_b) {
  const Vec4 r = a - b;
  const Mat4 metric = (cov_a + cov_b).inverse();
  return r.dot(metric * r);
}

double McGicpCore::intensityWeight(double intensity_residual,
                                   double intensity_epsilon) {
  const double s = std::max(intensity_epsilon, 1e-6);
  return std::exp(-0.5 * (intensity_residual / s) * (intensity_residual / s));
}

// ============================================================
// VoxelHashMap
// ============================================================

void VoxelHashMap::clear() { map_.clear(); }

void VoxelHashMap::addPoints(const std::vector<PointI>& points) {
  for (const auto& pt : points) {
    auto& vb = map_[toVoxel(pt.p)];
    if (static_cast<int>(vb.points.size()) < max_points_) {
      vb.points.push_back(pt.p);
      vb.intensity.push_back(pt.intensity);
    }
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
    const std::vector<PointI>& queries, double max_dist, int normal_min_neighbors,
    double intensity_scale, double normal_epsilon,
    double intensity_epsilon) const {
  std::vector<Correspondence> correspondences(queries.size());
  const double max_dist_sq = max_dist * max_dist;

  for (size_t i = 0; i < queries.size(); i++) {
    const auto& query = queries[i];
    auto key = toVoxel(query.p);
    bool found = false;
    std::vector<PointI> neighbors;
    neighbors.reserve(32);

    for (int dx = -1; dx <= 1; dx++)
      for (int dy = -1; dy <= 1; dy++)
        for (int dz = -1; dz <= 1; dz++) {
          auto it = map_.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == map_.end()) continue;
          for (size_t j = 0; j < it->second.points.size(); j++) {
            const double d = (it->second.points[j] - query.p).squaredNorm();
            if (d < max_dist_sq) {
              neighbors.push_back(
                  {it->second.points[j], it->second.intensity[j]});
              found = true;
            }
          }
        }

    Correspondence& c = correspondences[i];
    c.found = found;
    if (!found || static_cast<int>(neighbors.size()) < normal_min_neighbors) {
      continue;
    }

    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
    double mean_i = 0.0;
    for (const auto& n : neighbors) {
      centroid += n.p;
      mean_i += n.intensity;
    }
    centroid /= static_cast<double>(neighbors.size());
    mean_i /= static_cast<double>(neighbors.size());

    Eigen::Matrix3d cov3 = Eigen::Matrix3d::Zero();
    for (const auto& n : neighbors) {
      const Eigen::Vector3d d = n.p - centroid;
      cov3 += d * d.transpose();
    }
    cov3 /= static_cast<double>(neighbors.size());

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov3);
    if (solver.info() != Eigen::Success) continue;

    const Eigen::Vector3d ev = solver.eigenvalues();
    const double lambda2 = std::max(ev(2), 1e-12);
    c.planarity = std::clamp((ev(1) - ev(0)) / lambda2, 0.0, 1.0);
    c.normal = solver.eigenvectors().col(0).normalized();
    c.anchor = centroid;
    c.mean_intensity = mean_i;
    c.local_cov = McGicpCore::computeCovariance(neighbors, intensity_scale,
                                                normal_epsilon, intensity_epsilon);
    c.has_normal = true;
  }
  return correspondences;
}

// ============================================================
// McGicpLoPipeline
// ============================================================

McGicpLoPipeline::McGicpLoPipeline(const McGicpLoParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<PointI> McGicpLoPipeline::voxelDownsample(
    const std::vector<PointI>& points, double voxel_size) const {
  std::unordered_map<Eigen::Vector3i, PointI, VoxelHash> grid;
  for (const auto& pt : points) {
    Eigen::Vector3i key(static_cast<int>(std::floor(pt.p.x() / voxel_size)),
                        static_cast<int>(std::floor(pt.p.y() / voxel_size)),
                        static_cast<int>(std::floor(pt.p.z() / voxel_size)));
    grid.emplace(key, pt);
  }
  std::vector<PointI> out;
  out.reserve(grid.size());
  for (const auto& kv : grid) out.push_back(kv.second);
  return out;
}

std::vector<PointI> McGicpLoPipeline::rangeFilter(
    const std::vector<PointI>& points) const {
  std::vector<PointI> out;
  out.reserve(points.size());
  for (const auto& pt : points) {
    const double r = pt.p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(pt);
  }
  return out;
}

Eigen::Matrix4d McGicpLoPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  return pose_ * delta_window_.back();
}

Eigen::Matrix4d McGicpLoPipeline::runRegistration(
    const std::vector<PointI>& source, const Eigen::Matrix4d& base,
    McGicpLoResult* result) {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;

  const double intensity_scale =
      params_.enable_intensity ? params_.intensity_scale : 0.0;

  Eigen::Matrix4d T = base;
  int last_corr = 0;
  double mean_i_res = 0.0;

  for (int it = 0; it < params_.max_iterations; it++) {
    const auto transformed = transformPoints(source, T);
    const auto corr = local_map_.getCorrespondences(
        transformed, params_.max_correspondence_dist,
        params_.normal_min_neighbors, intensity_scale, params_.normal_epsilon,
        params_.intensity_epsilon);

    Mat6 H = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    int used = 0;
    double i_res_sum = 0.0;

    for (size_t k = 0; k < source.size(); k++) {
      const auto& c = corr[k];
      if (!c.found || !c.has_normal ||
          c.planarity < params_.planarity_threshold) {
        continue;
      }

      const double e = c.normal.dot(transformed[k].p - c.anchor);
      if (std::abs(e) > params_.initial_threshold) continue;
      const double di = transformed[k].intensity - c.mean_intensity;
      double w = 1.0;
      if (params_.enable_intensity) {
        w = McGicpCore::intensityWeight(di, params_.intensity_epsilon);
      }

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(transformed[k].p);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const Eigen::Matrix<double, 1, 6> Jr = c.normal.transpose() * J;

      H += w * Jr.transpose() * Jr;
      b -= w * Jr.transpose() * e;
      i_res_sum += std::abs(di);
      ++used;
    }

    if (used < 10) break;
    mean_i_res = i_res_sum / static_cast<double>(used);

    const Vec6 delta = H.ldlt().solve(b);
    if (!delta.allFinite()) break;
    T = expSE3(delta) * T;
    last_corr = used;
    if (result) result->iterations = it + 1;
    if (delta.norm() < params_.convergence_criterion) break;
  }

  if (result) {
    result->num_correspondences = last_corr;
    result->mean_intensity_residual = mean_i_res;
  }
  return T;
}

McGicpLoResult McGicpLoPipeline::registerFrame(
    const std::vector<PointI>& frame) {
  McGicpLoResult result;

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
  result.converged = pose_.array().isFinite().all();
  frame_count_++;
  return result;
}

}  // namespace mcgicp
}  // namespace localization_zoo
