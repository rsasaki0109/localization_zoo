#include "vilens/vilens.h"

#include <ceres/ceres.h>
#include <pcl/kdtree/kdtree_flann.h>

#include <Eigen/Eigenvalues>

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace localization_zoo {
namespace vilens {

namespace {

struct VoxelHash {
  std::size_t operator()(const Eigen::Vector3i& key) const {
    std::size_t seed = 0;
    auto mix = [&seed](int value) {
      seed ^= std::hash<int>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    mix(key.x());
    mix(key.y());
    mix(key.z());
    return seed;
  }
};

pcl::PointXYZ toPoint(const Eigen::Vector3d& point) {
  return pcl::PointXYZ(static_cast<float>(point.x()),
                       static_cast<float>(point.y()),
                       static_cast<float>(point.z()));
}

template <typename T>
Eigen::Matrix<T, 3, 3> skew(const Eigen::Matrix<T, 3, 1>& v) {
  Eigen::Matrix<T, 3, 3> m;
  m << T(0), -v.z(), v.y(),
       v.z(), T(0), -v.x(),
       -v.y(), v.x(), T(0);
  return m;
}

template <typename T>
Eigen::Matrix<T, 3, 3> expSO3(const Eigen::Matrix<T, 3, 1>& omega) {
  const T theta = ceres::sqrt(omega.squaredNorm() + T(1e-12));
  const Eigen::Matrix<T, 3, 1> axis = omega / theta;
  const Eigen::Matrix<T, 3, 3> K = skew(axis);
  return Eigen::Matrix<T, 3, 3>::Identity() +
         ceres::sin(theta) * K +
         (T(1.0) - ceres::cos(theta)) * K * K;
}

Eigen::Matrix3d expSO3d(const Eigen::Vector3d& omega) {
  const double theta = omega.norm();
  if (theta < 1e-12) {
    return Eigen::Matrix3d::Identity();
  }
  return Eigen::AngleAxisd(theta, omega / theta).toRotationMatrix();
}

struct ZeroPriorResidual {
  explicit ZeroPriorResidual(double weight) : weight(weight) {}

  template <typename T>
  bool operator()(const T* const delta, T* residuals) const {
    residuals[0] = T(weight) * delta[0];
    residuals[1] = T(weight) * delta[1];
    residuals[2] = T(weight) * delta[2];
    return true;
  }

  double weight = 0.0;
};

struct VisualReprojectionResidual {
  VisualReprojectionResidual(const Eigen::Matrix3d& prior_rotation,
                             const Eigen::Vector3d& prior_translation,
                             const CameraIntrinsics& camera,
                             const Eigen::Matrix3d& body_R_camera,
                             const Eigen::Vector3d& body_t_camera,
                             const Eigen::Vector3d& landmark_world,
                             const Eigen::Vector2d& pixel,
                             double pixel_sigma)
      : prior_rotation(prior_rotation),
        prior_translation(prior_translation),
        camera(camera),
        body_R_camera(body_R_camera),
        body_t_camera(body_t_camera),
        landmark_world(landmark_world),
        pixel(pixel),
        pixel_sigma(pixel_sigma) {}

  template <typename T>
  bool operator()(const T* const delta_rotation,
                  const T* const delta_translation,
                  T* residuals) const {
    const Eigen::Matrix<T, 3, 1> delta_rot(delta_rotation[0], delta_rotation[1],
                                           delta_rotation[2]);
    const Eigen::Matrix<T, 3, 1> delta_trans(delta_translation[0],
                                             delta_translation[1],
                                             delta_translation[2]);

    const Eigen::Matrix<T, 3, 3> R_wb =
        prior_rotation.cast<T>() * expSO3(delta_rot);
    const Eigen::Matrix<T, 3, 1> t_wb =
        prior_translation.cast<T>() + delta_trans;

    const Eigen::Matrix<T, 3, 3> R_wc = R_wb * body_R_camera.cast<T>();
    const Eigen::Matrix<T, 3, 1> t_wc =
        t_wb + R_wb * body_t_camera.cast<T>();
    const Eigen::Matrix<T, 3, 1> point_camera =
        R_wc.transpose() * (landmark_world.cast<T>() - t_wc);

    if (point_camera.x() <= T(1e-6)) {
      residuals[0] = T(0);
      residuals[1] = T(0);
      return true;
    }

    const T predicted_u =
        T(camera.fx) * point_camera.y() / point_camera.x() + T(camera.cx);
    const T predicted_v =
        T(camera.fy) * point_camera.z() / point_camera.x() + T(camera.cy);

    residuals[0] = (predicted_u - T(pixel.x())) / T(pixel_sigma);
    residuals[1] = (predicted_v - T(pixel.y())) / T(pixel_sigma);
    return true;
  }

  Eigen::Matrix3d prior_rotation;
  Eigen::Vector3d prior_translation;
  CameraIntrinsics camera;
  Eigen::Matrix3d body_R_camera;
  Eigen::Vector3d body_t_camera;
  Eigen::Vector3d landmark_world;
  Eigen::Vector2d pixel;
  double pixel_sigma = 1.0;
};

}  // namespace

VILENS::VILENS(const VILENSParams& params)
    : params_(params),
      filter_(params.filter),
      map_cloud_(new pcl::PointCloud<pcl::PointXYZ>) {}

void VILENS::setLandmarks(const std::vector<Landmark>& landmarks) {
  landmarks_ = landmarks;
  landmark_lookup_.clear();
  landmark_lookup_.reserve(landmarks_.size());
  for (size_t i = 0; i < landmarks_.size(); ++i) {
    landmark_lookup_[landmarks_[i].id] = i;
  }
}

void VILENS::clearLandmarks() {
  landmarks_.clear();
  landmark_lookup_.clear();
}

std::vector<Eigen::Vector3d> VILENS::preprocess(
    const std::vector<Eigen::Vector3d>& raw_points, double voxel_size) const {
  std::vector<Eigen::Vector3d> filtered;
  if (raw_points.empty() || voxel_size <= 0.0) {
    return filtered;
  }

  filtered.reserve(raw_points.size());
  for (const auto& point : raw_points) {
    if (!point.allFinite()) {
      continue;
    }
    const double range = point.norm();
    if (range < params_.min_range || range > params_.max_range) {
      continue;
    }
    filtered.push_back(point);
  }

  std::unordered_map<Eigen::Vector3i, Eigen::Vector3d, VoxelHash> voxels;
  voxels.reserve(filtered.size());
  for (const auto& point : filtered) {
    const Eigen::Vector3i key(
        static_cast<int>(std::floor(point.x() / voxel_size)),
        static_cast<int>(std::floor(point.y() / voxel_size)),
        static_cast<int>(std::floor(point.z() / voxel_size)));
    voxels.emplace(key, point);
  }

  std::vector<Eigen::Vector3d> downsampled;
  downsampled.reserve(voxels.size());
  for (const auto& [key, point] : voxels) {
    (void)key;
    downsampled.push_back(point);
  }
  return downsampled;
}

void VILENS::propagateImu(
    const std::vector<imu_preintegration::ImuSample>& imu_samples) {
  if (imu_samples.size() < 2) {
    return;
  }

  for (size_t i = 1; i < imu_samples.size(); ++i) {
    const double dt = imu_samples[i].timestamp - imu_samples[i - 1].timestamp;
    if (dt <= 0.0 || dt > 0.1) {
      continue;
    }

    relead::ImuMeasurement imu;
    imu.timestamp = imu_samples[i].timestamp;
    imu.angular_velocity = 0.5 * (imu_samples[i - 1].gyro + imu_samples[i].gyro);
    imu.linear_acceleration =
        0.5 * (imu_samples[i - 1].accel + imu_samples[i].accel);
    filter_.predict(imu, dt);
  }
}

std::vector<Eigen::Vector3d> VILENS::transformPoints(
    const std::vector<Eigen::Vector3d>& points, const relead::State& state) {
  std::vector<Eigen::Vector3d> transformed(points.size());
  for (size_t i = 0; i < points.size(); ++i) {
    transformed[i] = state.rotation * points[i] + state.position;
  }
  return transformed;
}

double VILENS::rotationAngle(const Eigen::Matrix3d& rotation) {
  const Eigen::Quaterniond quat(rotation);
  const double clamped = std::clamp(std::abs(quat.normalized().w()), 0.0, 1.0);
  return 2.0 * std::acos(clamped);
}

bool VILENS::shouldAddKeyframe(const relead::State& candidate_state) const {
  const double translation =
      (candidate_state.position - last_keyframe_state_.position).norm();
  const double rotation =
      rotationAngle(last_keyframe_state_.rotation.transpose() *
                    candidate_state.rotation);
  return translation >= params_.keyframe_translation_threshold ||
         rotation >= params_.keyframe_rotation_threshold_rad;
}

void VILENS::rebuildMap() {
  map_points_.clear();
  size_t total_points = 0;
  for (const auto& keyframe : keyframes_) {
    total_points += keyframe.size();
  }

  map_points_.reserve(total_points);
  for (const auto& keyframe : keyframes_) {
    map_points_.insert(map_points_.end(), keyframe.begin(), keyframe.end());
  }

  map_cloud_->clear();
  map_cloud_->reserve(map_points_.size());
  for (const auto& point : map_points_) {
    map_cloud_->push_back(toPoint(point));
  }
}

void VILENS::addKeyframe(const std::vector<Eigen::Vector3d>& points_world,
                         const relead::State& state) {
  keyframes_.push_back(points_world);
  while (static_cast<int>(keyframes_.size()) > params_.max_keyframes_in_map) {
    keyframes_.pop_front();
  }
  last_keyframe_state_ = state;
  rebuildMap();
}

std::vector<relead::PointWithNormal> VILENS::buildCorrespondences(
    const std::vector<Eigen::Vector3d>& points_world) const {
  std::vector<relead::PointWithNormal> correspondences;
  if (map_cloud_->size() < static_cast<size_t>(params_.knn)) {
    return correspondences;
  }

  pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;
  kdtree.setInputCloud(map_cloud_);

  const double max_distance_sq =
      params_.max_correspondence_distance * params_.max_correspondence_distance;

  std::vector<int> point_search_indices;
  std::vector<float> point_search_distances;
  correspondences.reserve(points_world.size());

  const size_t stride = std::max<size_t>(
      1, points_world.size() /
             static_cast<size_t>(std::max(1, params_.max_correspondences)));
  for (size_t point_idx = 0; point_idx < points_world.size(); point_idx += stride) {
    const auto& point_world = points_world[point_idx];
    const pcl::PointXYZ query = toPoint(point_world);
    point_search_indices.clear();
    point_search_distances.clear();

    kdtree.nearestKSearch(query, params_.knn, point_search_indices,
                          point_search_distances);
    if (point_search_distances.size() < static_cast<size_t>(params_.knn) ||
        point_search_distances.back() > max_distance_sq) {
      continue;
    }

    Eigen::Vector3d center = Eigen::Vector3d::Zero();
    for (int idx : point_search_indices) {
      center += map_points_[static_cast<size_t>(idx)];
    }
    center /= static_cast<double>(point_search_indices.size());

    Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
    for (int idx : point_search_indices) {
      const Eigen::Vector3d diff = map_points_[static_cast<size_t>(idx)] - center;
      covariance += diff * diff.transpose();
    }
    covariance /= static_cast<double>(point_search_indices.size());

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(covariance);
    if (solver.info() != Eigen::Success) {
      continue;
    }

    const Eigen::Vector3d eigenvalues = solver.eigenvalues();
    if (eigenvalues(2) < 1e-10) {
      continue;
    }

    const double planarity =
        (eigenvalues(1) - eigenvalues(0)) / eigenvalues(2);
    if (planarity < params_.planarity_threshold) {
      continue;
    }

    Eigen::Vector3d normal = solver.eigenvectors().col(0).normalized();
    if (normal.dot(point_world - center) < 0.0) {
      normal = -normal;
    }

    relead::PointWithNormal corr;
    corr.point = point_world;
    corr.normal = normal;
    corr.closest = center;
    correspondences.push_back(corr);
    if (static_cast<int>(correspondences.size()) >= params_.max_correspondences) {
      break;
    }
  }

  return correspondences;
}

bool VILENS::projectPoint(const relead::State& state,
                          const Eigen::Vector3d& landmark_world,
                          Eigen::Vector2d* pixel) const {
  const Eigen::Matrix3d R_wc = state.rotation * params_.body_R_camera;
  const Eigen::Vector3d t_wc =
      state.position + state.rotation * params_.body_t_camera;
  const Eigen::Vector3d point_camera =
      R_wc.transpose() * (landmark_world - t_wc);

  if (point_camera.x() <= 1e-6) {
    return false;
  }

  const double u =
      params_.camera.fx * point_camera.y() / point_camera.x() + params_.camera.cx;
  const double v =
      params_.camera.fy * point_camera.z() / point_camera.x() + params_.camera.cy;
  if (u < 0.0 || u >= params_.camera.width ||
      v < 0.0 || v >= params_.camera.height) {
    return false;
  }

  if (pixel != nullptr) {
    pixel->x() = u;
    pixel->y() = v;
  }
  return true;
}

VILENS::PoseOptimizationSummary VILENS::refineWithVision(
    const relead::State& prior_state,
    const std::vector<VisualObservation>& visual_observations) const {
  PoseOptimizationSummary summary;
  summary.state = prior_state;

  struct MatchedObservation {
    Eigen::Vector2d pixel;
    Eigen::Vector3d landmark_world;
  };

  std::vector<MatchedObservation> matched;
  matched.reserve(visual_observations.size());
  for (const auto& observation : visual_observations) {
    const auto it = landmark_lookup_.find(observation.landmark_id);
    if (it == landmark_lookup_.end() || !observation.pixel.allFinite()) {
      continue;
    }

    Eigen::Vector2d predicted_pixel;
    if (!projectPoint(prior_state, landmarks_[it->second].position,
                      &predicted_pixel)) {
      continue;
    }

    matched.push_back({observation.pixel, landmarks_[it->second].position});
  }

  if (static_cast<int>(matched.size()) < params_.min_visual_observations) {
    return summary;
  }

  double delta_rotation[3] = {0.0, 0.0, 0.0};
  double delta_translation[3] = {0.0, 0.0, 0.0};

  ceres::Problem problem;
  problem.AddParameterBlock(delta_rotation, 3);
  problem.AddParameterBlock(delta_translation, 3);

  auto* rotation_prior =
      new ceres::AutoDiffCostFunction<ZeroPriorResidual, 3, 3>(
          new ZeroPriorResidual(params_.lidar_prior_rotation_weight));
  problem.AddResidualBlock(rotation_prior, nullptr, delta_rotation);

  auto* translation_prior =
      new ceres::AutoDiffCostFunction<ZeroPriorResidual, 3, 3>(
          new ZeroPriorResidual(params_.lidar_prior_translation_weight));
  problem.AddResidualBlock(translation_prior, nullptr, delta_translation);

  const double huber_delta =
      params_.visual_huber_delta /
      std::max(1e-6, params_.visual_pixel_sigma);
  for (const auto& observation : matched) {
    auto* residual =
        new ceres::AutoDiffCostFunction<VisualReprojectionResidual, 2, 3, 3>(
            new VisualReprojectionResidual(
                prior_state.rotation, prior_state.position, params_.camera,
                params_.body_R_camera, params_.body_t_camera,
                observation.landmark_world, observation.pixel,
                params_.visual_pixel_sigma));
    problem.AddResidualBlock(residual, new ceres::HuberLoss(huber_delta),
                             delta_rotation, delta_translation);
  }

  ceres::Solver::Options options;
  options.max_num_iterations = params_.max_visual_iterations;
  options.linear_solver_type = ceres::DENSE_QR;
  options.minimizer_progress_to_stdout = false;

  ceres::Solver::Summary solver_summary;
  ceres::Solve(options, &problem, &solver_summary);

  summary.state = prior_state;
  summary.state.rotation =
      prior_state.rotation *
      expSO3d(Eigen::Vector3d(delta_rotation[0], delta_rotation[1],
                              delta_rotation[2]));
  summary.state.position =
      prior_state.position +
      Eigen::Vector3d(delta_translation[0], delta_translation[1],
                      delta_translation[2]);
  summary.visual_used = true;
  summary.num_factors = static_cast<int>(matched.size());
  summary.converged = solver_summary.termination_type != ceres::FAILURE;
  return summary;
}

VILENSResult VILENS::process(
    const std::vector<Eigen::Vector3d>& raw_points,
    const std::vector<VisualObservation>& visual_observations,
    const std::vector<imu_preintegration::ImuSample>& imu_samples) {
  VILENSResult result;
  result.state = filter_.state();
  result.initialized = initialized_;
  result.num_keyframes = numKeyframes();
  result.map_points = mapSize();
  result.imu_used = imu_samples.size() >= 2;

  const std::vector<Eigen::Vector3d> registration_points =
      preprocess(raw_points, params_.registration_voxel_size);
  if (registration_points.empty()) {
    return result;
  }

  const std::vector<Eigen::Vector3d> map_points =
      preprocess(raw_points, params_.map_voxel_size);
  if (map_points.empty()) {
    return result;
  }

  propagateImu(imu_samples);

  if (!initialized_) {
    const PoseOptimizationSummary visual_summary =
        refineWithVision(filter_.state(), visual_observations);
    if (visual_summary.visual_used) {
      filter_.state() = visual_summary.state;
    }

    addKeyframe(transformPoints(map_points, filter_.state()), filter_.state());
    initialized_ = true;

    result.state = filter_.state();
    result.initialized = true;
    result.valid = true;
    result.converged = !visual_summary.visual_used || visual_summary.converged;
    result.keyframe_added = true;
    result.visual_used = visual_summary.visual_used;
    result.num_visual_factors = visual_summary.num_factors;
    result.num_keyframes = numKeyframes();
    result.map_points = mapSize();
    return result;
  }

  const std::vector<Eigen::Vector3d> points_world =
      transformPoints(registration_points, filter_.state());
  const std::vector<relead::PointWithNormal> correspondences =
      buildCorrespondences(points_world);
  result.num_correspondences = static_cast<int>(correspondences.size());

  relead::State fused_state = filter_.state();
  bool lidar_valid = false;
  bool lidar_converged = false;
  if (!correspondences.empty()) {
    const relead::CESIKFResult update_result = filter_.update(correspondences);
    fused_state = update_result.state;
    result.degeneracy_info = update_result.degeneracy_info;
    result.num_iterations = update_result.iterations;
    lidar_valid = true;
    lidar_converged = update_result.converged;
  }

  const PoseOptimizationSummary visual_summary =
      refineWithVision(fused_state, visual_observations);
  if (visual_summary.visual_used) {
    fused_state = visual_summary.state;
  }
  filter_.state() = fused_state;

  result.state = filter_.state();
  result.initialized = true;
  result.valid = lidar_valid || visual_summary.visual_used;
  result.converged = lidar_converged || visual_summary.converged;
  result.visual_used = visual_summary.visual_used;
  result.num_visual_factors = visual_summary.num_factors;

  if (shouldAddKeyframe(filter_.state())) {
    addKeyframe(transformPoints(map_points, filter_.state()), filter_.state());
    result.keyframe_added = true;
  }

  result.num_keyframes = numKeyframes();
  result.map_points = mapSize();
  return result;
}

void VILENS::clear() {
  filter_ = relead::CESIKF(params_.filter);
  keyframes_.clear();
  map_points_.clear();
  map_cloud_->clear();
  last_keyframe_state_ = relead::State();
  initialized_ = false;
}

}  // namespace vilens
}  // namespace localization_zoo
