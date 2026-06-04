#include "d2lio/d2lio.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace d2lio {

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
  const double cos_theta = std::clamp((R.trace() - 1.0) * 0.5, -1.0, 1.0);
  const double theta = std::acos(cos_theta);
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
// D2LIO Pipeline
// ============================================================

D2LIOPipeline::D2LIOPipeline(const D2LIOParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> D2LIOPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> D2LIOPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d D2LIOPipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, const Eigen::Matrix4d& prediction,
    double t_rel, double theta_rel, const Eigen::Matrix3d& imu_rot_info,
    D2LIOResult* result) {
  Eigen::Matrix4d T = initial_guess;
  const double sin_half = std::sin(0.5 * theta_rel);
  const double cauchy_sq = std::max(params_.cauchy_scale * params_.cauchy_scale,
                                    1e-6);

  int last_deg_rot = 0;
  int last_deg_trans = 0;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const auto src_transformed = transformPoints(source, T);
    // 対応探索の許容距離は ε_j の上限で十分大きく取る。
    const auto correspondences = local_map_.getCorrespondences(
        src_transformed, params_.max_threshold, params_.normal_min_neighbors);

    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    int count = 0;

    for (size_t i = 0; i < source.size(); i++) {
      const auto& c = correspondences[i];
      if (!c.found) continue;

      // (1) 適応的外れ値除去しきい値 ε_j。
      const double range = source[i].norm();
      double eps = params_.base_threshold + t_rel + 2.0 * range * sin_half;
      eps = std::min(eps, params_.max_threshold);

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src_transformed[i]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      double e;
      Eigen::Matrix<double, 1, 6> Jrow;
      if (c.has_normal && c.planarity >= params_.planarity_threshold) {
        e = c.normal.dot(src_transformed[i] - c.anchor);
        Jrow = c.normal.transpose() * J;
      } else {
        const Eigen::Vector3d d = src_transformed[i] - c.point;
        const double dn = d.norm();
        if (dn < 1e-9) continue;
        Jrow = (d / dn).transpose() * J;
        e = dn;
      }

      // point-to-point 距離で ε_j ゲート (誤対応棄却)。
      if ((src_transformed[i] - c.point).norm() > eps) continue;

      // Cauchy robust kernel の重み。
      const double w = 1.0 / (1.0 + e * e / cauchy_sq);
      JtJ += w * Jrow.transpose() * Jrow;
      Jtb += w * Jrow.transpose() * e;
      ++count;
    }

    if (count < 10) break;

    // ---- (3) 退化対応ハイブリッド重み ----
    // 回転 (0..2) と並進 (3..5) の Hessian ブロックを別々に固有値分解し、
    // 小固有値 (退化) 方向に IMU 予測へ引き戻す prior を加える。
    Eigen::Matrix<double, 6, 6> H = JtJ;
    Eigen::Matrix<double, 6, 1> rhs = -Jtb;

    // prior の引き戻し先: 現在 T から prediction への左増分 δ_p。
    const Eigen::Matrix3d R_cur = T.block<3, 3>(0, 0);
    const Eigen::Matrix3d R_pred = prediction.block<3, 3>(0, 0);
    const Eigen::Vector3d p_rot = logSO3(R_pred * R_cur.transpose());
    const Eigen::Vector3d p_trans =
        prediction.block<3, 1>(0, 3) - T.block<3, 1>(0, 3);

    int deg_rot = 0;
    int deg_trans = 0;
    auto regularizeBlock = [&](int offset, const Eigen::Vector3d& pull,
                               const Eigen::Matrix3d& info, int* deg_count) {
      const Eigen::Matrix3d block = H.block<3, 3>(offset, offset);
      Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(block);
      if (es.info() != Eigen::Success) return;
      const Eigen::Vector3d lam = es.eigenvalues();  // 昇順
      const Eigen::Matrix3d V = es.eigenvectors();
      const double lam_max = std::max(lam(2), 1e-12);
      // スケール不変な退化判定: λ_k < r·λ_max なら退化。
      const double kappa = params_.degeneracy_ratio * lam_max;
      for (int k = 0; k < 3; k++) {
        const double l = lam(k);
        if (l >= kappa) continue;
        ++(*deg_count);
        const Eigen::Vector3d v = V.col(k);
        // 退化度 g = (κ - λ)/κ ∈ (0,1]。IMU 情報を方向 v に射影して強度を決める。
        const double g = (kappa - l) / kappa;
        const double imu_dir_info = std::max(v.dot(info * v), 1e-3);
        // 正則化強度は λ_max スケールに比例 (条件数を回復する向き)。
        const double s = params_.imu_prior_weight * g * imu_dir_info * lam_max;
        // H += s v vᵀ,  rhs += s (vᵀpull) v   (prior cost ½ s (vᵀδ - vᵀpull)²)
        H.block<3, 3>(offset, offset) += s * v * v.transpose();
        rhs.segment<3>(offset) += s * (v.dot(pull)) * v;
      }
    };
    // 並進 prior 情報は等方近似 (IMU 並進共分散は重力/バイアス未補正のため)。
    const Eigen::Matrix3d trans_info = Eigen::Matrix3d::Identity();
    regularizeBlock(0, p_rot, imu_rot_info, &deg_rot);
    regularizeBlock(3, p_trans, trans_info, &deg_trans);
    last_deg_rot = deg_rot;
    last_deg_trans = deg_trans;

    const Eigen::Matrix<double, 6, 1> delta = H.ldlt().solve(rhs);
    if (!delta.allFinite()) break;

    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;

    if (delta.norm() < params_.convergence_criterion) break;
  }

  if (result) {
    result->degenerate_rot_dirs = last_deg_rot;
    result->degenerate_trans_dirs = last_deg_trans;
  }
  return T;
}

