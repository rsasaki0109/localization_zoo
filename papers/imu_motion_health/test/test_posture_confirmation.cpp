#include "imu_motion_health/imu_motion_health.h"
#include "imu_motion_health/posture_confirmation.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using namespace localization_zoo::imu_motion_health;

namespace {

constexpr double kGravity = 9.80665;
constexpr double kRate = 100.0;

// Sensor tilted by `angle` about x: gravity seen in the sensor frame.
Eigen::Vector3d gravityAt(double angle) {
  return Eigen::Vector3d(0.0, kGravity * std::sin(angle), kGravity * std::cos(angle));
}

struct Stream {
  std::vector<double> t;
  std::vector<Eigen::Vector3d> gyro, accel;
  void add(double angle_rate, double angle, int samples) {
    for (int i = 0; i < samples; ++i) {
      t.push_back(t.size() / kRate);
      gyro.push_back(Eigen::Vector3d(angle_rate, 0, 0));
      accel.push_back(gravityAt(angle));
    }
  }
};

// Upright for 3 s, then `motion` from the impact at t = 3 s.
std::vector<double> run(const Stream& s, double impact_t) {
  PostureConfirmer confirmer;
  std::vector<double> confirmed;
  bool impact_added = false;
  for (std::size_t i = 0; i < s.t.size(); ++i) {
    confirmer.observe(s.t[i], s.gyro[i], s.accel[i]);
    if (!impact_added && s.t[i] >= impact_t) {
      confirmer.addImpact(s.t[i]);
      impact_added = true;
    }
    for (double c : confirmer.evaluate()) confirmed.push_back(c);
  }
  return confirmed;
}

}  // namespace

TEST(PostureConfirmation, FastPostureChangeConfirmsWithinDwellAfterTheTurn) {
  Stream s;
  s.add(0, 0, 300);
  // Fall: turn 90 deg in 0.2 s, then lie still.
  const double rate = (M_PI / 2) / 0.2;
  for (int i = 0; i < 20; ++i) {
    s.t.push_back(s.t.size() / kRate);
    s.gyro.push_back(Eigen::Vector3d(rate, 0, 0));
    s.accel.push_back(gravityAt(rate * (i + 1) / kRate));
  }
  s.add(0, M_PI / 2, 300);
  const auto confirmed = run(s, 3.0);
  ASSERT_EQ(confirmed.size(), 1u);
  // 50 deg is passed about 0.11 s into the turn; then a 0.1 s dwell.
  EXPECT_GT(confirmed[0], 3.15);
  EXPECT_LT(confirmed[0], 3.35);
}

TEST(PostureConfirmation, BriefTiltIsNotAFall) {
  Stream s;
  s.add(0, 0, 300);
  // Jump-like landing: 70 deg out and back within 0.12 s, then upright.
  const double rate = (70.0 * M_PI / 180) / 0.06;
  for (int i = 0; i < 12; ++i) {
    const double r = i < 6 ? rate : -rate;
    const double angle = i < 6 ? rate * (i + 1) / kRate : rate * (12 - i - 1) / kRate;
    s.t.push_back(s.t.size() / kRate);
    s.gyro.push_back(Eigen::Vector3d(r, 0, 0));
    s.accel.push_back(gravityAt(angle));
  }
  s.add(0, 0, 300);
  EXPECT_TRUE(run(s, 3.0).empty());
}

TEST(PostureConfirmation, SettledWindowFallbackDecidesWithoutGyroEvidence) {
  // The accelerometer shows a 90 deg change but the gyro saw no rotation, so
  // the VQF up direction only creeps (tau_acc = 3 s); the settled-window rule
  // confirms at exactly t + 1.5 s.
  Stream s;
  s.add(0, 0, 300);
  s.add(0, M_PI / 2, 300);
  const auto confirmed = run(s, 3.0);
  ASSERT_EQ(confirmed.size(), 1u);
  EXPECT_NEAR(confirmed[0], 4.5, 1e-9);
}

TEST(PostureConfirmation, SdkEmitsOneShotFallConfirmedOnlyWhenEnabled) {
  for (bool enabled : {false, true}) {
    ImuMotionHealthParams params;
    params.startup_duration_s = 0.2;
    params.startup_min_samples = 10;
    params.posture_confirmation = enabled;
    ImuMotionHealth pipeline(params);
    std::vector<ImuMotionEvent> events;
    auto feed = [&](double t, const Eigen::Vector3d& gyro, const Eigen::Vector3d& accel) {
      ImuSample sample;
      sample.timestamp = t;
      sample.gyro = gyro;
      sample.accel = accel;
      pipeline.process(sample);
      for (const ImuMotionEvent& e : pipeline.drainEvents()) events.push_back(e);
    };
    int k = 0;
    for (; k < 300; ++k) feed(k / kRate, Eigen::Vector3d::Zero(), gravityAt(0));
    feed(k++ / kRate, Eigen::Vector3d::Zero(), Eigen::Vector3d(0, 0, 40.0));  // impact
    const double rate = (M_PI / 2) / 0.2;
    for (int i = 0; i < 20; ++i, ++k) feed(k / kRate, Eigen::Vector3d(rate, 0, 0), gravityAt(rate * (i + 1) / kRate));
    for (int i = 0; i < 300; ++i, ++k) feed(k / kRate, Eigen::Vector3d::Zero(), gravityAt(M_PI / 2));

    std::vector<ImuMotionEvent> confirmed;
    for (const auto& e : events) {
      if (e.type == ImuEventType::kFallConfirmed) confirmed.push_back(e);
    }
    if (!enabled) {
      EXPECT_TRUE(confirmed.empty());
      continue;
    }
    ASSERT_EQ(confirmed.size(), 2u);
    EXPECT_EQ(confirmed[0].phase, ImuEventPhase::kStarted);
    EXPECT_EQ(confirmed[1].phase, ImuEventPhase::kEnded);
    EXPECT_EQ(confirmed[0].id, confirmed[1].id);
    EXPECT_DOUBLE_EQ(confirmed[1].duration_s, 0.0);
    EXPECT_GT(confirmed[0].timestamp, 3.0);
    EXPECT_LT(confirmed[0].timestamp, 3.4);
    EXPECT_STREQ(eventTypeName(confirmed[0].type), "fall_confirmed");
  }
}

