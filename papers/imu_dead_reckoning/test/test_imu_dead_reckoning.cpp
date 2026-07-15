#include "imu_dead_reckoning/imu_dead_reckoning.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::imu_dead_reckoning;

namespace {

constexpr double kGravity = 9.81;

ImuReading makeReading(double stamp, const Eigen::Vector3d& gyro,
                        const Eigen::Vector3d& accel) {
  ImuReading r;
  r.stamp = stamp;
  r.gyro = gyro;
  r.accel = accel;
  return r;
}

}  // namespace

// A perfectly static IMU with a known constant gyro bias: after static init
// absorbs the bias, continued integration should show ~zero position drift
// and negligible orientation drift, since the corrected gyro is ~0 and the
// corrected specific force cancels gravity exactly.
TEST(ImuDeadReckoning, StaticWithKnownBiasHasNoDrift) {
  ImuDeadReckoningParams params;
  ImuDeadReckoningPipeline pipe(params);

  const Eigen::Vector3d gyro_bias(0.01, -0.02, 0.015);
  const Eigen::Vector3d accel_static(0.0, 0.0, kGravity);
  const double dt = 0.02;
  const int total_steps = static_cast<int>(10.0 / dt);  // 10 s total.

  for (int i = 0; i < total_steps; ++i) {
    pipe.processImu(makeReading(i * dt, gyro_bias, accel_static));
  }

  const Eigen::Matrix4d T = pipe.pose();
  const Eigen::Vector3d p = T.block<3, 1>(0, 3);
  EXPECT_NEAR(p.norm(), 0.0, 1e-6);
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  EXPECT_NEAR((R - Eigen::Matrix3d::Identity()).norm(), 0.0, 1e-6);
}

// Constant angular rate about the body z axis with the specific force always
// exactly canceling gravity (accel = R(t)^-1 * g_w, which for a pure
// z-rotation and z-aligned gravity is simply constant): the integrated yaw
// should match the analytic omega * t, and position should stay near zero.
TEST(ImuDeadReckoning, ConstantYawRateNoTranslation) {
  ImuDeadReckoningParams params;
  ImuDeadReckoningPipeline pipe(params);

  const Eigen::Vector3d accel_static(0.0, 0.0, kGravity);
  const double dt = 0.01;

  // Static init window establishes R0 = I, g_w = (0,0,g).
  const int init_steps =
      static_cast<int>(params.static_init_duration_s / dt) + 1;
  for (int i = 0; i < init_steps; ++i) {
    pipe.processImu(makeReading(i * dt, Eigen::Vector3d::Zero(), accel_static));
  }

  const double omega = 0.5;  // rad/s about z.
  const double duration = 5.0;
  const int steps = static_cast<int>(duration / dt);
  double t = init_steps * dt;
  for (int i = 0; i < steps; ++i) {
    t += dt;
    pipe.processImu(
        makeReading(t, Eigen::Vector3d(0.0, 0.0, omega), accel_static));
  }

  const Eigen::Matrix4d T = pipe.pose();
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const double yaw = std::atan2(R(1, 0), R(0, 0));
  const double expected_yaw = omega * duration;
  EXPECT_NEAR(yaw, expected_yaw, 1e-4);
  const Eigen::Vector3d p = T.block<3, 1>(0, 3);
  EXPECT_NEAR(p.norm(), 0.0, 1e-6);
}

// A constant world-frame acceleration (on top of gravity) applied for 2 s
// after static init should produce position ~= 0.5 * a * t^2.
TEST(ImuDeadReckoning, ConstantWorldAccelerationMatchesAnalytic) {
  ImuDeadReckoningParams params;
  ImuDeadReckoningPipeline pipe(params);

  const Eigen::Vector3d accel_static(0.0, 0.0, kGravity);
  const double dt = 0.01;

  const int init_steps =
      static_cast<int>(params.static_init_duration_s / dt) + 1;
  for (int i = 0; i < init_steps; ++i) {
    pipe.processImu(makeReading(i * dt, Eigen::Vector3d::Zero(), accel_static));
  }

  // R stays identity (no rotation), so world accel == body accel.
  const double a_x = 1.0;
  const Eigen::Vector3d accel_dynamic(a_x, 0.0, kGravity);
  const double duration = 2.0;
  const int steps = static_cast<int>(duration / dt);
  double t = init_steps * dt;
  for (int i = 0; i < steps; ++i) {
    t += dt;
    pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_dynamic));
  }

  const double expected_x = 0.5 * a_x * duration * duration;
  const Eigen::Vector3d p = pipe.pose().block<3, 1>(0, 3);
  EXPECT_NEAR(p.x(), expected_x, 0.05 * expected_x);
  EXPECT_NEAR(p.y(), 0.0, 1e-6);
  EXPECT_NEAR(p.z(), 0.0, 1e-6);
}

