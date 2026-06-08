#include "pg_lio/pg_lio.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace pg_lio {

namespace {

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0, -v.z(), v.y(), v.z(), 0, -v.x(), -v.y(), v.x(), 0;
  return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& w) {
  const double t = w.norm();
  if (t < 1e-10) return Eigen::Matrix3d::Identity() + skew(w);
  const Eigen::Matrix3d K = skew(w / t);
  return Eigen::Matrix3d::Identity() + std::sin(t) * K +
         (1.0 - std::cos(t)) * K * K;
}

std::vector<Eigen::Vector3d> transformPoints(
    const std::vector<Eigen::Vector3d>& pts, const Eigen::Matrix4d& T) {
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d tr = T.block<3, 1>(0, 3);
  std::vector<Eigen::Vector3d> out(pts.size());
  for (size_t i = 0; i < pts.size(); ++i) out[i] = R * pts[i] + tr;
  return out;
}

void normalizePatch(const std::vector<float>& raw, std::vector<float>* out,
                    double* mean, double* sigma) {
  out->clear();
  if (raw.empty()) {
    *mean = 0.0;
    *sigma = 1.0;
    return;
  }
  double m = 0.0;
  for (float v : raw) m += v;
  m /= static_cast<double>(raw.size());
  double var = 0.0;
  for (float v : raw) {
    const double d = v - m;
    var += d * d;
  }
  const double s = std::sqrt(var / static_cast<double>(raw.size()));
  *mean = m;
  *sigma = std::max(s, 1e-3);
  out->reserve(raw.size());
  for (float v : raw) out->push_back(static_cast<float>((v - m) / (*sigma)));
}

}  // namespace

// ============================================================
// RangeImage
// ============================================================

RangeImage::RangeImage(int width, int height, double vfov_rad)
    : width_(width),
      height_(height),
      fx_(-static_cast<double>(width) / (2.0 * M_PI)),
      fy_(-static_cast<double>(height) / vfov_rad),
      cx_(width * 0.5),
      cy_(height * 0.5) {}

void RangeImage::build(const std::vector<IntensityPoint>& points) {
  const int n = width_ * height_;
  intensity_.assign(n, -1.0f);
  for (const auto& ip : points) {
    double u = 0.0, v = 0.0;
    if (!project(ip.p, u, v)) continue;
    const int iu = static_cast<int>(std::lround(u));
    const int iv = static_cast<int>(std::lround(v));
    if (iu < 0 || iu >= width_ || iv < 0 || iv >= height_) continue;
    float& cell = intensity_[iv * width_ + iu];
    cell = std::max(cell, static_cast<float>(ip.intensity));
  }
}

void RangeImage::filterAndSmooth() {
  filtered_ = intensity_;
  const int n = width_ * height_;
  std::vector<float> tmp(n, -1.0f);

  // Horizontal low-pass then vertical high-pass (line artifact reduction).
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      const float c = intensity_[y * width_ + x];
      if (c < 0.0f) continue;
      const float l = intensity_[y * width_ + std::max(0, x - 1)];
      const float r = intensity_[y * width_ + std::max(0, std::min(width_ - 1, x + 1))];
      const float lp = (l >= 0 ? 0.25f * l : 0.0f) + 0.5f * c +
                       (r >= 0 ? 0.25f * r : 0.0f);
      tmp[y * width_ + x] = lp;
    }
  }
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      const float c = tmp[y * width_ + x];
      if (c < 0.0f) continue;
      const float u = tmp[std::max(0, y - 1) * width_ + x];
      const float d = tmp[std::min(height_ - 1, y + 1) * width_ + x];
      const float hp = c - 0.5f * ((u >= 0 ? u : c) + (d >= 0 ? d : c));
      filtered_[y * width_ + x] = hp;
    }
  }

  // Local brightness normalization over a coarse window.
  const int win = 16;
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      const float c = filtered_[y * width_ + x];
      if (c < 0.0f) continue;
      double sum = 0.0;
      int cnt = 0;
      for (int dy = -win; dy <= win; dy += win) {
        for (int dx = -win; dx <= win; dx += win) {
          const int xx = std::clamp(x + dx, 0, width_ - 1);
          const int yy = std::clamp(y + dy, 0, height_ - 1);
          const float v = filtered_[yy * width_ + xx];
          if (v >= 0.0f) {
            sum += v;
            ++cnt;
          }
        }
      }
      if (cnt > 0) filtered_[y * width_ + x] = c - static_cast<float>(sum / cnt);
    }
  }

  // 3x3 Gaussian smooth.
  tmp = filtered_;
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      double acc = 0.0;
      double wsum = 0.0;
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
          const int xx = std::clamp(x + dx, 0, width_ - 1);
          const int yy = std::clamp(y + dy, 0, height_ - 1);
          const float v = tmp[yy * width_ + xx];
          if (v < 0.0f) continue;
          const double w = (dx == 0 && dy == 0) ? 4.0 : (dx == 0 || dy == 0 ? 2.0 : 1.0);
          acc += w * v;
          wsum += w;
        }
      }
      if (wsum > 0.0) filtered_[y * width_ + x] = static_cast<float>(acc / wsum);
    }
  }
}

