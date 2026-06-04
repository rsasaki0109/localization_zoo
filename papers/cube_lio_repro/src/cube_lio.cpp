#include "cube_lio/cube_lio.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace cube_lio {

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
// CubeMap
// ============================================================

void CubeMap::project(const Eigen::Vector3d& d, int& face, double& u,
                      double& v) const {
  face = -1;
  const double ax = std::abs(d.x()), ay = std::abs(d.y()), az = std::abs(d.z());
  double sc, tc, ma;
  // OpenGL 風キューブマップ面割り当て。face: 0:+X 1:-X 2:+Y 3:-Y 4:+Z 5:-Z
  if (ax >= ay && ax >= az) {
    ma = ax;
    if (d.x() > 0) { face = 0; sc = -d.z(); tc = -d.y(); }
    else { face = 1; sc = d.z(); tc = -d.y(); }
  } else if (ay >= ax && ay >= az) {
    ma = ay;
    if (d.y() > 0) { face = 2; sc = d.x(); tc = d.z(); }
    else { face = 3; sc = d.x(); tc = -d.z(); }
  } else {
    ma = az;
    if (d.z() > 0) { face = 4; sc = d.x(); tc = -d.y(); }
    else { face = 5; sc = -d.x(); tc = -d.y(); }
  }
  if (ma < 1e-9) { face = -1; return; }
  u = (sc / ma + 1.0) * 0.5 * (size_ - 1);
  v = (tc / ma + 1.0) * 0.5 * (size_ - 1);
}

void CubeMap::build(const std::vector<IntensityPoint>& points) {
  const int n = size_ * size_;
  for (int f = 0; f < 6; f++) {
    intensity_[f].assign(n, 0.0f);
    igm_[f].assign(n, 0.0f);
  }

  // 各セルに最大強度をラスタ化 (疎点群)。
  for (const auto& ip : points) {
    const double r = ip.p.norm();
    if (r < 1e-6) continue;
    int face;
    double u, v;
    project(ip.p / r, face, u, v);
    if (face < 0) continue;
    const int iu = static_cast<int>(std::lround(u));
    const int iv = static_cast<int>(std::lround(v));
    if (iu < 0 || iu >= size_ || iv < 0 || iv >= size_) continue;
    float& cell = intensity_[face][iv * size_ + iu];
    cell = std::max(cell, static_cast<float>(ip.intensity));
  }

  // ガウシアン平滑化 (分離可能な 3-tap 近似を sigma 回数ぶん適用)。
  const int passes = std::max(1, static_cast<int>(std::lround(gaussian_sigma_)));
  std::vector<float> tmp(n);
  for (int f = 0; f < 6; f++) {
    auto& img = intensity_[f];
    for (int pass = 0; pass < passes; pass++) {
      // 水平
      for (int y = 0; y < size_; y++)
        for (int x = 0; x < size_; x++) {
          const float c = img[y * size_ + x];
          const float l = img[y * size_ + std::max(0, x - 1)];
          const float r = img[y * size_ + std::min(size_ - 1, x + 1)];
          tmp[y * size_ + x] = 0.25f * l + 0.5f * c + 0.25f * r;
        }
      // 垂直
      for (int y = 0; y < size_; y++)
        for (int x = 0; x < size_; x++) {
          const float c = tmp[y * size_ + x];
          const float u = tmp[std::max(0, y - 1) * size_ + x];
          const float d = tmp[std::min(size_ - 1, y + 1) * size_ + x];
          img[y * size_ + x] = 0.25f * u + 0.5f * c + 0.25f * d;
        }
    }

    // Sobel 勾配ノルム (IGM)。
    auto& igm = igm_[f];
    auto at = [&](int x, int y) {
      x = std::clamp(x, 0, size_ - 1);
      y = std::clamp(y, 0, size_ - 1);
      return img[y * size_ + x];
    };
    for (int y = 0; y < size_; y++)
      for (int x = 0; x < size_; x++) {
        const float gx = (at(x + 1, y - 1) + 2 * at(x + 1, y) + at(x + 1, y + 1)) -
                         (at(x - 1, y - 1) + 2 * at(x - 1, y) + at(x - 1, y + 1));
        const float gy = (at(x - 1, y + 1) + 2 * at(x, y + 1) + at(x + 1, y + 1)) -
                         (at(x - 1, y - 1) + 2 * at(x, y - 1) + at(x + 1, y - 1));
        igm[y * size_ + x] = std::sqrt(gx * gx + gy * gy);
      }
  }
}

