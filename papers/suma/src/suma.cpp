#include "suma/suma.h"

#include "aloam/lidar_factor.h"

#include <ceres/ceres.h>
#include <pcl/kdtree/kdtree_flann.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace localization_zoo {
namespace suma {

namespace {

struct VoxelKey {
  int x = 0;
  int y = 0;
  int z = 0;

  bool operator==(const VoxelKey& other) const {
    return x == other.x && y == other.y && z == other.z;
  }
};

struct VoxelKeyHash {
  std::size_t operator()(const VoxelKey& key) const {
    std::size_t seed = 0;
    auto mix = [&seed](int value) {
      seed ^= std::hash<int>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    mix(key.x);
    mix(key.y);
    mix(key.z);
    return seed;
  }
};

struct SurfelAccumulator {
  Eigen::Vector3d point_sum = Eigen::Vector3d::Zero();
  Eigen::Vector3d normal_sum = Eigen::Vector3d::Zero();
  int count = 0;
};

struct RangePixel {
  bool valid = false;
  double range = std::numeric_limits<double>::infinity();
  aloam::PointT point;
};

Eigen::Vector3d toEigen(const aloam::PointT& point) {
  return Eigen::Vector3d(point.x, point.y, point.z);
}

aloam::PointT toPoint(const Eigen::Vector3d& point) {
  aloam::PointT pcl_point;
  pcl_point.x = static_cast<float>(point.x());
  pcl_point.y = static_cast<float>(point.y());
  pcl_point.z = static_cast<float>(point.z());
  pcl_point.intensity = 0.0f;
  return pcl_point;
}

int wrapColumn(int column, int num_columns) {
  int wrapped = column % num_columns;
  if (wrapped < 0) {
    wrapped += num_columns;
  }
  return wrapped;
}

}  // namespace

SuMa::SuMa(const SuMaParams& params)
    : params_(params), map_points_(new aloam::PointCloud) {}

int SuMa::computeScanId(float x, float y, float z) const {
  const float vertical_angle =
      std::atan2(z, std::sqrt(x * x + y * y)) * 180.0f / static_cast<float>(M_PI);

  int scan_id = 0;
  if (params_.n_scans == 16) {
    scan_id = static_cast<int>(vertical_angle + 15.0f) / 2 + 0.5f;
    if (scan_id < 0 || scan_id >= params_.n_scans) {
      return -1;
    }
  } else if (params_.n_scans == 32) {
    scan_id = static_cast<int>((vertical_angle + 92.0f / 3.0f) * 3.0f / 4.0f);
    if (scan_id < 0 || scan_id >= params_.n_scans) {
      return -1;
    }
  } else if (params_.n_scans == 64) {
    if (vertical_angle >= -8.83f) {
      scan_id = static_cast<int>((2.0f - vertical_angle) * 3.0f + 0.5f);
    } else {
      scan_id = params_.n_scans / 2 +
                static_cast<int>((-8.83f - vertical_angle) * 2.0f + 0.5f);
    }
    if (scan_id < 0 || scan_id >= params_.n_scans) {
      return -1;
    }
  } else {
    const float fov = 30.0f;
    scan_id = static_cast<int>((vertical_angle + fov / 2.0f) /
                               (fov / params_.n_scans));
    if (scan_id < 0 || scan_id >= params_.n_scans) {
      return -1;
    }
  }

  return scan_id;
}

SuMa::PoseState SuMa::composePose(const PoseState& a, const PoseState& b) {
  PoseState composed;
  composed.q = (a.q * b.q).normalized();
  composed.t = a.q * b.t + a.t;
  return composed;
}

SuMa::PoseState SuMa::relativePose(const PoseState& from, const PoseState& to) {
  PoseState relative;
  relative.q = from.q.conjugate() * to.q;
  relative.t = from.q.conjugate() * (to.t - from.t);
  return relative;
}

SuMa::SurfelFrame SuMa::voxelizeSurfels(const SurfelFrame& frame) const {
  SurfelFrame voxelized;
  voxelized.points.reset(new aloam::PointCloud);
  if (frame.points == nullptr || frame.points->empty()) {
    return voxelized;
  }

  std::unordered_map<VoxelKey, SurfelAccumulator, VoxelKeyHash> accumulators;
  accumulators.reserve(frame.points->size());

  for (size_t i = 0; i < frame.points->size(); ++i) {
    const Eigen::Vector3d point = toEigen(frame.points->points[i]);
    const VoxelKey key{
        static_cast<int>(std::floor(point.x() / params_.surfel_resolution)),
        static_cast<int>(std::floor(point.y() / params_.surfel_resolution)),
        static_cast<int>(std::floor(point.z() / params_.surfel_resolution))};
    auto& accumulator = accumulators[key];
    accumulator.point_sum += point;
    accumulator.normal_sum += frame.normals[i];
    accumulator.count += 1;
  }

  voxelized.points->reserve(accumulators.size());
  voxelized.normals.reserve(accumulators.size());
  for (const auto& [key, accumulator] : accumulators) {
    (void)key;
    const Eigen::Vector3d point =
        accumulator.point_sum / static_cast<double>(accumulator.count);
    Eigen::Vector3d normal = accumulator.normal_sum;
    if (normal.norm() < 1e-6) {
      continue;
    }
    normal.normalize();
    voxelized.points->push_back(toPoint(point));
    voxelized.normals.push_back(normal);
  }

  return voxelized;
}

SuMa::SurfelFrame SuMa::extractSurfels(
    const aloam::PointCloudConstPtr& cloud) const {
  SuMa::SurfelFrame frame;
  frame.points.reset(new aloam::PointCloud);
  if (cloud == nullptr || cloud->empty()) {
    return frame;
  }

  std::vector<RangePixel> range_image(
      static_cast<size_t>(params_.n_scans * params_.num_columns));

  for (const auto& point : cloud->points) {
    if (!std::isfinite(point.x) || !std::isfinite(point.y) ||
        !std::isfinite(point.z)) {
      continue;
    }

    const double range =
        std::sqrt(point.x * point.x + point.y * point.y + point.z * point.z);
    if (range < params_.min_range || range > params_.max_range) {
      continue;
    }

    const int row = computeScanId(point.x, point.y, point.z);
    if (row < 0) {
      continue;
    }

    const double yaw = std::atan2(point.y, point.x);
    const int column = std::clamp(
        static_cast<int>(((yaw + M_PI) / (2.0 * M_PI)) * params_.num_columns),
        0, params_.num_columns - 1);
    RangePixel& pixel =
        range_image[static_cast<size_t>(row * params_.num_columns + column)];
    if (!pixel.valid || range < pixel.range) {
      pixel.valid = true;
      pixel.range = range;
      pixel.point = point;
    }
  }

  auto pixelAt = [&](int row, int column) -> const RangePixel* {
    if (row < 0 || row >= params_.n_scans) {
      return nullptr;
    }
    const int wrapped_column = wrapColumn(column, params_.num_columns);
    const RangePixel& pixel =
        range_image[static_cast<size_t>(row * params_.num_columns + wrapped_column)];
    return pixel.valid ? &pixel : nullptr;
  };

  auto findNeighbor = [&](int row, int column, int d_row, int d_column,
                          int max_steps) -> const RangePixel* {
    for (int step = 1; step <= max_steps; ++step) {
      const RangePixel* pixel =
          pixelAt(row + step * d_row, column + step * d_column);
      if (pixel != nullptr) {
        return pixel;
      }
    }
    return nullptr;
  };

  for (int row = 1; row < params_.n_scans - 1; ++row) {
    for (int column = 0; column < params_.num_columns; ++column) {
      const RangePixel* center = pixelAt(row, column);
      const RangePixel* left = findNeighbor(row, column, 0, -1, 6);
      const RangePixel* right = findNeighbor(row, column, 0, 1, 6);
      const RangePixel* up = findNeighbor(row, column, -1, 0, 2);
      const RangePixel* down = findNeighbor(row, column, 1, 0, 2);
      if (center == nullptr || left == nullptr || right == nullptr ||
          up == nullptr || down == nullptr) {
        continue;
      }

      const Eigen::Vector3d p_center = toEigen(center->point);
      const Eigen::Vector3d dx = toEigen(right->point) - toEigen(left->point);
      const Eigen::Vector3d dy = toEigen(down->point) - toEigen(up->point);
      if (dx.norm() < 1e-3 || dy.norm() < 1e-3 || dx.norm() > 5.0 ||
          dy.norm() > 5.0) {
        continue;
      }

      Eigen::Vector3d normal = dy.cross(dx);
      if (normal.norm() < 1e-6) {
        continue;
      }
      normal.normalize();
      if (normal.dot(p_center) > 0.0) {
        normal = -normal;
      }

      frame.points->push_back(center->point);
      frame.normals.push_back(normal);
    }
  }

  return voxelizeSurfels(frame);
}

SuMa::SurfelFrame SuMa::transformFrame(const SurfelFrame& frame,
                                       const Eigen::Quaterniond& quat,
                                       const Eigen::Vector3d& trans) const {
  SuMa::SurfelFrame transformed;
  transformed.points.reset(new aloam::PointCloud);
  if (frame.points == nullptr || frame.points->empty()) {
    return transformed;
  }

  transformed.points->reserve(frame.points->size());
  transformed.normals.reserve(frame.normals.size());
  for (size_t i = 0; i < frame.points->size(); ++i) {
    const Eigen::Vector3d point_world = quat * toEigen(frame.points->points[i]) + trans;
    transformed.points->push_back(toPoint(point_world));
    transformed.normals.push_back((quat * frame.normals[i]).normalized());
  }

  return voxelizeSurfels(transformed);
}

void SuMa::rebuildMap() {
  map_points_->clear();
  map_normals_.clear();

  for (const auto& frame : frame_history_) {
    *map_points_ += *frame.points;
    map_normals_.insert(map_normals_.end(), frame.normals.begin(), frame.normals.end());
  }
}

SuMaResult SuMa::process(const aloam::PointCloudConstPtr& cloud) {
  SuMaResult result;
  result.q_w_curr = q_w_curr_;
  result.t_w_curr = t_w_curr_;
  result.num_frames_in_map = frame_history_.size();

  const SurfelFrame local_frame = extractSurfels(cloud);
  result.num_surfels = local_frame.points->size();
  result.initialized = initialized_;
  if (local_frame.points->empty()) {
    return result;
  }

  if (!initialized_ || map_points_->empty()) {
    const SurfelFrame world_frame =
        transformFrame(local_frame, q_w_curr_, t_w_curr_);
    frame_history_.push_back(world_frame);
    initialized_ = true;
    while (static_cast<int>(frame_history_.size()) > params_.max_frames_in_map) {
      frame_history_.pop_front();
    }
    rebuildMap();
    result.initialized = true;
    result.num_frames_in_map = frame_history_.size();
    return result;
  }

  PoseState predicted_pose;
  predicted_pose.q = q_w_curr_;
  predicted_pose.t = t_w_curr_;

  double parameters[7] = {predicted_pose.q.x(), predicted_pose.q.y(),
                          predicted_pose.q.z(), predicted_pose.q.w(),
                          predicted_pose.t.x(), predicted_pose.t.y(),
                          predicted_pose.t.z()};
  Eigen::Map<Eigen::Quaterniond> q_w_curr(parameters);
  Eigen::Map<Eigen::Vector3d> t_w_curr(parameters + 4);

  pcl::KdTreeFLANN<aloam::PointT> map_tree;
  map_tree.setInputCloud(map_points_);

  std::vector<int> search_indices;
  std::vector<float> search_distances;

  for (int iter = 0; iter < params_.max_iterations; ++iter) {
    ceres::Problem problem;
    problem.AddParameterBlock(parameters, 4);
    problem.SetManifold(parameters, new ceres::EigenQuaternionManifold());
    problem.AddParameterBlock(parameters + 4, 3);

    result.num_correspondences = 0;

    for (size_t i = 0; i < local_frame.points->size();
         i += std::max(1, params_.surfel_stride)) {
      const Eigen::Vector3d local_point = toEigen(local_frame.points->points[i]);
      const Eigen::Vector3d world_point = q_w_curr * local_point + t_w_curr;
      aloam::PointT query = toPoint(world_point);

      search_indices.clear();
      search_distances.clear();
      map_tree.nearestKSearch(query, std::max(1, params_.knn), search_indices,
                              search_distances);
      if (search_indices.empty() ||
          search_distances.front() >
              params_.max_correspondence_distance * params_.max_correspondence_distance) {
        continue;
      }

      const Eigen::Vector3d world_normal =
          (q_w_curr * local_frame.normals[i]).normalized();
      int best_index = -1;
      double best_alignment = -1.0;
      for (size_t k = 0; k < search_indices.size(); ++k) {
        if (search_distances[k] >
            params_.max_correspondence_distance * params_.max_correspondence_distance) {
          continue;
        }
        const int idx = search_indices[k];
        const double alignment =
            std::abs(world_normal.dot(map_normals_[static_cast<size_t>(idx)]));
        if (alignment > best_alignment) {
          best_alignment = alignment;
          best_index = idx;
        }
      }
      if (best_index < 0 || best_alignment < params_.normal_alignment_threshold) {
        continue;
      }

      const Eigen::Vector3d plane_center =
          toEigen(map_points_->points[best_index]);
      Eigen::Vector3d plane_normal = map_normals_[static_cast<size_t>(best_index)];
      if (plane_normal.dot(world_normal) < 0.0) {
        plane_normal = -plane_normal;
      }

      const double negative_oa_dot_norm = -plane_normal.dot(plane_center);
      problem.AddResidualBlock(
          aloam::LidarPlaneNormFactor::Create(local_point, plane_normal,
                                              negative_oa_dot_norm),
          new ceres::HuberLoss(params_.huber_loss_s), parameters, parameters + 4);
      ++result.num_correspondences;
    }

    if (result.num_correspondences == 0) {
      break;
    }

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.max_num_iterations = params_.ceres_max_iterations;
    options.minimizer_progress_to_stdout = false;

    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
  }

  const PoseState previous_pose{q_w_curr_, t_w_curr_};
  q_w_curr_ = q_w_curr.normalized();
  t_w_curr_ = t_w_curr;
  const PoseState current_pose{q_w_curr_, t_w_curr_};
  last_motion_ = relativePose(previous_pose, current_pose);

  const SurfelFrame world_frame = transformFrame(local_frame, q_w_curr_, t_w_curr_);
  frame_history_.push_back(world_frame);
  while (static_cast<int>(frame_history_.size()) > params_.max_frames_in_map) {
    frame_history_.pop_front();
  }
  rebuildMap();

  result.q_w_curr = q_w_curr_;
  result.t_w_curr = t_w_curr_;
  result.num_frames_in_map = frame_history_.size();
  result.initialized = true;
  result.valid = result.num_correspondences > 0;
  return result;
}

void SuMa::clear() {
  frame_history_.clear();
  map_points_->clear();
  map_normals_.clear();
  q_w_curr_ = Eigen::Quaterniond::Identity();
  t_w_curr_.setZero();
  last_motion_ = PoseState();
  initialized_ = false;
}

}  // namespace suma
}  // namespace localization_zoo