void RangeImage::computeGradients() {
  grad_u_.assign(width_ * height_, 0.0f);
  grad_v_.assign(width_ * height_, 0.0f);
  auto at = [&](int x, int y) {
    x = std::clamp(x, 0, width_ - 1);
    y = std::clamp(y, 0, height_ - 1);
    return filtered_[y * width_ + x];
  };
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      if (filtered_[y * width_ + x] < 0.0f) continue;
      grad_u_[y * width_ + x] =
          0.5f * (at(x + 1, y) - at(x - 1, y));
      grad_v_[y * width_ + x] =
          0.5f * (at(x, y + 1) - at(x, y - 1));
    }
  }
}

bool RangeImage::project(const Eigen::Vector3d& p_lidar, double& u,
                         double& v) const {
  const double x = p_lidar.x();
  const double y = p_lidar.y();
  const double z = p_lidar.z();
  const double L = std::sqrt(x * x + y * y);
  const double R = std::sqrt(L * L + z * z);
  if (R < 1e-6) return false;
  u = fx_ * std::atan2(y, x) + cx_;
  v = fy_ * std::asin(z / R) + cy_;
  return u >= 0.0 && u < width_ && v >= 0.0 && v < height_;
}

float RangeImage::sample(const std::vector<float>& img, double u, double v) const {
  if (u < 0.0 || v < 0.0 || u >= width_ - 1 || v >= height_ - 1) return -1.0f;
  const int x0 = static_cast<int>(std::floor(u));
  const int y0 = static_cast<int>(std::floor(v));
  const double ax = u - x0;
  const double ay = v - y0;
  const float v00 = img[y0 * width_ + x0];
  const float v10 = img[y0 * width_ + x0 + 1];
  const float v01 = img[(y0 + 1) * width_ + x0];
  const float v11 = img[(y0 + 1) * width_ + x0 + 1];
  if (v00 < 0.0f || v10 < 0.0f || v01 < 0.0f || v11 < 0.0f) return -1.0f;
  return static_cast<float>((1.0 - ax) * (1.0 - ay) * v00 + ax * (1.0 - ay) * v10 +
                            (1.0 - ax) * ay * v01 + ax * ay * v11);
}

float RangeImage::sampleFiltered(double u, double v) const {
  return sample(filtered_, u, v);
}

float RangeImage::gradU(double u, double v) const {
  const int x = std::clamp(static_cast<int>(std::lround(u)), 0, width_ - 1);
  const int y = std::clamp(static_cast<int>(std::lround(v)), 0, height_ - 1);
  return grad_u_[y * width_ + x];
}

float RangeImage::gradV(double u, double v) const {
  const int x = std::clamp(static_cast<int>(std::lround(u)), 0, width_ - 1);
  const int y = std::clamp(static_cast<int>(std::lround(v)), 0, height_ - 1);
  return grad_v_[y * width_ + x];
}

// ============================================================
// VoxelHashMap
// ============================================================

VoxelHashMap::VoxelHashMap(double voxel_size, int max_points_per_voxel)
    : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

Eigen::Vector3i VoxelHashMap::toVoxel(const Eigen::Vector3d& p) const {
  return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / voxel_size_)),
                         static_cast<int>(std::floor(p.y() / voxel_size_)),
                         static_cast<int>(std::floor(p.z() / voxel_size_)));
}

void VoxelHashMap::addPoints(const std::vector<Eigen::Vector3d>& points) {
  for (const auto& p : points) {
    auto& vb = map_[toVoxel(p)];
    if (static_cast<int>(vb.points.size()) < max_points_) vb.points.push_back(p);
  }
}

void VoxelHashMap::pruneFarVoxels(const Eigen::Vector3d& center,
                                  double max_distance) {
  const double max_sq = max_distance * max_distance;
  for (auto it = map_.begin(); it != map_.end();) {
    const Eigen::Vector3d vc =
        (it->first.cast<double>() + Eigen::Vector3d::Constant(0.5)) * voxel_size_;
    if ((vc - center).squaredNorm() > max_sq)
      it = map_.erase(it);
    else
      ++it;
  }
}

