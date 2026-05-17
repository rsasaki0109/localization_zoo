#include "ct_icp/ct_icp_registration.h"
#include "ct_icp/cost_functions.h"

#include <ceres/ceres.h>
#include <Eigen/Eigenvalues>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <common/ceres_compat.h>

namespace localization_zoo {
namespace ct_icp {

namespace {

Voxel pointToVoxel(const Eigen::Vector3d& point, double resolution) {
  return Voxel{static_cast<int>(std::floor(point.x() / resolution)),
               static_cast<int>(std::floor(point.y() / resolution)),
               static_cast<int>(std::floor(point.z() / resolution))};
}

void mergeFrameMap(const VoxelHashMap& frame_map, VoxelHashMap* voxel_map,
                   double min_distance_sq = 0.0) {
  for (const auto& [voxel, block] : frame_map) {
    auto& dst = (*voxel_map)[voxel];
    for (int i = 0; i < block.num_points; i++) {
      dst.addPoint(block.points[i], min_distance_sq);
    }
  }
}

}  // namespace

VoxelHashMap CTICPRegistration::buildFrameMap(
    const std::vector<Eigen::Vector3d>& world_points) const {
  VoxelHashMap frame_map;
  for (const auto& p : world_points) {
    Voxel voxel = pointToVoxel(p, params_.voxel_resolution);
    frame_map[voxel].addPoint(p);
  }
  return frame_map;
}

void CTICPRegistration::rebuildMapFromWindow() {
  voxel_map_.clear();
  const double min_dist_sq = params_.min_distance_between_points *
                             params_.min_distance_between_points;
  // 古いフレームから新しいフレーム順に再構築すると、min-distance チェックで
  // 新しい点が rejection されやすいので、新しいフレームから順に追加する。
  for (auto it = frame_maps_.rbegin(); it != frame_maps_.rend(); ++it) {
    mergeFrameMap(*it, &voxel_map_, min_dist_sq);
  }
}

void CTICPRegistration::addPointsToMap(
    const std::vector<Eigen::Vector3d>& world_points) {
  if (world_points.empty()) return;

  const double min_dist_sq = params_.min_distance_between_points *
                             params_.min_distance_between_points;
  if (params_.max_frames_in_map <= 0) {
    mergeFrameMap(buildFrameMap(world_points), &voxel_map_, min_dist_sq);
    return;
  }

  frame_maps_.push_back(buildFrameMap(world_points));
  while (static_cast<int>(frame_maps_.size()) > params_.max_frames_in_map) {
    frame_maps_.pop_front();
  }
  rebuildMapFromWindow();
}

void CTICPRegistration::clearMap() {
  voxel_map_.clear();
  frame_maps_.clear();
}

std::vector<TimedPoint> CTICPRegistration::subsampleKeypoints(
    const std::vector<TimedPoint>& points) const {
  std::unordered_map<Voxel, int, VoxelHash> voxel_to_idx;
  std::vector<TimedPoint> keypoints;

  for (size_t i = 0; i < points.size(); i++) {
    Voxel v = pointToVoxel(points[i].raw_point, params_.keypoint_voxel_size);
    if (voxel_to_idx.find(v) == voxel_to_idx.end()) {
      voxel_to_idx[v] = keypoints.size();
      keypoints.push_back(points[i]);
    }
  }
  return keypoints;
}

std::vector<CTICPRegistration::Correspondence>
CTICPRegistration::findCorrespondences(
    const std::vector<TimedPoint>& keypoints,
    const TrajectoryFrame& frame,
    int outer_iter) const {
  std::vector<Correspondence> corrs;

  // Coarse phase 判定: 最初の coarse_iterations 回の outer iteration では
  // 探索半径を coarse_search_radius まで拡げ、planarity を緩める。
  const bool coarse_phase =
      params_.coarse_to_fine && outer_iter < params_.coarse_iterations;
  const int phase_search_radius =
      coarse_phase ? std::max(2, params_.coarse_search_radius) : 1;
  const double phase_planarity_threshold =
      coarse_phase ? std::min(params_.planarity_threshold,
                              params_.coarse_planarity_threshold)
                   : params_.planarity_threshold;

  for (size_t i = 0; i < keypoints.size(); i++) {
    // 補間ポーズで変換
    Eigen::Vector3d world_point =
        frame.transformPoint(keypoints[i].raw_point, keypoints[i].timestamp);

    Voxel center = pointToVoxel(world_point, params_.voxel_resolution);

    auto gather_neighborhood = [&](int radius, std::vector<Eigen::Vector3d>& out) {
      for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
          for (int dz = -radius; dz <= radius; dz++) {
            if (std::abs(dx) < radius && std::abs(dy) < radius &&
                std::abs(dz) < radius)
              continue;  // すでに小半径で収集済みのシェルを除外
            Voxel v{center.x + dx, center.y + dy, center.z + dz};
            auto it = voxel_map_.find(v);
            if (it != voxel_map_.end()) {
              for (int k = 0; k < it->second.num_points; k++) {
                out.push_back(it->second.points[k]);
              }
            }
          }
        }
      }
    };