double CubeMap::sampleIGM(const Eigen::Vector3d& p_sensor) const {
  const double r = p_sensor.norm();
  if (r < 1e-6) return 0.0;
  int face;
  double u, v;
  project(p_sensor / r, face, u, v);
  if (face < 0) return 0.0;
  const int iu = static_cast<int>(std::lround(u));
  const int iv = static_cast<int>(std::lround(v));
  if (iu < 0 || iu >= size_ || iv < 0 || iv >= size_) return 0.0;
  return igm_[face][iv * size_ + iu];
}

// ============================================================
// VoxelHashMap
// ============================================================

void VoxelHashMap::addPoints(const std::vector<IntensityPoint>& points) {
  for (const auto& ip : points) {
    auto key = toVoxel(ip.p);
    auto& vb = map_[key];
    if (static_cast<int>(vb.points.size()) < max_points_) {
      vb.points.push_back(ip.p);
      vb.intensities.push_back(ip.intensity);
    }
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
    double best_intensity = 0.0;
    bool found = false;

    std::vector<Eigen::Vector3d> neighbors;
    std::vector<double> neighbor_intensity;
    neighbors.reserve(32);

    for (int dx = -1; dx <= 1; dx++)
      for (int dy = -1; dy <= 1; dy++)
        for (int dz = -1; dz <= 1; dz++) {
          auto it = map_.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == map_.end()) continue;
          for (size_t k = 0; k < it->second.points.size(); k++) {
            const auto& mp = it->second.points[k];
            const double d = (mp - query).squaredNorm();
            if (d < max_dist_sq) {
              neighbors.push_back(mp);
              neighbor_intensity.push_back(it->second.intensities[k]);
            }
            if (d < best_dist) {
              best_dist = d;
              best_point = mp;
              best_intensity = it->second.intensities[k];
              found = true;
            }
          }
        }

    Correspondence& c = correspondences[i];
    c.point = best_point;
    c.intensity = best_intensity;
    c.found = found;
    if (!found) continue;

    if (static_cast<int>(neighbors.size()) >= normal_min_neighbors) {
      Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
      double mean_int = 0.0;
      for (size_t k = 0; k < neighbors.size(); k++) {
        centroid += neighbors[k];
        mean_int += neighbor_intensity[k];
      }
      centroid /= static_cast<double>(neighbors.size());
      mean_int /= static_cast<double>(neighbors.size());

      Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
      for (const auto& nb : neighbors) {
        const Eigen::Vector3d d = nb - centroid;
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
        c.intensity = mean_int;
        c.has_normal = true;
      }
    }
  }
  return correspondences;
}

// ============================================================
// CubeLIO Pipeline
// ============================================================