std::vector<VoxelHashMap::Correspondence> VoxelHashMap::getCorrespondences(
    const std::vector<Eigen::Vector3d>& points, double max_dist,
    int normal_min_neighbors) const {
  std::vector<Correspondence> correspondences(points.size());
  const double max_sq = max_dist * max_dist;
  for (size_t i = 0; i < points.size(); ++i) {
    const Eigen::Vector3i key = toVoxel(points[i]);
    std::vector<Eigen::Vector3d> neighbors;
    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dz = -1; dz <= 1; ++dz) {
          const auto it = map_.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == map_.end()) continue;
          for (const auto& nb : it->second.points) {
            if ((nb - points[i]).squaredNorm() <= max_sq) neighbors.push_back(nb);
          }
        }
      }
    }
    if (neighbors.empty()) continue;
    Eigen::Vector3d best = neighbors.front();
    double best_d = (best - points[i]).squaredNorm();
    for (const auto& nb : neighbors) {
      const double d = (nb - points[i]).squaredNorm();
      if (d < best_d) {
        best_d = d;
        best = nb;
      }
    }
    Correspondence& c = correspondences[i];
    c.point = best;
    c.found = true;
    if (static_cast<int>(neighbors.size()) < normal_min_neighbors) continue;
    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
    for (const auto& nb : neighbors) centroid += nb;
    centroid /= static_cast<double>(neighbors.size());
    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    for (const auto& nb : neighbors) {
      const Eigen::Vector3d d = nb - centroid;
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
    c.has_normal = true;
  }
  return correspondences;
}

// ============================================================
// PG-LIO pipeline
// ============================================================

PGLIOPipeline::PGLIOPipeline(const PGLIOParams& params)
    : params_(params),
      local_map_(params.voxel_size, params.max_points_per_voxel) {}

void PGLIOPipeline::setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }

std::vector<IntensityPoint> PGLIOPipeline::rangeFilter(
    const std::vector<IntensityPoint>& points) const {
  std::vector<IntensityPoint> out;
  out.reserve(points.size());
  for (const auto& ip : points) {
    const double r = ip.p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(ip);
  }
  return out;
}

std::vector<IntensityPoint> PGLIOPipeline::voxelDownsample(
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

std::vector<Eigen::Vector3d> PGLIOPipeline::toPoints(
    const std::vector<IntensityPoint>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& ip : points) out.push_back(ip.p);
  return out;
}

void PGLIOPipeline::extractPatches(const RangeImage& image,
                                   const std::vector<IntensityPoint>& points,
                                   const Eigen::Matrix4d& pose_w) {
  RangeImage grad_img(params_.range_image_width, params_.range_image_height);
  grad_img.build(points);
  grad_img.filterAndSmooth();
  grad_img.computeGradients();

  struct Candidate {
    double score;
    int u;
    int v;
  };
  std::vector<Candidate> cands;
  const int r = params_.patch_radius;
  for (int y = r; y < params_.range_image_height - r; ++y) {
    for (int x = r; x < params_.range_image_width - r; ++x) {
      const float gu = grad_img.gradU(x, y);
      const float gv = grad_img.gradV(x, y);
      const double g = std::sqrt(gu * gu + gv * gv);
      if (g >= params_.min_patch_gradient) cands.push_back({g, x, y});
    }
  }
  std::partial_sort(cands.begin(),
                    cands.begin() + std::min(cands.size(), static_cast<size_t>(params_.max_patches)),
                    cands.end(),
                    [](const Candidate& a, const Candidate& b) {
                      return a.score > b.score;
                    });

  const int take = std::min(static_cast<int>(cands.size()), params_.max_patches);
  for (int i = 0; i < take; ++i) {
    const int cx = cands[i].u;
    const int cy = cands[i].v;
    PhotometricPatch patch;
    std::vector<float> raw;
    std::vector<Eigen::Vector3d> anchors_l;
    for (int dy = -r; dy <= r; ++dy) {
      for (int dx = -r; dx <= r; ++dx) {
        const double u = cx + dx;
        const double v = cy + dy;
        const float val = image.sampleFiltered(u, v);
        if (val < 0.0f) continue;
        double best_d = std::numeric_limits<double>::max();
        Eigen::Vector3d best_p = Eigen::Vector3d::Zero();
        for (const auto& ip : points) {
          double pu = 0.0, pv = 0.0;
          if (!image.project(ip.p, pu, pv)) continue;
          const double d = (pu - u) * (pu - u) + (pv - v) * (pv - v);
          if (d < best_d) {
            best_d = d;
            best_p = ip.p;
          }
        }
        if (best_d >= 4.0) continue;
        raw.push_back(val);
        anchors_l.push_back(best_p);
      }
    }
    if (raw.size() < 4 || anchors_l.size() != raw.size()) continue;
    normalizePatch(raw, &patch.ref_norm, &patch.ref_mean, &patch.ref_sigma);
    patch.anchors_w.reserve(anchors_l.size());
    for (const auto& p_l : anchors_l) {
      patch.anchors_w.push_back(pose_w.block<3, 3>(0, 0) * p_l + pose_w.block<3, 1>(0, 3));
    }
    patches_.push_back(std::move(patch));
  }
  if (static_cast<int>(patches_.size()) > params_.max_patches) {
    patches_.erase(patches_.begin(),
                    patches_.end() - static_cast<size_t>(params_.max_patches));
  }
}

