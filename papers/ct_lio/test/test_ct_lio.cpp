#include "ct_lio/cost_functions.h"

#include <ceres/ceres.h>
#include "ct_lio/ct_lio_registration.h"

#include <gtest/gtest.h>

#include <random>
#include <common/ceres_compat.h>

using namespace localization_zoo::ct_lio;

namespace {

std::vector<Eigen::Vector3d> generateEnv(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> points;
  std::normal_distribution<double> noise(0.0, 0.01);
  std::uniform_real_distribution<double> xy(-30.0, 30.0);
  std::uniform_real_distribution<double> z(0.0, 4.0);

  for (int i = 0; i < 2500; ++i) {
    points.emplace_back(xy(rng), xy(rng), noise(rng));
  }
  for (int i = 0; i < 1000; ++i) {
    points.emplace_back(xy(rng), 15.0 + noise(rng), z(rng));
  }
  for (int i = 0; i < 1000; ++i) {
    points.emplace_back(-15.0 + noise(rng), xy(rng), z(rng));
  }
  return points;
}

std::vector<localization_zoo::ct_icp::TimedPoint> simulateScan(
    const std::vector<Eigen::Vector3d>& env,
    const localization_zoo::ct_icp::TrajectoryFrame& frame,
    std::mt19937& rng) {
  std::vector<localization_zoo::ct_icp::TimedPoint> timed_points;
  std::normal_distribution<double> noise(0.0, 0.02);

  for (const auto& world_point : env) {
    double timestamp = std::uniform_real_distribution<double>(0.0, 1.0)(rng);
    localization_zoo::ct_icp::SE3 pose = localization_zoo::ct_icp::SE3::interpolate(
        frame.begin_pose, frame.end_pose, frame.getAlpha(timestamp));
    Eigen::Vector3d sensor_point = pose.inverse() * world_point;
    if (sensor_point.norm() > 30.0 || sensor_point.norm() < 0.5) {
      continue;
    }

    localization_zoo::ct_icp::TimedPoint point;
    point.raw_point = sensor_point +
                      Eigen::Vector3d(noise(rng), noise(rng), noise(rng));
    point.timestamp = timestamp;
    timed_points.push_back(point);
    if (timed_points.size() > 2500) {
      break;
    }
  }

  return timed_points;
}

std::vector<localization_zoo::imu_preintegration::ImuSample> stationaryGravityImu(
    double duration, double dt) {
  std::vector<localization_zoo::imu_preintegration::ImuSample> samples;
  for (int i = 0; i <= static_cast<int>(duration / dt); ++i) {
    localization_zoo::imu_preintegration::ImuSample sample;
    sample.timestamp = i * dt;
    sample.gyro = Eigen::Vector3d::Zero();
    sample.accel = Eigen::Vector3d(0.0, 0.0, 9.81);
    samples.push_back(sample);
  }
  return samples;
}

std::vector<localization_zoo::imu_preintegration::ImuSample> biasedStationaryGravityImu(
    double duration, double dt, const Eigen::Vector3d& gyro_bias,
    const Eigen::Vector3d& accel_bias) {
  auto samples = stationaryGravityImu(duration, dt);
  for (auto& sample : samples) {
    sample.gyro += gyro_bias;
    sample.accel += accel_bias;
  }
  return samples;
}

}  // namespace

TEST(CTLIO, PredictsMotionFromImuWithoutMap) {
  CTLIORegistration registration;
  CTLIOState initial_state;
  initial_state.frame.begin_pose.trans = Eigen::Vector3d(0.0, 0.0, 1.0);
  initial_state.frame.end_pose = initial_state.frame.begin_pose;
  initial_state.begin_velocity = Eigen::Vector3d(1.0, 0.0, 0.0);

  auto imu_samples = stationaryGravityImu(1.0, 0.01);
  auto result = registration.registerFrame({}, initial_state, imu_samples);

  EXPECT_TRUE(result.converged);
  EXPECT_NEAR(result.preintegration.delta_t, 1.0, 1e-6);
  EXPECT_NEAR(result.state.frame.end_pose.trans.x(), 1.0, 0.05);
  EXPECT_NEAR(result.state.frame.end_pose.trans.y(), 0.0, 0.05);
  EXPECT_NEAR(result.state.frame.end_pose.trans.z(), 1.0, 0.05);
  EXPECT_NEAR(result.state.end_velocity.x(), 1.0, 0.05);
}

