#include "inten_loam/inten_loam.h"
#include "inten_loam/factors.h"

#include "common/ceres_compat.h"

#include <ceres/ceres.h>
#include <Eigen/Dense>
#include <pcl/kdtree/kdtree_flann.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numeric>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace inten_loam {

namespace {

int pixelIndex(int u, int v, int width) { return v * width + u; }

bool inImage(int u, int v, int width, int height) {
  return u >= 0 && u < width && v >= 0 && v < height;
}

}  // namespace

CylindricalImage InTenLoam::projectCylindrical(const std::vector<PointI>& points,
                                               const CylindricalParams& params) {
  CylindricalImage img;
  img.params = params;
  const size_t n = static_cast<size_t>(params.width * params.height);
  img.range.assign(n, 0.0f);
  img.intensity.assign(n, 0.0f);
  img.label.assign(n, static_cast<uint8_t>(FeatureLabel::kNone));
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
    const double v_f =
        (std::asin(z / r) + theta_d) / theta * params.height;
    const int u = static_cast<int>(std::floor(u_f));
    const int v = static_cast<int>(std::floor(v_f));
    if (!inImage(u, v, params.width, params.height)) continue;

    const int idx = pixelIndex(u, v, params.width);
    if (img.range[idx] == 0.0f || r < img.range[idx]) {
      img.range[idx] = static_cast<float>(r);
      img.intensity[idx] = pt.intensity;
      img.points[idx] = pt.p;
    } else if (std::abs(r - img.range[idx]) < 0.05f) {
      img.intensity[idx] = std::max(img.intensity[idx], pt.intensity);
    }
  }
  return img;
}

void InTenLoam::computeCurvature(CylindricalImage* image) {
  if (!image) return;
  const int w = image->params.width;
  const int h = image->params.height;
  std::vector<float> curvature(image->range.size(), 0.0f);
  for (int v = 1; v < h - 1; ++v) {
    for (int u = 1; u < w - 1; ++u) {
      const int idx = pixelIndex(u, v, w);
      if (image->range[idx] <= 0.0f) continue;
      double diff = 0.0;
      int cnt = 0;
      const double center = image->range[idx];
      for (int dv = -1; dv <= 1; ++dv) {
        for (int du = -1; du <= 1; ++du) {
          if (du == 0 && dv == 0) continue;
          const int j = pixelIndex(u + du, v + dv, w);
          if (image->range[j] <= 0.0f) continue;
          diff += std::abs(center - image->range[j]);
          ++cnt;
        }
      }
      if (cnt > 0) curvature[idx] = static_cast<float>(diff / (cnt * center));
    }
  }
  // Store curvature temporarily in unused label channel high bits - use range
  // variance proxy: overwrite intensity delta computation inline in extract.
  (void)curvature;
  image->label = std::vector<uint8_t>(image->label.size(), 0);
  for (int v = 1; v < h - 1; ++v) {
    for (int u = 1; u < w - 1; ++u) {
      const int idx = pixelIndex(u, v, w);
      if (image->range[idx] <= 0.0f) continue;
      double diff = 0.0;
      int cnt = 0;
      const double center = image->range[idx];
      for (int dv = -1; dv <= 1; ++dv) {
        for (int du = -1; du <= 1; ++du) {
          if (du == 0 && dv == 0) continue;
          const int j = pixelIndex(u + du, v + dv, w);
          if (image->range[j] <= 0.0f) continue;
          diff += std::abs(center - image->range[j]);
          ++cnt;
        }
      }
      if (cnt > 0 && diff / (cnt * center) > 0.05) {
        image->label[idx] = static_cast<uint8_t>(FeatureLabel::kEdge);
      }
    }
  }
}

