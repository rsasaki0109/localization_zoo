#include "ct_voxelmap/ct_voxelmap.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace ct_voxelmap {

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

}  // namespace

// ============================================================
// ProbabilisticVoxelMap
// ============================================================

void ProbabilisticVoxelMap::refreshFeature(Voxel& v) const {
  VoxelFeature& f = v.feature;
  f.count = v.count;
  f.valid = false;
  f.is_planar = false;
  if (v.count < min_points_) return;

  const double inv = 1.0 / static_cast<double>(v.count);
  f.mean = v.sum * inv;
  // Σ = E[xxᵀ] - μμᵀ
  f.cov = v.sum_outer * inv - f.mean * f.mean.transpose();
  // 数値安定化のため対称化 + 微小正則化。
  f.cov = 0.5 * (f.cov + f.cov.transpose());
  f.cov += 1e-9 * Eigen::Matrix3d::Identity();
  f.valid = true;

  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(f.cov);
  if (es.info() != Eigen::Success) return;
  const Eigen::Vector3d ev = es.eigenvalues();  // 昇順
  const double lambda2 = std::max(ev(2), 1e-12);
  if (ev(0) < planarity_ratio_ * lambda2) {
    f.is_planar = true;
    f.normal = es.eigenvectors().col(0).normalized();
    f.plane_variance = std::max(ev(0), 1e-6);
  }
}

void ProbabilisticVoxelMap::addPoints(
    const std::vector<Eigen::Vector3d>& points) {
  std::vector<Eigen::Vector3i> touched;
  touched.reserve(points.size());
  for (const auto& p : points) {
    auto key = toVoxel(p);
    auto& v = voxels_[key];
    v.count += 1;
    v.sum += p;
    v.sum_outer += p * p.transpose();
    touched.push_back(key);
  }
  // 触れたボクセルの特徴のみ更新する。
  std::sort(touched.begin(), touched.end(),
            [](const Eigen::Vector3i& a, const Eigen::Vector3i& b) {
              if (a.x() != b.x()) return a.x() < b.x();
              if (a.y() != b.y()) return a.y() < b.y();
              return a.z() < b.z();
            });
  touched.erase(std::unique(touched.begin(), touched.end()), touched.end());
  for (const auto& key : touched) refreshFeature(voxels_[key]);
}

ProbabilisticVoxelMap::Query ProbabilisticVoxelMap::query(
    const Eigen::Vector3d& world_point) const {
  Query q;
  const auto key = toVoxel(world_point);
  // 自ボクセル優先、無ければ 27 近傍で最も有効かつ近い特徴を返す。
  auto it = voxels_.find(key);
  if (it != voxels_.end() && it->second.feature.valid) {
    q.feature = it->second.feature;
    q.found = true;
    return q;
  }

  double best = std::numeric_limits<double>::max();
  for (int dx = -1; dx <= 1; dx++)
    for (int dy = -1; dy <= 1; dy++)
      for (int dz = -1; dz <= 1; dz++) {
        auto jt = voxels_.find(key + Eigen::Vector3i(dx, dy, dz));
        if (jt == voxels_.end() || !jt->second.feature.valid) continue;
        const double d = (jt->second.feature.mean - world_point).squaredNorm();
        if (d < best) {
          best = d;
          q.feature = jt->second.feature;
          q.found = true;
        }
      }
  return q;
}

