#include "dilo/dilo.h"

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace localization_zoo {
namespace dilo {

namespace {

constexpr double kDeg2Rad = M_PI / 180.0;

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

Eigen::Matrix4d expSE3(const Eigen::Matrix<double, 6, 1>& xi) {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = expSO3(xi.head<3>());
  T.block<3, 1>(0, 3) = xi.tail<3>();
  return T;
}

}  // namespace

// ============================================================
// SphericalRangeImage
// ============================================================

bool SphericalRangeImage::project(const Eigen::Vector3d& p, int* row,
                                  int* col) const {
  const double range = p.norm();
  if (range < params_.min_range || range > params_.max_range) return false;
  const double yaw = std::atan2(p.y(), p.x());           // [-pi, pi]
  const double pitch = std::asin(std::clamp(p.z() / range, -1.0, 1.0));
  const double fov_up = params_.fov_up_deg * kDeg2Rad;
  const double fov_down = params_.fov_down_deg * kDeg2Rad;
  const double fov = fov_up - fov_down;
  if (fov <= 1e-9) return false;
  // 行: 仰角 (上=0)。列: 方位角。
  double r = (1.0 - (pitch - fov_down) / fov) * params_.height;
  double c = (0.5 * (yaw / M_PI + 1.0)) * params_.width;
  int ri = static_cast<int>(std::floor(r));
  int ci = static_cast<int>(std::floor(c));
  if (ri < 0 || ri >= params_.height) return false;
  ci = ((ci % params_.width) + params_.width) % params_.width;  // 方位は巡回
  *row = ri;
  *col = ci;
  return true;
}

void SphericalRangeImage::build(const std::vector<Eigen::Vector3d>& points,
                                const Params& params) {
  params_ = params;
  pixels_.assign(static_cast<size_t>(params.height) * params.width, Pixel());
  for (const auto& p : points) {
    int r, c;
    if (!project(p, &r, &c)) continue;
    const double range = p.norm();
    Pixel& px = pixels_[idx(r, c)];
    if (!px.valid || range < px.range) {  // 最近 range を採用
      px.point = p;
      px.range = range;
      px.valid = true;
    }
  }
  computeNormals();
}

void SphericalRangeImage::computeNormals() {
  const int H = params_.height, W = params_.width;
  for (int r = 0; r < H - 1; r++) {
    for (int c = 0; c < W; c++) {
      Pixel& px = pixels_[idx(r, c)];
      if (!px.valid) continue;
      const int cr = (c + 1) % W;
      const Pixel& right = pixels_[idx(r, cr)];
      const Pixel& down = pixels_[idx(r + 1, c)];
      if (!right.valid || !down.valid) continue;
      const Eigen::Vector3d v1 = right.point - px.point;
      const Eigen::Vector3d v2 = down.point - px.point;
      Eigen::Vector3d n = v1.cross(v2);
      const double nn = n.norm();
      if (nn < 1e-9) continue;
      n /= nn;
      // センサ (原点) 方向へ向ける。
      if (n.dot(px.point) > 0.0) n = -n;
      px.normal = n;
      px.has_normal = true;
    }
  }
}

SphericalRangeImage::Pixel SphericalRangeImage::lookup(
    const Eigen::Vector3d& p) const {
  int r, c;
  if (pixels_.empty() || !project(p, &r, &c)) return Pixel();
  return pixels_[idx(r, c)];
}

// ============================================================
// DiloPipeline
// ============================================================

DiloPipeline::DiloPipeline(const DiloParams& params) : params_(params) {}

void DiloPipeline::setInitialPose(const Eigen::Matrix4d& pose) {
  pose_ = pose;
  keyframe_pose_ = pose;
}

std::vector<Eigen::Vector3d> DiloPipeline::prepare(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> filtered;
  filtered.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) filtered.push_back(p);
  }
  if (params_.source_voxel_size <= 0.0) return filtered;
  // 軽量ボクセル間引き。
  std::unordered_map<long long, Eigen::Vector3d> grid;
  const double vs = params_.source_voxel_size;
  for (const auto& p : filtered) {
    const long long kx = static_cast<long long>(std::floor(p.x() / vs));
    const long long ky = static_cast<long long>(std::floor(p.y() / vs));
    const long long kz = static_cast<long long>(std::floor(p.z() / vs));
    const long long key = (kx * 73856093LL) ^ (ky * 19349663LL) ^ (kz * 83492791LL);
    grid.emplace(key, p);
  }
  std::vector<Eigen::Vector3d> out;
  out.reserve(grid.size());
  for (const auto& kv : grid) out.push_back(kv.second);
  return out;
}

