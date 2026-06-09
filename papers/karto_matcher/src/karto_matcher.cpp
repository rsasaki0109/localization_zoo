#include "karto_matcher/karto_matcher.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace karto_matcher {

namespace {

constexpr float kDistInf = 1e6f;

Eigen::Matrix3d matrixFromIncrement(double tx, double ty, double dtheta) {
  Eigen::Matrix3d T = Eigen::Matrix3d::Identity();
  T(0, 0) = std::cos(dtheta);
  T(0, 1) = -std::sin(dtheta);
  T(1, 0) = std::sin(dtheta);
  T(1, 1) = std::cos(dtheta);
  T(0, 2) = tx;
  T(1, 2) = ty;
  return T;
}

Eigen::Vector2d transformPoint(const Eigen::Matrix3d& T, const Eigen::Vector2d& p) {
  return (T.block<2, 2>(0, 0) * p + T.block<2, 1>(0, 2)).eval();
}

}  // namespace

Eigen::Matrix3d pose2D(double x, double y, double yaw) {
  return matrixFromIncrement(x, y, yaw);
}

KartoMatcherEstimator::KartoMatcherEstimator(const KartoMatcherParams& params)
    : params_(params) {}

void KartoMatcherEstimator::reset() {
  initialized_ = false;
  map_points_world_.clear();
  pose_ = Eigen::Matrix3d::Identity();
  last_increment_ = Eigen::Matrix3d::Identity();
}

void KartoMatcherEstimator::setInitialPose(const Eigen::Matrix3d& pose) { pose_ = pose; }

std::vector<Eigen::Vector2d> KartoMatcherEstimator::scanToPoints(const LaserScan& scan) const {
  std::vector<Eigen::Vector2d> points;
  points.reserve(scan.size());
  const double inc =
      scan.angle_increment > 0.0 ? scan.angle_increment
                                 : (scan.size() > 1 ? (scan.angle_max - scan.angle_min) /
                                                          static_cast<double>(scan.size() - 1)
                                                    : 0.0);
  for (size_t i = 0; i < scan.size(); ++i) {
    const double r = scan.ranges[i];
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    const double a = scan.angle_min + static_cast<double>(i) * inc;
    points.emplace_back(r * std::cos(a), r * std::sin(a));
  }
  return points;
}

std::vector<Eigen::Vector2d> KartoMatcherEstimator::localMapPoints() const {
  std::vector<Eigen::Vector2d> local;
  local.reserve(map_points_world_.size());
  const Eigen::Vector2d center(pose_(0, 2), pose_(1, 2));
  const double r2 = params_.local_map_radius * params_.local_map_radius;
  for (const auto& p : map_points_world_) {
    if ((p - center).squaredNorm() <= r2) {
      local.push_back(p);
    }
  }
  return local;
}

KartoMatcherEstimator::Grid KartoMatcherEstimator::buildGrid(
    const std::vector<Eigen::Vector2d>& points, double resolution) const {
  Grid grid;
  grid.resolution = resolution;
  if (points.empty()) return grid;

  double min_x = points.front().x();
  double max_x = points.front().x();
  double min_y = points.front().y();
  double max_y = points.front().y();
  for (const auto& p : points) {
    min_x = std::min(min_x, p.x());
    max_x = std::max(max_x, p.x());
    min_y = std::min(min_y, p.y());
    max_y = std::max(max_y, p.y());
  }
  const double pad = std::max(params_.search_xy_range, 0.5);
  min_x -= pad;
  min_y -= pad;
  max_x += pad;
  max_y += pad;

  grid.origin_x = min_x;
  grid.origin_y = min_y;
  grid.width = std::max(1, static_cast<int>(std::ceil((max_x - min_x) / resolution)));
  grid.height = std::max(1, static_cast<int>(std::ceil((max_y - min_y) / resolution)));
  grid.dist_m.assign(static_cast<size_t>(grid.width * grid.height), kDistInf);

  const int radius = std::max(1, static_cast<int>(std::ceil(0.15 / resolution)));
  for (const auto& p : points) {
    const int cx = static_cast<int>(std::floor((p.x() - grid.origin_x) / resolution));
    const int cy = static_cast<int>(std::floor((p.y() - grid.origin_y) / resolution));
    for (int dy = -radius; dy <= radius; ++dy) {
      for (int dx = -radius; dx <= radius; ++dx) {
        const int x = cx + dx;
        const int y = cy + dy;
        if (x < 0 || y < 0 || x >= grid.width || y >= grid.height) continue;
        grid.dist_m[static_cast<size_t>(y * grid.width + x)] = 0.f;
      }
    }
  }
  return grid;
}