void InTenLoam::segmentGround(CylindricalImage* image, double residual_thresh) {
  if (!image) return;
  const int w = image->params.width;
  const int h = image->params.height;
  for (int u = 0; u < w; ++u) {
    std::vector<std::pair<double, double>> samples;
    for (int v = h - 1; v >= 0; --v) {
      const int idx = pixelIndex(u, v, w);
      if (image->range[idx] <= 0.0f) continue;
      samples.emplace_back(image->range[idx], image->points[idx].z());
    }
    if (samples.size() < 4) continue;
    double sum_r = 0, sum_z = 0, sum_rr = 0, sum_rz = 0;
    for (const auto& s : samples) {
      sum_r += s.first;
      sum_z += s.second;
      sum_rr += s.first * s.first;
      sum_rz += s.first * s.second;
    }
    const double n = static_cast<double>(samples.size());
    const double denom = n * sum_rr - sum_r * sum_r;
    if (std::abs(denom) < 1e-9) continue;
    const double a = (n * sum_rz - sum_r * sum_z) / denom;
    const double b = (sum_z - a * sum_r) / n;
    for (int v = h - 1; v >= 0; --v) {
      const int idx = pixelIndex(u, v, w);
      if (image->range[idx] <= 0.0f) continue;
      const double pred_z = a * image->range[idx] + b;
      if (std::abs(pred_z - image->points[idx].z()) < residual_thresh) {
        image->label[idx] = static_cast<uint8_t>(FeatureLabel::kGround);
      }
    }
  }
}

void InTenLoam::extractFeatureLabels(
    CylindricalImage* image, double edge_curvature_thresh,
    double reflector_intensity_delta_thresh, double reflector_intensity_abs_thresh) {
  if (!image) return;
  computeCurvature(image);
  segmentGround(image, 0.15);

  const int w = image->params.width;
  const int h = image->params.height;
  for (int v = 1; v < h - 1; ++v) {
    for (int u = 1; u < w - 1; ++u) {
      const int idx = pixelIndex(u, v, w);
      if (image->range[idx] <= 0.0f) continue;
      if (image->label[idx] == static_cast<uint8_t>(FeatureLabel::kGround)) continue;

      double diff = 0.0;
      int cnt = 0;
      const double center = image->range[idx];
      for (int dv = -1; dv <= 1; ++dv) {
        for (int du = -1; du <= 1; ++du) {
          if (du == 0 && dv == 0) continue;
          const int j = pixelIndex(u + du, v + dv, w);
          if (image->range[j] <= 0.0f) continue;
          diff += std::abs(center - image->range[j]);
          ++cnt;
        }
      }
      const double curv = cnt ? diff / (cnt * center) : 0.0;
      const float i_delta =
          std::abs(image->intensity[idx] - image->intensity[pixelIndex(u, v + 1, w)]);

      if (curv > edge_curvature_thresh) {
        image->label[idx] = static_cast<uint8_t>(FeatureLabel::kEdge);
      } else if (i_delta > reflector_intensity_delta_thresh &&
                 image->intensity[idx] > reflector_intensity_abs_thresh) {
        image->label[idx] = static_cast<uint8_t>(FeatureLabel::kReflector);
      } else if (image->label[idx] != static_cast<uint8_t>(FeatureLabel::kEdge)) {
        image->label[idx] = static_cast<uint8_t>(FeatureLabel::kFacade);
      }
    }
  }
}

std::vector<PointI> InTenLoam::collectFeatures(const CylindricalImage& image,
                                               FeatureLabel label,
                                               int max_count) {
  struct Cand {
    float score;
    PointI pt;
  };
  std::vector<Cand> cands;
  const int w = image.params.width;
  const int h = image.params.height;
  for (int v = 0; v < h; ++v) {
    for (int u = 0; u < w; ++u) {
      const int idx = pixelIndex(u, v, w);
      if (image.range[idx] <= 0.0f) continue;
      if (static_cast<FeatureLabel>(image.label[idx]) != label) continue;
      PointI p;
      p.p = image.points[idx];
      p.intensity = image.intensity[idx];
      p.label = label;
      float score = image.intensity[idx];
      if (label == FeatureLabel::kEdge) {
        score = 1.0f;
        for (int dv = -1; dv <= 1; ++dv) {
          for (int du = -1; du <= 1; ++du) {
            const int j = pixelIndex(u + du, v + dv, w);
            if (image.range[j] > 0.0f)
              score += std::abs(image.range[idx] - image.range[j]);
          }
        }
      }
      cands.push_back({score, p});
    }
  }
  std::sort(cands.begin(), cands.end(),
            [](const Cand& a, const Cand& b) { return a.score > b.score; });
  const int take = std::min(max_count, static_cast<int>(cands.size()));
  std::vector<PointI> out;
  out.reserve(static_cast<size_t>(take));
  for (int i = 0; i < take; ++i) out.push_back(cands[i].pt);
  return out;
}

