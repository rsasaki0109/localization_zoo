#include "r_voxelmap/r_voxelmap.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace localization_zoo {
namespace r_voxelmap {

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

/// 点集合に平面を最小二乗フィットし、平面性 (λ0<ratio·λ2) なら平面を返す。
bool fitPlane(const std::vector<Eigen::Vector3d>& pts, double planarity_ratio,
              Plane& out) {
  if (pts.size() < 4) return false;
  Eigen::Vector3d c = Eigen::Vector3d::Zero();
  for (const auto& p : pts) c += p;
  c /= static_cast<double>(pts.size());
  Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
  for (const auto& p : pts) {
    const Eigen::Vector3d d = p - c;
    cov += d * d.transpose();
  }
  cov /= static_cast<double>(pts.size());
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
  if (es.info() != Eigen::Success) return false;
  const Eigen::Vector3d ev = es.eigenvalues();
  const double lambda2 = std::max(ev(2), 1e-12);
  if (ev(0) >= planarity_ratio * lambda2) return false;  // 分布妥当性チェック
  out.centroid = c;
  out.normal = es.eigenvectors().col(0).normalized();
  out.plane_variance = std::max(ev(0), 1e-6);
  out.valid = true;
  return true;
}

}  // namespace

// ============================================================
// RecursiveVoxelMap
// ============================================================

int RecursiveVoxelMap::childIndex(const Node& node,
                                  const Eigen::Vector3d& p) const {
  int idx = 0;
  if (p.x() >= node.center.x()) idx |= 1;
  if (p.y() >= node.center.y()) idx |= 2;
  if (p.z() >= node.center.z()) idx |= 4;
  return idx;
}

void RecursiveVoxelMap::refit(Node& node) const {
  node.plane.valid = false;
  node.has_children = false;
  for (auto& ch : node.children) ch.reset();
  if (static_cast<int>(node.points.size()) < min_points_) return;

  // このレベルで平面フィット。
  Plane plane;
  if (fitPlane(node.points, planarity_ratio_, plane)) {
    // インライア/外れ値判定。
    std::vector<Eigen::Vector3d> outliers;
    for (const auto& p : node.points) {
      const double dist = std::abs(plane.normal.dot(p - plane.centroid));
      if (dist > inlier_dist_) outliers.push_back(p);
    }
    node.plane = plane;
    // 外れ値が少なければこのレベルの平面を確定。
    if (node.depth >= max_depth_ ||
        outliers.size() < static_cast<size_t>(min_points_)) {
      return;
    }
    // 外れ値を子へ伝播して再帰処理 (detect-and-reuse)。
    for (const auto& p : outliers) {
      const int ci = childIndex(node, p);
      if (!node.children[ci]) {
        auto child = std::make_unique<Node>();
        child->depth = node.depth + 1;
        child->half = node.half * 0.5;
        Eigen::Vector3d off(((ci & 1) ? 1 : -1), ((ci & 2) ? 1 : -1),
                            ((ci & 4) ? 1 : -1));
        child->center = node.center + child->half * off;
        node.children[ci] = std::move(child);
      }
      node.children[ci]->points.push_back(p);
    }
    node.has_children = true;
    for (auto& ch : node.children)
      if (ch) refit(*ch);
    return;
  }

  // 平面が取れない: 細分化して子で再帰 (over-segmentation 回避のため深さ制限)。
  if (node.depth >= max_depth_) return;
  for (const auto& p : node.points) {
    const int ci = childIndex(node, p);
    if (!node.children[ci]) {
      auto child = std::make_unique<Node>();
      child->depth = node.depth + 1;
      child->half = node.half * 0.5;
      Eigen::Vector3d off(((ci & 1) ? 1 : -1), ((ci & 2) ? 1 : -1),
                          ((ci & 4) ? 1 : -1));
      child->center = node.center + child->half * off;
      node.children[ci] = std::move(child);
    }
    node.children[ci]->points.push_back(p);
  }
  node.has_children = true;
  for (auto& ch : node.children)
    if (ch) refit(*ch);
}

