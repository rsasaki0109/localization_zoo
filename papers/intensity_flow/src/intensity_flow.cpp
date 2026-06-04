#include "intensity_flow/intensity_flow.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace intensity_flow {

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
// 自由関数: RMS gradient-flow サンプリング
// ============================================================

std::vector<int> gradientFlowSample(const std::vector<PointI>& points,
                                    double radius, int num_bins,
                                    double keep_ratio) {
  const int N = static_cast<int>(points.size());
  std::vector<int> selected;
  if (N == 0) return selected;
  if (num_bins < 1) num_bins = 1;

  // インデックスグリッド (cell=radius)。
  std::unordered_map<Eigen::Vector3i, std::vector<int>, VoxelHash> grid;
  auto cellOf = [radius](const Eigen::Vector3d& p) {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / radius)),
                           static_cast<int>(std::floor(p.y() / radius)),
                           static_cast<int>(std::floor(p.z() / radius)));
  };
  for (int i = 0; i < N; i++) grid[cellOf(points[i].p)].push_back(i);

  // ∆p = mean(neighbor - p) の大きさ。
  const double r2 = radius * radius;
  std::vector<double> flow(N, 0.0);
  double max_flow = 0.0;
  for (int i = 0; i < N; i++) {
    const Eigen::Vector3d& p = points[i].p;
    const Eigen::Vector3i key = cellOf(p);
    Eigen::Vector3d acc = Eigen::Vector3d::Zero();
    int cnt = 0;
    for (int dx = -1; dx <= 1; dx++)
      for (int dy = -1; dy <= 1; dy++)
        for (int dz = -1; dz <= 1; dz++) {
          auto it = grid.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == grid.end()) continue;
          for (int j : it->second) {
            if (j == i) continue;
            const Eigen::Vector3d d = points[j].p - p;
            if (d.squaredNorm() <= r2) {
              acc += d;
              ++cnt;
            }
          }
        }
    if (cnt > 0) flow[i] = (acc / cnt).norm();
    max_flow = std::max(max_flow, flow[i]);
  }
  if (max_flow < 1e-12) max_flow = 1.0;

  // K ビンへ振り分け、各ビンを ∆p 降順にソート。
  std::vector<std::vector<int>> bins(num_bins);
  for (int i = 0; i < N; i++) {
    int bi = static_cast<int>((flow[i] / max_flow) * num_bins);
    if (bi >= num_bins) bi = num_bins - 1;
    bins[bi].push_back(i);
  }
  for (auto& bin : bins)
    std::sort(bin.begin(), bin.end(),
              [&flow](int a, int b) { return flow[a] > flow[b]; });

  // ビンを高 gradient → 低 へ巡回し、各ビンから 1 点ずつ取り出す
  // (冗長を抑えつつ分布を保つ)。target に達するまで。
  const int target =
      std::max(1, std::min(N, static_cast<int>(keep_ratio * N)));
  std::vector<size_t> cursor(num_bins, 0);
  selected.reserve(target);
  bool progressed = true;
  while (static_cast<int>(selected.size()) < target && progressed) {
    progressed = false;
    for (int b = num_bins - 1; b >= 0; b--) {
      if (cursor[b] < bins[b].size()) {
        selected.push_back(bins[b][cursor[b]++]);
        progressed = true;
        if (static_cast<int>(selected.size()) >= target) break;
      }
    }
  }
  return selected;
}

// ============================================================
// VoxelHashMap
// ============================================================

void VoxelHashMap::addPoints(const std::vector<PointI>& points) {
  for (const auto& pi : points) {
    auto key = toVoxel(pi.p);
    auto& vb = map_[key];
    if (static_cast<int>(vb.points.size()) < max_points_) {
      vb.points.push_back(pi.p);
      vb.intensity.push_back(pi.intensity);
    }
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
    std::vector<double> neighbor_int;
    neighbors.reserve(32);
    neighbor_int.reserve(32);

    for (int dx = -1; dx <= 1; dx++)
      for (int dy = -1; dy <= 1; dy++)
        for (int dz = -1; dz <= 1; dz++) {
          auto it = map_.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == map_.end()) continue;
          for (size_t k = 0; k < it->second.points.size(); k++) {
            const Eigen::Vector3d& mp = it->second.points[k];
            const double d = (mp - query).squaredNorm();
            if (d < max_dist_sq) {
              neighbors.push_back(mp);
              neighbor_int.push_back(it->second.intensity[k]);
            }
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
      double mean_i = 0.0;
      for (size_t k = 0; k < neighbors.size(); k++) {
        centroid += neighbors[k];
        mean_i += neighbor_int[k];
      }
      centroid /= static_cast<double>(neighbors.size());
      mean_i /= static_cast<double>(neighbors.size());
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
        c.mean_intensity = mean_i;
        c.has_normal = true;
      }
    }
  }
  return correspondences;
}

// ============================================================
// IntensityFlowPipeline
// ============================================================

