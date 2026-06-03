#include "bievr_lio/bievr_lio.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace bievr_lio {

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
// BumpVoxelMap
// ============================================================

void BumpVoxelMap::rebuild(Voxel& v) const {
  v.has_plane = false;
  if (v.count < min_points_) return;
  const double inv = 1.0 / static_cast<double>(v.count);
  v.mean = v.sum * inv;
  Eigen::Matrix3d cov = v.sum_outer * inv - v.mean * v.mean.transpose();
  cov = 0.5 * (cov + cov.transpose()) + 1e-9 * Eigen::Matrix3d::Identity();
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
  if (es.info() != Eigen::Success) return;
  const Eigen::Vector3d ev = es.eigenvalues();
  const double lambda2 = std::max(ev(2), 1e-12);
  if (ev(0) >= planarity_ratio_ * lambda2) return;  // 平面でないボクセルは bump 無効
  v.normal = es.eigenvectors().col(0).normalized();
  v.plane_variance = std::max(ev(0), 1e-6);
  // 面内基底: 最大/中間固有ベクトル。
  v.ex = es.eigenvectors().col(2).normalized();
  v.ey = es.eigenvectors().col(1).normalized();
  v.has_plane = true;

  // bump 高さ画像を最近点から再構築。
  const int n = grid_ * grid_;
  v.img.assign(n, 0.0);
  v.wimg.assign(n, 0.0);
  const double half = 0.5 * voxel_size_;
  for (const auto& p : v.recent) {
    const Eigen::Vector3d d = p - v.mean;
    const double cx = v.ex.dot(d);
    const double cy = v.ey.dot(d);
    const double h = v.normal.dot(d);
    int iu = static_cast<int>((cx + half) / pixel_res_);
    int iv = static_cast<int>((cy + half) / pixel_res_);
    if (iu < 0 || iu >= grid_ || iv < 0 || iv >= grid_) continue;
    const double w = std::min(0.5, 1.0 / std::max(p.norm(), 1e-3));
    double& I = v.img[iv * grid_ + iu];
    double& W = v.wimg[iv * grid_ + iu];
    I = (W * I + w * h) / (W + w);
    W += w;
  }
}

void BumpVoxelMap::addPoints(const std::vector<Eigen::Vector3d>& points) {
  std::vector<Eigen::Vector3i> touched;
  for (const auto& p : points) {
    auto key = toVoxel(p);
    auto& v = voxels_[key];
    v.count += 1;
    v.sum += p;
    v.sum_outer += p * p.transpose();
    v.recent.push_back(p);
    if (v.recent.size() > 400)
      v.recent.erase(v.recent.begin(),
                     v.recent.begin() + (v.recent.size() - 400));
    touched.push_back(key);
  }
  std::sort(touched.begin(), touched.end(),
            [](const Eigen::Vector3i& a, const Eigen::Vector3i& b) {
              if (a.x() != b.x()) return a.x() < b.x();
              if (a.y() != b.y()) return a.y() < b.y();
              return a.z() < b.z();
            });
  touched.erase(std::unique(touched.begin(), touched.end()), touched.end());
  for (const auto& key : touched) rebuild(voxels_[key]);
}

bool BumpVoxelMap::pixelOf(const Voxel& v, const Eigen::Vector3d& p, int& iu,
                           int& iv, double& fu, double& fv, double& h) const {
  const Eigen::Vector3d d = p - v.mean;
  const double half = 0.5 * voxel_size_;
  const double cx = v.ex.dot(d);
  const double cy = v.ey.dot(d);
  h = v.normal.dot(d);
  const double gu = (cx + half) / pixel_res_ - 0.5;
  const double gv = (cy + half) / pixel_res_ - 0.5;
  iu = static_cast<int>(std::floor(gu));
  iv = static_cast<int>(std::floor(gv));
  fu = gu - iu;
  fv = gv - iv;
  return (iu >= 0 && iu + 1 < grid_ && iv >= 0 && iv + 1 < grid_);
}

