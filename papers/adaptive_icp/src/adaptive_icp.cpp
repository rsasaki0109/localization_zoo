#include "adaptive_icp/adaptive_icp.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace adaptive_icp {

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

double rotationAngle(const Eigen::Matrix3d& R) {
  const double c = std::clamp((R.trace() - 1.0) * 0.5, -1.0, 1.0);
  return std::acos(c);
}

}  // namespace

// ============================================================
// VoxelHashMap (genz_icp と同等)
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
// AdaptiveICP Pipeline
// ============================================================

AdaptiveICPPipeline::AdaptiveICPPipeline(const AdaptiveICPParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> AdaptiveICPPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> AdaptiveICPPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

std::vector<Eigen::Vector3d> AdaptiveICPPipeline::densityFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  if (points.size() < 8) return points;

  // 近傍点数を voxel グリッドで近似カウントする (KD-tree の代替)。
  const double radius = params_.voxel_size * params_.density_radius_scale;
  const double cell = radius;
  std::unordered_map<Eigen::Vector3i, std::vector<int>, VoxelHash> grid;
  grid.reserve(points.size());
  auto cellOf = [cell](const Eigen::Vector3d& p) {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / cell)),
                           static_cast<int>(std::floor(p.y() / cell)),
                           static_cast<int>(std::floor(p.z() / cell)));
  };
  for (int i = 0; i < static_cast<int>(points.size()); i++) {
    grid[cellOf(points[i])].push_back(i);
  }

  const double radius_sq = radius * radius;
  std::vector<int> density(points.size(), 0);
  for (int i = 0; i < static_cast<int>(points.size()); i++) {
    const auto key = cellOf(points[i]);
    int count = 0;
    for (int dx = -1; dx <= 1; dx++)
      for (int dy = -1; dy <= 1; dy++)
        for (int dz = -1; dz <= 1; dz++) {
          auto it = grid.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == grid.end()) continue;
          for (int j : it->second) {
            if ((points[i] - points[j]).squaredNorm() <= radius_sq) ++count;
          }
        }
    density[i] = count;
  }

  // α-percentile を密度しきい値にする。
  std::vector<int> sorted = density;
  std::sort(sorted.begin(), sorted.end());
  const double frac = std::clamp(params_.density_percentile / 100.0, 0.0, 0.95);
  const int idx = static_cast<int>(frac * (sorted.size() - 1));
  const int threshold = sorted[idx];

  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (int i = 0; i < static_cast<int>(points.size()); i++) {
    if (density[i] >= threshold) out.push_back(points[i]);
  }
  if (out.empty()) return points;
  return out;
}

Eigen::Matrix4d AdaptiveICPPipeline::predictMotion() const {
  if (recent_deltas_.empty()) return Eigen::Matrix4d::Identity();
  if (recent_deltas_.size() == 1) return pose_ * recent_deltas_.back();

  // 直近 2 つの相対姿勢を重み付き平均する (新しいほうを重く)。
  const Eigen::Matrix4d& d_old = recent_deltas_[recent_deltas_.size() - 2];
  const Eigen::Matrix4d& d_new = recent_deltas_.back();
  const double w_old = 0.35;
  const double w_new = 0.65;

  // 並進は線形ブレンド、回転は新しいほうを基準に補間。
  Eigen::Vector3d t_blend =
      w_old * d_old.block<3, 1>(0, 3) + w_new * d_new.block<3, 1>(0, 3);
  // 回転は新しい delta を採用 (短時間では近似十分)。
  Eigen::Matrix4d delta = d_new;
  delta.block<3, 1>(0, 3) = t_blend / (w_old + w_new);
  return pose_ * delta;
}

double AdaptiveICPPipeline::computeSigmaTh() const {
  if (weighted_errors_.empty()) return params_.initial_threshold;
  double num = 0.0;
  double den = 0.0;
  for (size_t i = 0; i < weighted_errors_.size(); i++) {
    const double g = motion_weights_[i];
    num += g * weighted_errors_[i] * weighted_errors_[i];
    den += 1.0;
  }
  if (den <= 0.0) return params_.initial_threshold;
  const double sigma = std::sqrt(num / den);
  // 初期値を上限、0.1*初期値を下限としてクランプ (発散・収縮防止)。
  return std::clamp(sigma, params_.initial_threshold * 0.1,
                    params_.initial_threshold);
}

