#include "opl_lvio/opl_lvio.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <unordered_set>

namespace localization_zoo {
namespace opl_lvio {

namespace {

constexpr double kPi = 3.14159265358979323846;

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0.0, -v.z(), v.y(), v.z(), 0.0, -v.x(), -v.y(), v.x(), 0.0;
  return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& w) {
  const double theta = w.norm();
  if (theta < 1e-10) return Eigen::Matrix3d::Identity() + skew(w);
  const Eigen::Matrix3d K = skew(w / theta);
  return Eigen::Matrix3d::Identity() + std::sin(theta) * K +
         (1.0 - std::cos(theta)) * K * K;
}

Eigen::Matrix4d expSE3(const Eigen::Matrix<double, 6, 1>& xi) {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = expSO3(xi.head<3>());
  T.block<3, 1>(0, 3) = xi.tail<3>();
  return T;
}

std::vector<Eigen::Vector3d> transformPoints(
    const std::vector<Eigen::Vector3d>& points, const Eigen::Matrix4d& T) {
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) out.push_back(R * p + t);
  return out;
}

LineFeature transformLine(const LineFeature& line, const Eigen::Matrix4d& T) {
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  LineFeature out = line;
  out.center = R * line.center + t;
  out.direction = (R * line.direction).normalized();
  return out;
}

PointFeature transformPointFeature(const PointFeature& point,
                                   const Eigen::Matrix4d& T) {
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  PointFeature out = point;
  out.point = R * point.point + t;
  return out;
}

double clampScale(double value, double lo, double hi) {
  if (!std::isfinite(value)) return 1.0;
  return std::clamp(value, lo, hi);
}

}  // namespace

OplLvioPipeline::OplLvioPipeline(const OplLvioParams& params) : params_(params) {}

void OplLvioPipeline::reset() {
  frame_count_ = 0;
  pose_.setIdentity();
  last_delta_.setIdentity();
  map_points_.clear();
  visual_points_.clear();
  map_lines_.clear();
  point_voxels_.clear();
  visual_point_voxels_.clear();
  line_voxels_.clear();
}

size_t OplLvioPipeline::mapSize() const {
  return static_cast<size_t>(std::count_if(
      map_points_.begin(), map_points_.end(),
      [](const MapPoint& p) { return !p.removed; }));
}

size_t OplLvioPipeline::lineMapSize() const {
  return static_cast<size_t>(std::count_if(
      map_lines_.begin(), map_lines_.end(),
      [](const MapLine& l) { return !l.removed; }));
}

Eigen::Vector3i OplLvioPipeline::voxelKey(const Eigen::Vector3d& p) const {
  return Eigen::Vector3i(
      static_cast<int>(std::floor(p.x() / params_.voxel_size)),
      static_cast<int>(std::floor(p.y() / params_.voxel_size)),
      static_cast<int>(std::floor(p.z() / params_.voxel_size)));
}

int OplLvioPipeline::imageIndex(int row, int col) const {
  col = ((col % params_.image_width) + params_.image_width) % params_.image_width;
  return row * params_.image_width + col;
}

bool OplLvioPipeline::projectToSpherical(const Eigen::Vector3d& point, int* row,
                                        int* col, double* range) const {
  const double r = point.norm();
  if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) {
    return false;
  }
  const double yaw = std::atan2(point.y(), point.x());
  const double pitch = std::asin(std::clamp(point.z() / r, -1.0, 1.0));
  const double fov_up = params_.fov_up_deg * kPi / 180.0;
  const double fov_down = params_.fov_down_deg * kPi / 180.0;
  if (pitch > fov_up || pitch < fov_down || fov_up <= fov_down) return false;

  int c = static_cast<int>(
      std::floor(0.5 * (yaw / kPi + 1.0) * params_.image_width));
  int rr = static_cast<int>(
      std::floor(((fov_up - pitch) / (fov_up - fov_down)) *
                 params_.image_height));
  c = ((c % params_.image_width) + params_.image_width) % params_.image_width;
  if (rr < 0 || rr >= params_.image_height) return false;
  if (row) *row = rr;
  if (col) *col = c;
  if (range) *range = r;
  return true;
}

std::vector<Eigen::Vector3d> OplLvioPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

