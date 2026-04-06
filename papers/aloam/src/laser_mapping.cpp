#include "aloam/laser_mapping.h"
#include "aloam/lidar_factor.h"

#include <ceres/ceres.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <Eigen/Eigenvalues>

namespace localization_zoo {
namespace aloam {

LaserMapping::LaserMapping(const LaserMappingParams& params)
    : params_(params),
      corner_from_map_(new PointCloud),
      surf_from_map_(new PointCloud) {
  center_cube_i_ = params_.cube_width / 2;
  center_cube_j_ = params_.cube_height / 2;
  center_cube_k_ = params_.cube_depth / 2;

  for (int i = 0; i < kMaxCubes; i++) {
    corner_cube_array_[i].reset(new PointCloud);
    surf_cube_array_[i].reset(new PointCloud);
  }
}

int LaserMapping::toIndex(int i, int j, int k) const {
  return i + params_.cube_width * j +
         params_.cube_width * params_.cube_height * k;
}

void LaserMapping::shiftCubeGrid() {
  // 現在位置のキューブ座標
  int ci = static_cast<int>(
      std::round(t_w_curr_.x() / params_.cube_size) + center_cube_i_);
  int cj = static_cast<int>(
      std::round(t_w_curr_.y() / params_.cube_size) + center_cube_j_);
  int ck = static_cast<int>(
      std::round(t_w_curr_.z() / params_.cube_size) + center_cube_k_);

  // 端に近い場合シフト
  while (ci < 3) {
    for (int j = 0; j < params_.cube_height; j++) {
      for (int k = 0; k < params_.cube_depth; k++) {
        for (int i = params_.cube_width - 1; i >= 1; i--) {
          corner_cube_array_[toIndex(i, j, k)] =
              corner_cube_array_[toIndex(i - 1, j, k)];
          surf_cube_array_[toIndex(i, j, k)] =
              surf_cube_array_[toIndex(i - 1, j, k)];
        }
        corner_cube_array_[toIndex(0, j, k)].reset(new PointCloud);
        surf_cube_array_[toIndex(0, j, k)].reset(new PointCloud);
      }
    }
    ci++;
    center_cube_i_++;
  }
  while (ci >= params_.cube_width - 3) {
    for (int j = 0; j < params_.cube_height; j++) {
      for (int k = 0; k < params_.cube_depth; k++) {
        for (int i = 0; i < params_.cube_width - 1; i++) {
          corner_cube_array_[toIndex(i, j, k)] =
              corner_cube_array_[toIndex(i + 1, j, k)];
          surf_cube_array_[toIndex(i, j, k)] =
              surf_cube_array_[toIndex(i + 1, j, k)];
        }
        corner_cube_array_[toIndex(params_.cube_width - 1, j, k)].reset(
            new PointCloud);
        surf_cube_array_[toIndex(params_.cube_width - 1, j, k)].reset(
            new PointCloud);
      }
    }
    ci--;
    center_cube_i_--;
  }

  while (cj < 3) {
    for (int i = 0; i < params_.cube_width; i++) {
      for (int k = 0; k < params_.cube_depth; k++) {
        for (int j = params_.cube_height - 1; j >= 1; j--) {
          corner_cube_array_[toIndex(i, j, k)] =
              corner_cube_array_[toIndex(i, j - 1, k)];
          surf_cube_array_[toIndex(i, j, k)] =
              surf_cube_array_[toIndex(i, j - 1, k)];
        }
        corner_cube_array_[toIndex(i, 0, k)].reset(new PointCloud);
        surf_cube_array_[toIndex(i, 0, k)].reset(new PointCloud);
      }
    }
    cj++;
    center_cube_j_++;
  }
  while (cj >= params_.cube_height - 3) {
    for (int i = 0; i < params_.cube_width; i++) {
      for (int k = 0; k < params_.cube_depth; k++) {
        for (int j = 0; j < params_.cube_height - 1; j++) {
          corner_cube_array_[toIndex(i, j, k)] =
              corner_cube_array_[toIndex(i, j + 1, k)];
          surf_cube_array_[toIndex(i, j, k)] =
              surf_cube_array_[toIndex(i, j + 1, k)];
        }
        corner_cube_array_[toIndex(i, params_.cube_height - 1, k)].reset(
            new PointCloud);
        surf_cube_array_[toIndex(i, params_.cube_height - 1, k)].reset(
            new PointCloud);
      }
    }
    cj--;
    center_cube_j_--;
  }

  while (ck < 3) {
    for (int i = 0; i < params_.cube_width; i++) {
      for (int j = 0; j < params_.cube_height; j++) {
        for (int k = params_.cube_depth - 1; k >= 1; k--) {
          corner_cube_array_[toIndex(i, j, k)] =
              corner_cube_array_[toIndex(i, j, k - 1)];
          surf_cube_array_[toIndex(i, j, k)] =
              surf_cube_array_[toIndex(i, j, k - 1)];
        }
        corner_cube_array_[toIndex(i, j, 0)].reset(new PointCloud);
        surf_cube_array_[toIndex(i, j, 0)].reset(new PointCloud);
      }
    }
    ck++;
    center_cube_k_++;
  }
  while (ck >= params_.cube_depth - 3) {
    for (int i = 0; i < params_.cube_width; i++) {
      for (int j = 0; j < params_.cube_height; j++) {
        for (int k = 0; k < params_.cube_depth - 1; k++) {
          corner_cube_array_[toIndex(i, j, k)] =
              corner_cube_array_[toIndex(i, j, k + 1)];
          surf_cube_array_[toIndex(i, j, k)] =
              surf_cube_array_[toIndex(i, j, k + 1)];
        }
        corner_cube_array_[toIndex(i, j, params_.cube_depth - 1)].reset(
            new PointCloud);
        surf_cube_array_[toIndex(i, j, params_.cube_depth - 1)].reset(
            new PointCloud);
      }
    }
    ck--;
    center_cube_k_--;
  }
}

void LaserMapping::extractSurroundingKeyFrames() {
  corner_from_map_->clear();
  surf_from_map_->clear();
  surrounding_cube_ids_.clear();

  int ci = static_cast<int>(
      std::round(t_w_curr_.x() / params_.cube_size) + center_cube_i_);
  int cj = static_cast<int>(
      std::round(t_w_curr_.y() / params_.cube_size) + center_cube_j_);
  int ck = static_cast<int>(
      std::round(t_w_curr_.z() / params_.cube_size) + center_cube_k_);

  // 5x5x3 周辺キューブ
  for (int i = ci - 2; i <= ci + 2; i++) {
    for (int j = cj - 2; j <= cj + 2; j++) {
      for (int k = ck - 1; k <= ck + 1; k++) {
        if (i >= 0 && i < params_.cube_width && j >= 0 &&
            j < params_.cube_height && k >= 0 && k < params_.cube_depth) {
          int idx = toIndex(i, j, k);
          surrounding_cube_ids_.push_back(idx);
          *corner_from_map_ += *corner_cube_array_[idx];
          *surf_from_map_ += *surf_cube_array_[idx];
        }
      }
    }
  }
}

void LaserMapping::optimizeTransform(const PointCloudPtr& corner_from_map,
                                     const PointCloudPtr& surf_from_map,
                                     const PointCloudPtr& corner_last,
                                     const PointCloudPtr& surf_last) {
  if (corner_from_map->empty() && surf_from_map->empty()) return;

  pcl::KdTreeFLANN<PointT> kd_corner_map;
  pcl::KdTreeFLANN<PointT> kd_surf_map;

  if (!corner_from_map->empty()) kd_corner_map.setInputCloud(corner_from_map);
  if (!surf_from_map->empty()) kd_surf_map.setInputCloud(surf_from_map);

  // Ceres最適化変数 (Eigen四元数メモリレイアウト: x,y,z,w)
  double parameters[7] = {q_w_curr_.x(), q_w_curr_.y(), q_w_curr_.z(),
                           q_w_curr_.w(), t_w_curr_.x(), t_w_curr_.y(),
                           t_w_curr_.z()};

  Eigen::Map<Eigen::Quaterniond> q_w_curr(parameters);
  Eigen::Map<Eigen::Vector3d> t_w_curr(parameters + 4);

  for (int iter = 0; iter < params_.num_optimization_iters; iter++) {
    ceres::Problem problem;
    ceres::LossFunction* loss = new ceres::HuberLoss(params_.huber_loss_s);

    problem.AddParameterBlock(parameters, 4);
    problem.SetManifold(parameters, new ceres::EigenQuaternionManifold());
    problem.AddParameterBlock(parameters + 4, 3);

    std::vector<int> point_search_idx;
    std::vector<float> point_search_dist;

    // エッジ対応
    int corner_num = corner_last->size();
    for (int i = 0; i < corner_num; i++) {
      PointT point_ori = corner_last->points[i];
      // ワールド座標に変換
      Eigen::Vector3d point_curr(point_ori.x, point_ori.y, point_ori.z);
      Eigen::Vector3d point_w = q_w_curr * point_curr + t_w_curr;
      PointT point_sel;
      point_sel.x = point_w.x();
      point_sel.y = point_w.y();
      point_sel.z = point_w.z();

      if (corner_from_map->empty()) continue;
      kd_corner_map.nearestKSearch(point_sel, params_.knn, point_search_idx,
                                   point_search_dist);

      if (point_search_dist.back() > params_.knn_max_dist_sq) continue;

      // 5点の共分散行列
      Eigen::Vector3d center(0, 0, 0);
      for (int j = 0; j < params_.knn; j++) {
        center.x() += corner_from_map->points[point_search_idx[j]].x;
        center.y() += corner_from_map->points[point_search_idx[j]].y;
        center.z() += corner_from_map->points[point_search_idx[j]].z;
      }
      center /= params_.knn;

      Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
      for (int j = 0; j < params_.knn; j++) {
        Eigen::Vector3d diff;
        diff.x() = corner_from_map->points[point_search_idx[j]].x - center.x();
        diff.y() = corner_from_map->points[point_search_idx[j]].y - center.y();
        diff.z() = corner_from_map->points[point_search_idx[j]].z - center.z();
        cov += diff * diff.transpose();
      }
      cov /= params_.knn;

      Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
      Eigen::Vector3d eigenvalues = solver.eigenvalues();

      // 最大固有値 >> 他 → 直線特徴
      if (eigenvalues(2) > params_.edge_eigenvalue_ratio * eigenvalues(1)) {
        Eigen::Vector3d direction = solver.eigenvectors().col(2);
        Eigen::Vector3d point_a = center + 0.1 * direction;
        Eigen::Vector3d point_b = center - 0.1 * direction;

        problem.AddResidualBlock(
            LidarEdgeFactor::Create(point_curr, point_a, point_b, 1.0), loss,
            parameters, parameters + 4);
      }
    }

    // 平面対応
    int surf_num = surf_last->size();
    for (int i = 0; i < surf_num; i++) {
      PointT point_ori = surf_last->points[i];
      Eigen::Vector3d point_curr(point_ori.x, point_ori.y, point_ori.z);
      Eigen::Vector3d point_w = q_w_curr * point_curr + t_w_curr;
      PointT point_sel;
      point_sel.x = point_w.x();
      point_sel.y = point_w.y();
      point_sel.z = point_w.z();

      if (surf_from_map->empty()) continue;
      kd_surf_map.nearestKSearch(point_sel, params_.knn, point_search_idx,
                                 point_search_dist);

      if (point_search_dist.back() > params_.knn_max_dist_sq) continue;

      // 平面方程式 ax + by + cz + 1 = 0 をフィット
      Eigen::Matrix<double, 5, 3> matA0;
      Eigen::Matrix<double, 5, 1> matB0;
      matB0.fill(-1.0);

      for (int j = 0; j < params_.knn; j++) {
        matA0(j, 0) = surf_from_map->points[point_search_idx[j]].x;
        matA0(j, 1) = surf_from_map->points[point_search_idx[j]].y;
        matA0(j, 2) = surf_from_map->points[point_search_idx[j]].z;
      }

      Eigen::Vector3d norm =
          matA0.colPivHouseholderQr().solve(matB0);
      double negative_oa_dot_norm = 1.0 / norm.norm();
      norm.normalize();

      // 全5点が平面に近いかチェック
      bool plane_valid = true;
      for (int j = 0; j < params_.knn; j++) {
        double d = std::abs(
            norm.x() * surf_from_map->points[point_search_idx[j]].x +
            norm.y() * surf_from_map->points[point_search_idx[j]].y +
            norm.z() * surf_from_map->points[point_search_idx[j]].z +
            negative_oa_dot_norm);
        if (d > params_.plane_threshold) {
          plane_valid = false;
          break;
        }
      }

      if (plane_valid) {
        problem.AddResidualBlock(
            LidarPlaneNormFactor::Create(point_curr, norm,
                                         negative_oa_dot_norm),
            loss, parameters, parameters + 4);
      }
    }

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.max_num_iterations = params_.ceres_max_iterations;
    options.minimizer_progress_to_stdout = false;

    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
  }

  q_w_curr_ = q_w_curr;
  t_w_curr_ = t_w_curr;
}

void LaserMapping::updateMap(const PointCloudPtr& corner_last,
                             const PointCloudPtr& surf_last) {
  // 特徴点をワールド座標に変換してキューブに追加
  auto transformToMap = [&](const PointT& pi) {
    Eigen::Vector3d p(pi.x, pi.y, pi.z);
    Eigen::Vector3d pw = q_w_curr_ * p + t_w_curr_;
    PointT po;
    po.x = pw.x();
    po.y = pw.y();
    po.z = pw.z();
    po.intensity = pi.intensity;
    return po;
  };

  for (const auto& p : corner_last->points) {
    PointT pw = transformToMap(p);
    int ci =
        static_cast<int>(std::round(pw.x / params_.cube_size)) + center_cube_i_;
    int cj =
        static_cast<int>(std::round(pw.y / params_.cube_size)) + center_cube_j_;
    int ck =
        static_cast<int>(std::round(pw.z / params_.cube_size)) + center_cube_k_;
    if (ci >= 0 && ci < params_.cube_width && cj >= 0 &&
        cj < params_.cube_height && ck >= 0 && ck < params_.cube_depth) {
      corner_cube_array_[toIndex(ci, cj, ck)]->push_back(pw);
    }
  }

  for (const auto& p : surf_last->points) {
    PointT pw = transformToMap(p);
    int ci =
        static_cast<int>(std::round(pw.x / params_.cube_size)) + center_cube_i_;
    int cj =
        static_cast<int>(std::round(pw.y / params_.cube_size)) + center_cube_j_;
    int ck =
        static_cast<int>(std::round(pw.z / params_.cube_size)) + center_cube_k_;
    if (ci >= 0 && ci < params_.cube_width && cj >= 0 &&
        cj < params_.cube_height && ck >= 0 && ck < params_.cube_depth) {
      surf_cube_array_[toIndex(ci, cj, ck)]->push_back(pw);
    }
  }

  // 周辺キューブをダウンサンプリング
  pcl::VoxelGrid<PointT> downsize_corner;
  downsize_corner.setLeafSize(params_.line_resolution, params_.line_resolution,
                              params_.line_resolution);
  pcl::VoxelGrid<PointT> downsize_surf;
  downsize_surf.setLeafSize(params_.plane_resolution, params_.plane_resolution,
                            params_.plane_resolution);

  for (int idx : surrounding_cube_ids_) {
    if (!corner_cube_array_[idx]->empty()) {
      PointCloudPtr tmp(new PointCloud);
      downsize_corner.setInputCloud(corner_cube_array_[idx]);
      downsize_corner.filter(*tmp);
      corner_cube_array_[idx] = tmp;
    }
    if (!surf_cube_array_[idx]->empty()) {
      PointCloudPtr tmp(new PointCloud);
      downsize_surf.setInputCloud(surf_cube_array_[idx]);
      downsize_surf.filter(*tmp);
      surf_cube_array_[idx] = tmp;
    }
  }
}

MappingResult LaserMapping::process(const PointCloudPtr& corner_last,
                                    const PointCloudPtr& surf_last,
                                    const PointCloudPtr& full_res,
                                    const Eigen::Quaterniond& q_odom,
                                    const Eigen::Vector3d& t_odom) {
  // Odom座標 → Map座標に変換
  q_w_curr_ = q_wmap_wodom_ * q_odom;
  t_w_curr_ = q_wmap_wodom_ * t_odom + t_wmap_wodom_;

  // キューブグリッドをシフト
  shiftCubeGrid();

  // 周辺キューブを抽出
  extractSurroundingKeyFrames();

  // 最適化
  optimizeTransform(corner_from_map_, surf_from_map_, corner_last, surf_last);

  // Odom→Map 補正を更新
  q_wmap_wodom_ = q_w_curr_ * q_odom.inverse();
  t_wmap_wodom_ = t_w_curr_ - q_wmap_wodom_ * t_odom;

  // マップに点を追加
  updateMap(corner_last, surf_last);

  MappingResult result;
  result.q_w_curr = q_w_curr_;
  result.t_w_curr = t_w_curr_;
  result.valid = true;
  return result;
}

}  // namespace aloam
}  // namespace localization_zoo
