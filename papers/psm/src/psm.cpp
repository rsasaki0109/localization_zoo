#include "psm/psm.h"

#include <cmath>
#include <limits>

namespace localization_zoo {
namespace psm {

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

double angleIncrement(const LaserScan& scan) {
  if (scan.angle_increment > 0.0) return scan.angle_increment;
  if (scan.size() <= 1) return 0.0;
  return (scan.angle_max - scan.angle_min) / static_cast<double>(scan.size() - 1);
}

}  // namespace

Eigen::Matrix3d pose2D(double x, double y, double yaw) {
  return matrixFromIncrement(x, y, yaw);
}

PSMEstimator::PSMEstimator(const PSMParams& params) : params_(params) {}

void PSMEstimator::reset() {
  initialized_ = false;
  ref_scan_ = LaserScan{};
  ref_polar_.clear();
  pose_ = Eigen::Matrix3d::Identity();
  last_increment_ = Eigen::Matrix3d::Identity();
}

void PSMEstimator::setInitialPose(const Eigen::Matrix3d& pose) { pose_ = pose; }

Eigen::Vector2d PSMEstimator::transformPoint(const Eigen::Matrix3d& T,
                                             const Eigen::Vector2d& p) {
  return (T.block<2, 2>(0, 0) * p + T.block<2, 1>(0, 2)).eval();
}

std::vector<Eigen::Vector2d> PSMEstimator::scanToPoints(const LaserScan& scan) const {
  std::vector<Eigen::Vector2d> points;
  points.reserve(scan.size());
  const double inc = angleIncrement(scan);
  for (size_t i = 0; i < scan.size(); ++i) {
    const double r = scan.ranges[i];
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    const double a = scan.angle_min + static_cast<double>(i) * inc;
    points.emplace_back(r * std::cos(a), r * std::sin(a));
  }
  return points;
}

std::vector<double> PSMEstimator::polarProfileFromPoints(
    const std::vector<Eigen::Vector2d>& points, const LaserScan& grid) const {
  const size_t n = grid.size();
  std::vector<double> profile(n, std::numeric_limits<double>::quiet_NaN());
  if (n == 0) return profile;

  const double inc = angleIncrement(grid);
  const double half_bin = inc > 0.0 ? 0.5 * inc : M_PI;

  for (const auto& p : points) {
    const double r = p.norm();
    if (r < params_.min_range || r > params_.max_range) continue;
    const double a = std::atan2(p.y(), p.x());
    if (a < grid.angle_min - half_bin || a > grid.angle_max + half_bin) continue;

    const double idx_f = inc > 0.0 ? (a - grid.angle_min) / inc : 0.0;
    const int idx = static_cast<int>(std::lround(idx_f));
    if (idx < 0 || static_cast<size_t>(idx) >= n) continue;

    const size_t ui = static_cast<size_t>(idx);
    if (!std::isfinite(profile[ui]) || r < profile[ui]) {
      profile[ui] = r;
    }
  }
  return profile;
}

double PSMEstimator::scorePolarProfiles(const std::vector<double>& ref,
                                        const std::vector<double>& cur) const {
  if (ref.size() != cur.size() || ref.empty()) return 0.0;

  double mse = 0.0;
  int used = 0;
  for (size_t i = 0; i < ref.size(); ++i) {
    if (!std::isfinite(ref[i]) || !std::isfinite(cur[i])) continue;
    const double dr = ref[i] - cur[i];
    mse += dr * dr;
    ++used;
  }
  if (used < params_.min_valid_bins) return 0.0;
  mse /= static_cast<double>(used);

  const double inv_sigma2 = 1.0 / (params_.range_sigma * params_.range_sigma);
  return std::exp(-mse * inv_sigma2);
}

double PSMEstimator::scoreTransform(const std::vector<Eigen::Vector2d>& points,
                                    const Eigen::Matrix3d& transform) const {
  std::vector<Eigen::Vector2d> warped;
  warped.reserve(points.size());
  for (const auto& p : points) {
    warped.push_back(transformPoint(transform, p));
  }
  const auto cur_polar = polarProfileFromPoints(warped, ref_scan_);
  return scorePolarProfiles(ref_polar_, cur_polar);
}

Eigen::Matrix3d PSMEstimator::searchBestTransform(
    const std::vector<Eigen::Vector2d>& points, const Eigen::Matrix3d& prior) const {
  double center_x = prior(0, 2);
  double center_y = prior(1, 2);
  double center_yaw = std::atan2(prior(1, 0), prior(0, 0));

  Eigen::Matrix3d best = prior;
  double best_score = scoreTransform(points, prior);

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
      {params_.search_xy_range * 0.08, params_.search_yaw_range * 0.08,
       std::max(5, params_.fine_xy_steps), std::max(5, params_.fine_yaw_steps)},
  };