// A motion segment (constant accel, building up velocity) followed by a
// segment whose IMU signature reads "static" (near-zero gyro, |accel| ~= g):
// with ZUPT enabled velocity should reset to ~0, and position drift during
// the static tail should be much smaller than with ZUPT disabled (where the
// pipeline has no way to know it stopped moving and keeps coasting).
TEST(ImuDeadReckoning, ZuptResetsVelocityAndLimitsTailDrift) {
  const Eigen::Vector3d accel_static(0.0, 0.0, kGravity);
  const double dt = 0.01;
  const double motion_a_x = 1.0;
  const double motion_duration = 1.0;
  const double tail_duration = 3.0;

  auto run = [&](bool enable_zupt, long* zupt_frames) {
    ImuDeadReckoningParams params;
    params.enable_zupt = enable_zupt;
    ImuDeadReckoningPipeline pipe(params);

    const int init_steps =
        static_cast<int>(params.static_init_duration_s / dt) + 1;
    double t = 0.0;
    for (int i = 0; i < init_steps; ++i) {
      pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_static));
      t += dt;
    }

    const int motion_steps = static_cast<int>(motion_duration / dt);
    const Eigen::Vector3d accel_motion(motion_a_x, 0.0, kGravity);
    for (int i = 0; i < motion_steps; ++i) {
      pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_motion));
      t += dt;
    }

    const double tail_start_x = pipe.pose()(0, 3);
    const int tail_steps = static_cast<int>(tail_duration / dt);
    long zupt = 0;
    for (int i = 0; i < tail_steps; ++i) {
      const auto stats =
          pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_static));
      if (stats.zupt_active) ++zupt;
      t += dt;
    }
    if (zupt_frames) *zupt_frames = zupt;
    return pipe.pose()(0, 3) - tail_start_x;
  };

  long zupt_frames_on = 0;
  long zupt_frames_off = 0;
  const double tail_drift_zupt_on = run(true, &zupt_frames_on);
  const double tail_drift_zupt_off = run(false, &zupt_frames_off);

  EXPECT_GT(zupt_frames_on, 0);
  EXPECT_EQ(zupt_frames_off, 0);
  EXPECT_GT(std::abs(tail_drift_zupt_off), 0.5);
  EXPECT_LT(std::abs(tail_drift_zupt_on),
            0.1 * std::abs(tail_drift_zupt_off));
}

// Sanity check of the frame-sampling free function: it should reject empty
// IMU input and otherwise return one relative pose per frame timestamp with
// the first pose at identity (no IMU processed yet before the first query).
TEST(ImuDeadReckoning, IntegrateTrajectoryFrameSampling) {
  ImuDeadReckoningParams params;
  std::string error;
  EXPECT_TRUE(integrateImuTrajectory({}, {0.0, 1.0}, params, nullptr, &error)
                  .empty());
  EXPECT_FALSE(error.empty());

  std::vector<ImuReading> imu;
  const double dt = 0.01;
  for (int i = 0; i < 500; ++i) {
    imu.push_back(
        makeReading(i * dt, Eigen::Vector3d::Zero(), Eigen::Vector3d(0, 0, kGravity)));
  }
  const std::vector<double> frame_timestamps = {0.0, 1.0, 2.5, 4.0};
  long zupt_frames = 0;
  std::string warn;
  const auto poses = integrateImuTrajectory(imu, frame_timestamps, params,
                                             &zupt_frames, &warn);
  ASSERT_EQ(poses.size(), frame_timestamps.size());
  EXPECT_TRUE(poses.front().isApprox(Eigen::Matrix4d::Identity()));
}

