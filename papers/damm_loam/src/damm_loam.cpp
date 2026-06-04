#include "damm_loam/damm_loam.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace damm_loam {

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

double conditionNumber(const Eigen::Matrix3d& block) {
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(block);
  if (es.info() != Eigen::Success) return 1.0;
  const Eigen::Vector3d ev = es.eigenvalues();  // 昇順
  const double lmin = std::max(ev(0), 1e-9);
  const double lmax = std::max(ev(2), 1e-9);
  return lmax / lmin;
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
// DAMM-LOAM Pipeline
// ============================================================

DammLoamPipeline::DammLoamPipeline(const DammLoamParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> DammLoamPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> DammLoamPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

// 局所法線コンセンサスでスキャンを分類 (range-image の法線支配を近似)。
std::vector<PointClass> DammLoamPipeline::classifyScan(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<PointClass> labels(points.size(), PointClass::kUnknown);
  // 近傍探索用に一時ボクセルグリッドを構築。
  std::unordered_map<Eigen::Vector3i, std::vector<int>, VoxelHash> grid;
  const double vs = params_.voxel_size;
  auto toVox = [&](const Eigen::Vector3d& p) {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / vs)),
                           static_cast<int>(std::floor(p.y() / vs)),
                           static_cast<int>(std::floor(p.z() / vs)));
  };
  for (int i = 0; i < static_cast<int>(points.size()); i++)
    grid[toVox(points[i])].push_back(i);

  for (size_t i = 0; i < points.size(); i++) {
    const auto& q = points[i];
    const auto key = toVox(q);
    std::vector<Eigen::Vector3d> nbr;
    nbr.reserve(32);
    for (int dx = -1; dx <= 1; dx++)
      for (int dy = -1; dy <= 1; dy++)
        for (int dz = -1; dz <= 1; dz++) {
          auto it = grid.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == grid.end()) continue;
          for (int idx : it->second) nbr.push_back(points[idx]);
        }
    if (static_cast<int>(nbr.size()) < params_.normal_min_neighbors) {
      labels[i] = PointClass::kNonPlanar;
      continue;
    }
    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
    for (const auto& n : nbr) centroid += n;
    centroid /= static_cast<double>(nbr.size());
    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    for (const auto& n : nbr) {
      const Eigen::Vector3d d = n - centroid;
      cov += d * d.transpose();
    }
    cov /= static_cast<double>(nbr.size());
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
    if (es.info() != Eigen::Success) {
      labels[i] = PointClass::kNonPlanar;
      continue;
    }
    const Eigen::Vector3d ev = es.eigenvalues();  // 昇順
    const double lambda2 = std::max(ev(2), 1e-12);
    const double planarity = std::clamp((ev(1) - ev(0)) / lambda2, 0.0, 1.0);
    const double linearity = std::clamp((ev(2) - ev(1)) / lambda2, 0.0, 1.0);
    const Eigen::Vector3d normal = es.eigenvectors().col(0).normalized();
    const double nz = std::abs(normal.z());

    if (planarity >= params_.planarity_threshold) {
      if (nz >= params_.vertical_cos) {
        labels[i] = (q.z() < params_.ground_z) ? PointClass::kGround
                                               : PointClass::kRoof;
      } else if (nz <= params_.horizontal_cos) {
        labels[i] = PointClass::kWall;
      } else {
        labels[i] = PointClass::kNonPlanar;
      }
    } else if (linearity >= params_.planarity_threshold) {
      labels[i] = PointClass::kEdge;
    } else {
      labels[i] = PointClass::kNonPlanar;
    }
  }
  return labels;
}

