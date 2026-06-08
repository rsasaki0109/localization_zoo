#include "idc/idc.h"

#include <Eigen/Dense>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace idc {

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
  if (scan.size() > 1) {
    return (scan.angle_max - scan.angle_min) / static_cast<double>(scan.size() - 1);
  }
  return 0.0;
}

}  // namespace

Eigen::Matrix3d pose2D(double x, double y, double yaw) {
  return matrixFromIncrement(x, y, yaw);
}

IDCEstimator::IDCEstimator(const IDCParams& params) : params_(params) {}

void IDCEstimator::reset() {
  initialized_ = false;
  ref_model_.clear();
  pose_ = Eigen::Matrix3d::Identity();
  last_increment_ = Eigen::Matrix3d::Identity();
}

void IDCEstimator::setInitialPose(const Eigen::Matrix3d& pose) { pose_ = pose; }

Eigen::Vector2d IDCEstimator::transformPoint(const Eigen::Matrix3d& T,
                                             const Eigen::Vector2d& p) {
  return (T.block<2, 2>(0, 0) * p + T.block<2, 1>(0, 2)).eval();
}

std::vector<IDCEstimator::Sample> IDCEstimator::scanToSamples(const LaserScan& scan) const {
  std::vector<Sample> samples;
  samples.reserve(scan.size());
  const double inc = angleIncrement(scan);
  for (size_t i = 0; i < scan.size(); ++i) {
    const double r = scan.ranges[i];
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    const double a = scan.angle_min + static_cast<double>(i) * inc;
    samples.push_back({Eigen::Vector2d(r * std::cos(a), r * std::sin(a)), r,
                       static_cast<int>(i)});
  }
  return samples;
}

std::vector<IDCEstimator::RefPoint> IDCEstimator::buildReferenceModel(
    const LaserScan& scan) const {
  const double inc = angleIncrement(scan);
  std::vector<RefPoint> model(scan.size());
  for (size_t i = 0; i < scan.size(); ++i) {
    const double r = scan.ranges[i];
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    const double a = scan.angle_min + static_cast<double>(i) * inc;
    model[i].point = Eigen::Vector2d(r * std::cos(a), r * std::sin(a));
    model[i].range = r;
    model[i].valid = true;
  }
  return model;
}

bool IDCEstimator::solveCP(const std::vector<Sample>& current, const Eigen::Matrix3d& transform,
                           double* tx, double* ty) const {
  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  int used = 0;

  for (const auto& sample : current) {
    const Eigen::Vector2d p_tr = transformPoint(transform, sample.point);
    double best_dist = params_.max_cp_distance;
    const RefPoint* best = nullptr;
    for (const auto& ref : ref_model_) {
      if (!ref.valid) continue;
      const double d = (ref.point - p_tr).norm();
      if (d < best_dist) {
        best_dist = d;
        best = &ref;
      }
    }
    if (best == nullptr) continue;

    const Eigen::Vector2d err = p_tr - best->point;
    Eigen::Matrix<double, 2, 3> J;
    J << 1.0, 0.0, -p_tr.y(), 0.0, 1.0, p_tr.x();
    A += J.transpose() * J;
    b += J.transpose() * err;
    ++used;
  }

  if (used < params_.min_correspondences) return false;
  const Eigen::Vector3d x = A.ldlt().solve(-b);
  if (!x.allFinite()) return false;
  *tx = x(0);
  *ty = x(1);
  return true;
}

bool IDCEstimator::solveRR(const std::vector<Sample>& current, const Eigen::Matrix3d& transform,
                           double* yaw) const {
  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  int used = 0;

  for (const auto& sample : current) {
    const Eigen::Vector2d p_tr = transformPoint(transform, sample.point);
    const double range_tr = p_tr.norm();

    double best_diff = params_.max_range_diff;
    const RefPoint* best = nullptr;
    const int w = params_.range_match_beam_window;
    const int n = static_cast<int>(ref_model_.size());
    for (int j = std::max(0, sample.beam - w); j < std::min(n, sample.beam + w + 1); ++j) {
      const auto& ref = ref_model_[static_cast<size_t>(j)];
      if (!ref.valid) continue;
      const double diff = std::abs(range_tr - ref.range);
      if (diff < best_diff) {
        best_diff = diff;
        best = &ref;
      }
    }
    if (best == nullptr) continue;

    const Eigen::Vector2d err = p_tr - best->point;
    Eigen::Matrix<double, 2, 3> J;
    J << 1.0, 0.0, -p_tr.y(), 0.0, 1.0, p_tr.x();
    A += J.transpose() * J;
    b += J.transpose() * err;
    ++used;
  }

  if (used < params_.min_correspondences) return false;
  const Eigen::Vector3d x = A.ldlt().solve(-b);
  if (!x.allFinite()) return false;
  *yaw = x(2);
  return true;
}

IDCResult IDCEstimator::registerScan(const LaserScan& scan) {
  IDCResult result;
  result.pose = pose_;

  const auto current = scanToSamples(scan);
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

  Eigen::Matrix3d transform =
      params_.use_motion_prior ? last_increment_ : Eigen::Matrix3d::Identity();
  int iterations = 0;

  for (; iterations < params_.max_iterations; ++iterations) {
    double tx = 0.0;
    double ty = 0.0;
    double dyaw = 0.0;
    const bool cp_ok = solveCP(current, transform, &tx, &ty);
    const bool rr_ok = solveRR(current, transform, &dyaw);
    if (!cp_ok && !rr_ok) break;

    const double use_tx = cp_ok ? tx : 0.0;
    const double use_ty = cp_ok ? ty : 0.0;
    const double use_yaw = rr_ok ? dyaw : 0.0;
    const Eigen::Matrix3d delta = matrixFromIncrement(use_tx, use_ty, use_yaw);
    transform = delta * transform;

    if (std::hypot(use_tx, use_ty) < params_.convergence_translation &&
        std::abs(use_yaw) < params_.convergence_rotation) {
      ++iterations;
      break;
    }
  }

  pose_ = pose_ * transform;
  last_increment_ = transform;
  ref_model_ = buildReferenceModel(scan);
  result.increment = transform;
  result.pose = pose_;
  result.valid = true;
  result.iterations = iterations;
  return result;
}

}  // namespace idc
}  // namespace localization_zoo