std::vector<Eigen::Vector3d> OplLvioPipeline::voxelDownsample(
    const std::vector<Eigen::Vector3d>& points, double voxel_size) const {
  if (voxel_size <= 0.0) return points;
  std::unordered_map<Eigen::Vector3i, Eigen::Vector3d, VoxelHash> grid;
  grid.reserve(points.size());
  for (const auto& p : points) {
    const Eigen::Vector3i key(
        static_cast<int>(std::floor(p.x() / voxel_size)),
        static_cast<int>(std::floor(p.y() / voxel_size)),
        static_cast<int>(std::floor(p.z() / voxel_size)));
    grid.emplace(key, p);
  }
  std::vector<Eigen::Vector3d> out;
  out.reserve(grid.size());
  for (const auto& kv : grid) out.push_back(kv.second);
  return out;
}

std::vector<OplLvioPipeline::RangeCell> OplLvioPipeline::buildRangeImage(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<RangeCell> image(
      static_cast<size_t>(params_.image_width) * params_.image_height);
  for (const auto& p : points) {
    int row = 0, col = 0;
    double r = 0.0;
    if (!projectToSpherical(p, &row, &col, &r)) continue;
    RangeCell& cell = image[static_cast<size_t>(imageIndex(row, col))];
    if (!cell.valid || r < cell.range) {
      cell.point = p;
      cell.range = r;
      cell.valid = true;
    }
  }
  return image;
}

std::vector<PointFeature> OplLvioPipeline::extractVisualPoints(
    const std::vector<RangeCell>& image) const {
  std::vector<PointFeature> points;
  points.reserve(static_cast<size_t>(params_.max_visual_points));
  const int gap = std::max(1, params_.visual_point_neighbor_gap);

  for (int row = gap; row + gap < params_.image_height; ++row) {
    for (int col = 0; col < params_.image_width; ++col) {
      const RangeCell& c = image[static_cast<size_t>(imageIndex(row, col))];
      if (!c.valid || c.range <= 1e-6) continue;
      const RangeCell& left =
          image[static_cast<size_t>(imageIndex(row, col - gap))];
      const RangeCell& right =
          image[static_cast<size_t>(imageIndex(row, col + gap))];
      const RangeCell& up =
          image[static_cast<size_t>(imageIndex(row - gap, col))];
      const RangeCell& down =
          image[static_cast<size_t>(imageIndex(row + gap, col))];
      if (!left.valid || !right.valid || !up.valid || !down.valid) continue;

      const double horizontal =
          std::abs(left.range + right.range - 2.0 * c.range) / c.range;
      const double vertical =
          std::abs(up.range + down.range - 2.0 * c.range) / c.range;
      const double curvature = std::max(horizontal, vertical);
      if (curvature < params_.visual_point_curvature_threshold) continue;
      PointFeature point;
      point.point = c.point;
      point.weight = std::clamp(
          curvature / std::max(1e-6, params_.visual_point_curvature_threshold),
          0.5, 4.0);
      points.push_back(point);
    }
  }

  std::sort(points.begin(), points.end(),
            [](const PointFeature& a, const PointFeature& b) {
              return a.weight > b.weight;
            });
  if (static_cast<int>(points.size()) > params_.max_visual_points) {
    points.resize(static_cast<size_t>(params_.max_visual_points));
  }
  return points;
}

bool OplLvioPipeline::fitLine(const std::vector<Eigen::Vector3d>& points,
                             double min_linearity, LineFeature* line) const {
  if (static_cast<int>(points.size()) < params_.min_line_neighbors) return false;
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  for (const auto& p : points) mean += p;
  mean /= static_cast<double>(points.size());

  Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
  for (const auto& p : points) {
    const Eigen::Vector3d d = p - mean;
    cov += d * d.transpose();
  }
  cov /= static_cast<double>(points.size());
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
  if (es.info() != Eigen::Success) return false;
  const Eigen::Vector3d evals = es.eigenvalues();
  if (evals.z() < 1e-8) return false;
  const double linearity = (evals.z() - evals.y()) / evals.z();
  if (linearity < min_linearity) return false;

  line->center = mean;
  line->direction = es.eigenvectors().col(2).normalized();
  line->weight = std::clamp(linearity, 0.1, 1.0);
  return line->center.allFinite() && line->direction.allFinite();
}

