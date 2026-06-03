#include "ua_lio/ua_lio.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace ua_lio {

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
// GaussianVoxelMap
// ============================================================

void GaussianVoxelMap::refresh(Voxel& v) const {
  v.feat.found = false;
  v.feat.has_normal = false;
  if (v.count < min_points_) return;
  const double inv = 1.0 / static_cast<double>(v.count);
  v.feat.mean = v.sum * inv;
  Eigen::Matrix3d cov = v.sum_outer * inv - v.feat.mean * v.feat.mean.transpose();
  cov = 0.5 * (cov + cov.transpose()) + 1e-9 * Eigen::Matrix3d::Identity();
  v.feat.cov = cov;
  v.feat.found = true;
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
  if (es.info() == Eigen::Success) {
    v.feat.normal = es.eigenvectors().col(0).normalized();
    v.feat.plane_variance = std::max(es.eigenvalues()(0), 1e-6);
    v.feat.has_normal = true;
  }
}

void GaussianVoxelMap::addPoints(const std::vector<Eigen::Vector3d>& points) {
  std::vector<Eigen::Vector3i> touched;
  for (const auto& p : points) {
    auto key = toVoxel(p);
    auto& v = voxels_[key];
    v.count += 1;
    v.sum += p;
    v.sum_outer += p * p.transpose();
    touched.push_back(key);
  }
  std::sort(touched.begin(), touched.end(),
            [](const Eigen::Vector3i& a, const Eigen::Vector3i& b) {
              if (a.x() != b.x()) return a.x() < b.x();
              if (a.y() != b.y()) return a.y() < b.y();
              return a.z() < b.z();
            });
  touched.erase(std::unique(touched.begin(), touched.end()), touched.end());
  for (const auto& key : touched) refresh(voxels_[key]);
}

GaussianVoxelMap::Feature GaussianVoxelMap::query(
    const Eigen::Vector3d& world_point) const {
  const auto key = toVoxel(world_point);
  auto it = voxels_.find(key);
  if (it != voxels_.end() && it->second.feat.found) return it->second.feat;
  double best = std::numeric_limits<double>::max();
  Feature result;
  for (int dx = -1; dx <= 1; dx++)
    for (int dy = -1; dy <= 1; dy++)
      for (int dz = -1; dz <= 1; dz++) {
        auto jt = voxels_.find(key + Eigen::Vector3i(dx, dy, dz));
        if (jt == voxels_.end() || !jt->second.feat.found) continue;
        const double d = (jt->second.feat.mean - world_point).squaredNorm();
        if (d < best) { best = d; result = jt->second.feat; }
      }
  return result;
}

void GaussianVoxelMap::pruneFarVoxels(const Eigen::Vector3d& center,
                                      double max_distance) {
  const double max_distance_sq = max_distance * max_distance;
  for (auto it = voxels_.begin(); it != voxels_.end();) {
    const Eigen::Vector3d c =
        (it->first.cast<double>() + Eigen::Vector3d::Constant(0.5)) * voxel_size_;
    if ((c - center).squaredNorm() > max_distance_sq) {
      it = voxels_.erase(it);
    } else {
      ++it;
    }
  }
}

// ============================================================
// UALIO Pipeline
// ============================================================

UALIOPipeline::UALIOPipeline(const UALIOParams& params)
    : params_(params), map_(params.voxel_size, params.voxel_min_points) {}

std::vector<Eigen::Vector3d> UALIOPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> UALIOPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix3d UALIOPipeline::pointCovariance(
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

Eigen::Matrix4d UALIOPipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, int* ground_cells) {
  Eigen::Matrix4d T = initial_guess;
  std::vector<Eigen::Matrix3d> src_cov(source.size());
  for (size_t i = 0; i < source.size(); i++)
    src_cov[i] = pointCovariance(source[i]);

  int last_ground = 0;
  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);
    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    int count = 0, ground = 0;

    for (size_t i = 0; i < source.size(); i++) {
      const Eigen::Vector3d p_world = R * source[i] + t;
      const auto f = map_.query(p_world);
      if (!f.found || !f.has_normal) continue;

      const Eigen::Matrix3d Rp_cov = R * src_cov[i] * R.transpose();

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(p_world);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      // distribution-to-distribution の point-to-plane:
      // 残差 r = nᵀ(p - μ), 分散 = nᵀ(R Σ_p Rᵀ + Σ_voxel) n。
      const double r = f.normal.dot(p_world - f.mean);
      if (std::abs(r) > params_.max_correspondence_dist) continue;
      const Eigen::Matrix3d S = Rp_cov + f.cov;
      const double var = std::max(f.normal.dot(S * f.normal), 1e-9);
      const double w = 1.0 / var;
      const Eigen::Matrix<double, 1, 6> Jr = f.normal.transpose() * J;
      JtJ += w * Jr.transpose() * Jr;
      Jtb += w * Jr.transpose() * r;
      ++count;

      // 地面拘束: 法線がほぼ z の平面に対し z 方向残差を追加で重く拘束。
      if (params_.ground_constraint &&
          std::abs(f.normal.z()) >= params_.ground_normal_z_min) {
        const Eigen::Vector3d nz(0, 0, 1);
        const double rz = nz.dot(p_world - f.mean);
        if (std::abs(rz) <= params_.max_correspondence_dist) {
          const double wz = params_.ground_weight * w;
          const Eigen::Matrix<double, 1, 6> Jz = nz.transpose() * J;
          JtJ += wz * Jz.transpose() * Jz;
          Jtb += wz * Jz.transpose() * rz;
          ++ground;
        }
      }
    }
    if (count < 10) break;
    last_ground = ground;
    const Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) break;
    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;
    if (delta.norm() < params_.convergence_criterion) break;
  }
  if (ground_cells) *ground_cells = last_ground;
  return T;
}

UALIOResult UALIOPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame,
    const std::vector<localization_zoo::imu_preintegration::ImuSample>& imu) {
  UALIOResult result;
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
    map_.addPoints(toWorld(downsampled));
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

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

  int ground_cells = 0;
  const Eigen::Matrix4d new_pose = runICP(reg, prediction, &ground_cells);
  last_delta_ = pose_.inverse() * new_pose;

  if (result.used_imu && params_.gyro_bias_gain > 0.0 && imu_dt > 1e-6) {
    const Eigen::Vector3d residual =
        logSO3(imu_delta_R.transpose() * last_delta_.block<3, 3>(0, 0));
    gyro_bias_ -= params_.gyro_bias_gain * residual / imu_dt;
    const double bn = gyro_bias_.norm();
    if (bn > params_.max_gyro_bias) gyro_bias_ *= params_.max_gyro_bias / bn;
  }

  pose_ = new_pose;
  map_.addPoints(toWorld(downsampled));
  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.converged = (pose_.array().isFinite().all() &&
                      (new_pose - prediction).cwiseAbs().maxCoeff() < 1e3);
  result.iterations = params_.max_icp_iterations;
  result.ground_cells = ground_cells;
  frame_count_++;
  return result;
}

}  // namespace ua_lio
}  // namespace localization_zoo