  for (const auto& pass : passes) {
    const double xy_step =
        pass.xy_steps > 1 ? (2.0 * pass.xy_range) / static_cast<double>(pass.xy_steps - 1) : 0.0;
    const double yaw_step =
        pass.yaw_steps > 1 ? (2.0 * pass.yaw_range) / static_cast<double>(pass.yaw_steps - 1)
                           : 0.0;

    Eigen::Matrix3d local_best = best;
    double local_best_score = best_score;

    for (int iy = 0; iy < pass.xy_steps; ++iy) {
      const double ty = center_y - pass.xy_range + static_cast<double>(iy) * xy_step;
      for (int ix = 0; ix < pass.xy_steps; ++ix) {
        const double tx = center_x - pass.xy_range + static_cast<double>(ix) * xy_step;
        for (int it = 0; it < pass.yaw_steps; ++it) {
          const double yaw =
              center_yaw - pass.yaw_range + static_cast<double>(it) * yaw_step;
          const Eigen::Matrix3d T = matrixFromIncrement(tx, ty, yaw);
          const double s = scoreTransform(points, T);
          if (s > local_best_score) {
            local_best_score = s;
            local_best = T;
          }
        }
      }
    }

    best = local_best;
    best_score = local_best_score;
    center_x = best(0, 2);
    center_y = best(1, 2);
    center_yaw = std::atan2(best(1, 0), best(0, 0));
  }

  return best;
}

Eigen::Matrix3d PSMEstimator::refineTransform(const std::vector<Eigen::Vector2d>& points,
                                              const Eigen::Matrix3d& initial) const {
  Eigen::Matrix3d transform = initial;
  double step_tx = 0.02;
  double step_ty = 0.02;
  double step_yaw = 0.01;

  for (int iter = 0; iter < params_.refine_iterations; ++iter) {
    const double base_score = scoreTransform(points, transform);
    const double tx0 = transform(0, 2);
    const double ty0 = transform(1, 2);
    const double yaw0 = std::atan2(transform(1, 0), transform(0, 0));

    double best_score = base_score;
    Eigen::Matrix3d best = transform;

    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dt = -1; dt <= 1; ++dt) {
          if (dx == 0 && dy == 0 && dt == 0) continue;
          const Eigen::Matrix3d cand =
              matrixFromIncrement(tx0 + dx * step_tx, ty0 + dy * step_ty, yaw0 + dt * step_yaw);
          const double s = scoreTransform(points, cand);
          if (s > best_score) {
            best_score = s;
            best = cand;
          }
        }
      }
    }

    if (best_score <= base_score + 1e-9) break;
    transform = best;
    step_tx *= 0.5;
    step_ty *= 0.5;
    step_yaw *= 0.5;
  }

  return transform;
}

PSMResult PSMEstimator::registerScan(const LaserScan& scan) {
  PSMResult result;
  result.pose = pose_;

  const auto current = scanToPoints(scan);
  if (current.size() < static_cast<size_t>(params_.min_valid_bins)) {
    return result;
  }

  if (!initialized_) {
    ref_scan_ = scan;
    ref_polar_ = ref_scan_.ranges;
    initialized_ = true;
    result.valid = true;
    result.pose = pose_;
    return result;
  }

  const Eigen::Matrix3d motion_prior =
      params_.use_motion_prior ? last_increment_ : Eigen::Matrix3d::Identity();

  Eigen::Matrix3d transform = searchBestTransform(current, motion_prior);
  transform = refineTransform(current, transform);
  const double best_score = scoreTransform(current, transform);

  pose_ = pose_ * transform;
  last_increment_ = transform;
  ref_scan_ = scan;
  ref_polar_ = ref_scan_.ranges;

  result.increment = transform;
  result.pose = pose_;
  result.valid = true;
  result.score = best_score;
  return result;
}

}  // namespace psm
}  // namespace localization_zoo