void ProbabilisticVoxelMap::pruneFarVoxels(const Eigen::Vector3d& center,
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
// CTVoxelMap Pipeline
// ============================================================

CTVoxelMapPipeline::CTVoxelMapPipeline(const CTVoxelMapParams& params)
    : params_(params),
      map_(params.voxel_size, params.voxel_min_points, params.planarity_ratio) {}

std::vector<Eigen::Vector3d> CTVoxelMapPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> CTVoxelMapPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix3d CTVoxelMapPipeline::pointCovariance(
    const Eigen::Vector3d& p_sensor) const {
  const double d = p_sensor.norm();
  if (d < 1e-6) return params_.sigma_depth * params_.sigma_depth *
                      Eigen::Matrix3d::Identity();
  const Eigen::Vector3d dir = p_sensor / d;  // bearing 単位ベクトル
  // 距離方向: σ_depth²、横方向: (d·σ_bearing)²。
  const double var_depth = params_.sigma_depth * params_.sigma_depth;
  const double var_tan = (d * params_.sigma_bearing) * (d * params_.sigma_bearing);
  Eigen::Matrix3d cov = var_tan * Eigen::Matrix3d::Identity() +
                        (var_depth - var_tan) * dir * dir.transpose();
  return cov;
}

Eigen::Matrix4d CTVoxelMapPipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, double* planar_ratio_out) {
  Eigen::Matrix4d T = initial_guess;
  // 点共分散はセンサ系で前計算 (姿勢で回すだけにする)。
  std::vector<Eigen::Matrix3d> src_cov(source.size());
  for (size_t i = 0; i < source.size(); i++)
    src_cov[i] = pointCovariance(source[i]);

  double last_planar_ratio = 0.0;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);

    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    int count = 0;
    int planar = 0;

    for (size_t i = 0; i < source.size(); i++) {
      const Eigen::Vector3d p_world = R * source[i] + T.block<3, 1>(0, 3);
      const auto q = map_.query(p_world);
      if (!q.found) continue;

      const Eigen::Matrix3d Rp_cov = R * src_cov[i] * R.transpose();

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(p_world);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      if (q.feature.is_planar) {
        // Mahalanobis point-to-plane: r = nᵀ(p - μ),
        // var = nᵀ R Σ_p Rᵀ n + σ_plane²。
        const Eigen::Vector3d& n = q.feature.normal;
        const double r = n.dot(p_world - q.feature.mean);
        if (std::abs(r) > params_.max_correspondence_dist) continue;
        const double var = n.dot(Rp_cov * n) + q.feature.plane_variance;
        const double w = 1.0 / std::max(var, 1e-9);
        const Eigen::Matrix<double, 1, 6> Jr = n.transpose() * J;
        JtJ += w * Jr.transpose() * Jr;
        Jtb += w * Jr.transpose() * r;
        ++planar;
        ++count;
      } else {
        // point-to-distribution (GICP 風): r = p - μ,
        // 情報行列 = (R Σ_p Rᵀ + Σ_voxel)⁻¹。
        const Eigen::Vector3d r = p_world - q.feature.mean;
        if (r.norm() > params_.max_correspondence_dist) continue;
        Eigen::Matrix3d S = Rp_cov + q.feature.cov;
        S += 1e-6 * Eigen::Matrix3d::Identity();
        const Eigen::Matrix3d info = S.inverse();
        JtJ += J.transpose() * info * J;
        Jtb += J.transpose() * info * r;
        ++count;
      }
    }

    if (count < 10) break;
    last_planar_ratio = static_cast<double>(planar) / count;

    const Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) break;

    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;

    if (delta.norm() < params_.convergence_criterion) break;
  }

  if (planar_ratio_out) *planar_ratio_out = last_planar_ratio;
  return T;
}

CTVoxelMapResult CTVoxelMapPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  CTVoxelMapResult result;

  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto registration_points = voxelDownsample(downsampled, params_.voxel_size);
  if (registration_points.empty()) registration_points = downsampled;

  if (frame_count_ == 0) {
    Eigen::Matrix3d R = pose_.block<3, 3>(0, 0);
    Eigen::Vector3d t = pose_.block<3, 1>(0, 3);
    std::vector<Eigen::Vector3d> world(downsampled.size());
    for (size_t i = 0; i < downsampled.size(); i++)
      world[i] = R * downsampled[i] + t;
    map_.addPoints(world);
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  const Eigen::Matrix4d prediction = pose_ * last_delta_;
  double planar_ratio = 0.0;
  const Eigen::Matrix4d new_pose =
      runICP(registration_points, prediction, &planar_ratio);

  last_delta_ = pose_.inverse() * new_pose;
  pose_ = new_pose;

  Eigen::Matrix3d R = pose_.block<3, 3>(0, 0);
  Eigen::Vector3d t = pose_.block<3, 1>(0, 3);
  std::vector<Eigen::Vector3d> world(downsampled.size());
  for (size_t i = 0; i < downsampled.size(); i++)
    world[i] = R * downsampled[i] + t;
  map_.addPoints(world);
  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.converged = (pose_.array().isFinite().all() &&
                      (new_pose - prediction).cwiseAbs().maxCoeff() < 1e3);
  result.iterations = params_.max_icp_iterations;
  result.planar_ratio = planar_ratio;
  frame_count_++;
  return result;
}

}  // namespace ct_voxelmap
}  // namespace localization_zoo
