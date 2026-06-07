#include "imls_slam/imls_slam.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace localization_zoo {
namespace imls_slam {

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
// ImlsModelMap
// ============================================================

void ImlsModelMap::addSamples(const std::vector<Eigen::Vector3d>& points,
                              const std::vector<Eigen::Vector3d>& normals) {
  const size_t n = std::min(points.size(), normals.size());
  for (size_t i = 0; i < n; i++) {
    auto key = toVoxel(points[i]);
    auto& vb = map_[key];
    if (static_cast<int>(vb.samples.size()) < max_points_)
      vb.samples.push_back({points[i], normals[i]});
  }
}

void ImlsModelMap::pruneFarVoxels(const Eigen::Vector3d& center,
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

ImlsModelMap::Neighbors ImlsModelMap::query(const Eigen::Vector3d& x,
                                            double support_radius) const {
  Neighbors nb;
  const double r_sq = support_radius * support_radius;
  // 支持半径をまたぐボクセル数を計算 (voxel_size より大きい半径も扱う)。
  const int span =
      std::max(1, static_cast<int>(std::ceil(support_radius / voxel_size_)));
  const auto key = toVoxel(x);
  double best = std::numeric_limits<double>::max();

  for (int dx = -span; dx <= span; dx++)
    for (int dy = -span; dy <= span; dy++)
      for (int dz = -span; dz <= span; dz++) {
        auto it = map_.find(key + Eigen::Vector3i(dx, dy, dz));
        if (it == map_.end()) continue;
        for (const auto& s : it->second.samples) {
          const double d = (s.point - x).squaredNorm();
          if (d < best) {
            best = d;
            nb.nearest_normal = s.normal;
          }
          if (d < r_sq) {
            nb.points.push_back(s.point);
            nb.normals.push_back(s.normal);
            nb.found = true;
          }
        }
      }
  nb.nearest_dist_sq = nb.found ? best : std::numeric_limits<double>::max();
  return nb;
}

// ============================================================
// ImlsSlamPipeline — 機構ヘルパ
// ============================================================

double ImlsSlamPipeline::imlsDistance(
    const Eigen::Vector3d& x, const std::vector<Eigen::Vector3d>& pts,
    const std::vector<Eigen::Vector3d>& normals, double h) {
  const size_t n = std::min(pts.size(), normals.size());
  if (n == 0) return 0.0;
  const double inv_h2 = 1.0 / std::max(h * h, 1e-12);
  double w_sum = 0.0;
  double num = 0.0;
  for (size_t i = 0; i < n; i++) {
    const Eigen::Vector3d d = x - pts[i];
    const double w = std::exp(-d.squaredNorm() * inv_h2);
    w_sum += w;
    num += w * d.dot(normals[i]);
  }
  if (w_sum <= 1e-12) return 0.0;
  return num / w_sum;
}

std::vector<int> ImlsSlamPipeline::selectObservabilityPoints(
    const std::vector<Eigen::Vector3d>& points,
    const std::vector<Eigen::Vector3d>& normals, int samples_per_axis) {
  const size_t n = std::min(points.size(), normals.size());
  std::vector<int> all(n);
  std::iota(all.begin(), all.end(), 0);
  if (samples_per_axis <= 0 || n <= static_cast<size_t>(6 * samples_per_axis))
    return all;

  // 各点の並進拘束 |n·e_a| と回転拘束 |(p×n)·e_a| を 6 リストで評価し、
  // 各リスト上位 samples_per_axis 点を集める (観測性ベースのサンプリング)。
  std::vector<char> selected(n, 0);
  std::vector<int> idx(n);

  auto take_top = [&](auto score) {
    std::iota(idx.begin(), idx.end(), 0);
    std::partial_sort(
        idx.begin(), idx.begin() + samples_per_axis, idx.end(),
        [&](int a, int b) { return score(a) > score(b); });
    for (int k = 0; k < samples_per_axis; k++) selected[idx[k]] = 1;
  };

  for (int axis = 0; axis < 3; axis++) {
    Eigen::Vector3d e = Eigen::Vector3d::Zero();
    e(axis) = 1.0;
    take_top([&](int i) { return std::abs(normals[i].dot(e)); });
    take_top([&](int i) {
      return std::abs(points[i].cross(normals[i]).dot(e));
    });
  }

  std::vector<int> out;
  out.reserve(n);
  for (size_t i = 0; i < n; i++)
    if (selected[i]) out.push_back(static_cast<int>(i));
  return out;
}

// ============================================================
// ImlsSlamPipeline — パイプライン
// ============================================================

ImlsSlamPipeline::ImlsSlamPipeline(const ImlsSlamParams& params)
    : params_(params),
      model_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> ImlsSlamPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> ImlsSlamPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

// センサ系のダウンサンプル点に PCA 法線を付ける (視点 = 原点に向けて符号統一)。
void ImlsSlamPipeline::computeNormals(const std::vector<Eigen::Vector3d>& pts,
                                      double radius,
                                      std::vector<Eigen::Vector3d>* normals,
                                      std::vector<char>* valid) const {
  normals->assign(pts.size(), Eigen::Vector3d::Zero());
  valid->assign(pts.size(), 0);

  // radius サイズのボクセルに点を入れ、3x3x3 近傍で PCA。
  std::unordered_map<Eigen::Vector3i, std::vector<int>, VoxelHash> grid;
  auto to_key = [&](const Eigen::Vector3d& p) {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / radius)),
                           static_cast<int>(std::floor(p.y() / radius)),
                           static_cast<int>(std::floor(p.z() / radius)));
  };
  for (int i = 0; i < static_cast<int>(pts.size()); i++)
    grid[to_key(pts[i])].push_back(i);

  const double r_sq = radius * radius;
  for (int i = 0; i < static_cast<int>(pts.size()); i++) {
    const auto& q = pts[i];
    const auto key = to_key(q);
    std::vector<Eigen::Vector3d> nbrs;
    nbrs.reserve(32);
    for (int dx = -1; dx <= 1; dx++)
      for (int dy = -1; dy <= 1; dy++)
        for (int dz = -1; dz <= 1; dz++) {
          auto it = grid.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == grid.end()) continue;
          for (int j : it->second)
            if ((pts[j] - q).squaredNorm() < r_sq) nbrs.push_back(pts[j]);
        }
    if (static_cast<int>(nbrs.size()) < params_.normal_min_neighbors) continue;

    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
    for (const auto& p : nbrs) centroid += p;
    centroid /= static_cast<double>(nbrs.size());
    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    for (const auto& p : nbrs) {
      const Eigen::Vector3d d = p - centroid;
      cov += d * d.transpose();
    }
    cov /= static_cast<double>(nbrs.size());
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
    if (solver.info() != Eigen::Success) continue;
    Eigen::Vector3d nrm = solver.eigenvectors().col(0).normalized();
    // 視点 (センサ原点) に向ける: n·(0 − p) > 0 となるよう符号を統一。
    if (nrm.dot(q) > 0.0) nrm = -nrm;
    (*normals)[i] = nrm;
    (*valid)[i] = 1;
  }
}

