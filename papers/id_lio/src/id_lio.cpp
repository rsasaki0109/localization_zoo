#include "id_lio/id_lio.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

namespace localization_zoo {
namespace id_lio {

namespace {

constexpr double kPi = 3.14159265358979323846;

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0, -v.z(), v.y(), v.z(), 0, -v.x(), -v.y(), v.x(), 0;
  return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& w) {
  const double theta = w.norm();
  if (theta < 1e-10) return Eigen::Matrix3d::Identity() + skew(w);
  const Eigen::Matrix3d K = skew(w / theta);
  return Eigen::Matrix3d::Identity() + std::sin(theta) * K +
         (1.0 - std::cos(theta)) * K * K;
}

Eigen::Vector3d logSO3(const Eigen::Matrix3d& R) {
  const double c = std::clamp((R.trace() - 1.0) * 0.5, -1.0, 1.0);
  const double theta = std::acos(c);
  if (theta < 1e-9) return Eigen::Vector3d::Zero();
  const Eigen::Vector3d w(R(2, 1) - R(1, 2), R(0, 2) - R(2, 0),
                          R(1, 0) - R(0, 1));
  return w * (theta / (2.0 * std::sin(theta)));
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

}  // namespace

// ============================================================
// IndexedVoxelMap
// ============================================================

double IndexedVoxelMap::staticWeight(const IndexedPoint& p,
                                     double dynamic_weight) const {
  if (p.removed) return 0.0;
  double w = std::clamp(p.confidence, dynamic_weight, 1.0);
  if (p.missing_age > 0 || p.dynamic_age > 0) {
    const double age = static_cast<double>(std::max(p.missing_age, p.dynamic_age));
    w *= std::pow(dynamic_weight, std::min(1.0, age / 3.0));
  }
  return std::clamp(w, dynamic_weight, 1.0);
}

size_t IndexedVoxelMap::size() const {
  size_t n = 0;
  for (const auto& p : points_) {
    if (!p.removed) ++n;
  }
  return n;
}

size_t IndexedVoxelMap::dynamicCount() const {
  size_t n = 0;
  for (const auto& p : points_) {
    if (!p.removed && (p.missing_age > 0 || p.dynamic_age > 0)) ++n;
  }
  return n;
}

size_t IndexedVoxelMap::removedCount() const {
  size_t n = 0;
  for (const auto& p : points_) {
    if (p.removed) ++n;
  }
  return n;
}

void IndexedVoxelMap::addOrUpdatePoints(
    const std::vector<Eigen::Vector3d>& points,
    const std::vector<double>& weights, int frame_index, double update_radius) {
  const double update_radius_sq = update_radius * update_radius;
  for (size_t i = 0; i < points.size(); ++i) {
    const Eigen::Vector3d& p = points[i];
    const double w =
        weights.empty() ? 1.0 : std::clamp(weights[i], 0.0, 1.0);
    const auto key = toVoxel(p);

    size_t best_idx = std::numeric_limits<size_t>::max();
    double best_dist = update_radius_sq;
    for (int dx = -1; dx <= 1; ++dx)
      for (int dy = -1; dy <= 1; ++dy)
        for (int dz = -1; dz <= 1; ++dz) {
          auto it = voxels_.find(key + Eigen::Vector3i(dx, dy, dz));
          if (it == voxels_.end()) continue;
          for (size_t idx : it->second) {
            IndexedPoint& q = points_[idx];
            if (q.removed) continue;
            const double d = (q.point - p).squaredNorm();
            if (d < best_dist) {
              best_dist = d;
              best_idx = idx;
            }
          }
        }

    if (best_idx != std::numeric_limits<size_t>::max()) {
      IndexedPoint& q = points_[best_idx];
      const double alpha =
          std::clamp(1.0 / static_cast<double>(q.observations + 1), 0.05, 0.35);
      q.point = (1.0 - alpha) * q.point + alpha * p;
      q.confidence = std::clamp(0.8 * q.confidence + 0.2 * w, 0.0, 1.0);
      q.observations += 1;
      q.last_seen_frame = frame_index;
      if (w > 0.5) {
        q.missing_age = std::max(0, q.missing_age - 1);
        q.dynamic_age = std::max(0, q.dynamic_age - 1);
      } else {
        q.dynamic_age += 1;
      }
      continue;
    }

    auto& bucket = voxels_[key];
    if (static_cast<int>(bucket.size()) >= max_points_) continue;
    IndexedPoint q;
    q.id = next_id_++;
    q.point = p;
    q.observations = 1;
    q.last_seen_frame = frame_index;
    q.confidence = w;
    q.dynamic_age = (w > 0.5) ? 0 : 1;
    points_.push_back(q);
    bucket.push_back(points_.size() - 1);
  }
}

IndexedVoxelMap::Correspondence IndexedVoxelMap::query(
    const Eigen::Vector3d& world_point, double max_dist,
    int normal_min_neighbors, double dynamic_weight) const {
  Correspondence out;
  const auto key = toVoxel(world_point);
  const double max_dist_sq = max_dist * max_dist;
  std::vector<std::pair<Eigen::Vector3d, double>> neighbors;
  neighbors.reserve(48);
  Eigen::Vector3d best = Eigen::Vector3d::Zero();
  double best_dist = max_dist_sq;

  for (int dx = -1; dx <= 1; ++dx)
    for (int dy = -1; dy <= 1; ++dy)
      for (int dz = -1; dz <= 1; ++dz) {
        auto it = voxels_.find(key + Eigen::Vector3i(dx, dy, dz));
        if (it == voxels_.end()) continue;
        for (size_t idx : it->second) {
          const IndexedPoint& q = points_[idx];
          if (q.removed) continue;
          const double d = (q.point - world_point).squaredNorm();
          if (d >= max_dist_sq) continue;
          const double w = staticWeight(q, dynamic_weight);
          if (w <= 0.0) continue;
          neighbors.emplace_back(q.point, w);
          if (d < best_dist) {
            best_dist = d;
            best = q.point;
            out.found = true;
          }
        }
      }

  if (!out.found) return out;
  if (static_cast<int>(neighbors.size()) < normal_min_neighbors) {
    out.anchor = best;
    out.weight = dynamic_weight;
    return out;
  }

  double w_sum = 0.0;
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  for (const auto& n : neighbors) {
    mean += n.second * n.first;
    w_sum += n.second;
  }
  if (w_sum <= 1e-9) return out;
  mean /= w_sum;

  Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
  for (const auto& n : neighbors) {
    const Eigen::Vector3d d = n.first - mean;
    cov += n.second * d * d.transpose();
  }
  cov /= w_sum;

  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
  if (es.info() == Eigen::Success) {
    out.anchor = mean;
    out.normal = es.eigenvectors().col(0).normalized();
    out.weight = std::clamp(w_sum / static_cast<double>(neighbors.size()),
                            dynamic_weight, 1.0);
    out.has_normal = true;
  }
  return out;
}

void IndexedVoxelMap::pruneFarVoxels(const Eigen::Vector3d& center,
                                     double max_distance) {
  const double max_distance_sq = max_distance * max_distance;
  for (auto& p : points_) {
    if (p.removed) continue;
    if ((p.point - center).squaredNorm() > max_distance_sq) p.removed = true;
  }
}

void IndexedVoxelMap::pruneDynamic(int delayed_removal_frames) {
  for (auto& p : points_) {
    if (p.removed) continue;
    if (p.missing_age >= delayed_removal_frames ||
        p.dynamic_age >= delayed_removal_frames) {
      p.removed = true;
    }
  }
}

// ============================================================
// IDLIOPipeline
// ============================================================

IDLIOPipeline::IDLIOPipeline(const IDLIOParams& params)
    : params_(params),
      map_(params.voxel_size, params.max_points_per_voxel) {}

std::vector<Eigen::Vector3d> IDLIOPipeline::voxelDownsample(
    const std::vector<Eigen::Vector3d>& points, double voxel_size) const {
  std::unordered_map<Eigen::Vector3i, Eigen::Vector3d, VoxelHash> grid;
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

std::vector<Eigen::Vector3d> IDLIOPipeline::rangeFilter(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

bool IDLIOPipeline::projectToRangeImage(const Eigen::Vector3d& p_sensor,
                                        int* idx, double* range) const {
  const double r = p_sensor.norm();
  if (r < params_.min_range || r > params_.max_range) return false;
  const double yaw = std::atan2(p_sensor.y(), p_sensor.x());
  const double pitch = std::asin(std::clamp(p_sensor.z() / r, -1.0, 1.0));
  const double fov_up = params_.fov_up_deg * kPi / 180.0;
  const double fov_down = params_.fov_down_deg * kPi / 180.0;
  if (pitch > fov_up || pitch < fov_down) return false;
  int u = static_cast<int>((0.5 * (yaw / kPi + 1.0)) *
                           static_cast<double>(params_.range_image_width));
  int v = static_cast<int>(((fov_up - pitch) / (fov_up - fov_down)) *
                           static_cast<double>(params_.range_image_height));
  u = std::clamp(u, 0, params_.range_image_width - 1);
  v = std::clamp(v, 0, params_.range_image_height - 1);
  *idx = v * params_.range_image_width + u;
  if (range) *range = r;
  return true;
}

IDLIOPipeline::RangeImage IDLIOPipeline::buildCurrentRangeImage(
    const std::vector<Eigen::Vector3d>& source) const {
  const int n = params_.range_image_width * params_.range_image_height;
  RangeImage img;
  img.ranges.assign(n, std::numeric_limits<double>::infinity());
  img.valid.assign(n, false);
  for (const auto& p : source) {
    int idx = 0;
    double r = 0.0;
    if (!projectToRangeImage(p, &idx, &r)) continue;
    if (r < img.ranges[idx]) {
      img.ranges[idx] = r;
      img.valid[idx] = true;
    }
  }
  return img;
}

IDLIOPipeline::RangeImage IDLIOPipeline::buildMapRangeImage(
    const Eigen::Matrix4d& sensor_to_world) const {
  const int n = params_.range_image_width * params_.range_image_height;
  RangeImage img;
  img.ranges.assign(n, std::numeric_limits<double>::infinity());
  img.valid.assign(n, false);
  const Eigen::Matrix3d R_ws = sensor_to_world.block<3, 3>(0, 0);
  const Eigen::Vector3d t_ws = sensor_to_world.block<3, 1>(0, 3);
  const Eigen::Matrix3d R_sw = R_ws.transpose();
  for (const auto& mp : map_.points()) {
    if (mp.removed) continue;
    const Eigen::Vector3d p_sensor = R_sw * (mp.point - t_ws);
    int idx = 0;
    double r = 0.0;
    if (!projectToRangeImage(p_sensor, &idx, &r)) continue;
    if (r < img.ranges[idx]) {
      img.ranges[idx] = r;
      img.valid[idx] = true;
    }
  }
  return img;
}

std::vector<double> IDLIOPipeline::classifySourceWeights(
    const std::vector<Eigen::Vector3d>& source, const RangeImage& map_ranges,
    int* dynamic_points) const {
  std::vector<double> weights(source.size(), 1.0);
  int dynamic = 0;
  for (size_t i = 0; i < source.size(); ++i) {
    int idx = 0;
    double r = 0.0;
    if (!projectToRangeImage(source[i], &idx, &r)) continue;
    if (map_ranges.valid[idx] &&
        r + params_.new_object_margin < map_ranges.ranges[idx]) {
      weights[i] = params_.dynamic_weight;
      ++dynamic;
    }
  }
  if (dynamic_points) *dynamic_points = dynamic;
  return weights;
}

void IDLIOPipeline::markMissingMapPoints(
    const RangeImage& current_ranges, const Eigen::Matrix4d& sensor_to_world) {
  const Eigen::Matrix3d R_ws = sensor_to_world.block<3, 3>(0, 0);
  const Eigen::Vector3d t_ws = sensor_to_world.block<3, 1>(0, 3);
  const Eigen::Matrix3d R_sw = R_ws.transpose();
  for (auto& mp : map_.points()) {
    if (mp.removed) continue;
    const Eigen::Vector3d p_sensor = R_sw * (mp.point - t_ws);
    int idx = 0;
    double r = 0.0;
    if (!projectToRangeImage(p_sensor, &idx, &r)) continue;
    if (!current_ranges.valid[idx]) continue;
    if (r + params_.disappearance_margin < current_ranges.ranges[idx]) {
      mp.missing_age += 1;
      mp.dynamic_age += 1;
      mp.confidence = std::max(0.0, mp.confidence * 0.8);
    } else {
      mp.missing_age = std::max(0, mp.missing_age - 1);
      if (current_ranges.ranges[idx] + params_.new_object_margin >= r) {
        mp.dynamic_age = std::max(0, mp.dynamic_age - 1);
        mp.confidence = std::min(1.0, mp.confidence + 0.05);
      }
    }
  }
  map_.pruneDynamic(params_.delayed_removal_frames);
}

Eigen::Matrix4d IDLIOPipeline::runRegistration(
    const std::vector<Eigen::Vector3d>& source,
    const std::vector<double>& source_weights,
    const Eigen::Matrix4d& initial_guess, IDLIOResult* result) const {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;
  Eigen::Matrix4d T = initial_guess;
  const double kernel = params_.max_correspondence_dist;

  int last_count = 0;
  for (int iter = 0; iter < params_.max_icp_iterations; ++iter) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);
    Mat6 A = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    int count = 0;

    for (size_t i = 0; i < source.size(); ++i) {
      const Eigen::Vector3d p_world = R * source[i] + t;
      const auto c = map_.query(p_world, params_.max_correspondence_dist,
                                params_.normal_min_neighbors,
                                params_.dynamic_weight);
      if (!c.found || !c.has_normal) continue;
      const double e = c.normal.dot(p_world - c.anchor);
      if (std::abs(e) > params_.max_correspondence_dist) continue;

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(p_world);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const Eigen::Matrix<double, 1, 6> Jr = c.normal.transpose() * J;
      const double robust = std::exp(-0.5 * (e / kernel) * (e / kernel));
      const double w = robust * c.weight * source_weights[i];
      if (w < 1e-4) continue;
      A += w * Jr.transpose() * Jr;
      b -= w * Jr.transpose() * e;
      ++count;
    }

    if (count < 10) break;
    const Vec6 delta = A.ldlt().solve(b);
    if (!delta.allFinite()) break;
    T = expSE3(delta) * T;
    last_count = count;
    if (result) result->iterations = iter + 1;
    if (delta.norm() < params_.convergence_criterion) break;
  }
  if (result) result->num_correspondences = last_count;
  return T;
}

IDLIOResult IDLIOPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame,
    const std::vector<localization_zoo::imu_preintegration::ImuSample>& imu) {
  IDLIOResult result;
  auto filtered = rangeFilter(frame);
  auto downsampled = voxelDownsample(filtered, params_.voxel_size * 0.5);
  auto reg = voxelDownsample(downsampled, params_.voxel_size);
  if (reg.empty()) reg = downsampled;

  if (frame_count_ == 0) {
    std::vector<double> weights(reg.size(), 1.0);
    map_.addOrUpdatePoints(transformPoints(reg, pose_), weights, frame_count_,
                           params_.map_update_radius);
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  Eigen::Matrix3d imu_delta_R = last_delta_.block<3, 3>(0, 0);
  double imu_dt = 0.0;
  if (imu.size() >= 2) {
    localization_zoo::imu_preintegration::ImuPreintegrator preint(
        localization_zoo::imu_preintegration::ImuNoiseParams(), gyro_bias_,
        Eigen::Vector3d::Zero());
    preint.integrateBatch(imu);
    const Eigen::Matrix3d dR = preint.result().delta_R;
    imu_dt = imu.back().timestamp - imu.front().timestamp;
    if (dR.allFinite()) {
      imu_delta_R = dR;
      result.used_imu = true;
    }
  }

  Eigen::Matrix4d motion = Eigen::Matrix4d::Identity();
  motion.block<3, 3>(0, 0) = imu_delta_R;
  motion.block<3, 1>(0, 3) = last_delta_.block<3, 1>(0, 3);
  const Eigen::Matrix4d prediction = pose_ * motion;

  const auto predicted_map_ranges = buildMapRangeImage(prediction);
  int dynamic_source = 0;
  const auto source_weights =
      classifySourceWeights(reg, predicted_map_ranges, &dynamic_source);
  result.num_dynamic_points = dynamic_source;

  const Eigen::Matrix4d new_pose =
      runRegistration(reg, source_weights, prediction, &result);
  last_delta_ = pose_.inverse() * new_pose;

  if (result.used_imu && params_.gyro_bias_gain > 0.0 && imu_dt > 1e-6) {
    const Eigen::Vector3d residual =
        logSO3(imu_delta_R.transpose() * last_delta_.block<3, 3>(0, 0));
    gyro_bias_ -= params_.gyro_bias_gain * residual / imu_dt;
    const double bn = gyro_bias_.norm();
    if (bn > params_.max_gyro_bias) gyro_bias_ *= params_.max_gyro_bias / bn;
  }

  pose_ = new_pose;
  const auto current_ranges = buildCurrentRangeImage(reg);
  markMissingMapPoints(current_ranges, pose_);
  map_.addOrUpdatePoints(transformPoints(reg, pose_), source_weights,
                         frame_count_, params_.map_update_radius);

  if (params_.local_map_radius > 0.0 && params_.map_cleanup_interval > 0 &&
      (frame_count_ % params_.map_cleanup_interval) == 0) {
    map_.pruneFarVoxels(pose_.block<3, 1>(0, 3), params_.local_map_radius);
  }

  result.pose = pose_;
  result.num_removed_points = static_cast<int>(map_.removedCount());
  result.converged =
      pose_.array().isFinite().all() && result.num_correspondences >= 10;
  frame_count_++;
  return result;
}

}  // namespace id_lio
}  // namespace localization_zoo
