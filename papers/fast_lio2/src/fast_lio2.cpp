#include "fast_lio2/fast_lio2.h"

#include <ceres/ceres.h>
#include <Eigen/Eigenvalues>

#include <algorithm>
#include <array>
#include <cmath>
#include <unordered_map>

namespace localization_zoo {
namespace fast_lio2 {

namespace {

struct PointToPlaneCost {
  PointToPlaneCost(const Eigen::Vector3d& raw_point,
                   const Eigen::Vector3d& reference,
                   const Eigen::Vector3d& normal,
                   double weight)
      : raw_point_(raw_point),
        reference_(reference),
        normal_(normal),
        weight_(weight) {}

  template <typename T>
  bool operator()(const T* quat_ptr, const T* trans_ptr,
                  T* residual_ptr) const {
    const Eigen::Quaternion<T> quat(quat_ptr[3], quat_ptr[0], quat_ptr[1],
                                    quat_ptr[2]);
    const Eigen::Matrix<T, 3, 1> trans(trans_ptr[0], trans_ptr[1], trans_ptr[2]);
    const Eigen::Matrix<T, 3, 1> raw(T(raw_point_.x()), T(raw_point_.y()),
                                     T(raw_point_.z()));
    const Eigen::Matrix<T, 3, 1> reference(T(reference_.x()), T(reference_.y()),
                                           T(reference_.z()));
    const Eigen::Matrix<T, 3, 1> normal(T(normal_.x()), T(normal_.y()),
                                        T(normal_.z()));
    residual_ptr[0] = T(weight_) * normal.dot(reference - (quat * raw + trans));
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& raw_point,
                                     const Eigen::Vector3d& reference,
                                     const Eigen::Vector3d& normal,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<PointToPlaneCost, 1, 4, 3>(
        new PointToPlaneCost(raw_point, reference, normal, weight));
  }

  Eigen::Vector3d raw_point_;
  Eigen::Vector3d reference_;
  Eigen::Vector3d normal_;
  double weight_;
};

struct RotationPrior {
  RotationPrior(const Eigen::Quaterniond& previous_quat,
                const Eigen::Quaterniond& delta_quat,
                double weight)
      : expected_quat_((previous_quat * delta_quat).normalized()),
        weight_(weight) {}

  template <typename T>
  bool operator()(const T* quat_ptr, T* residual_ptr) const {
    const Eigen::Quaternion<T> expected(T(expected_quat_.w()),
                                        T(expected_quat_.x()),
                                        T(expected_quat_.y()),
                                        T(expected_quat_.z()));
    const Eigen::Quaternion<T> current(quat_ptr[3], quat_ptr[0], quat_ptr[1],
                                       quat_ptr[2]);
    const Eigen::Quaternion<T> error = expected.conjugate() * current;
    residual_ptr[0] = T(weight_) * T(2.0) * error.x();
    residual_ptr[1] = T(weight_) * T(2.0) * error.y();
    residual_ptr[2] = T(weight_) * T(2.0) * error.z();
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Quaterniond& previous_quat,
                                     const Eigen::Quaterniond& delta_quat,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<RotationPrior, 3, 4>(
        new RotationPrior(previous_quat, delta_quat, weight));
  }

  Eigen::Quaterniond expected_quat_;
  double weight_;
};

struct TranslationPrior {
  TranslationPrior(const Eigen::Quaterniond& previous_quat,
                   const Eigen::Vector3d& previous_trans,
                   const Eigen::Vector3d& previous_velocity,
                   const Eigen::Vector3d& delta_p,
                   double delta_t,
                   const Eigen::Vector3d& gravity,
                   double weight)
      : expected_trans_(previous_trans + previous_velocity * delta_t +
                        0.5 * gravity * delta_t * delta_t +
                        previous_quat * delta_p),
        weight_(weight) {}

  template <typename T>
  bool operator()(const T* trans_ptr, T* residual_ptr) const {
    residual_ptr[0] = T(weight_) * (trans_ptr[0] - T(expected_trans_.x()));
    residual_ptr[1] = T(weight_) * (trans_ptr[1] - T(expected_trans_.y()));
    residual_ptr[2] = T(weight_) * (trans_ptr[2] - T(expected_trans_.z()));
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Quaterniond& previous_quat,
                                     const Eigen::Vector3d& previous_trans,
                                     const Eigen::Vector3d& previous_velocity,
                                     const Eigen::Vector3d& delta_p,
                                     double delta_t,
                                     const Eigen::Vector3d& gravity,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<TranslationPrior, 3, 3>(
        new TranslationPrior(previous_quat, previous_trans, previous_velocity,
                             delta_p, delta_t, gravity, weight));
  }

  Eigen::Vector3d expected_trans_;
  double weight_;
};

struct VectorPrior {
  VectorPrior(const Eigen::Vector3d& target, double weight)
      : target_(target), weight_(weight) {}

