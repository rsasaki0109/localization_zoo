#include "mulls/mulls.h"

#include "aloam/lidar_factor.h"

#include <ceres/ceres.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <Eigen/Eigenvalues>

#include <algorithm>
#include <common/ceres_compat.h>

namespace localization_zoo {
namespace mulls {

namespace {

struct LidarPointFactor {
  LidarPointFactor(const Eigen::Vector3d& curr_point,
                   const Eigen::Vector3d& map_point)
      : curr_point_(curr_point), map_point_(map_point) {}

  template <typename T>
  bool operator()(const T* q, const T* t, T* residual) const {
    const Eigen::Quaternion<T> q_w{q[3], q[0], q[1], q[2]};
    const Eigen::Matrix<T, 3, 1> t_w{t[0], t[1], t[2]};
    const Eigen::Matrix<T, 3, 1> curr{T(curr_point_.x()), T(curr_point_.y()),
                                      T(curr_point_.z())};
    const Eigen::Matrix<T, 3, 1> map{T(map_point_.x()), T(map_point_.y()),
                                     T(map_point_.z())};
    const Eigen::Matrix<T, 3, 1> point_w = q_w * curr + t_w;
    residual[0] = point_w.x() - map.x();
    residual[1] = point_w.y() - map.y();
    residual[2] = point_w.z() - map.z();
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& curr_point,
                                     const Eigen::Vector3d& map_point) {
    return new ceres::AutoDiffCostFunction<LidarPointFactor, 3, 4, 3>(
        new LidarPointFactor(curr_point, map_point));
  }