TEST(CTLIO, RegistersFrameWithMapAndImuPrior) {
  std::mt19937 rng(42);
  auto env = generateEnv(rng);

  CTLIOParams params;
  params.voxel_resolution = 1.0;
  params.max_iterations = 12;
  params.ceres_max_iterations = 4;
  params.planarity_threshold = 0.1;
  CTLIORegistration registration(params);

  localization_zoo::ct_icp::TrajectoryFrame frame0;
  frame0.begin_pose.trans = Eigen::Vector3d(0.0, 0.0, 1.0);
  frame0.end_pose.trans = Eigen::Vector3d(0.0, 0.0, 1.0);
  auto scan0 = simulateScan(env, frame0, rng);

  std::vector<Eigen::Vector3d> map_points;
  map_points.reserve(scan0.size());
  for (const auto& point : scan0) {
    map_points.push_back(frame0.transformPoint(point.raw_point, point.timestamp));
  }
  registration.addPointsToMap(map_points);

  localization_zoo::ct_icp::TrajectoryFrame gt_frame;
  gt_frame.begin_pose.trans = Eigen::Vector3d(0.0, 0.0, 1.0);
  gt_frame.end_pose.trans = Eigen::Vector3d(1.0, 0.0, 1.0);
  auto scan1 = simulateScan(env, gt_frame, rng);

  CTLIOState initial_state;
  initial_state.frame.begin_pose = frame0.end_pose;
  initial_state.frame.end_pose = frame0.end_pose;
  initial_state.begin_velocity = Eigen::Vector3d(1.0, 0.0, 0.0);

  auto imu_samples = stationaryGravityImu(1.0, 0.01);
  auto result = registration.registerFrame(scan1, initial_state, imu_samples);

  EXPECT_GT(result.num_correspondences, 0);
  EXPECT_TRUE(result.converged || result.num_iterations > 0);
  EXPECT_LT((result.state.frame.end_pose.trans - gt_frame.end_pose.trans).norm(),
            1.5);
  EXPECT_NEAR(result.state.end_velocity.x(), 1.0, 0.1);
}

TEST(CTLIO, SlidingWindowMap) {
  CTLIOParams params;
  params.max_frames_in_map = 2;
  CTLIORegistration registration(params);

  std::vector<Eigen::Vector3d> frame0;
  std::vector<Eigen::Vector3d> frame1;
  std::vector<Eigen::Vector3d> frame2;
  for (int i = 0; i < 10; ++i) {
    frame0.emplace_back(0.01 * i, 0.0, 0.0);
    frame1.emplace_back(10.0 + 0.01 * i, 0.0, 0.0);
    frame2.emplace_back(20.0 + 0.01 * i, 0.0, 0.0);
  }

  registration.addPointsToMap(frame0);
  registration.addPointsToMap(frame1);
  EXPECT_EQ(registration.mapSize(), 2u);

  registration.addPointsToMap(frame2);
  EXPECT_EQ(registration.mapSize(), 2u);
  EXPECT_EQ(registration.map().find(localization_zoo::ct_icp::Voxel{0, 0, 0}),
            registration.map().end());
  EXPECT_NE(
      registration.map().find(localization_zoo::ct_icp::Voxel{10, 0, 0}),
      registration.map().end());
  EXPECT_NE(
      registration.map().find(localization_zoo::ct_icp::Voxel{20, 0, 0}),
      registration.map().end());
}