bool OplLvioPipeline::fitPlane(const std::vector<Eigen::Vector3d>& points,
                              Eigen::Vector3d* center,
                              Eigen::Vector3d* normal) const {
  if (static_cast<int>(points.size()) < params_.normal_min_neighbors) return false;
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  for (const auto& p : points) mean += p;
  mean /= static_cast<double>(points.size());

  Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
  for (const auto& p : points) {
    const Eigen::Vector3d d = p - mean;
    cov += d * d.transpose();
  }
  cov /= static_cast<double>(points.size());
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
  if (es.info() != Eigen::Success) return false;
  const Eigen::Vector3d evals = es.eigenvalues();
  const double sum = evals.sum();
  if (sum <= 1e-10 || evals.x() / sum > 0.25) return false;
  *center = mean;
  *normal = es.eigenvectors().col(0).normalized();
  return center->allFinite() && normal->allFinite();
}

std::vector<LineFeature> OplLvioPipeline::extractLidarLines(
    const std::vector<RangeCell>& image) const {
  std::vector<LineFeature> lines;
  lines.reserve(static_cast<size_t>(params_.max_lidar_lines));
  const int row_step = std::max(1, params_.lidar_line_window_rows);
  const int col_step = std::max(2, params_.lidar_line_window_cols);

  for (int row = params_.lidar_line_window_rows;
       row + params_.lidar_line_window_rows < params_.image_height;
       row += row_step) {
    for (int col = 0; col < params_.image_width; col += col_step) {
      const RangeCell& center =
          image[static_cast<size_t>(imageIndex(row, col))];
      if (!center.valid) continue;
      std::vector<Eigen::Vector3d> neighbors;
      neighbors.reserve(64);
      for (int dr = -params_.lidar_line_window_rows;
           dr <= params_.lidar_line_window_rows; ++dr) {
        const int rr = row + dr;
        if (rr < 0 || rr >= params_.image_height) continue;
        for (int dc = -params_.lidar_line_window_cols;
             dc <= params_.lidar_line_window_cols; ++dc) {
          const RangeCell& cell =
              image[static_cast<size_t>(imageIndex(rr, col + dc))];
          if (!cell.valid) continue;
          if ((cell.point - center.point).norm() <=
              params_.max_correspondence_dist * 1.5) {
            neighbors.push_back(cell.point);
          }
        }
      }
      LineFeature line;
      if (!fitLine(neighbors, params_.line_linearity_threshold, &line)) continue;
      line.visual_supported = false;
      lines.push_back(line);
      if (static_cast<int>(lines.size()) >= params_.max_lidar_lines) return lines;
    }
  }
  return lines;
}

std::vector<LineFeature> OplLvioPipeline::extractVisualLines(
    const std::vector<RangeCell>& image) const {
  std::vector<LineFeature> lines;
  lines.reserve(static_cast<size_t>(params_.max_visual_lines));

  for (int row = 0; row < params_.image_height; ++row) {
    std::vector<Eigen::Vector3d> segment;
    segment.reserve(64);
    double prev_range = 0.0;
    int prev_col = -1000000;
    auto flush = [&]() {
      if (static_cast<int>(segment.size()) >= params_.visual_min_segment_pixels) {
        LineFeature line;
        if (fitLine(segment, params_.visual_linearity_threshold, &line)) {
          line.visual_supported = true;
          line.weight *= params_.visual_line_bonus;
          lines.push_back(line);
        }
      }
      segment.clear();
    };

    for (int col = 0; col < params_.image_width; ++col) {
      const RangeCell& cell =
          image[static_cast<size_t>(imageIndex(row, col))];
      if (!cell.valid) continue;
      const bool contiguous = col - prev_col <= params_.visual_max_pixel_gap;
      const bool smooth =
          segment.empty() || std::abs(cell.range - prev_range) <=
                                 params_.visual_range_jump;
      if (!contiguous || !smooth) flush();
      segment.push_back(cell.point);
      prev_range = cell.range;
      prev_col = col;
      if (static_cast<int>(lines.size()) >= params_.max_visual_lines) return lines;
    }
    flush();
    if (static_cast<int>(lines.size()) >= params_.max_visual_lines) return lines;
  }
  return lines;
}