void KartoMatcherEstimator::computeDistanceTransform(Grid* grid) const {
  if (grid == nullptr || grid->dist_m.empty()) return;
  const int w = grid->width;
  const int h = grid->height;
  auto& d = grid->dist_m;

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      const size_t i = static_cast<size_t>(y * w + x);
      float v = d[i];
      if (x > 0) v = std::min(v, d[i - 1] + 1.f);
      if (y > 0) v = std::min(v, d[i - static_cast<size_t>(w)] + 1.f);
      if (x > 0 && y > 0) v = std::min(v, d[i - static_cast<size_t>(w) - 1] + 1.41421356f);
      if (x + 1 < w && y > 0) {
        v = std::min(v, d[i - static_cast<size_t>(w) + 1] + 1.41421356f);
      }
      d[i] = v;
    }
  }
  for (int y = h - 1; y >= 0; --y) {
    for (int x = w - 1; x >= 0; --x) {
      const size_t i = static_cast<size_t>(y * w + x);
      float v = d[i];
      if (x + 1 < w) v = std::min(v, d[i + 1] + 1.f);
      if (y + 1 < h) v = std::min(v, d[i + static_cast<size_t>(w)] + 1.f);
      if (x + 1 < w && y + 1 < h) {
        v = std::min(v, d[i + static_cast<size_t>(w) + 1] + 1.41421356f);
      }
      if (x > 0 && y + 1 < h) {
        v = std::min(v, d[i + static_cast<size_t>(w) - 1] + 1.41421356f);
      }
      d[i] = v;
    }
  }

  const float scale = static_cast<float>(grid->resolution);
  for (float& v : d) {
    if (v >= kDistInf * 0.5f) {
      v = 5.f;
    } else {
      v *= scale;
    }
  }
}

std::vector<KartoMatcherEstimator::Grid> KartoMatcherEstimator::buildPyramid(
    const std::vector<Eigen::Vector2d>& points) const {
  const int levels = std::max(1, params_.pyramid_levels);
  std::vector<Grid> pyramid;
  pyramid.reserve(static_cast<size_t>(levels));
  for (int level = 0; level < levels; ++level) {
    const int exponent = levels - 1 - level;
    const double resolution = params_.grid_resolution * static_cast<double>(1 << exponent);
    Grid grid = buildGrid(points, resolution);
    computeDistanceTransform(&grid);
    pyramid.push_back(std::move(grid));
  }
  return pyramid;
}

double KartoMatcherEstimator::lookupDistanceM(const Grid& grid,
                                              const Eigen::Vector2d& p) const {
  const double fx = (p.x() - grid.origin_x) / grid.resolution - 0.5;
  const double fy = (p.y() - grid.origin_y) / grid.resolution - 0.5;
  const int x0 = static_cast<int>(std::floor(fx));
  const int y0 = static_cast<int>(std::floor(fy));
  const double ax = fx - static_cast<double>(x0);
  const double ay = fy - static_cast<double>(y0);

  auto sample = [&](int x, int y) -> double {
    if (x < 0 || y < 0 || x >= grid.width || y >= grid.height) return 5.0;
    return static_cast<double>(grid.dist_m[static_cast<size_t>(y * grid.width + x)]);
  };

  const double d00 = sample(x0, y0);
  const double d10 = sample(x0 + 1, y0);
  const double d01 = sample(x0, y0 + 1);
  const double d11 = sample(x0 + 1, y0 + 1);
  const double d0 = d00 * (1.0 - ax) + d10 * ax;
  const double d1 = d01 * (1.0 - ax) + d11 * ax;
  return d0 * (1.0 - ay) + d1 * ay;
}

double KartoMatcherEstimator::scorePose(const Grid& grid,
                                        const std::vector<Eigen::Vector2d>& points,
                                        const Eigen::Matrix3d& world_transform) const {
  if (points.empty()) return 0.0;
  const double sigma = params_.score_sigma;
  const double inv_sigma2 = 1.0 / (sigma * sigma);
  double score = 0.0;
  for (const auto& p : points) {
    const Eigen::Vector2d q = transformPoint(world_transform, p);
    const double d = lookupDistanceM(grid, q);
    score += std::exp(-d * d * inv_sigma2);
  }
  return score / static_cast<double>(points.size());
}

