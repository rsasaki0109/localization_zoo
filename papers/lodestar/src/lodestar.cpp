#include "lodestar/lodestar.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace lodestar {

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
// Lodestar Pipeline
// ============================================================

LodestarPipeline::LodestarPipeline(const LodestarParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> LodestarPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> LodestarPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d LodestarPipeline::predict() const {
  // 滑動窓の active 過去状態 (相対運動) を平均して等速予測。
  // 純 LiDAR scope では IMU 予測の代替であり、退化方向の anchor となる。
  if (delta_window_.empty()) return pose_;
  const int n = std::min(static_cast<int>(delta_window_.size()),
                         std::max(1, params_.active_window));
  Eigen::Vector3d mean_t = Eigen::Vector3d::Zero();
  Eigen::Vector3d mean_w = Eigen::Vector3d::Zero();
  for (int k = 0; k < n; k++) {
    const Eigen::Matrix4d& d = delta_window_[delta_window_.size() - 1 - k];
    mean_t += d.block<3, 1>(0, 3);
    mean_w += logSO3(d.block<3, 3>(0, 0));
  }
  mean_t /= static_cast<double>(n);
  mean_w /= static_cast<double>(n);
  Eigen::Matrix4d motion = Eigen::Matrix4d::Identity();
  motion.block<3, 3>(0, 0) = expSO3(mean_w);
  motion.block<3, 1>(0, 3) = mean_t;
  return pose_ * motion;
}

Eigen::Matrix4d LodestarPipeline::runRegistration(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, const Eigen::Matrix4d& prediction,
    LodestarResult* result) {
  Eigen::Matrix4d T = initial_guess;
  const double kernel = params_.initial_threshold;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const auto src = transformPoints(source, T);
    const auto corr = local_map_.getCorrespondences(
        src, params_.initial_threshold, params_.normal_min_neighbors);

    // --- pass 1: 全測定で残差・Jacobian・法線を蓄積 ---
    struct Meas {
      Eigen::Matrix<double, 1, 6> Jr;
      double e;
      double w;
      Eigen::Vector3d normal;  // 並進寄与方向 (平面=法線, p2p=方向)
    };
    std::vector<Meas> meas;
    meas.reserve(source.size());
    for (size_t i = 0; i < source.size(); i++) {
      const auto& c = corr[i];
      if (!c.found) continue;
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src[i]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      double e;
      Eigen::Matrix<double, 1, 6> Jr;
      Eigen::Vector3d dir;
      if (c.has_normal && c.planarity >= params_.planarity_threshold) {
        e = c.normal.dot(src[i] - c.anchor);
        Jr = c.normal.transpose() * J;
        dir = c.normal;
      } else {
        const Eigen::Vector3d d = src[i] - c.point;
        const double dn = d.norm();
        if (dn < 1e-9) continue;
        dir = d / dn;
        Jr = dir.transpose() * J;
        e = dn;
      }
      if (std::abs(e) > kernel) continue;
      const double w = std::exp(-0.5 * (e / kernel) * (e / kernel));
      meas.push_back({Jr, e, w, dir});
    }
    if (static_cast<int>(meas.size()) < 10) break;

    // 退化検出は回転/並進ブロックを分離して行う (両ブロックは固有値スケールが
    // 大きく異なるため、6x6 一括の条件数は常に巨大になり使えない)。
    // 並進: H_tt = Σ w n nᵀ、回転: H_rr = Σ w (skew(p)ᵀ n)(...)ᵀ。
    Eigen::Matrix3d Htt = Eigen::Matrix3d::Zero();
    Eigen::Matrix3d Hrr = Eigen::Matrix3d::Zero();
    for (const auto& m : meas) {
      const Eigen::Vector3d jr_rot = m.Jr.template head<3>().transpose();
      Htt += m.w * m.normal * m.normal.transpose();
      Hrr += m.w * jr_rot * jr_rot.transpose();
    }
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> est(Htt);
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> esr(Hrr);
    const Eigen::Vector3d evt = est.eigenvalues();   // 昇順
    const Eigen::Vector3d evr = esr.eigenvalues();
    const Eigen::Matrix3d Vt = est.eigenvectors();   // 列が昇順固有ベクトル
    const double lmax_t = std::max(evt(2), 1e-12);
    const double lmax_r = std::max(evr(2), 1e-12);
    const double chi_t = lmax_t / std::max(evt(0), 1e-12);
    const double chi_r = lmax_r / std::max(evr(0), 1e-12);
    const double floor_t = lmax_t / params_.t_chi;
    const bool deg_t = chi_t >= params_.t_chi;
    // 退化判定は並進ブロックのみで行う。LiDAR オドメトリで実際に観測不能に
    // なるのは並進方向 (廊下・高高度) であり、回転 Hessian は点の距離² で
    // スケールするため条件数が常に大きく、退化指標として使えない。
    const bool degenerate = deg_t;

    // --- DA-DE: 退化時のみ曖昧測定を剪定 ---
    std::vector<char> keep(meas.size(), 1);
    int pruned = 0;
    if (deg_t && params_.enable_data_exploitation) {
      for (size_t i = 0; i < meas.size(); i++) {
        // 局所化寄与 = いずれかの並進主方向との最大整列度 |n·v_ℓ|。
        double best = 0.0;
        for (int l = 0; l < 3; l++)
          best = std::max(best, std::abs(meas[i].normal.dot(Vt.col(l))));
        if (best < params_.t_loc) {
          keep[i] = 0;
          ++pruned;
        }
      }
      // 剪定しすぎたら破棄 (全点使用)。
      const int kept = static_cast<int>(meas.size()) - pruned;
      if (kept < static_cast<int>(params_.min_keep_ratio * meas.size())) {
        std::fill(keep.begin(), keep.end(), 1);
        pruned = 0;
      }
    }

    // --- pass 2: 活用測定で系を再構築 ---
    Eigen::Matrix<double, 6, 6> A = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> b = Eigen::Matrix<double, 6, 1>::Zero();
    int active = 0;
    for (size_t i = 0; i < meas.size(); i++) {
      if (!keep[i]) continue;
      A += meas[i].w * meas[i].Jr.transpose() * meas[i].Jr;
      b -= meas[i].w * meas[i].Jr.transpose() * meas[i].e;
      ++active;
    }

    // --- DA-ASKF: 退化方向に fixed 状態 (等速予測) anchor を Schmidt-Kalman 注入 ---
    // current 推定 T から予測 prediction への左摂動 δ_pred (anchor target)。
    const Eigen::Matrix4d M = prediction * T.inverse();
    Eigen::Matrix<double, 6, 1> dpred;
    dpred.head<3>() = logSO3(M.block<3, 3>(0, 0));
    dpred.tail<3>() = M.block<3, 1>(0, 3);
    int num_deg = 0;
    // 並進退化方向: v6 = [0; v_t]。
    if (deg_t) {
      for (int k = 0; k < 3; k++) {
        if (evt(k) >= floor_t) continue;
        Eigen::Matrix<double, 6, 1> v = Eigen::Matrix<double, 6, 1>::Zero();
        v.tail<3>() = Vt.col(k);
        const double rho = params_.anchor_strength * (floor_t - evt(k));
        A += rho * v * v.transpose();
        b += rho * v * (v.dot(dpred));
        ++num_deg;
      }
    }

    const Eigen::Matrix<double, 6, 1> delta = A.ldlt().solve(b);
    if (!delta.allFinite()) break;
    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;

    if (result) {
      result->condition_number = std::max(chi_t, chi_r);
      result->condition_number_trans = chi_t;
      result->degenerate = degenerate;
      result->num_degenerate_dirs = num_deg;
      result->num_pruned = pruned;
      result->num_active = active;
      result->iterations = iter + 1;
    }
    if (delta.norm() < params_.convergence_criterion) break;
  }
  return T;
}

LodestarResult LodestarPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  LodestarResult result;

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

  const Eigen::Matrix4d prediction = predict();
  const Eigen::Matrix4d new_pose =
      runRegistration(reg, prediction, prediction, &result);

  const Eigen::Matrix4d delta = pose_.inverse() * new_pose;
  delta_window_.push_back(delta);
  if (static_cast<int>(delta_window_.size()) > std::max(1, params_.active_window))
    delta_window_.pop_front();

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

}  // namespace lodestar
}  // namespace localization_zoo
