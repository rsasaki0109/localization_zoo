#include "lidar_iba/lidar_iba.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace lidar_iba {

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
// ステレオ投影 (Eq.9, 20)
// ============================================================

Eigen::Vector3d stereoToNormal(const Eigen::Vector2d& u) {
  const double s = u.x() * u.x() + u.y() * u.y();
  const double denom = 1.0 + s;
  return Eigen::Vector3d(2.0 * u.x(), 2.0 * u.y(), s - 1.0) / denom;
}

Eigen::Vector2d normalToStereo(const Eigen::Vector3d& n_in) {
  Eigen::Vector3d n = n_in.normalized();
  // 北極 (n_z=1) は特異。法線の符号は平面表現で自由なので反転して回避する。
  if (n.z() > 0.999) n = -n;
  const double d = 1.0 - n.z();
  return Eigen::Vector2d(n.x() / d, n.y() / d);
}

Eigen::Matrix<double, 3, 2> stereoJacobian(const Eigen::Vector2d& u) {
  const double ux = u.x(), uy = u.y();
  const double denom = 1.0 + ux * ux + uy * uy;
  const double d2 = denom * denom;
  Eigen::Matrix<double, 3, 2> H;
  H(0, 0) = 2.0 * (1.0 + uy * uy - ux * ux) / d2;
  H(0, 1) = -4.0 * ux * uy / d2;
  H(1, 0) = -4.0 * ux * uy / d2;
  H(1, 1) = 2.0 * (1.0 + ux * ux - uy * uy) / d2;
  H(2, 0) = 4.0 * ux / d2;
  H(2, 1) = 4.0 * uy / d2;
  return H;
}

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
// LidarIbaPipeline
// ============================================================

LidarIbaPipeline::LidarIbaPipeline(const LidarIbaParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> LidarIbaPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> LidarIbaPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d LidarIbaPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  return pose_ * delta_window_.back();
}

