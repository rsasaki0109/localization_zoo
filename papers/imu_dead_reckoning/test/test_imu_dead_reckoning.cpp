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
