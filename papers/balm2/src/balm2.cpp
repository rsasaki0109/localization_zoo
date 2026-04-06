#include "balm2/balm2.h"

#include "aloam/lidar_factor.h"

#include <ceres/ceres.h>
#include <Eigen/Eigenvalues>
#include <pcl/filters/voxel_grid.h>
#include <pcl/kdtree/kdtree_flann.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>
#include <common/ceres_compat.h>

namespace localization_zoo {
namespace balm2 {

namespace {

struct PoseParameters {
  double q[4] = {0.0, 0.0, 0.0, 1.0};
  double t[3] = {0.0, 0.0, 0.0};
};

struct PointRef {
  int local_frame_index = -1;
  int point_index = -1;
};

struct PosePriorCost {
  PosePriorCost(const Eigen::Quaterniond& q_ij, const Eigen::Vector3d& t_ij,
                double rotation_weight, double translation_weight)
      : q_ij_(q_ij),
        t_ij_(t_ij),
        rotation_weight_(rotation_weight),
        translation_weight_(translation_weight) {}

  template <typename T>
  bool operator()(const T* const q_i_ptr, const T* const t_i_ptr,
                  const T* const q_j_ptr, const T* const t_j_ptr,
                  T* residuals_ptr) const {
    const Eigen::Quaternion<T> q_i{q_i_ptr[3], q_i_ptr[0], q_i_ptr[1],
                                   q_i_ptr[2]};
    const Eigen::Quaternion<T> q_j{q_j_ptr[3], q_j_ptr[0], q_j_ptr[1],
                                   q_j_ptr[2]};
    const Eigen::Matrix<T, 3, 1> t_i{t_i_ptr[0], t_i_ptr[1], t_i_ptr[2]};
    const Eigen::Matrix<T, 3, 1> t_j{t_j_ptr[0], t_j_ptr[1], t_j_ptr[2]};

    const Eigen::Quaternion<T> q_meas{T(q_ij_.w()), T(q_ij_.x()),
                                      T(q_ij_.y()), T(q_ij_.z())};
    const Eigen::Matrix<T, 3, 1> t_meas = t_ij_.template cast<T>();

    const Eigen::Quaternion<T> q_est = q_i.conjugate() * q_j;
    const Eigen::Quaternion<T> q_err = q_meas.conjugate() * q_est;
    const Eigen::Matrix<T, 3, 1> t_err =
        q_i.conjugate() * (t_j - t_i) - t_meas;

    Eigen::Map<Eigen::Matrix<T, 6, 1>> residuals(residuals_ptr);
    residuals.template head<3>() =
        T(rotation_weight_) * T(2.0) * q_err.vec();
    residuals.template tail<3>() = T(translation_weight_) * t_err;
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Quaterniond& q_ij,
                                     const Eigen::Vector3d& t_ij,
                                     double rotation_weight,
                                     double translation_weight) {
    return new ceres::AutoDiffCostFunction<PosePriorCost, 6, 4, 3, 4, 3>(
        new PosePriorCost(q_ij, t_ij, rotation_weight, translation_weight));
  }

  Eigen::Quaterniond q_ij_;
  Eigen::Vector3d t_ij_;
  double rotation_weight_;
  double translation_weight_;
};

template <typename T>
Eigen::Matrix<T, 3, 1> transformPoint(const T* const q_ptr,
                                      const T* const t_ptr,
                                      const Eigen::Vector3d& point) {
  const Eigen::Quaternion<T> q{q_ptr[3], q_ptr[0], q_ptr[1], q_ptr[2]};
  const Eigen::Matrix<T, 3, 1> t{t_ptr[0], t_ptr[1], t_ptr[2]};
  return q * point.template cast<T>() + t;
}

struct MultiPoseEdgeFactor {
  MultiPoseEdgeFactor(const Eigen::Vector3d& curr_point,
                      const Eigen::Vector3d& point_a,
                      const Eigen::Vector3d& point_b)
      : curr_point_(curr_point), point_a_(point_a), point_b_(point_b) {}

