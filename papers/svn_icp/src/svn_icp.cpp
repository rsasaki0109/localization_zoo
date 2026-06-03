#include "svn_icp/svn_icp.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>

namespace localization_zoo {
namespace svn_icp {

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

// Exp([w,v]) = [[expSO3(w), v],[0,1]] (小運動近似、本リポジトリの dT 構築と整合)。
Eigen::Matrix4d expSE3(const Eigen::Matrix<double, 6, 1>& xi) {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = expSO3(xi.head<3>());
  T.block<3, 1>(0, 3) = xi.tail<3>();
  return T;
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
// SvnIcpPipeline
// ============================================================

SvnIcpPipeline::SvnIcpPipeline(const SvnIcpParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> SvnIcpPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> SvnIcpPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d SvnIcpPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  return pose_ * delta_window_.back();
}

Eigen::Matrix4d SvnIcpPipeline::runSvn(const std::vector<Eigen::Vector3d>& source,
                                       const Eigen::Matrix4d& base,
                                       SvnIcpResult* result) {
  const int M = std::max(2, params_.num_particles);
  const double inv_sigma2 =
      1.0 / (params_.lidar_sigma * params_.lidar_sigma);
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;

  // --- 粒子初期化 (予測まわりに散布、粒子0は予測そのもの) ---
  std::mt19937 rng(static_cast<unsigned>(params_.rng_seed + frame_count_));
  std::normal_distribution<double> nr(0.0, params_.init_spread_rot);
  std::normal_distribution<double> nt(0.0, params_.init_spread_trans);
  std::vector<Vec6> theta(M, Vec6::Zero());
  for (int i = 1; i < M; i++) {
    theta[i] << nr(rng), nr(rng), nr(rng), nt(rng), nt(rng), nt(rng);
  }

  int last_corr = 0;
  for (int iter = 0; iter < params_.svn_iterations; iter++) {
    // 平均姿勢で対応を 1 度だけ求め、全粒子で再利用 (関連付けは近傍粒子間で不変)。
    Vec6 mean = Vec6::Zero();
    for (int i = 0; i < M; i++) mean += theta[i];
    mean /= M;
    const Eigen::Matrix4d T_mean = expSE3(mean) * base;
    const auto src_mean = transformPoints(source, T_mean);
    const auto corr = local_map_.getCorrespondences(
        src_mean, params_.initial_threshold, params_.normal_min_neighbors);

    struct Plane {
      Eigen::Vector3d p;  // body 点
      Eigen::Vector3d n;
      Eigen::Vector3d q;  // 平面上の点 (anchor)
    };
    std::vector<Plane> planes;
    planes.reserve(source.size());
    for (size_t k = 0; k < source.size(); k++) {
      const auto& c = corr[k];
      if (c.found && c.has_normal && c.planarity >= params_.planarity_threshold)
        planes.push_back({source[k], c.normal, c.anchor});
    }
    last_corr = static_cast<int>(planes.size());
    if (last_corr < 10) break;

    // --- 各粒子の勾配 g_i = ∇log p, Hessian H_i = -∇²log p (GN) ---
    std::vector<Vec6> g(M, Vec6::Zero());
    std::vector<Mat6> H(M, Mat6::Zero());
    for (int i = 0; i < M; i++) {
      const Eigen::Matrix4d T_i = expSE3(theta[i]) * base;
      const Eigen::Matrix3d R_i = T_i.block<3, 3>(0, 0);
      const Eigen::Vector3d t_i = T_i.block<3, 1>(0, 3);
      Vec6 gi = Vec6::Zero();
      Mat6 Hi = Mat6::Zero();
      for (const auto& pl : planes) {
        const Eigen::Vector3d p_w = R_i * pl.p + t_i;
        const double r = pl.n.dot(p_w - pl.q);
        Eigen::Matrix<double, 3, 6> J;
        J.block<3, 3>(0, 0) = -skew(p_w);
        J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
        const Eigen::Matrix<double, 1, 6> Jr = pl.n.transpose() * J;
        gi += -inv_sigma2 * Jr.transpose() * r;       // ∇log p
        Hi += inv_sigma2 * Jr.transpose() * Jr;       // GN Hessian
      }
      // 予測 (θ=0) への弱い prior。
      gi += -params_.prior_precision * theta[i];
      Hi += params_.prior_precision * Mat6::Identity();
      g[i] = gi;
      H[i] = Hi;
    }

    // --- RBF 核バンド幅 (median heuristic) ---
    std::vector<double> sqd;
    sqd.reserve(M * (M - 1) / 2);
    for (int i = 0; i < M; i++)
      for (int j = i + 1; j < M; j++)
        sqd.push_back((theta[i] - theta[j]).squaredNorm());
    double med = 0.0;
    if (!sqd.empty()) {
      std::nth_element(sqd.begin(), sqd.begin() + sqd.size() / 2, sqd.end());
      med = sqd[sqd.size() / 2];
    }
    double h = med / std::log(static_cast<double>(M) + 1.0);
    h = std::max(h, params_.min_bandwidth);

    // --- SVN 更新: θ_i ← θ_i + α H_svn(θ_i)⁻¹ φ(θ_i) ---
    std::vector<Vec6> theta_new(M);
    for (int i = 0; i < M; i++) {
      Vec6 phi = Vec6::Zero();
      Mat6 Hsvn = Mat6::Zero();
      for (int j = 0; j < M; j++) {
        const double d2 = (theta[j] - theta[i]).squaredNorm();
        const double k = std::exp(-d2 / h);
        // ∇_{θ_j} k(θ_j,θ_i) = -2/h (θ_j-θ_i) k。
        const Vec6 gradk = (-2.0 / h) * (theta[j] - theta[i]) * k;
        phi += k * g[j] + gradk;
        Hsvn += k * k * H[j] + gradk * gradk.transpose();
      }
      phi /= M;
      Hsvn /= M;
      Hsvn += 1e-6 * Mat6::Identity();
      const Vec6 step = params_.step_size * Hsvn.ldlt().solve(phi);
      theta_new[i] = theta[i] + (step.allFinite() ? step : Vec6::Zero());
    }
    theta = theta_new;
    if (result) result->iterations = iter + 1;
  }

  // --- 粒子平均姿勢と共分散 (不確かさ推定) ---
  Vec6 mean = Vec6::Zero();
  for (int i = 0; i < M; i++) mean += theta[i];
  mean /= M;
  Eigen::Matrix<double, 6, 6> C = Eigen::Matrix<double, 6, 6>::Zero();
  for (int i = 0; i < M; i++) {
    const Vec6 d = theta[i] - mean;
    C += d * d.transpose();
  }
  C /= std::max(1, M - 1);

  if (result) {
    result->covariance_trace = C.trace();
    result->rot_std = std::sqrt(std::max(0.0, (C(0, 0) + C(1, 1) + C(2, 2)) / 3.0));
    result->trans_std =
        std::sqrt(std::max(0.0, (C(3, 3) + C(4, 4) + C(5, 5)) / 3.0));
    result->num_correspondences = last_corr;
  }
  return expSE3(mean) * base;
}

SvnIcpResult SvnIcpPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  SvnIcpResult result;

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
  const Eigen::Matrix4d new_pose = runSvn(reg, base, &result);

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

}  // namespace svn_icp
}  // namespace localization_zoo