Eigen::Matrix4d LidarIbaPipeline::runRegistration(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, LidarIbaResult* result) {
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

void LidarIbaPipeline::runWindowBA(LidarIbaResult* result) {
  const int nkf = static_cast<int>(keyframes_.size());
  if (nkf < 2) return;

  // --- 平面ランドマーク抽出: 全キーフレーム点を world 系で plane-voxel に集約 ---
  struct VoxAcc {
    Eigen::Vector3d sum = Eigen::Vector3d::Zero();
    Eigen::Matrix3d sumsq = Eigen::Matrix3d::Zero();
    int n = 0;
    std::vector<std::pair<int, Eigen::Vector3d>> obs;  // (kf_index, body_point)
  };
  std::unordered_map<Eigen::Vector3i, VoxAcc, VoxelHash> grid;
  const double pv = params_.plane_voxel;
  for (int k = 0; k < nkf; k++) {
    const Eigen::Matrix3d R = keyframes_[k].pose.block<3, 3>(0, 0);
    const Eigen::Vector3d t = keyframes_[k].pose.block<3, 1>(0, 3);
    for (const auto& p_body : keyframes_[k].points) {
      const Eigen::Vector3d w = R * p_body + t;
      Eigen::Vector3i key(static_cast<int>(std::floor(w.x() / pv)),
                          static_cast<int>(std::floor(w.y() / pv)),
                          static_cast<int>(std::floor(w.z() / pv)));
      auto& va = grid[key];
      va.sum += w;
      va.sumsq += w * w.transpose();
      va.n += 1;
      if (static_cast<int>(va.obs.size()) < params_.max_obs_per_landmark)
        va.obs.emplace_back(k, p_body);
    }
  }

  // --- ランドマーク化 (planarity 判定 + stereo 初期化) ---
  struct Landmark {
    Eigen::Vector2d u;
    double oa;
    std::vector<std::pair<int, Eigen::Vector3d>> obs;
  };
  std::vector<Landmark> landmarks;
  landmarks.reserve(grid.size());
  for (auto& kv : grid) {
    VoxAcc& va = kv.second;
    if (va.n < params_.plane_min_points) continue;
    const Eigen::Vector3d centroid = va.sum / va.n;
    Eigen::Matrix3d cov = va.sumsq / va.n - centroid * centroid.transpose();
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
    if (es.info() != Eigen::Success) continue;
    const Eigen::Vector3d ev = es.eigenvalues();
    const double planarity =
        std::clamp((ev(1) - ev(0)) / std::max(ev(2), 1e-12), 0.0, 1.0);
    if (planarity < params_.plane_min_planarity) continue;
    Eigen::Vector3d n = es.eigenvectors().col(0).normalized();
    Landmark lm;
    lm.u = normalToStereo(n);
    // OA = -n·centroid (平面が centroid を通る)。n の符号は normalToStereo で
    // 反転している場合があるので、確定後の n(u) で計算する。
    const Eigen::Vector3d n_fixed = stereoToNormal(lm.u);
    lm.oa = -n_fixed.dot(centroid);
    lm.obs = std::move(va.obs);
    landmarks.push_back(std::move(lm));
  }
  if (landmarks.empty()) return;

  // 観測数の多い順に上位 max_landmarks を採用。
  if (static_cast<int>(landmarks.size()) > params_.max_landmarks) {
    std::nth_element(landmarks.begin(),
                     landmarks.begin() + params_.max_landmarks, landmarks.end(),
                     [](const Landmark& a, const Landmark& b) {
                       return a.obs.size() > b.obs.size();
                     });
    landmarks.resize(params_.max_landmarks);
  }

  // --- 変数レイアウト: free pose (kf 1..nkf-1) 6DoF + 各ランドマーク 3DoF ---
  // FEJ: 最古キーフレーム (index 0) は線形化点を凍結し gauge 固定 (free にしない)。
  const int nfree = nkf - 1;
  const int npose = nfree * 6;
  const int nland = static_cast<int>(landmarks.size());
  const int dim = npose + nland * 3;
  const double weight = 1.0 / std::max(params_.lidar_sigma, 1e-6);
  const double radius_limit = 1.0 + params_.stereo_radius_margin;

  auto poseFreeIndex = [](int k) { return (k - 1) * 6; };  // k>=1

  auto computeCost = [&]() {
    double cost = 0.0;
    int nobs = 0;
    for (const auto& lm : landmarks) {
      const Eigen::Vector3d n = stereoToNormal(lm.u);
      for (const auto& ob : lm.obs) {
        const int k = ob.first;
        const Eigen::Matrix3d R = keyframes_[k].pose.block<3, 3>(0, 0);
        const Eigen::Vector3d t = keyframes_[k].pose.block<3, 1>(0, 3);
        const Eigen::Vector3d w = R * ob.second + t;
        const double r = n.dot(w) + lm.oa;
        cost += weight * r * r;
        ++nobs;
      }
    }
    if (result) result->ba_observations = nobs;
    return cost;
  };

  if (result) {
    result->ba_landmarks = nland;
    result->ba_cost_before = computeCost();
  }

  for (int it = 0; it < params_.ba_iterations; it++) {
    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(dim, dim);
    Eigen::VectorXd b = Eigen::VectorXd::Zero(dim);
    for (int li = 0; li < nland; li++) {
      Landmark& lm = landmarks[li];
      const Eigen::Vector3d n = stereoToNormal(lm.u);
      const Eigen::Matrix<double, 3, 2> Hu = stereoJacobian(lm.u);
      const int loff = npose + li * 3;
      for (const auto& ob : lm.obs) {
        const int k = ob.first;
        const Eigen::Matrix3d R = keyframes_[k].pose.block<3, 3>(0, 0);
        const Eigen::Vector3d t = keyframes_[k].pose.block<3, 1>(0, 3);
        const Eigen::Vector3d w = R * ob.second + t;
        const double r = n.dot(w) + lm.oa;

        // 平面 Jacobian: dr/du = wᵀ H(u) (1x2), dr/dOA = 1。
        Eigen::Matrix<double, 1, 3> Jl;
        Jl.head<2>() = w.transpose() * Hu;
        Jl(0, 2) = 1.0;

        // pose Jacobian (k が free のときのみ): dr/dδ = nᵀ[-skew(w), I] (1x6)。
        // k==0 (最古) は FEJ で固定 → pose ブロックには寄与しない。
        if (k >= 1) {
          Eigen::Matrix<double, 1, 6> Jp;
          Jp.head<3>() = -n.transpose() * skew(w);
          Jp.tail<3>() = n.transpose();
          const int poff = poseFreeIndex(k);
          A.block<6, 6>(poff, poff) += weight * Jp.transpose() * Jp;
          b.segment<6>(poff) -= weight * Jp.transpose() * r;
          // pose-landmark cross term。
          A.block<6, 3>(poff, loff) += weight * Jp.transpose() * Jl;
          A.block<3, 6>(loff, poff) += weight * Jl.transpose() * Jp;
        }
        A.block<3, 3>(loff, loff) += weight * Jl.transpose() * Jl;
        b.segment<3>(loff) -= weight * Jl.transpose() * r;
      }
    }

    // --- front-end (odometry) prior: 各 free pose を front-end 推定へ柔らかく
    //     繋ぐ (IMU/marginalization prior r_I の純 LiDAR 代替)。自己相似シーンで
    //     pose/plane を同時に動かせる gauge 崩壊を防ぐ。 ---
    if (params_.pose_prior_weight > 0.0) {
      const double wp = params_.pose_prior_weight;
      for (int k = 1; k < nkf; k++) {
        const Eigen::Matrix4d& Tk = keyframes_[k].pose;
        const Eigen::Matrix4d& Tfe = keyframes_[k].fe_pose;
        const Eigen::Matrix3d Rk = Tk.block<3, 3>(0, 0);
        const Eigen::Vector3d tk = Tk.block<3, 1>(0, 3);
        // 残差: 回転 = Log(Rfe^T Rk)、並進 = tk - tfe (world)。
        Eigen::Matrix<double, 6, 1> rp;
        rp.head<3>() = logSO3(Tfe.block<3, 3>(0, 0).transpose() * Rk);
        rp.tail<3>() = tk - Tfe.block<3, 1>(0, 3);
        // Jacobian (left perturbation δ=[δθ,δt]): 回転 ≈ I (世界系微小回転)、
        // 並進 d(tk)/dδ = [-skew(tk), I]。
        Eigen::Matrix<double, 6, 6> Jp = Eigen::Matrix<double, 6, 6>::Zero();
        Jp.block<3, 3>(0, 0) = Eigen::Matrix3d::Identity();
        Jp.block<3, 3>(3, 0) = -skew(tk);
        Jp.block<3, 3>(3, 3) = Eigen::Matrix3d::Identity();
        const int poff = poseFreeIndex(k);
        A.block<6, 6>(poff, poff) += wp * Jp.transpose() * Jp;
        b.segment<6>(poff) -= wp * Jp.transpose() * rp;
      }
    }

    // Levenberg ダンピングで数値安定化。
    for (int d = 0; d < dim; d++) A(d, d) += 1e-6 * A(d, d) + 1e-9;
    const Eigen::VectorXd delta = A.ldlt().solve(b);
    if (!delta.allFinite()) break;

    // free pose 更新 (left perturbation)。
    for (int k = 1; k < nkf; k++) {
      const Eigen::Matrix<double, 6, 1> dx = delta.segment<6>(poseFreeIndex(k));
      Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
      dT.block<3, 3>(0, 0) = expSO3(dx.head<3>());
      dT.block<3, 1>(0, 3) = dx.tail<3>();
      keyframes_[k].pose = dT * keyframes_[k].pose;
    }
    // ランドマーク更新 (u は半径制限で数値安定化)。
    for (int li = 0; li < nland; li++) {
      const int loff = npose + li * 3;
      landmarks[li].u += delta.segment<2>(loff);
      const double un = landmarks[li].u.norm();
      if (un > radius_limit) landmarks[li].u *= radius_limit / un;
      landmarks[li].oa += delta(loff + 2);
    }
    if (delta.norm() < 1e-6) break;
  }

  if (result) {
    result->ba_ran = true;
    result->ba_cost_after = computeCost();
  }
}

LidarIbaResult LidarIbaPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  LidarIbaResult result;

  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg = voxelDownsample(downsampled, params_.voxel_size);
  if (reg.empty()) reg = downsampled;

  if (frame_count_ == 0) {
    local_map_.addPoints(transformPoints(downsampled, pose_));
    keyframes_.push_back({pose_, pose_, reg});
    frame_count_++;
    frames_since_kf_ = 0;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  const Eigen::Matrix4d prediction = predict();
  const Eigen::Matrix4d new_pose = runRegistration(reg, prediction, &result);

  const Eigen::Matrix4d delta = pose_.inverse() * new_pose;
  delta_window_.push_back(delta);
  if (static_cast<int>(delta_window_.size()) > 2) delta_window_.pop_front();
  pose_ = new_pose;

  local_map_.addPoints(transformPoints(downsampled, pose_));

  // --- キーフレーム選択 + sliding-window BA ---
  frames_since_kf_++;
  if (params_.enable_ba &&
      frames_since_kf_ >= std::max(1, params_.keyframe_interval)) {
    keyframes_.push_back({pose_, pose_, reg});
    frames_since_kf_ = 0;
    while (static_cast<int>(keyframes_.size()) > std::max(2, params_.window_size))
      keyframes_.pop_front();  // 最古を marginalize (FEJ で線形化点固定)

    runWindowBA(&result);
    // BA で精緻化した最新キーフレーム pose を読み出し位置とする。
    const Eigen::Matrix4d refined = keyframes_.back().pose;
    if (refined.array().isFinite().all()) pose_ = refined;
  }

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

}  // namespace lidar_iba
}  // namespace localization_zoo
