#include "dali_slam/dali_slam.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace dali_slam {

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
// DaliSlamPipeline
// ============================================================

DaliSlamPipeline::DaliSlamPipeline(const DaliSlamParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> DaliSlamPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> DaliSlamPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d DaliSlamPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  const Eigen::Matrix<double, 6, 1> twist = scanMotionTwist();
  Eigen::Matrix4d motion = Eigen::Matrix4d::Identity();
  motion.block<3, 3>(0, 0) = expSO3(twist.head<3>());
  motion.block<3, 1>(0, 3) = twist.tail<3>();
  return pose_ * motion;
}

Eigen::Matrix<double, 6, 1> DaliSlamPipeline::scanMotionTwist() const {
  // dual-spline の核: lf_spline (LiDAR 姿勢列) から現フレームの相対運動を推定。
  // 2 次外挿は加速度を考慮する連続時間スプラインの代替 (lf_imu_spline 役は等速)。
  Eigen::Matrix<double, 6, 1> last = Eigen::Matrix<double, 6, 1>::Zero();
  if (delta_window_.empty()) return last;
  const Eigen::Matrix4d& d1 = delta_window_.back();
  last.head<3>() = logSO3(d1.block<3, 3>(0, 0));
  last.tail<3>() = d1.block<3, 1>(0, 3);
  if (!params_.spline_quadratic || delta_window_.size() < 2) return last;
  const Eigen::Matrix4d& d0 = delta_window_[delta_window_.size() - 2];
  Eigen::Matrix<double, 6, 1> prev;
  prev.head<3>() = logSO3(d0.block<3, 3>(0, 0));
  prev.tail<3>() = d0.block<3, 1>(0, 3);
  // 2 次外挿: next ≈ last + (last - prev)。
  return last + (last - prev);
}

std::vector<Eigen::Vector3d> DaliSlamPipeline::deskew(
    const std::vector<Eigen::Vector3d>& points,
    const Eigen::Matrix<double, 6, 1>& twist) const {
  // 各点を方位角から推定した scan 内時刻 s∈[0,1) で scan 末端フレームへ補正。
  // p_end = Exp((s-1)·twist) ⊙ p。
  std::vector<Eigen::Vector3d> out(points.size());
  const double two_pi = 2.0 * M_PI;
  for (size_t i = 0; i < points.size(); i++) {
    const Eigen::Vector3d& p = points[i];
    double s = (std::atan2(p.y(), p.x()) + M_PI) / two_pi;  // [0,1)
    const double f = s - 1.0;
    const Eigen::Vector3d w = f * twist.head<3>();
    const Eigen::Vector3d v = f * twist.tail<3>();
    out[i] = expSO3(w) * p + v;
  }
  return out;
}

Eigen::Matrix4d DaliSlamPipeline::runRegistration(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, DaliSlamResult* result) {
  Eigen::Matrix4d T = initial_guess;
  const double kernel = params_.initial_threshold;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const auto src = transformPoints(source, T);
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
      A += w * Jr.transpose() * Jr;
      b -= w * Jr.transpose() * e;
      ++used;
    }
    if (used < 10) break;

    // --- degeneracy-aware solution remapping ---
    // A を固有分解し、しきい値未満の方向は更新から除去 (退化方向は予測を維持)。
    Eigen::Matrix<double, 6, 1> delta;
    if (params_.enable_degeneracy) {
      Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 6, 6>> es(A);
      const Eigen::Matrix<double, 6, 1> ev = es.eigenvalues();      // 昇順
      const Eigen::Matrix<double, 6, 6> V = es.eigenvectors();
      delta.setZero();
      int ndeg = 0;
      for (int k = 0; k < 6; k++) {
        if (ev(k) < params_.degeneracy_threshold) {
          ++ndeg;
          continue;  // 退化方向: 更新しない
        }
        const double coeff = (V.col(k).dot(b)) / ev(k);
        delta += coeff * V.col(k);
      }
      if (result) {
        result->min_eigenvalue = ev(0);
        result->num_degenerate_dirs = ndeg;
        result->degenerate = (ndeg > 0);
      }
    } else {
      delta = A.ldlt().solve(b);
    }
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

DaliSlamResult DaliSlamPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  DaliSlamResult result;

  auto filtered = rangeFilter(frame);

  // --- DS-MDC: スプライン外挿 twist で deskew ---
  if (params_.enable_deskew && frame_count_ > 0 && !delta_window_.empty()) {
    const Eigen::Matrix<double, 6, 1> twist = scanMotionTwist();
    filtered = deskew(filtered, twist);
    result.deskew_applied = true;
  }

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

  const Eigen::Matrix4d prediction = predict();
  const Eigen::Matrix4d new_pose = runRegistration(reg, prediction, &result);

  const Eigen::Matrix4d delta = pose_.inverse() * new_pose;
  delta_window_.push_back(delta);
  if (static_cast<int>(delta_window_.size()) > 4) delta_window_.pop_front();
  pose_ = new_pose;

  local_map_.addPoints(transformPoints(downsampled, pose_));
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

}  // namespace dali_slam
}  // namespace localization_zoo