    std::vector<Eigen::Vector3d> neighbors;
    // radius=0 (中心ボクセル) + radius=1 (3x3x3 シェル)。
    // coarse phase では phase_search_radius まで unconditionally に拡げる。
    gather_neighborhood(0, neighbors);
    gather_neighborhood(1, neighbors);
    if (coarse_phase) {
      for (int r = 2; r <= phase_search_radius; ++r) {
        gather_neighborhood(r, neighbors);
      }
    } else if (params_.multi_scale_correspondences &&
               static_cast<int>(neighbors.size()) < params_.knn) {
      // 27 ボクセルで knn 未満 → 5x5x5 (125 ボクセル) に拡張
      gather_neighborhood(2, neighbors);
    }

    if (static_cast<int>(neighbors.size()) < params_.knn) continue;

    // 距離ソート (PCA + reference 用)
    std::vector<std::pair<double, int>> dists;
    for (size_t j = 0; j < neighbors.size(); j++) {
      double d = (neighbors[j] - world_point).squaredNorm();
      dists.emplace_back(d, j);
    }
    const int pca_n = (params_.pca_neighbor_count > 0)
                          ? std::min(params_.pca_neighbor_count,
                                     static_cast<int>(dists.size()))
                          : std::min(params_.knn,
                                     static_cast<int>(dists.size()));
    const int sort_n = std::max(params_.knn, pca_n);
    std::partial_sort(dists.begin(),
                      dists.begin() + std::min(sort_n, static_cast<int>(dists.size())),
                      dists.end());

    if (dists[params_.knn - 1].first > params_.max_correspondence_dist)
      continue;

    // PCAで法線計算
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    int k = pca_n;
    for (int j = 0; j < k; j++) {
      mean += neighbors[dists[j].second];
    }
    mean /= k;

    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    for (int j = 0; j < k; j++) {
      Eigen::Vector3d diff = neighbors[dists[j].second] - mean;
      cov += diff * diff.transpose();
    }
    cov /= k;

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
    Eigen::Vector3d eigenvalues = solver.eigenvalues();

    // 平面性 a2D = (lambda2 - lambda1) / lambda3
    double a2D = 0.0;
    if (eigenvalues(2) > 1e-10) {
      a2D = (eigenvalues(1) - eigenvalues(0)) / eigenvalues(2);
    }

    if (a2D < phase_planarity_threshold) continue;
    if (a2D < params_.min_planarity_floor) continue;

    Eigen::Vector3d normal = solver.eigenvectors().col(0);  // 最小固有値の固有ベクトル

    // 法線の向きを統一 (ワールドポイント側を向く)
    if (normal.dot(world_point - mean) < 0) {
      normal = -normal;
    }