double InTenLoam::predictIntensityTrilinear(
    const std::vector<PointI>& target_reflectors, const Eigen::Vector3d& query,
    double voxel_size) {
  if (target_reflectors.empty()) return 0.0;
  const double h = 0.5 * voxel_size;
  Eigen::Vector3d bmin = query;
  Eigen::Vector3d bmax = query;
  std::array<double, 8> corners{};
  std::array<int, 8> counts{};
  for (const auto& tp : target_reflectors) {
    if ((tp.p - query).norm() > 3.0 * voxel_size) continue;
    bmin = bmin.cwiseMin(tp.p);
    bmax = bmax.cwiseMax(tp.p);
    const Eigen::Vector3i cell = (tp.p / voxel_size).cast<int>();
    const Eigen::Vector3d rel = tp.p - query;
    const int ix = rel.x() >= 0 ? 1 : 0;
    const int iy = rel.y() >= 0 ? 1 : 0;
    const int iz = rel.z() >= 0 ? 1 : 0;
    const int corner = ix + 2 * iy + 4 * iz;
    corners[corner] += tp.intensity;
    counts[corner]++;
    (void)cell;
  }
  for (int i = 0; i < 8; ++i) {
    if (counts[i] > 0) corners[i] /= counts[i];
  }
  bmin = query - Eigen::Vector3d(h, h, h);
  bmax = query + Eigen::Vector3d(h, h, h);
  const Eigen::Vector3d denom = bmax - bmin + Eigen::Vector3d::Constant(1e-6);
  const Eigen::Vector3d rel = (query - bmin).cwiseQuotient(denom);
  const double u = rel.x(), v = rel.y(), w = rel.z();
  const auto lerp = [](double a, double b, double t) {
    return a * (1.0 - t) + b * t;
  };
  const double c00 = lerp(corners[0], corners[1], u);
  const double c10 = lerp(corners[2], corners[3], u);
  const double c01 = lerp(corners[4], corners[5], u);
  const double c11 = lerp(corners[6], corners[7], u);
  const double c0 = lerp(c00, c10, v);
  const double c1 = lerp(c01, c11, v);
  return lerp(c0, c1, w);
}

double InTenLoam::intensityResidual(double predicted, double observed,
                                    double sigma) {
  if (sigma <= 0.0) return predicted - observed;
  return (predicted - observed) / sigma;
}

InTenLoam::InTenLoam(const InTenLoamParams& params) : params_(params) {}

void InTenLoam::clear() {
  last_edges_.clear();
  last_surfs_.clear();
  last_reflectors_.clear();
  map_edges_.clear();
  map_surfs_.clear();
  map_reflectors_.clear();
  temporal_voxels_.clear();
  q_w_curr_ = Eigen::Quaterniond::Identity();
  t_w_curr_ = Eigen::Vector3d::Zero();
  q_last_curr_ = Eigen::Quaterniond::Identity();
  t_last_curr_ = Eigen::Vector3d::Zero();
  initialized_ = false;
  frame_count_ = 0;
}

InTenLoam::VoxelKey InTenLoam::voxelKey(const Eigen::Vector3d& p,
                                        double voxel_size) const {
  VoxelKey key;
  key.x = static_cast<int>(std::floor(p.x() / voxel_size));
  key.y = static_cast<int>(std::floor(p.y() / voxel_size));
  key.z = static_cast<int>(std::floor(p.z() / voxel_size));
  return key;
}

