#include "kinematic_icp/kinematic_icp.h"

#include <Eigen/Dense>
#include <cmath>

namespace localization_zoo {
namespace kinematic_icp {

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

}  // namespace

Eigen::Matrix3d pose2D(double x, double y, double yaw) {
  return matrixFromIncrement(x, y, yaw);
}

KinematicICPEstimator::KinematicICPEstimator(const KinematicICPParams& params)
    : params_(params) {}

void KinematicICPEstimator::reset() {
  initialized_ = false;
  ref_model_.clear();
  pose_ = Eigen::Matrix3d::Identity();
  last_increment_ = Eigen::Matrix3d::Identity();
}

void KinematicICPEstimator::setInitialPose(const Eigen::Matrix3d& pose) { pose_ = pose; }

WheelOdom KinematicICPEstimator::incrementToWheelOdom(const Eigen::Matrix3d& T) {
  WheelOdom odom;
  odom.yaw_rad = std::atan2(T(1, 0), T(0, 0));
  odom.forward_m = std::hypot(T(0, 2), T(1, 2));
  return odom;
}

Eigen::Matrix3d KinematicICPEstimator::wheelOdomToIncrement(double forward_m, double yaw_rad) {
  return matrixFromIncrement(forward_m * std::cos(yaw_rad), forward_m * std::sin(yaw_rad),
                            yaw_rad);
}

Eigen::Matrix3d KinematicICPEstimator::projectToUnicycle(const Eigen::Matrix3d& T) {
  const double yaw = std::atan2(T(1, 0), T(0, 0));
  const double forward = std::hypot(T(0, 2), T(1, 2));
  return wheelOdomToIncrement(forward, yaw);
}

Eigen::Vector2d KinematicICPEstimator::transformPoint(const Eigen::Matrix3d& T,
                                                    const Eigen::Vector2d& p) {
  return (T.block<2, 2>(0, 0) * p + T.block<2, 1>(0, 2)).eval();
}

std::vector<Eigen::Vector2d> KinematicICPEstimator::scanToPoints(const LaserScan& scan) const {
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

std::vector<KinematicICPEstimator::RefPoint> KinematicICPEstimator::buildReferenceModel(
    const LaserScan& scan) const {
  const double inc =
      scan.angle_increment > 0.0 ? scan.angle_increment
                                 : (scan.size() > 1 ? (scan.angle_max - scan.angle_min) /
                                                          static_cast<double>(scan.size() - 1)
                                                    : 0.0);
  std::vector<RefPoint> model(scan.size());
  for (size_t i = 0; i < scan.size(); ++i) {
    const double r = scan.ranges[i];
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    const double a = scan.angle_min + static_cast<double>(i) * inc;
    model[i].point = Eigen::Vector2d(r * std::cos(a), r * std::sin(a));
    model[i].valid_point = true;
  }

  for (size_t i = 0; i < scan.size(); ++i) {
    if (!model[i].valid_point) continue;
    int prev = static_cast<int>(i) - 1;
    int next = static_cast<int>(i) + 1;
    while (prev >= 0 && !model[static_cast<size_t>(prev)].valid_point) --prev;
    while (next < static_cast<int>(scan.size()) && !model[static_cast<size_t>(next)].valid_point) {
      ++next;
    }
    if (prev < 0 || next >= static_cast<int>(scan.size())) continue;
    const Eigen::Vector2d tangent =
        model[static_cast<size_t>(next)].point - model[static_cast<size_t>(prev)].point;
    const double len = tangent.norm();
    if (len < 1e-3 || len > params_.max_neighbor_gap) continue;
    model[i].normal = Eigen::Vector2d(-tangent.y(), tangent.x()) / len;
    model[i].valid_line = true;
  }
  return model;
}

bool KinematicICPEstimator::solveKinematicIncrement(
    const std::vector<Eigen::Vector2d>& current, const Eigen::Matrix3d& transform,
    const WheelOdom& wheel_prior, Eigen::Matrix3d* increment) const {
  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  int used = 0;

  for (const auto& p_cur : current) {
    const Eigen::Vector2d p_tr = transformPoint(transform, p_cur);
    double best_dist = params_.max_correspondence_distance;
    const RefPoint* best = nullptr;
    for (const auto& ref : ref_model_) {
      if (!ref.valid_line) continue;
      const double d = (ref.point - p_tr).norm();
      if (d < best_dist) {
        best_dist = d;
        best = &ref;
      }
    }
    if (best == nullptr) continue;

    const Eigen::Vector2d n = best->normal;
    const double err = n.dot(p_tr - best->point);
    Eigen::Vector3d row;
    row(0) = n.x();
    row(1) = n.y();
    row(2) = -n.x() * p_tr.y() + n.y() * p_tr.x();
    A += row * row.transpose();
    b += -err * row;
    ++used;
  }

  if (used < params_.min_correspondences) return false;

  const double w = params_.wheel_odom_weight;
  const WheelOdom kin_guess = incrementToWheelOdom(transform);
  A(0, 0) += w;
  A(1, 1) += w;
  A(2, 2) += w;
  b(0) += w * (wheel_prior.forward_m * std::cos(wheel_prior.yaw_rad) -
               kin_guess.forward_m * std::cos(kin_guess.yaw_rad));
  b(1) += w * (wheel_prior.forward_m * std::sin(wheel_prior.yaw_rad) -
               kin_guess.forward_m * std::sin(kin_guess.yaw_rad));
  b(2) += w * (wheel_prior.yaw_rad - kin_guess.yaw_rad);

  const Eigen::Vector3d delta = A.ldlt().solve(b);
  if (!delta.allFinite()) return false;

  const Eigen::Matrix3d free_inc =
      matrixFromIncrement(delta(0), delta(1), delta(2)) * transform;
  *increment = projectToUnicycle(free_inc);
  return true;
}

KinematicICPResult KinematicICPEstimator::registerScan(
    const LaserScan& scan, const std::optional<WheelOdom>& wheel_odom) {
  KinematicICPResult result;
  result.pose = pose_;

  const auto current = scanToPoints(scan);
  if (current.size() < static_cast<size_t>(params_.min_correspondences)) {
    return result;
  }

  if (!initialized_) {
    ref_model_ = buildReferenceModel(scan);
    initialized_ = true;
    result.valid = true;
    result.pose = pose_;
    return result;
  }

  WheelOdom wheel_prior = wheel_odom.value_or(WheelOdom{});
  if (!wheel_odom.has_value() && params_.use_last_increment_as_wheel_odom) {
    wheel_prior = incrementToWheelOdom(last_increment_);
  }

  Eigen::Matrix3d transform = Eigen::Matrix3d::Identity();
  if (wheel_odom.has_value()) {
    transform = wheelOdomToIncrement(wheel_prior.forward_m, wheel_prior.yaw_rad);
  } else if (params_.use_last_increment_as_wheel_odom) {
    transform = last_increment_;
  }

  int iterations = 0;
  for (; iterations < params_.max_iterations; ++iterations) {
    Eigen::Matrix3d next = Eigen::Matrix3d::Identity();
    if (!solveKinematicIncrement(current, transform, wheel_prior, &next)) break;

    const WheelOdom before = incrementToWheelOdom(transform);
    const WheelOdom after = incrementToWheelOdom(next);
    transform = next;
    if (std::abs(after.forward_m - before.forward_m) < params_.convergence_translation &&
        std::abs(after.yaw_rad - before.yaw_rad) < params_.convergence_rotation) {
      ++iterations;
      break;
    }
  }

  transform = projectToUnicycle(transform);
  const WheelOdom solved = incrementToWheelOdom(transform);
  pose_ = pose_ * transform;
  last_increment_ = transform;
  ref_model_ = buildReferenceModel(scan);

  result.increment = transform;
  result.pose = pose_;
  result.valid = true;
  result.iterations = iterations;
  result.forward_m = solved.forward_m;
  result.yaw_rad = solved.yaw_rad;
  return result;
}

}  // namespace kinematic_icp
}  // namespace localization_zoo