    // 重み: weight_alpha * a2D^power_planarity + weight_neighborhood * exp(-d_closest/d_max)
    // 既定 (power=1, alpha=1, neighborhood=0) では w = a2D。
    double a2D_p = (params_.power_planarity == 1.0)
                       ? a2D
                       : std::pow(a2D, params_.power_planarity);
    double w = params_.weight_alpha * a2D_p;
    if (params_.weight_neighborhood > 0.0) {
      double d_closest = std::sqrt(dists[0].first);
      double d_max = std::sqrt(params_.max_correspondence_dist);
      double w_nb = (d_max > 1e-9) ? std::exp(-d_closest / d_max) : 0.0;
      w += params_.weight_neighborhood * w_nb;
    }

    Correspondence corr;
    corr.point_idx = i;
    // paper: reference は最近傍点 (mean ではなく)。mean は plane 中心の近似で、
    // 遠方では数 m もずれた点を anchor にしてしまい point-to-plane の sign が
    // 不安定になる。closest-neighbor anchor は plane 上の単一点を使うため
    // local geometry に sharp に hooked できる。
    corr.reference = params_.use_closest_neighbor_reference
                         ? neighbors[dists[0].second]
                         : mean;
    corr.normal = normal;
    corr.weight = w;
    corrs.push_back(corr);
  }

  return corrs;
}

