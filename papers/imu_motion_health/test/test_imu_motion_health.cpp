#include "imu_motion_health/imu_motion_health.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <string>

using localization_zoo::imu_motion_health::HealthState;
using localization_zoo::imu_motion_health::ImuMotionHealth;
using localization_zoo::imu_motion_health::ImuMotionHealthParams;
using localization_zoo::imu_motion_health::ImuSample;
using localization_zoo::imu_motion_health::MotionState;

namespace {

constexpr double kGravity = 9.80665;
constexpr double kPi = 3.14159265358979323846;

ImuSample sample(double timestamp, const Eigen::Vector3d& gyro,
                 const Eigen::Vector3d& accel) {
  ImuSample result;
  result.timestamp = timestamp;
  result.gyro = gyro;
  result.accel = accel;
  return result;
}

void feedStatic(ImuMotionHealth* pipeline, int count, double dt = 0.01,
                double start = 0.0,
                const Eigen::Vector3d& gyro = Eigen::Vector3d::Zero()) {
  for (int i = 0; i < count; ++i) {
    pipeline->process(sample(start + i * dt, gyro,
                             Eigen::Vector3d(0.0, 0.0, kGravity)));
  }
}

ImuMotionHealthParams testParams() {
  ImuMotionHealthParams params;
  params.startup_duration_s = 0.20;
  params.startup_min_samples = 10;
  params.max_gap_s = 0.05;
  params.max_integration_dt_s = 0.05;
  params.motion_window_samples = 8;
  params.stationary_bias_gain = 0.0;
  params.leveling_gain = 0.0;
  return params;
}

}  // namespace

TEST(ImuMotionHealth, StaticStartupEstimatesBiasAndGravity) {
  ImuMotionHealthParams params = testParams();
  ImuMotionHealth pipeline(params);
  const Eigen::Vector3d gyro_bias(0.02, -0.01, 0.03);
  feedStatic(&pipeline, 31, 0.01, 0.0, gyro_bias);

  const auto state = pipeline.snapshot();
  EXPECT_TRUE(state.startup_complete);
  EXPECT_TRUE(state.startup_quality_ok);
  EXPECT_TRUE(state.startup_bias_trusted);
  EXPECT_EQ(state.motion_state, MotionState::kStationary);
  EXPECT_EQ(state.health_state, HealthState::kReady);
  EXPECT_NEAR(state.gravity_magnitude, kGravity, 1e-6);
  EXPECT_NEAR((state.gyro_bias - gyro_bias).norm(), 0.0, 1e-6);
  EXPECT_NEAR((state.orientation * Eigen::Vector3d(0.0, 0.0, kGravity) -
               Eigen::Vector3d(0.0, 0.0, kGravity))
                  .norm(),
              0.0, 1e-6);
  EXPECT_NEAR(state.position.norm(), 0.0, 1e-9);
  EXPECT_NEAR(state.velocity.norm(), 0.0, 1e-9);
  EXPECT_EQ(pipeline.counters().accepted_samples, 31u);
  EXPECT_EQ(pipeline.counters().stationary_samples, 10u);
  EXPECT_GT(state.confidence, 0.8);
}

TEST(ImuMotionHealth, NoisyStartupIsReportedAndBiasIsNotTrusted) {
  ImuMotionHealth pipeline(testParams());
  for (int i = 0; i < 31; ++i) {
    const double alternating_rate = i % 2 == 0 ? 0.30 : -0.30;
    pipeline.process(sample(i * 0.01,
                            Eigen::Vector3d(alternating_rate, 0.0, 0.0),
                            Eigen::Vector3d(0.0, 0.0, kGravity)));
  }

  const auto state = pipeline.snapshot();
  EXPECT_TRUE(state.startup_complete);
  EXPECT_FALSE(state.startup_quality_ok);
  EXPECT_FALSE(state.startup_bias_trusted);
  EXPECT_EQ(state.health_state, HealthState::kDegraded);
  EXPECT_NE(state.diagnostic.find("startup quality gate failed"),
            std::string::npos);
  EXPECT_NEAR(state.gyro_bias.norm(), 0.0, 1e-12);
  EXPECT_LT(state.confidence, 0.5);
}