void InTenLoam::applyTemporalVoxelFilter(std::vector<PointI>* points,
                                           size_t* removed) {
  if (removed) *removed = 0;
  if (!points || !params_.enable_tvf || points->empty()) return;

  std::vector<PointI> kept;
  kept.reserve(points->size());
  for (const auto& pt : *points) {
    const VoxelKey key = voxelKey(pt.p, params_.tvf_voxel_size);
    TemporalVoxel& voxel = temporal_voxels_[key];
    const float range = static_cast<float>(pt.p.norm());
    if (voxel.obs_count == 0) {
      voxel.mean_range = range;
    } else {
      voxel.mean_range = 0.9f * voxel.mean_range + 0.1f * range;
    }
    voxel.obs_count++;
    voxel.last_frame = frame_count_;

    if (frame_count_ < params_.tvf_min_observations ||
        voxel.obs_count >= params_.tvf_min_observations) {
      kept.push_back(pt);
    } else if (removed) {
      (*removed)++;
    }
  }
  *points = std::move(kept);
}

void InTenLoam::applyDynamicObjectRemoval(CylindricalImage* image,
                                          size_t* removed) {
  if (removed) *removed = 0;
  if (!image || !params_.enable_dor || map_surfs_.empty()) return;

  pcl::KdTreeFLANN<pcl::PointXYZ> kd;
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
  for (const auto& p : map_surfs_) {
    cloud->push_back(pcl::PointXYZ(static_cast<float>(p.p.x()),
                                   static_cast<float>(p.p.y()),
                                   static_cast<float>(p.p.z())));
  }
  kd.setInputCloud(cloud);

  const int w = image->params.width;
  const int h = image->params.height;
  std::vector<int> idx(1);
  std::vector<float> dist(1);
  for (int v = 0; v < h; ++v) {
    for (int u = 0; u < w; ++u) {
      const int pix = pixelIndex(u, v, w);
      if (image->range[pix] <= 0.0f) continue;
      const Eigen::Vector3d pw =
          q_w_curr_ * image->points[pix] + t_w_curr_;
      pcl::PointXYZ q(static_cast<float>(pw.x()), static_cast<float>(pw.y()),
                      static_cast<float>(pw.z()));
      if (kd.nearestKSearch(q, 1, idx, dist) <= 0) continue;
      const Eigen::Vector3d p_map = map_surfs_[idx[0]].p;
      const Eigen::Vector3d p_map_body =
          q_w_curr_.inverse() * (p_map - t_w_curr_);
      const double map_range = p_map_body.norm();
      const double curr_range = image->range[pix];
      if (std::abs(curr_range - map_range) > params_.dor_range_delta_thresh) {
        image->range[pix] = 0.0f;
        image->intensity[pix] = 0.0f;
        image->label[pix] = static_cast<uint8_t>(FeatureLabel::kNone);
        if (removed) (*removed)++;
      }
    }
  }
}

std::vector<PointI> InTenLoam::transformPoints(
    const std::vector<PointI>& pts) const {
  std::vector<PointI> out;
  out.reserve(pts.size());
  for (const auto& pt : pts) {
    PointI tp = pt;
    tp.p = q_w_curr_ * pt.p + t_w_curr_;
    out.push_back(tp);
  }
  return out;
}

std::vector<PointI> InTenLoam::transformWorldToBody(
    const std::vector<PointI>& world_pts) const {
  std::vector<PointI> out;
  out.reserve(world_pts.size());
  const Eigen::Quaterniond q_inv = q_w_curr_.inverse();
  for (const auto& pt : world_pts) {
    PointI bp = pt;
    bp.p = q_inv * (pt.p - t_w_curr_);
    out.push_back(bp);
  }
  return out;
}