D2LIOResult D2LIOPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame,
    const std::vector<localization_zoo::imu_preintegration::ImuSample>& imu) {
  D2LIOResult result;

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

  // (2) IMU 回転先験。
  Eigen::Matrix3d imu_delta_R = last_delta_.block<3, 3>(0, 0);
  Eigen::Matrix3d imu_rot_info = Eigen::Matrix3d::Identity();
  double imu_dt = 0.0;
  if (imu.size() >= 2) {
    localization_zoo::imu_preintegration::ImuPreintegrator preint(
        localization_zoo::imu_preintegration::ImuNoiseParams(), gyro_bias_,
        Eigen::Vector3d::Zero());
    preint.integrateBatch(imu);
    const Eigen::Matrix3d dR = preint.result().delta_R;
    imu_dt = imu.back().timestamp - imu.front().timestamp;
    if (dR.allFinite()) {
      imu_delta_R = dR;
      result.used_imu = true;
      // δθ ブロックの共分散から情報行列 (= 逆共分散) を作る。
      const Eigen::Matrix3d cov_theta = preint.result().covariance.block<3, 3>(0, 0);
      Eigen::Matrix3d reg = cov_theta + 1e-6 * Eigen::Matrix3d::Identity();
      imu_rot_info = reg.inverse();
      // スケールを正規化 (退化方向に相対的な重みを与えるだけにする)。
      const double tr = imu_rot_info.trace();
      if (tr > 1e-9) imu_rot_info *= (3.0 / tr);
    }
  }

  Eigen::Matrix4d motion = Eigen::Matrix4d::Identity();
  motion.block<3, 3>(0, 0) = imu_delta_R;
  motion.block<3, 1>(0, 3) = last_delta_.block<3, 1>(0, 3);
  const Eigen::Matrix4d prediction = pose_ * motion;

  const double t_rel = last_delta_.block<3, 1>(0, 3).norm();
  const double theta_rel = logSO3(imu_delta_R).norm();

  const Eigen::Matrix4d new_pose =
      runICP(registration_points, prediction, prediction, t_rel, theta_rel,
             imu_rot_info, &result);

  last_delta_ = pose_.inverse() * new_pose;

  // gyro バイアスオンライン補正 (RKO-LIO と同方式)。
  if (result.used_imu && params_.gyro_bias_gain > 0.0 && imu_dt > 1e-6) {
    const Eigen::Matrix3d delta_R_icp = last_delta_.block<3, 3>(0, 0);
    const Eigen::Vector3d residual = logSO3(imu_delta_R.transpose() * delta_R_icp);
    gyro_bias_ -= params_.gyro_bias_gain * residual / imu_dt;
    const double bn = gyro_bias_.norm();
    if (bn > params_.max_gyro_bias) gyro_bias_ *= params_.max_gyro_bias / bn;
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
                      (new_pose - prediction).cwiseAbs().maxCoeff() < 1e3);
  result.iterations = params_.max_icp_iterations;
  frame_count_++;

  return result;
}

}  // namespace d2lio
}  // namespace localization_zoo
