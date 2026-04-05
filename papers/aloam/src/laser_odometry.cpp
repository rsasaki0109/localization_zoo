#include "aloam/laser_odometry.h"
#include "aloam/lidar_factor.h"

#include <ceres/ceres.h>

#include <vector>

namespace localization_zoo {
namespace aloam {

void LaserOdometry::transformToStart(const PointT& pi, PointT& po) const {
  double s = params_.enable_distortion
                 ? (pi.intensity - static_cast<int>(pi.intensity)) /
                       0.1  // scan_period
                 : 1.0;

  Eigen::Quaterniond q_point =
      Eigen::Quaterniond::Identity().slerp(s, q_last_curr_);
  Eigen::Vector3d t_point = s * t_last_curr_;
  Eigen::Vector3d point(pi.x, pi.y, pi.z);
  Eigen::Vector3d un_point = q_point * point + t_point;

  po.x = un_point.x();
  po.y = un_point.y();
  po.z = un_point.z();
  po.intensity = pi.intensity;
}

OdometryResult LaserOdometry::process(const FeatureCloud& features) {
  OdometryResult result;

  if (!initialized_) {
    // 最初のフレーム: 前フレームとして保存
    corner_last_ = features.corner_less_sharp;
    surf_last_ = features.surf_less_flat;
    kd_corner_last_.setInputCloud(corner_last_);
    kd_surf_last_.setInputCloud(surf_last_);
    initialized_ = true;
    result.valid = false;
    return result;
  }

  int corner_sharp_num = features.corner_sharp->size();
  int surf_flat_num = features.surf_flat->size();

  // Ceres最適化変数 (Eigen四元数: x,y,z,w)
  double para_q[4] = {q_last_curr_.x(), q_last_curr_.y(), q_last_curr_.z(),
                       q_last_curr_.w()};
  double para_t[3] = {t_last_curr_.x(), t_last_curr_.y(), t_last_curr_.z()};

  Eigen::Map<Eigen::Quaterniond> q_last_curr(para_q);
  Eigen::Map<Eigen::Vector3d> t_last_curr(para_t);

  for (int iter = 0; iter < params_.num_optimization_iters; iter++) {
    ceres::Problem problem;
    ceres::LossFunction* loss = new ceres::HuberLoss(params_.huber_loss_s);

    problem.AddParameterBlock(para_q, 4);
    problem.SetManifold(para_q, new ceres::EigenQuaternionManifold());
    problem.AddParameterBlock(para_t, 3);

    // エッジ対応
    std::vector<int> point_search_idx;
    std::vector<float> point_search_dist;

    for (int i = 0; i < corner_sharp_num; i++) {
      PointT point_sel;
      transformToStart(features.corner_sharp->points[i], point_sel);

      kd_corner_last_.nearestKSearch(point_sel, 1, point_search_idx,
                                     point_search_dist);

      if (point_search_dist[0] > params_.distance_sq_threshold) continue;

      int closest_idx = point_search_idx[0];
      int closest_scan_id =
          static_cast<int>(corner_last_->points[closest_idx].intensity);

      // 異なるスキャンラインから2番目の対応点を探索
      int min_idx2 = -1;
      double min_dist2 = params_.distance_sq_threshold;

      for (int j = closest_idx + 1;
           j < static_cast<int>(corner_last_->size()); j++) {
        int scan_id =
            static_cast<int>(corner_last_->points[j].intensity);
        if (scan_id <= closest_scan_id) continue;
        if (scan_id > closest_scan_id + params_.nearby_scan) break;

        double dx = corner_last_->points[j].x - point_sel.x;
        double dy = corner_last_->points[j].y - point_sel.y;
        double dz = corner_last_->points[j].z - point_sel.z;
        double dist = dx * dx + dy * dy + dz * dz;
        if (dist < min_dist2) {
          min_dist2 = dist;
          min_idx2 = j;
        }
      }
      for (int j = closest_idx - 1; j >= 0; j--) {
        int scan_id =
            static_cast<int>(corner_last_->points[j].intensity);
        if (scan_id >= closest_scan_id) continue;
        if (scan_id < closest_scan_id - params_.nearby_scan) break;

        double dx = corner_last_->points[j].x - point_sel.x;
        double dy = corner_last_->points[j].y - point_sel.y;
        double dz = corner_last_->points[j].z - point_sel.z;
        double dist = dx * dx + dy * dy + dz * dz;
        if (dist < min_dist2) {
          min_dist2 = dist;
          min_idx2 = j;
        }
      }

      if (min_idx2 >= 0) {
        Eigen::Vector3d curr_point(features.corner_sharp->points[i].x,
                                   features.corner_sharp->points[i].y,
                                   features.corner_sharp->points[i].z);
        Eigen::Vector3d last_point_a(corner_last_->points[closest_idx].x,
                                     corner_last_->points[closest_idx].y,
                                     corner_last_->points[closest_idx].z);
        Eigen::Vector3d last_point_b(corner_last_->points[min_idx2].x,
                                     corner_last_->points[min_idx2].y,
                                     corner_last_->points[min_idx2].z);
        problem.AddResidualBlock(
            LidarEdgeFactor::Create(curr_point, last_point_a, last_point_b,
                                   1.0),
            loss, para_q, para_t);
      }
    }

    // 平面対応
    for (int i = 0; i < surf_flat_num; i++) {
      PointT point_sel;
      transformToStart(features.surf_flat->points[i], point_sel);

      kd_surf_last_.nearestKSearch(point_sel, 1, point_search_idx,
                                   point_search_dist);

      if (point_search_dist[0] > params_.distance_sq_threshold) continue;

      int closest_idx = point_search_idx[0];
      int closest_scan_id =
          static_cast<int>(surf_last_->points[closest_idx].intensity);

      int min_idx2 = -1, min_idx3 = -1;
      double min_dist2 = params_.distance_sq_threshold;
      double min_dist3 = params_.distance_sq_threshold;

      // 同じスキャンラインから1点、異なるスキャンラインから1点
      for (int j = closest_idx + 1;
           j < static_cast<int>(surf_last_->size()); j++) {
        int scan_id =
            static_cast<int>(surf_last_->points[j].intensity);
        if (scan_id > closest_scan_id + params_.nearby_scan) break;

        double dx = surf_last_->points[j].x - point_sel.x;
        double dy = surf_last_->points[j].y - point_sel.y;
        double dz = surf_last_->points[j].z - point_sel.z;
        double dist = dx * dx + dy * dy + dz * dz;

        if (scan_id == closest_scan_id && dist < min_dist2) {
          min_dist2 = dist;
          min_idx2 = j;
        } else if (scan_id != closest_scan_id && dist < min_dist3) {
          min_dist3 = dist;
          min_idx3 = j;
        }
      }
      for (int j = closest_idx - 1; j >= 0; j--) {
        int scan_id =
            static_cast<int>(surf_last_->points[j].intensity);
        if (scan_id < closest_scan_id - params_.nearby_scan) break;

        double dx = surf_last_->points[j].x - point_sel.x;
        double dy = surf_last_->points[j].y - point_sel.y;
        double dz = surf_last_->points[j].z - point_sel.z;
        double dist = dx * dx + dy * dy + dz * dz;

        if (scan_id == closest_scan_id && dist < min_dist2) {
          min_dist2 = dist;
          min_idx2 = j;
        } else if (scan_id != closest_scan_id && dist < min_dist3) {
          min_dist3 = dist;
          min_idx3 = j;
        }
      }

      if (min_idx2 >= 0 && min_idx3 >= 0) {
        Eigen::Vector3d curr_point(features.surf_flat->points[i].x,
                                   features.surf_flat->points[i].y,
                                   features.surf_flat->points[i].z);
        Eigen::Vector3d last_point_j(surf_last_->points[closest_idx].x,
                                     surf_last_->points[closest_idx].y,
                                     surf_last_->points[closest_idx].z);
        Eigen::Vector3d last_point_l(surf_last_->points[min_idx2].x,
                                     surf_last_->points[min_idx2].y,
                                     surf_last_->points[min_idx2].z);
        Eigen::Vector3d last_point_m(surf_last_->points[min_idx3].x,
                                     surf_last_->points[min_idx3].y,
                                     surf_last_->points[min_idx3].z);
        problem.AddResidualBlock(
            LidarPlaneFactor::Create(curr_point, last_point_j, last_point_l,
                                    last_point_m, 1.0),
            loss, para_q, para_t);
      }
    }

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.max_num_iterations = params_.ceres_max_iterations;
    options.minimizer_progress_to_stdout = false;

    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
  }

  // ワールド座標系のポーズを更新
  q_last_curr_ = q_last_curr;
  t_last_curr_ = t_last_curr;

  t_w_curr_ = t_w_curr_ + q_w_curr_ * t_last_curr_;
  q_w_curr_ = q_w_curr_ * q_last_curr_;

  // 前フレームを更新
  corner_last_ = features.corner_less_sharp;
  surf_last_ = features.surf_less_flat;
  kd_corner_last_.setInputCloud(corner_last_);
  kd_surf_last_.setInputCloud(surf_last_);

  result.q_w_curr = q_w_curr_;
  result.t_w_curr = t_w_curr_;
  result.valid = true;
  frame_count_++;

  return result;
}

}  // namespace aloam
}  // namespace localization_zoo