  template <typename T>
  bool operator()(const T* const q_curr, const T* const t_curr,
                  const T* const q_a, const T* const t_a, const T* const q_b,
                  const T* const t_b, T* residuals_ptr) const {
    const Eigen::Matrix<T, 3, 1> curr =
        transformPoint(q_curr, t_curr, curr_point_);
    const Eigen::Matrix<T, 3, 1> point_a =
        transformPoint(q_a, t_a, point_a_);
    const Eigen::Matrix<T, 3, 1> point_b =
        transformPoint(q_b, t_b, point_b_);

    const Eigen::Matrix<T, 3, 1> numerator =
        (curr - point_a).cross(curr - point_b);
    const T denominator = (point_a - point_b).norm() + T(1e-6);

    residuals_ptr[0] = numerator.x() / denominator;
    residuals_ptr[1] = numerator.y() / denominator;
    residuals_ptr[2] = numerator.z() / denominator;
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& curr_point,
                                     const Eigen::Vector3d& point_a,
                                     const Eigen::Vector3d& point_b) {
    return new ceres::AutoDiffCostFunction<MultiPoseEdgeFactor, 3, 4, 3, 4, 3,
                                           4, 3>(
        new MultiPoseEdgeFactor(curr_point, point_a, point_b));
  }

  Eigen::Vector3d curr_point_;
  Eigen::Vector3d point_a_;
  Eigen::Vector3d point_b_;
};

struct MultiPosePlaneFactor {
  MultiPosePlaneFactor(const Eigen::Vector3d& curr_point,
                       const Eigen::Vector3d& point_a,
                       const Eigen::Vector3d& point_b,
                       const Eigen::Vector3d& point_c)
      : curr_point_(curr_point),
        point_a_(point_a),
        point_b_(point_b),
        point_c_(point_c) {}

  template <typename T>
  bool operator()(const T* const q_curr, const T* const t_curr,
                  const T* const q_a, const T* const t_a, const T* const q_b,
                  const T* const t_b, const T* const q_c, const T* const t_c,
                  T* residuals_ptr) const {
    const Eigen::Matrix<T, 3, 1> curr =
        transformPoint(q_curr, t_curr, curr_point_);
    const Eigen::Matrix<T, 3, 1> point_a =
        transformPoint(q_a, t_a, point_a_);
    const Eigen::Matrix<T, 3, 1> point_b =
        transformPoint(q_b, t_b, point_b_);
    const Eigen::Matrix<T, 3, 1> point_c =
        transformPoint(q_c, t_c, point_c_);

    const Eigen::Matrix<T, 3, 1> normal =
        (point_a - point_b).cross(point_a - point_c);
    const T normal_norm = normal.norm() + T(1e-6);
    residuals_ptr[0] = (curr - point_a).dot(normal) / normal_norm;
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& curr_point,
                                     const Eigen::Vector3d& point_a,
                                     const Eigen::Vector3d& point_b,
                                     const Eigen::Vector3d& point_c) {
    return new ceres::AutoDiffCostFunction<MultiPosePlaneFactor, 1, 4, 3, 4,
                                           3, 4, 3, 4, 3>(
        new MultiPosePlaneFactor(curr_point, point_a, point_b, point_c));
  }