  template <typename T>
  bool operator()(const T* value_ptr, T* residual_ptr) const {
    residual_ptr[0] = T(weight_) * (value_ptr[0] - T(target_.x()));
    residual_ptr[1] = T(weight_) * (value_ptr[1] - T(target_.y()));
    residual_ptr[2] = T(weight_) * (value_ptr[2] - T(target_.z()));
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Vector3d& target,
                                     double weight) {
    return new ceres::AutoDiffCostFunction<VectorPrior, 3, 3>(
        new VectorPrior(target, weight));
  }

  Eigen::Vector3d target_;
  double weight_;
};

double rotationDelta(const Eigen::Quaterniond& before,
                     const Eigen::Quaterniond& after) {
  const Eigen::Quaterniond delta = before.conjugate() * after;
  const double clamped = std::clamp(std::abs(delta.normalized().w()), 0.0, 1.0);
  return 2.0 * std::acos(clamped);
}

}  // namespace

ct_icp::Voxel FastLio2::pointToVoxel(const Eigen::Vector3d& point,
                                     double resolution) {
  return ct_icp::Voxel{static_cast<int>(std::floor(point.x() / resolution)),
                       static_cast<int>(std::floor(point.y() / resolution)),
                       static_cast<int>(std::floor(point.z() / resolution))};
}

ct_icp::VoxelHashMap FastLio2::buildFrameMap(
    const std::vector<Eigen::Vector3d>& world_points,
    double resolution) {
  ct_icp::VoxelHashMap frame_map;
  for (const auto& point : world_points) {
    frame_map[pointToVoxel(point, resolution)].addPoint(point);
  }
  return frame_map;
}

void FastLio2::mergeFrameMap(const ct_icp::VoxelHashMap& frame_map,
                             ct_icp::VoxelHashMap* voxel_map) {
  for (const auto& [voxel, block] : frame_map) {
    auto& destination = (*voxel_map)[voxel];
    for (int i = 0; i < block.num_points; ++i) {
      destination.addPoint(block.points[i]);
    }
  }
}

void FastLio2::rebuildMapFromWindow() {
  voxel_map_.clear();
  for (auto it = frame_maps_.rbegin(); it != frame_maps_.rend(); ++it) {
    mergeFrameMap(*it, &voxel_map_);
  }
}

void FastLio2::addPointsToMap(const std::vector<Eigen::Vector3d>& world_points) {
  if (world_points.empty()) {
    return;
  }

  if (params_.max_frames_in_map <= 0) {
    mergeFrameMap(buildFrameMap(world_points, params_.voxel_resolution),
                  &voxel_map_);
    return;
  }

  frame_maps_.push_back(buildFrameMap(world_points, params_.voxel_resolution));
  while (static_cast<int>(frame_maps_.size()) > params_.max_frames_in_map) {
    frame_maps_.pop_front();
  }
  rebuildMapFromWindow();
}

std::vector<Eigen::Vector3d> FastLio2::subsamplePoints(
    const std::vector<Eigen::Vector3d>& raw_points) const {
  std::unordered_map<ct_icp::Voxel, int, ct_icp::VoxelHash> voxel_to_index;
  std::vector<Eigen::Vector3d> keypoints;
  keypoints.reserve(raw_points.size());

  for (const auto& point : raw_points) {
    const ct_icp::Voxel voxel =
        pointToVoxel(point, params_.keypoint_voxel_size);
    if (voxel_to_index.find(voxel) != voxel_to_index.end()) {
      continue;
    }
    voxel_to_index[voxel] = static_cast<int>(keypoints.size());
    keypoints.push_back(point);
  }

  return keypoints;
}

std::vector<FastLio2::Correspondence> FastLio2::findCorrespondences(
    const std::vector<Eigen::Vector3d>& keypoints,
    const Eigen::Quaterniond& quat,
    const Eigen::Vector3d& trans) const {
  std::vector<Correspondence> correspondences;
  const double max_distance_sq =
      params_.max_correspondence_distance * params_.max_correspondence_distance;

  for (size_t i = 0; i < keypoints.size(); ++i) {
    const Eigen::Vector3d world_point = quat * keypoints[i] + trans;
    const ct_icp::Voxel center =
        pointToVoxel(world_point, params_.voxel_resolution);

    std::vector<Eigen::Vector3d> neighbors;
    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dz = -1; dz <= 1; ++dz) {
          auto it = voxel_map_.find(
              ct_icp::Voxel{center.x + dx, center.y + dy, center.z + dz});
          if (it == voxel_map_.end()) {
            continue;
          }
          for (int point_index = 0; point_index < it->second.num_points;
               ++point_index) {
            neighbors.push_back(it->second.points[point_index]);
          }
        }
      }
    }

