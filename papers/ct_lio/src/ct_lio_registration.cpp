#include "ct_lio/ct_lio_registration.h"

#include "ct_icp/cost_functions.h"
#include "ct_lio/cost_functions.h"

#include <ceres/ceres.h>
#include <Eigen/Eigenvalues>

#include <algorithm>
#include <array>
#include <cmath>
#include <unordered_map>

namespace localization_zoo {
namespace ct_lio {

namespace {

ct_icp::Voxel pointToVoxel(const Eigen::Vector3d& point, double resolution) {
  return ct_icp::Voxel{static_cast<int>(std::floor(point.x() / resolution)),
                       static_cast<int>(std::floor(point.y() / resolution)),
                       static_cast<int>(std::floor(point.z() / resolution))};
}

void mergeFrameMap(const ct_icp::VoxelHashMap& frame_map,
                   ct_icp::VoxelHashMap* voxel_map) {
  for (const auto& [voxel, block] : frame_map) {
    auto& dst = (*voxel_map)[voxel];
    for (int i = 0; i < block.num_points; ++i) {
      dst.addPoint(block.points[i]);
    }
  }
}

double biasRandomWalkWeight(double base_weight, double delta_t) {
  if (base_weight <= 0.0) {
    return 0.0;
  }
  if (delta_t <= 0.0) {
    return base_weight;
  }
  return base_weight / std::sqrt(std::max(delta_t, 1e-6));
}

}  // namespace

ct_icp::VoxelHashMap CTLIORegistration::buildFrameMap(
    const std::vector<Eigen::Vector3d>& world_points) const {
  ct_icp::VoxelHashMap frame_map;
  for (const auto& point : world_points) {
    frame_map[pointToVoxel(point, params_.voxel_resolution)].addPoint(point);
  }
  return frame_map;
}

void CTLIORegistration::rebuildMapFromWindow() {
  voxel_map_.clear();
  for (auto it = frame_maps_.rbegin(); it != frame_maps_.rend(); ++it) {
    mergeFrameMap(*it, &voxel_map_);
  }
}

void CTLIORegistration::addPointsToMap(
    const std::vector<Eigen::Vector3d>& world_points) {
  if (world_points.empty()) {
    return;
  }

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

void CTLIORegistration::clearMap() {
  voxel_map_.clear();
  frame_maps_.clear();
  state_history_.clear();
}

CTLIORegistration::LaggedStatePrior
CTLIORegistration::computeLaggedStatePrior() const {
  LaggedStatePrior prior;
  if (params_.fixed_lag_state_window <= 1 || state_history_.empty()) {
    return prior;
  }

  const int max_history =
      std::min<int>(params_.fixed_lag_state_window - 1, state_history_.size());
  const double history_decay =
      std::clamp(params_.fixed_lag_history_decay, 0.0, 1.0);
  double total_weight = 0.0;
  for (int i = 0; i < max_history; ++i) {
    const double linear_weight = static_cast<double>(max_history - i);
    const double weight = linear_weight * std::pow(history_decay, i);
    const auto& state = state_history_[state_history_.size() - 1 - i].state;
    prior.velocity += weight * state.end_velocity;
    prior.gyro_bias += weight * state.gyro_bias;
    prior.accel_bias += weight * state.accel_bias;
    total_weight += weight;
  }

  if (total_weight <= 0.0) {
    return prior;
  }

  prior.valid = true;
  prior.velocity /= total_weight;
  prior.gyro_bias /= total_weight;
  prior.accel_bias /= total_weight;
  return prior;
}

void CTLIORegistration::rememberState(
    const CTLIOState& state,
    const imu_preintegration::PreintegrationResult& preintegration,
    const Eigen::Vector3d& reference_gyro_bias,
    const Eigen::Vector3d& reference_accel_bias,
    std::vector<ct_icp::TimedPoint> keypoints) {
  if (params_.fixed_lag_state_window <= 1) {
    return;
  }

  LaggedStateEntry entry;
  entry.state = state;
  entry.preintegration = preintegration;
  entry.reference_gyro_bias = reference_gyro_bias;
  entry.reference_accel_bias = reference_accel_bias;
  entry.keypoints = std::move(keypoints);
  state_history_.push_back(std::move(entry));
  const int max_history = std::max(params_.fixed_lag_state_window, 0);
  while (static_cast<int>(state_history_.size()) > max_history) {
    state_history_.pop_front();
  }
}

void CTLIORegistration::optimizeLaggedHistory() {
  if (!params_.fixed_lag_optimize_history || state_history_.size() < 2) {
    return;
  }

  const int num_states = static_cast<int>(state_history_.size());
  std::vector<std::array<double, 4>> begin_q(num_states);
  std::vector<std::array<double, 4>> end_q(num_states);
  std::vector<std::array<double, 3>> begin_t(num_states);
  std::vector<std::array<double, 3>> end_t(num_states);
  std::vector<std::array<double, 3>> begin_v(num_states);
  std::vector<std::array<double, 3>> gyro_bias(num_states);
  std::vector<std::array<double, 3>> accel_bias(num_states);

  for (int i = 0; i < num_states; ++i) {
    const auto& entry = state_history_[i];
    begin_q[i] = {entry.state.frame.begin_pose.quat.x(),
                  entry.state.frame.begin_pose.quat.y(),
                  entry.state.frame.begin_pose.quat.z(),
                  entry.state.frame.begin_pose.quat.w()};
    end_q[i] = {entry.state.frame.end_pose.quat.x(), entry.state.frame.end_pose.quat.y(),
                entry.state.frame.end_pose.quat.z(),
                entry.state.frame.end_pose.quat.w()};
    begin_t[i] = {entry.state.frame.begin_pose.trans.x(),
                  entry.state.frame.begin_pose.trans.y(),
                  entry.state.frame.begin_pose.trans.z()};
    end_t[i] = {entry.state.frame.end_pose.trans.x(), entry.state.frame.end_pose.trans.y(),
                entry.state.frame.end_pose.trans.z()};
    begin_v[i] = {entry.state.begin_velocity.x(), entry.state.begin_velocity.y(),
                  entry.state.begin_velocity.z()};
    gyro_bias[i] = {entry.state.gyro_bias.x(), entry.state.gyro_bias.y(),
                    entry.state.gyro_bias.z()};
    accel_bias[i] = {entry.state.accel_bias.x(), entry.state.accel_bias.y(),
                     entry.state.accel_bias.z()};
  }

  const auto syncStateHistory = [&]() {
    for (int i = 0; i < num_states; ++i) {
      auto& entry = state_history_[i];
      entry.state.frame.begin_pose.quat =
          Eigen::Quaterniond(begin_q[i][3], begin_q[i][0], begin_q[i][1],
                             begin_q[i][2]);
      entry.state.frame.end_pose.quat =
          Eigen::Quaterniond(end_q[i][3], end_q[i][0], end_q[i][1], end_q[i][2]);
      entry.state.frame.begin_pose.trans =
          Eigen::Vector3d(begin_t[i][0], begin_t[i][1], begin_t[i][2]);
      entry.state.frame.end_pose.trans =
          Eigen::Vector3d(end_t[i][0], end_t[i][1], end_t[i][2]);
      entry.state.begin_velocity =
          Eigen::Vector3d(begin_v[i][0], begin_v[i][1], begin_v[i][2]);
      entry.state.gyro_bias =
          Eigen::Vector3d(gyro_bias[i][0], gyro_bias[i][1], gyro_bias[i][2]);
      entry.state.accel_bias =
          Eigen::Vector3d(accel_bias[i][0], accel_bias[i][1], accel_bias[i][2]);
    }
  };

  const auto updateEndVelocities = [&]() {
    for (int i = 0; i < num_states; ++i) {
      auto& entry = state_history_[i];
      if (entry.preintegration.delta_t > 0.0) {
        const auto corrected_preintegration = entry.preintegration.correct(
            entry.state.gyro_bias - entry.reference_gyro_bias,
            entry.state.accel_bias - entry.reference_accel_bias);
        entry.state.end_velocity =
            entry.state.begin_velocity +
            params_.gravity * corrected_preintegration.delta_t +
            entry.state.frame.begin_pose.quat * corrected_preintegration.delta_v;
      } else {
        entry.state.end_velocity = entry.state.begin_velocity;
      }
    }
  };

  const Eigen::Vector3d oldest_begin_velocity =
      state_history_.front().state.begin_velocity;
  const Eigen::Vector3d oldest_gyro_bias = state_history_.front().state.gyro_bias;
  const Eigen::Vector3d oldest_accel_bias =
      state_history_.front().state.accel_bias;
  const int outer_iterations = std::max(1, params_.fixed_lag_outer_iterations);
  for (int outer_iter = 0; outer_iter < outer_iterations; ++outer_iter) {
    syncStateHistory();
    ceres::Problem problem;
    for (int i = 0; i < num_states; ++i) {
      problem.AddParameterBlock(
          begin_q[i].data(), 4);
    problem.SetManifold(begin_q[i].data(), new ceres::EigenQuaternionManifold());
      problem.AddParameterBlock(
          end_q[i].data(), 4);
    problem.SetManifold(end_q[i].data(), new ceres::EigenQuaternionManifold());
      problem.AddParameterBlock(begin_t[i].data(), 3);
      problem.AddParameterBlock(end_t[i].data(), 3);
      problem.AddParameterBlock(begin_v[i].data(), 3);
      problem.AddParameterBlock(gyro_bias[i].data(), 3);
      problem.AddParameterBlock(accel_bias[i].data(), 3);

      if (!params_.estimate_imu_bias) {
        problem.SetParameterBlockConstant(gyro_bias[i].data());
        problem.SetParameterBlockConstant(accel_bias[i].data());
      }
    }

    if (params_.velocity_prior_weight > 0.0) {
      problem.AddResidualBlock(
          VelocityPrior::Create(oldest_begin_velocity,
                                params_.velocity_prior_weight),
          nullptr, begin_v.front().data());
    }

    if (params_.estimate_imu_bias && params_.gyro_bias_prior_weight > 0.0) {
      problem.AddResidualBlock(
          BiasPrior::Create(oldest_gyro_bias, params_.gyro_bias_prior_weight),
          nullptr, gyro_bias.front().data());
    }
    if (params_.estimate_imu_bias && params_.accel_bias_prior_weight > 0.0) {
      problem.AddResidualBlock(
          BiasPrior::Create(oldest_accel_bias, params_.accel_bias_prior_weight),
          nullptr, accel_bias.front().data());
    }

    for (int i = 0; i < num_states; ++i) {
      const auto& entry = state_history_[i];
      const auto correspondences =
          findCorrespondences(entry.keypoints, entry.state.frame);
      ceres::LossFunction* surface_loss =
          new ceres::CauchyLoss(params_.cauchy_loss_param);
      for (const auto& corr : correspondences) {
        const auto& point = entry.keypoints[corr.point_idx];
        const double alpha = entry.state.frame.getAlpha(point.timestamp);
        problem.AddResidualBlock(
            ct_icp::CTPointToPlane::Create(point.raw_point, corr.reference,
                                           corr.normal, alpha, corr.weight),
            surface_loss, begin_q[i].data(), begin_t[i].data(), end_q[i].data(),
            end_t[i].data());
      }

      if (i + 1 < num_states) {
        const double pose_translation_weight =
            std::max(params_.imu_translation_weight, 1.0);
        const double pose_rotation_weight =
            std::max(params_.imu_rotation_weight, 1.0);
        problem.AddResidualBlock(
            TranslationDifferencePrior::Create(pose_translation_weight), nullptr,
            end_t[i].data(), begin_t[i + 1].data());
        problem.AddResidualBlock(
            OrientationDifferencePrior::Create(pose_rotation_weight), nullptr,
            end_q[i].data(), begin_q[i + 1].data());
      }

      if (entry.preintegration.delta_t <= 0.0) {
        continue;
      }

      if (params_.imu_rotation_weight > 0.0) {
        problem.AddResidualBlock(
            IMURotationBiasPrior::Create(
                entry.preintegration, entry.reference_gyro_bias,
                params_.imu_rotation_weight),
            nullptr, begin_q[i].data(), end_q[i].data(), gyro_bias[i].data());
      }
      if (params_.imu_translation_weight > 0.0) {
        problem.AddResidualBlock(
            IMUTranslationBiasPrior::Create(
                entry.preintegration, entry.reference_gyro_bias,
                entry.reference_accel_bias, entry.preintegration.delta_t,
                params_.gravity, params_.imu_translation_weight),
            nullptr, begin_q[i].data(), begin_t[i].data(), end_t[i].data(),
            begin_v[i].data(), gyro_bias[i].data(), accel_bias[i].data());
      }

      if (i + 1 < num_states && params_.imu_translation_weight > 0.0) {
        problem.AddResidualBlock(
            IMUVelocityBiasPrior::Create(
                entry.preintegration, entry.reference_gyro_bias,
                entry.reference_accel_bias, entry.preintegration.delta_t,
                params_.gravity, params_.imu_translation_weight),
            nullptr, begin_q[i].data(), begin_v[i].data(),
            begin_v[i + 1].data(), gyro_bias[i].data(), accel_bias[i].data());
      }

      if (i + 1 < num_states && params_.estimate_imu_bias) {
        const double gyro_diff_weight = biasRandomWalkWeight(
            params_.gyro_bias_prior_weight *
                params_.fixed_lag_gyro_bias_weight_scale,
            entry.preintegration.delta_t);
        if (gyro_diff_weight > 0.0) {
          problem.AddResidualBlock(BiasDifferencePrior::Create(gyro_diff_weight),
                                   nullptr, gyro_bias[i].data(),
                                   gyro_bias[i + 1].data());
        }

        const double accel_diff_weight = biasRandomWalkWeight(
            params_.accel_bias_prior_weight *
                params_.fixed_lag_accel_bias_weight_scale,
            entry.preintegration.delta_t);
        if (accel_diff_weight > 0.0) {
          problem.AddResidualBlock(
              BiasDifferencePrior::Create(accel_diff_weight), nullptr,
              accel_bias[i].data(), accel_bias[i + 1].data());
        }
      }
    }

    std::vector<std::array<double, 3>> previous_begin_t = begin_t;
    std::vector<std::array<double, 3>> previous_end_t = end_t;

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.max_num_iterations = params_.ceres_max_iterations;
    options.trust_region_strategy_type = ceres::LEVENBERG_MARQUARDT;
    options.minimizer_progress_to_stdout = false;
    options.num_threads = 1;

    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    double max_translation_update = 0.0;
    for (int i = 0; i < num_states; ++i) {
      const Eigen::Vector3d begin_update =
          Eigen::Vector3d(begin_t[i][0], begin_t[i][1], begin_t[i][2]) -
          Eigen::Vector3d(previous_begin_t[i][0], previous_begin_t[i][1],
                          previous_begin_t[i][2]);
      const Eigen::Vector3d end_update =
          Eigen::Vector3d(end_t[i][0], end_t[i][1], end_t[i][2]) -
          Eigen::Vector3d(previous_end_t[i][0], previous_end_t[i][1],
                          previous_end_t[i][2]);
      max_translation_update =
          std::max(max_translation_update, begin_update.norm());
      max_translation_update =
          std::max(max_translation_update, end_update.norm());
    }
    if (max_translation_update < params_.convergence_threshold) {
      break;
    }
  }

  syncStateHistory();
  updateEndVelocities();
}

std::vector<ct_icp::TimedPoint> CTLIORegistration::subsampleKeypoints(
    const std::vector<ct_icp::TimedPoint>& points) const {
  std::unordered_map<ct_icp::Voxel, int, ct_icp::VoxelHash> voxel_to_idx;
  std::vector<ct_icp::TimedPoint> keypoints;

  for (const auto& point : points) {
    ct_icp::Voxel voxel =
        pointToVoxel(point.raw_point, params_.keypoint_voxel_size);
    if (voxel_to_idx.find(voxel) != voxel_to_idx.end()) {
      continue;
    }
    voxel_to_idx[voxel] = static_cast<int>(keypoints.size());
    keypoints.push_back(point);
  }

  return keypoints;
}

std::vector<CTLIORegistration::Correspondence>
CTLIORegistration::findCorrespondences(
    const std::vector<ct_icp::TimedPoint>& keypoints,
    const ct_icp::TrajectoryFrame& frame) const {
  std::vector<Correspondence> correspondences;

  for (size_t i = 0; i < keypoints.size(); ++i) {
    Eigen::Vector3d world_point =
        frame.transformPoint(keypoints[i].raw_point, keypoints[i].timestamp);
    ct_icp::Voxel center = pointToVoxel(world_point, params_.voxel_resolution);

    std::vector<Eigen::Vector3d> neighbors;
    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dz = -1; dz <= 1; ++dz) {
          ct_icp::Voxel voxel{center.x + dx, center.y + dy, center.z + dz};
          auto it = voxel_map_.find(voxel);
          if (it == voxel_map_.end()) {
            continue;
          }
          for (int k = 0; k < it->second.num_points; ++k) {
            neighbors.push_back(it->second.points[k]);
          }
        }
      }
    }

