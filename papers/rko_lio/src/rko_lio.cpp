#include "rko_lio/rko_lio.h"

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>

namespace localization_zoo {
namespace rko_lio {

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

// SO3 の対数写像 (回転行列 -> 軸角ベクトル)。
Eigen::Vector3d logSO3(const Eigen::Matrix3d& R) {
  const double cos_theta = std::clamp((R.trace() - 1.0) / 2.0, -1.0, 1.0);
  const double theta = std::acos(cos_theta);
  if (theta < 1e-9) return Eigen::Vector3d::Zero();
  const Eigen::Vector3d axis(R(2, 1) - R(1, 2), R(0, 2) - R(2, 0),
                             R(1, 0) - R(0, 1));
  return axis * (theta / (2.0 * std::sin(theta)));
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
    const std::vector<Eigen::Vector3d>& points, double max_dist) const {
  std::vector<Correspondence> correspondences(points.size());
  const double max_dist_sq = max_dist * max_dist;

  for (size_t i = 0; i < points.size(); i++) {
    const auto& query = points[i];
    auto key = toVoxel(query);
    double best_dist = max_dist_sq;
    Eigen::Vector3d best_point = Eigen::Vector3d::Zero();
    bool found = false;

    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
          Eigen::Vector3i neighbor = key + Eigen::Vector3i(dx, dy, dz);
          auto it = map_.find(neighbor);
          if (it == map_.end()) continue;
          for (const auto& mp : it->second.points) {
            const double d = (mp - query).squaredNorm();
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
// RKO-LIO Pipeline
// ============================================================

RKOLIOPipeline::RKOLIOPipeline(const RKOLIOParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> RKOLIOPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> RKOLIOPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d RKOLIOPipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, double max_correspondence_dist) {
  Eigen::Matrix4d T = initial_guess;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const auto src_transformed = transformPoints(source, T);
    const auto correspondences =
        local_map_.getCorrespondences(src_transformed, max_correspondence_dist);

    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    const double kernel = max_correspondence_dist;
    int valid = 0;

    for (size_t i = 0; i < source.size(); i++) {
      if (!correspondences[i].found) continue;
      const Eigen::Vector3d residual = src_transformed[i] - correspondences[i].point;
      const double r_norm = residual.norm();
      if (r_norm > max_correspondence_dist) continue;
      ++valid;

      const double w = std::exp(-0.5 * (r_norm / kernel) * (r_norm / kernel));
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src_transformed[i]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      JtJ += w * J.transpose() * J;
      Jtb += w * J.transpose() * residual;
    }

    if (valid < 10) break;

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

double RKOLIOPipeline::computeAdaptiveThreshold() {
  if (model_errors_.empty()) return params_.initial_threshold;
  auto sorted = model_errors_;
  std::sort(sorted.begin(), sorted.end());
  const double median = sorted[sorted.size() / 2];
  return std::max(3.0 * median, params_.initial_threshold * 0.1);
}

RKOLIOResult RKOLIOPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame,
    const std::vector<localization_zoo::imu_preintegration::ImuSample>& imu) {
  RKOLIOResult result;

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

  // IMU 回転先験。推定 gyro バイアスを差し引いて事前積分し、直前フレームからの
  // 相対回転を得る。バイアス推定により生 gyro のドリフトが先験を歪めるのを防ぐ。
  Eigen::Matrix3d imu_delta_R = last_delta_.block<3, 3>(0, 0);  // 既定は等速
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

  // 予測: 回転は IMU 先験、並進は等速モデル (body 系の前回並進)。
  Eigen::Matrix4d motion = Eigen::Matrix4d::Identity();
  motion.block<3, 3>(0, 0) = imu_delta_R;
  motion.block<3, 1>(0, 3) = last_delta_.block<3, 1>(0, 3);
  const Eigen::Matrix4d prediction = pose_ * motion;

  const double adaptive_threshold = computeAdaptiveThreshold();
  const Eigen::Matrix4d new_pose =
      runICP(registration_points, prediction, adaptive_threshold);

  last_delta_ = pose_.inverse() * new_pose;
  const double model_error = last_delta_.block<3, 1>(0, 3).norm();
  model_errors_.push_back(model_error);
  if (model_errors_.size() > 100) model_errors_.erase(model_errors_.begin());

  // gyro バイアスのオンライン補正。IMU 先験回転と ICP が解いた実回転の差分は
  // バイアス起因の積分誤差。差分を角速度に直してバイアスを少しずつ更新する。
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

}  // namespace rko_lio
}  // namespace localization_zoo