std::vector<LineFeature> OplLvioPipeline::fuseLines(
    const std::vector<LineFeature>& lidar_lines,
    const std::vector<LineFeature>& visual_lines) const {
  std::vector<LineFeature> fused;
  fused.reserve(lidar_lines.size() + visual_lines.size());
  std::vector<char> used_visual(visual_lines.size(), 0);

  for (const auto& lidar : lidar_lines) {
    int lr = 0, lc = 0;
    if (!projectToSpherical(lidar.center, &lr, &lc)) continue;
    int best = -1;
    double best_dist = params_.fusion_pixel_gate * params_.fusion_pixel_gate;
    for (size_t i = 0; i < visual_lines.size(); ++i) {
      int vr = 0, vc = 0;
      if (!projectToSpherical(visual_lines[i].center, &vr, &vc)) continue;
      int dc = std::abs(lc - vc);
      dc = std::min(dc, params_.image_width - dc);
      const double dist = static_cast<double>((lr - vr) * (lr - vr) + dc * dc);
      if (dist < best_dist) {
        best_dist = dist;
        best = static_cast<int>(i);
      }
    }

    LineFeature line = lidar;
    if (best >= 0) {
      const LineFeature& visual = visual_lines[static_cast<size_t>(best)];
      Eigen::Vector3d vdir = visual.direction;
      if (vdir.dot(line.direction) < 0.0) vdir = -vdir;
      line.direction = (line.direction * lidar.weight + vdir * visual.weight)
                           .normalized();
      line.center = 0.65 * lidar.center + 0.35 * visual.center;
      line.weight = std::min(2.0, 0.5 * (lidar.weight + visual.weight) +
                                      params_.visual_line_bonus);
      line.visual_supported = true;
      used_visual[static_cast<size_t>(best)] = 1;
    }
    fused.push_back(line);
  }

  for (size_t i = 0; i < visual_lines.size(); ++i) {
    if (used_visual[i]) continue;
    LineFeature line = visual_lines[i];
    line.weight *= 0.5;
    fused.push_back(line);
  }
  if (static_cast<int>(fused.size()) > params_.max_lidar_lines + params_.max_visual_lines) {
    fused.resize(static_cast<size_t>(params_.max_lidar_lines + params_.max_visual_lines));
  }
  return fused;
}

FrameFeatures OplLvioPipeline::extractFeatures(
    const std::vector<Eigen::Vector3d>& frame) const {
  FrameFeatures features;
  const auto filtered = rangeFilter(frame);
  features.registration_points =
      voxelDownsample(filtered, params_.registration_voxel_size);
  if (static_cast<int>(features.registration_points.size()) >
      params_.max_registration_points) {
    features.registration_points.resize(
        static_cast<size_t>(params_.max_registration_points));
  }
  const auto image = buildRangeImage(filtered);
  features.visual_points = extractVisualPoints(image);
  features.lidar_lines = extractLidarLines(image);
  features.visual_lines = extractVisualLines(image);
  features.fused_lines = fuseLines(features.lidar_lines, features.visual_lines);
  if (static_cast<int>(features.visual_lines.size()) < params_.min_visual_lines &&
      static_cast<int>(features.visual_points.size()) < params_.min_visual_points) {
    features.fused_lines = features.lidar_lines;
  }
  return features;
}