TEST(ImuMotionHealth, DetectsDataQualityFaultsAndDoesNotIntegrateThem) {
  ImuMotionHealth pipeline(testParams());
  feedStatic(&pipeline, 31);

  ImuSample bad = sample(0.31, Eigen::Vector3d::Zero(),
                         Eigen::Vector3d::Constant(
                             std::numeric_limits<double>::quiet_NaN()));
  auto state = pipeline.process(bad);
  EXPECT_TRUE(state.nonfinite);
  EXPECT_EQ(state.health_state, HealthState::kInvalid);
  EXPECT_EQ(pipeline.counters().nonfinite_samples, 1u);

  state = pipeline.process(sample(0.30, Eigen::Vector3d::Zero(),
                                  Eigen::Vector3d(0.0, 0.0, kGravity)));
  EXPECT_TRUE(state.nonmonotonic);
  EXPECT_EQ(pipeline.counters().nonmonotonic_timestamps, 1u);

  state = pipeline.process(sample(0.40, Eigen::Vector3d::Zero(),
                                  Eigen::Vector3d(0.0, 0.0, kGravity)));
  EXPECT_TRUE(state.gap);
  EXPECT_EQ(pipeline.counters().timestamp_gaps, 1u);
  EXPECT_EQ(state.health_state, HealthState::kDegraded);
  EXPECT_FALSE(state.integrated);

  ImuMotionHealthParams saturated_params = testParams();
  saturated_params.gyro_saturation_rad_s = 1.0;
  saturated_params.accel_saturation_mps2 = 20.0;
  ImuMotionHealth saturated(saturated_params);
  feedStatic(&saturated, 31);
  state = saturated.process(sample(0.31, Eigen::Vector3d(2.0, 0.0, 0.0),
                                   Eigen::Vector3d(0.0, 0.0, kGravity)));
  EXPECT_TRUE(state.gyro_saturated);
  EXPECT_FALSE(state.integrated);
  EXPECT_EQ(saturated.counters().gyro_saturated_samples, 1u);
}

TEST(ImuMotionHealth, IntegratesShortTermRelativeMotionWithoutLidar) {
  ImuMotionHealth pipeline(testParams());
  feedStatic(&pipeline, 31);

  // Constant 1 m/s^2 world-x acceleration for 0.20 seconds.  The first
  // post-startup sample is the integration anchor, so expected v/p are the
  // ordinary 0.20-second analytic values within one sample period.
  ImuMotionHealth::State state;
  for (int i = 1; i <= 20; ++i) {
    state = pipeline.process(sample(0.30 + i * 0.01, Eigen::Vector3d::Zero(),
                                    Eigen::Vector3d(1.0, 0.0, kGravity)));
  }
  EXPECT_EQ(state.motion_state, MotionState::kMoving);
  EXPECT_TRUE(state.integrated);
  EXPECT_NEAR(state.velocity.x(), 0.20, 0.02);
  EXPECT_NEAR(state.position.x(), 0.02, 0.004);
  EXPECT_NEAR(state.velocity.y(), 0.0, 1e-6);
  EXPECT_NEAR(state.velocity.z(), 0.0, 1e-6);
  EXPECT_GT(state.relative_position.x(), 0.0);
}

TEST(ImuMotionHealth, DetectsImpactFallAndVibrationEvents) {
  ImuMotionHealthParams params = testParams();
  params.event_hold_duration_s = 0.10;
  params.fall_min_duration_s = 0.03;
  params.vibration_rms_threshold = 0.50;
  ImuMotionHealth pipeline(params);
  feedStatic(&pipeline, 31);

  auto state = pipeline.process(sample(0.31, Eigen::Vector3d::Zero(),
                                       Eigen::Vector3d(0.0, 0.0, 30.0)));
  EXPECT_EQ(state.motion_state, MotionState::kImpact);
  EXPECT_TRUE(state.impact);

  // Free fall for five 10-ms samples, followed by a normal sample.  The
  // latter closes the free-fall interval and makes the latched fall event
  // visible without relying on wall-clock time.
  for (int i = 1; i <= 5; ++i) {
    state = pipeline.process(sample(0.32 + i * 0.01, Eigen::Vector3d::Zero(),
                                    Eigen::Vector3d::Zero()));
  }
  state = pipeline.process(sample(0.38, Eigen::Vector3d::Zero(),
                                  Eigen::Vector3d(0.0, 0.0, kGravity)));
  EXPECT_EQ(state.motion_state, MotionState::kFall);
  EXPECT_TRUE(state.fall);

  // Use a fresh pipeline so the event hold and the static history do not make
  // the vibration assertion depend on the preceding fall sequence.
  ImuMotionHealth vibration_pipeline(params);
  feedStatic(&vibration_pipeline, 31);
  for (int i = 1; i <= 8; ++i) {
    const double sign = i % 2 == 0 ? 1.0 : -1.0;
    state = vibration_pipeline.process(
        sample(0.30 + i * 0.01, Eigen::Vector3d::Zero(),
               Eigen::Vector3d(2.0 * sign, 0.0, kGravity)));
  }
  EXPECT_EQ(state.motion_state, MotionState::kVibration);
  EXPECT_TRUE(state.vibration);
  EXPECT_GT(state.vibration_rms, params.vibration_rms_threshold);
}

