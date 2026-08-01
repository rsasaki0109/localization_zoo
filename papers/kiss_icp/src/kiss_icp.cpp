#include "kiss_icp/kiss_icp.h"

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>

namespace localization_zoo {
namespace kiss_icp {

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

double modelDeviationError(const Eigen::Matrix4d& deviation,
                           double max_range) {
  const double angle =
      Eigen::AngleAxisd(deviation.block<3, 3>(0, 0)).angle();
  const double rotational_displacement =
      2.0 * max_range * std::sin(std::abs(angle) / 2.0);
  return deviation.block<3, 1>(0, 3).norm() + rotational_displacement;
}

Eigen::Vector3d logSO3(const Eigen::Matrix3d& rotation) {
  Eigen::AngleAxisd angle_axis(rotation);
  if (!std::isfinite(angle_axis.angle()) ||
      !angle_axis.axis().array().isFinite().all()) {
    return Eigen::Vector3d::Zero();
  }
  return angle_axis.angle() * angle_axis.axis();
}

Eigen::Matrix3d leftJacobianSO3(const Eigen::Vector3d& omega) {
  const double theta = omega.norm();
  const Eigen::Matrix3d omega_hat = skew(omega);
  if (theta < 1e-8) {
    return Eigen::Matrix3d::Identity() + 0.5 * omega_hat +
           (1.0 / 6.0) * omega_hat * omega_hat;
  }
  const double theta_squared = theta * theta;
  return Eigen::Matrix3d::Identity() +
         ((1.0 - std::cos(theta)) / theta_squared) * omega_hat +
         ((theta - std::sin(theta)) / (theta_squared * theta)) *
             omega_hat * omega_hat;
}

void correctElevationAngle(std::vector<Eigen::Vector3d>& points,
                           double angle_rad) {
  if (angle_rad == 0.0) return;
  const double sin_angle = std::sin(angle_rad);
  const double cos_angle = std::cos(angle_rad);
  for (auto& point : points) {
    const double horizontal = std::hypot(point.x(), point.y());
    if (horizontal <= 1e-12) continue;
    const double corrected_horizontal =
        horizontal * cos_angle - point.z() * sin_angle;
    const double corrected_z =
        point.z() * cos_angle + horizontal * sin_angle;
    const double horizontal_scale = corrected_horizontal / horizontal;
    point.x() *= horizontal_scale;
    point.y() *= horizontal_scale;
    point.z() = corrected_z;
  }
}

std::vector<Eigen::Vector3d> deskewScanToEnd(
    const std::vector<Eigen::Vector3d>& points,
    const std::vector<double>& relative_times,
    const Eigen::Matrix4d& relative_motion) {
  if (points.size() != relative_times.size() || points.empty()) return points;

  const Eigen::Matrix3d relative_rotation =
      relative_motion.block<3, 3>(0, 0);
  const Eigen::Vector3d omega = logSO3(relative_rotation);
  const Eigen::Vector3d translation =
      relative_motion.block<3, 1>(0, 3);
  const Eigen::Vector3d velocity =
      leftJacobianSO3(omega).colPivHouseholderQr().solve(translation);
  if (!omega.array().isFinite().all() ||
      !velocity.array().isFinite().all()) {
    return points;
  }

  std::vector<Eigen::Vector3d> deskewed(points.size());
  for (std::size_t index = 0; index < points.size(); ++index) {
    const double factor = std::clamp(relative_times[index], 0.0, 1.0) - 1.0;
    const Eigen::Vector3d scaled_omega = factor * omega;
    const Eigen::Matrix3d rotation = expSO3(scaled_omega);
    const Eigen::Vector3d offset =
        leftJacobianSO3(scaled_omega) * (factor * velocity);
    deskewed[index] = rotation * points[index] + offset;
  }
  return deskewed;
}

bool motionWithinLimits(const Eigen::Matrix4d& relative_motion,
                        double max_translation_m,
                        double max_rotation_rad) {
  if (!relative_motion.array().isFinite().all()) return false;
  const double translation =
      relative_motion.block<3, 1>(0, 3).norm();
  const double rotation = std::abs(
      Eigen::AngleAxisd(relative_motion.block<3, 3>(0, 0)).angle());
  return std::isfinite(translation) && std::isfinite(rotation) &&
         translation <= max_translation_m && rotation <= max_rotation_rad;
}

bool motionWithinAdaptiveLimits(
    const Eigen::Matrix4d& relative_motion,
    const Eigen::Matrix4d& previous_motion,
    double max_translation_m,
    double max_rotation_rad,
    double max_translation_multiplier,
    double translation_consistency_m,
    double rotation_consistency_rad) {
  if (!relative_motion.array().isFinite().all() ||
      !previous_motion.array().isFinite().all()) {
    return false;
  }
  const double translation =
      relative_motion.block<3, 1>(0, 3).norm();
  const double rotation = std::abs(
      Eigen::AngleAxisd(relative_motion.block<3, 3>(0, 0)).angle());
  if (!std::isfinite(translation) || !std::isfinite(rotation) ||
      translation > max_translation_m * max_translation_multiplier ||
      rotation > max_rotation_rad) {
    return false;
  }
  const Eigen::Matrix4d deviation =
      previous_motion.inverse() * relative_motion;
  return motionWithinLimits(deviation, translation_consistency_m,
                            rotation_consistency_rad);
}

// ============================================================
// KISS Pair Matcher
// ============================================================

KISSMatcher::KISSMatcher(const KISSMatcherParams& params)
    : params_(params),
      target_map_(params.target_voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> KISSMatcher::preprocess(
    const std::vector<Eigen::Vector3d>& points, double voxel_size) const {
  std::unordered_map<Eigen::Vector3i, Eigen::Vector3d, VoxelHash> grid;
  for (const auto& p : points) {
    const double r = p.norm();
    if (r < params_.min_range || r > params_.max_range) continue;
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

void KISSMatcher::setTarget(const std::vector<Eigen::Vector3d>& target) {
  target_map_.clear();
  target_map_.addPoints(preprocess(target, params_.target_voxel_size));
}

KISSMatcherResult KISSMatcher::align(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess) const {
  KISSMatcherResult result;
  result.transform = initial_guess;

  const auto source_points = preprocess(source, params_.source_voxel_size);
  if (source_points.empty() || target_map_.size() == 0) return result;

  for (int iter = 0; iter < params_.max_icp_iterations; ++iter) {
    const auto src_transformed = transformPoints(source_points, result.transform);
    const auto correspondences = target_map_.getCorrespondences(
        src_transformed, params_.max_correspondence_distance);

    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    double squared_error_sum = 0.0;
    int valid_correspondences = 0;

    for (size_t i = 0; i < source_points.size(); ++i) {
      if (!correspondences[i].found) continue;

      const Eigen::Vector3d residual =
          src_transformed[i] - correspondences[i].point;
      const double r_norm = residual.norm();
      if (r_norm > params_.max_correspondence_distance) continue;

      const double sigma = params_.max_correspondence_distance;
      const double weight = std::exp(-0.5 * (r_norm / sigma) * (r_norm / sigma));

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src_transformed[i]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      JtJ += weight * J.transpose() * J;
      Jtb += weight * J.transpose() * residual;
      squared_error_sum += residual.squaredNorm();
      ++valid_correspondences;
    }

    result.iterations = iter + 1;
    result.num_correspondences = valid_correspondences;
    result.rmse = valid_correspondences > 0
                      ? std::sqrt(squared_error_sum / valid_correspondences)
                      : std::numeric_limits<double>::infinity();
    const Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 6, 6>>
        information_solver(JtJ);
    if (information_solver.info() == Eigen::Success) {
      result.information_min_eigenvalue =
          information_solver.eigenvalues().minCoeff();
      result.information_max_eigenvalue =
          information_solver.eigenvalues().maxCoeff();
      if (result.information_min_eigenvalue > 1e-12) {
        result.information_condition =
            result.information_max_eigenvalue /
            result.information_min_eigenvalue;
      }
    }

    if (valid_correspondences < params_.min_correspondences) break;

    const Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) break;

    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    result.transform = dT * result.transform;

    if (delta.norm() < params_.convergence_criterion) {
      result.converged = true;
      break;
    }
  }

  if (!result.converged) {
    result.converged =
        result.num_correspondences >= params_.min_correspondences &&
        result.transform.array().isFinite().all();
  }
  return result;
}

// ============================================================
// VoxelHashMap
// ============================================================

void VoxelHashMap::addPoints(const std::vector<Eigen::Vector3d>& points) {
  for (auto& p : points) {
    auto key = toVoxel(p);
    auto& vb = map_[key];
    if (static_cast<int>(vb.points.size()) < max_points_) {
      vb.points.push_back(p);
    } else if (update_full_voxels_) {
      const std::size_t replacement =
          static_cast<std::size_t>(vb.observations %
                                   static_cast<std::uint64_t>(max_points_));
      vb.points[replacement] = p;
    }
    ++vb.observations;
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
    const std::vector<Eigen::Vector3d>& points, double max_dist) const {
  std::vector<Correspondence> correspondences(points.size());
  double max_dist_sq = max_dist * max_dist;

  // Queries are independent and the map is immutable during registration.
  // Keep normal-equation accumulation serial so this changes runtime, not the
  // selected correspondences or floating-point reduction order.
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
  for (std::int64_t i = 0; i < static_cast<std::int64_t>(points.size()); i++) {
    const auto& query = points[i];
    auto key = toVoxel(query);
    const int search_radius = std::max(
        0, static_cast<int>(std::ceil(max_dist / voxel_size_)));
    double best_dist = max_dist_sq;
    Eigen::Vector3d best_point = Eigen::Vector3d::Zero();
    bool found = false;

    for (int dx = -search_radius; dx <= search_radius; dx++) {
      for (int dy = -search_radius; dy <= search_radius; dy++) {
        for (int dz = -search_radius; dz <= search_radius; dz++) {
          Eigen::Vector3i neighbor = key + Eigen::Vector3i(dx, dy, dz);
          const Eigen::Vector3d voxel_min =
              neighbor.cast<double>() * voxel_size_;
          const Eigen::Vector3d voxel_max =
              voxel_min + Eigen::Vector3d::Constant(voxel_size_);
          const Eigen::Vector3d outside =
              (voxel_min - query).cwiseMax(Eigen::Vector3d::Zero()) +
              (query - voxel_max).cwiseMax(Eigen::Vector3d::Zero());
          if (outside.squaredNorm() >= best_dist) continue;
          auto it = map_.find(neighbor);
          if (it == map_.end()) continue;
          for (const auto& mp : it->second.points) {
            const double ddx = mp.x() - query.x();
            const double ddy = mp.y() - query.y();
            const double ddz = mp.z() - query.z();
            const double d = ddx * ddx + ddy * ddy + ddz * ddz;
            if (d < best_dist) {
              best_dist = d;
              best_point = mp;
              found = true;
            }
          }
        }
      }
    }
    correspondences[i].point = best_point;
    correspondences[i].found = found;
  }
  return correspondences;
}

// ============================================================
// KISS-ICP Pipeline
// ============================================================

KISSICPPipeline::KISSICPPipeline(const KISSICPParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel,
                 params.update_full_voxels),
      model_error_squared_sum_(params.initial_threshold *
                               params.initial_threshold),
      model_error_samples_(1) {}

std::vector<Eigen::Vector3d> KISSICPPipeline::voxelDownsample(
    const std::vector<Eigen::Vector3d>& points, double voxel_size) const {
  std::unordered_map<Eigen::Vector3i, Eigen::Vector3d, VoxelHash> grid;
  for (auto& p : points) {
    Eigen::Vector3i key(static_cast<int>(std::floor(p.x() / voxel_size)),
                        static_cast<int>(std::floor(p.y() / voxel_size)),
                        static_cast<int>(std::floor(p.z() / voxel_size)));
    grid.emplace(key, p);  // 各ボクセルから1点だけ
  }
  std::vector<Eigen::Vector3d> out;
  out.reserve(grid.size());
  for (auto& [k, v] : grid) out.push_back(v);
  return out;
}

std::vector<Eigen::Vector3d> KISSICPPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (auto& p : points) {
    double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d KISSICPPipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const std::vector<Eigen::Vector3d>& target,
    const Eigen::Matrix4d& initial_guess, double max_correspondence_dist,
    double kernel_threshold) {
  (void)target;
  Eigen::Matrix4d T = initial_guess;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    auto src_transformed = transformPoints(source, T);

    // 対応関係
    auto correspondences =
        local_map_.getCorrespondences(src_transformed, max_correspondence_dist);

    // Point-to-Point ICP (ロバストカーネル付き)
    // min Σ w_i ||T p_i - q_i||²
    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();

    int valid_correspondences = 0;

    for (size_t i = 0; i < source.size(); i++) {
      if (!correspondences[i].found) continue;

      Eigen::Vector3d residual = src_transformed[i] - correspondences[i].point;
      double r_norm = residual.norm();
      if (r_norm > max_correspondence_dist) continue;
      valid_correspondences++;

      // Welsch kernel weight
      double w = std::exp(-0.5 * (r_norm / kernel_threshold) *
                          (r_norm / kernel_threshold));

      // Residual r = T p - q, so J = [-skew(Tp), I]
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src_transformed[i]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      JtJ += w * J.transpose() * J;
      Jtb += w * J.transpose() * residual;
    }

    if (valid_correspondences < 10) {
      break;
    }

    Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) {
      break;
    }

    // 更新
    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;

    if (delta.norm() < params_.convergence_criterion) break;
  }
  return T;
}

double KISSICPPipeline::computeAdaptiveThreshold() {
  if (params_.use_model_deviation_threshold) {
    return std::sqrt(model_error_squared_sum_ /
                     static_cast<double>(model_error_samples_));
  }
  if (model_errors_.empty()) return params_.initial_threshold;
  // σ = median of recent model errors
  auto sorted = model_errors_;
  std::sort(sorted.begin(), sorted.end());
  double median = sorted[sorted.size() / 2];
  return std::max(3.0 * median, params_.initial_threshold * 0.1);
}

double KISSICPPipeline::adaptiveThreshold() const {
  if (params_.use_model_deviation_threshold) {
    return std::sqrt(model_error_squared_sum_ /
                     static_cast<double>(model_error_samples_));
  }
  if (model_errors_.empty()) return params_.initial_threshold;
  auto sorted = model_errors_;
  std::sort(sorted.begin(), sorted.end());
  const double median = sorted[sorted.size() / 2];
  return std::max(3.0 * median, params_.initial_threshold * 0.1);
}

KISSICPResult KISSICPPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  KISSICPResult result;

  // 1. Range filter + Voxel downsample
  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto registration_points = voxelDownsample(downsampled, params_.voxel_size);
  if (registration_points.empty()) registration_points = downsampled;

  if (frame_count_ == 0) {
    // 最初のフレーム
    auto world_pts = transformPoints(downsampled, pose_);
    local_map_.addPoints(world_pts);
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  // 2. Motion prediction (等速モデル)
  Eigen::Matrix4d prediction = pose_ * last_delta_;

  // 3. 適応的閾値
  double adaptive_threshold = computeAdaptiveThreshold();

  // 4. ICP
  const double correspondence_threshold =
      params_.use_model_deviation_threshold
          ? params_.model_deviation_correspondence_multiplier *
                adaptive_threshold
          : adaptive_threshold;
  Eigen::Matrix4d new_pose = runICP(registration_points, {}, prediction,
                                    correspondence_threshold,
                                    adaptive_threshold);

  // 5. モデルエラーの更新
  Eigen::Matrix4d candidate_delta = pose_.inverse() * new_pose;
  const bool within_fixed_guard = motionWithinLimits(
      candidate_delta, params_.max_step_translation_m,
      params_.max_step_rotation_rad);
  const bool within_adaptive_guard =
      params_.enable_motion_guard && params_.enable_adaptive_motion_guard &&
      trusted_motion_steps_ >=
          params_.adaptive_motion_guard_min_trusted_steps &&
      motionWithinAdaptiveLimits(
          candidate_delta, last_delta_, params_.max_step_translation_m,
          params_.max_step_rotation_rad,
          params_.adaptive_motion_guard_max_translation_multiplier,
          params_.adaptive_motion_guard_translation_consistency_m,
          params_.adaptive_motion_guard_rotation_consistency_rad);
  if (params_.enable_motion_guard && !within_fixed_guard &&
      !within_adaptive_guard) {
    // A rejected pose must not poison the constant-velocity deskew or map.
    // Hold the last trusted pose and restart the motion prior from rest.
    new_pose = pose_;
    last_delta_ = Eigen::Matrix4d::Identity();
    result.motion_guard_rejected = true;
    ++motion_guard_rejections_;
  } else {
    last_delta_ = candidate_delta;
    ++trusted_motion_steps_;
    if (!within_fixed_guard && within_adaptive_guard) {
      result.motion_guard_adaptive_accepted = true;
      ++adaptive_motion_guard_acceptances_;
    }
  }
  if (!result.motion_guard_rejected &&
      params_.use_model_deviation_threshold) {
    // KISS-ICP defines uncertainty as prediction-versus-registration
    // deviation, not vehicle motion. Rotation is converted to the maximum
    // induced point displacement at the configured sensor range.
    const Eigen::Matrix4d model_deviation = prediction.inverse() * new_pose;
    const double model_error =
        modelDeviationError(model_deviation, params_.max_range);
    if (model_error > 0.1 && std::isfinite(model_error)) {
      model_error_squared_sum_ += model_error * model_error;
      ++model_error_samples_;
    }
  } else if (!result.motion_guard_rejected) {
    const double model_error = last_delta_.block<3, 1>(0, 3).norm();
    model_errors_.push_back(model_error);
    if (model_errors_.size() > 100) model_errors_.erase(model_errors_.begin());
  }

  pose_ = new_pose;

  // 6. マップ更新
  if (!result.motion_guard_rejected) {
    auto world_pts = transformPoints(downsampled, pose_);
    local_map_.addPoints(world_pts);
    if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
        (frame_count_ % params_.map_cleanup_interval) == 0) {
      local_map_.pruneFarVoxels(pose_.block<3, 1>(0, 3),
                                params_.local_map_radius);
    }
  }

  result.pose = pose_;
  result.converged = (!result.motion_guard_rejected &&
                      pose_.array().isFinite().all() &&
                      (new_pose - prediction).cwiseAbs().maxCoeff() < 1e3);
  result.iterations = params_.max_icp_iterations;
  frame_count_++;

  return result;
}

}  // namespace kiss_icp
}  // namespace localization_zoo
