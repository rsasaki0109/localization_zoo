#include "vibration_lio/vibration_lio.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace vibration_lio {

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

VoxelHashMap::PlaneFeature VoxelHashMap::query(const Eigen::Vector3d& query_pt,
                                               double max_dist,
                                               int normal_min_neighbors) const {
  PlaneFeature f;
  const auto key = toVoxel(query_pt);
  const double max_dist_sq = max_dist * max_dist;
  double best = max_dist_sq;
  Eigen::Vector3d best_point = Eigen::Vector3d::Zero();
  std::vector<Eigen::Vector3d> neighbors;
  neighbors.reserve(32);

  for (int dx = -1; dx <= 1; dx++)
    for (int dy = -1; dy <= 1; dy++)
      for (int dz = -1; dz <= 1; dz++) {
        auto it = map_.find(key + Eigen::Vector3i(dx, dy, dz));
        if (it == map_.end()) continue;
        for (const auto& mp : it->second.points) {
          const double d = (mp - query_pt).squaredNorm();
          if (d < max_dist_sq) neighbors.push_back(mp);
          if (d < best) {
            best = d;
            best_point = mp;
            f.found = true;
          }
        }
      }
  if (!f.found) return f;

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
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
    if (es.info() == Eigen::Success) {
      const Eigen::Vector3d ev = es.eigenvalues();
      f.mean = centroid;
      f.normal = es.eigenvectors().col(0).normalized();
      f.plane_variance = std::max(ev(0), 1e-6);
      f.has_normal = true;
    }
  } else {
    f.mean = best_point;
  }
  return f;
}

// ============================================================
// VibrationLIO Pipeline
// ============================================================