double PGLIOPipeline::patchNccError(const PhotometricPatch& patch,
                                    const RangeImage& image,
                                    const Eigen::Matrix4d& pose_w,
                                    Eigen::Matrix<double, 6, 1>* jacobian) const {
  const Eigen::Matrix3d R = pose_w.block<3, 3>(0, 0);
  const Eigen::Vector3d t = pose_w.block<3, 1>(0, 3);
  const Eigen::Matrix3d R_inv = R.transpose();

  std::vector<float> cur;
  cur.reserve(patch.ref_norm.size());
  std::vector<Eigen::Vector3d> pts_l;
  pts_l.reserve(patch.anchors_w.size());

  size_t n = std::min(patch.anchors_w.size(), patch.ref_norm.size());
  for (size_t i = 0; i < n; ++i) {
    const Eigen::Vector3d p_l = R_inv * (patch.anchors_w[i] - t);
    pts_l.push_back(p_l);
    double u = 0.0, v = 0.0;
    if (!image.project(p_l, u, v)) return params_.ncc_outlier_threshold + 1.0;
    const float val = image.sampleFiltered(u, v);
    if (val < 0.0f) return params_.ncc_outlier_threshold + 1.0;
    cur.push_back(val);
  }
  if (cur.size() < 4) return params_.ncc_outlier_threshold + 1.0;

  double mean = 0.0;
  for (float v : cur) mean += v;
  mean /= static_cast<double>(cur.size());
  double var = 0.0;
  for (float v : cur) {
    const double d = v - mean;
    var += d * d;
  }
  const double sigma = std::max(std::sqrt(var / static_cast<double>(cur.size())), 1e-3);
  std::vector<float> cur_norm(cur.size());
  for (size_t i = 0; i < cur.size(); ++i) cur_norm[i] = static_cast<float>((cur[i] - mean) / sigma);

  double dot = 0.0;
  for (size_t i = 0; i < cur_norm.size(); ++i) dot += patch.ref_norm[i] * cur_norm[i];
  const double znssd = std::max(0.0, 2.0 - 2.0 * dot / static_cast<double>(cur_norm.size()));

  if (jacobian) {
    jacobian->setZero();
    const double eps = 1e-4;
    for (int k = 0; k < 6; ++k) {
      Eigen::Matrix<double, 6, 1> delta = Eigen::Matrix<double, 6, 1>::Zero();
      delta(k) = eps;
      Eigen::Matrix4d pert = pose_w;
      pert.block<3, 3>(0, 0) = expSO3(delta.head<3>()) * pert.block<3, 3>(0, 0);
      pert.block<3, 1>(0, 3) += delta.tail<3>();
      const double e_plus = patchNccError(patch, image, pert, nullptr);
      (*jacobian)(k) = (e_plus - znssd) / eps;
    }
  }
  return znssd;
}

int PGLIOPipeline::countDegenerateTransDirs(
    const Eigen::Matrix<double, 6, 6>& jtJ) const {
  const Eigen::Matrix3d block = jtJ.block<3, 3>(3, 3);
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(block);
  if (es.info() != Eigen::Success) return 0;
  const Eigen::Vector3d lam = es.eigenvalues();
  const double lam_max = std::max(lam(2), 1e-12);
  const double kappa = params_.degeneracy_ratio * lam_max;
  int count = 0;
  for (int k = 0; k < 3; ++k)
    if (lam(k) < kappa) ++count;
  return count;
}