Eigen::Matrix4d DammLoamPipeline::runICP(
    const std::vector<Eigen::Vector3d>& source,
    const Eigen::Matrix4d& initial_guess, Eigen::Matrix<double, 6, 6>* H_out,
    double* S_trans_out, bool* degenerate_out) {
  Eigen::Matrix4d T = initial_guess;
  const double kernel = params_.initial_threshold;
  Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
  double S_trans = 1.0;
  bool degenerate = false;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const auto src = transformPoints(source, T);
    const auto corr = local_map_.getCorrespondences(
        src, params_.initial_threshold, params_.normal_min_neighbors);

    // --- (1) 平面法線から並進 Hessian の幾何ブロックを組み、退化方向を抽出 ---
    Eigen::Matrix3d Htt_geom = Eigen::Matrix3d::Zero();
    for (size_t i = 0; i < source.size(); i++) {
      const auto& c = corr[i];
      if (!c.found || !c.has_normal ||
          c.planarity < params_.planarity_threshold)
        continue;
      const double e = c.normal.dot(src[i] - c.anchor);
      if (std::abs(e) > kernel) continue;
      Htt_geom += c.normal * c.normal.transpose();
    }
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(Htt_geom);
    Eigen::Vector3d evals = Eigen::Vector3d::Ones();
    Eigen::Matrix3d evecs = Eigen::Matrix3d::Identity();
    if (es.info() == Eigen::Success && Htt_geom.trace() > 1e-9) {
      evals = es.eigenvalues();  // 昇順
      evecs = es.eigenvectors();
    }
    const double lmax = std::max(evals(2), 1e-9);
    S_trans = lmax / std::max(evals(0), 1e-9);
    // 相対基準で退化方向 (固有値が小さい軸) を判定。
    std::array<bool, 3> deg_dir{false, false, false};
    for (int k = 0; k < 3; k++)
      deg_dir[k] = evals(k) < params_.degeneracy_ratio * lmax;
    degenerate = deg_dir[0];  // 最小軸が退化していれば真

    // --- (2)(3) クラス別多メトリック残差 + 退化方向への適応的重み増幅 ---
    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    int count = 0;
    for (size_t i = 0; i < source.size(); i++) {
      const auto& c = corr[i];
      if (!c.found) continue;
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src[i]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      const bool planar =
          c.has_normal && c.planarity >= params_.planarity_threshold;
      if (planar) {
        // point-to-plane: 並進ヤコビ行は法線 n。退化軸に整列する点を増幅。
        const double e = c.normal.dot(src[i] - c.anchor);
        if (std::abs(e) > kernel) continue;
        double boost = 1.0;
        for (int k = 0; k < 3; k++) {
          if (!deg_dir[k]) continue;
          const double align = std::abs(c.normal.dot(evecs.col(k)));
          boost = std::max(boost, 1.0 + (params_.degeneracy_boost - 1.0) * align);
        }
        const double w =
            boost * std::exp(-0.5 * (e / kernel) * (e / kernel));
        const Eigen::Matrix<double, 1, 6> Jr = c.normal.transpose() * J;
        JtJ += w * Jr.transpose() * Jr;
        Jtb += w * Jr.transpose() * e;
        ++count;
      } else {
        // edge / non-planar: point-to-point。退化時は edge 重みを増幅して
        // 拘束不足方向を補償する (多メトリックの α 適応)。
        const Eigen::Vector3d d = src[i] - c.point;
        const double dn = d.norm();
        if (dn < 1e-9 || dn > kernel) continue;
        double alpha = params_.edge_weight;
        if (degenerate) alpha *= params_.degeneracy_boost;
        const double rw = std::exp(-0.5 * (dn / kernel) * (dn / kernel));
        const double w = alpha * rw;
        JtJ += w * J.transpose() * J;
        Jtb += w * J.transpose() * d;
        ++count;
      }
    }
    if (count < 10) break;
    H = JtJ;
    const Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) break;
    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;
    if (delta.norm() < params_.convergence_criterion) break;
  }
  if (H_out) *H_out = H;
  if (S_trans_out) *S_trans_out = S_trans;
  if (degenerate_out) *degenerate_out = degenerate;
  return T;
}

DammLoamResult DammLoamPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  DammLoamResult result;

  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg = voxelDownsample(downsampled, params_.voxel_size);
  if (reg.empty()) reg = downsampled;

  // クラス内訳 (診断用)。
  const auto labels = classifyScan(reg);
  for (auto l : labels) {
    switch (l) {
      case PointClass::kGround: result.counts.ground++; break;
      case PointClass::kRoof: result.counts.roof++; break;
      case PointClass::kWall: result.counts.wall++; break;
      case PointClass::kEdge: result.counts.edge++; break;
      case PointClass::kNonPlanar: result.counts.non_planar++; break;
      default: result.counts.unknown++; break;
    }
  }

  if (frame_count_ == 0) {
    local_map_.addPoints(transformPoints(downsampled, pose_));
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  const Eigen::Matrix4d prediction = pose_ * last_delta_;

  Eigen::Matrix<double, 6, 6> H;
  double S_trans = 1.0;
  bool degenerate = false;
  const Eigen::Matrix4d new_pose =
      runICP(reg, prediction, &H, &S_trans, &degenerate);

  last_delta_ = pose_.inverse() * new_pose;
  pose_ = new_pose;

  local_map_.addPoints(transformPoints(downsampled, pose_));
  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    local_map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.degeneracy_factor_trans = S_trans;
  result.degenerate = degenerate;
  result.converged = (pose_.array().isFinite().all() &&
                      (new_pose - prediction).cwiseAbs().maxCoeff() < 1e3);
  result.iterations = params_.max_icp_iterations;
  frame_count_++;
  return result;
}

}  // namespace damm_loam
}  // namespace localization_zoo