void RecursiveVoxelMap::addPoints(const std::vector<Eigen::Vector3d>& points) {
  std::vector<Eigen::Vector3i> touched;
  for (const auto& p : points) {
    const auto key = toVoxel(p);
    auto it = roots_.find(key);
    if (it == roots_.end()) {
      auto node = std::make_unique<Node>();
      node->depth = 0;
      node->half = voxel_size_ * 0.5;
      node->center =
          (key.cast<double>() + Eigen::Vector3d::Constant(0.5)) * voxel_size_;
      it = roots_.emplace(key, std::move(node)).first;
    }
    it->second->points.push_back(p);
    it->second->dirty = true;
    touched.push_back(key);
  }
  std::sort(touched.begin(), touched.end(),
            [](const Eigen::Vector3i& a, const Eigen::Vector3i& b) {
              if (a.x() != b.x()) return a.x() < b.x();
              if (a.y() != b.y()) return a.y() < b.y();
              return a.z() < b.z();
            });
  touched.erase(std::unique(touched.begin(), touched.end()), touched.end());
  for (const auto& key : touched) {
    auto& node = roots_[key];
    // 点が多すぎる場合は古い点を間引いて再フィットコストを抑える。
    if (node->points.size() > 200)
      node->points.erase(node->points.begin(),
                         node->points.begin() + (node->points.size() - 200));
    refit(*node);
    node->dirty = false;
  }
}

const Plane* RecursiveVoxelMap::queryNode(const Node& node,
                                          const Eigen::Vector3d& p) const {
  // より細かい子の有効平面を優先 (最も細かい有効平面を返す)。
  if (node.has_children) {
    const int ci = childIndex(node, p);
    if (node.children[ci]) {
      const Plane* deep = queryNode(*node.children[ci], p);
      if (deep) return deep;
    }
  }
  if (node.plane.valid) return &node.plane;
  return nullptr;
}

Plane RecursiveVoxelMap::query(const Eigen::Vector3d& world_point) const {
  const auto key = toVoxel(world_point);
  // 含有ボクセル優先 (決定的)。その octree を降りて最細の有効平面を返す。
  auto it = roots_.find(key);
  if (it != roots_.end()) {
    const Plane* pl = queryNode(*it->second, world_point);
    if (pl) return *pl;
  }
  // 含有ボクセルに平面が無い場合のみ、26 近傍から最近傍重心の有効平面を選ぶ。
  double best = std::numeric_limits<double>::max();
  Plane result;
  for (int dx = -1; dx <= 1; dx++)
    for (int dy = -1; dy <= 1; dy++)
      for (int dz = -1; dz <= 1; dz++) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        auto jt = roots_.find(key + Eigen::Vector3i(dx, dy, dz));
        if (jt == roots_.end()) continue;
        const Plane* pl = queryNode(*jt->second, world_point);
        if (!pl) continue;
        const double d = (pl->centroid - world_point).squaredNorm();
        if (d < best) {
          best = d;
          result = *pl;
        }
      }
  return result;
}

void RecursiveVoxelMap::pruneFarVoxels(const Eigen::Vector3d& center,
                                       double max_distance) {
  const double max_distance_sq = max_distance * max_distance;
  for (auto it = roots_.begin(); it != roots_.end();) {
    if ((it->second->center - center).squaredNorm() > max_distance_sq) {
      it = roots_.erase(it);
    } else {
      ++it;
    }
  }
}

// ============================================================
// RVoxelMap Pipeline
// ============================================================

RVoxelMapPipeline::RVoxelMapPipeline(const RVoxelMapParams& params)
    : params_(params),
      map_(params.voxel_size, params.voxel_min_points, params.planarity_ratio,
           params.inlier_dist, params.max_depth) {}

