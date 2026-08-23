#include "imu_motion_health/imu_motion_health.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

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

TEST(ImuMotionHealthTest, ImpactCanRequireAccelAndGyroTogether) {
  ImuMotionHealthParams params;
  params.startup_duration_s = 0.02;
  params.startup_min_samples = 2;
  params.impact_accel_threshold = 15.0;
  params.impact_gyro_threshold = 2.0;
  params.impact_requires_accel_and_gyro = true;
  ImuMotionHealth imu(params);
  imu.process(0.00, Eigen::Vector3d::Zero(), Eigen::Vector3d(0, 0, 9.80665));
  imu.process(0.02, Eigen::Vector3d::Zero(), Eigen::Vector3d(0, 0, 9.80665));
  auto accel_only = imu.process(0.03, Eigen::Vector3d::Zero(), Eigen::Vector3d(20, 0, 0));
  EXPECT_FALSE(accel_only.impact);
  auto combined = imu.process(0.04, Eigen::Vector3d(3, 0, 0), Eigen::Vector3d(20, 0, 0));
  EXPECT_TRUE(combined.impact);
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

TEST(ImuMotionHealth, EventLifecycleHasStableIdPeaksAndJsonContract) {
  ImuMotionHealthParams params = testParams();
  params.event_hold_duration_s = 0.02;
  params.motion_window_samples = 4;
  params.vibration_rms_threshold = 100.0;
  ImuMotionHealth pipeline(params);
  feedStatic(&pipeline, 31);

  pipeline.process(sample(0.31, Eigen::Vector3d::Zero(),
                          Eigen::Vector3d(0.0, 0.0, 30.0)));
  auto pending = pipeline.drainEvents();
  ASSERT_EQ(pending.size(), 1u);
  EXPECT_EQ(pending.front().phase,
            localization_zoo::imu_motion_health::ImuEventPhase::kStarted);
  EXPECT_EQ(pending.front().type,
            localization_zoo::imu_motion_health::ImuEventType::kImpact);
  const std::uint64_t id = pending.front().id;
  EXPECT_GT(id, 0u);
  EXPECT_GT(pending.front().peak_accel_norm, 25.0);
  EXPECT_NE(localization_zoo::imu_motion_health::toJson(pending.front()).find(
                "imu_motion_health_event_v1"),
            std::string::npos);

  pipeline.process(sample(0.32, Eigen::Vector3d::Zero(),
                          Eigen::Vector3d(0.0, 0.0, kGravity)));
  pipeline.process(sample(0.35, Eigen::Vector3d::Zero(),
                          Eigen::Vector3d(0.0, 0.0, kGravity)));
  pending = pipeline.drainEvents();
  ASSERT_EQ(pending.size(), 1u);
  EXPECT_EQ(pending.front().phase,
            localization_zoo::imu_motion_health::ImuEventPhase::kEnded);
  EXPECT_EQ(pending.front().id, id);
  EXPECT_TRUE(pending.front().has_end_timestamp);
  EXPECT_NEAR(pending.front().duration_s, 0.04, 1e-9);
  EXPECT_GT(pending.front().peak_accel_norm, 25.0);
  EXPECT_NE(localization_zoo::imu_motion_health::toJson(pending.front()).find(
                "\"phase\":\"ended\""),
            std::string::npos);
}

TEST(ImuMotionHealth, EventQueueIsBoundedAndSupportsSimultaneousTypes) {
  ImuMotionHealthParams params = testParams();
  params.event_hold_duration_s = 0.02;
  params.motion_window_samples = 4;
  params.vibration_rms_threshold = 0.2;
  params.event_queue_capacity = 2;
  ImuMotionHealth pipeline(params);
  feedStatic(&pipeline, 31);

  // A large sample starts impact and, after the rolling window fills, the
  // alternating samples keep vibration active at the same time.
  pipeline.process(sample(0.31, Eigen::Vector3d::Zero(),
                          Eigen::Vector3d(0.0, 0.0, 30.0)));
  for (int i = 1; i <= 4; ++i) {
    const double sign = i % 2 == 0 ? 1.0 : -1.0;
    pipeline.process(sample(0.31 + i * 0.01, Eigen::Vector3d::Zero(),
                            Eigen::Vector3d(2.0 * sign, 0.0, kGravity)));
  }
  EXPECT_LE(pipeline.pendingEventCount(), 2u);
  pipeline.flushEvents(0.40);
  EXPECT_LE(pipeline.pendingEventCount(), 2u);
  EXPECT_GT(pipeline.droppedEventCount(), 0u);
}

TEST(ImuMotionHealth, ActiveEventSnapshotsContinueWithSameIdAndGrowingPeaks) {
  ImuMotionHealthParams params = testParams();
  params.event_hold_duration_s = 0.10;
  params.vibration_rms_threshold = 100.0;
  ImuMotionHealth pipeline(params);
  feedStatic(&pipeline, 31);

  pipeline.process(sample(0.31, Eigen::Vector3d::Zero(),
                          Eigen::Vector3d(0.0, 0.0, 30.0)));
  auto lifecycle = pipeline.drainEvents();
  ASSERT_EQ(lifecycle.size(), 1u);
  ASSERT_EQ(lifecycle.front().phase,
            localization_zoo::imu_motion_health::ImuEventPhase::kStarted);
  const std::uint64_t id = lifecycle.front().id;
  EXPECT_EQ(pipeline.activeEventCount(), 1u);
  auto active = pipeline.activeEvents();
  ASSERT_EQ(active.size(), 1u);
  EXPECT_EQ(active.front().id, id);
  EXPECT_EQ(active.front().phase,
            localization_zoo::imu_motion_health::ImuEventPhase::kUpdated);
  EXPECT_NE(localization_zoo::imu_motion_health::toJson(active.front()).find(
                "\"phase\":\"updated\""),
            std::string::npos);
  EXPECT_NEAR(active.front().duration_s, 0.0, 1e-12);
  EXPECT_EQ(pipeline.pendingEventCount(), 0u);

  pipeline.process(sample(0.32, Eigen::Vector3d::Zero(),
                          Eigen::Vector3d(0.0, 0.0, 35.0)));
  active = pipeline.activeEvents();
  ASSERT_EQ(active.size(), 1u);
  EXPECT_EQ(active.front().id, id);
  EXPECT_GT(active.front().duration_s, 0.0);
  EXPECT_GT(active.front().peak_accel_norm, lifecycle.front().peak_accel_norm);
  EXPECT_TRUE(active.front().peak_accel >= active.front().peak_accel_norm);

  pipeline.flushEvents(0.50);
  EXPECT_EQ(pipeline.activeEventCount(), 0u);
  lifecycle = pipeline.drainEvents();
  ASSERT_EQ(lifecycle.size(), 1u);
  EXPECT_EQ(lifecycle.front().phase,
            localization_zoo::imu_motion_health::ImuEventPhase::kEnded);
  EXPECT_EQ(lifecycle.front().id, id);
  EXPECT_GT(lifecycle.front().duration_s, active.front().duration_s);
  EXPECT_NEAR(lifecycle.front().peak_accel_norm, active.front().peak_accel_norm,
              1e-12);
}

TEST(ImuMotionHealth, BuiltInAndYamlProfilesAreStrictAndLayerable) {
  ImuMotionHealthParams wearable;
  ImuMotionHealthParams vehicle;
  std::string error;
  ASSERT_TRUE(localization_zoo::imu_motion_health::applyProfile(
      "wearable", &wearable, &error)) << error;
  ASSERT_TRUE(localization_zoo::imu_motion_health::applyProfile(
      "vehicle", &vehicle, &error)) << error;
  EXPECT_NE(wearable.impact_accel_threshold, vehicle.impact_accel_threshold);
  EXPECT_EQ(localization_zoo::imu_motion_health::profileName("UAV"),
            std::string("drone"));

  const std::string path = "imu_motion_health_profile_test.yaml";
  {
    std::ofstream output(path);
    ASSERT_TRUE(output.good());
    output << "schema: imu_motion_health_profile_v1\n"
           << "profile: cargo\n"
           << "impact_accel_threshold: 9.5\n"
           << "event_queue_capacity: 7\n";
  }
  ImuMotionHealthParams loaded;
  ASSERT_TRUE(localization_zoo::imu_motion_health::loadProfileFile(
      path, &loaded, &error)) << error;
  EXPECT_NEAR(loaded.impact_accel_threshold, 9.5, 1e-12);
  EXPECT_EQ(loaded.event_queue_capacity, 7u);
  std::remove(path.c_str());

  {
    std::ofstream output(path);
    ASSERT_TRUE(output.good());
    output << "impct_accel_threshold: 1.0\n";
  }
  EXPECT_FALSE(localization_zoo::imu_motion_health::loadYamlProfile(
      path, &loaded, &error));
  EXPECT_NE(error.find("unknown profile key"), std::string::npos);
  std::remove(path.c_str());
}

TEST(ImuMotionHealth, DetectsPersistentStationaryGyroBiasJump) {
  ImuMotionHealthParams params = testParams();
  params.gyro_bias_jump_threshold = 0.15;
  params.gyro_bias_jump_min_duration_s = 0.02;
  params.event_hold_duration_s = 0.05;
  params.vibration_rms_threshold = 100.0;
  ImuMotionHealth pipeline(params);
  feedStatic(&pipeline, 31);

  pipeline.process(sample(0.31, Eigen::Vector3d(0.30, 0.0, 0.0),
                          Eigen::Vector3d(0.0, 0.0, kGravity)));
  pipeline.process(sample(0.32, Eigen::Vector3d(0.30, 0.0, 0.0),
                          Eigen::Vector3d(0.0, 0.0, kGravity)));
  const auto state = pipeline.process(
      sample(0.33, Eigen::Vector3d(0.30, 0.0, 0.0),
             Eigen::Vector3d(0.0, 0.0, kGravity)));
  EXPECT_TRUE(state.gyro_bias_jump);
  EXPECT_NEAR(state.gyro_bias_delta_norm, 0.30, 1e-9);
  EXPECT_NEAR(state.gyro_bias_jump_duration_s, 0.02, 1e-9);
  EXPECT_EQ(state.health_state, HealthState::kDegraded);
  EXPECT_NE(state.diagnostic.find("gyro bias jump"), std::string::npos);
  EXPECT_EQ(pipeline.counters().gyro_bias_jump_detections, 1u);
  EXPECT_GT(pipeline.counters().gyro_bias_jump_samples, 0u);

  auto events = pipeline.drainEvents();
  ASSERT_FALSE(events.empty());
  ASSERT_EQ(events.back().type,
            localization_zoo::imu_motion_health::ImuEventType::kBiasJump);
  EXPECT_EQ(events.back().phase,
            localization_zoo::imu_motion_health::ImuEventPhase::kStarted);
  const std::uint64_t id = events.back().id;

  pipeline.process(sample(0.34, Eigen::Vector3d::Zero(),
                          Eigen::Vector3d(0.0, 0.0, kGravity)));
  const auto recovered = pipeline.process(
      sample(0.38, Eigen::Vector3d::Zero(),
             Eigen::Vector3d(0.0, 0.0, kGravity)));
  EXPECT_FALSE(recovered.gyro_bias_jump);
  EXPECT_EQ(recovered.health_state, HealthState::kReady)
      << "diagnostic=" << recovered.diagnostic
      << " startup_quality=" << recovered.startup_quality_ok
      << " gap=" << recovered.gap << " saturated=" << recovered.saturated
      << " gyro_jump=" << recovered.gyro_bias_jump;
  events = pipeline.drainEvents();
  ASSERT_FALSE(events.empty());
  EXPECT_EQ(events.back().type,
            localization_zoo::imu_motion_health::ImuEventType::kBiasJump);
  EXPECT_EQ(events.back().phase,
            localization_zoo::imu_motion_health::ImuEventPhase::kEnded);
  EXPECT_EQ(events.back().id, id);
}