    if (static_cast<int>(neighbors.size()) < params_.knn) {
      continue;
    }

    std::vector<std::pair<double, int>> distances;
    distances.reserve(neighbors.size());
    for (size_t neighbor_index = 0; neighbor_index < neighbors.size();
         ++neighbor_index) {
      distances.emplace_back((neighbors[neighbor_index] - world_point).squaredNorm(),
                             static_cast<int>(neighbor_index));
    }

    const int knn = std::min(params_.knn, static_cast<int>(distances.size()));
    std::partial_sort(distances.begin(), distances.begin() + knn,
                      distances.end());
    if (distances[knn - 1].first > max_distance_sq) {
      continue;
    }

    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    for (int j = 0; j < knn; ++j) {
      mean += neighbors[distances[j].second];
    }
    mean /= static_cast<double>(knn);

    Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
    for (int j = 0; j < knn; ++j) {
      const Eigen::Vector3d diff = neighbors[distances[j].second] - mean;
      covariance += diff * diff.transpose();
    }
    covariance /= static_cast<double>(knn);

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(covariance);
    const Eigen::Vector3d eigenvalues = solver.eigenvalues();
    if (eigenvalues(2) < 1e-10) {
      continue;
    }

    const double planarity =
        (eigenvalues(1) - eigenvalues(0)) / eigenvalues(2);
    if (planarity < params_.planarity_threshold) {
      continue;
    }

    Eigen::Vector3d normal = solver.eigenvectors().col(0);
    if (normal.dot(world_point - mean) < 0.0) {
      normal = -normal;
    }

    correspondences.push_back(
        {static_cast<int>(i), mean, normal, std::max(planarity, 1e-3)});
  }

  return correspondences;
}

void FastLio2::predictFromImu(
    const FastLio2State& previous_state,
    const imu_preintegration::PreintegrationResult& preintegration,
    FastLio2State* predicted_state) const {
  if (predicted_state == nullptr || preintegration.delta_t <= 0.0) {
    return;
  }

  predicted_state->quat =
      (previous_state.quat * Eigen::Quaterniond(preintegration.delta_R))
          .normalized();
  predicted_state->trans =
      previous_state.trans + previous_state.velocity * preintegration.delta_t +
      0.5 * params_.gravity * preintegration.delta_t * preintegration.delta_t +
      previous_state.quat * preintegration.delta_p;
  predicted_state->velocity =
      previous_state.velocity + params_.gravity * preintegration.delta_t +
      previous_state.quat * preintegration.delta_v;
}

std::vector<Eigen::Vector3d> FastLio2::transformPoints(
    const std::vector<Eigen::Vector3d>& raw_points,
    const Eigen::Quaterniond& quat,
    const Eigen::Vector3d& trans) {
  std::vector<Eigen::Vector3d> world_points;
  world_points.reserve(raw_points.size());
  for (const auto& point : raw_points) {
    world_points.push_back(quat * point + trans);
  }
  return world_points;
}