    if (static_cast<int>(neighbors.size()) < params_.knn) {
      continue;
    }

    std::vector<std::pair<double, int>> distances;
    distances.reserve(neighbors.size());
    for (size_t j = 0; j < neighbors.size(); ++j) {
      distances.emplace_back((neighbors[j] - world_point).squaredNorm(),
                             static_cast<int>(j));
    }

    const int knn = std::min(params_.knn, static_cast<int>(distances.size()));
    std::partial_sort(distances.begin(), distances.begin() + knn,
                      distances.end());
    if (distances[knn - 1].first > params_.max_correspondence_dist) {
      continue;
    }

    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    for (int j = 0; j < knn; ++j) {
      mean += neighbors[distances[j].second];
    }
    mean /= static_cast<double>(knn);

    Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
    for (int j = 0; j < knn; ++j) {
      Eigen::Vector3d diff = neighbors[distances[j].second] - mean;
      covariance += diff * diff.transpose();
    }
    covariance /= static_cast<double>(knn);

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(covariance);
    Eigen::Vector3d eigenvalues = solver.eigenvalues();
    if (eigenvalues(2) < 1e-10) {
      continue;
    }

    double planarity = (eigenvalues(1) - eigenvalues(0)) / eigenvalues(2);
    if (planarity < params_.planarity_threshold) {
      continue;
    }

