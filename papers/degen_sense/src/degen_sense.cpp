#include "degen_sense/degen_sense.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace degen_sense {

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

double conditionNumber(const Eigen::Matrix3d& block) {
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(block);
  if (es.info() != Eigen::Success) return 1.0;
  const Eigen::Vector3d ev = es.eigenvalues();  // 昇順
  const double lmin = std::max(ev(0), 1e-9);
  const double lmax = std::max(ev(2), 1e-9);
  return lmax / lmin;
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
// DegenSense Pipeline
// ============================================================

DegenSensePipeline::DegenSensePipeline(const DegenSenseParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> DegenSensePipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> DegenSensePipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d DegenSensePipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, Eigen::Matrix<double, 6, 6>* H_out) {
  Eigen::Matrix4d T = initial_guess;
  const double kernel = params_.initial_threshold;
  Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const auto src = transformPoints(source, T);
    const auto corr = local_map_.getCorrespondences(src, params_.initial_threshold,
                                                    params_.normal_min_neighbors);
    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    int count = 0;
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
      JtJ += w * Jr.transpose() * Jr;
      Jtb += w * Jr.transpose() * e;
      ++count;
    }
    if (count < 10) break;
    H = JtJ;
    const Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) break;
    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;
    if (delta.norm() < params_.convergence_criterion) break;
  }
  if (H_out) *H_out = H;
  return T;
}

bool DegenSensePipeline::isDegenerate(double factor,
                                      const std::deque<double>& history) const {
  if (static_cast<int>(history.size()) < params_.warmup_frames) return false;
  std::vector<double> sorted(history.begin(), history.end());
  std::sort(sorted.begin(), sorted.end());
  const double median = sorted[sorted.size() / 2];
  std::vector<double> dev(sorted.size());
  for (size_t i = 0; i < sorted.size(); i++)
    dev[i] = std::abs(sorted[i] - median);
  std::sort(dev.begin(), dev.end());
  const double mad = dev[dev.size() / 2];
  // 退化スパイクは正常クラスタから外れた "ノイズ点" (DBSCAN 概念の近似)。
  return factor > median + params_.mad_k * (mad + 1e-6);
}

DegenSenseResult DegenSensePipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame,
    const std::vector<localization_zoo::imu_preintegration::ImuSample>& imu) {
  DegenSenseResult result;

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

  // IMU 回転先験。
  Eigen::Matrix3d imu_delta_R = last_delta_.block<3, 3>(0, 0);
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
    }
  }

  Eigen::Matrix4d motion = Eigen::Matrix4d::Identity();
  motion.block<3, 3>(0, 0) = imu_delta_R;
  motion.block<3, 1>(0, 3) = last_delta_.block<3, 1>(0, 3);
  const Eigen::Matrix4d prediction = pose_ * motion;

  // LiDAR registration + 収束 Hessian。
  Eigen::Matrix<double, 6, 6> H;
  const Eigen::Matrix4d lidar_pose = runICP(reg, prediction, &H);

  // (1) 退化ファクタ = Hessian サブブロックの条件数。
  const double S_R = conditionNumber(H.block<3, 3>(0, 0));
  const double S_t = conditionNumber(H.block<3, 3>(3, 3));
  result.degeneracy_factor_rot = S_R;
  result.degeneracy_factor_trans = S_t;

  // (2) 適応的外れ値検出で退化判定。
  const bool deg_rot = isDegenerate(S_R, rot_history_);
  const bool deg_trans = isDegenerate(S_t, trans_history_);
  result.degenerate = deg_rot || deg_trans;

  // (3) 退化時は、実 IMU がある場合だけ IMU 予測と LiDAR 解を 1/S で融合して補償。
  // no-IMU では退化診断だけを残し、等速予測を inertial reference として扱わない。
  Eigen::Matrix4d fused = lidar_pose;
  if (result.used_imu && deg_trans) {
    const double w = std::clamp(1.0 / S_t, params_.min_fusion_weight, 1.0);
    fused.block<3, 1>(0, 3) =
        (1.0 - w) * prediction.block<3, 1>(0, 3) +
        w * lidar_pose.block<3, 1>(0, 3);
  }
  if (result.used_imu && deg_rot) {
    const double w = std::clamp(1.0 / S_R, params_.min_fusion_weight, 1.0);
    const Eigen::Quaterniond q_imu(prediction.block<3, 3>(0, 0));
    const Eigen::Quaterniond q_lidar(lidar_pose.block<3, 3>(0, 0));
    const Eigen::Quaterniond q = q_imu.slerp(w, q_lidar).normalized();
    fused.block<3, 3>(0, 0) = q.toRotationMatrix();
  }

  const Eigen::Matrix4d new_pose = fused;
  last_delta_ = pose_.inverse() * new_pose;

  // gyro バイアスオンライン補正。
  if (result.used_imu && params_.gyro_bias_gain > 0.0 && imu_dt > 1e-6) {
    const Eigen::Vector3d residual =
        logSO3(imu_delta_R.transpose() * last_delta_.block<3, 3>(0, 0));
    gyro_bias_ -= params_.gyro_bias_gain * residual / imu_dt;
    const double bn = gyro_bias_.norm();
    if (bn > params_.max_gyro_bias) gyro_bias_ *= params_.max_gyro_bias / bn;
  }

  pose_ = new_pose;

  // 履歴更新。
  rot_history_.push_back(S_R);
  trans_history_.push_back(S_t);
  if (static_cast<int>(rot_history_.size()) > params_.buffer_size)
    rot_history_.pop_front();
  if (static_cast<int>(trans_history_.size()) > params_.buffer_size)
    trans_history_.pop_front();

  local_map_.addPoints(transformPoints(downsampled, pose_));
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

}  // namespace degen_sense
}  // namespace localization_zoo
