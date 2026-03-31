#include "scan_context/scan_context.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <utility>

namespace localization_zoo {
namespace scan_context {

namespace {

constexpr double kTwoPi = 2.0 * M_PI;

double wrapAngle(double angle) {
  while (angle < 0.0) {
    angle += kTwoPi;
  }
  while (angle >= kTwoPi) {
    angle -= kTwoPi;
  }
  return angle;
}

}  // namespace

Eigen::MatrixXd ScanContextManager::makeScanContext(
    const std::vector<Eigen::Vector3d>& points) const {
  Eigen::MatrixXd descriptor =
      Eigen::MatrixXd::Zero(params_.num_rings, params_.num_sectors);

  if (params_.num_rings <= 0 || params_.num_sectors <= 0 ||
      params_.max_radius <= 0.0) {
    return descriptor;
  }

  for (const auto& point : points) {
    const double range = point.head<2>().norm();
    if (range <= 1e-6 || range > params_.max_radius) {
      continue;
    }

    const double angle = wrapAngle(std::atan2(point.y(), point.x()));
    const int ring_idx = std::min(
        params_.num_rings - 1,
        static_cast<int>(std::floor(range / params_.max_radius * params_.num_rings)));
    const int sector_idx = std::min(
        params_.num_sectors - 1,
        static_cast<int>(std::floor(angle / kTwoPi * params_.num_sectors)));
    const double value = std::max(0.0, point.z() + params_.lidar_height);
    descriptor(ring_idx, sector_idx) = std::max(descriptor(ring_idx, sector_idx), value);
  }

  return descriptor;
}

Eigen::VectorXd ScanContextManager::makeRingKey(
    const Eigen::MatrixXd& descriptor) const {
  if (descriptor.rows() == 0) {
    return Eigen::VectorXd();
  }
  return descriptor.rowwise().mean();
}

Eigen::VectorXd ScanContextManager::makeSectorKey(
    const Eigen::MatrixXd& descriptor) const {
  if (descriptor.cols() == 0) {
    return Eigen::VectorXd();
  }
  return descriptor.colwise().mean().transpose();
}

int ScanContextManager::addScan(const std::vector<Eigen::Vector3d>& points) {
  StoredContext context;
  context.descriptor = makeScanContext(points);
  context.ring_key = makeRingKey(context.descriptor);
  context.sector_key = makeSectorKey(context.descriptor);
  contexts_.push_back(std::move(context));
  return static_cast<int>(contexts_.size()) - 1;
}

LoopCandidate ScanContextManager::detectLoop(
    const std::vector<Eigen::Vector3d>& points) const {
  LoopCandidate result;
  if (contexts_.empty()) {
    return result;
  }

  const Eigen::MatrixXd query_descriptor = makeScanContext(points);
  const Eigen::VectorXd query_ring_key = makeRingKey(query_descriptor);
  const Eigen::VectorXd query_sector_key = makeSectorKey(query_descriptor);

  const int searchable_size = std::max(
      0, static_cast<int>(contexts_.size()) - params_.exclude_recent_frames);
  if (searchable_size <= 0) {
    return result;
  }

  std::vector<std::pair<double, int>> ring_key_distances;
  ring_key_distances.reserve(searchable_size);
  for (int i = 0; i < searchable_size; ++i) {
    const double distance = (contexts_[i].ring_key - query_ring_key).norm();
    ring_key_distances.emplace_back(distance, i);
  }

  const int candidate_count =
      std::min(params_.num_candidates, static_cast<int>(ring_key_distances.size()));
  std::partial_sort(ring_key_distances.begin(),
                    ring_key_distances.begin() + candidate_count,
                    ring_key_distances.end());

  double best_distance = std::numeric_limits<double>::infinity();
  int best_shift = 0;
  int best_index = -1;
  for (int i = 0; i < candidate_count; ++i) {
    int shift = 0;
    const double distance = compareDescriptors(
        query_descriptor, query_sector_key,
        contexts_[ring_key_distances[i].second], &shift);
    if (distance < best_distance) {
      best_distance = distance;
      best_shift = shift;
      best_index = ring_key_distances[i].second;
    }
  }

  if (best_index >= 0 && best_distance < params_.distance_threshold) {
    result.valid = true;
    result.index = best_index;
    result.distance = best_distance;
    result.yaw_shift = best_shift;
    result.yaw_offset_rad =
        static_cast<double>(best_shift) * kTwoPi / params_.num_sectors;
  }
  return result;
}

void ScanContextManager::clear() { contexts_.clear(); }

int ScanContextManager::alignSectorKeys(const Eigen::VectorXd& query,
                                        const Eigen::VectorXd& candidate) const {
  if (query.size() == 0 || candidate.size() != query.size()) {
    return 0;
  }

  double best_distance = std::numeric_limits<double>::infinity();
  int best_shift = 0;
  for (int shift = 0; shift < query.size(); ++shift) {
    double distance = 0.0;
    for (int col = 0; col < query.size(); ++col) {
      const int shifted_col = (col + shift) % query.size();
      distance += std::abs(query[col] - candidate[shifted_col]);
    }
    if (distance < best_distance) {
      best_distance = distance;
      best_shift = shift;
    }
  }
  return best_shift;
}

double ScanContextManager::compareDescriptors(
    const Eigen::MatrixXd& query, const Eigen::VectorXd& query_sector_key,
    const StoredContext& candidate, int* best_shift) const {
  if (query.rows() == 0 || query.cols() == 0 ||
      candidate.descriptor.rows() != query.rows() ||
      candidate.descriptor.cols() != query.cols()) {
    if (best_shift != nullptr) {
      *best_shift = 0;
    }
    return 1.0;
  }

  const int initial_shift = alignSectorKeys(query_sector_key, candidate.sector_key);
  const int search_radius = std::max(
      0, static_cast<int>(std::round(params_.search_ratio * query.cols())));

  double best_distance = std::numeric_limits<double>::infinity();
  int best_local_shift = initial_shift;
  for (int offset = -search_radius; offset <= search_radius; ++offset) {
    const int shift = (initial_shift + offset + query.cols()) % query.cols();
    const Eigen::MatrixXd shifted = circularShiftColumns(candidate.descriptor, shift);

    double similarity = 0.0;
    int valid_columns = 0;
    for (int col = 0; col < query.cols(); ++col) {
      const Eigen::VectorXd query_col = query.col(col);
      const Eigen::VectorXd candidate_col = shifted.col(col);
      const double query_norm = query_col.norm();
      const double candidate_norm = candidate_col.norm();
      if (query_norm < 1e-6 || candidate_norm < 1e-6) {
        continue;
      }
      similarity += query_col.dot(candidate_col) / (query_norm * candidate_norm);
      ++valid_columns;
    }

    const double distance =
        valid_columns > 0 ? 1.0 - similarity / valid_columns : 1.0;
    if (distance < best_distance) {
      best_distance = distance;
      best_local_shift = shift;
    }
  }

  if (best_shift != nullptr) {
    *best_shift = best_local_shift;
  }
  return best_distance;
}

Eigen::MatrixXd ScanContextManager::circularShiftColumns(
    const Eigen::MatrixXd& matrix, int shift) {
  if (matrix.cols() == 0) {
    return matrix;
  }

  Eigen::MatrixXd shifted = matrix;
  const int wrapped_shift = (shift % matrix.cols() + matrix.cols()) % matrix.cols();
  for (int col = 0; col < matrix.cols(); ++col) {
    shifted.col(col) = matrix.col((col + wrapped_shift) % matrix.cols());
  }
  return shifted;
}

}  // namespace scan_context
}  // namespace localization_zoo