OplLvioPipeline::PlaneMatch OplLvioPipeline::queryPlane(
    const Eigen::Vector3d& world_point) const {
  PlaneMatch out;
  const Eigen::Vector3i key = voxelKey(world_point);
  const double max_dist_sq =
      params_.max_correspondence_dist * params_.max_correspondence_dist;
  std::vector<Eigen::Vector3d> neighbors;
  neighbors.reserve(64);
  Eigen::Vector3d nearest = Eigen::Vector3d::Zero();
  double best_dist = max_dist_sq;

  for (int dx = -1; dx <= 1; ++dx)
    for (int dy = -1; dy <= 1; ++dy)
      for (int dz = -1; dz <= 1; ++dz) {
        auto it = point_voxels_.find(key + Eigen::Vector3i(dx, dy, dz));
        if (it == point_voxels_.end()) continue;
        for (size_t idx : it->second) {
          const MapPoint& mp = map_points_[idx];
          if (mp.removed) continue;
          const double d = (mp.point - world_point).squaredNorm();
          if (d > max_dist_sq) continue;
          neighbors.push_back(mp.point);
          if (d < best_dist) {
            best_dist = d;
            nearest = mp.point;
          }
        }
      }
  if (neighbors.size() < static_cast<size_t>(params_.normal_min_neighbors)) {
    return out;
  }
  if (fitPlane(neighbors, &out.anchor, &out.normal)) {
    if (out.normal.dot(world_point - out.anchor) > 0.0) out.normal = -out.normal;
    out.found = true;
  } else if (best_dist < max_dist_sq) {
    out.anchor = nearest;
  }
  return out;
}

OplLvioPipeline::PointMatch OplLvioPipeline::queryVisualPoint(
    const Eigen::Vector3d& world_point) const {
  PointMatch out;
  const Eigen::Vector3i key = voxelKey(world_point);
  const double max_dist_sq =
      params_.max_correspondence_dist * params_.max_correspondence_dist;
  double best_dist = max_dist_sq;

  for (int dx = -1; dx <= 1; ++dx)
    for (int dy = -1; dy <= 1; ++dy)
      for (int dz = -1; dz <= 1; ++dz) {
        auto it =
            visual_point_voxels_.find(key + Eigen::Vector3i(dx, dy, dz));
        if (it == visual_point_voxels_.end()) continue;
        for (size_t idx : it->second) {
          const VisualMapPoint& mp = visual_points_[idx];
          if (mp.removed) continue;
          const double d = (mp.point - world_point).squaredNorm();
          if (d < best_dist) {
            best_dist = d;
            out.anchor = mp.point;
            out.weight = std::clamp(0.5 + 0.1 * mp.observations, 0.5, 2.5);
            out.found = true;
          }
        }
      }
  return out;
}

OplLvioPipeline::LineMatch OplLvioPipeline::queryLine(
    const Eigen::Vector3d& world_point,
    const Eigen::Vector3d& source_direction_world) const {
  LineMatch out;
  const Eigen::Vector3i key = voxelKey(world_point);
  const double max_dist_sq =
      params_.max_correspondence_dist * params_.max_correspondence_dist;
  double best_score = std::numeric_limits<double>::infinity();

  for (int dx = -1; dx <= 1; ++dx)
    for (int dy = -1; dy <= 1; ++dy)
      for (int dz = -1; dz <= 1; ++dz) {
        auto it = line_voxels_.find(key + Eigen::Vector3i(dx, dy, dz));
        if (it == line_voxels_.end()) continue;
        for (size_t idx : it->second) {
          const MapLine& ml = map_lines_[idx];
          if (ml.removed) continue;
          const Eigen::Vector3d d = world_point - ml.center;
          const double lateral =
              (d - ml.direction * ml.direction.dot(d)).squaredNorm();
          if (lateral > max_dist_sq) continue;
          const double align =
              std::abs(ml.direction.normalized().dot(source_direction_world));
          if (align < 0.35) continue;
          const double score = lateral / std::max(0.1, align);
          if (score < best_score) {
            best_score = score;
            out.anchor = ml.center;
            out.direction = ml.direction.normalized();
            if (out.direction.dot(source_direction_world) < 0.0) {
              out.direction = -out.direction;
            }
            out.weight = std::clamp(0.5 + 0.1 * ml.observations, 0.5, 2.0);
            out.found = true;
          }
        }
      }
  return out;
}