VibrationLIOPipeline::VibrationLIOPipeline(const VibrationLIOParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> VibrationLIOPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> VibrationLIOPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix3d VibrationLIOPipeline::measurementCovariance(
    const Eigen::Vector3d& p) const {
  const double d = p.norm();
  if (d < 1e-6)
    return params_.sigma_depth * params_.sigma_depth * Eigen::Matrix3d::Identity();
  const Eigen::Vector3d dir = p / d;
  const double var_depth = params_.sigma_depth * params_.sigma_depth;
  const double var_tan = (d * params_.sigma_bearing) * (d * params_.sigma_bearing);
  return var_tan * Eigen::Matrix3d::Identity() +
         (var_depth - var_tan) * dir * dir.transpose();
}

Eigen::Matrix3d VibrationLIOPipeline::pointCovariance(const Eigen::Vector3d& p,
                                                      double sigma_r,
                                                      double sigma_T) const {
  // 角振動: Σ_rot = ⌊p⌋× (σ_r² I) ⌊p⌋×ᵀ
  const Eigen::Matrix3d S = skew(p);
  const Eigen::Matrix3d cov_rot = (sigma_r * sigma_r) * (S * S.transpose());
  // 線振動: Σ_trans = σ_T² I (位置非依存)
  const Eigen::Matrix3d cov_trans =
      (sigma_T * sigma_T) * Eigen::Matrix3d::Identity();
  // 測定 (range/bearing)
  const Eigen::Matrix3d cov_meas = measurementCovariance(p);
  return cov_rot + cov_trans + cov_meas;
}

Eigen::Matrix4d VibrationLIOPipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, double sigma_r, double sigma_T) {
  Eigen::Matrix4d T = initial_guess;
  // センサ系点共分散を前計算。
  std::vector<Eigen::Matrix3d> src_cov(source.size());
  for (size_t i = 0; i < source.size(); i++)
    src_cov[i] = pointCovariance(source[i], sigma_r, sigma_T);

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);

    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    int count = 0;

    for (size_t i = 0; i < source.size(); i++) {
      const Eigen::Vector3d p_world = R * source[i] + t;
      const auto f = local_map_.query(p_world, params_.max_correspondence_dist,
                                      params_.normal_min_neighbors);
      if (!f.found || !f.has_normal) continue;
      const double r = f.normal.dot(p_world - f.mean);
      if (std::abs(r) > params_.max_correspondence_dist) continue;

      // 不確かさを姿勢で伝播し、平面法線方向に射影した分散で重み付け
      // (Mahalanobis 重み): R_j = nᵀ R Σ_p Rᵀ n + σ_plane²。
      const Eigen::Matrix3d Rp_cov = R * src_cov[i] * R.transpose();
      const double var = f.normal.dot(Rp_cov * f.normal) + f.plane_variance;
      const double w = 1.0 / std::max(var, 1e-9);

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(p_world);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const Eigen::Matrix<double, 1, 6> Jr = f.normal.transpose() * J;
      JtJ += w * Jr.transpose() * Jr;
      Jtb += w * Jr.transpose() * r;
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

VibrationLIOResult VibrationLIOPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame,
    const std::vector<localization_zoo::imu_preintegration::ImuSample>& imu) {
  VibrationLIOResult result;

  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg = voxelDownsample(downsampled, params_.voxel_size);
  if (reg.empty()) reg = downsampled;

  auto toWorld = [&](const std::vector<Eigen::Vector3d>& pts) {
    Eigen::Matrix3d R = pose_.block<3, 3>(0, 0);
    Eigen::Vector3d t = pose_.block<3, 1>(0, 3);
    std::vector<Eigen::Vector3d> w(pts.size());
    for (size_t i = 0; i < pts.size(); i++) w[i] = R * pts[i] + t;
    return w;
  };

  if (frame_count_ == 0) {
    local_map_.addPoints(toWorld(downsampled));
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  // IMU から振動強度 (角速度/加速度の MAD) を推定し、振動由来 σ を決める。
  Eigen::Matrix3d imu_delta_R = last_delta_.block<3, 3>(0, 0);
  double sigma_r = params_.gamma * params_.scan_period * params_.min_vibration;
  double sigma_T = sigma_r;
  double imu_dt = 0.0;
  if (imu.size() >= 2) {
    // 角速度・加速度の平均と MAD。
    Eigen::Vector3d w_ave = Eigen::Vector3d::Zero();
    Eigen::Vector3d a_ave = Eigen::Vector3d::Zero();
    for (const auto& s : imu) {
      w_ave += s.gyro;
      a_ave += s.accel;
    }
    w_ave /= static_cast<double>(imu.size());
    a_ave /= static_cast<double>(imu.size());
    double k_w = 0.0, k_v = 0.0;
    for (const auto& s : imu) {
      k_w += (s.gyro - w_ave).norm();
      k_v += (s.accel - a_ave).norm();
    }
    k_w /= static_cast<double>(imu.size());
    k_v /= static_cast<double>(imu.size());
    result.vibration_omega = k_w;
    result.vibration_accel = k_v;
    sigma_r = params_.gamma * params_.scan_period * std::max(k_w, params_.min_vibration);
    sigma_T = params_.gamma * params_.scan_period * std::max(k_v, params_.min_vibration);

    // IMU 回転先験。
    localization_zoo::imu_preintegration::ImuPreintegrator preint(
        localization_zoo::imu_preintegration::ImuNoiseParams(), gyro_bias_,
        Eigen::Vector3d::Zero());
    preint.integrateBatch(imu);
    const Eigen::Matrix3d dR = preint.result().delta_R;
    imu_dt = imu.back().timestamp - imu.front().timestamp;
    if (dR.allFinite()) {
      imu_delta_R = dR;
      result.used_imu = true;
    }
  }

  Eigen::Matrix4d motion = Eigen::Matrix4d::Identity();
  motion.block<3, 3>(0, 0) = imu_delta_R;
  motion.block<3, 1>(0, 3) = last_delta_.block<3, 1>(0, 3);
  const Eigen::Matrix4d prediction = pose_ * motion;

  const Eigen::Matrix4d new_pose = runICP(reg, prediction, sigma_r, sigma_T);
  last_delta_ = pose_.inverse() * new_pose;

  if (result.used_imu && params_.gyro_bias_gain > 0.0 && imu_dt > 1e-6) {
    const Eigen::Vector3d residual =
        logSO3(imu_delta_R.transpose() * last_delta_.block<3, 3>(0, 0));
    gyro_bias_ -= params_.gyro_bias_gain * residual / imu_dt;
    const double bn = gyro_bias_.norm();
    if (bn > params_.max_gyro_bias) gyro_bias_ *= params_.max_gyro_bias / bn;
  }

  pose_ = new_pose;
  local_map_.addPoints(toWorld(downsampled));
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

}  // namespace vibration_lio
}  // namespace localization_zoo
