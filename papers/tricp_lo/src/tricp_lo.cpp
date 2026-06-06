#include "tricp_lo/tricp_lo.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>

namespace localization_zoo {
namespace tricp_lo {

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

std::vector<VoxelHashMap::Correspondence> VoxelHashMap::getCorrespondences(
    const std::vector<Eigen::Vector3d>& points, double max_dist,
    int normal_min_neighbors) const {
  std::vector<Correspondence> correspondences(points.size());
  const double max_dist_sq = max_dist * max_dist;

  for (size_t i = 0; i < points.size(); i++) {
    const auto& query = points[i];
    auto key = toVoxel(query);
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
            if (d < max_dist_sq) {
              neighbors.push_back(mp);
              found = true;
            }
          }
        }

    Correspondence& c = correspondences[i];
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
// TrICP-LO — 機構ヘルパ (Least Trimmed Squares / FRMSD)
// ============================================================

double TricpLoPipeline::trimmedMeanSqResidual(
    const std::vector<double>& sorted_sq, double xi) {
  const int N = static_cast<int>(sorted_sq.size());
  if (N == 0) return 0.0;
  int k = static_cast<int>(std::floor(std::clamp(xi, 0.0, 1.0) * N));
  k = std::max(1, std::min(k, N));
  double sum = 0.0;
  for (int i = 0; i < k; i++) sum += sorted_sq[i];
  return sum / static_cast<double>(k);
}

double TricpLoPipeline::frmsd(const std::vector<double>& sorted_sq, double xi,
                              double lambda) {
  const double e = trimmedMeanSqResidual(sorted_sq, xi);
  const double x = std::max(xi, 1e-6);
  // FRMSD(ξ) = sqrt(e(ξ)) / ξ^λ。ξ を縮めれば e は下がるが ξ^λ ペナルティが増す。
  return std::sqrt(e) / std::pow(x, lambda);
}

double TricpLoPipeline::estimateOverlap(std::vector<double> sq_residuals,
                                        double min_overlap, double lambda) {
  std::sort(sq_residuals.begin(), sq_residuals.end());
  const int N = static_cast<int>(sq_residuals.size());
  if (N == 0) return 1.0;
  // 昇順二乗残差の累積和で e(ξ)=cum[k]/k を O(N) 走査。
  std::vector<double> cum(N + 1, 0.0);
  for (int i = 0; i < N; i++) cum[i + 1] = cum[i] + sq_residuals[i];

  const int kmin = std::max(1, static_cast<int>(std::floor(
                                  std::clamp(min_overlap, 0.0, 1.0) * N)));
  double best_xi = 1.0;
  double best_f = std::numeric_limits<double>::max();
  for (int k = kmin; k <= N; k++) {
    const double xi = static_cast<double>(k) / static_cast<double>(N);
    const double e = cum[k] / static_cast<double>(k);
    const double f = std::sqrt(e) / std::pow(xi, lambda);
    if (f < best_f) {
      best_f = f;
      best_xi = xi;
    }
  }
  return best_xi;
}

// ============================================================
// TricpLoPipeline — パイプライン
// ============================================================

TricpLoPipeline::TricpLoPipeline(const TricpLoParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> TricpLoPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> TricpLoPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d TricpLoPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  return pose_ * delta_window_.back();
}

Eigen::Matrix4d TricpLoPipeline::runRegistration(
    const std::vector<Eigen::Vector3d>& source, const Eigen::Matrix4d& base,
    TricpLoResult* result) {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;

  Eigen::Matrix4d T = base;
  int last_corr = 0;
  int last_inliers = 0;
  double last_overlap = params_.overlap_ratio;

  for (int it = 0; it < params_.max_iterations; it++) {
    const auto src = transformPoints(source, T);
    const auto corr = local_map_.getCorrespondences(
        src, params_.max_correspondence_dist, params_.normal_min_neighbors);

    // 全有効対応の point-to-plane 残差を集める。
    struct Item {
      Eigen::Matrix<double, 1, 6> Jr;
      double e;
      double sq;
    };
    std::vector<Item> items;
    items.reserve(source.size());
    std::vector<double> sq;
    sq.reserve(source.size());
    for (size_t k = 0; k < source.size(); k++) {
      const auto& cc = corr[k];
      if (!cc.found || !cc.has_normal ||
          cc.planarity < params_.planarity_threshold)
        continue;
      const double e = cc.normal.dot(src[k] - cc.anchor);
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src[k]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      items.push_back({cc.normal.transpose() * J, e, e * e});
      sq.push_back(e * e);
    }
    if (static_cast<int>(items.size()) < 10) break;

    // オーバーラップ率 ξ を決める (自動: FRMSD 最小化 / 固定: overlap_ratio)。
    const double xi =
        params_.auto_overlap
            ? estimateOverlap(sq, params_.min_overlap, params_.frmsd_lambda)
            : std::clamp(params_.overlap_ratio, 0.0, 1.0);
    const int N = static_cast<int>(items.size());
    int k = std::max(10, static_cast<int>(std::floor(xi * N)));
    k = std::min(k, N);

    // 最小 k 個の二乗残差を採用 (Least Trimmed Squares): k 番目の閾値を求める。
    std::vector<double> tmp = sq;
    std::nth_element(tmp.begin(), tmp.begin() + (k - 1), tmp.end());
    const double thresh = tmp[k - 1];

    Mat6 A = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    int inliers = 0;
    for (const auto& item : items) {
      if (item.sq > thresh) continue;  // トリミング (順位で棄却)
      A += item.Jr.transpose() * item.Jr;
      b -= item.Jr.transpose() * item.e;
      inliers++;
    }
    if (inliers < 6) break;

    const Vec6 d = A.ldlt().solve(b);
    if (!d.allFinite()) break;
    T = expSE3(d) * T;
    last_corr = N;
    last_inliers = inliers;
    last_overlap = xi;
    if (result) result->iterations = it + 1;

    if (d.norm() < params_.convergence_criterion && it > 2) break;
  }

  if (result) {
    result->num_correspondences = last_corr;
    result->num_inliers = last_inliers;
    result->overlap = last_overlap;
  }
  return T;
}

TricpLoResult TricpLoPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  TricpLoResult result;

  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg = voxelDownsample(downsampled, params_.voxel_size);
  if (reg.empty()) reg = downsampled;

  if (frame_count_ == 0) {
    local_map_.addPoints(transformPoints(downsampled, pose_));
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    result.overlap = 1.0;
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

}  // namespace tricp_lo
}  // namespace localization_zoo