void InTenLoam::updateLocalMap(const std::vector<PointI>& edges,
                               const std::vector<PointI>& surfs,
                               const std::vector<PointI>& reflectors) {
  if (!params_.enable_mapping) return;
  auto appendUnique = [this](std::vector<PointI>* map,
                              const std::vector<PointI>& src) {
    for (const auto& pt : src) {
      PointI wp = pt;
      wp.p = q_w_curr_ * pt.p + t_w_curr_;
      const VoxelKey key = voxelKey(wp.p, params_.map_voxel_size);
      bool exists = false;
      for (const auto& mp : *map) {
        if (voxelKey(mp.p, params_.map_voxel_size) == key) {
          exists = true;
          break;
        }
      }
      if (!exists) map->push_back(wp);
    }
  };
  appendUnique(&map_edges_, edges);
  appendUnique(&map_surfs_, surfs);
  appendUnique(&map_reflectors_, reflectors);
  pruneLocalMap();
}

void InTenLoam::pruneLocalMap() {
  auto prune = [this](std::vector<PointI>* map) {
    std::vector<PointI> kept;
    kept.reserve(map->size());
    for (const auto& pt : *map) {
      if ((pt.p - t_w_curr_).norm() <= params_.local_map_radius) {
        kept.push_back(pt);
      }
    }
    *map = std::move(kept);
  };
  prune(&map_edges_);
  prune(&map_surfs_);
  prune(&map_reflectors_);
}

std::vector<InTenLoam::GeomMatch> InTenLoam::findGeomMatches(
    const std::vector<PointI>& curr, const std::vector<PointI>& last) const {
  std::vector<GeomMatch> matches;
  if (curr.empty() || last.empty()) return matches;
  pcl::KdTreeFLANN<pcl::PointXYZ> kd;
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
  for (const auto& p : last) {
    cloud->push_back(pcl::PointXYZ(static_cast<float>(p.p.x()),
                                   static_cast<float>(p.p.y()),
                                   static_cast<float>(p.p.z())));
  }
  kd.setInputCloud(cloud);
  std::vector<int> idx(5);
  std::vector<float> dist(5);
  const double max_sq = params_.geom_distance_sq_threshold;
  for (const auto& cp : curr) {
    pcl::PointXYZ q(static_cast<float>(cp.p.x()), static_cast<float>(cp.p.y()),
                    static_cast<float>(cp.p.z()));
    if (kd.nearestKSearch(q, 5, idx, dist) < 3) continue;
    if (dist[1] > max_sq) continue;
    GeomMatch m;
    m.curr = cp.p;
    m.is_edge = cp.label == FeatureLabel::kEdge;
    if (m.is_edge) {
      m.last_j = last[idx[0]].p;
      m.last_l = last[idx[1]].p;
      m.last_m = last[idx[1]].p;
    } else {
      m.last_j = last[idx[0]].p;
      m.last_l = last[idx[1]].p;
      m.last_m = last[idx[2]].p;
    }
    matches.push_back(m);
  }
  return matches;
}

std::vector<std::pair<size_t, size_t>> InTenLoam::findReflectorMatches(
    const std::vector<PointI>& curr, const std::vector<PointI>& last) const {
  std::vector<std::pair<size_t, size_t>> matches;
  const double max_sq = params_.geom_distance_sq_threshold;
  for (size_t i = 0; i < curr.size(); ++i) {
    int best = -1;
    double best_d = max_sq;
    for (size_t j = 0; j < last.size(); ++j) {
      const double d = (curr[i].p - last[j].p).squaredNorm();
      if (d < best_d) {
        best_d = d;
        best = static_cast<int>(j);
      }
    }
    if (best >= 0) matches.emplace_back(i, static_cast<size_t>(best));
  }
  return matches;
}