Eigen::Matrix4d ImlsSlamPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  return pose_ * delta_window_.back();
}

Eigen::Matrix4d ImlsSlamPipeline::runRegistration(
    const std::vector<Eigen::Vector3d>& source,
    const std::vector<int>& sample_idx, const Eigen::Matrix4d& base,
    ImlsSlamResult* result) {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;

  const double support = params_.imls_support_radius > 0.0
                             ? params_.imls_support_radius
                             : 3.0 * params_.imls_h;
  const double gate_sq =
      params_.max_correspondence_dist * params_.max_correspondence_dist;

  Eigen::Matrix4d T = base;
  int last_corr = 0;
  double mean_abs = 0.0;

  for (int it = 0; it < params_.max_iterations; it++) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);

    Mat6 A = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    double abs_sum = 0.0;
    int used = 0;

    for (int idx : sample_idx) {
      const Eigen::Vector3d x = R * source[idx] + t;  // world 系
      const auto nb = model_.query(x, support);
      if (!nb.found || nb.nearest_dist_sq > gate_sq) continue;

      // 暗黙的曲面距離 (符号つき) と最近傍点の法線 (投影方向)。
      const double e = imlsDistance(x, nb.points, nb.normals, params_.imls_h);
      const Eigen::Vector3d n = nb.nearest_normal;
      if (n.squaredNorm() < 0.5) continue;  // 法線無し

      // point-to-implicit-surface: 残差 e を法線 n 方向の point-to-plane で線形化。
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(x);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const Eigen::Matrix<double, 1, 6> Jr = n.transpose() * J;
      A += Jr.transpose() * Jr;
      b -= Jr.transpose() * e;
      abs_sum += std::abs(e);
      used++;
    }
    if (used < 10) break;

    const Vec6 d = A.ldlt().solve(b);
    if (!d.allFinite()) break;
    T = expSE3(d) * T;
    last_corr = used;
    mean_abs = abs_sum / static_cast<double>(used);
    if (result) result->iterations = it + 1;

    if (d.norm() < params_.convergence_criterion && it > 2) break;
  }

  if (result) {
    result->num_correspondences = last_corr;
    result->mean_abs_residual = mean_abs;
  }
  return T;
}