TEST(CTLIO, EstimatesBiasWhileKeepingPoseConsistentWithMap) {
  std::mt19937 rng(7);
  auto env = generateEnv(rng);

  CTLIOParams params;
  params.voxel_resolution = 1.0;
  params.max_iterations = 12;
  params.ceres_max_iterations = 4;
  params.planarity_threshold = 0.1;
  params.imu_rotation_weight = 10.0;
  params.imu_translation_weight = 10.0;
  params.gyro_bias_prior_weight = 1e-3;
  params.accel_bias_prior_weight = 1e-3;
  params.velocity_prior_weight = 0.1;
  params.estimate_imu_bias = true;
  CTLIORegistration registration(params);

  localization_zoo::ct_icp::TrajectoryFrame frame0;
  frame0.begin_pose.trans = Eigen::Vector3d(0.0, 0.0, 1.0);
  frame0.end_pose.trans = Eigen::Vector3d(0.0, 0.0, 1.0);
  auto scan0 = simulateScan(env, frame0, rng);

  std::vector<Eigen::Vector3d> map_points;
  map_points.reserve(scan0.size());
  for (const auto& point : scan0) {
    map_points.push_back(frame0.transformPoint(point.raw_point, point.timestamp));
  }
  registration.addPointsToMap(map_points);

  auto scan1 = simulateScan(env, frame0, rng);

  CTLIOState initial_state;
  initial_state.frame.begin_pose = frame0.end_pose;
  initial_state.frame.end_pose = frame0.end_pose;
  initial_state.begin_velocity = Eigen::Vector3d::Zero();

  const Eigen::Vector3d gyro_bias(0.0, 0.0, 0.02);
  const Eigen::Vector3d accel_bias(0.25, -0.10, 0.0);
  auto imu_samples =
      biasedStationaryGravityImu(1.0, 0.01, gyro_bias, accel_bias);

  auto result = registration.registerFrame(scan1, initial_state, imu_samples);

  EXPECT_GT(result.num_correspondences, 0);
  EXPECT_LT((result.state.frame.end_pose.trans - frame0.end_pose.trans).norm(),
            0.25);
  EXPECT_LT(result.state.frame.end_pose.quat.angularDistance(frame0.end_pose.quat),
            0.05);
  EXPECT_GT(result.state.gyro_bias.norm() + result.state.accel_bias.norm(), 0.01);
}

TEST(CTLIO, BiasResidualsRecoverStationaryImuBias) {
  using namespace localization_zoo::imu_preintegration;

  const Eigen::Vector3d gyro_bias_true(0.0, 0.0, 0.02);
  const Eigen::Vector3d accel_bias_true(0.08, -0.03, 0.0);
  auto imu_samples =
      biasedStationaryGravityImu(1.0, 0.01, gyro_bias_true, accel_bias_true);

  ImuPreintegrator preintegrator;
  preintegrator.integrateBatch(imu_samples);
  const auto& preintegration = preintegrator.result();

  double begin_q[4] = {0.0, 0.0, 0.0, 1.0};
  double end_q[4] = {0.0, 0.0, 0.0, 1.0};
  double begin_t[3] = {0.0, 0.0, 0.0};
  double end_t[3] = {0.0, 0.0, 0.0};
  double begin_v[3] = {0.0, 0.0, 0.0};
  double gyro_bias[3] = {0.0, 0.0, 0.0};
  double accel_bias[3] = {0.0, 0.0, 0.0};

  ceres::Problem problem;
  problem.AddParameterBlock(
      begin_q, 4);
    localization_zoo::SetEigenQuaternionManifold(problem, begin_q);
  problem.AddParameterBlock(
      end_q, 4);
    localization_zoo::SetEigenQuaternionManifold(problem, end_q);
  problem.AddParameterBlock(begin_t, 3);
  problem.AddParameterBlock(end_t, 3);
  problem.AddParameterBlock(begin_v, 3);
  problem.AddParameterBlock(gyro_bias, 3);
  problem.AddParameterBlock(accel_bias, 3);

  problem.SetParameterBlockConstant(begin_q);
  problem.SetParameterBlockConstant(end_q);
  problem.SetParameterBlockConstant(begin_t);
  problem.SetParameterBlockConstant(end_t);
  problem.SetParameterBlockConstant(begin_v);

  problem.AddResidualBlock(
      IMURotationBiasPrior::Create(preintegration, Eigen::Vector3d::Zero(), 1.0),
      nullptr, begin_q, end_q, gyro_bias);
  problem.AddResidualBlock(
      IMUTranslationBiasPrior::Create(
          preintegration, Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(),
          preintegration.delta_t, Eigen::Vector3d(0.0, 0.0, -9.81), 1.0),
      nullptr, begin_q, begin_t, end_t, begin_v, gyro_bias, accel_bias);

  ceres::Solver::Options options;
  options.linear_solver_type = ceres::DENSE_QR;
  options.max_num_iterations = 20;
  options.minimizer_progress_to_stdout = false;

  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);

  Eigen::Map<Eigen::Vector3d> estimated_gyro_bias(gyro_bias);
  Eigen::Map<Eigen::Vector3d> estimated_accel_bias(accel_bias);

  EXPECT_GT(estimated_gyro_bias.z(), 0.01);
  EXPECT_GT(estimated_accel_bias.x(), 0.03);
  EXPECT_LT(estimated_accel_bias.y(), -0.01);
  EXPECT_NEAR(estimated_accel_bias.z(), 0.0, 0.05);
}