TEST(ImuMotionHealth, DetectsQuietPostImpactTiltAsFall) {
  ImuMotionHealthParams params = testParams();
  params.motion_window_samples = 4;
  params.tilt_angle_threshold_deg = 45.0;
  params.tilt_min_duration_s = 0.02;
  params.event_hold_duration_s = 0.10;
  params.vibration_rms_threshold = 10.0;
  ImuMotionHealth pipeline(params);
  feedStatic(&pipeline, 31);

  // The body has come to rest on its side.  Feed the static gravity vector in
  // the tilted body frame but no gyro samples: this represents a post-impact
  // pose change whose angular-rate sample was missed.  Gravity direction must
  // still expose the persistent 60-degree tilt.
  const Eigen::Quaterniond body_tilt(
      Eigen::AngleAxisd(60.0 * kPi / 180.0, Eigen::Vector3d::UnitY()));
  const Eigen::Vector3d tilted_accel =
      body_tilt.inverse() * Eigen::Vector3d(0.0, 0.0, kGravity);
  ImuMotionHealth::State state;
  for (int i = 1; i <= 10; ++i) {
    state = pipeline.process(
        sample(0.30 + i * 0.01, Eigen::Vector3d::Zero(), tilted_accel));
  }

  EXPECT_TRUE(state.tilted);
  EXPECT_EQ(state.motion_state, MotionState::kFall);
  EXPECT_NEAR(state.tilt_angle_deg, 60.0, 1.0);
  EXPECT_NEAR(state.tilt_angle_rad, 60.0 * kPi / 180.0, 0.02);
  EXPECT_NE(state.diagnostic.find("tilt"), std::string::npos);
  EXPECT_NE(pipeline.toJson().find("\"tilted\":true"), std::string::npos);
}

TEST(ImuMotionHealth, JsonSnapshotIsSelfContained) {
  ImuMotionHealth pipeline(testParams());
  feedStatic(&pipeline, 31);
  const std::string json = pipeline.toJson();
  EXPECT_NE(json.find("\"motion_state\":\"stationary\""),
            std::string::npos);
  EXPECT_NE(json.find("\"health_state\":\"ready\""),
            std::string::npos);
  EXPECT_NE(json.find("\"gyro_bias\":["), std::string::npos);
  EXPECT_NE(json.find("\"orientation_wxyz\":["), std::string::npos);
  EXPECT_NE(json.find("\"tilt_angle_deg\":"), std::string::npos);
  EXPECT_NE(json.find("\"diagnostic\":"), std::string::npos);
}

TEST(ImuMotionHealth, ResetStartsAQuiescentStream) {
  ImuMotionHealth pipeline(testParams());
  feedStatic(&pipeline, 31);
  ASSERT_TRUE(pipeline.startupComplete());
  pipeline.reset();
  EXPECT_FALSE(pipeline.startupComplete());
  EXPECT_EQ(pipeline.counters().samples, 0u);
  EXPECT_EQ(pipeline.snapshot().motion_state, MotionState::kInitializing);
  EXPECT_EQ(pipeline.snapshot().health_state, HealthState::kInitializing);
  EXPECT_NEAR(pipeline.snapshot().position.norm(), 0.0, 1e-12);
}