FastLio2Result FastLio2::process(
    const std::vector<Eigen::Vector3d>& raw_points,
    const std::vector<imu_preintegration::ImuSample>& imu_samples) {
  FastLio2Result result;
  result.state = state_;
  result.initialized = initialized_;
  result.map_size = mapSize();

  if (raw_points.empty()) {
    return result;
  }

  imu_preintegration::ImuPreintegrator preintegrator(params_.imu_noise);
  preintegrator.setBias(state_.gyro_bias, state_.accel_bias);
  if (imu_samples.size() >= 2) {
    preintegrator.integrateBatch(imu_samples);
    result.preintegration = preintegrator.result();
    result.imu_used = result.preintegration.delta_t > 0.0;
  }

  const FastLio2State previous_state = state_;
  FastLio2State predicted_state = state_;
  if (result.imu_used) {
    predictFromImu(previous_state, result.preintegration, &predicted_state);
  }

  if (!initialized_ || voxel_map_.empty()) {
    state_ = predicted_state;
    initialized_ = true;
    addPointsToMap(transformPoints(raw_points, state_.quat, state_.trans));
    result.state = state_;
    result.converged = true;
    result.initialized = true;
    result.map_size = mapSize();
    return result;
  }

  const std::vector<Eigen::Vector3d> keypoints = subsamplePoints(raw_points);
  if (keypoints.empty()) {
    state_ = predicted_state;
    addPointsToMap(transformPoints(raw_points, state_.quat, state_.trans));
    result.state = state_;
    result.valid = true;
    result.converged = true;
    result.initialized = true;
    result.map_size = mapSize();
    return result;
  }

  double quat_param[4] = {predicted_state.quat.x(), predicted_state.quat.y(),
                          predicted_state.quat.z(), predicted_state.quat.w()};
  double trans_param[3] = {predicted_state.trans.x(), predicted_state.trans.y(),
                           predicted_state.trans.z()};
  double velocity_param[3] = {predicted_state.velocity.x(),
                              predicted_state.velocity.y(),
                              predicted_state.velocity.z()};

  for (int iter = 0; iter < params_.max_iterations; ++iter) {
    const Eigen::Quaterniond current_quat(quat_param[3], quat_param[0],
                                          quat_param[1], quat_param[2]);
    const Eigen::Vector3d current_trans(trans_param[0], trans_param[1],
                                        trans_param[2]);
    const auto correspondences =
        findCorrespondences(keypoints, current_quat.normalized(), current_trans);
    result.num_correspondences = static_cast<int>(correspondences.size());
    result.num_iterations = iter + 1;

    if (correspondences.size() < 10) {
      break;
    }

    const Eigen::Quaterniond previous_quat = current_quat.normalized();
    const Eigen::Vector3d previous_trans = current_trans;

    ceres::Problem problem;
    problem.AddParameterBlock(quat_param, 4);
    problem.SetManifold(quat_param, new ceres::EigenQuaternionManifold());
    problem.AddParameterBlock(trans_param, 3);
    problem.AddParameterBlock(velocity_param, 3);

    ceres::LossFunction* lidar_loss =
        new ceres::CauchyLoss(params_.cauchy_loss_param);
    for (const auto& correspondence : correspondences) {
      problem.AddResidualBlock(
          PointToPlaneCost::Create(keypoints[correspondence.point_idx],
                                   correspondence.reference,
                                   correspondence.normal,
                                   correspondence.weight),
          lidar_loss, quat_param, trans_param);
    }

    if (params_.velocity_prior_weight > 0.0) {
      problem.AddResidualBlock(
          VectorPrior::Create(predicted_state.velocity,
                              params_.velocity_prior_weight),
          nullptr, velocity_param);
    }

    if (result.imu_used) {
      if (params_.imu_rotation_weight > 0.0) {
        problem.AddResidualBlock(
            RotationPrior::Create(previous_state.quat,
                                  Eigen::Quaterniond(result.preintegration.delta_R),
                                  params_.imu_rotation_weight),
            nullptr, quat_param);
      }
      if (params_.imu_translation_weight > 0.0) {
        problem.AddResidualBlock(
            TranslationPrior::Create(previous_state.quat, previous_state.trans,
                                     previous_state.velocity,
                                     result.preintegration.delta_p,
                                     result.preintegration.delta_t,
                                     params_.gravity,
                                     params_.imu_translation_weight),
            nullptr, trans_param);
      }
      if (params_.imu_velocity_weight > 0.0) {
        const Eigen::Vector3d expected_velocity =
            previous_state.velocity +
            params_.gravity * result.preintegration.delta_t +
            previous_state.quat * result.preintegration.delta_v;
        problem.AddResidualBlock(
            VectorPrior::Create(expected_velocity, params_.imu_velocity_weight),
            nullptr, velocity_param);
      }
    }

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_QR;
    options.max_num_iterations = params_.ceres_max_iterations;
    options.trust_region_strategy_type = ceres::LEVENBERG_MARQUARDT;
    options.minimizer_progress_to_stdout = false;
    options.num_threads = 1;

    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    const Eigen::Quaterniond solved_quat(quat_param[3], quat_param[0],
                                         quat_param[1], quat_param[2]);
    const Eigen::Vector3d solved_trans(trans_param[0], trans_param[1],
                                       trans_param[2]);
    if ((solved_trans - previous_trans).norm() <
            params_.convergence_threshold &&
        rotationDelta(previous_quat, solved_quat.normalized()) <
            params_.convergence_threshold) {
      result.converged = true;
      break;
    }
  }

  state_.quat =
      Eigen::Quaterniond(quat_param[3], quat_param[0], quat_param[1],
                         quat_param[2])
          .normalized();
  state_.trans = Eigen::Vector3d(trans_param[0], trans_param[1], trans_param[2]);
  state_.velocity =
      Eigen::Vector3d(velocity_param[0], velocity_param[1], velocity_param[2]);
  if (result.imu_used && result.preintegration.delta_t > 0.0) {
    state_.velocity =
        (state_.trans - previous_state.trans) / result.preintegration.delta_t;
  }

  addPointsToMap(transformPoints(raw_points, state_.quat, state_.trans));

  result.state = state_;
  result.valid = result.num_correspondences >= 10;
  result.initialized = initialized_;
  result.map_size = mapSize();
  return result;
}

void FastLio2::clear() {
  voxel_map_.clear();
  frame_maps_.clear();
  state_ = FastLio2State();
  initialized_ = false;
}

}  // namespace fast_lio2
}  // namespace localization_zoo