// Inject lateral body velocity via a brief y-acceleration impulse; with NHC
// enabled the lateral component should be stripped each step so tail position
// drift stays much smaller than without NHC.
TEST(ImuDeadReckoning, NhcSuppressesLateralVelocity) {
  const Eigen::Vector3d accel_static(0.0, 0.0, kGravity);
  const double dt = 0.01;
  const double impulse_duration = 0.2;
  const double tail_duration = 2.0;

  auto run = [&](bool enable_nhc) {
    ImuDeadReckoningParams params;
    params.enable_nhc = enable_nhc;
    ImuDeadReckoningPipeline pipe(params);

    const int init_steps =
        static_cast<int>(params.static_init_duration_s / dt) + 1;
    double t = 0.0;
    for (int i = 0; i < init_steps; ++i) {
      pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_static));
      t += dt;
    }

    const int impulse_steps = static_cast<int>(impulse_duration / dt);
    const Eigen::Vector3d accel_impulse(0.0, 2.0, kGravity);
    for (int i = 0; i < impulse_steps; ++i) {
      pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_impulse));
      t += dt;
    }

    const double y_after_impulse = pipe.pose()(1, 3);
    const int tail_steps = static_cast<int>(tail_duration / dt);
    for (int i = 0; i < tail_steps; ++i) {
      pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_static));
      t += dt;
    }
    return std::abs(pipe.pose()(1, 3) - y_after_impulse);
  };

  const double tail_drift_nhc_on = run(true);
  const double tail_drift_nhc_off = run(false);
  EXPECT_GT(tail_drift_nhc_off, 0.05);
  EXPECT_LT(tail_drift_nhc_on, 0.2 * tail_drift_nhc_off);
}

// ZUPT and accel-bias estimation combined: the stationary gate must compare
// the bias-corrected accel norm against the fixed gravity magnitude, so a
// large constant accel bias (which pushes the raw norm outside the ZUPT
// accel tolerance) must not prevent zero-velocity detection on a static
// segment.
TEST(ImuDeadReckoning, ZuptGateUsesBiasCorrectedAccelNorm) {
  ImuDeadReckoningParams params;
  params.enable_zupt = true;
  params.estimate_accel_bias = true;
  ImuDeadReckoningPipeline pipe(params);

  // Raw static norm is ~10.81, ~1.0 above standard gravity -- outside the
  // 0.8 m/s^2 ZUPT tolerance unless the estimated bias is subtracted first.
  const Eigen::Vector3d accel_bias(0.0, 0.0, 1.0);
  const Eigen::Vector3d accel_static =
      Eigen::Vector3d(0.0, 0.0, kGravity) + accel_bias;
  const double dt = 0.02;
  const int total_steps = static_cast<int>(10.0 / dt);

  long zupt = 0;
  for (int i = 0; i < total_steps; ++i) {
    const auto stats = pipe.processImu(
        makeReading(i * dt, Eigen::Vector3d::Zero(), accel_static));
    if (stats.zupt_active) ++zupt;
  }
  EXPECT_GT(zupt, 0);
}

// RK4 attitude integration should track a constant yaw rate to analytic
// precision, matching the midpoint scheme's guarantee on the same signal.
TEST(ImuDeadReckoning, Rk4ConstantYawRateMatchesAnalytic) {
  ImuDeadReckoningParams params;
  params.rk4_integration = true;
  ImuDeadReckoningPipeline pipe(params);

  const Eigen::Vector3d accel_static(0.0, 0.0, kGravity);
  const double dt = 0.01;
  const int init_steps =
      static_cast<int>(params.static_init_duration_s / dt) + 1;
  for (int i = 0; i < init_steps; ++i) {
    pipe.processImu(makeReading(i * dt, Eigen::Vector3d::Zero(), accel_static));
  }

  const double omega = 0.5;
  const double duration = 5.0;
  const int steps = static_cast<int>(duration / dt);
  double t = init_steps * dt;
  for (int i = 0; i < steps; ++i) {
    t += dt;
    pipe.processImu(
        makeReading(t, Eigen::Vector3d(0.0, 0.0, omega), accel_static));
  }

  const Eigen::Matrix4d T = pipe.pose();
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const double yaw = std::atan2(R(1, 0), R(0, 0));
  EXPECT_NEAR(yaw, omega * duration, 1e-3);
  // Still a proper rotation after RK4 + re-orthonormalization.
  EXPECT_NEAR((R.transpose() * R - Eigen::Matrix3d::Identity()).norm(), 0.0,
              1e-9);
}

