#include "fast_livo2/fast_livo2.h"

#include <ceres/ceres.h>

#include <cmath>

namespace localization_zoo {
namespace fast_livo2 {

namespace {

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
  VisualReprojectionResidual(const Eigen::Quaterniond& prior_quat,
                             const Eigen::Vector3d& prior_trans,
                             const CameraIntrinsics& camera,
                             const Eigen::Matrix3d& body_R_camera,
                             const Eigen::Vector3d& body_t_camera,
                             const Eigen::Vector3d& landmark_world,
                             const Eigen::Vector2d& pixel,
                             double pixel_sigma)
      : prior_quat(prior_quat),
        prior_trans(prior_trans),
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

    const Eigen::Quaternion<T> q_prior(T(prior_quat.w()), T(prior_quat.x()),
                                       T(prior_quat.y()), T(prior_quat.z()));
    const Eigen::Quaternion<T> q_wb(
        q_prior * Eigen::Quaternion<T>(expSO3(delta_rot)));
    const Eigen::Matrix<T, 3, 1> t_wb = prior_trans.cast<T>() + delta_trans;

    const Eigen::Quaternion<T> q_bc(
        T(Eigen::Quaterniond(body_R_camera).w()),
        T(Eigen::Quaterniond(body_R_camera).x()),
        T(Eigen::Quaterniond(body_R_camera).y()),
        T(Eigen::Quaterniond(body_R_camera).z()));
    const Eigen::Quaternion<T> q_wc = q_wb * q_bc;
    const Eigen::Matrix<T, 3, 1> t_wc =
        t_wb + q_wb * body_t_camera.cast<T>();
    const Eigen::Matrix<T, 3, 1> p_c =
        q_wc.conjugate() * (landmark_world.cast<T>() - t_wc);

    if (p_c.x() <= T(1e-6)) {
      residuals[0] = T(0);
      residuals[1] = T(0);
      return true;
    }

    const T u = T(camera.fx) * p_c.y() / p_c.x() + T(camera.cx);
    const T v = T(camera.fy) * p_c.z() / p_c.x() + T(camera.cy);
    residuals[0] = (u - T(pixel.x())) / T(pixel_sigma);
    residuals[1] = (v - T(pixel.y())) / T(pixel_sigma);
    return true;
  }

  Eigen::Quaterniond prior_quat;
  Eigen::Vector3d prior_trans;
  CameraIntrinsics camera;
  Eigen::Matrix3d body_R_camera;
  Eigen::Vector3d body_t_camera;
  Eigen::Vector3d landmark_world;
  Eigen::Vector2d pixel;
  double pixel_sigma = 1.0;
};

}  // namespace

FastLivo2::FastLivo2(const FastLivo2Params& params)
    : params_(params), front_end_(params.front_end) {}

void FastLivo2::setLandmarks(const std::vector<Landmark>& landmarks) {
  landmarks_ = landmarks;
  landmark_lookup_.clear();
  landmark_lookup_.reserve(landmarks.size());
  for (size_t i = 0; i < landmarks.size(); ++i) {
    landmark_lookup_[landmarks[i].id] = i;
  }
}

void FastLivo2::clearLandmarks() {
  landmarks_.clear();
  landmark_lookup_.clear();
}

bool FastLivo2::projectLandmark(const fast_lio2::FastLio2State& state,
                                const Eigen::Vector3d& landmark_world,
                                Eigen::Vector2d* pixel) const {
  const Eigen::Quaterniond q_wc =
      state.quat * Eigen::Quaterniond(params_.body_R_camera);
  const Eigen::Vector3d t_wc = state.trans + state.quat * params_.body_t_camera;
  const Eigen::Vector3d p_c = q_wc.conjugate() * (landmark_world - t_wc);
  if (p_c.x() <= 1e-6) {
    return false;
  }

  const double u = params_.camera.fx * p_c.y() / p_c.x() + params_.camera.cx;
  const double v = params_.camera.fy * p_c.z() / p_c.x() + params_.camera.cy;
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

FastLivo2::PoseOptimizationSummary FastLivo2::refineWithVision(
    const fast_lio2::FastLio2State& prior_state,
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
    if (!projectLandmark(prior_state, landmarks_[it->second].position,
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

  problem.AddResidualBlock(
      new ceres::AutoDiffCostFunction<ZeroPriorResidual, 3, 3>(
          new ZeroPriorResidual(params_.lidar_prior_rotation_weight)),
      nullptr, delta_rotation);
  problem.AddResidualBlock(
      new ceres::AutoDiffCostFunction<ZeroPriorResidual, 3, 3>(
          new ZeroPriorResidual(params_.lidar_prior_translation_weight)),
      nullptr, delta_translation);

  const double huber_delta =
      params_.visual_huber_delta /
      std::max(1e-6, params_.visual_pixel_sigma);
  for (const auto& observation : matched) {
    problem.AddResidualBlock(
        new ceres::AutoDiffCostFunction<VisualReprojectionResidual, 2, 3, 3>(
            new VisualReprojectionResidual(
                prior_state.quat, prior_state.trans, params_.camera,
                params_.body_R_camera, params_.body_t_camera,
                observation.landmark_world, observation.pixel,
                params_.visual_pixel_sigma)),
        new ceres::HuberLoss(huber_delta), delta_rotation, delta_translation);
  }

  ceres::Solver::Options options;
  options.max_num_iterations = params_.max_visual_iterations;
  options.linear_solver_type = ceres::DENSE_QR;
  options.minimizer_progress_to_stdout = false;

  ceres::Solver::Summary solver_summary;
  ceres::Solve(options, &problem, &solver_summary);

  summary.state = prior_state;
  summary.state.quat =
      (prior_state.quat *
       Eigen::Quaterniond(expSO3d(Eigen::Vector3d(
           delta_rotation[0], delta_rotation[1], delta_rotation[2]))))
          .normalized();
  summary.state.trans =
      prior_state.trans +
      Eigen::Vector3d(delta_translation[0], delta_translation[1],
                      delta_translation[2]);
  summary.visual_used = true;
  summary.converged = solver_summary.termination_type != ceres::FAILURE;
  summary.num_factors = static_cast<int>(matched.size());
  return summary;
}

FastLivo2Result FastLivo2::process(
    const std::vector<Eigen::Vector3d>& raw_points,
    const std::vector<VisualObservation>& visual_observations,
    const std::vector<imu_preintegration::ImuSample>& imu_samples) {
  FastLivo2Result result;
  const fast_lio2::FastLio2Result front_end_result =
      front_end_.process(raw_points, imu_samples);
  result.front_end_state = front_end_result.state;
  result.state = front_end_result.state;
  result.initialized = front_end_result.initialized;
  result.valid = front_end_result.valid;
  result.converged = front_end_result.converged;
  result.imu_used = front_end_result.imu_used;
  result.map_size = front_end_result.map_size;

  if (!front_end_result.initialized) {
    return result;
  }

  const PoseOptimizationSummary visual_summary =
      refineWithVision(front_end_result.state, visual_observations);
  if (visual_summary.visual_used) {
    front_end_.state() = visual_summary.state;
    result.state = visual_summary.state;
    result.valid = front_end_result.valid || visual_summary.visual_used;
    result.converged = front_end_result.converged || visual_summary.converged;
    result.visual_used = true;
    result.num_visual_factors = visual_summary.num_factors;
  }

  result.map_size = front_end_.mapSize();
  return result;
}

void FastLivo2::clear() {
  front_end_.clear();
}

}  // namespace fast_livo2
}  // namespace localization_zoo