std::vector<Eigen::Vector3d> RVoxelMapPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> RVoxelMapPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d RVoxelMapPipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, double* matched_ratio_out) {
  Eigen::Matrix4d T = initial_guess;
  double last_ratio = 0.0;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);

    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    int count = 0;

    for (size_t i = 0; i < source.size(); i++) {
      const Eigen::Vector3d p_world = R * source[i] + t;
      const Plane pl = map_.query(p_world);
      if (!pl.valid) continue;
      const double r = pl.normal.dot(p_world - pl.centroid);
      if (std::abs(r) > params_.max_correspondence_dist) continue;
      // 平面分散で重み付け (薄い平面ほど信頼)。
      double w = std::min(params_.max_plane_weight,
                          1.0 / std::max(pl.plane_variance, 1e-4));
      const double abs_r = std::abs(r);
      if (params_.huber_residual > 0.0 && abs_r > params_.huber_residual) {
        w *= params_.huber_residual / abs_r;
      }

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(p_world);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const Eigen::Matrix<double, 1, 6> Jr = pl.normal.transpose() * J;
      JtJ += w * Jr.transpose() * Jr;
      Jtb += w * Jr.transpose() * r;
      ++count;
    }

    if (count < 10) break;
    last_ratio = static_cast<double>(count) / source.size();

    const double diag_mean =
        JtJ.diagonal().cwiseAbs().mean();
    const double damping =
        params_.icp_damping * std::max(1.0, diag_mean);
    JtJ.diagonal().array() += damping;

    const Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) break;
    Eigen::Matrix<double, 6, 1> limited_delta = delta;
    const double rot_norm = limited_delta.head<3>().norm();
    if (params_.max_iteration_rotation > 0.0 &&
        rot_norm > params_.max_iteration_rotation) {
      limited_delta.head<3>() *= params_.max_iteration_rotation / rot_norm;
    }
    const double trans_norm = limited_delta.tail<3>().norm();
    if (params_.max_iteration_translation > 0.0 &&
        trans_norm > params_.max_iteration_translation) {
      limited_delta.tail<3>() *=
          params_.max_iteration_translation / trans_norm;
    }
    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(limited_delta.head<3>());
    dT.block<3, 1>(0, 3) = limited_delta.tail<3>();
    T = dT * T;
    if (limited_delta.norm() < params_.convergence_criterion) break;
  }

  if (matched_ratio_out) *matched_ratio_out = last_ratio;
  return T;
}

std::vector<Eigen::Vector3d> RVoxelMapPipeline::transformToWorld(
    const std::vector<Eigen::Vector3d>& pts, const Eigen::Matrix4d& pose) const {
  const Eigen::Matrix3d R = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d t = pose.block<3, 1>(0, 3);
  std::vector<Eigen::Vector3d> w(pts.size());
  for (size_t i = 0; i < pts.size(); i++) w[i] = R * pts[i] + t;
  return w;
}

bool RVoxelMapPipeline::registrationMotionOk(
    const Eigen::Matrix4d& reference, const Eigen::Matrix4d& candidate) const {
  if (!candidate.array().isFinite().all()) return false;
  const Eigen::Matrix4d rel = candidate * reference.inverse();
  const double trans = rel.block<3, 1>(0, 3).norm();
  const Eigen::Matrix3d R = rel.block<3, 3>(0, 0);
  const double tr = std::clamp((R.trace() - 1.0) * 0.5, -1.0, 1.0);
  const double angle = std::acos(tr);
  return trans <= params_.max_registration_translation &&
         angle <= params_.max_registration_rotation;
}