bool InTenLoam::optimizeFrame(
    const std::vector<PointI>& curr_edges, const std::vector<PointI>& curr_surfs,
    const std::vector<PointI>& curr_reflectors,
    const std::vector<PointI>& last_edges,
    const std::vector<PointI>& last_surfs,
    const std::vector<PointI>& last_reflectors, InTenLoamResult* result) {
  ceres::Problem problem;
  double para_q[4] = {q_last_curr_.x(), q_last_curr_.y(), q_last_curr_.z(),
                      q_last_curr_.w()};
  double para_t[3] = {t_last_curr_.x(), t_last_curr_.y(), t_last_curr_.z()};
  problem.AddParameterBlock(para_q, 4);
  localization_zoo::SetEigenQuaternionManifold(problem, para_q);
  problem.AddParameterBlock(para_t, 3);

  size_t geom_count = 0;
  auto geom_edges = findGeomMatches(curr_edges, last_edges);
  for (const auto& m : geom_edges) {
    problem.AddResidualBlock(
        EdgeFactor::Create(m.curr, m.last_j, m.last_l, 1.0, 1.0),
        new ceres::HuberLoss(params_.huber_loss_s), para_q, para_t);
    geom_count++;
  }

  auto geom_surfs = findGeomMatches(curr_surfs, last_surfs);
  for (const auto& m : geom_surfs) {
    if (!m.is_edge) {
      problem.AddResidualBlock(PlaneFactor::Create(m.curr, m.last_j, m.last_l,
                                                   m.last_m, 1.0, 1.0),
                               new ceres::HuberLoss(params_.huber_loss_s), para_q,
                               para_t);
      geom_count++;
    }
  }

  size_t intensity_count = 0;
  double int_res_acc = 0.0;
  if (params_.use_intensity_registration && !curr_reflectors.empty()) {
    const auto refl_matches = findReflectorMatches(curr_reflectors, last_reflectors);
    for (const auto& m : refl_matches) {
      const auto& src = curr_reflectors[m.first];
      const auto& tgt = last_reflectors[m.second];
      const double h = 0.5 * params_.intensity_voxel_size;
      Eigen::Vector3d box_min = tgt.p - Eigen::Vector3d(h, h, h);
      Eigen::Vector3d box_max = tgt.p + Eigen::Vector3d(h, h, h);
      std::array<double, 8> corners{};
      std::array<int, 8> counts8{};
      // build corners from neighbors
      for (const auto& tp : last_reflectors) {
        if ((tp.p - tgt.p).norm() > 3.0 * params_.intensity_voxel_size) continue;
        const Eigen::Vector3d rel = tp.p - tgt.p;
        const int ix = rel.x() >= 0 ? 1 : 0;
        const int iy = rel.y() >= 0 ? 1 : 0;
        const int iz = rel.z() >= 0 ? 1 : 0;
        const int corner = ix + 2 * iy + 4 * iz;
        corners[corner] += tp.intensity;
        counts8[corner]++;
      }
      for (int i = 0; i < 8; ++i) {
        if (counts8[i] > 0) corners[i] /= counts8[i];
        else corners[i] = tgt.intensity;
      }
      problem.AddResidualBlock(
          IntensityBsplineFactor::Create(src.p, src.intensity, box_min, box_max,
                                         corners, params_.intensity_weight),
          new ceres::HuberLoss(params_.huber_loss_s), para_q, para_t);
      const double pred = predictIntensityTrilinear(last_reflectors, tgt.p,
                                                    params_.intensity_voxel_size);
      int_res_acc += std::abs(intensityResidual(pred, src.intensity, params_.intensity_sigma));
      intensity_count++;
    }
  }

  if (problem.NumResidualBlocks() < 4) return false;

  ceres::Solver::Options options;
  options.linear_solver_type = ceres::DENSE_QR;
  options.max_num_iterations = params_.ceres_max_iterations;
  options.logging_type = ceres::SILENT;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  if (!summary.IsSolutionUsable()) return false;

  q_last_curr_ = Eigen::Quaterniond(para_q[3], para_q[0], para_q[1], para_q[2]);
  q_last_curr_.normalize();
  t_last_curr_ = Eigen::Vector3d(para_t[0], para_t[1], para_t[2]);

  result->num_geom_factors = geom_count;
  result->num_intensity_factors = intensity_count;
  result->mean_intensity_residual =
      intensity_count ? int_res_acc / intensity_count : 0.0;
  return true;
}