CubeLIOPipeline::CubeLIOPipeline(const CubeLIOParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<IntensityPoint> CubeLIOPipeline::rangeFilter(
    const std::vector<IntensityPoint>& points) const {
  std::vector<IntensityPoint> out;
  out.reserve(points.size());
  for (const auto& ip : points) {
    const double r = ip.p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(ip);
  }
  return out;
}

std::vector<IntensityPoint> CubeLIOPipeline::voxelDownsample(
    const std::vector<IntensityPoint>& points, double voxel_size) const {
  std::unordered_map<Eigen::Vector3i, IntensityPoint, VoxelHash> grid;
  for (const auto& ip : points) {
    Eigen::Vector3i key(static_cast<int>(std::floor(ip.p.x() / voxel_size)),
                        static_cast<int>(std::floor(ip.p.y() / voxel_size)),
                        static_cast<int>(std::floor(ip.p.z() / voxel_size)));
    grid.emplace(key, ip);
  }
  std::vector<IntensityPoint> out;
  out.reserve(grid.size());
  for (const auto& kv : grid) out.push_back(kv.second);
  return out;
}

std::vector<IntensityPoint> CubeLIOPipeline::selectSemiDense(
    const std::vector<IntensityPoint>& points, const CubeMap& cube) const {
  if (points.size() < static_cast<size_t>(params_.min_semi_dense_points))
    return points;

  std::vector<double> igm(points.size());
  for (size_t i = 0; i < points.size(); i++)
    igm[i] = cube.sampleIGM(points[i].p);

  std::vector<double> sorted = igm;
  std::sort(sorted.begin(), sorted.end());
  const double q = std::clamp(params_.semi_dense_quantile, 0.0, 0.95);
  const double threshold = sorted[static_cast<size_t>(q * (sorted.size() - 1))];

  std::vector<IntensityPoint> out;
  out.reserve(points.size());
  for (size_t i = 0; i < points.size(); i++)
    if (igm[i] >= threshold) out.push_back(points[i]);

  if (out.size() < static_cast<size_t>(params_.min_semi_dense_points))
    return points;
  return out;
}

Eigen::Matrix4d CubeLIOPipeline::runICP(
    const std::vector<IntensityPoint>& source,
    const Eigen::Matrix4d& initial_guess, double max_correspondence_dist) {
  Eigen::Matrix4d T = initial_guess;
  const double kernel = max_correspondence_dist;
  const double int_sigma_sq =
      std::max(params_.intensity_sigma * params_.intensity_sigma, 1e-6);

  std::vector<Eigen::Vector3d> src_pts(source.size());
  for (size_t i = 0; i < source.size(); i++) src_pts[i] = source[i].p;

  for (int iter = 0; iter < params_.max_icp_iterations; iter++) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);
    std::vector<Eigen::Vector3d> src_world(source.size());
    for (size_t i = 0; i < source.size(); i++)
      src_world[i] = R * src_pts[i] + t;

    const auto corr = local_map_.getCorrespondences(
        src_world, max_correspondence_dist, params_.normal_min_neighbors);

    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    int count = 0;

    for (size_t i = 0; i < source.size(); i++) {
      const auto& c = corr[i];
      if (!c.found) continue;
      const Eigen::Vector3d e = src_world[i] - c.point;
      const double r_norm = e.norm();
      if (r_norm > max_correspondence_dist) continue;

      // 幾何カーネル × 強度整合性 (反射強度の一致で重み付け = 測光的拘束の近似)。
      const double w_geo = std::exp(-0.5 * (r_norm / kernel) * (r_norm / kernel));
      const double di = source[i].intensity - c.intensity;
      const double w_int = std::exp(-0.5 * di * di / int_sigma_sq);
      const double w = w_geo * w_int;

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(src_world[i]);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();

      if (c.has_normal && c.planarity >= params_.planarity_threshold) {
        const Eigen::Matrix<double, 1, 6> Jr = c.normal.transpose() * J;
        const double rp = c.normal.dot(src_world[i] - c.anchor);
        JtJ += w * Jr.transpose() * Jr;
        Jtb += w * Jr.transpose() * rp;
      } else {
        JtJ += w * J.transpose() * J;
        Jtb += w * J.transpose() * e;
      }
      ++count;
    }

    if (count < 10) break;
    const Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) break;

    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;
    if (delta.norm() < params_.convergence_criterion) break;
  }
  return T;
}

CubeLIOResult CubeLIOPipeline::registerFrame(
    const std::vector<IntensityPoint>& frame) {
  CubeLIOResult result;

  auto filtered = rangeFilter(frame);

  // キューブマップ IGM はフル解像度のフィルタ済み点で構築する。
  CubeMap cube(params_.cubemap_size, params_.gaussian_sigma);
  cube.build(filtered);

  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg_full = voxelDownsample(downsampled, params_.voxel_size);
  if (reg_full.empty()) reg_full = downsampled;

  // semi-dense 特徴 (IGM 上位) を登録対象に選ぶ。
  auto registration_points = selectSemiDense(reg_full, cube);
  result.semi_dense_points = static_cast<int>(registration_points.size());

  if (frame_count_ == 0) {
    Eigen::Matrix3d R = pose_.block<3, 3>(0, 0);
    Eigen::Vector3d t = pose_.block<3, 1>(0, 3);
    std::vector<IntensityPoint> world(downsampled.size());
    for (size_t i = 0; i < downsampled.size(); i++) {
      world[i].p = R * downsampled[i].p + t;
      world[i].intensity = downsampled[i].intensity;
    }
    local_map_.addPoints(world);
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  const Eigen::Matrix4d prediction = pose_ * last_delta_;
  const Eigen::Matrix4d new_pose =
      runICP(registration_points, prediction, params_.initial_threshold);

  last_delta_ = pose_.inverse() * new_pose;
  pose_ = new_pose;

  Eigen::Matrix3d R = pose_.block<3, 3>(0, 0);
  Eigen::Vector3d t = pose_.block<3, 1>(0, 3);
  std::vector<IntensityPoint> world(downsampled.size());
  for (size_t i = 0; i < downsampled.size(); i++) {
    world[i].p = R * downsampled[i].p + t;
    world[i].intensity = downsampled[i].intensity;
  }
  local_map_.addPoints(world);
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

}  // namespace cube_lio
}  // namespace localization_zoo
