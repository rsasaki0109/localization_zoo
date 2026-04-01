#include "kiss_icp/kiss_icp.h"

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace localization_zoo {
namespace kiss_icp {

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
  for (auto& p : points) {
    auto key = toVoxel(p);
    auto& vb = map_[key];
    if (static_cast<int>(vb.points.size()) < max_points_) {
      vb.points.push_back(p);
    }
  }
}

void VoxelHashMap::pruneFarVoxels(const Eigen::Vector3d& center,
                                  double max_distance) {
  const double max_distance_sq = max_distance * max_distance;
  for (auto it = map_.begin(); it != map_.end();) {
    const Eigen::Vector3d voxel_center =
        (it->first.cast<double>() + Eigen::Vector3d::Constant(0.5)) * voxel_size_;
    if ((voxel_center - center).squaredNorm() > max_distance_sq) {
      it = map_.erase(it);
    } else {
      ++it;
    }
  }
}

std::vector<VoxelHashMap::Correspondence> VoxelHashMap::getCorrespondences(
    const std::vector<Eigen::Vector3d>& points, double max_dist) const {
  std::vector<Correspondence> correspondences(points.size());
  double max_dist_sq = max_dist * max_dist;

  for (size_t i = 0; i < points.size(); i++) {
    const auto& query = points[i];
    auto key = toVoxel(query);
    double best_dist = max_dist_sq;
    Eigen::Vector3d best_point = Eigen::Vector3d::Zero();
    bool found = false;

    // 27近傍ボクセルを探索
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
          Eigen::Vector3i neighbor = key + Eigen::Vector3i(dx, dy, dz);
          auto it = map_.find(neighbor);
          if (it == map_.end()) continue;
          for (const auto& mp : it->second.points) {
            const double ddx = mp.x() - query.x();
            const double ddy = mp.y() - query.y();
            const double ddz = mp.z() - query.z();
            const double d = ddx * ddx + ddy * ddy + ddz * ddz;
            if (d < best_dist) {
              best_dist = d;
              best_point = mp;
              found = true;
            }
          }
        }
      }
    }
    correspondences[i].point = best_point;
    correspondences[i].found = found;
  }
  return correspondences;
}

// ============================================================
// KISS-ICP Pipeline
// ============================================================

KISSICPPipeline::KISSICPPipeline(const KISSICPParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> KISSICPPipeline::voxelDownsample(
    const std::vector<Eigen::Vector3d>& points, double voxel_size) const {
  std::unordered_map<Eigen::Vector3i, Eigen::Vector3d, VoxelHash> grid;
  for (auto& p : points) {
    Eigen::Vector3i key(static_cast<int>(std::floor(p.x() / voxel_size)),
                        static_cast<int>(std::floor(p.y() / voxel_size)),
                        static_cast<int>(std::floor(p.z() / voxel_size)));
    grid.emplace(key, p);  // 各ボクセルから1点だけ
  }
  std::vector<Eigen::Vector3d> out;
  out.reserve(grid.size());
  for (auto& [k, v] : grid) out.push_back(v);
  return out;
}

std::vector<Eigen::Vector3d> KISSICPPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (auto& p : points) {
    double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

Eigen::Matrix4d KISSICPPipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const std::vector<Eigen::Vector3d>& target,
    const Eigen::Matrix4d& initial_guess, double max_correspondence_dist) {
  (void)target;
  Eigen::Matrix4d T = initial_guess;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    auto src_transformed = transformPoints(source, T);

    // 対応関係
    auto correspondences =
        local_map_.getCorrespondences(src_transformed, max_correspondence_dist);

    // Point-to-Point ICP (ロバストカーネル付き)
    // min Σ w_i ||T p_i - q_i||²
    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();

    double kernel_threshold = max_correspondence_dist;
    int valid_correspondences = 0;

    for (size_t i = 0; i < source.size(); i++) {
      if (!correspondences[i].found) continue;

      Eigen::Vector3d residual = src_transformed[i] - correspondences[i].point;
      double r_norm = residual.norm();
      if (r_norm > max_correspondence_dist) continue;
      valid_correspondences++;

      // Welsch kernel weight
      double w = std::exp(-0.5 * (r_norm / kernel_threshold) *
                          (r_norm / kernel_threshold));

      // Residual r = T p - q, so J = [-skew(Tp), I]
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src_transformed[i]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      JtJ += w * J.transpose() * J;
      Jtb += w * J.transpose() * residual;
    }

    if (valid_correspondences < 10) {
      break;
    }

    Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) {
      break;
    }

    // 更新
    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;

    if (delta.norm() < params_.convergence_criterion) break;
  }
  return T;
}

double KISSICPPipeline::computeAdaptiveThreshold() {
  if (model_errors_.empty()) return params_.initial_threshold;
  // σ = median of recent model errors
  auto sorted = model_errors_;
  std::sort(sorted.begin(), sorted.end());
  double median = sorted[sorted.size() / 2];
  return std::max(3.0 * median, params_.initial_threshold * 0.1);
}

KISSICPResult KISSICPPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  KISSICPResult result;

  // 1. Range filter + Voxel downsample
  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto registration_points = voxelDownsample(downsampled, params_.voxel_size);
  if (registration_points.empty()) registration_points = downsampled;

  if (frame_count_ == 0) {
    // 最初のフレーム
    auto world_pts = transformPoints(downsampled, pose_);
    local_map_.addPoints(world_pts);
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  // 2. Motion prediction (等速モデル)
  Eigen::Matrix4d prediction = pose_ * last_delta_;

  // 3. 適応的閾値
  double adaptive_threshold = computeAdaptiveThreshold();

  // 4. ICP
  Eigen::Matrix4d new_pose =
      runICP(registration_points, {}, prediction, adaptive_threshold);

  // 5. モデルエラーの更新
  last_delta_ = pose_.inverse() * new_pose;
  double model_error = last_delta_.block<3, 1>(0, 3).norm();
  model_errors_.push_back(model_error);
  if (model_errors_.size() > 100) model_errors_.erase(model_errors_.begin());

  pose_ = new_pose;

  // 6. マップ更新
  auto world_pts = transformPoints(downsampled, pose_);
  local_map_.addPoints(world_pts);
  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    local_map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.converged = (pose_.array().isFinite().all() &&
                      (new_pose - prediction).cwiseAbs().maxCoeff() < 1e3);
  result.iterations = params_.max_icp_iterations;
  frame_count_++;

  return result;
}

}  // namespace kiss_icp
}  // namespace localization_zoo
