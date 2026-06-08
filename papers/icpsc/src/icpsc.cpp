#include "icpsc/icpsc.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace icpsc {

namespace {

int pixelIndex(int u, int v, int width) { return v * width + u; }

bool inImage(int u, int v, int width, int height) {
  return u >= 0 && u < width && v >= 0 && v < height;
}

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

std::vector<PointI> transformPoints(const std::vector<PointI>& pts,
                                    const Eigen::Matrix4d& T) {
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  std::vector<PointI> out(pts.size());
  for (size_t i = 0; i < pts.size(); i++) {
    out[i].p = R * pts[i].p + t;
    out[i].intensity = pts[i].intensity;
  }
  return out;
}

Eigen::MatrixXd circularShiftColumns(const Eigen::MatrixXd& matrix, int shift) {
  if (matrix.cols() == 0) return matrix;
  shift = ((shift % matrix.cols()) + matrix.cols()) % matrix.cols();
  if (shift == 0) return matrix;
  Eigen::MatrixXd out(matrix.rows(), matrix.cols());
  out << matrix.rightCols(matrix.cols() - shift),
      matrix.leftCols(shift);
  return out;
}

}  // namespace

// ============================================================
// IcpscCore
// ============================================================

CylindricalImage IcpscCore::projectCylindrical(const std::vector<PointI>& points,
                                              const CylindricalParams& params) {
  CylindricalImage img;
  img.params = params;
  const size_t n = static_cast<size_t>(params.width * params.height);
  img.range.assign(n, 0.0f);
  img.intensity.assign(n, 0.0f);
  img.count.assign(n, 0);
  img.points.assign(n, Eigen::Vector3d::Zero());

  const double theta = (params.theta_down_deg + params.theta_up_deg) * M_PI / 180.0;
  const double theta_d = params.theta_down_deg * M_PI / 180.0;

  for (const auto& pt : points) {
    const double x = pt.p.x();
    const double y = pt.p.y();
    const double z = pt.p.z();
    const double r = std::sqrt(x * x + y * y + z * z);
    if (r < params.min_range || r > params.max_range) continue;

    const double u_f = (1.0 - std::atan2(y, x) / M_PI) * 0.5 * params.width;
    const double v_f = (std::asin(z / r) + theta_d) / theta * params.height;
    const int u = static_cast<int>(std::floor(u_f));
    const int v = static_cast<int>(std::floor(v_f));
    if (!inImage(u, v, params.width, params.height)) continue;

    const int idx = pixelIndex(u, v, params.width);
    img.count[idx]++;
    if (img.range[idx] == 0.0f || r < img.range[idx]) {
      img.range[idx] = static_cast<float>(r);
      img.intensity[idx] = pt.intensity;
      img.points[idx] = pt.p;
    } else {
      img.intensity[idx] =
          0.5f * (img.intensity[idx] + pt.intensity);
    }
  }
  return img;
}

IcpscDescriptor IcpscCore::buildDescriptor(const CylindricalImage& image,
                                           int num_rings, int num_sectors) {
  IcpscDescriptor desc;
  desc.intensity_map = Eigen::MatrixXd::Zero(num_rings, num_sectors);
  desc.density_map = Eigen::MatrixXd::Zero(num_rings, num_sectors);

  const int w = image.params.width;
  const int h = image.params.height;
  for (int ring = 0; ring < num_rings; ring++) {
    const int v0 = ring * h / num_rings;
    const int v1 = (ring + 1) * h / num_rings;
    for (int sector = 0; sector < num_sectors; sector++) {
      const int u0 = sector * w / num_sectors;
      const int u1 = (sector + 1) * w / num_sectors;
      double i_sum = 0.0;
      double cnt = 0.0;
      for (int v = v0; v < v1; v++) {
        for (int u = u0; u < u1; u++) {
          const int idx = pixelIndex(u, v, w);
          if (image.range[idx] <= 0.0f) continue;
          i_sum += image.intensity[idx];
          cnt += static_cast<double>(std::max<uint16_t>(image.count[idx], 1));
        }
      }
      if (cnt > 0.0) {
        desc.intensity_map(ring, sector) = i_sum / cnt;
        desc.density_map(ring, sector) = std::log1p(cnt);
      }
    }
  }

  desc.ring_key = Eigen::VectorXd(num_rings);
  desc.sector_key = Eigen::VectorXd(num_sectors);
  for (int r = 0; r < num_rings; r++) {
    desc.ring_key(r) = desc.intensity_map.row(r).mean() + desc.density_map.row(r).mean();
  }
  for (int s = 0; s < num_sectors; s++) {
    desc.sector_key(s) =
        desc.intensity_map.col(s).mean() + desc.density_map.col(s).mean();
  }
  return desc;
}