CTICPResult CTICPRegistration::registerFrame(
    const std::vector<TimedPoint>& timed_points,
    const TrajectoryFrame& initial_frame,
    const TrajectoryFrame* previous_frame) {
  CTICPResult result;
  result.frame = initial_frame;

  if (voxel_map_.empty()) {
    // 最初のフレーム: マップに追加するだけ
    result.converged = true;
    return result;
  }

  // キーポイントにダウンサンプリング
  auto keypoints = subsampleKeypoints(timed_points);
  if (keypoints.empty()) {
    std::cerr << "[CT-ICP] No keypoints after subsampling." << std::endl;
    return result;
  }

  // alpha を計算
  for (auto& kp : keypoints) {
    kp.alpha = result.frame.getAlpha(kp.timestamp);
  }

  // Ceresパラメータ: begin_quat(4), begin_trans(3), end_quat(4), end_trans(3)
  double begin_q[4] = {result.frame.begin_pose.quat.x(),
                        result.frame.begin_pose.quat.y(),
                        result.frame.begin_pose.quat.z(),
                        result.frame.begin_pose.quat.w()};
  double begin_t[3] = {result.frame.begin_pose.trans.x(),
                        result.frame.begin_pose.trans.y(),
                        result.frame.begin_pose.trans.z()};
  double end_q[4] = {result.frame.end_pose.quat.x(),
                      result.frame.end_pose.quat.y(),
                      result.frame.end_pose.quat.z(),
                      result.frame.end_pose.quat.w()};
  double end_t[3] = {result.frame.end_pose.trans.x(),
                      result.frame.end_pose.trans.y(),
                      result.frame.end_pose.trans.z()};

  Eigen::Map<Eigen::Quaterniond> q_begin(begin_q);
  Eigen::Map<Eigen::Vector3d> t_begin(begin_t);
  Eigen::Map<Eigen::Quaterniond> q_end(end_q);
  Eigen::Map<Eigen::Vector3d> t_end(end_t);

  for (int iter = 0; iter < params_.max_iterations; iter++) {
    // 現在のポーズを更新
    result.frame.begin_pose.quat = q_begin;
    result.frame.begin_pose.trans = t_begin;
    result.frame.end_pose.quat = q_end;
    result.frame.end_pose.trans = t_end;

    // 対応関係を更新
    auto corrs = findCorrespondences(keypoints, result.frame, iter);
    if (corrs.empty()) {
      std::cerr << "[CT-ICP] No correspondences at iteration " << iter
                << std::endl;
      break;
    }

    // Ceres問題を構築
    ceres::Problem problem;
    problem.AddParameterBlock(begin_q, 4);
    localization_zoo::SetEigenQuaternionManifold(problem, begin_q);
    problem.AddParameterBlock(begin_t, 3);
    problem.AddParameterBlock(end_q, 4);
    localization_zoo::SetEigenQuaternionManifold(problem, end_q);
    problem.AddParameterBlock(end_t, 3);

    // coarse phase は Cauchy σ を coarse_cauchy_sigma_mult 倍してロバスト化、
    // outlier 受容を緩める。fine phase は通常 σ。
    const bool ct_coarse_phase =
        params_.coarse_to_fine && iter < params_.coarse_iterations;
    double cauchy_sigma = params_.cauchy_loss_param;
    if (ct_coarse_phase && params_.coarse_cauchy_sigma_mult > 0.0) {
      cauchy_sigma *= params_.coarse_cauchy_sigma_mult;
    }
    ceres::LossFunction* loss = new ceres::CauchyLoss(cauchy_sigma);

    // 幾何残差
    double weight_scale = std::sqrt(corrs.size());
    for (const auto& corr : corrs) {
      double w = corr.weight;
      double alpha = keypoints[corr.point_idx].alpha;

      problem.AddResidualBlock(
          CTPointToPlane::Create(keypoints[corr.point_idx].raw_point,
                                corr.reference, corr.normal, alpha, w),
          loss, begin_q, begin_t, end_q, end_t);
    }

    // 正則化重み:
    //   flat_regularizer_weight=true → sqrt(β) (paper 一致)
    //   regularizer_n_cap>0          → sqrt(min(N_corr, cap) * β) (中間策)
    //   既定                          → sqrt(N_corr * β)
    auto reg_weight = [&](double beta) {
      if (params_.flat_regularizer_weight) return std::sqrt(beta);
      int n = static_cast<int>(corrs.size());
      if (params_.regularizer_n_cap > 0 && n > params_.regularizer_n_cap) {
        n = params_.regularizer_n_cap;
      }
      return std::sqrt(static_cast<double>(n) * beta);
    };
    if (previous_frame && params_.location_consistency_weight > 0) {
      double w = reg_weight(params_.location_consistency_weight);
      problem.AddResidualBlock(
          LocationConsistency::Create(previous_frame->end_pose.trans, w),
          nullptr, begin_t);
    }

    if (previous_frame && params_.orientation_consistency_weight > 0) {
      double w = reg_weight(params_.orientation_consistency_weight);
      problem.AddResidualBlock(
          OrientationConsistency::Create(previous_frame->end_pose.quat, w),
          nullptr, begin_q);
    }

    if (previous_frame && params_.constant_velocity_weight > 0) {
      Eigen::Vector3d prev_velocity =
          previous_frame->end_pose.trans - previous_frame->begin_pose.trans;
      double w = reg_weight(params_.constant_velocity_weight);
      problem.AddResidualBlock(
          ConstantVelocity::Create(prev_velocity, w), nullptr, begin_t,
          end_t);
    }

    // 最適化
    ceres::Solver::Options options;
    options.linear_solver_type = params_.use_normal_cholesky_solver
                                     ? ceres::DENSE_NORMAL_CHOLESKY
                                     : ceres::DENSE_QR;
    options.max_num_iterations = params_.ceres_max_iterations;
    options.trust_region_strategy_type = ceres::LEVENBERG_MARQUARDT;
    options.minimizer_progress_to_stdout = false;
    options.num_threads = 1;

    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    // 収束判定
    Eigen::Vector3d t_diff_begin =
        Eigen::Map<Eigen::Vector3d>(begin_t) - result.frame.begin_pose.trans;
    Eigen::Vector3d t_diff_end =
        Eigen::Map<Eigen::Vector3d>(end_t) - result.frame.end_pose.trans;

    result.num_iterations = iter + 1;

    if (t_diff_begin.norm() < params_.convergence_threshold &&
        t_diff_end.norm() < params_.convergence_threshold) {
      result.converged = true;
      break;
    }
  }

  // 最終ポーズを設定
  result.frame.begin_pose.quat = q_begin;
  result.frame.begin_pose.trans = t_begin;
  result.frame.end_pose.quat = q_end;
  result.frame.end_pose.trans = t_end;

  return result;
}

}  // namespace ct_icp
}  // namespace localization_zoo