BumpVoxelMap::Feature BumpVoxelMap::query(
    const Eigen::Vector3d& world_point) const {
  Feature f;
  const auto key = toVoxel(world_point);
  auto it = voxels_.find(key);
  if (it == voxels_.end() || !it->second.has_plane) {
    // 近傍で平面を持つボクセルを探す。
    double best = std::numeric_limits<double>::max();
    const Voxel* chosen = nullptr;
    for (int dx = -1; dx <= 1; dx++)
      for (int dy = -1; dy <= 1; dy++)
        for (int dz = -1; dz <= 1; dz++) {
          auto jt = voxels_.find(key + Eigen::Vector3i(dx, dy, dz));
          if (jt == voxels_.end() || !jt->second.has_plane) continue;
          const double d = (jt->second.mean - world_point).squaredNorm();
          if (d < best) { best = d; chosen = &jt->second; }
        }
    if (!chosen) return f;
    const Voxel& v = *chosen;
    f.found = true;
    f.has_plane = true;
    f.mean = v.mean;
    f.normal = v.normal;
    f.ex = v.ex;
    f.ey = v.ey;
    f.plane_variance = v.plane_variance;
    return f;  // bump 補間は隣接ボクセルでは省略 (法線拘束のみ)
  }

  const Voxel& v = it->second;
  f.found = true;
  f.has_plane = true;
  f.mean = v.mean;
  f.normal = v.normal;
  f.ex = v.ex;
  f.ey = v.ey;
  f.plane_variance = v.plane_variance;

  int iu, iv;
  double fu, fv, h;
  if (!v.img.empty() && pixelOf(v, world_point, iu, iv, fu, fv, h)) {
    auto I = [&](int u, int w) { return v.img[w * grid_ + u]; };
    // 双線形補間で bump 高さ。
    const double i00 = I(iu, iv), i10 = I(iu + 1, iv);
    const double i01 = I(iu, iv + 1), i11 = I(iu + 1, iv + 1);
    const double itop = i00 * (1 - fu) + i10 * fu;
    const double ibot = i01 * (1 - fu) + i11 * fu;
    f.height = itop * (1 - fv) + ibot * fv;
    // 画像勾配 (中央差分の双線形近似)。
    f.grad_u = ((i10 - i00) * (1 - fv) + (i11 - i01) * fv) / pixel_res_;
    f.grad_v = (ibot - itop) / pixel_res_;
  }
  return f;
}

void BumpVoxelMap::pruneFarVoxels(const Eigen::Vector3d& center,
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
// BievrLIO Pipeline
// ============================================================

BievrLIOPipeline::BievrLIOPipeline(const BievrLIOParams& params)
    : params_(params),
      map_(params.voxel_size, params.voxel_min_points, params.planarity_ratio,
           params.pixel_res) {}

std::vector<Eigen::Vector3d> BievrLIOPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> BievrLIOPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d BievrLIOPipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, double* bump_ratio_out) {
  Eigen::Matrix4d T = initial_guess;
  const double hd = params_.huber_delta;
  double last_ratio = 0.0;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);
    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    int count = 0, bump_count = 0;

    for (size_t i = 0; i < source.size(); i++) {
      const Eigen::Vector3d p_world = R * source[i] + t;
      const auto f = map_.query(p_world);
      if (!f.has_plane) continue;

      // 残差: 法線方向の高さ - bump 画像高さ。
      const double h = f.normal.dot(p_world - f.mean);
      const double r = h - f.height;
      if (std::abs(r) > params_.max_correspondence_dist) continue;

      // 有効法線 = n - (g_u ex + g_v ey): 法線 + 画像勾配で面内 2 方向も拘束。
      Eigen::Vector3d eff_normal = f.normal;
      if (f.grad_u != 0.0 || f.grad_v != 0.0) {
        eff_normal -= f.grad_u * f.ex + f.grad_v * f.ey;
        ++bump_count;
      }

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(p_world);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const Eigen::Matrix<double, 1, 6> Jr = eff_normal.transpose() * J;

      // Huber 重み + 平面分散重み。
      const double huber = std::abs(r) <= hd ? 1.0 : hd / std::abs(r);
      const double w = huber / std::max(f.plane_variance, 1e-4);
      JtJ += w * Jr.transpose() * Jr;
      Jtb += w * Jr.transpose() * r;
      ++count;
    }
    if (count < 10) break;
    last_ratio = static_cast<double>(bump_count) / count;

    const Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) break;
    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;
    if (delta.norm() < params_.convergence_criterion) break;
  }
  if (bump_ratio_out) *bump_ratio_out = last_ratio;
  return T;
}

BievrLIOResult BievrLIOPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  BievrLIOResult result;
  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.pixel_res);
  auto reg = voxelDownsample(downsampled, params_.voxel_size * 0.5);
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

  const Eigen::Matrix4d prediction = pose_ * last_delta_;
  double bump_ratio = 0.0;
  const Eigen::Matrix4d new_pose = runICP(reg, prediction, &bump_ratio);
  last_delta_ = pose_.inverse() * new_pose;
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
  result.bump_constraint_ratio = bump_ratio;
  frame_count_++;
  return result;
}

}  // namespace bievr_lio
}  // namespace localization_zoo