Eigen::Matrix4d DiloPipeline::align(const std::vector<Eigen::Vector3d>& source,
                                    const Eigen::Matrix4d& init_rel,
                                    DiloResult* result) {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;
  Eigen::Matrix4d T = init_rel;  // キーフレーム→現在 の相対姿勢
  int last_corr = 0;

  for (int it = 0; it < params_.max_iterations; it++) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);
    Mat6 A = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    int used = 0;

    for (const auto& p : source) {
      // 現在点をキーフレーム座標へ。
      const Eigen::Vector3d p_kf = R * p + t;
      const auto px = keyframe_sri_.lookup(p_kf);
      if (!px.valid || !px.has_normal) continue;
      const double e = px.normal.dot(p_kf - px.point);
      if (std::abs(e) > params_.initial_threshold) continue;
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(p_kf);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const Eigen::Matrix<double, 1, 6> Jr = px.normal.transpose() * J;
      double w = 1.0;
      if (params_.robust_scale > 0.0) {
        const double s = e / params_.robust_scale;
        w /= (1.0 + s * s);
      }
      A += w * Jr.transpose() * Jr;
      b -= w * Jr.transpose() * e;
      ++used;
    }

    last_corr = used;
    if (used < 10) break;
    const Vec6 dx = A.ldlt().solve(b);
    if (!dx.allFinite()) break;
    T = expSE3(dx) * T;
    if (result) result->iterations = it + 1;
    if (dx.norm() < params_.convergence_criterion) break;
  }

  if (result) result->num_correspondences = last_corr;
  return T;
}

DiloResult DiloPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  DiloResult result;
  const auto src = prepare(frame);

  SphericalRangeImage::Params sp;
  sp.height = params_.sri_height;
  sp.width = params_.sri_width;
  sp.fov_up_deg = params_.fov_up_deg;
  sp.fov_down_deg = params_.fov_down_deg;
  sp.min_range = params_.min_range;
  sp.max_range = params_.max_range;

  if (frame_count_ == 0) {
    keyframe_sri_.build(frame, sp);
    keyframe_pose_ = pose_;
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    result.keyframe_updated = true;
    return result;
  }

  // 等速予測 (世界) → キーフレーム相対の初期値。
  const Eigen::Matrix4d predicted_world = pose_ * last_delta_;
  const Eigen::Matrix4d init_rel = keyframe_pose_.inverse() * predicted_world;
  const Eigen::Matrix4d T_rel = align(src, init_rel, &result);

  const Eigen::Matrix4d new_pose = keyframe_pose_ * T_rel;
  last_delta_ = pose_.inverse() * new_pose;
  pose_ = new_pose;

  // キーフレーム切替判定 (相対運動が大きくなったら更新)。
  const double trans = T_rel.block<3, 1>(0, 3).norm();
  const double rot_deg = logSO3(T_rel.block<3, 3>(0, 0)).norm() * 180.0 / M_PI;
  if (trans > params_.keyframe_translation ||
      rot_deg > params_.keyframe_rotation_deg) {
    keyframe_sri_.build(frame, sp);
    keyframe_pose_ = pose_;
    result.keyframe_updated = true;
  }

  result.pose = pose_;
  result.converged = (pose_.array().isFinite().all() &&
                      result.num_correspondences >= 10);
  frame_count_++;
  return result;
}

}  // namespace dilo
}  // namespace localization_zoo