Eigen::Matrix3d KartoMatcherEstimator::searchBestTransform(
    const std::vector<Grid>& pyramid, const std::vector<Eigen::Vector2d>& points,
    const Eigen::Matrix3d& prior) const {
  if (pyramid.empty()) return prior;

  Eigen::Matrix3d best = prior;

  const size_t n_levels = pyramid.size();
  for (size_t level = 0; level < n_levels; ++level) {
    const Grid& grid = pyramid[level];
    const bool finest = (level + 1 == n_levels);
    const double scale = finest ? 1.0 : static_cast<double>(1 << (n_levels - 1 - level));
    const double xy_range =
        finest ? params_.search_xy_range : params_.search_xy_range / scale;
    const double yaw_range =
        finest ? params_.search_yaw_range : params_.search_yaw_range / scale;
    const int xy_steps = finest ? params_.fine_xy_steps : params_.coarse_xy_steps;
    const int yaw_steps = finest ? params_.fine_yaw_steps : params_.coarse_yaw_steps;

    const double xy_step =
        xy_steps > 1 ? (2.0 * xy_range) / static_cast<double>(xy_steps - 1) : 0.0;
    const double yaw_step =
        yaw_steps > 1 ? (2.0 * yaw_range) / static_cast<double>(yaw_steps - 1) : 0.0;

    double local_best_score = scorePose(grid, points, pose_ * best);
    Eigen::Matrix3d local_best = best;

    for (int iy = 0; iy < xy_steps; ++iy) {
      const double dty = -xy_range + static_cast<double>(iy) * xy_step;
      for (int ix = 0; ix < xy_steps; ++ix) {
        const double dtx = -xy_range + static_cast<double>(ix) * xy_step;
        for (int it = 0; it < yaw_steps; ++it) {
          const double dyaw = -yaw_range + static_cast<double>(it) * yaw_step;
          const Eigen::Matrix3d T = best * matrixFromIncrement(dtx, dty, dyaw);
          const double s = scorePose(grid, points, pose_ * T);
          if (s > local_best_score) {
            local_best_score = s;
            local_best = T;
          }
        }
      }
    }

    best = local_best;
  }

  return best;
}

void KartoMatcherEstimator::addScanToMap(const std::vector<Eigen::Vector2d>& points) {
  map_points_world_.reserve(map_points_world_.size() + points.size());
  for (const auto& p : points) {
    map_points_world_.push_back(transformPoint(pose_, p));
  }
}

void KartoMatcherEstimator::pruneMap() {
  const Eigen::Vector2d center(pose_(0, 2), pose_(1, 2));
  const double r2 = params_.local_map_radius * params_.local_map_radius;
  map_points_world_.erase(
      std::remove_if(map_points_world_.begin(), map_points_world_.end(),
                     [&](const Eigen::Vector2d& p) { return (p - center).squaredNorm() > r2; }),
      map_points_world_.end());
}

KartoMatcherResult KartoMatcherEstimator::registerScan(const LaserScan& scan) {
  KartoMatcherResult result;
  result.pose = pose_;

  const auto current = scanToPoints(scan);
  if (current.size() < 10) return result;

  if (!initialized_) {
    addScanToMap(current);
    initialized_ = true;
    result.valid = true;
    result.pose = pose_;
    return result;
  }

  const auto local_map = localMapPoints();
  if (local_map.size() < 20) {
    addScanToMap(current);
    pruneMap();
    result.valid = true;
    result.pose = pose_;
    return result;
  }

  const auto pyramid = buildPyramid(local_map);
  const Eigen::Matrix3d motion_prior =
      params_.use_motion_prior ? last_increment_ : Eigen::Matrix3d::Identity();

  const Eigen::Matrix3d increment = searchBestTransform(pyramid, current, motion_prior);
  const double best_score =
      pyramid.empty() ? 0.0
                      : scorePose(pyramid.back(), current, pose_ * increment);

  pose_ = pose_ * increment;
  last_increment_ = increment;
  addScanToMap(current);
  pruneMap();

  result.increment = increment;
  result.pose = pose_;
  result.valid = true;
  result.score = best_score;
  return result;
}

}  // namespace karto_matcher
}  // namespace localization_zoo