Eigen::Matrix4d AdaptiveICPPipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, double sigma_th,
    int max_iters) const {
  Eigen::Matrix4d T = initial_guess;
  const double max_corr_dist = std::max(3.0 * sigma_th, params_.initial_threshold);
  const double sigma_sq = std::max(sigma_th * sigma_th, 1e-6);

  for (int iter = 0; iter < max_iters; iter++) {
    const auto src_transformed = transformPoints(source, T);
    const auto correspondences = local_map_.getCorrespondences(
        src_transformed, max_corr_dist, params_.normal_min_neighbors);

    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    int count = 0;

    for (size_t i = 0; i < source.size(); i++) {
      const auto& c = correspondences[i];
      if (!c.found) continue;

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src_transformed[i]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      double e;
      Eigen::Matrix<double, 1, 6> Jrow;
      if (c.has_normal && c.planarity >= params_.planarity_threshold) {
        // point-to-plane 残差
        e = c.normal.dot(src_transformed[i] - c.anchor);
        Jrow = c.normal.transpose() * J;
      } else {
        // 平面が取れない場合は最近傍点方向の point-to-point を 1 次元化。
        const Eigen::Vector3d d = src_transformed[i] - c.point;
        const double dn = d.norm();
        if (dn < 1e-9) continue;
        const Eigen::Vector3d dir = d / dn;
        e = dn;
        Jrow = dir.transpose() * J;
      }
      if (std::abs(e) > max_corr_dist) continue;

      // 適応ロバスト重み β = σ_th² / (σ_th² + e²)。
      const double beta = sigma_sq / (sigma_sq + e * e);
      JtJ += beta * Jrow.transpose() * Jrow;
      Jtb += beta * Jrow.transpose() * e;
      ++count;
    }

    if (count < 10) break;

    const Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) break;

    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;

    if (delta.norm() < params_.convergence_criterion) break;
  }
  return T;
}

AdaptiveICPResult AdaptiveICPPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  AdaptiveICPResult result;

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

  const double sigma_th = computeSigmaTh();
  result.sigma_th = sigma_th;

  // (1) 動き予測 T_pred。
  const Eigen::Matrix4d T_pred = predictMotion();

  // (2) 密度フィルタ点で粗位置合わせ T_align (少数反復)。
  const auto dense_pts = densityFilter(registration_points);
  const Eigen::Matrix4d T_align =
      runICP(dense_pts, T_pred, sigma_th, params_.coarse_max_iterations);

  // (3) 信頼初期姿勢の選択: T_pred と T_align の並進差で判定。
  const Eigen::Matrix4d diff = T_pred.inverse() * T_align;
  const double trans_diff = diff.block<3, 1>(0, 3).norm();
  Eigen::Matrix4d T_init;
  if (trans_diff <= params_.reliable_translation_tau) {
    T_init = T_align;
    result.used_coarse_pose = true;
  } else {
    T_init = T_pred;  // 粗解が予測から離れすぎ → 局所最適回避のため予測を採用。
    result.used_coarse_pose = false;
  }

  // (4) 適応 point-to-plane ICP (精位置合わせ)。
  const Eigen::Matrix4d new_pose =
      runICP(registration_points, T_init, sigma_th, params_.max_icp_iterations);

  // --- 動き予測履歴の更新 ---
  const Eigen::Matrix4d delta = pose_.inverse() * new_pose;
  recent_deltas_.push_back(delta);
  if (recent_deltas_.size() > 3) recent_deltas_.pop_front();

  // --- 適応しきい値履歴の更新 (motion-state 重み付き誤差) ---
  // モデル偏差 D = T_init^-1 * T_new から回転角 + 並進で誤差を作る。
  const Eigen::Matrix4d dev = T_init.inverse() * new_pose;
  const double theta = rotationAngle(dev.block<3, 3>(0, 0));
  const double e_i = params_.rot_scale * std::tanh(params_.rot_beta * theta) +
                     dev.block<3, 1>(0, 3).norm();

  // 加速度変化率から動的重み γ_i = exp(-||Δaccel|| / σ_decay)。
  const Eigen::Vector3d velocity = delta.block<3, 1>(0, 3);
  const Eigen::Vector3d accel = velocity - last_velocity_;
  const double accel_change = (accel - last_accel_).norm();
  const double gamma =
      std::exp(-accel_change / std::max(params_.motion_decay, 1e-3));
  last_accel_ = accel;
  last_velocity_ = velocity;

  weighted_errors_.push_back(e_i);
  motion_weights_.push_back(gamma);
  if (weighted_errors_.size() > 100) {
    weighted_errors_.pop_front();
    motion_weights_.pop_front();
  }

  pose_ = new_pose;

  auto world_pts = transformPoints(downsampled, pose_);
  local_map_.addPoints(world_pts);
  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    local_map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.converged = (pose_.array().isFinite().all() &&
                      (new_pose - T_init).cwiseAbs().maxCoeff() < 1e3);
  result.iterations = params_.max_icp_iterations;
  frame_count_++;

  return result;
}

}  // namespace adaptive_icp
}  // namespace localization_zoo