Eigen::Matrix4d OplLvioPipeline::align(const FrameFeatures& features,
                                      const Eigen::Matrix4d& initial_guess,
                                      OplLvioResult* result) const {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;
  Eigen::Matrix4d T = initial_guess;
  const double kernel = std::max(0.1, params_.max_correspondence_dist);
  double point_scale = 1.0;
  double line_scale = 1.0;
  double residual_sum = 0.0;
  int residual_count = 0;

  for (int iter = 0; iter < params_.max_icp_iterations; ++iter) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);
    Mat6 A = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    int plane_count = 0;
    int point_count = 0;
    int line_count = 0;
    double plane_sum2 = 0.0;
    double point_sum2 = 0.0;
    double line_sum2 = 0.0;
    residual_sum = 0.0;
    residual_count = 0;

    for (const auto& p : features.registration_points) {
      const Eigen::Vector3d x = R * p + t;
      const PlaneMatch m = queryPlane(x);
      if (!m.found) continue;
      const double e = m.normal.dot(x - m.anchor);
      if (std::abs(e) > params_.max_correspondence_dist) continue;
      Eigen::Matrix<double, 3, 6> Jx;
      Jx.block<3, 3>(0, 0) = -skew(x);
      Jx.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const Eigen::Matrix<double, 1, 6> J = m.normal.transpose() * Jx;
      const double robust = std::exp(-0.5 * (e / kernel) * (e / kernel));
      const double w = params_.plane_weight * robust;
      A += w * J.transpose() * J;
      b -= w * J.transpose() * e;
      residual_sum += std::abs(e);
      plane_sum2 += e * e;
      ++residual_count;
      ++plane_count;
    }

    for (const auto& point : features.visual_points) {
      const Eigen::Vector3d x = R * point.point + t;
      const PointMatch m = queryVisualPoint(x);
      if (!m.found) continue;
      const Eigen::Vector3d r = x - m.anchor;
      if (r.norm() > params_.max_correspondence_dist) continue;
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(x);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const double robust = std::exp(-0.5 * r.squaredNorm() / (kernel * kernel));
      const double w =
          params_.visual_point_weight * point_scale * m.weight * point.weight *
          robust;
      A += w * J.transpose() * J;
      b -= w * J.transpose() * r;
      residual_sum += r.norm();
      point_sum2 += r.squaredNorm();
      ++residual_count;
      ++point_count;
    }

    for (const auto& line : features.fused_lines) {
      const Eigen::Vector3d x = R * line.center + t;
      Eigen::Vector3d d_src = R * line.direction;
      if (d_src.squaredNorm() < 1e-10) continue;
      d_src.normalize();
      const LineMatch m = queryLine(x, d_src);
      if (!m.found) continue;
      const Eigen::Matrix3d P =
          Eigen::Matrix3d::Identity() - m.direction * m.direction.transpose();
      const Eigen::Vector3d r = P * (x - m.anchor);
      if (r.norm() > params_.max_correspondence_dist) continue;
      Eigen::Matrix<double, 3, 6> Jx;
      Jx.block<3, 3>(0, 0) = -skew(x);
      Jx.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const Eigen::Matrix<double, 3, 6> J = P * Jx;
      const double robust = std::exp(-0.5 * r.squaredNorm() / (kernel * kernel));
      const double w =
          params_.line_weight * line_scale * m.weight * line.weight * robust;
      A += w * J.transpose() * J;
      b -= w * J.transpose() * r;
      residual_sum += r.norm();
      line_sum2 += r.squaredNorm();
      ++residual_count;
      ++line_count;

      const Eigen::Vector3d rd = m.direction.cross(d_src);
      Eigen::Matrix<double, 3, 6> Jd = Eigen::Matrix<double, 3, 6>::Zero();
      Jd.block<3, 3>(0, 0) = skew(m.direction) * (-skew(d_src));
      const double wd =
          params_.direction_weight * line_scale * line.weight * robust;
      A += wd * Jd.transpose() * Jd;
      b -= wd * Jd.transpose() * rd;
      residual_sum += rd.norm();
      line_sum2 += rd.squaredNorm();
      ++residual_count;
    }

    if (params_.motion_prior_weight > 0.0) {
      A += params_.motion_prior_weight * Mat6::Identity();
    }

    const int total = plane_count + point_count + line_count;
    if (total < params_.min_correspondences) break;
    const Vec6 delta = A.ldlt().solve(b);
    if (!delta.allFinite()) break;
    T = expSE3(delta) * T;

    const double plane_var =
        plane_count > 0 ? plane_sum2 / static_cast<double>(plane_count) : 0.0;
    const double point_var =
        point_count > 0 ? point_sum2 / static_cast<double>(point_count * 3) : 0.0;
    const double line_var =
        line_count > 0 ? line_sum2 / static_cast<double>(line_count * 4) : 0.0;
    double ref_var = 0.0;
    int ref_count = 0;
    if (plane_var > 0.0) {
      ref_var += plane_var;
      ++ref_count;
    }
    if (point_var > 0.0) {
      ref_var += point_var;
      ++ref_count;
    }
    if (line_var > 0.0) {
      ref_var += line_var;
      ++ref_count;
    }
    if (ref_count > 0) {
      ref_var /= static_cast<double>(ref_count);
      if (point_var > 0.0) {
        point_scale = 0.5 * point_scale +
                      0.5 * clampScale(ref_var / (point_var + 1e-9),
                                       params_.helmert_min_scale,
                                       params_.helmert_max_scale);
      }
      if (line_var > 0.0) {
        line_scale = 0.5 * line_scale +
                     0.5 * clampScale(ref_var / (line_var + 1e-9),
                                      params_.helmert_min_scale,
                                      params_.helmert_max_scale);
      }
    }

    if (result) {
      result->iterations = iter + 1;
      result->num_plane_correspondences = plane_count;
      result->num_point_correspondences = point_count;
      result->num_line_correspondences = line_count;
      result->point_weight_scale = point_scale;
      result->line_weight_scale = line_scale;
    }
    if (delta.norm() < params_.convergence_criterion) break;
  }
  if (result) {
    result->mean_abs_residual =
        residual_count > 0 ? residual_sum / static_cast<double>(residual_count)
                           : 0.0;
  }
  return T;
}