  Eigen::Vector3d curr_point_;
  Eigen::Vector3d map_point_;
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

aloam::PointT transformPoint(const aloam::PointT& point,
                             const Eigen::Quaterniond& quat,
                             const Eigen::Vector3d& trans) {
  const Eigen::Vector3d local(point.x, point.y, point.z);
  const Eigen::Vector3d world = quat * local + trans;
  aloam::PointT transformed = point;
  transformed.x = world.x();
  transformed.y = world.y();
  transformed.z = world.z();
  return transformed;
}

}  // namespace

MULLSMapping::MULLSMapping(const MULLSMappingParams& params)
    : params_(params),
      corner_map_(new aloam::PointCloud),
      surf_map_(new aloam::PointCloud),
      point_map_(new aloam::PointCloud) {}

void MULLSMapping::rebuildMaps() {
  corner_map_->clear();
  surf_map_->clear();
  point_map_->clear();

  for (const auto& frame : frame_history_) {
    *corner_map_ += *frame.corner;
    *surf_map_ += *frame.surf;
    *point_map_ += *frame.points;
  }
}

MULLSMapping::FrameClouds MULLSMapping::makeFrameClouds(
    const aloam::PointCloudPtr& corner_last,
    const aloam::PointCloudPtr& surf_last,
    const aloam::PointCloudPtr& full_res) const {
  FrameClouds frame;
  frame.corner = aloam::PointCloudPtr(new aloam::PointCloud);
  frame.surf = aloam::PointCloudPtr(new aloam::PointCloud);
  frame.points = aloam::PointCloudPtr(new aloam::PointCloud);

  if (corner_last != nullptr) {
    *frame.corner = *downsampleCloud(corner_last, params_.line_resolution);
  }
  if (surf_last != nullptr) {
    *frame.surf = *downsampleCloud(surf_last, params_.plane_resolution);
  }
  if (full_res != nullptr) {
    aloam::PointCloudPtr sampled(new aloam::PointCloud);
    for (size_t i = 0; i < full_res->size();
         i += std::max(1, params_.full_downsample_rate)) {
      sampled->push_back(full_res->points[i]);
    }
    *frame.points = *downsampleCloud(sampled, params_.point_resolution);
  }

  return frame;
}

MULLSMappingResult MULLSMapping::process(const aloam::PointCloudPtr& corner_last,
                                         const aloam::PointCloudPtr& surf_last,
                                         const aloam::PointCloudPtr& full_res,
                                         const Eigen::Quaterniond& q_init,
                                         const Eigen::Vector3d& t_init) {
  MULLSMappingResult result;
  result.q_w_curr = q_init;
  result.t_w_curr = t_init;
  result.num_frames_in_map = frame_history_.size();

  if (corner_last == nullptr || surf_last == nullptr || full_res == nullptr) {
    return result;
  }

  q_w_curr_ = q_init.normalized();
  t_w_curr_ = t_init;

  if (!initialized_ || corner_map_->empty() || surf_map_->empty()) {
    FrameClouds frame = makeFrameClouds(corner_last, surf_last, full_res);
    for (auto& point : frame.corner->points) {
      point = transformPoint(point, q_w_curr_, t_w_curr_);
    }
    for (auto& point : frame.surf->points) {
      point = transformPoint(point, q_w_curr_, t_w_curr_);
    }
    for (auto& point : frame.points->points) {
      point = transformPoint(point, q_w_curr_, t_w_curr_);
    }
    frame_history_.push_back(frame);
    initialized_ = true;
    while (static_cast<int>(frame_history_.size()) > params_.max_frames_in_map) {
      frame_history_.pop_front();
    }
    rebuildMaps();
    result.num_frames_in_map = frame_history_.size();
    return result;
  }

  pcl::KdTreeFLANN<aloam::PointT> kd_corner_map;
  pcl::KdTreeFLANN<aloam::PointT> kd_surf_map;
  pcl::KdTreeFLANN<aloam::PointT> kd_point_map;
  kd_corner_map.setInputCloud(corner_map_);
  kd_surf_map.setInputCloud(surf_map_);
  kd_point_map.setInputCloud(point_map_);

  double parameters[7] = {q_w_curr_.x(), q_w_curr_.y(), q_w_curr_.z(),
                          q_w_curr_.w(), t_w_curr_.x(), t_w_curr_.y(),
                          t_w_curr_.z()};
  Eigen::Map<Eigen::Quaterniond> q_w_curr(parameters);
  Eigen::Map<Eigen::Vector3d> t_w_curr(parameters + 4);

  std::vector<int> point_search_indices;
  std::vector<float> point_search_distances;

  for (int iter = 0; iter < params_.num_optimization_iters; ++iter) {
    ceres::Problem problem;
    problem.AddParameterBlock(parameters, 4);
    localization_zoo::SetEigenQuaternionManifold(problem, parameters);
    problem.AddParameterBlock(parameters + 4, 3);

    ceres::LossFunction* edge_loss =
        new ceres::ScaledLoss(new ceres::HuberLoss(params_.huber_loss_s),
                              params_.line_weight, ceres::TAKE_OWNERSHIP);
    ceres::LossFunction* plane_loss =
        new ceres::ScaledLoss(new ceres::HuberLoss(params_.huber_loss_s),
                              params_.plane_weight, ceres::TAKE_OWNERSHIP);
    ceres::LossFunction* point_loss =
        new ceres::ScaledLoss(new ceres::HuberLoss(params_.huber_loss_s),
                              params_.point_weight, ceres::TAKE_OWNERSHIP);

    result.num_line_constraints = 0;
    result.num_plane_constraints = 0;
    result.num_point_constraints = 0;

    for (const auto& point_ori : corner_last->points) {
      const Eigen::Vector3d point_curr(point_ori.x, point_ori.y, point_ori.z);
      const Eigen::Vector3d point_w = q_w_curr * point_curr + t_w_curr;
      aloam::PointT point_sel;
      point_sel.x = point_w.x();
      point_sel.y = point_w.y();
      point_sel.z = point_w.z();

      kd_corner_map.nearestKSearch(point_sel, params_.knn, point_search_indices,
                                   point_search_distances);
      if (point_search_distances.size() < static_cast<size_t>(params_.knn) ||
          point_search_distances.back() > params_.knn_max_dist_sq) {
        continue;
      }

      Eigen::Vector3d center = Eigen::Vector3d::Zero();
      for (int j = 0; j < params_.knn; ++j) {
        center.x() += corner_map_->points[point_search_indices[j]].x;
        center.y() += corner_map_->points[point_search_indices[j]].y;
        center.z() += corner_map_->points[point_search_indices[j]].z;
      }
      center /= static_cast<double>(params_.knn);

      Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
      for (int j = 0; j < params_.knn; ++j) {
        Eigen::Vector3d diff;
        diff.x() = corner_map_->points[point_search_indices[j]].x - center.x();
        diff.y() = corner_map_->points[point_search_indices[j]].y - center.y();
        diff.z() = corner_map_->points[point_search_indices[j]].z - center.z();
        covariance += diff * diff.transpose();
      }
      covariance /= static_cast<double>(params_.knn);

      Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(covariance);
      const Eigen::Vector3d eigenvalues = solver.eigenvalues();
      if (eigenvalues(2) <= params_.edge_eigenvalue_ratio * eigenvalues(1)) {
        continue;
      }

      const Eigen::Vector3d direction = solver.eigenvectors().col(2);
      const Eigen::Vector3d point_a = center + 0.1 * direction;
      const Eigen::Vector3d point_b = center - 0.1 * direction;
      problem.AddResidualBlock(
          aloam::LidarEdgeFactor::Create(point_curr, point_a, point_b, 1.0),
          edge_loss, parameters, parameters + 4);
      ++result.num_line_constraints;
    }

    for (const auto& point_ori : surf_last->points) {
      const Eigen::Vector3d point_curr(point_ori.x, point_ori.y, point_ori.z);
      const Eigen::Vector3d point_w = q_w_curr * point_curr + t_w_curr;
      aloam::PointT point_sel;
      point_sel.x = point_w.x();
      point_sel.y = point_w.y();
      point_sel.z = point_w.z();

      kd_surf_map.nearestKSearch(point_sel, params_.knn, point_search_indices,
                                 point_search_distances);
      if (point_search_distances.size() < static_cast<size_t>(params_.knn) ||
          point_search_distances.back() > params_.knn_max_dist_sq) {
        continue;
      }

      Eigen::Matrix<double, 5, 3> matA0;
      Eigen::Matrix<double, 5, 1> matB0;
      matB0.fill(-1.0);
      for (int j = 0; j < params_.knn; ++j) {
        matA0(j, 0) = surf_map_->points[point_search_indices[j]].x;
        matA0(j, 1) = surf_map_->points[point_search_indices[j]].y;
        matA0(j, 2) = surf_map_->points[point_search_indices[j]].z;
      }

      Eigen::Vector3d normal = matA0.colPivHouseholderQr().solve(matB0);
      double negative_oa_dot_norm = 1.0 / normal.norm();
      normal.normalize();

      bool plane_valid = true;
      for (int j = 0; j < params_.knn; ++j) {
        const auto& point = surf_map_->points[point_search_indices[j]];
        const double distance =
            std::abs(normal.x() * point.x + normal.y() * point.y +
                     normal.z() * point.z + negative_oa_dot_norm);
        if (distance > params_.plane_threshold) {
          plane_valid = false;
          break;
        }
      }
      if (!plane_valid) {
        continue;
      }

      problem.AddResidualBlock(
          aloam::LidarPlaneNormFactor::Create(point_curr, normal,
                                              negative_oa_dot_norm),
          plane_loss, parameters, parameters + 4);
      ++result.num_plane_constraints;
    }

    for (size_t i = 0; i < full_res->size();
         i += std::max(1, params_.full_downsample_rate)) {
      const auto& point_ori = full_res->points[i];
      const Eigen::Vector3d point_curr(point_ori.x, point_ori.y, point_ori.z);
      const Eigen::Vector3d point_w = q_w_curr * point_curr + t_w_curr;
      aloam::PointT point_sel;
      point_sel.x = point_w.x();
      point_sel.y = point_w.y();
      point_sel.z = point_w.z();

      kd_point_map.nearestKSearch(point_sel, 1, point_search_indices,
                                  point_search_distances);
      if (point_search_distances.empty() ||
          point_search_distances.front() > params_.point_neighbor_max_dist_sq) {
        continue;
      }

      const auto& map_point = point_map_->points[point_search_indices.front()];
      problem.AddResidualBlock(
          LidarPointFactor::Create(
              point_curr,
              Eigen::Vector3d(map_point.x, map_point.y, map_point.z)),
          point_loss, parameters, parameters + 4);
      ++result.num_point_constraints;
    }

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.max_num_iterations = params_.ceres_max_iterations;
    options.minimizer_progress_to_stdout = false;

    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
  }

  q_w_curr_ = q_w_curr.normalized();
  t_w_curr_ = t_w_curr;

  FrameClouds frame = makeFrameClouds(corner_last, surf_last, full_res);
  for (auto& point : frame.corner->points) {
    point = transformPoint(point, q_w_curr_, t_w_curr_);
  }
  for (auto& point : frame.surf->points) {
    point = transformPoint(point, q_w_curr_, t_w_curr_);
  }
  for (auto& point : frame.points->points) {
    point = transformPoint(point, q_w_curr_, t_w_curr_);
  }
  frame_history_.push_back(frame);
  while (static_cast<int>(frame_history_.size()) > params_.max_frames_in_map) {
    frame_history_.pop_front();
  }
  rebuildMaps();

  result.q_w_curr = q_w_curr_;
  result.t_w_curr = t_w_curr_;
  result.num_frames_in_map = frame_history_.size();
  result.valid = result.num_line_constraints + result.num_plane_constraints +
                     result.num_point_constraints >
                 0;
  return result;
}

void MULLSMapping::clear() {
  frame_history_.clear();
  corner_map_->clear();
  surf_map_->clear();
  point_map_->clear();
  q_w_curr_ = Eigen::Quaterniond::Identity();
  t_w_curr_ = Eigen::Vector3d::Zero();
  initialized_ = false;
}

MULLS::MULLS(const MULLSParams& params)
    : scan_registration_(params.scan_registration),
      odometry_(params.odometry),
      mapping_(params.mapping) {}

MULLSResult MULLS::process(const aloam::PointCloudConstPtr& cloud) {
  MULLSResult result;

  const aloam::FeatureCloud features = scan_registration_.extract(cloud);
  result.num_corner_features = features.corner_less_sharp->size();
  result.num_surface_features = features.surf_less_flat->size();

  const auto odometry_result = odometry_.process(features);
  if (!odometry_result.valid) {
    return result;
  }

  const auto mapping_result = mapping_.process(features.corner_less_sharp,
                                               features.surf_less_flat,
                                               features.full_cloud,
                                               odometry_result.q_w_curr,
                                               odometry_result.t_w_curr);
  result.q_w_curr = mapping_result.q_w_curr;
  result.t_w_curr = mapping_result.t_w_curr;
  result.num_line_constraints = mapping_result.num_line_constraints;
  result.num_plane_constraints = mapping_result.num_plane_constraints;
  result.num_point_constraints = mapping_result.num_point_constraints;
  result.num_frames_in_map = mapping_result.num_frames_in_map;
  result.valid = mapping_result.valid;
  return result;
}

}  // namespace mulls
}  // namespace localization_zoo