// Motion-gated static init: a start window that is NOT truly static (its accel
// norm oscillates with "road vibration") carries a contaminated mean gyro that
// must NOT be trusted as a bias. With the quality gate on, bias estimation is
// auto-skipped (matching a manual --no-gyro-bias run); with it off the bogus
// bias is applied and rotates the subsequent trajectory.
TEST(ImuDeadReckoning, MotionGatedStaticInitSkipsContaminatedBias) {
  const double dt = 0.02;
  const int init_steps = 130;  // 2.6 s > static_init_duration_s, ~130 samples.
  const int post_steps = 200;
  // Non-static window: constant gyro offset (looks like bias, is really motion)
  // + oscillating accel norm (vibration) that trips the accel-std gate. gyro
  // std is 0 so only the accel gate fires -- exercises that path specifically.
  const Eigen::Vector3d contaminated_gyro(0.0, 0.0, 0.1);

  auto run = [&](bool motion_gated, bool estimate_gyro_bias, bool* tripped) {
    ImuDeadReckoningParams params;
    params.motion_gated_static_init = motion_gated;
    params.estimate_gyro_bias = estimate_gyro_bias;
    ImuDeadReckoningPipeline pipe(params);
    double t = 0.0;
    for (int i = 0; i < init_steps; ++i) {
      const double wobble = (i % 2 == 0) ? 1.0 : -1.0;  // +/-1 m/s^2 on |a|.
      pipe.processImu(makeReading(
          t, contaminated_gyro, Eigen::Vector3d(0.0, 0.0, kGravity + wobble)));
      t += dt;
    }
    if (tripped) *tripped = pipe.staticGateTripped();
    for (int i = 0; i < post_steps; ++i) {
      pipe.processImu(
          makeReading(t, Eigen::Vector3d::Zero(), Eigen::Vector3d(0, 0, kGravity)));
      t += dt;
    }
    const Eigen::Matrix3d R = pipe.pose().block<3, 3>(0, 0);
    return std::atan2(R(1, 0), R(0, 0));
  };

  bool tripped_gated = false, tripped_ungated = false, tripped_manual = false;
  const double yaw_gated = run(true, true, &tripped_gated);
  const double yaw_manual = run(false, false, &tripped_manual);   // reference.
  const double yaw_applied = run(false, true, &tripped_ungated);  // bad bias.

  EXPECT_TRUE(tripped_gated);
  EXPECT_FALSE(tripped_ungated);
  EXPECT_FALSE(tripped_manual);
  // Gate auto-does what a manual --no-gyro-bias run does: identical trajectory.
  EXPECT_NEAR(yaw_gated, yaw_manual, 1e-9);
  // Without the gate the contaminated bias rotates the whole tail trajectory
  // the other way, well clear of the gated/manual result.
  EXPECT_GT(std::abs(yaw_applied), 0.3);
  EXPECT_GT(std::abs(yaw_applied - yaw_manual), 0.3);
}

// Motion-gated static init: a genuinely static window (constant accel = g, low
// gyro noise) must PASS the gate so the real gyro bias is still estimated and
// removed -- the gate must not fire on legitimately parked starts.
TEST(ImuDeadReckoning, MotionGatedStaticInitPassesOnTrulyStatic) {
  ImuDeadReckoningParams params;
  params.motion_gated_static_init = true;
  ImuDeadReckoningPipeline pipe(params);

  const Eigen::Vector3d gyro_bias(0.01, -0.02, 0.015);
  const Eigen::Vector3d accel_static(0.0, 0.0, kGravity);
  const double dt = 0.02;
  const int total_steps = static_cast<int>(10.0 / dt);
  for (int i = 0; i < total_steps; ++i) {
    pipe.processImu(makeReading(i * dt, gyro_bias, accel_static));
  }

  EXPECT_FALSE(pipe.staticGateTripped());
  // Real bias absorbed -> no drift, same guarantee as the ungated static test.
  const Eigen::Vector3d p = pipe.pose().block<3, 1>(0, 3);
  EXPECT_NEAR(p.norm(), 0.0, 1e-6);
}