void OplLvioPipeline::addPoint(const Eigen::Vector3d& point, int frame_index) {
  const Eigen::Vector3i key = voxelKey(point);
  auto& bucket = point_voxels_[key];
  double best_dist = params_.map_update_radius * params_.map_update_radius;
  size_t best_idx = std::numeric_limits<size_t>::max();
  for (size_t idx : bucket) {
    MapPoint& mp = map_points_[idx];
    if (mp.removed) continue;
    const double d = (mp.point - point).squaredNorm();
    if (d < best_dist) {
      best_dist = d;
      best_idx = idx;
    }
  }
  if (best_idx != std::numeric_limits<size_t>::max()) {
    MapPoint& mp = map_points_[best_idx];
    mp.point = 0.8 * mp.point + 0.2 * point;
    mp.stamp = frame_index;
    return;
  }
  if (static_cast<int>(bucket.size()) >= params_.max_points_per_voxel) return;
  MapPoint mp;
  mp.point = point;
  mp.stamp = frame_index;
  map_points_.push_back(mp);
  bucket.push_back(map_points_.size() - 1);
}

void OplLvioPipeline::addVisualPoint(const PointFeature& point,
                                     int frame_index) {
  const Eigen::Vector3i key = voxelKey(point.point);
  auto& bucket = visual_point_voxels_[key];
  double best_dist = params_.map_update_radius * params_.map_update_radius;
  size_t best_idx = std::numeric_limits<size_t>::max();
  for (size_t idx : bucket) {
    VisualMapPoint& mp = visual_points_[idx];
    if (mp.removed) continue;
    const double d = (mp.point - point.point).squaredNorm();
    if (d < best_dist) {
      best_dist = d;
      best_idx = idx;
    }
  }
  if (best_idx != std::numeric_limits<size_t>::max()) {
    VisualMapPoint& mp = visual_points_[best_idx];
    const double alpha =
        std::clamp(1.0 / static_cast<double>(mp.observations + 1), 0.05, 0.35);
    mp.point = (1.0 - alpha) * mp.point + alpha * point.point;
    mp.observations += 1;
    mp.stamp = frame_index;
    return;
  }
  if (static_cast<int>(bucket.size()) >= params_.max_points_per_voxel) return;
  VisualMapPoint mp;
  mp.point = point.point;
  mp.observations = 1;
  mp.stamp = frame_index;
  visual_points_.push_back(mp);
  bucket.push_back(visual_points_.size() - 1);
}

