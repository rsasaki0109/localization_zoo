#include "i_loam/i_loam.h"
#include "i_loam/intensity_factors.h"

#include <ceres/ceres.h>
#include <common/ceres_compat.h>

#include <cmath>
#include <limits>
#include <vector>

namespace localization_zoo {
namespace i_loam {

using aloam::FeatureCloud;
using aloam::PointCloud;
using aloam::PointCloudConstPtr;
using aloam::PointCloudPtr;
using aloam::PointT;

ILoam::ILoam(const ILoamParams& params)
    : params_(params),
      scan_registration_(params.scan_registration),
      corner_last_(new PointCloud),
      surf_last_(new PointCloud) {}

double ILoam::intensityWeight(double delta_intensity, double sigma) {
  if (sigma <= 0.0) return 1.0;
  const double d = delta_intensity / sigma;
  return std::exp(-0.5 * d * d);
}

double ILoam::augmentedDistanceSq(double geom_dist_sq, double delta_intensity,
                                  double intensity_corr_weight) {
  return geom_dist_sq +
         intensity_corr_weight * delta_intensity * delta_intensity;
}

void ILoam::clear() {
  corner_last_.reset(new PointCloud);
  surf_last_.reset(new PointCloud);
  corner_last_refl_.clear();
  surf_last_refl_.clear();
  q_w_curr_ = Eigen::Quaterniond::Identity();
  t_w_curr_ = Eigen::Vector3d::Zero();
  q_last_curr_ = Eigen::Quaterniond::Identity();
  t_last_curr_ = Eigen::Vector3d::Zero();
  initialized_ = false;
  frame_count_ = 0;
}

PointCloudPtr ILoam::strideCloud(const PointCloudConstPtr& cloud) const {
  if (params_.input_point_stride <= 1) {
    return PointCloudPtr(new PointCloud(*cloud));
  }
  PointCloudPtr out(new PointCloud);
  out->reserve(cloud->size() / params_.input_point_stride + 1);
  for (size_t i = 0; i < cloud->size(); i += params_.input_point_stride) {
    out->push_back(cloud->points[i]);
  }
  return out;
}

std::vector<float> ILoam::lookupReflectance(
    const PointCloudConstPtr& features,
    const pcl::KdTreeFLANN<PointT>& kd_raw,
    const PointCloudConstPtr& raw) const {
  std::vector<float> refl(features->size(), 0.0f);
  if (features->empty() || raw->empty()) return refl;
  std::vector<int> idx(1);
  std::vector<float> dist(1);
  for (size_t i = 0; i < features->size(); ++i) {
    // 特徴点の intensity は scan_id にエンコードされているため幾何だけで照合。
    PointT q = features->points[i];
    if (kd_raw.nearestKSearch(q, 1, idx, dist) > 0) {
      refl[i] = raw->points[idx[0]].intensity;
    }
  }
  return refl;
}

void ILoam::transformToStart(const PointT& pi, const Eigen::Quaterniond& q,
                             const Eigen::Vector3d& t, PointT& po) const {
  double s = params_.enable_distortion
                 ? (pi.intensity - static_cast<int>(pi.intensity)) /
                       params_.scan_registration.scan_period
                 : 1.0;
  Eigen::Quaterniond q_point = Eigen::Quaterniond::Identity().slerp(s, q);
  Eigen::Vector3d t_point = s * t;
  Eigen::Vector3d point(pi.x, pi.y, pi.z);
  Eigen::Vector3d un_point = q_point * point + t_point;
  po.x = un_point.x();
  po.y = un_point.y();
  po.z = un_point.z();
  po.intensity = pi.intensity;
}

ILoamResult ILoam::process(const PointCloudConstPtr& cloud) {
  ILoamResult result;
  if (!cloud) return result;

  PointCloudPtr working = strideCloud(cloud);
  result.num_input_points = working->size();

  FeatureCloud features = scan_registration_.extract(working);
  result.num_corner_sharp = features.corner_sharp->size();
  result.num_surf_flat = features.surf_flat->size();

  // 生点群への反射強度参照木 (この working フレームで有効)
  pcl::KdTreeFLANN<PointT> kd_raw;
  if (!working->empty()) kd_raw.setInputCloud(working);

  std::vector<float> sharp_refl =
      lookupReflectance(features.corner_sharp, kd_raw, working);
  std::vector<float> flat_refl =
      lookupReflectance(features.surf_flat, kd_raw, working);
  std::vector<float> less_sharp_refl =
      lookupReflectance(features.corner_less_sharp, kd_raw, working);
  std::vector<float> less_flat_refl =
      lookupReflectance(features.surf_less_flat, kd_raw, working);

  auto store_last = [&]() {
    corner_last_ = features.corner_less_sharp;
    surf_last_ = features.surf_less_flat;
    corner_last_refl_ = less_sharp_refl;
    surf_last_refl_ = less_flat_refl;
    if (!corner_last_->empty()) kd_corner_last_.setInputCloud(corner_last_);
    if (!surf_last_->empty()) kd_surf_last_.setInputCloud(surf_last_);
  };

  if (!initialized_) {
    store_last();
    initialized_ = true;
    frame_count_++;
    result.q_w_curr = q_w_curr_;
    result.t_w_curr = t_w_curr_;
    result.frame_count = frame_count_;
    return result;
  }

  const int corner_sharp_num = static_cast<int>(features.corner_sharp->size());
  const int surf_flat_num = static_cast<int>(features.surf_flat->size());

  double para_q[4] = {q_last_curr_.x(), q_last_curr_.y(), q_last_curr_.z(),
                      q_last_curr_.w()};
  double para_t[3] = {t_last_curr_.x(), t_last_curr_.y(), t_last_curr_.z()};
  Eigen::Map<Eigen::Quaterniond> q_lc(para_q);
  Eigen::Map<Eigen::Vector3d> t_lc(para_t);

  double weight_sum = 0.0;
  size_t weight_count = 0;
  size_t edge_corr = 0, plane_corr = 0;

  for (int iter = 0; iter < params_.num_optimization_iters; ++iter) {
    ceres::Problem problem;
    ceres::LossFunction* loss = new ceres::HuberLoss(params_.huber_loss_s);
    problem.AddParameterBlock(para_q, 4);
    localization_zoo::SetEigenQuaternionManifold(problem, para_q);
    problem.AddParameterBlock(para_t, 3);

    std::vector<int> search_idx;
    std::vector<float> search_dist;

    // --- エッジ対応 ---
    for (int i = 0; i < corner_sharp_num; ++i) {
      PointT psel;
      transformToStart(features.corner_sharp->points[i], q_lc, t_lc, psel);
      if (kd_corner_last_.nearestKSearch(psel, 1, search_idx, search_dist) < 1)
        continue;
      if (search_dist[0] > params_.distance_sq_threshold) continue;

      const int closest_idx = search_idx[0];
      const int closest_scan =
          static_cast<int>(corner_last_->points[closest_idx].intensity);
      const double i_curr = sharp_refl[i];

      int min_idx2 = -1;
      double min_cost2 = std::numeric_limits<double>::max();
      auto consider = [&](int j) {
        const double dx = corner_last_->points[j].x - psel.x;
        const double dy = corner_last_->points[j].y - psel.y;
        const double dz = corner_last_->points[j].z - psel.z;
        const double gd = dx * dx + dy * dy + dz * dz;
        if (gd >= params_.distance_sq_threshold) return;
        double cost = gd;
        if (params_.use_intensity_correspondence) {
          cost = augmentedDistanceSq(gd, i_curr - corner_last_refl_[j],
                                     params_.intensity_corr_weight);
        }
        if (cost < min_cost2) {
          min_cost2 = cost;
          min_idx2 = j;
        }
      };
      for (int j = closest_idx + 1;
           j < static_cast<int>(corner_last_->size()); ++j) {
        const int sid = static_cast<int>(corner_last_->points[j].intensity);
        if (sid <= closest_scan) continue;
        if (sid > closest_scan + params_.nearby_scan) break;
        consider(j);
      }
      for (int j = closest_idx - 1; j >= 0; --j) {
        const int sid = static_cast<int>(corner_last_->points[j].intensity);
        if (sid >= closest_scan) continue;
        if (sid < closest_scan - params_.nearby_scan) break;
        consider(j);
      }

      if (min_idx2 >= 0) {
        Eigen::Vector3d cp(features.corner_sharp->points[i].x,
                           features.corner_sharp->points[i].y,
                           features.corner_sharp->points[i].z);
        Eigen::Vector3d lpa(corner_last_->points[closest_idx].x,
                            corner_last_->points[closest_idx].y,
                            corner_last_->points[closest_idx].z);
        Eigen::Vector3d lpb(corner_last_->points[min_idx2].x,
                            corner_last_->points[min_idx2].y,
                            corner_last_->points[min_idx2].z);
        double w = params_.use_intensity_weight
                       ? intensityWeight(i_curr - corner_last_refl_[closest_idx],
                                         params_.intensity_sigma)
                       : 1.0;
        problem.AddResidualBlock(
            IntensityWeightedEdgeFactor::Create(cp, lpa, lpb, 1.0, w), loss,
            para_q, para_t);
        if (iter == 0) {
          weight_sum += w;
          ++weight_count;
          ++edge_corr;
        }
      }
    }

    // --- 平面対応 ---
    for (int i = 0; i < surf_flat_num; ++i) {
      PointT psel;
      transformToStart(features.surf_flat->points[i], q_lc, t_lc, psel);
      if (kd_surf_last_.nearestKSearch(psel, 1, search_idx, search_dist) < 1)
        continue;
      if (search_dist[0] > params_.distance_sq_threshold) continue;

      const int closest_idx = search_idx[0];
      const int closest_scan =
          static_cast<int>(surf_last_->points[closest_idx].intensity);
      const double i_curr = flat_refl[i];

      int min_idx2 = -1, min_idx3 = -1;
      double min_cost2 = std::numeric_limits<double>::max();
      double min_cost3 = std::numeric_limits<double>::max();
      auto consider = [&](int j) {
        const int sid = static_cast<int>(surf_last_->points[j].intensity);
        const double dx = surf_last_->points[j].x - psel.x;
        const double dy = surf_last_->points[j].y - psel.y;
        const double dz = surf_last_->points[j].z - psel.z;
        const double gd = dx * dx + dy * dy + dz * dz;
        if (gd >= params_.distance_sq_threshold) return;
        double cost = gd;
        if (params_.use_intensity_correspondence) {
          cost = augmentedDistanceSq(gd, i_curr - surf_last_refl_[j],
                                     params_.intensity_corr_weight);
        }
        if (sid == closest_scan) {
          if (cost < min_cost2) {
            min_cost2 = cost;
            min_idx2 = j;
          }
        } else {
          if (cost < min_cost3) {
            min_cost3 = cost;
            min_idx3 = j;
          }
        }
      };
      for (int j = closest_idx + 1;
           j < static_cast<int>(surf_last_->size()); ++j) {
        const int sid = static_cast<int>(surf_last_->points[j].intensity);
        if (sid > closest_scan + params_.nearby_scan) break;
        consider(j);
      }
      for (int j = closest_idx - 1; j >= 0; --j) {
        const int sid = static_cast<int>(surf_last_->points[j].intensity);
        if (sid < closest_scan - params_.nearby_scan) break;
        consider(j);
      }

      if (min_idx2 >= 0 && min_idx3 >= 0) {
        Eigen::Vector3d cp(features.surf_flat->points[i].x,
                           features.surf_flat->points[i].y,
                           features.surf_flat->points[i].z);
        Eigen::Vector3d lpj(surf_last_->points[closest_idx].x,
                            surf_last_->points[closest_idx].y,
                            surf_last_->points[closest_idx].z);
        Eigen::Vector3d lpl(surf_last_->points[min_idx2].x,
                            surf_last_->points[min_idx2].y,
                            surf_last_->points[min_idx2].z);
        Eigen::Vector3d lpm(surf_last_->points[min_idx3].x,
                            surf_last_->points[min_idx3].y,
                            surf_last_->points[min_idx3].z);
        double w = params_.use_intensity_weight
                       ? intensityWeight(i_curr - surf_last_refl_[closest_idx],
                                         params_.intensity_sigma)
                       : 1.0;
        problem.AddResidualBlock(
            IntensityWeightedPlaneFactor::Create(cp, lpj, lpl, lpm, 1.0, w),
            loss, para_q, para_t);
        if (iter == 0) {
          weight_sum += w;
          ++weight_count;
          ++plane_corr;
        }
      }
    }

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.max_num_iterations = params_.ceres_max_iterations;
    options.minimizer_progress_to_stdout = false;
    options.logging_type = ceres::SILENT;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
  }

  q_last_curr_ = q_lc;
  t_last_curr_ = t_lc;
  t_w_curr_ = t_w_curr_ + q_w_curr_ * t_last_curr_;
  q_w_curr_ = q_w_curr_ * q_last_curr_;

  store_last();

  result.q_w_curr = q_w_curr_;
  result.t_w_curr = t_w_curr_;
  result.valid = true;
  result.odom_valid = true;
  result.num_edge_correspondences = edge_corr;
  result.num_plane_correspondences = plane_corr;
  result.mean_intensity_weight =
      weight_count ? weight_sum / static_cast<double>(weight_count) : 1.0;
  frame_count_++;
  result.frame_count = frame_count_;
  return result;
}

}  // namespace i_loam
}  // namespace localization_zoo