double IcpscCore::descriptorDistance(const IcpscDescriptor& a,
                                     const IcpscDescriptor& b) {
  if (a.intensity_map.size() == 0 || b.intensity_map.size() == 0) return 1.0;
  int best_shift = 0;
  double best = std::numeric_limits<double>::infinity();
  const int sectors = static_cast<int>(a.intensity_map.cols());
  for (int shift = 0; shift < sectors; shift++) {
    const Eigen::MatrixXd shifted = circularShiftColumns(b.intensity_map, shift);
    const Eigen::MatrixXd dshift = circularShiftColumns(b.density_map, shift);
    const Eigen::MatrixXd diff_i = a.intensity_map - shifted;
    const Eigen::MatrixXd diff_d = a.density_map - dshift;
    const double dist =
        diff_i.squaredNorm() / static_cast<double>(diff_i.size()) +
        diff_d.squaredNorm() / static_cast<double>(diff_d.size());
    if (dist < best) {
      best = dist;
      best_shift = shift;
    }
  }
  (void)best_shift;
  return std::sqrt(std::max(best, 0.0));
}

double IcpscCore::adaptiveGeomWeight(size_t num_geom, size_t num_intensity,
                                   double alpha, double floor_w, double ceil_w) {
  const double denom =
      static_cast<double>(num_geom) + alpha * static_cast<double>(num_intensity);
  if (denom < 1e-6) return 0.5;
  const double w = static_cast<double>(num_geom) / denom;
  return std::clamp(w, floor_w, ceil_w);
}

// ============================================================
// VoxelHashMap
// ============================================================

void VoxelHashMap::clear() { map_.clear(); }