// ZUPT-time attitude leveling: after a spurious pitch error is injected into
// the estimator's orientation (gyro rotates R while the body-frame accel keeps
// reading gravity straight down -- i.e. a pure estimator tilt), a stationary
// tail with leveling on should pull roll/pitch back toward level, while
// without leveling the tilt just persists (zero gyro -> no attitude change).
TEST(ImuDeadReckoning, LevelingCorrectsAttitudeTiltWhenStationary) {
  const double dt = 0.02;
  const Eigen::Vector3d accel_down(0.0, 0.0, kGravity);  // body-frame gravity.

  auto tilt_of = [](const Eigen::Matrix3d& R) {
    // Angle between the body z axis expressed in world and world +z.
    const Eigen::Vector3d bz = (R * Eigen::Vector3d::UnitZ()).normalized();
    return std::acos(std::max(-1.0, std::min(1.0, bz.dot(Eigen::Vector3d::UnitZ()))));
  };

  auto run = [&](bool enable_leveling) {
    ImuDeadReckoningParams params;
    params.enable_leveling = enable_leveling;
    params.leveling_gain = 0.2;
    ImuDeadReckoningPipeline pipe(params);

    double t = 0.0;
    const int init_steps =
        static_cast<int>(params.static_init_duration_s / dt) + 5;
    for (int i = 0; i < init_steps; ++i) {
      pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_down));
      t += dt;
    }
    // Inject an estimator tilt: rotate R about body y for a short burst whose
    // gyro is well above the ZUPT gate, so leveling does NOT fire mid-burst.
    for (int i = 0; i < 20; ++i) {
      pipe.processImu(makeReading(t, Eigen::Vector3d(0.0, 0.5, 0.0), accel_down));
      t += dt;
    }
    const double tilt_after_burst = tilt_of(pipe.pose().block<3, 3>(0, 0));
    // Stationary tail: gate warms up over zupt_window then leveling can act.
    for (int i = 0; i < 400; ++i) {
      pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_down));
      t += dt;
    }
    return std::make_pair(tilt_after_burst, tilt_of(pipe.pose().block<3, 3>(0, 0)));
  };

  const auto with_lvl = run(true);
  const auto without_lvl = run(false);
  // Both injected the same tilt.
  EXPECT_GT(with_lvl.first, 0.1);
  EXPECT_NEAR(with_lvl.first, without_lvl.first, 1e-9);
  // Without leveling the tilt persists through the static tail (zero gyro ->
  // no attitude change; only a tiny midpoint carry-over from the last burst
  // sample, so it stays essentially unchanged rather than being corrected).
  EXPECT_GT(without_lvl.second, 0.9 * without_lvl.first);
  // With leveling it is pulled back toward level.
  EXPECT_LT(with_lvl.second, 0.2 * with_lvl.first);
}

// ZARU (zero angular rate update): with static-init bias estimation disabled,
// a constant gyro bias below the ZUPT gate would otherwise integrate into an
// unbounded yaw drift. ZARU samples the raw gyro while stationary and low-passes
// the running bias toward it, so the corrected rate -> 0 and yaw stops growing.
TEST(ImuDeadReckoning, ZaruTracksGyroBiasAndStopsYawDrift) {
  const double dt = 0.02;
  const Eigen::Vector3d gyro_bias(0.0, 0.0, 0.03);  // below zupt_gyro_threshold.
  const Eigen::Vector3d accel_down(0.0, 0.0, kGravity);

  auto final_yaw = [&](bool enable_zaru) {
    ImuDeadReckoningParams params;
    params.estimate_gyro_bias = false;  // init leaves the bias in place.
    params.enable_zaru = enable_zaru;
    params.zaru_gain = 0.05;
    ImuDeadReckoningPipeline pipe(params);
    const int total_steps = static_cast<int>(12.0 / dt);
    for (int i = 0; i < total_steps; ++i) {
      pipe.processImu(makeReading(i * dt, gyro_bias, accel_down));
    }
    const Eigen::Matrix3d R = pipe.pose().block<3, 3>(0, 0);
    return std::atan2(R(1, 0), R(0, 0));
  };

  const double yaw_off = final_yaw(false);
  const double yaw_on = final_yaw(true);
  EXPECT_GT(std::abs(yaw_off), 0.15);              // uncorrected bias drifts.
  EXPECT_LT(std::abs(yaw_on), 0.2 * std::abs(yaw_off));  // ZARU bounds it.
}