InTenLoamResult InTenLoam::process(const std::vector<PointI>& points) {
  InTenLoamResult result;
  std::vector<PointI> subsampled;
  subsampled.reserve(points.size() / params_.input_stride + 1);
  for (size_t i = 0; i < points.size(); i += params_.input_stride) {
    subsampled.push_back(points[i]);
  }

  size_t tvf_removed = 0;
  applyTemporalVoxelFilter(&subsampled, &tvf_removed);
  result.num_tvf_removed = tvf_removed;

  CylindricalImage img = projectCylindrical(subsampled, params_.cylindrical);
  size_t dor_removed = 0;
  if (initialized_ && params_.enable_dor) {
    applyDynamicObjectRemoval(&img, &dor_removed);
  }
  result.num_dor_removed = dor_removed;

  extractFeatureLabels(&img, params_.edge_curvature_thresh,
                       params_.reflector_intensity_delta_thresh,
                       params_.reflector_intensity_abs_thresh);

  auto edges = collectFeatures(img, FeatureLabel::kEdge, params_.max_edge_features);
  auto grounds = collectFeatures(img, FeatureLabel::kGround, params_.max_surface_features / 2);
  auto facades = collectFeatures(img, FeatureLabel::kFacade, params_.max_surface_features / 2);
  auto reflectors =
      collectFeatures(img, FeatureLabel::kReflector, params_.max_reflector_features);

  std::vector<PointI> surfs;
  surfs.insert(surfs.end(), grounds.begin(), grounds.end());
  surfs.insert(surfs.end(), facades.begin(), facades.end());

  result.num_ground = grounds.size();
  result.num_facade = facades.size();
  result.num_edge = edges.size();
  result.num_reflector = reflectors.size();

  if (!initialized_) {
    last_edges_ = edges;
    last_surfs_ = surfs;
    last_reflectors_ = reflectors;
    initialized_ = true;
    frame_count_++;
    result.valid = true;
    result.frame_count = frame_count_;
    return result;
  }

  const bool ok = optimizeFrame(edges, surfs, reflectors, last_edges_, last_surfs_,
                                last_reflectors_, &result);

  if (params_.enable_mapping && !map_edges_.empty() &&
      frame_count_ % std::max(1, params_.mapping_keyframe_interval) == 0) {
    const auto map_edges_body = transformWorldToBody(map_edges_);
    const auto map_surfs_body = transformWorldToBody(map_surfs_);
    const auto map_refl_body = transformWorldToBody(map_reflectors_);
    InTenLoamResult map_result;
    if (optimizeFrame(edges, surfs, reflectors, map_edges_body, map_surfs_body,
                      map_refl_body, &map_result)) {
      result.num_geom_factors += map_result.num_geom_factors;
      result.num_intensity_factors += map_result.num_intensity_factors;
      result.num_map_factors =
          map_result.num_geom_factors + map_result.num_intensity_factors;
      result.mapping_updates++;
    }
  }

  Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
  T_rel.block<3, 3>(0, 0) = q_last_curr_.toRotationMatrix();
  T_rel.block<3, 1>(0, 3) = t_last_curr_;

  Eigen::Matrix4d T_world = Eigen::Matrix4d::Identity();
  T_world.block<3, 3>(0, 0) = q_w_curr_.toRotationMatrix();
  T_world.block<3, 1>(0, 3) = t_w_curr_;
  T_world = T_world * T_rel;
  q_w_curr_ = Eigen::Quaterniond(T_world.block<3, 3>(0, 0));
  t_w_curr_ = T_world.block<3, 1>(0, 3);

  last_edges_ = std::move(edges);
  last_surfs_ = std::move(surfs);
  last_reflectors_ = std::move(reflectors);
  if (params_.enable_mapping && ok) {
    updateLocalMap(last_edges_, last_surfs_, last_reflectors_);
  }
  frame_count_++;

  result.q_w_curr = q_w_curr_;
  result.t_w_curr = t_w_curr_;
  result.valid = ok;
  result.frame_count = frame_count_;
  return result;
}

}  // namespace inten_loam
}  // namespace localization_zoo