void VoxelHashMap::addPoints(const std::vector<PointI>& points) {
  for (const auto& pt : points) {
    auto& vb = map_[toVoxel(pt.p)];
    if (static_cast<int>(vb.points.size()) < max_points_) {
      vb.points.push_back(pt.p);
      vb.intensity.push_back(pt.intensity);
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
    const std::vector<PointI>& queries, double max_dist,
    int normal_min_neighbors) const {
  std::vector<Correspondence> correspondences(queries.size());
  const double max_dist_sq = max_dist * max_dist;

  for (size_t i = 0; i < queries.size(); i++) {
    const auto& query = queries[i];
    auto key = toVoxel(query.p);
    bool found = false;
    std::vector<Eigen::Vector3d> neighbors;
    std::vector<double> neighbor_intensity;
    neighbors.reserve(32);
    neighbor_intensity.reserve(32);

    for (int dx = -1; dx <= 1; dx++)
      for (int dy = -1; dy <= 1; dy++)
        for (int dz = -1; dz <= 1; dz++) {
          auto it = map_.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == map_.end()) continue;
          for (size_t j = 0; j < it->second.points.size(); j++) {
            const double d = (it->second.points[j] - query.p).squaredNorm();
            if (d < max_dist_sq) {
              neighbors.push_back(it->second.points[j]);
              neighbor_intensity.push_back(it->second.intensity[j]);
              found = true;
            }
          }
        }

    Correspondence& c = correspondences[i];
    c.found = found;
    if (!found || static_cast<int>(neighbors.size()) < normal_min_neighbors) {
      continue;
    }

    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
    double mean_i = 0.0;
    for (size_t j = 0; j < neighbors.size(); j++) {
      centroid += neighbors[j];
      mean_i += neighbor_intensity[j];
    }
    centroid /= static_cast<double>(neighbors.size());
    mean_i /= static_cast<double>(neighbors.size());

    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    for (const auto& n : neighbors) {
      const Eigen::Vector3d d = n - centroid;
      cov += d * d.transpose();
    }
    cov /= static_cast<double>(neighbors.size());

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
    if (solver.info() != Eigen::Success) continue;

    const Eigen::Vector3d ev = solver.eigenvalues();
    const double lambda2 = std::max(ev(2), 1e-12);
    c.planarity = std::clamp((ev(1) - ev(0)) / lambda2, 0.0, 1.0);
    c.normal = solver.eigenvectors().col(0).normalized();
    c.anchor = centroid;
    c.mean_intensity = mean_i;
    c.has_normal = true;
  }
  return correspondences;
}

// ============================================================
// IcpscPipeline
// ============================================================

IcpscPipeline::IcpscPipeline(const IcpscParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

IcpscPipeline::Features IcpscPipeline::extractFeatures(
    const CylindricalImage& image) const {
  Features out;
  const int w = image.params.width;
  const int h = image.params.height;

  for (int v = 1; v < h - 1; v++) {
    for (int u = 1; u < w - 1; u++) {
      const int idx = pixelIndex(u, v, w);
      if (image.range[idx] <= 0.0f) continue;
      const double center = image.range[idx];
      double diff = 0.0;
      int cnt = 0;
      for (int dv = -1; dv <= 1; dv++) {
        for (int du = -1; du <= 1; du++) {
          if (du == 0 && dv == 0) continue;
          const int j = pixelIndex(u + du, v + dv, w);
          if (image.range[j] <= 0.0f) continue;
          diff += std::abs(center - image.range[j]);
          cnt++;
        }
      }
      if (cnt == 0) continue;
      const double curv = diff / (cnt * center);
      PointI pt;
      pt.p = image.points[idx];
      pt.intensity = image.intensity[idx];

      if (curv > params_.edge_curvature_thresh) {
        if (static_cast<int>(out.edges.size()) < params_.max_edge_features) {
          out.edges.push_back(pt);
        }
      } else if (curv < params_.geom_residual_thresh) {
        if (static_cast<int>(out.surfs.size()) < params_.max_surf_features) {
          out.surfs.push_back(pt);
        }
      }

      if (!params_.enable_intensity) continue;
      float grad = 0.0f;
      int gc = 0;
      for (int dv = -1; dv <= 1; dv++) {
        for (int du = -1; du <= 1; du++) {
          if (du == 0 && dv == 0) continue;
          const int j = pixelIndex(u + du, v + dv, w);
          if (image.range[j] <= 0.0f) continue;
          grad += std::abs(image.intensity[idx] - image.intensity[j]);
          gc++;
        }
      }
      if (gc > 0 &&
          grad / static_cast<float>(gc) > params_.intensity_grad_thresh) {
        if (static_cast<int>(out.intensity_pts.size()) <
            params_.max_intensity_features) {
          out.intensity_pts.push_back(pt);
        }
      }
    }
  }
  return out;
}

std::vector<PointI> IcpscPipeline::voxelDownsample(
    const std::vector<PointI>& points, double voxel_size) const {
  std::unordered_map<Eigen::Vector3i, PointI, VoxelHash> grid;
  for (const auto& pt : points) {
    Eigen::Vector3i key(static_cast<int>(std::floor(pt.p.x() / voxel_size)),
                        static_cast<int>(std::floor(pt.p.y() / voxel_size)),
                        static_cast<int>(std::floor(pt.p.z() / voxel_size)));
    grid.emplace(key, pt);
  }
  std::vector<PointI> out;
  out.reserve(grid.size());
  for (const auto& kv : grid) out.push_back(kv.second);
  return out;
}

Eigen::Matrix4d IcpscPipeline::predict() const {
  if (delta_window_.empty()) return pose_;
  return pose_ * delta_window_.back();
}

double IcpscPipeline::meanAbsResidual(const Features& features,
                                      const Eigen::Matrix4d& pose) const {
  std::vector<PointI> source;
  source.reserve(features.edges.size() + features.surfs.size() +
                 features.intensity_pts.size());
  for (const auto& p : features.surfs) source.push_back(p);
  for (const auto& p : features.edges) source.push_back(p);
  for (const auto& p : features.intensity_pts) source.push_back(p);
  if (source.empty()) return std::numeric_limits<double>::infinity();

  const auto transformed = transformPoints(source, pose);
  const auto corr = local_map_.getCorrespondences(
      transformed, params_.max_correspondence_dist,
      params_.normal_min_neighbors);

  double sum = 0.0;
  int used = 0;
  for (size_t k = 0; k < source.size(); k++) {
    const auto& c = corr[k];
    if (!c.found || !c.has_normal ||
        c.planarity < params_.planarity_threshold) {
      continue;
    }
    const double e = c.normal.dot(transformed[k].p - c.anchor);
    if (std::abs(e) > params_.initial_threshold) continue;
    sum += std::abs(e);
    ++used;
  }
  return used > 0 ? sum / static_cast<double>(used)
                  : std::numeric_limits<double>::infinity();
}

Eigen::Matrix4d IcpscPipeline::runRegistration(const Features& features,
                                               const Eigen::Matrix4d& base,
                                               IcpscResult* result) {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;

  std::vector<PointI> source;
  source.reserve(features.edges.size() + features.surfs.size() +
                 features.intensity_pts.size());
  for (const auto& p : features.surfs) source.push_back(p);
  for (const auto& p : features.edges) source.push_back(p);
  for (const auto& p : features.intensity_pts) source.push_back(p);
  if (source.empty()) return base;

  const double w_geom = IcpscCore::adaptiveGeomWeight(
      features.edges.size() + features.surfs.size(),
      features.intensity_pts.size(), params_.adaptive_alpha,
      params_.geom_weight_floor, params_.geom_weight_ceil);
  const double w_int = params_.enable_intensity ? (1.0 - w_geom) : 0.0;
  if (result) {
    result->geom_weight = w_geom;
    result->num_geom_factors = static_cast<int>(features.edges.size() +
                                                features.surfs.size());
    result->num_intensity_factors = static_cast<int>(features.intensity_pts.size());
  }

  Eigen::Matrix4d T = base;
  int last_used = 0;
  for (int it = 0; it < params_.max_iterations; it++) {
    const auto transformed = transformPoints(source, T);
    const auto corr = local_map_.getCorrespondences(
        transformed, params_.max_correspondence_dist,
        params_.normal_min_neighbors);

    Mat6 H = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    int used = 0;

    for (size_t k = 0; k < source.size(); k++) {
      const auto& c = corr[k];
      if (!c.found || !c.has_normal ||
          c.planarity < params_.planarity_threshold) {
        continue;
      }
      const double e = c.normal.dot(transformed[k].p - c.anchor);
      if (std::abs(e) > params_.initial_threshold) continue;

      const bool is_intensity_only =
          k >= features.surfs.size() + features.edges.size();
      double w = is_intensity_only ? w_int : w_geom;
      if (params_.enable_intensity) {
        const double di = transformed[k].intensity - c.mean_intensity;
        const double iw = std::exp(-0.5 * (di / params_.intensity_sigma) *
                                   (di / params_.intensity_sigma));
        w *= iw;
      }
      if (w < 1e-4) continue;

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(transformed[k].p);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const Eigen::Matrix<double, 1, 6> Jr = c.normal.transpose() * J;
      H += w * Jr.transpose() * Jr;
      b -= w * Jr.transpose() * e;
      ++used;
    }

    if (used < 6) break;
    const Vec6 delta = H.ldlt().solve(b);
    if (!delta.allFinite()) break;
    T = expSE3(delta) * T;
    last_used = used;
    if (result) result->iterations = it + 1;
    if (delta.norm() < params_.convergence_criterion) break;
  }

  if (result) result->num_geom_factors = last_used;
  return T;
}

IcpscResult IcpscPipeline::registerFrame(const std::vector<PointI>& frame) {
  IcpscResult result;
  const auto cyl =
      IcpscCore::projectCylindrical(frame, params_.cylindrical);
  const auto desc = IcpscCore::buildDescriptor(
      cyl, params_.num_rings, params_.num_sectors);
  if (has_last_descriptor_) {
    result.descriptor_distance =
        IcpscCore::descriptorDistance(last_descriptor_, desc);
  }
  last_descriptor_ = desc;
  has_last_descriptor_ = true;

  auto features = extractFeatures(cyl);
  const auto dense_geom =
      voxelDownsample(frame, params_.voxel_size * 0.5);
  if (!dense_geom.empty()) {
    features.surfs = dense_geom;
  } else if (features.surfs.size() + features.edges.size() < 50) {
    const int w = cyl.params.width;
    const int h = cyl.params.height;
    for (int v = 0; v < h; v += 2) {
      for (int u = 0; u < w; u += 2) {
        const int idx = pixelIndex(u, v, w);
        if (cyl.range[idx] <= 0.0f) continue;
        PointI pt;
        pt.p = cyl.points[idx];
        pt.intensity = cyl.intensity[idx];
        features.surfs.push_back(pt);
        if (static_cast<int>(features.surfs.size()) >= 500) break;
      }
    }
  }
  const auto map_points = voxelDownsample(frame, params_.voxel_size);

  if (frame_count_ == 0) {
    local_map_.addPoints(transformPoints(map_points, pose_));
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  const Eigen::Matrix4d guess_pose = pose_;
  const Eigen::Matrix4d guess_motion =
      delta_window_.size() >= 2 ? predict() : pose_;
  Eigen::Matrix4d new_pose =
      runRegistration(features, guess_motion, &result);
  const Eigen::Matrix4d alt_pose =
      runRegistration(features, guess_pose, nullptr);
  if (meanAbsResidual(features, alt_pose) <
      meanAbsResidual(features, new_pose)) {
    new_pose = alt_pose;
  }

  const Eigen::Matrix4d delta = pose_.inverse() * new_pose;
  delta_window_.push_back(delta);
  if (static_cast<int>(delta_window_.size()) > 2) delta_window_.pop_front();
  pose_ = new_pose;

  local_map_.addPoints(transformPoints(map_points, pose_));
  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    local_map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.converged = pose_.array().isFinite().all();
  frame_count_++;
  return result;
}

}  // namespace icpsc
}  // namespace localization_zoo
