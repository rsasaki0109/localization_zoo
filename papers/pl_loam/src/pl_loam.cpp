#include "pl_loam/pl_loam.h"
#include "pl_loam/factors.h"

#include <ceres/ceres.h>
#include <ceres/rotation.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>

namespace localization_zoo {
namespace pl_loam {

namespace {

int clampInt(int v, int lo, int hi) {
  return std::max(lo, std::min(hi, v));
}

int pixelIndex(int u, int v, int width) { return v * width + u; }

bool inImage(int u, int v, int width, int height) {
  return u >= 0 && u < width && v >= 0 && v < height;
}

double medianInPlace(std::vector<double>* values) {
  if (values->empty()) return 0.0;
  const size_t mid = values->size() / 2;
  std::nth_element(values->begin(), values->begin() + static_cast<long>(mid),
                   values->end());
  return (*values)[mid];
}

Eigen::Matrix3d rotationFromQuat(const Eigen::Quaterniond& q) {
  return q.normalized().toRotationMatrix();
}

}  // namespace

CameraModel CameraModel::kittiHalfRes() {
  CameraModel cam;
  cam.width = 621;
  cam.height = 188;
  cam.fx = 359.428;
  cam.fy = 359.428;
  cam.cx = 303.596;
  cam.cy = 92.6079;
  cam.T_velo_cam.setIdentity();
  cam.T_velo_cam << 7.533745e-03, -9.999714e-01, -6.166020e-04, -4.069766e-03,
      1.480227e-02, 7.280733e-04, -9.998902e-01, -7.631618e-02,
      9.998621e-01, 7.523790e-03, 1.480755e-02, -2.717806e-01, 0.0, 0.0, 0.0,
      1.0;
  return cam;
}

Eigen::Vector3d CameraModel::veloToCam(const Eigen::Vector3d& p_velo) const {
  const Eigen::Matrix3d R = T_velo_cam.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T_velo_cam.block<3, 1>(0, 3);
  return R * p_velo + t;
}

bool CameraModel::projectVelo(const Eigen::Vector3d& p_velo, Eigen::Vector2d* uv,
                              double* depth_cam) const {
  const Eigen::Vector3d p_cam = veloToCam(p_velo);
  if (p_cam.z() <= 0.1) return false;
  uv->x() = fx * p_cam.x() / p_cam.z() + cx;
  uv->y() = fy * p_cam.y() / p_cam.z() + cy;
  if (uv->x() < 0 || uv->x() >= width || uv->y() < 0 || uv->y() >= height) {
    return false;
  }
  *depth_cam = p_cam.z();
  return true;
}

Eigen::Vector3d CameraModel::backProject(const Eigen::Vector2d& uv,
                                         double depth) const {
  const double x = (uv.x() - cx) / fx * depth;
  const double y = (uv.y() - cy) / fy * depth;
  return Eigen::Vector3d(x, y, depth);
}

RangeImage PlLoam::buildRangeImage(const std::vector<Eigen::Vector3d>& points,
                                   const std::vector<float>& intensity,
                                   const CameraModel& camera) {
  RangeImage img;
  img.width = camera.width;
  img.height = camera.height;
  const size_t n = static_cast<size_t>(img.width * img.height);
  img.depth.assign(n, 0.0f);
  img.intensity.assign(n, 0.0f);

  for (size_t i = 0; i < points.size(); ++i) {
    Eigen::Vector2d uv;
    double depth = 0.0;
    if (!camera.projectVelo(points[i], &uv, &depth)) continue;
    const int u = static_cast<int>(std::round(uv.x()));
    const int v = static_cast<int>(std::round(uv.y()));
    if (!inImage(u, v, img.width, img.height)) continue;
    const int idx = pixelIndex(u, v, img.width);
    const float inten =
        i < intensity.size() ? intensity[i] : static_cast<float>(depth * 0.01);
    if (img.depth[idx] == 0.0f || depth < img.depth[idx]) {
      img.depth[idx] = static_cast<float>(depth);
      img.intensity[idx] = inten;
    }
  }
  return img;
}

bool PlLoam::extractPointDepth(const RangeImage& image,
                               const Eigen::Vector2d& uv, int patch_radius,
                               double min_depth, double max_depth,
                               double* depth_out) {
  std::vector<double> samples;
  const int u0 = static_cast<int>(std::round(uv.x()));
  const int v0 = static_cast<int>(std::round(uv.y()));
  for (int dv = -patch_radius; dv <= patch_radius; ++dv) {
    for (int du = -patch_radius; du <= patch_radius; ++du) {
      const int u = u0 + du;
      const int v = v0 + dv;
      if (!inImage(u, v, image.width, image.height)) continue;
      const float d = image.depth[pixelIndex(u, v, image.width)];
      if (d >= min_depth && d <= max_depth) samples.push_back(d);
    }
  }
  if (samples.size() < 3) return false;
  *depth_out = medianInPlace(&samples);
  return true;
}

bool PlLoam::extractLineDepths(const RangeImage& image, LineFeature* line,
                               int patch_radius, double min_depth,
                               double max_depth) {
  if (!extractPointDepth(image, line->start, patch_radius, min_depth, max_depth,
                         &line->depth_start)) {
    return false;
  }
  if (!extractPointDepth(image, line->end, patch_radius, min_depth, max_depth,
                         &line->depth_end)) {
    return false;
  }
  line->has_depth = true;
  return true;
}

void PlLoam::detectPointFeatures(const RangeImage& image, int max_features,
                                 int block_size, double harris_k,
                                 std::vector<PointFeature>* features) {
  features->clear();
  if (image.width < 8 || image.height < 8) return;

  const int w = image.width;
  const int h = image.height;
  std::vector<float> response(static_cast<size_t>(w * h), 0.0f);

  for (int y = block_size; y < h - block_size; ++y) {
    for (int x = block_size; x < w - block_size; ++x) {
      const int idx = pixelIndex(x, y, w);
      if (image.depth[idx] <= 0.0f) continue;
      double Ix2 = 0.0, Iy2 = 0.0, Ixy = 0.0;
      for (int dy = -block_size; dy <= block_size; ++dy) {
        for (int dx = -block_size; dx <= block_size; ++dx) {
          const int px = x + dx;
          const int py = y + dy;
          if (px < 0 || px >= w - 1 || py < 0 || py >= h - 1) continue;
          const int ix = pixelIndex(px, py, w);
          const int ix1 = pixelIndex(px + 1, py, w);
          const int iy1 = pixelIndex(px, py + 1, w);
          if (image.depth[ix] <= 0.0f || image.depth[ix1] <= 0.0f ||
              image.depth[iy1] <= 0.0f) {
            continue;
          }
          const double gx =
              static_cast<double>(image.depth[ix1] - image.depth[ix]);
          const double gy =
              static_cast<double>(image.depth[iy1] - image.depth[ix]);
          Ix2 += gx * gx;
          Iy2 += gy * gy;
          Ixy += gx * gy;
        }
      }
      const double det = Ix2 * Iy2 - Ixy * Ixy;
      const double trace = Ix2 + Iy2;
      const double r = det - harris_k * trace * trace;
      response[idx] = static_cast<float>(std::max(0.0, r));
    }
  }

  struct Candidate {
    float score;
    int u;
    int v;
  };
  std::vector<Candidate> candidates;
  const int nms = 6;
  for (int y = nms; y < h - nms; y += 2) {
    for (int x = nms; x < w - nms; x += 2) {
      const int idx = pixelIndex(x, y, w);
      const float r = response[idx];
      if (r <= 1e-6f) continue;
      bool is_max = true;
      for (int dy = -nms; dy <= nms && is_max; ++dy) {
        for (int dx = -nms; dx <= nms; ++dx) {
          if (dx == 0 && dy == 0) continue;
          if (response[pixelIndex(x + dx, y + dy, w)] > r) {
            is_max = false;
            break;
          }
        }
      }
      if (is_max) candidates.push_back({r, x, y});
    }
  }

  std::sort(candidates.begin(), candidates.end(),
            [](const Candidate& a, const Candidate& b) {
              return a.score > b.score;
            });
  const int take = std::min(max_features, static_cast<int>(candidates.size()));
  features->reserve(static_cast<size_t>(take));
  for (int i = 0; i < take; ++i) {
    PointFeature f;
    f.uv = Eigen::Vector2d(candidates[i].u, candidates[i].v);
    f.score = candidates[i].score;
    features->push_back(f);
  }
}

void PlLoam::detectLineFeatures(const RangeImage& image, int max_features,
                                std::vector<LineFeature>* features) {
  features->clear();
  if (image.width < 16 || image.height < 16) return;

  const int w = image.width;
  const int h = image.height;
  std::vector<float> gx(static_cast<size_t>(w * h), 0.0f);
  std::vector<float> gy(static_cast<size_t>(w * h), 0.0f);
  std::vector<float> mag(static_cast<size_t>(w * h), 0.0f);

  for (int y = 1; y < h - 1; ++y) {
    for (int x = 1; x < w - 1; ++x) {
      const int idx = pixelIndex(x, y, w);
      if (image.depth[idx] <= 0.0f) continue;
      const float sx =
          image.depth[pixelIndex(x + 1, y, w)] -
          image.depth[pixelIndex(x - 1, y, w)];
      const float sy =
          image.depth[pixelIndex(x, y + 1, w)] -
          image.depth[pixelIndex(x, y - 1, w)];
      gx[idx] = sx;
      gy[idx] = sy;
      mag[idx] = std::sqrt(sx * sx + sy * sy);
    }
  }

  struct SegmentCandidate {
    float score;
    LineFeature line;
  };
  std::vector<SegmentCandidate> candidates;

  for (int y = 4; y < h - 4; y += 3) {
    for (int x = 4; x < w - 4; x += 3) {
      const int idx = pixelIndex(x, y, w);
      if (mag[idx] < 0.05f) continue;
      const float theta = std::atan2(gy[idx], gx[idx]);
      const float c = std::cos(theta);
      const float s = std::sin(theta);
      int u0 = x, v0 = y, u1 = x, v1 = y;
      float len = 0.0f;
      for (int step = 1; step < 40; ++step) {
        const int ux = static_cast<int>(std::round(x + c * step));
        const int uy = static_cast<int>(std::round(y + s * step));
        if (!inImage(ux, uy, w, h)) break;
        const int j = pixelIndex(ux, uy, w);
        if (image.depth[j] <= 0.0f || mag[j] < 0.03f) break;
        u1 = ux;
        v1 = uy;
        len += mag[j];
      }
      for (int step = 1; step < 40; ++step) {
        const int ux = static_cast<int>(std::round(x - c * step));
        const int uy = static_cast<int>(std::round(y - s * step));
        if (!inImage(ux, uy, w, h)) break;
        const int j = pixelIndex(ux, uy, w);
        if (image.depth[j] <= 0.0f || mag[j] < 0.03f) break;
        u0 = ux;
        v0 = uy;
        len += mag[j];
      }
      const double seg_len =
          std::hypot(u1 - u0, v1 - v0);
      if (seg_len < 12.0) continue;
      SegmentCandidate cand;
      cand.score = len;
      cand.line.start = Eigen::Vector2d(u0, v0);
      cand.line.end = Eigen::Vector2d(u1, v1);
      candidates.push_back(cand);
    }
  }

  std::sort(candidates.begin(), candidates.end(),
            [](const SegmentCandidate& a, const SegmentCandidate& b) {
              return a.score > b.score;
            });
  const int take = std::min(max_features, static_cast<int>(candidates.size()));
  for (int i = 0; i < take; ++i) {
    candidates[i].line.score = candidates[i].score;
    features->push_back(candidates[i].line);
  }
}

double PlLoam::scaleCorrectionFactor(
    const std::vector<double>& lidar_depths,
    const std::vector<double>& visual_depths) {
  if (lidar_depths.size() != visual_depths.size() || lidar_depths.empty()) {
    return 1.0;
  }
  std::vector<double> ratios;
  ratios.reserve(lidar_depths.size());
  for (size_t i = 0; i < lidar_depths.size(); ++i) {
    if (visual_depths[i] < 0.5 || lidar_depths[i] < 0.5) continue;
    ratios.push_back(lidar_depths[i] / visual_depths[i]);
  }
  if (ratios.size() < 3) return 1.0;
  return medianInPlace(&ratios);
}

bool PlLoam::optimizeRelativePose(
    const CameraModel& camera, const PlLoamParams& params,
    const std::vector<PointFeature>& prev_points,
    const std::vector<PointFeature>& curr_points,
    const std::vector<LineFeature>& prev_lines,
    const std::vector<LineFeature>& curr_lines,
    const std::vector<std::pair<int, int>>& point_matches,
    const std::vector<std::pair<int, int>>& line_matches,
    Eigen::Matrix4d* T_prev_curr, double* scale_correction,
    size_t* num_depth_priors, double* mean_depth_prior_residual,
    const Eigen::Matrix4d& init_T) {
  if (T_prev_curr == nullptr) return false;
  *num_depth_priors = 0;
  *mean_depth_prior_residual = 0.0;
  *scale_correction = 1.0;

  if (point_matches.size() + line_matches.size() < 6) return false;

  std::vector<double> lidar_depths;
  std::vector<double> visual_depths;
  lidar_depths.reserve(point_matches.size());
  visual_depths.reserve(point_matches.size());

  ceres::Problem problem;
  Eigen::Quaterniond q_init(init_T.block<3, 3>(0, 0));
  q_init.normalize();
  double q[4] = {q_init.x(), q_init.y(), q_init.z(), q_init.w()};
  double t[3] = {init_T(0, 3), init_T(1, 3), init_T(2, 3)};
  const double reproj_w = 1.0 / params.reproj_sigma_px;
  const double line_w = 1.0 / params.line_sigma_px;
  const double depth_w =
      params.use_depth_prior ? params.depth_prior_weight : 0.0;

  double depth_residual_acc = 0.0;
  size_t depth_count = 0;

  for (const auto& m : point_matches) {
    const PointFeature& prev = prev_points[m.first];
    const PointFeature& curr = curr_points[m.second];
    if (!prev.has_depth || !curr.has_depth) continue;
    const Eigen::Vector3d p_prev =
        camera.backProject(prev.uv, prev.depth_prior);
    problem.AddResidualBlock(
        PointReprojFactor::Create(p_prev, curr.uv, camera.fx, camera.fy,
                                  camera.cx, camera.cy, reproj_w),
        new ceres::HuberLoss(2.0), q, t);
    if (depth_w > 0.0) {
      problem.AddResidualBlock(
          PointDepthPriorFactor::Create(p_prev, curr.depth_prior, depth_w),
          nullptr, q, t);
      depth_residual_acc += std::abs(curr.depth_prior - prev.depth_prior);
      depth_count++;
    }
    visual_depths.push_back(prev.depth_prior);
    lidar_depths.push_back(curr.depth_prior);
  }

  for (const auto& m : line_matches) {
    const LineFeature& prev = prev_lines[m.first];
    const LineFeature& curr = curr_lines[m.second];
    if (!prev.has_depth || !curr.has_depth) continue;
    const Eigen::Vector3d ps =
        camera.backProject(prev.start, prev.depth_start);
    const Eigen::Vector3d pe = camera.backProject(prev.end, prev.depth_end);
    problem.AddResidualBlock(
        LineEndpointReprojFactor::Create(ps, curr.start, camera.fx, camera.fy,
                                         camera.cx, camera.cy, line_w),
        new ceres::HuberLoss(2.0), q, t);
    problem.AddResidualBlock(
        LineEndpointReprojFactor::Create(pe, curr.end, camera.fx, camera.fy,
                                         camera.cx, camera.cy, line_w),
        new ceres::HuberLoss(2.0), q, t);
  }

  if (problem.NumResidualBlocks() < 4) return false;

  ceres::Solver::Options options;
  options.linear_solver_type = ceres::DENSE_QR;
  options.max_num_iterations = params.ceres_max_iterations;
  options.minimizer_progress_to_stdout = false;
  options.logging_type = ceres::SILENT;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  if (!summary.IsSolutionUsable()) return false;

  Eigen::Quaterniond q_e(q[3], q[0], q[1], q[2]);
  q_e.normalize();
  Eigen::Vector3d t_e(t[0], t[1], t[2]);

  if (params.use_scale_correction) {
    *scale_correction = scaleCorrectionFactor(lidar_depths, visual_depths);
    t_e *= *scale_correction;
  }

  T_prev_curr->setIdentity();
  T_prev_curr->block<3, 3>(0, 0) = q_e.toRotationMatrix();
  T_prev_curr->block<3, 1>(0, 3) = t_e;

  *num_depth_priors = depth_count;
  if (depth_count > 0) {
    *mean_depth_prior_residual = depth_residual_acc / depth_count;
  }
  return true;
}

PlLoam::PlLoam(const PlLoamParams& params) : params_(params) {}

void PlLoam::clear() {
  prev_points_.clear();
  prev_lines_.clear();
  q_w_curr_ = Eigen::Quaterniond::Identity();
  t_w_curr_ = Eigen::Vector3d::Zero();
  q_last_curr_ = Eigen::Quaterniond::Identity();
  t_last_curr_ = Eigen::Vector3d::Zero();
  initialized_ = false;
  frame_count_ = 0;
}

std::vector<std::pair<int, int>> PlLoam::matchPoints(
    const std::vector<PointFeature>& prev,
    const std::vector<PointFeature>& curr) const {
  std::vector<std::pair<int, int>> matches;
  const double max_sq = params_.match_max_px * params_.match_max_px;
  for (size_t i = 0; i < curr.size(); ++i) {
    int best = -1;
    double best_d = max_sq;
    for (size_t j = 0; j < prev.size(); ++j) {
      const double d = (curr[i].uv - prev[j].uv).squaredNorm();
      if (d < best_d) {
        best_d = d;
        best = static_cast<int>(j);
      }
    }
    if (best >= 0) matches.emplace_back(best, static_cast<int>(i));
  }
  return matches;
}

std::vector<std::pair<int, int>> PlLoam::matchLines(
    const std::vector<LineFeature>& prev,
    const std::vector<LineFeature>& curr) const {
  std::vector<std::pair<int, int>> matches;
  const double max_sq = (params_.match_max_px * 2.0) * (params_.match_max_px * 2.0);
  for (size_t i = 0; i < curr.size(); ++i) {
    int best = -1;
    double best_d = max_sq;
    const Eigen::Vector2d c_curr =
        0.5 * (curr[i].start + curr[i].end);
    for (size_t j = 0; j < prev.size(); ++j) {
      const Eigen::Vector2d c_prev = 0.5 * (prev[j].start + prev[j].end);
      const double d = (c_curr - c_prev).squaredNorm();
      if (d < best_d) {
        best_d = d;
        best = static_cast<int>(j);
      }
    }
    if (best >= 0) matches.emplace_back(best, static_cast<int>(i));
  }
  return matches;
}

PlLoamResult PlLoam::process(const std::vector<Eigen::Vector3d>& points,
                             const std::vector<float>& intensity) {
  PlLoamResult result;
  result.frame_count = frame_count_;

  std::vector<Eigen::Vector3d> subsampled;
  std::vector<float> sub_intensity;
  subsampled.reserve(points.size() / params_.input_stride + 1);
  sub_intensity.reserve(points.size() / params_.input_stride + 1);
  for (size_t i = 0; i < points.size(); i += params_.input_stride) {
    subsampled.push_back(points[i]);
    sub_intensity.push_back(i < intensity.size() ? intensity[i] : 0.5f);
  }

  const RangeImage image =
      buildRangeImage(subsampled, sub_intensity, params_.camera);

  std::vector<PointFeature> points_feat;
  detectPointFeatures(image, params_.max_point_features, params_.harris_block,
                      params_.harris_k, &points_feat);
  for (auto& f : points_feat) {
    f.has_depth = extractPointDepth(image, f.uv, params_.patch_radius,
                                    params_.min_depth, params_.max_depth,
                                    &f.depth_prior);
  }

  std::vector<LineFeature> lines_feat;
  if (params_.use_line_features) {
    detectLineFeatures(image, params_.max_line_features, &lines_feat);
    for (auto& l : lines_feat) {
      extractLineDepths(image, &l, params_.patch_radius, params_.min_depth,
                        params_.max_depth);
    }
  }

  result.num_points = points_feat.size();
  result.num_lines = lines_feat.size();

  if (!initialized_) {
    prev_points_ = points_feat;
    prev_lines_ = lines_feat;
    initialized_ = true;
    frame_count_++;
    result.valid = true;
    result.frame_count = frame_count_;
    return result;
  }

  const auto point_matches = matchPoints(prev_points_, points_feat);
  const auto line_matches =
      params_.use_line_features ? matchLines(prev_lines_, lines_feat)
                                : std::vector<std::pair<int, int>>{};

  result.num_point_matches = point_matches.size();
  result.num_line_matches = line_matches.size();

  Eigen::Matrix4d T_cam_prev_curr = Eigen::Matrix4d::Identity();
  T_cam_prev_curr.block<3, 3>(0, 0) = q_last_curr_.toRotationMatrix();
  T_cam_prev_curr.block<3, 1>(0, 3) = t_last_curr_;

  size_t num_depth_priors = 0;
  double mean_depth_residual = 0.0;
  double scale = 1.0;
  const bool ok = optimizeRelativePose(
      params_.camera, params_, prev_points_, points_feat, prev_lines_,
      lines_feat, point_matches, line_matches, &T_cam_prev_curr, &scale,
      &num_depth_priors, &mean_depth_residual, T_cam_prev_curr);

  if (!ok) {
    T_cam_prev_curr.setIdentity();
    T_cam_prev_curr.block<3, 3>(0, 0) = q_last_curr_.toRotationMatrix();
    T_cam_prev_curr.block<3, 1>(0, 3) = t_last_curr_;
    scale = 1.0;
  }

  q_last_curr_ = Eigen::Quaterniond(T_cam_prev_curr.block<3, 3>(0, 0));
  t_last_curr_ = T_cam_prev_curr.block<3, 1>(0, 3);

  const Eigen::Matrix4d T_velo_cam = params_.camera.T_velo_cam;
  const Eigen::Matrix4d T_cam_velo = T_velo_cam.inverse();
  const Eigen::Matrix4d T_velo_prev_curr =
      T_cam_velo * T_cam_prev_curr * T_velo_cam;

  Eigen::Matrix4d T_world_curr = Eigen::Matrix4d::Identity();
  T_world_curr.block<3, 3>(0, 0) = q_w_curr_.toRotationMatrix();
  T_world_curr.block<3, 1>(0, 3) = t_w_curr_;
  T_world_curr = T_world_curr * T_velo_prev_curr;

  q_w_curr_ = Eigen::Quaterniond(T_world_curr.block<3, 3>(0, 0));
  t_w_curr_ = T_world_curr.block<3, 1>(0, 3);

  prev_points_ = std::move(points_feat);
  prev_lines_ = std::move(lines_feat);
  frame_count_++;

  result.q_w_curr = q_w_curr_;
  result.t_w_curr = t_w_curr_;
  result.valid = ok;
  result.frame_count = frame_count_;
  result.scale_correction = scale;
  result.num_depth_priors = num_depth_priors;
  result.mean_depth_prior_residual = mean_depth_residual;
  return result;
}

}  // namespace pl_loam
}  // namespace localization_zoo