ImlsSlamResult ImlsSlamPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  ImlsSlamResult result;

  const double nr =
      params_.normal_radius > 0.0 ? params_.normal_radius : params_.voxel_size * 1.5;

  auto filtered = rangeFilter(frame);
  // モデル用 (密) とレジストレーション用 (疎) を分けて down-sample する。
  auto map_ds = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg_ds = voxelDownsample(filtered, params_.voxel_size);
  if (reg_ds.empty()) reg_ds = map_ds;

  // モデル用有向点 (センサ系法線)。
  std::vector<Eigen::Vector3d> map_normals;
  std::vector<char> map_valid;
  computeNormals(map_ds, nr, &map_normals, &map_valid);

  auto add_to_model = [&](const Eigen::Matrix4d& pose) {
    const Eigen::Matrix3d R = pose.block<3, 3>(0, 0);
    std::vector<Eigen::Vector3d> wp, wn;
    wp.reserve(map_ds.size());
    wn.reserve(map_ds.size());
    for (size_t i = 0; i < map_ds.size(); i++) {
      if (!map_valid[i]) continue;
      wp.push_back(R * map_ds[i] + pose.block<3, 1>(0, 3));
      wn.push_back(R * map_normals[i]);
    }
    model_.addSamples(wp, wn);
  };

  if (frame_count_ == 0) {
    add_to_model(pose_);
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  // レジストレーション用有向点 + 観測性サンプリング。
  std::vector<Eigen::Vector3d> reg_normals;
  std::vector<char> reg_valid;
  computeNormals(reg_ds, nr, &reg_normals, &reg_valid);

  std::vector<Eigen::Vector3d> src;
  std::vector<Eigen::Vector3d> src_n;
  src.reserve(reg_ds.size());
  src_n.reserve(reg_ds.size());
  for (size_t i = 0; i < reg_ds.size(); i++) {
    if (!reg_valid[i]) continue;
    src.push_back(reg_ds[i]);
    src_n.push_back(reg_normals[i]);
  }

  std::vector<int> sample_idx;
  if (params_.use_observability_sampling) {
    sample_idx = selectObservabilityPoints(src, src_n, params_.samples_per_axis);
  } else {
    sample_idx.resize(src.size());
    std::iota(sample_idx.begin(), sample_idx.end(), 0);
  }
  result.num_samples = static_cast<int>(sample_idx.size());

  const Eigen::Matrix4d basep = predict();
  const Eigen::Matrix4d new_pose =
      runRegistration(src, sample_idx, basep, &result);

  const Eigen::Matrix4d delta = pose_.inverse() * new_pose;
  delta_window_.push_back(delta);
  if (static_cast<int>(delta_window_.size()) > 2) delta_window_.pop_front();
  pose_ = new_pose;

  add_to_model(pose_);
  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    model_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.converged = (pose_.array().isFinite().all() &&
                      (new_pose - basep).cwiseAbs().maxCoeff() < 1e3);
  frame_count_++;
  return result;
}

}  // namespace imls_slam
}  // namespace localization_zoo