    Eigen::Vector3d normal = solver.eigenvectors().col(0);
    if (normal.dot(world_point - mean) < 0.0) {
      normal = -normal;
    }

    Correspondence corr;
    corr.point_idx = static_cast<int>(i);
    corr.reference = mean;
    corr.normal = normal;
    corr.weight = planarity;
    correspondences.push_back(corr);
  }

  return correspondences;
}

void CTLIORegistration::initializeFromImu(
    const imu_preintegration::PreintegrationResult& preintegration,
    CTLIOState* state) const {
  if (preintegration.delta_t <= 0.0 || state == nullptr) {
    return;
  }

  const Eigen::Quaterniond begin_quat = state->frame.begin_pose.quat;
  state->frame.end_pose.quat =
      begin_quat * Eigen::Quaterniond(preintegration.delta_R);
  state->frame.end_pose.trans =
      state->frame.begin_pose.trans +
      state->begin_velocity * preintegration.delta_t +
      0.5 * params_.gravity * preintegration.delta_t * preintegration.delta_t +
      begin_quat * preintegration.delta_p;
  state->end_velocity =
      state->begin_velocity + params_.gravity * preintegration.delta_t +
      begin_quat * preintegration.delta_v;
}

CTLIOResult CTLIORegistration::registerFrame(
    const std::vector<ct_icp::TimedPoint>& timed_points,
    const CTLIOState& initial_state,
    const std::vector<imu_preintegration::ImuSample>& imu_samples) {
  CTLIOResult result;
  result.state = initial_state;

  imu_preintegration::ImuPreintegrator preintegrator;
  if (imu_samples.size() >= 2) {
    preintegrator.setBias(initial_state.gyro_bias, initial_state.accel_bias);
    preintegrator.integrateBatch(imu_samples);
    result.preintegration = preintegrator.result();
    initializeFromImu(result.preintegration, &result.state);
  }

  if (voxel_map_.empty()) {
    result.converged = true;
    rememberState(result.state, result.preintegration, initial_state.gyro_bias,
                  initial_state.accel_bias, {});
    optimizeLaggedHistory();
    if (!state_history_.empty()) {
      result.state = state_history_.back().state;
    }
    return result;
  }

  auto keypoints = subsampleKeypoints(timed_points);
  if (keypoints.empty()) {
    rememberState(result.state, result.preintegration, initial_state.gyro_bias,
                  initial_state.accel_bias, {});
    optimizeLaggedHistory();
    if (!state_history_.empty()) {
      result.state = state_history_.back().state;
    }
    return result;
  }

  const auto lagged_prior = computeLaggedStatePrior();

  for (auto& keypoint : keypoints) {
    keypoint.alpha = result.state.frame.getAlpha(keypoint.timestamp);
  }

  double begin_q[4] = {result.state.frame.begin_pose.quat.x(),
                       result.state.frame.begin_pose.quat.y(),
                       result.state.frame.begin_pose.quat.z(),
                       result.state.frame.begin_pose.quat.w()};
  double begin_t[3] = {result.state.frame.begin_pose.trans.x(),
                       result.state.frame.begin_pose.trans.y(),
                       result.state.frame.begin_pose.trans.z()};
  double end_q[4] = {result.state.frame.end_pose.quat.x(),
                     result.state.frame.end_pose.quat.y(),
                     result.state.frame.end_pose.quat.z(),
                     result.state.frame.end_pose.quat.w()};
  double end_t[3] = {result.state.frame.end_pose.trans.x(),
                     result.state.frame.end_pose.trans.y(),
                     result.state.frame.end_pose.trans.z()};
  double begin_v[3] = {result.state.begin_velocity.x(),
                       result.state.begin_velocity.y(),
                       result.state.begin_velocity.z()};
  double gyro_bias[3] = {result.state.gyro_bias.x(),
                         result.state.gyro_bias.y(),
                         result.state.gyro_bias.z()};
  double accel_bias[3] = {result.state.accel_bias.x(),
                          result.state.accel_bias.y(),
                          result.state.accel_bias.z()};

  Eigen::Map<Eigen::Quaterniond> q_begin(begin_q);
  Eigen::Map<Eigen::Vector3d> t_begin(begin_t);
  Eigen::Map<Eigen::Quaterniond> q_end(end_q);
  Eigen::Map<Eigen::Vector3d> t_end(end_t);
  Eigen::Map<Eigen::Vector3d> v_begin(begin_v);
  Eigen::Map<Eigen::Vector3d> bg(gyro_bias);
  Eigen::Map<Eigen::Vector3d> ba(accel_bias);

  for (int iter = 0; iter < params_.max_iterations; ++iter) {
    result.state.frame.begin_pose.quat = q_begin;
    result.state.frame.begin_pose.trans = t_begin;
    result.state.frame.end_pose.quat = q_end;
    result.state.frame.end_pose.trans = t_end;
    result.state.begin_velocity = v_begin;
    result.state.gyro_bias = bg;
    result.state.accel_bias = ba;

    auto correspondences = findCorrespondences(keypoints, result.state.frame);
    result.num_correspondences = static_cast<int>(correspondences.size());
    if (correspondences.empty()) {
      break;
    }

    ceres::Problem problem;
    problem.AddParameterBlock(
        begin_q, 4);
    problem.SetManifold(begin_q, new ceres::EigenQuaternionManifold());
    problem.AddParameterBlock(begin_t, 3);
    problem.AddParameterBlock(
        end_q, 4);
    problem.SetManifold(end_q, new ceres::EigenQuaternionManifold());
    problem.AddParameterBlock(end_t, 3);
    problem.AddParameterBlock(begin_v, 3);
    problem.AddParameterBlock(gyro_bias, 3);
    problem.AddParameterBlock(accel_bias, 3);
    if (!params_.estimate_imu_bias) {
      problem.SetParameterBlockConstant(gyro_bias);
      problem.SetParameterBlockConstant(accel_bias);
    }

    ceres::LossFunction* loss =
        new ceres::CauchyLoss(params_.cauchy_loss_param);
    for (const auto& corr : correspondences) {
      const auto& point = keypoints[corr.point_idx];
      problem.AddResidualBlock(
          ct_icp::CTPointToPlane::Create(point.raw_point, corr.reference,
                                         corr.normal, point.alpha, corr.weight),
          loss, begin_q, begin_t, end_q, end_t);
    }

    if (result.preintegration.delta_t > 0.0) {
      if (params_.imu_rotation_weight > 0.0) {
        problem.AddResidualBlock(
            IMURotationBiasPrior::Create(
                result.preintegration, initial_state.gyro_bias,
                params_.imu_rotation_weight),
            nullptr, begin_q, end_q, gyro_bias);
      }

      if (params_.imu_translation_weight > 0.0) {
        problem.AddResidualBlock(
            IMUTranslationBiasPrior::Create(
                result.preintegration, initial_state.gyro_bias,
                initial_state.accel_bias, result.preintegration.delta_t,
                params_.gravity, params_.imu_translation_weight),
            nullptr, begin_q, begin_t, end_t, begin_v, gyro_bias, accel_bias);
      }
    }

    if (params_.velocity_prior_weight > 0.0) {
      problem.AddResidualBlock(
          VelocityPrior::Create(initial_state.begin_velocity,
                                params_.velocity_prior_weight),
          nullptr, begin_v);
    }

    if (lagged_prior.valid && params_.fixed_lag_velocity_prior_weight > 0.0) {
      problem.AddResidualBlock(
          VelocityPrior::Create(lagged_prior.velocity,
                                params_.fixed_lag_velocity_prior_weight),
          nullptr, begin_v);
    }

    const double bias_delta_t =
        result.preintegration.delta_t > 0.0 ? result.preintegration.delta_t : 0.0;
    const double gyro_bias_prior_weight =
        params_.estimate_imu_bias
            ? biasRandomWalkWeight(params_.gyro_bias_prior_weight, bias_delta_t)
            : params_.gyro_bias_prior_weight;
    const double accel_bias_prior_weight =
        params_.estimate_imu_bias
            ? biasRandomWalkWeight(params_.accel_bias_prior_weight, bias_delta_t)
            : params_.accel_bias_prior_weight;

    if (gyro_bias_prior_weight > 0.0) {
      problem.AddResidualBlock(
          BiasPrior::Create(initial_state.gyro_bias, gyro_bias_prior_weight),
          nullptr, gyro_bias);
    }

    if (lagged_prior.valid && params_.estimate_imu_bias &&
        gyro_bias_prior_weight > 0.0 &&
        params_.fixed_lag_gyro_bias_weight_scale > 0.0) {
      problem.AddResidualBlock(
          BiasPrior::Create(
              lagged_prior.gyro_bias,
              gyro_bias_prior_weight * params_.fixed_lag_gyro_bias_weight_scale),
          nullptr, gyro_bias);
    }

    if (accel_bias_prior_weight > 0.0) {
      problem.AddResidualBlock(
          BiasPrior::Create(initial_state.accel_bias, accel_bias_prior_weight),
          nullptr, accel_bias);
    }

    if (lagged_prior.valid && params_.estimate_imu_bias &&
        accel_bias_prior_weight > 0.0 &&
        params_.fixed_lag_accel_bias_weight_scale > 0.0) {
      problem.AddResidualBlock(
          BiasPrior::Create(
              lagged_prior.accel_bias,
              accel_bias_prior_weight * params_.fixed_lag_accel_bias_weight_scale),
          nullptr, accel_bias);
    }

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.max_num_iterations = params_.ceres_max_iterations;
    options.trust_region_strategy_type = ceres::LEVENBERG_MARQUARDT;
    options.minimizer_progress_to_stdout = false;
    options.num_threads = 1;

    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    Eigen::Vector3d begin_diff =
        Eigen::Map<Eigen::Vector3d>(begin_t) - result.state.frame.begin_pose.trans;
    Eigen::Vector3d end_diff =
        Eigen::Map<Eigen::Vector3d>(end_t) - result.state.frame.end_pose.trans;
    result.num_iterations = iter + 1;

    if (begin_diff.norm() < params_.convergence_threshold &&
        end_diff.norm() < params_.convergence_threshold) {
      result.converged = true;
      break;
    }
  }

  result.state.frame.begin_pose.quat = q_begin;
  result.state.frame.begin_pose.trans = t_begin;
  result.state.frame.end_pose.quat = q_end;
  result.state.frame.end_pose.trans = t_end;
  result.state.begin_velocity = v_begin;
  result.state.gyro_bias = bg;
  result.state.accel_bias = ba;

  if (result.preintegration.delta_t > 0.0) {
    auto corrected_preintegration =
        result.preintegration.correct(result.state.gyro_bias - initial_state.gyro_bias,
                                      result.state.accel_bias - initial_state.accel_bias);
    result.state.end_velocity =
        result.state.begin_velocity +
        params_.gravity * corrected_preintegration.delta_t +
        result.state.frame.begin_pose.quat * corrected_preintegration.delta_v;
  } else {
    result.state.end_velocity = result.state.begin_velocity;
  }

  rememberState(result.state, result.preintegration, initial_state.gyro_bias,
                initial_state.accel_bias, std::move(keypoints));
  optimizeLaggedHistory();
  if (!state_history_.empty()) {
    result.state = state_history_.back().state;
  }
  return result;
}

}  // namespace ct_lio
}  // namespace localization_zoo
