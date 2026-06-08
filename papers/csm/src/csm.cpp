#include "csm/csm.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace csm {

namespace {

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

CSMEstimator::CSMEstimator(const CSMParams& params) : params_(params) {}

void CSMEstimator::reset() {
  initialized_ = false;
  ref_points_.clear();
  pose_ = Eigen::Matrix3d::Identity();
  last_increment_ = Eigen::Matrix3d::Identity();
}

void CSMEstimator::setInitialPose(const Eigen::Matrix3d& pose) { pose_ = pose; }

std::vector<Eigen::Vector2d> CSMEstimator::scanToPoints(const LaserScan& scan) const {
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

CSMEstimator::Grid CSMEstimator::buildGrid(const std::vector<Eigen::Vector2d>& points,
                                           double resolution) const {
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
  grid.cells.assign(static_cast<size_t>(grid.width * grid.height), 0.f);

  const float hit = 1.f;
  const int radius = std::max(1, static_cast<int>(std::ceil(0.15 / resolution)));
  for (const auto& p : points) {
    const int cx = static_cast<int>(std::floor((p.x() - grid.origin_x) / resolution));
    const int cy = static_cast<int>(std::floor((p.y() - grid.origin_y) / resolution));
    for (int dy = -radius; dy <= radius; ++dy) {
      for (int dx = -radius; dx <= radius; ++dx) {
        const int x = cx + dx;
        const int y = cy + dy;
        if (x < 0 || y < 0 || x >= grid.width || y >= grid.height) continue;
        grid.cells[static_cast<size_t>(y * grid.width + x)] = hit;
      }
    }
  }
  return grid;
}

double CSMEstimator::scorePoseBruteForce(const std::vector<Eigen::Vector2d>& ref,
                                         const std::vector<Eigen::Vector2d>& points,
                                         const Eigen::Matrix3d& transform) const {
  if (ref.empty() || points.empty()) return 0.0;
  double score = 0.0;
  const double sigma = 0.15;
  const double inv_sigma2 = 1.0 / (sigma * sigma);
  for (const auto& p : points) {
    const Eigen::Vector2d q = transformPoint(transform, p);
    double best = std::numeric_limits<double>::infinity();
    for (const auto& r : ref) {
      best = std::min(best, (r - q).squaredNorm());
    }
    score += std::exp(-best * inv_sigma2);
  }
  return score / static_cast<double>(points.size());
}

double CSMEstimator::scorePose(const Grid& grid, const std::vector<Eigen::Vector2d>& points,
                               const Eigen::Matrix3d& transform) const {
  return scorePoseBruteForce(ref_points_, points, transform);
}

Eigen::Matrix3d CSMEstimator::searchBestTransform(
    const Grid& grid, const std::vector<Eigen::Vector2d>& points,
    const Eigen::Matrix3d& prior) const {
  const double prior_x = prior(0, 2);
  const double prior_y = prior(1, 2);
  const double prior_yaw = std::atan2(prior(1, 0), prior(0, 0));

  double best_score = -1.0;
  Eigen::Matrix3d best = Eigen::Matrix3d::Identity();

  const struct SearchPass {
    double xy_range;
    double yaw_range;
    int xy_steps;
    int yaw_steps;
  } passes[] = {
      {params_.search_xy_range, params_.search_yaw_range, params_.coarse_xy_steps,
       params_.coarse_yaw_steps},
      {params_.search_xy_range * 0.25, params_.search_yaw_range * 0.25, params_.fine_xy_steps,
       params_.fine_yaw_steps},
  };

  double center_x = prior_x;
  double center_y = prior_y;
  double center_yaw = prior_yaw;

  for (const auto& pass : passes) {
    const double xy_step =
        pass.xy_steps > 1 ? (2.0 * pass.xy_range) / static_cast<double>(pass.xy_steps - 1) : 0.0;
    const double yaw_step =
        pass.yaw_steps > 1 ? (2.0 * pass.yaw_range) / static_cast<double>(pass.yaw_steps - 1)
                           : 0.0;

    double local_best_score = scorePoseBruteForce(ref_points_, points, prior);
    Eigen::Matrix3d local_best = prior;

    for (int iy = 0; iy < pass.xy_steps; ++iy) {
      const double ty = center_y - pass.xy_range + static_cast<double>(iy) * xy_step;
      for (int ix = 0; ix < pass.xy_steps; ++ix) {
        const double tx = center_x - pass.xy_range + static_cast<double>(ix) * xy_step;
        for (int it = 0; it < pass.yaw_steps; ++it) {
          const double yaw =
              center_yaw - pass.yaw_range + static_cast<double>(it) * yaw_step;
          const Eigen::Matrix3d T = matrixFromIncrement(tx, ty, yaw);
          const double s = scorePoseBruteForce(ref_points_, points, T);
          if (s > local_best_score) {
            local_best_score = s;
            local_best = T;
          }
        }
      }
    }

    if (local_best_score >= 0.0) {
      best = local_best;
      best_score = local_best_score;
      center_x = best(0, 2);
      center_y = best(1, 2);
      center_yaw = std::atan2(best(1, 0), best(0, 0));
    }
  }

  (void)best_score;
  return best;
}

CSMResult CSMEstimator::registerScan(const LaserScan& scan) {
  CSMResult result;
  result.pose = pose_;

  const auto current = scanToPoints(scan);
  if (current.size() < 10) return result;

  if (!initialized_) {
    ref_points_ = current;
    initialized_ = true;
    result.valid = true;
    result.pose = pose_;
    return result;
  }

  const Eigen::Matrix3d motion_prior =
      params_.use_motion_prior ? last_increment_ : Eigen::Matrix3d::Identity();

  Eigen::Matrix3d transform = motion_prior;
  transform = searchBestTransform(Grid{}, current, transform);
  const double best_score = scorePoseBruteForce(ref_points_, current, transform);

  pose_ = pose_ * transform;
  last_increment_ = transform;
  ref_points_ = current;

  result.increment = transform;
  result.pose = pose_;
  result.valid = true;
  result.score = best_score;
  return result;
}

}  // namespace csm
}  // namespace localization_zoo