Eigen::Matrix4d RVoxelMapPipeline::runScanToScanICP(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, double* matched_ratio_out) const {
  if (last_scan_world_.empty() || source.empty()) {
    if (matched_ratio_out) *matched_ratio_out = 0.0;
    return initial_guess;
  }

  const double max_dist = params_.fallback_max_correspondence_dist;
  const double max_dist_sq = max_dist * max_dist;
  const double cell = std::max(0.25, max_dist);
  std::unordered_map<Eigen::Vector3i, std::vector<int>, VoxelHash> grid;
  grid.reserve(last_scan_world_.size());
  auto keyOf = [cell](const Eigen::Vector3d& p) {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / cell)),
                           static_cast<int>(std::floor(p.y() / cell)),
                           static_cast<int>(std::floor(p.z() / cell)));
  };
  for (int i = 0; i < static_cast<int>(last_scan_world_.size()); ++i) {
    grid[keyOf(last_scan_world_[i])].push_back(i);
  }

  Eigen::Matrix4d T = initial_guess;
  double last_ratio = 0.0;
  for (int iter = 0; iter < params_.fallback_max_iterations; ++iter) {
    const auto source_world = transformToWorld(source, T);
    std::vector<Eigen::Vector3d> src_corr;
    std::vector<Eigen::Vector3d> tgt_corr;
    src_corr.reserve(source_world.size());
    tgt_corr.reserve(source_world.size());

    for (const auto& p : source_world) {
      const Eigen::Vector3i key = keyOf(p);
      double best = max_dist_sq;
      int best_idx = -1;
      for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
          for (int dz = -1; dz <= 1; ++dz) {
            auto it = grid.find(key + Eigen::Vector3i(dx, dy, dz));
            if (it == grid.end()) continue;
            for (const int idx : it->second) {
              const double d = (last_scan_world_[idx] - p).squaredNorm();
              if (d < best) {
                best = d;
                best_idx = idx;
              }
            }
          }
      if (best_idx >= 0) {
        src_corr.push_back(p);
        tgt_corr.push_back(last_scan_world_[best_idx]);
      }
    }

    last_ratio = static_cast<double>(src_corr.size()) /
                 static_cast<double>(source.size());
    if (src_corr.size() < 20) break;

    Eigen::Vector3d src_mean = Eigen::Vector3d::Zero();
    Eigen::Vector3d tgt_mean = Eigen::Vector3d::Zero();
    for (size_t i = 0; i < src_corr.size(); ++i) {
      src_mean += src_corr[i];
      tgt_mean += tgt_corr[i];
    }
    src_mean /= static_cast<double>(src_corr.size());
    tgt_mean /= static_cast<double>(tgt_corr.size());

    Eigen::Matrix3d H = Eigen::Matrix3d::Zero();
    for (size_t i = 0; i < src_corr.size(); ++i) {
      H += (src_corr[i] - src_mean) * (tgt_corr[i] - tgt_mean).transpose();
    }
    Eigen::JacobiSVD<Eigen::Matrix3d> svd(
        H, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Eigen::Matrix3d R_delta = svd.matrixV() * svd.matrixU().transpose();
    if (R_delta.determinant() < 0.0) {
      Eigen::Matrix3d V = svd.matrixV();
      V.col(2) *= -1.0;
      R_delta = V * svd.matrixU().transpose();
    }
    Eigen::Vector3d t_delta = tgt_mean - R_delta * src_mean;

    Eigen::AngleAxisd aa(R_delta);
    double angle = std::abs(aa.angle());
    if (params_.max_iteration_rotation > 0.0 &&
        angle > params_.max_iteration_rotation) {
      R_delta = Eigen::AngleAxisd(
                    params_.max_iteration_rotation,
                    aa.axis().normalized()).toRotationMatrix();
      angle = params_.max_iteration_rotation;
    }
    const double trans_norm = t_delta.norm();
    if (params_.max_iteration_translation > 0.0 &&
        trans_norm > params_.max_iteration_translation) {
      t_delta *= params_.max_iteration_translation / trans_norm;
    }

    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = R_delta;
    dT.block<3, 1>(0, 3) = t_delta;
    T = dT * T;
    if (t_delta.norm() + angle < params_.convergence_criterion) break;
  }

  if (matched_ratio_out) *matched_ratio_out = last_ratio;
  return T;
}

RVoxelMapResult RVoxelMapPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  RVoxelMapResult result;

  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg = voxelDownsample(downsampled, params_.voxel_size);
  if (reg.empty()) reg = downsampled;

  if (frame_count_ == 0) {
    last_scan_world_ = transformToWorld(reg, pose_);
    map_.addPoints(transformToWorld(downsampled, pose_));
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  const Eigen::Matrix4d prediction = pose_ * last_delta_;
  double matched_ratio = 0.0;
  const Eigen::Matrix4d new_pose = runICP(reg, prediction, &matched_ratio);
  Eigen::Matrix4d accepted_pose = new_pose;
  bool accepted = matched_ratio >= params_.min_map_matched_ratio;
  const bool safety_enabled = params_.min_map_matched_ratio > 0.0 ||
                              params_.enable_scan_to_scan_fallback;
  if (accepted && safety_enabled) {
    accepted = registrationMotionOk(prediction, accepted_pose);
  }

  if (!accepted && params_.enable_scan_to_scan_fallback) {
    double fallback_ratio = 0.0;
    const Eigen::Matrix4d fallback_pose =
        runScanToScanICP(reg, prediction, &fallback_ratio);
    if (fallback_ratio >= params_.min_fallback_matched_ratio &&
        registrationMotionOk(prediction, fallback_pose)) {
      accepted_pose = fallback_pose;
      accepted = true;
      result.used_fallback = true;
      result.fallback_matched_ratio = fallback_ratio;
    }
  }

  if (!accepted) {
    accepted_pose = prediction;
  }

  last_delta_ = pose_.inverse() * accepted_pose;
  pose_ = accepted_pose;

  if (accepted) {
    map_.addPoints(transformToWorld(downsampled, pose_));
    last_scan_world_ = transformToWorld(reg, pose_);
  }
  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.converged = accepted && pose_.array().isFinite().all();
  result.iterations = params_.max_icp_iterations;
  result.matched_ratio = matched_ratio;
  frame_count_++;
  return result;
}

}  // namespace r_voxelmap
}  // namespace localization_zoo
