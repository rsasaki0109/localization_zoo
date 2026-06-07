#include "kc_lo/kc_lo.h"

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>

namespace localization_zoo {
namespace kc_lo {

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

std::vector<Eigen::Vector3d> transformPoints(
    const std::vector<Eigen::Vector3d>& pts, const Eigen::Matrix4d& T) {
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
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

std::vector<VoxelHashMap::KcTarget> VoxelHashMap::kernelTargets(
    const std::vector<Eigen::Vector3d>& points, double sigma,
    double radius) const {
  std::vector<KcTarget> targets(points.size());
  const double r_sq = radius * radius;
  const double inv_2s2 = 1.0 / (2.0 * std::max(sigma * sigma, 1e-12));
  const int span =
      std::max(1, static_cast<int>(std::ceil(radius / voxel_size_)));

  for (size_t i = 0; i < points.size(); i++) {
    const auto& query = points[i];
    const auto key = toVoxel(query);
    double w_sum = 0.0;
    Eigen::Vector3d wy = Eigen::Vector3d::Zero();

    for (int dx = -span; dx <= span; dx++)
      for (int dy = -span; dy <= span; dy++)
        for (int dz = -span; dz <= span; dz++) {
          auto it = map_.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == map_.end()) continue;
          for (const auto& mp : it->second.points) {
            const double d2 = (mp - query).squaredNorm();
            if (d2 > r_sq) continue;
            const double w = std::exp(-d2 * inv_2s2);
            w_sum += w;
            wy += w * mp;
          }
        }

    KcTarget& t = targets[i];
    if (w_sum > 1e-12) {
      t.mean = wy / w_sum;
      t.weight = w_sum;
      t.found = true;
    }
  }
  return targets;
}

// ============================================================
// KcLoPipeline — 機構ヘルパ
// ============================================================

double KcLoPipeline::kernelWeight(double dist_sq, double sigma) {
  const double s2 = std::max(sigma * sigma, 1e-12);
  return std::exp(-dist_sq / (2.0 * s2));
}

Eigen::Vector3d KcLoPipeline::weightedMean(
    const std::vector<Eigen::Vector3d>& pts, const Eigen::Vector3d& query,
    double sigma, double* weight_out) {
  double w_sum = 0.0;
  Eigen::Vector3d wy = Eigen::Vector3d::Zero();
  for (const auto& p : pts) {
    const double w = kernelWeight((p - query).squaredNorm(), sigma);
    w_sum += w;
    wy += w * p;
  }
  if (weight_out) *weight_out = w_sum;
  if (w_sum <= 1e-12) return query;
  return wy / w_sum;
}

// ============================================================
// KcLoPipeline — パイプライン
// ============================================================

KcLoPipeline::KcLoPipeline(const KcLoParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> KcLoPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> KcLoPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d KcLoPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  return pose_ * delta_window_.back();
}

Eigen::Matrix4d KcLoPipeline::runRegistration(
    const std::vector<Eigen::Vector3d>& source, const Eigen::Matrix4d& base,
    KcLoResult* result) {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;

  Eigen::Matrix4d T = base;
  int last_corr = 0;
  double mean_weight = 0.0;
  double sigma = std::max(params_.kc_sigma_init, params_.kc_sigma);

  for (int it = 0; it < params_.max_iterations; it++) {
    const double radius = params_.kc_radius_factor * sigma;
    const auto src = transformPoints(source, T);
    const auto targets = local_map_.kernelTargets(src, sigma, radius);

    // soft point-to-point: 各点を親和度重み平均 ȳ へ引き寄せる (総親和度で重み付け)。
    Mat6 A = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    double w_sum = 0.0;
    int used = 0;
    for (size_t k = 0; k < source.size(); k++) {
      const auto& tg = targets[k];
      if (!tg.found) continue;
      const Eigen::Vector3d r = src[k] - tg.mean;  // 3D 残差
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src[k]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const double w = tg.weight;
      A += w * J.transpose() * J;
      b -= w * J.transpose() * r;
      w_sum += w;
      used++;
    }
    if (used < 10) break;
    mean_weight = w_sum / static_cast<double>(used);

    const Vec6 d = A.ldlt().solve(b);
    if (!d.allFinite()) break;
    T = expSE3(d) * T;
    last_corr = used;
    if (result) result->iterations = it + 1;

    const bool at_min = sigma <= params_.kc_sigma * 1.0001;
    if (d.norm() < params_.convergence_criterion && at_min && it > 2) break;
    // coarse-to-fine: σ を縮める。
    sigma = std::max(sigma * params_.kc_anneal, params_.kc_sigma);
  }

  if (result) {
    result->num_correspondences = last_corr;
    result->sigma_used = sigma;
    result->mean_weight = mean_weight;
  }
  return T;
}

KcLoResult KcLoPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  KcLoResult result;

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
  result.converged = (pose_.array().isFinite().all() &&
                      (new_pose - base).cwiseAbs().maxCoeff() < 1e3);
  frame_count_++;
  return result;
}

}  // namespace kc_lo
}  // namespace localization_zoo