TEST(CTLIO, CarriesBiasAcrossFramesWithRandomWalkPrior) {
  std::mt19937 rng(11);
  auto env = generateEnv(rng);

  CTLIOParams params;
  params.voxel_resolution = 1.0;
  params.max_iterations = 12;
  params.ceres_max_iterations = 4;
  params.planarity_threshold = 0.1;
  params.imu_rotation_weight = 8.0;
  params.imu_translation_weight = 8.0;
  params.gyro_bias_prior_weight = 0.02;
  params.accel_bias_prior_weight = 0.02;
  params.velocity_prior_weight = 0.1;
  params.estimate_imu_bias = true;
  params.fixed_lag_state_window = 3;
  params.fixed_lag_velocity_prior_weight = 0.05;
  params.fixed_lag_gyro_bias_weight_scale = 0.5;
  params.fixed_lag_accel_bias_weight_scale = 0.5;
  params.fixed_lag_optimize_history = true;
  CTLIORegistration registration(params);

  localization_zoo::ct_icp::TrajectoryFrame frame0;
  frame0.begin_pose.trans = Eigen::Vector3d(0.0, 0.0, 1.0);
  frame0.end_pose.trans = Eigen::Vector3d(0.0, 0.0, 1.0);
  auto scan0 = simulateScan(env, frame0, rng);

  std::vector<Eigen::Vector3d> map_points;
  map_points.reserve(scan0.size());
  for (const auto& point : scan0) {
    map_points.push_back(frame0.transformPoint(point.raw_point, point.timestamp));
  }
  registration.addPointsToMap(map_points);

  auto scan1 = simulateScan(env, frame0, rng);
  auto scan2 = simulateScan(env, frame0, rng);

  const Eigen::Vector3d gyro_bias(0.0, 0.0, 0.02);
  const Eigen::Vector3d accel_bias(0.10, -0.04, 0.0);
  auto imu_samples =
      biasedStationaryGravityImu(1.0, 0.01, gyro_bias, accel_bias);

  CTLIOState initial_state;
  initial_state.frame.begin_pose = frame0.end_pose;
  initial_state.frame.end_pose = frame0.end_pose;
  initial_state.begin_velocity = Eigen::Vector3d::Zero();

  auto result1 = registration.registerFrame(scan1, initial_state, imu_samples);
  auto result2 = registration.registerFrame(scan2, result1.state, imu_samples);

  EXPECT_EQ(registration.stateHistorySize(), 2u);
  EXPECT_GT(result1.state.gyro_bias.norm() + result1.state.accel_bias.norm(), 0.01);
  EXPECT_GT(result2.state.gyro_bias.norm() + result2.state.accel_bias.norm(), 0.01);
  EXPECT_LT((result2.state.frame.end_pose.trans - frame0.end_pose.trans).norm(),
            0.25);
  EXPECT_LT(result2.state.frame.end_pose.quat.angularDistance(frame0.end_pose.quat),
            0.05);
}