TEST(PostureConfirmation, ProfileKeysAreStrict) {
  ImuMotionHealthParams params;
  std::string error;
  const std::string path = ::testing::TempDir() + "/posture_profile.yaml";
  {
    std::FILE* f = std::fopen(path.c_str(), "w");
    std::fputs("imu_motion_health:\n  posture_confirmation: true\n  posture_dwell_s: 0.2\n", f);
    std::fclose(f);
  }
  ASSERT_TRUE(loadProfileFile(path, &params, &error)) << error;
  EXPECT_TRUE(params.posture_confirmation);
  EXPECT_DOUBLE_EQ(params.posture_dwell_s, 0.2);
  {
    std::FILE* f = std::fopen(path.c_str(), "w");
    std::fputs("imu_motion_health:\n  posture_threshold_deg: 200\n", f);
    std::fclose(f);
  }
  EXPECT_FALSE(loadProfileFile(path, &params, &error));
}

TEST(VqfAttitude, SdkOrientationEqualsStandaloneVqfAfterStartup) {
  // The accuracy claim rests on BROAD (evaluation/broad_attitude.py); this
  // checks the wiring. At the end of the 1 s startup the SDK sets VQF's bias
  // to the startup gyro mean, replays the startup samples, and ends VQF's
  // initial averaging; the same steps on a standalone VQF must give exactly
  // the same orientation afterwards.
  ImuMotionHealthParams params;  // vqf_attitude is the default
  ImuMotionHealth pipeline(params);
  localization_zoo::attitude_estimation::VQFParams vqf_params;
  vqf_params.sampling_rate = kRate;
  localization_zoo::attitude_estimation::VQF reference(vqf_params);
  std::vector<ImuSample> startup;
  double max_diff = 0.0;
  for (int k = 0; k < 1500; ++k) {
    const double t = k / kRate;
    const double moving = t > 1.5 ? 1.0 : 0.0;
    const double angle = moving * 0.6 * std::sin(1.3 * (t - 1.5));
    ImuSample sample;
    sample.timestamp = t;
    sample.gyro = Eigen::Vector3d(moving * 0.6 * 1.3 * std::cos(1.3 * (t - 1.5)), 0.01, -0.02);
    sample.accel = gravityAt(angle) + Eigen::Vector3d(0.05 * std::sin(7.0 * t), 0.0, 0.0);
    const ImuMotionHealthState state = pipeline.process(sample);
    if (k <= 100) {  // the startup window is t in [0, 1.0]
      startup.push_back(sample);
      if (k == 100) {
        Eigen::Vector3d gyro_mean = Eigen::Vector3d::Zero();
        for (const ImuSample& s : startup) gyro_mean += s.gyro;
        reference.setBiasEstimate(gyro_mean / startup.size());
        for (const ImuSample& s : startup) reference.update(s.gyro, s.accel);
        reference.endInitialAveraging();
      }
    } else {
      reference.update(sample.gyro, sample.accel);
    }
    if (k >= 100) {
      ASSERT_TRUE(state.startup_complete);
      const auto q = reference.quat6D();
      const Eigen::Quaterniond expected(q[0], q[1], q[2], q[3]);
      max_diff = std::max(max_diff, state.orientation.angularDistance(expected));
    }
  }
  EXPECT_LT(max_diff, 1e-9);
}

TEST(VqfAttitude, MagnetometerGivesAnEnuHeading) {
  // Static sensor yawed 70 deg from ENU, with an ENU field pointing north
  // (+y) and down. With magnetometer samples the SDK orientation converges to
  // the true ENU orientation (heading included); without them only the
  // inclination is defined.
  const Eigen::Quaterniond truth(Eigen::AngleAxisd(70.0 * M_PI / 180.0, Eigen::Vector3d::UnitZ()) *
                                 Eigen::AngleAxisd(0.3, Eigen::Vector3d::UnitX()));
  const Eigen::Vector3d acc = truth.inverse() * Eigen::Vector3d(0, 0, kGravity);
  const Eigen::Vector3d field = truth.inverse() * Eigen::Vector3d(0, 20, -40);
  for (bool with_mag : {false, true}) {
    ImuMotionHealth pipeline{ImuMotionHealthParams()};
    ImuMotionHealthState state;
    for (int k = 0; k < 6000; ++k) {
      ImuSample sample;
      sample.timestamp = k / kRate;
      sample.accel = acc;
      sample.mag = field;
      sample.has_mag = with_mag;
      state = pipeline.process(sample);
    }
    EXPECT_EQ(state.magnetometer_used, with_mag);
    // VQF accepts its first field reference only after ~5 s of rotation
    // (>= 20 deg/s); a sensor that never moves keeps the flag set ("no
    // trusted field yet") while heading is corrected at the reduced gain.
    EXPECT_EQ(state.magnetic_disturbance, with_mag);
    const double heading_error = state.orientation.angularDistance(truth) * 180.0 / M_PI;
    if (with_mag) {
      EXPECT_LT(heading_error, 0.5);
    } else {
      // Inclination still right, heading arbitrary.
      const double tilt = std::acos((state.orientation * Eigen::Vector3d::UnitZ()).z());
      EXPECT_NEAR(tilt, 0.3, 0.01);
    }
  }
}
