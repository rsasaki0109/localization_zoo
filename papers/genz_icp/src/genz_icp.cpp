#include "genz_icp/genz_icp.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace genz_icp {

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
    if (static_cast<int>(vb.points.size()) < max_points_) {
      vb.points.push_back(p);
    }
  }
}

void VoxelHashMap::pruneFarVoxels(const Eigen::Vector3d& center,
                                  double max_distance) {
  const double max_distance_sq = max_distance * max_distance;
  for (auto it = map_.begin(); it != map_.end();) {
    const Eigen::Vector3d voxel_center =
        (it->first.cast<double>() + Eigen::Vector3d::Constant(0.5)) * voxel_size_;
    if ((voxel_center - center).squaredNorm() > max_distance_sq) {
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

    // 法線推定用に近傍点を集める。
    std::vector<Eigen::Vector3d> neighbors;
    neighbors.reserve(32);

    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
          Eigen::Vector3i neighbor = key + Eigen::Vector3i(dx, dy, dz);
          auto it = map_.find(neighbor);
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
        // 固有値は昇順 (λ0 <= λ1 <= λ2)。
        const Eigen::Vector3d ev = solver.eigenvalues();
        const double lambda2 = std::max(ev(2), 1e-12);
        // planarity = (λ1 - λ0) / λ2。平面なら λ0≈0, λ1≈λ2 で 1 に近づく。
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
// GenZ-ICP Pipeline
// ============================================================

GenZICPPipeline::GenZICPPipeline(const GenZICPParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> GenZICPPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> GenZICPPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d GenZICPPipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, double max_correspondence_dist,
    double* planar_ratio_out, double* alpha_out) {
  Eigen::Matrix4d T = initial_guess;
  double last_planar_ratio = 0.0;
  double last_alpha = 0.0;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const auto src_transformed = transformPoints(source, T);
    const auto correspondences = local_map_.getCorrespondences(
        src_transformed, max_correspondence_dist, params_.normal_min_neighbors);

    // 平面 (point-to-plane) と非平面 (point-to-point) を別々に蓄積する。
    Eigen::Matrix<double, 6, 6> JtJ_plane = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb_plane = Eigen::Matrix<double, 6, 1>::Zero();
    Eigen::Matrix<double, 6, 6> JtJ_point = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb_point = Eigen::Matrix<double, 6, 1>::Zero();
    int count_plane = 0;
    int count_point = 0;

    const double kernel = max_correspondence_dist;

    for (size_t i = 0; i < source.size(); i++) {
      const auto& c = correspondences[i];
      if (!c.found) continue;

      const Eigen::Vector3d e = src_transformed[i] - c.point;  // point-to-point 残差
      const double r_norm = e.norm();
      if (r_norm > max_correspondence_dist) continue;

      const double w = std::exp(-0.5 * (r_norm / kernel) * (r_norm / kernel));

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src_transformed[i]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      if (c.has_normal && c.planarity >= params_.planarity_threshold) {
        // point-to-plane: スカラー残差 n·(Tp - anchor)
        const Eigen::Matrix<double, 1, 6> Jp = c.normal.transpose() * J;
        const double rp = c.normal.dot(src_transformed[i] - c.anchor);
        JtJ_plane += w * Jp.transpose() * Jp;
        Jtb_plane += w * Jp.transpose() * rp;
        ++count_plane;
      } else {
        // point-to-point: 3 次元残差
        JtJ_point += w * J.transpose() * J;
        Jtb_point += w * J.transpose() * e;
        ++count_point;
      }
    }

    if (count_plane + count_point < 10) break;

    // GenZ-ICP の adaptive weighting。
    // 平面が支配的な (= 平面法線方向以外が拘束されにくい縮退) 場面ほど
    // point-to-point 項を相対的に強めて縮退を抑える。
    last_planar_ratio =
        static_cast<double>(count_plane) / (count_plane + count_point);
    const double alpha = last_planar_ratio;  // point-to-point 側の重み
    last_alpha = alpha;

    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    if (count_plane > 0) {
      JtJ += (1.0 - alpha) * (JtJ_plane / count_plane);
      Jtb += (1.0 - alpha) * (Jtb_plane / count_plane);
    }
    if (count_point > 0) {
      JtJ += alpha * (JtJ_point / count_point);
      Jtb += alpha * (Jtb_point / count_point);
    }

    const Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) break;

    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;

    if (delta.norm() < params_.convergence_criterion) break;
  }

  if (planar_ratio_out) *planar_ratio_out = last_planar_ratio;
  if (alpha_out) *alpha_out = last_alpha;
  return T;
}

double GenZICPPipeline::computeAdaptiveThreshold() {
  if (model_errors_.empty()) return params_.initial_threshold;
  auto sorted = model_errors_;
  std::sort(sorted.begin(), sorted.end());
  const double median = sorted[sorted.size() / 2];
  return std::max(3.0 * median, params_.initial_threshold * 0.1);
}

GenZICPResult GenZICPPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  GenZICPResult result;

  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto registration_points = voxelDownsample(downsampled, params_.voxel_size);
  if (registration_points.empty()) registration_points = downsampled;

  if (frame_count_ == 0) {
    auto world_pts = transformPoints(downsampled, pose_);
    local_map_.addPoints(world_pts);
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  const Eigen::Matrix4d prediction = pose_ * last_delta_;
  const double adaptive_threshold = computeAdaptiveThreshold();

  double planar_ratio = 0.0;
  double alpha = 0.0;
  const Eigen::Matrix4d new_pose = runICP(registration_points, prediction,
                                          adaptive_threshold, &planar_ratio,
                                          &alpha);

  last_delta_ = pose_.inverse() * new_pose;
  const double model_error = last_delta_.block<3, 1>(0, 3).norm();
  model_errors_.push_back(model_error);
  if (model_errors_.size() > 100) model_errors_.erase(model_errors_.begin());

  pose_ = new_pose;

  auto world_pts = transformPoints(downsampled, pose_);
  local_map_.addPoints(world_pts);
  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    local_map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.converged = (pose_.array().isFinite().all() &&
                      (new_pose - prediction).cwiseAbs().maxCoeff() < 1e3);
  result.iterations = params_.max_icp_iterations;
  result.planar_ratio = planar_ratio;
  result.alpha = alpha;
  frame_count_++;

  return result;
}

}  // namespace genz_icp
}  // namespace localization_zoo
