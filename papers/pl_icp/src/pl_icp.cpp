#include "pl_icp/pl_icp.h"

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace pl_icp {

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

PLICPEstimator::PLICPEstimator(const PLICPParams& params) : params_(params) {}

void PLICPEstimator::reset() {
  initialized_ = false;
  ref_model_.clear();
  local_map_.clear();
  pose_ = Eigen::Matrix3d::Identity();
  last_increment_ = Eigen::Matrix3d::Identity();
}

void PLICPEstimator::setInitialPose(const Eigen::Matrix3d& pose) { pose_ = pose; }

Eigen::Vector2d PLICPEstimator::transformPoint(const Eigen::Matrix3d& T,
                                               const Eigen::Vector2d& p) {
  return (T.block<2, 2>(0, 0) * p + T.block<2, 1>(0, 2)).eval();
}

std::vector<Eigen::Vector2d> PLICPEstimator::scanToPoints(const LaserScan& scan) const {
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

std::vector<PLICPEstimator::RefPoint> PLICPEstimator::buildReferenceModel(
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

void PLICPEstimator::addScanToLocalMap(const LaserScan& scan) {
  const auto model = buildReferenceModel(scan);
  const Eigen::Matrix2d R = pose_.block<2, 2>(0, 0);
  StoredScan stored;
  stored.refs.reserve(model.size());
  for (const auto& ref : model) {
    if (!ref.valid_line) continue;
    RefPoint wp;
    wp.point = transformPoint(pose_, ref.point);
    wp.normal = (R * ref.normal).normalized();
    wp.valid_line = true;
    wp.valid_point = ref.valid_point;
    stored.refs.push_back(wp);
  }
  if (!stored.refs.empty()) {
    local_map_.push_back(std::move(stored));
  }
  while (static_cast<int>(local_map_.size()) > params_.local_map_max_scans) {
    local_map_.erase(local_map_.begin());
  }
  pruneLocalMap();
}

void PLICPEstimator::pruneLocalMap() {
  if (params_.local_map_radius <= 0.0) return;
  const Eigen::Vector2d t = pose_.block<2, 1>(0, 2);
  const double r2 = params_.local_map_radius * params_.local_map_radius;
  for (auto& stored : local_map_) {
    stored.refs.erase(std::remove_if(stored.refs.begin(), stored.refs.end(),
                                     [&](const RefPoint& ref) {
                                       return !ref.valid_line ||
                                              (ref.point - t).squaredNorm() > r2;
                                     }),
                      stored.refs.end());
  }
  local_map_.erase(std::remove_if(local_map_.begin(), local_map_.end(),
                                  [](const StoredScan& stored) { return stored.refs.empty(); }),
                   local_map_.end());
}

std::vector<PLICPEstimator::RefPoint> PLICPEstimator::localMapInFrame(
    const Eigen::Matrix3d& frame) const {
  const Eigen::Matrix3d inv = frame.inverse();
  const Eigen::Matrix2d R = inv.block<2, 2>(0, 0);
  std::vector<RefPoint> local;
  size_t total = 0;
  for (const auto& stored : local_map_) total += stored.refs.size();
  local.reserve(total);
  for (const auto& stored : local_map_) {
    for (const auto& ref : stored.refs) {
      RefPoint mapped;
      mapped.point = transformPoint(inv, ref.point);
      mapped.normal = (R * ref.normal).normalized();
      mapped.valid_line = true;
      mapped.valid_point = ref.valid_point;
      local.push_back(mapped);
    }
  }
  return local;
}

bool PLICPEstimator::solveIncrement(const std::vector<Eigen::Vector2d>& current,
                                    const std::vector<RefPoint>& references,
                                    const Eigen::Matrix3d& transform,
                                    Eigen::Matrix3d* increment) const {
  Eigen::Matrix3d A = Eigen::Matrix3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  int used = 0;

  for (const auto& p_cur : current) {
    const Eigen::Vector2d p_tr = transformPoint(transform, p_cur);
    double best_dist = params_.max_correspondence_distance;
    const RefPoint* best = nullptr;
    for (const auto& ref : references) {
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
  const Eigen::Vector3d x = A.ldlt().solve(b);
  if (!x.allFinite()) return false;
  *increment = matrixFromIncrement(x(0), x(1), x(2));
  return true;
}

PLICPResult PLICPEstimator::registerScan(const LaserScan& scan) {
  PLICPResult result;
  result.pose = pose_;

  const auto current = scanToPoints(scan);
  if (current.size() < static_cast<size_t>(params_.min_correspondences)) {
    return result;
  }

  if (!initialized_) {
    if (params_.use_local_map) {
      addScanToLocalMap(scan);
    } else {
      ref_model_ = buildReferenceModel(scan);
    }
    initialized_ = true;
    result.valid = true;
    result.pose = pose_;
    return result;
  }

  Eigen::Matrix3d transform =
      params_.use_motion_prior ? last_increment_ : Eigen::Matrix3d::Identity();
  const std::vector<RefPoint> references =
      params_.use_local_map ? localMapInFrame(pose_) : ref_model_;
  int iterations = 0;
  for (; iterations < params_.max_iterations; ++iterations) {
    Eigen::Matrix3d delta = Eigen::Matrix3d::Identity();
    if (!solveIncrement(current, references, transform, &delta)) break;
    transform = delta * transform;

    const double tx = delta(0, 2);
    const double ty = delta(1, 2);
    const double dyaw = std::atan2(delta(1, 0), delta(0, 0));
    if (std::hypot(tx, ty) < params_.convergence_translation &&
        std::abs(dyaw) < params_.convergence_rotation) {
      ++iterations;
      break;
    }
  }

  pose_ = pose_ * transform;
  last_increment_ = transform;
  if (params_.use_local_map) {
    addScanToLocalMap(scan);
  } else {
    ref_model_ = buildReferenceModel(scan);
  }
  result.increment = transform;
  result.pose = pose_;
  result.valid = true;
  result.iterations = iterations;
  return result;
}

}  // namespace pl_icp
}  // namespace localization_zoo