Eigen::Matrix4d PGLIOPipeline::runJointOptimization(
    const std::vector<Eigen::Vector3d>& registration_points,
    const RangeImage& image, const Eigen::Matrix4d& initial_guess,
    const Eigen::Matrix4d& imu_prediction, bool used_imu, PGLIOResult* result) {
  Eigen::Matrix4d T = initial_guess;
  int max_geom = 0;
  int used_patches = 0;
  int deg_trans = 0;

  for (int iter = 0; iter < params_.max_icp_iterations; ++iter) {
    const auto src = transformPoints(registration_points, T);
    const auto corr = local_map_.getCorrespondences(
        src, params_.max_correspondence_dist, params_.normal_min_neighbors);

    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    int geom = 0;
    for (size_t i = 0; i < registration_points.size(); ++i) {
      const auto& c = corr[i];
      if (!c.found || !c.has_normal ||
          c.planarity < params_.planarity_threshold)
        continue;
      const Eigen::Vector3d q = src[i];
      const double e = c.normal.dot(q - c.anchor);
      Eigen::Matrix<double, 1, 6> J;
      J.block<1, 3>(0, 0) = (-c.normal.transpose() * skew(q));
      J.block<1, 3>(0, 3) = c.normal.transpose();
      JtJ += J.transpose() * J;
      Jtb += J.transpose() * e;
      ++geom;
    }
    max_geom = std::max(max_geom, geom);
    if (geom < 10) break;

    deg_trans = countDegenerateTransDirs(JtJ);
    const double photo_gain =
        params_.photometric_weight * (1.0 + static_cast<double>(deg_trans));

    used_patches = 0;
    if (photo_gain > 0.0 && deg_trans > 0) {
      for (const auto& patch : patches_) {
        Eigen::Matrix<double, 6, 1> Jp;
        const double ep = patchNccError(patch, image, T, &Jp);
        if (ep > params_.ncc_outlier_threshold) continue;
        JtJ += photo_gain * Jp * Jp.transpose();
        Jtb += photo_gain * Jp * ep;
        ++used_patches;
      }
    }

    const Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) break;
    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;
    if (delta.norm() < params_.convergence_criterion) break;
  }

  if (result) {
    result->geometric_matches = max_geom;
    result->photometric_patches = used_patches;
    result->degenerate_trans_dirs = deg_trans;
  }
  return T;
}

PGLIOResult PGLIOPipeline::registerFrame(
    const std::vector<IntensityPoint>& frame,
    const std::vector<imu_preintegration::ImuSample>& imu) {
  PGLIOResult result;
  auto filtered = rangeFilter(frame);
  auto down = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg = voxelDownsample(down, params_.voxel_size);
  if (reg.empty()) reg = down;

  RangeImage image(params_.range_image_width, params_.range_image_height);
  image.build(filtered);
  image.filterAndSmooth();
  image.computeGradients();

  if (reg.empty()) {
    if (frame_count_ == 0) {
      frame_count_++;
      result.pose = pose_;
      result.converged = true;
    }
    return result;
  }

  if (frame_count_ == 0) {
    local_map_.addPoints(transformPoints(toPoints(down), pose_));
    extractPatches(image, filtered, pose_);
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  Eigen::Matrix3d imu_delta_R = last_delta_.block<3, 3>(0, 0);
  if (imu.size() >= 2) {
    imu_preintegration::ImuPreintegrator preint;
    preint.integrateBatch(imu);
    const auto& pi = preint.result();
    if (pi.delta_R.allFinite()) {
      imu_delta_R = pi.delta_R;
      result.used_imu = true;
    }
  }

  Eigen::Matrix4d motion = Eigen::Matrix4d::Identity();
  motion.block<3, 3>(0, 0) = imu_delta_R;
  motion.block<3, 1>(0, 3) = last_delta_.block<3, 1>(0, 3);
  const Eigen::Matrix4d prediction = pose_ * motion;
  const Eigen::Matrix4d prev_pose = pose_;
  const Eigen::Matrix4d initial = prediction;

  const auto reg_pts = toPoints(reg);
  pose_ = runJointOptimization(reg_pts, image, initial, prediction,
                               result.used_imu, &result);
  result.converged = true;
  last_delta_ = prev_pose.inverse() * pose_;

  local_map_.addPoints(transformPoints(toPoints(down), pose_));
  extractPatches(image, filtered, pose_);

  if (frame_count_ % params_.map_cleanup_interval == 0) {
    local_map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  frame_count_++;
  result.pose = pose_;
  return result;
}

}  // namespace pg_lio
}  // namespace localization_zoo
