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

void mergeFrameMap(const VoxelHashMap& frame_map, VoxelHashMap* voxel_map) {
  for (const auto& [voxel, block] : frame_map) {
    auto& dst = (*voxel_map)[voxel];
    for (int i = 0; i < block.num_points; i++) {
      dst.addPoint(block.points[i]);
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
  for (auto it = frame_maps_.rbegin(); it != frame_maps_.rend(); ++it) {
    mergeFrameMap(*it, &voxel_map_);
  }
}

void CTICPRegistration::addPointsToMap(
    const std::vector<Eigen::Vector3d>& world_points) {
  if (world_points.empty()) return;

  if (params_.max_frames_in_map <= 0) {
    mergeFrameMap(buildFrameMap(world_points), &voxel_map_);
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
    const TrajectoryFrame& frame) const {
  std::vector<Correspondence> corrs;

  for (size_t i = 0; i < keypoints.size(); i++) {
    // 補間ポーズで変換
    Eigen::Vector3d world_point =
        frame.transformPoint(keypoints[i].raw_point, keypoints[i].timestamp);

    Voxel center = pointToVoxel(world_point, params_.voxel_resolution);

    // 周辺27ボクセルから近傍点を収集
    std::vector<Eigen::Vector3d> neighbors;
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
          Voxel v{center.x + dx, center.y + dy, center.z + dz};
          auto it = voxel_map_.find(v);
          if (it != voxel_map_.end()) {
            for (int k = 0; k < it->second.num_points; k++) {
              neighbors.push_back(it->second.points[k]);
            }
          }
        }
      }
    }

    if (static_cast<int>(neighbors.size()) < params_.knn) continue;

    // k最近傍を選択
    std::vector<std::pair<double, int>> dists;
    for (size_t j = 0; j < neighbors.size(); j++) {
      double d = (neighbors[j] - world_point).squaredNorm();
      dists.emplace_back(d, j);
    }
    std::partial_sort(dists.begin(),
                      dists.begin() + std::min(params_.knn,
                                               static_cast<int>(dists.size())),
                      dists.end());

    if (dists[params_.knn - 1].first > params_.max_correspondence_dist)
      continue;

    // PCAで法線計算
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    int k = std::min(params_.knn, static_cast<int>(dists.size()));
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

    if (a2D < params_.planarity_threshold) continue;

    Eigen::Vector3d normal = solver.eigenvectors().col(0);  // 最小固有値の固有ベクトル

    // 法線の向きを統一 (ワールドポイント側を向く)
    if (normal.dot(world_point - mean) < 0) {
      normal = -normal;
    }

    // 重み = a2D
    Correspondence corr;
    corr.point_idx = i;
    corr.reference = mean;
    corr.normal = normal;
    corr.weight = a2D;
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
    auto corrs = findCorrespondences(keypoints, result.frame);
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

    ceres::LossFunction* loss =
        new ceres::CauchyLoss(params_.cauchy_loss_param);

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

    // 正則化
    if (previous_frame && params_.location_consistency_weight > 0) {
      double w = std::sqrt(corrs.size() * params_.location_consistency_weight);
      problem.AddResidualBlock(
          LocationConsistency::Create(previous_frame->end_pose.trans, w),
          nullptr, begin_t);
    }

    if (previous_frame && params_.orientation_consistency_weight > 0) {
      double w = std::sqrt(corrs.size() * params_.orientation_consistency_weight);
      problem.AddResidualBlock(
          OrientationConsistency::Create(previous_frame->end_pose.quat, w),
          nullptr, begin_q);
    }

    if (previous_frame && params_.constant_velocity_weight > 0) {
      Eigen::Vector3d prev_velocity =
          previous_frame->end_pose.trans - previous_frame->begin_pose.trans;
      double w = std::sqrt(corrs.size() * params_.constant_velocity_weight);
      problem.AddResidualBlock(
          ConstantVelocity::Create(prev_velocity, w), nullptr, begin_t,
          end_t);
    }

    // 最適化
    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
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