IntensityFlowPipeline::IntensityFlowPipeline(const IntensityFlowParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<PointI> IntensityFlowPipeline::voxelDownsample(
    const std::vector<PointI>& points, double voxel_size) const {
  std::unordered_map<Eigen::Vector3i, PointI, VoxelHash> grid;
  for (const auto& pi : points) {
    Eigen::Vector3i key(static_cast<int>(std::floor(pi.p.x() / voxel_size)),
                        static_cast<int>(std::floor(pi.p.y() / voxel_size)),
                        static_cast<int>(std::floor(pi.p.z() / voxel_size)));
    grid.emplace(key, pi);
  }
  std::vector<PointI> out;
  out.reserve(grid.size());
  for (const auto& kv : grid) out.push_back(kv.second);
  return out;
}

std::vector<PointI> IntensityFlowPipeline::rangeFilter(
    const std::vector<PointI>& points) const {
  std::vector<PointI> out;
  out.reserve(points.size());
  for (const auto& pi : points) {
    const double r = pi.p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(pi);
  }
  return out;
}

Eigen::Matrix4d IntensityFlowPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  return pose_ * delta_window_.back();
}

Eigen::Matrix4d IntensityFlowPipeline::runRegistration(
    const std::vector<PointI>& source, const Eigen::Matrix4d& initial_guess,
    IntensityFlowResult* result) {
  Eigen::Matrix4d T = initial_guess;
  const double kernel = params_.initial_threshold;
  const double inv2si2 =
      1.0 / (2.0 * params_.intensity_sigma * params_.intensity_sigma);

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    std::vector<Eigen::Vector3d> src(source.size());
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);
    for (size_t i = 0; i < source.size(); i++) src[i] = R * source[i].p + t;
    const auto corr = local_map_.getCorrespondences(
        src, params_.initial_threshold, params_.normal_min_neighbors);

    Eigen::Matrix<double, 6, 6> A = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> b = Eigen::Matrix<double, 6, 1>::Zero();
    int used = 0;
    for (size_t i = 0; i < source.size(); i++) {
      const auto& c = corr[i];
      if (!c.found) continue;
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src[i]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      double e;
      Eigen::Matrix<double, 1, 6> Jr;
      double w_int = 1.0;
      if (c.has_normal && c.planarity >= params_.planarity_threshold) {
        e = c.normal.dot(src[i] - c.anchor);
        Jr = c.normal.transpose() * J;
        if (params_.enable_intensity) {
          const double di = source[i].intensity - c.mean_intensity;
          w_int = std::exp(-di * di * inv2si2);
        }
      } else {
        const Eigen::Vector3d d = src[i] - c.point;
        const double dn = d.norm();
        if (dn < 1e-9) continue;
        Jr = (d / dn).transpose() * J;
        e = dn;
      }
      if (std::abs(e) > kernel) continue;
      const double w = w_int * std::exp(-0.5 * (e / kernel) * (e / kernel));
      A += w * Jr.transpose() * Jr;
      b -= w * Jr.transpose() * e;
      ++used;
    }
    if (used < 10) break;
    const Eigen::Matrix<double, 6, 1> delta = A.ldlt().solve(b);
    if (!delta.allFinite()) break;
    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;
    if (result) result->iterations = iter + 1;
    if (delta.norm() < params_.convergence_criterion) break;
  }
  return T;
}

IntensityFlowResult IntensityFlowPipeline::registerFrame(
    const std::vector<PointI>& frame) {
  IntensityFlowResult result;

  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg = voxelDownsample(downsampled, params_.voxel_size);
  if (reg.empty()) reg = downsampled;
  result.num_input = static_cast<int>(reg.size());

  // --- RMS gradient-flow サンプリングで登録点を選ぶ ---
  std::vector<PointI> sampled = reg;
  if (params_.enable_gradient_flow && !reg.empty()) {
    const auto idx = gradientFlowSample(reg, params_.gf_radius,
                                        params_.gf_num_bins, params_.gf_keep_ratio);
    sampled.clear();
    sampled.reserve(idx.size());
    for (int i : idx) sampled.push_back(reg[i]);
  }
  result.num_sampled = static_cast<int>(sampled.size());

  auto toWorld = [](const std::vector<PointI>& pts, const Eigen::Matrix4d& T) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);
    std::vector<PointI> out(pts.size());
    for (size_t i = 0; i < pts.size(); i++) {
      out[i].p = R * pts[i].p + t;
      out[i].intensity = pts[i].intensity;
    }
    return out;
  };

  if (frame_count_ == 0) {
    local_map_.addPoints(toWorld(downsampled, pose_));
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  const Eigen::Matrix4d prediction = predict();
  const Eigen::Matrix4d new_pose = runRegistration(sampled, prediction, &result);

  const Eigen::Matrix4d delta = pose_.inverse() * new_pose;
  delta_window_.push_back(delta);
  if (static_cast<int>(delta_window_.size()) > 2) delta_window_.pop_front();
  pose_ = new_pose;

  local_map_.addPoints(toWorld(downsampled, pose_));
  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    local_map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.converged = (pose_.array().isFinite().all() &&
                      (new_pose - prediction).cwiseAbs().maxCoeff() < 1e3);
  frame_count_++;
  return result;
}

}  // namespace intensity_flow
}  // namespace localization_zoo