// ESKF mode, zero-velocity update: a motion segment builds velocity, then a
// segment that reads "static" should let the ZUPT measurement update pull the
// velocity error toward zero and bound tail drift -- the Kalman-weighted
// analogue of the hard ZUPT reset. Covariance must stay finite throughout.
TEST(ImuDeadReckoning, EskfZuptLimitsTailDrift) {
  const Eigen::Vector3d accel_static(0.0, 0.0, kGravity);
  const double dt = 0.01;

  auto run = [&](bool eskf_zupt, long* frames) {
    ImuDeadReckoningParams params;
    if (eskf_zupt) {
      params.enable_eskf = true;
      params.enable_zupt = true;
    }
    ImuDeadReckoningPipeline pipe(params);
    const int init_steps =
        static_cast<int>(params.static_init_duration_s / dt) + 1;
    double t = 0.0;
    for (int i = 0; i < init_steps; ++i) {
      pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_static));
      t += dt;
    }
    const Eigen::Vector3d accel_motion(1.0, 0.0, kGravity);
    for (int i = 0; i < 100; ++i) {  // 1 s of +x accel.
      pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_motion));
      t += dt;
    }
    const double tail_start_x = pipe.pose()(0, 3);
    long zupt = 0;
    for (int i = 0; i < 300; ++i) {  // 3 s static tail.
      const auto s =
          pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_static));
      if (s.zupt_active) ++zupt;
      t += dt;
    }
    if (frames) *frames = zupt;
    // Covariance must remain finite.
    EXPECT_TRUE(pipe.covariance().allFinite());
    return pipe.pose()(0, 3) - tail_start_x;
  };

  long frames_on = 0, frames_off = 0;
  const double drift_eskf = run(true, &frames_on);
  const double drift_dr = run(false, &frames_off);
  EXPECT_GT(frames_on, 0);
  EXPECT_EQ(frames_off, 0);
  EXPECT_GT(std::abs(drift_dr), 0.5);
  EXPECT_LT(std::abs(drift_eskf), 0.2 * std::abs(drift_dr));
}

// ESKF mode, gravity update: an injected estimator tilt should be corrected by
// the leveling (accel) measurement update during a stationary tail, just like
// the hard leveling aid but gain-weighted by the covariance.
TEST(ImuDeadReckoning, EskfLevelingCorrectsTilt) {
  const double dt = 0.02;
  const Eigen::Vector3d accel_down(0.0, 0.0, kGravity);
  auto tilt_of = [](const Eigen::Matrix3d& R) {
    const Eigen::Vector3d bz = (R * Eigen::Vector3d::UnitZ()).normalized();
    return std::acos(std::max(-1.0, std::min(1.0, bz.dot(Eigen::Vector3d::UnitZ()))));
  };

  ImuDeadReckoningParams params;
  params.enable_eskf = true;
  params.enable_leveling = true;
  ImuDeadReckoningPipeline pipe(params);
  double t = 0.0;
  const int init_steps =
      static_cast<int>(params.static_init_duration_s / dt) + 5;
  for (int i = 0; i < init_steps; ++i) {
    pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_down));
    t += dt;
  }
  for (int i = 0; i < 20; ++i) {  // tilt burst above the ZUPT gate.
    pipe.processImu(makeReading(t, Eigen::Vector3d(0.0, 0.5, 0.0), accel_down));
    t += dt;
  }
  const double tilt_after_burst = tilt_of(pipe.pose().block<3, 3>(0, 0));
  for (int i = 0; i < 400; ++i) {
    pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_down));
    t += dt;
  }
  const double tilt_final = tilt_of(pipe.pose().block<3, 3>(0, 0));
  EXPECT_GT(tilt_after_burst, 0.1);
  EXPECT_LT(tilt_final, 0.3 * tilt_after_burst);
  EXPECT_TRUE(pipe.covariance().allFinite());
}