void OplLvioPipeline::addLine(const LineFeature& line, int frame_index) {
  const Eigen::Vector3i key = voxelKey(line.center);
  auto& bucket = line_voxels_[key];
  double best_dist = params_.map_update_radius * params_.map_update_radius * 4.0;
  size_t best_idx = std::numeric_limits<size_t>::max();
  for (size_t idx : bucket) {
    MapLine& ml = map_lines_[idx];
    if (ml.removed) continue;
    const double d = (ml.center - line.center).squaredNorm();
    const double align = std::abs(ml.direction.dot(line.direction));
    if (d < best_dist && align > 0.5) {
      best_dist = d;
      best_idx = idx;
    }
  }
  if (best_idx != std::numeric_limits<size_t>::max()) {
    MapLine& ml = map_lines_[best_idx];
    const double alpha =
        std::clamp(1.0 / static_cast<double>(ml.observations + 1), 0.05, 0.3);
    Eigen::Vector3d dir = line.direction;
    if (dir.dot(ml.direction) < 0.0) dir = -dir;
    ml.center = (1.0 - alpha) * ml.center + alpha * line.center;
    ml.direction = ((1.0 - alpha) * ml.direction + alpha * dir).normalized();
    ml.observations += 1;
    ml.stamp = frame_index;
    return;
  }
  if (static_cast<int>(bucket.size()) >= params_.max_points_per_voxel) return;
  MapLine ml;
  ml.center = line.center;
  ml.direction = line.direction.normalized();
  ml.observations = 1;
  ml.stamp = frame_index;
  map_lines_.push_back(ml);
  bucket.push_back(map_lines_.size() - 1);
}

void OplLvioPipeline::updateMap(const FrameFeatures& features,
                               const Eigen::Matrix4d& pose, int frame_index) {
  for (const auto& p : transformPoints(features.registration_points, pose)) {
    addPoint(p, frame_index);
  }
  for (const auto& point : features.visual_points) {
    addVisualPoint(transformPointFeature(point, pose), frame_index);
  }
  for (const auto& line : features.fused_lines) {
    if (line.direction.squaredNorm() < 1e-10) continue;
    addLine(transformLine(line, pose), frame_index);
  }
}

void OplLvioPipeline::pruneFar(const Eigen::Vector3d& center) {
  if (params_.local_map_radius <= 0.0) return;
  const double r2 = params_.local_map_radius * params_.local_map_radius;
  for (auto& p : map_points_) {
    if (!p.removed && (p.point - center).squaredNorm() > r2) p.removed = true;
  }
  for (auto& p : visual_points_) {
    if (!p.removed && (p.point - center).squaredNorm() > r2) p.removed = true;
  }
  for (auto& l : map_lines_) {
    if (!l.removed && (l.center - center).squaredNorm() > r2) l.removed = true;
  }
}

OplLvioResult OplLvioPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  OplLvioResult result;
  FrameFeatures features = extractFeatures(frame);
  result.num_visual_points = static_cast<int>(features.visual_points.size());
  result.num_visual_lines = static_cast<int>(features.visual_lines.size());
  result.num_fused_lines = static_cast<int>(features.fused_lines.size());
  result.visual_used = result.num_visual_lines >= params_.min_visual_lines ||
                       result.num_visual_points >= params_.min_visual_points;
  result.lidar_fallback = !result.visual_used;

  if (features.registration_points.empty()) {
    result.pose = pose_;
    return result;
  }

  if (frame_count_ == 0) {
    updateMap(features, pose_, frame_count_);
    result.pose = pose_;
    result.converged = true;
    frame_count_++;
    return result;
  }

  const Eigen::Matrix4d prediction = pose_ * last_delta_;
  const Eigen::Matrix4d new_pose = align(features, prediction, &result);
  last_delta_ = pose_.inverse() * new_pose;
  pose_ = new_pose;
  updateMap(features, pose_, frame_count_);
  if (params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    pruneFar(pose_.block<3, 1>(0, 3));
  }
  result.pose = pose_;
  result.converged = pose_.array().isFinite().all() &&
                     (result.num_plane_correspondences +
                      result.num_point_correspondences +
                      result.num_line_correspondences) >=
                         params_.min_correspondences;
  frame_count_++;
  return result;
}

}  // namespace opl_lvio
}  // namespace localization_zoo