  Eigen::Vector3d curr_point_;
  Eigen::Vector3d point_a_;
  Eigen::Vector3d point_b_;
  Eigen::Vector3d point_c_;
};

aloam::PointCloudPtr downsampleCloud(const aloam::PointCloudPtr& cloud,
                                     double resolution) {
  auto downsampled = aloam::PointCloudPtr(new aloam::PointCloud);
  if (cloud == nullptr || cloud->empty()) {
    return downsampled;
  }

  pcl::VoxelGrid<aloam::PointT> voxel_grid;
  voxel_grid.setInputCloud(cloud);
  voxel_grid.setLeafSize(resolution, resolution, resolution);
  voxel_grid.filter(*downsampled);
  return downsampled;
}

Eigen::Vector3d toEigen(const aloam::PointT& point) {
  return Eigen::Vector3d(point.x, point.y, point.z);
}

aloam::PointT transformToWorld(const aloam::PointT& point,
                               const PoseParameters& pose) {
  const Eigen::Quaterniond quat(pose.q[3], pose.q[0], pose.q[1], pose.q[2]);
  const Eigen::Vector3d trans(pose.t[0], pose.t[1], pose.t[2]);
  const Eigen::Vector3d world = quat * toEigen(point) + trans;

  aloam::PointT transformed = point;
  transformed.x = static_cast<float>(world.x());
  transformed.y = static_cast<float>(world.y());
  transformed.z = static_cast<float>(world.z());
  transformed.intensity = 0.0f;
  return transformed;
}

int chooseLinePair(const std::vector<Eigen::Vector3d>& neighbors,
                   const std::vector<PointRef>& refs,
                   std::array<int, 2>* pair) {
  if (neighbors.size() < 2) {
    return 0;
  }

  double best_distance_sq = 0.0;
  std::array<int, 2> best_pair{0, 1};
  for (size_t i = 0; i < neighbors.size(); ++i) {
    for (size_t j = i + 1; j < neighbors.size(); ++j) {
      if (refs[i].local_frame_index == refs[j].local_frame_index) {
        continue;
      }
      const double distance_sq = (neighbors[i] - neighbors[j]).squaredNorm();
      if (distance_sq > best_distance_sq) {
        best_distance_sq = distance_sq;
        best_pair = {static_cast<int>(i), static_cast<int>(j)};
      }
    }
  }

  *pair = best_pair;
  return best_distance_sq > 1e-6 ? 1 : 0;
}

int choosePlaneTriplet(const std::vector<Eigen::Vector3d>& neighbors,
                       const std::vector<PointRef>& refs,
                       std::array<int, 3>* triplet) {
  if (neighbors.size() < 3) {
    return 0;
  }

  double best_area_sq = 0.0;
  std::array<int, 3> best_triplet{0, 1, 2};
  for (size_t i = 0; i < neighbors.size(); ++i) {
    for (size_t j = i + 1; j < neighbors.size(); ++j) {
      for (size_t k = j + 1; k < neighbors.size(); ++k) {
        if (refs[i].local_frame_index == refs[j].local_frame_index ||
            refs[i].local_frame_index == refs[k].local_frame_index ||
            refs[j].local_frame_index == refs[k].local_frame_index) {
          continue;
        }
        const Eigen::Vector3d cross =
            (neighbors[i] - neighbors[j]).cross(neighbors[i] - neighbors[k]);
        const double area_sq = cross.squaredNorm();
        if (area_sq > best_area_sq) {
          best_area_sq = area_sq;
          best_triplet = {static_cast<int>(i), static_cast<int>(j),
                          static_cast<int>(k)};
        }
      }
    }
  }

  *triplet = best_triplet;
  return best_area_sq > 1e-6 ? 1 : 0;
}

}  // namespace

BALM2::BALM2(const Balm2Params& params)
    : params_(params),
      scan_registration_(params.scan_registration),
      odometry_(params.odometry) {}

int BALM2::currentWindowSize() const {
  return std::min(params_.window_size, static_cast<int>(keyframes_.size()));
}

BALM2::PoseState BALM2::relativePose(const PoseState& from, const PoseState& to) {
  PoseState relative;
  relative.q = from.q.conjugate() * to.q;
  relative.t = from.q.conjugate() * (to.t - from.t);
  return relative;
}

BALM2::PoseState BALM2::composePose(const PoseState& a, const PoseState& b) {
  PoseState composed;
  composed.q = (a.q * b.q).normalized();
  composed.t = a.q * b.t + a.t;
  return composed;
}

BALM2::PoseState BALM2::invertPose(const PoseState& pose) {
  PoseState inverted;
  inverted.q = pose.q.conjugate();
  inverted.t = -(inverted.q * pose.t);
  return inverted;
}

aloam::PointCloudPtr BALM2::prepareCornerCloud(
    const aloam::FeatureCloud& features) const {
  aloam::PointCloudPtr source = features.corner_less_sharp;
  if (source == nullptr || source->empty()) {
    source = features.corner_sharp;
  }
  return downsampleCloud(source, params_.corner_resolution);
}

aloam::PointCloudPtr BALM2::prepareSurfaceCloud(
    const aloam::FeatureCloud& features) const {
  aloam::PointCloudPtr source = features.surf_less_flat;
  if (source == nullptr || source->empty()) {
    source = features.surf_flat;
  }
  return downsampleCloud(source, params_.surface_resolution);
}

Balm2Result BALM2::process(const aloam::PointCloudConstPtr& cloud) {
  Balm2Result result;

  const aloam::FeatureCloud features = scan_registration_.extract(cloud);
  const aloam::OdometryResult odom_result = odometry_.process(features);
  if (!odom_result.valid) {
    result.num_keyframes = numKeyframes();
    result.num_window_keyframes = currentWindowSize();
    return result;
  }

  PoseState raw_pose;
  raw_pose.q = odom_result.q_w_curr.normalized();
  raw_pose.t = odom_result.t_w_curr;

  PoseState optimized_pose = raw_pose;
  if (!keyframes_.empty()) {
    const PoseState correction = composePose(
        keyframes_.back().optimized_pose, invertPose(keyframes_.back().raw_pose));
    optimized_pose = composePose(correction, raw_pose);
  }

  Keyframe keyframe;
  keyframe.raw_pose = raw_pose;
  keyframe.optimized_pose = optimized_pose;
  keyframe.corner_points = prepareCornerCloud(features);
  keyframe.surface_points = prepareSurfaceCloud(features);
  keyframes_.push_back(std::move(keyframe));

  result.valid = true;
  optimizeWindow(&result);

  result.q_w_curr = keyframes_.back().optimized_pose.q;
  result.t_w_curr = keyframes_.back().optimized_pose.t;
  result.num_keyframes = numKeyframes();
  result.num_window_keyframes = currentWindowSize();
  return result;
}

void BALM2::clear() {
  keyframes_.clear();
  odometry_ = aloam::LaserOdometry(params_.odometry);
}

void BALM2::optimizeWindow(Balm2Result* result) {
  if (keyframes_.empty()) {
    return;
  }

  const int window_end = static_cast<int>(keyframes_.size());
  const int window_start = std::max(0, window_end - params_.window_size);
  const int window_size = window_end - window_start;
  if (window_size <= 1) {
    result->q_w_curr = keyframes_.back().optimized_pose.q;
    result->t_w_curr = keyframes_.back().optimized_pose.t;
    return;
  }

  std::vector<PoseParameters> pose_parameters(window_size);
  for (int i = 0; i < window_size; ++i) {
    const PoseState& pose = keyframes_[window_start + i].optimized_pose;
    pose_parameters[i].q[0] = pose.q.x();
    pose_parameters[i].q[1] = pose.q.y();
    pose_parameters[i].q[2] = pose.q.z();
    pose_parameters[i].q[3] = pose.q.w();
    pose_parameters[i].t[0] = pose.t.x();
    pose_parameters[i].t[1] = pose.t.y();
    pose_parameters[i].t[2] = pose.t.z();
  }

  std::vector<aloam::PointCloudPtr> corner_world(window_size);
  std::vector<aloam::PointCloudPtr> surface_world(window_size);

  for (int outer_iter = 0; outer_iter < params_.num_outer_iterations;
       ++outer_iter) {
    for (int i = 0; i < window_size; ++i) {
      corner_world[i].reset(new aloam::PointCloud);
      surface_world[i].reset(new aloam::PointCloud);

      const auto& corner_points = keyframes_[window_start + i].corner_points;
      const auto& surface_points = keyframes_[window_start + i].surface_points;
      corner_world[i]->reserve(corner_points->size());
      surface_world[i]->reserve(surface_points->size());

      for (const auto& point : corner_points->points) {
        corner_world[i]->push_back(transformToWorld(point, pose_parameters[i]));
      }
      for (const auto& point : surface_points->points) {
        surface_world[i]->push_back(transformToWorld(point, pose_parameters[i]));
      }
    }

    ceres::Problem problem;
    for (int i = 0; i < window_size; ++i) {
      problem.AddParameterBlock(
          pose_parameters[i].q, 4);
    localization_zoo::SetEigenQuaternionManifold(problem, pose_parameters[i].q);
      problem.AddParameterBlock(pose_parameters[i].t, 3);
    }
    problem.SetParameterBlockConstant(pose_parameters.front().q);
    problem.SetParameterBlockConstant(pose_parameters.front().t);

    ceres::LossFunction* geometric_loss =
        new ceres::HuberLoss(params_.huber_loss_s);

    result->num_line_constraints = 0;
    result->num_plane_constraints = 0;

    std::vector<int> search_indices;
    std::vector<float> search_distances;

    for (int local_frame = 0; local_frame < window_size; ++local_frame) {
      auto corner_map = aloam::PointCloudPtr(new aloam::PointCloud);
      auto surface_map = aloam::PointCloudPtr(new aloam::PointCloud);
      std::vector<PointRef> corner_refs;
      std::vector<PointRef> surface_refs;

      for (int other = 0; other < window_size; ++other) {
        if (other == local_frame) {
          continue;
        }

        for (size_t point_idx = 0; point_idx < corner_world[other]->size();
             ++point_idx) {
          corner_map->push_back(corner_world[other]->points[point_idx]);
          corner_refs.push_back({other, static_cast<int>(point_idx)});
        }
        for (size_t point_idx = 0; point_idx < surface_world[other]->size();
             ++point_idx) {
          surface_map->push_back(surface_world[other]->points[point_idx]);
          surface_refs.push_back({other, static_cast<int>(point_idx)});
        }
      }

      pcl::KdTreeFLANN<aloam::PointT> corner_tree;
      pcl::KdTreeFLANN<aloam::PointT> surface_tree;
      const bool has_corner_map =
          corner_map->size() >= static_cast<size_t>(params_.knn);
      const bool has_surface_map =
          surface_map->size() >= static_cast<size_t>(params_.knn);
      if (has_corner_map) {
        corner_tree.setInputCloud(corner_map);
      }
      if (has_surface_map) {
        surface_tree.setInputCloud(surface_map);
      }

      const auto& source_corner =
          keyframes_[window_start + local_frame].corner_points;
      const auto& source_surface =
          keyframes_[window_start + local_frame].surface_points;

      if (has_corner_map) {
        for (size_t source_idx = 0; source_idx < source_corner->size();
             source_idx += std::max(1, params_.corner_stride)) {
          const aloam::PointT world_query =
              transformToWorld(source_corner->points[source_idx],
                               pose_parameters[local_frame]);
          search_indices.clear();
          search_distances.clear();
          corner_tree.nearestKSearch(world_query, params_.knn, search_indices,
                                     search_distances);
          if (search_distances.size() < static_cast<size_t>(params_.knn) ||
              search_distances.back() > params_.knn_max_dist_sq) {
            continue;
          }

          std::vector<Eigen::Vector3d> neighbors;
          neighbors.reserve(search_indices.size());
          for (int idx : search_indices) {
            neighbors.push_back(toEigen(corner_map->points[idx]));
          }

          Eigen::Vector3d center = Eigen::Vector3d::Zero();
          for (const auto& neighbor : neighbors) {
            center += neighbor;
          }
          center /= static_cast<double>(neighbors.size());

          Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
          for (const auto& neighbor : neighbors) {
            const Eigen::Vector3d diff = neighbor - center;
            covariance += diff * diff.transpose();
          }
          covariance /= static_cast<double>(neighbors.size());

          Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(covariance);
          const Eigen::Vector3d eigenvalues = solver.eigenvalues();
          if (eigenvalues(2) <=
              params_.edge_eigenvalue_ratio * eigenvalues(1)) {
            continue;
          }

          std::array<int, 2> pair;
          std::vector<PointRef> local_refs;
          local_refs.reserve(search_indices.size());
          for (int idx : search_indices) {
            local_refs.push_back(corner_refs[idx]);
          }

          if (!chooseLinePair(neighbors, local_refs, &pair)) {
            continue;
          }

          const PointRef ref_a = local_refs[pair[0]];
          const PointRef ref_b = local_refs[pair[1]];
          const Eigen::Vector3d curr_point =
              toEigen(source_corner->points[source_idx]);
          const Eigen::Vector3d point_a = toEigen(
              keyframes_[window_start + ref_a.local_frame_index]
                  .corner_points->points[ref_a.point_index]);
          const Eigen::Vector3d point_b = toEigen(
              keyframes_[window_start + ref_b.local_frame_index]
                  .corner_points->points[ref_b.point_index]);

          problem.AddResidualBlock(
              MultiPoseEdgeFactor::Create(curr_point, point_a, point_b),
              geometric_loss, pose_parameters[local_frame].q,
              pose_parameters[local_frame].t,
              pose_parameters[ref_a.local_frame_index].q,
              pose_parameters[ref_a.local_frame_index].t,
              pose_parameters[ref_b.local_frame_index].q,
              pose_parameters[ref_b.local_frame_index].t);
          ++result->num_line_constraints;
        }
      }

      if (has_surface_map) {
        for (size_t source_idx = 0; source_idx < source_surface->size();
             source_idx += std::max(1, params_.surface_stride)) {
          const aloam::PointT world_query =
              transformToWorld(source_surface->points[source_idx],
                               pose_parameters[local_frame]);
          search_indices.clear();
          search_distances.clear();
          surface_tree.nearestKSearch(world_query, params_.knn, search_indices,
                                      search_distances);
          if (search_distances.size() < static_cast<size_t>(params_.knn) ||
              search_distances.back() > params_.knn_max_dist_sq) {
            continue;
          }

          std::vector<Eigen::Vector3d> neighbors;
          neighbors.reserve(search_indices.size());
          for (int idx : search_indices) {
            neighbors.push_back(toEigen(surface_map->points[idx]));
          }

          std::array<int, 3> triplet;
          std::vector<PointRef> local_refs;
          local_refs.reserve(search_indices.size());
          for (int idx : search_indices) {
            local_refs.push_back(surface_refs[idx]);
          }

          if (!choosePlaneTriplet(neighbors, local_refs, &triplet)) {
            continue;
          }

          const Eigen::Vector3d normal =
              (neighbors[triplet[0]] - neighbors[triplet[1]])
                  .cross(neighbors[triplet[0]] - neighbors[triplet[2]]);
          if (normal.norm() < 1e-6) {
            continue;
          }

          const Eigen::Vector3d unit_normal = normal.normalized();
          bool plane_valid = true;
          for (const auto& neighbor : neighbors) {
            if (std::abs((neighbor - neighbors[triplet[0]]).dot(unit_normal)) >
                params_.plane_threshold) {
              plane_valid = false;
              break;
            }
          }
          if (!plane_valid) {
            continue;
          }

          const PointRef ref_a = local_refs[triplet[0]];
          const PointRef ref_b = local_refs[triplet[1]];
          const PointRef ref_c = local_refs[triplet[2]];
          const Eigen::Vector3d curr_point =
              toEigen(source_surface->points[source_idx]);
          const Eigen::Vector3d point_a = toEigen(
              keyframes_[window_start + ref_a.local_frame_index]
                  .surface_points->points[ref_a.point_index]);
          const Eigen::Vector3d point_b = toEigen(
              keyframes_[window_start + ref_b.local_frame_index]
                  .surface_points->points[ref_b.point_index]);
          const Eigen::Vector3d point_c = toEigen(
              keyframes_[window_start + ref_c.local_frame_index]
                  .surface_points->points[ref_c.point_index]);

          problem.AddResidualBlock(
              MultiPosePlaneFactor::Create(curr_point, point_a, point_b, point_c),
              geometric_loss, pose_parameters[local_frame].q,
              pose_parameters[local_frame].t,
              pose_parameters[ref_a.local_frame_index].q,
              pose_parameters[ref_a.local_frame_index].t,
              pose_parameters[ref_b.local_frame_index].q,
              pose_parameters[ref_b.local_frame_index].t,
              pose_parameters[ref_c.local_frame_index].q,
              pose_parameters[ref_c.local_frame_index].t);
          ++result->num_plane_constraints;
        }
      }
    }

    for (int i = 1; i < window_size; ++i) {
      const PoseState relative = relativePose(
          keyframes_[window_start + i - 1].raw_pose,
          keyframes_[window_start + i].raw_pose);
      problem.AddResidualBlock(
          PosePriorCost::Create(relative.q, relative.t,
                                params_.odom_rotation_weight,
                                params_.odom_translation_weight),
          nullptr, pose_parameters[i - 1].q, pose_parameters[i - 1].t,
          pose_parameters[i].q, pose_parameters[i].t);
    }

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.max_num_iterations = params_.ceres_max_iterations;
    options.minimizer_progress_to_stdout = false;

    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
  }

  for (int i = 0; i < window_size; ++i) {
    PoseState optimized_pose;
    optimized_pose.q = Eigen::Quaterniond(pose_parameters[i].q[3],
                                          pose_parameters[i].q[0],
                                          pose_parameters[i].q[1],
                                          pose_parameters[i].q[2])
                           .normalized();
    optimized_pose.t = Eigen::Vector3d(pose_parameters[i].t[0],
                                       pose_parameters[i].t[1],
                                       pose_parameters[i].t[2]);
    keyframes_[window_start + i].optimized_pose = optimized_pose;
  }

  result->q_w_curr = keyframes_.back().optimized_pose.q;
  result->t_w_curr = keyframes_.back().optimized_pose.t;
}

}  // namespace balm2
}  // namespace localization_zoo