// ESKF measurement updates must reduce the filter's own uncertainty: after a
// static segment with ZUPT + leveling, the velocity and roll/pitch covariance
// blocks should shrink below their initial values (information was gained).
TEST(ImuDeadReckoning, EskfUpdatesShrinkCovariance) {
  ImuDeadReckoningParams params;
  params.enable_eskf = true;
  params.enable_zupt = true;
  params.enable_leveling = true;
  ImuDeadReckoningPipeline pipe(params);
  const Eigen::Vector3d accel_static(0.0, 0.0, kGravity);
  const double dt = 0.02;

  // Advance just past init so P is seeded, capture it, then run a static tail.
  const int init_steps = static_cast<int>(params.static_init_duration_s / dt) + 2;
  for (int i = 0; i < init_steps; ++i) {
    pipe.processImu(makeReading(i * dt, Eigen::Vector3d::Zero(), accel_static));
  }
  const double vel_var_init = pipe.covariance().block<3, 3>(3, 3).trace();
  double t = init_steps * dt;
  for (int i = 0; i < 400; ++i) {
    pipe.processImu(makeReading(t, Eigen::Vector3d::Zero(), accel_static));
    t += dt;
  }
  const Eigen::Matrix<double, 15, 15> P = pipe.covariance();
  EXPECT_TRUE(P.allFinite());
  // Velocity uncertainty shrinks under repeated ZUPT.
  const double vel_var_final = P.block<3, 3>(3, 3).trace();
  EXPECT_LT(vel_var_final, vel_var_init);
  // Roll/pitch (x,y attitude) uncertainty shrinks under leveling.
  const double rp_var = P(6, 6) + P(7, 7);
  const double rp_var_init =
      params.eskf_init_sigma_att * params.eskf_init_sigma_att * 2.0;
  EXPECT_LT(rp_var, rp_var_init);
  // Covariance stays symmetric PSD-ish: diagonal non-negative.
  for (int i = 0; i < 15; ++i) EXPECT_GE(P(i, i), 0.0);
}

// Covariance is only populated in ESKF mode; the default pipeline leaves it
// zero (no filter running).
TEST(ImuDeadReckoning, CovarianceZeroWhenEskfOff) {
  ImuDeadReckoningParams params;
  ImuDeadReckoningPipeline pipe(params);
  const Eigen::Vector3d accel_static(0.0, 0.0, kGravity);
  for (int i = 0; i < 300; ++i) {
    pipe.processImu(makeReading(i * 0.02, Eigen::Vector3d::Zero(), accel_static));
  }
  EXPECT_EQ(pipe.covariance().norm(), 0.0);
}

// A constant accelerometer bias orthogonal to gravity should be absorbed when
// estimate_accel_bias is enabled, keeping position near zero on an otherwise
// static segment after init.
TEST(ImuDeadReckoning, AccelBiasEstimationRemovesStaticBias) {
  const Eigen::Vector3d gyro_bias(0.01, -0.02, 0.015);
  const Eigen::Vector3d accel_bias(0.05, -0.03, 0.02);
  const Eigen::Vector3d accel_static =
      Eigen::Vector3d(0.0, 0.0, kGravity) + accel_bias;
  const double dt = 0.02;
  const int total_steps = static_cast<int>(10.0 / dt);

  auto run = [&](bool estimate_accel_bias) {
    ImuDeadReckoningParams params;
    params.gravity_from_static_norm = false;
    params.estimate_accel_bias = estimate_accel_bias;
    ImuDeadReckoningPipeline pipe(params);
    for (int i = 0; i < total_steps; ++i) {
      pipe.processImu(makeReading(i * dt, gyro_bias, accel_static));
    }
    return pipe.pose().block<3, 1>(0, 3).norm();
  };

  const double drift_with = run(true);
  const double drift_without = run(false);
  EXPECT_GT(drift_without, 0.1);
  EXPECT_LT(drift_with, 0.01);
}
