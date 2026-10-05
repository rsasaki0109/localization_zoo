#include "attitude_estimation/attitude_estimation.h"
#include "attitude_estimation/vqf.h"

#include <gtest/gtest.h>

#include <cmath>

using namespace localization_zoo::attitude_estimation;

namespace {

constexpr double kDeg = M_PI / 180.0;

Quat axisAngle(const Eigen::Vector3d& axis, double angle) {
  const Eigen::Vector3d a = axis.normalized() * std::sin(angle / 2);
  return Quat(std::cos(angle / 2), a.x(), a.y(), a.z());
}

// Specific force and field measured by a static sensor with orientation q
// (sensor relative to an x-north, z-up earth frame).
void staticMeasurement(const Quat& q, Eigen::Vector3d* acc, Eigen::Vector3d* mag) {
  const Quat qi = quatConjugate(q);
  *acc = quatRotate(qi, Eigen::Vector3d(0, 0, 9.81));
  *mag = quatRotate(qi, Eigen::Vector3d(20, 0, -40));
}

double angleBetween(const Quat& a, const Quat& b) {
  const Quat d = quatMultiply(a, quatConjugate(b));
  return 2 * std::acos(std::min(1.0, std::abs(d[0]) / d.norm()));
}

}  // namespace

TEST(AttitudeEstimation, MetricSeparatesHeadingAndInclination) {
  const std::vector<Quat> ref(4, Quat(1, 0, 0, 0));
  const std::vector<bool> movement = {true, true, false, true};
  // 10 deg heading error on the moving samples, 90 deg on a rest sample.
  std::vector<Quat> est = {axisAngle({0, 0, 1}, 10 * kDeg), axisAngle({0, 0, 1}, 10 * kDeg),
                           axisAngle({0, 0, 1}, 90 * kDeg), axisAngle({0, 0, 1}, 10 * kDeg)};
  OrientationErrors e = broadErrors(est, ref, movement);
  EXPECT_EQ(e.samples, 3);
  EXPECT_NEAR(e.heading_rmse_deg, 10.0, 1e-9);
  EXPECT_NEAR(e.inclination_rmse_deg, 0.0, 1e-6);
  EXPECT_NEAR(e.total_rmse_deg, 10.0, 1e-9);

  est.assign(4, axisAngle({1, 0, 0}, 5 * kDeg));
  e = broadErrors(est, ref, movement);
  EXPECT_NEAR(e.inclination_rmse_deg, 5.0, 1e-9);
  EXPECT_NEAR(e.heading_rmse_deg, 0.0, 1e-9);
}

TEST(AttitudeEstimation, MetricSkipsMissingReference) {
  std::vector<Quat> ref(2, Quat(1, 0, 0, 0));
  ref[1] = Quat(NAN, NAN, NAN, NAN);
  const std::vector<Quat> est = {axisAngle({0, 0, 1}, 4 * kDeg), axisAngle({0, 0, 1}, 90 * kDeg)};
  const OrientationErrors e = broadErrors(est, ref, {true, true});
  EXPECT_EQ(e.samples, 1);
  EXPECT_NEAR(e.heading_rmse_deg, 4.0, 1e-9);
}

TEST(AttitudeEstimation, InitialOrientationMatchesBroadExampleCode) {
  // Golden values from broad_utils.quatFromAccMag (BROAD example_code); the
  // first is the first sample of trial 01. The port must match the reference
  // protocol exactly, including its choice of earth frame.
  struct Case {
    Eigen::Vector3d acc, mag;
    Quat expected;
  };
  const Case cases[] = {
      {{-0.23618741817190653, -0.36316485200778093, 9.9193149826840603},
       {0.22260694406690718, 15.82182425322685, -38.337894271593719},
       {0.723799406673461, 0.021450369951014975, 0.004005922426010866, -0.6896653196396259}},
      {{3.0, -4.0, 8.0},
       {-20.0, 10.0, -30.0},
       {0.9266928231673417, 0.2565697620940609, 0.10086032508599715, 0.2554203662449141}},
  };
  for (const Case& c : cases) {
    EXPECT_LT((quatFromAccMag(c.acc, c.mag) - c.expected).norm(), 1e-12);
  }
}

TEST(AttitudeEstimation, FiltersConvergeFromWrongInitialState) {
  const Quat truth = axisAngle({1, 2, 0.5}, 30 * kDeg);
  Eigen::Vector3d acc, mag;
  staticMeasurement(truth, &acc, &mag);
  const Eigen::Vector3d gyr = Eigen::Vector3d::Zero();

  MadgwickParams mp;
  mp.beta = 0.5;
  mp.sampling_rate = 100;
  for (bool legacy : {false, true}) {
    mp.legacy_xio_field_scale = legacy;
    MadgwickFilter f(mp);
    for (int i = 0; i < 3000; ++i) f.update(gyr, acc, mag);
    EXPECT_LT(angleBetween(f.state(), truth), 0.5 * kDeg) << "legacy=" << legacy;
  }

  MahonyParams hp;
  hp.kp = 2.0;
  hp.ki = 0.01;
  hp.sampling_rate = 100;
  MahonyFilter h(hp);
  for (int i = 0; i < 3000; ++i) h.update(gyr, acc, mag);
  EXPECT_LT(angleBetween(h.state(), truth), 0.5 * kDeg);
}

TEST(AttitudeEstimation, SixDofKeepsHeadingAndCorrectsInclination) {
  const Quat truth = axisAngle({1, 0, 0}, 20 * kDeg);
  Eigen::Vector3d acc, mag;
  staticMeasurement(truth, &acc, &mag);
  MadgwickParams mp;
  mp.beta = 0.5;
  mp.sampling_rate = 100;
  MadgwickFilter f(mp);
  for (int i = 0; i < 3000; ++i) f.updateImu(Eigen::Vector3d::Zero(), acc);
  const OrientationErrors e = broadErrors({f.state()}, {truth}, {true});
  EXPECT_LT(e.inclination_rmse_deg, 0.5);
}

TEST(AttitudeEstimation, MahonyIntegralEstimatesGyroBias) {
  const Quat truth(1, 0, 0, 0);
  Eigen::Vector3d acc, mag;
  staticMeasurement(truth, &acc, &mag);
  const Eigen::Vector3d bias(0.01, -0.02, 0.015);
  MahonyParams hp;
  hp.kp = 1.0;
  hp.ki = 0.05;
  hp.sampling_rate = 100;
  MahonyFilter h(hp);
  for (int i = 0; i < 60000; ++i) h.update(bias, acc, mag);
  EXPECT_LT((h.integralFeedback() + bias).norm(), 1e-4);
  EXPECT_LT(angleBetween(h.state(), truth), 0.1 * kDeg);
}

TEST(AttitudeEstimation, LegacyFieldScaleOnlyChangesMagnetometerUpdate) {
  const Quat start = axisAngle({0, 0, 1}, 25 * kDeg);
  Eigen::Vector3d acc, mag;
  staticMeasurement(Quat(1, 0, 0, 0), &acc, &mag);
  const Eigen::Vector3d gyr(0.01, 0.02, -0.03);
  MadgwickParams a, b;
  a.beta = b.beta = 0.1;
  a.sampling_rate = b.sampling_rate = 100;
  b.legacy_xio_field_scale = true;
  MadgwickFilter fa(a), fb(b);
  fa.setState(start);
  fb.setState(start);
  fa.updateImu(gyr, acc);
  fb.updateImu(gyr, acc);
  EXPECT_LT((fa.state() - fb.state()).norm(), 1e-15);
  fa.update(gyr, acc, mag);
  fb.update(gyr, acc, mag);
  EXPECT_GT((fa.state() - fb.state()).norm(), 1e-6);
}

// ---------------------------------------------------------------------------
// VQF

namespace {

// Static ENU measurements for sensor orientation q (sensor -> ENU); the field
// points north (+y) and down.
void staticEnu(const Quat& q, Eigen::Vector3d* acc, Eigen::Vector3d* mag) {
  const Quat qi = quatConjugate(q);
  *acc = quatRotate(qi, Eigen::Vector3d(0, 0, 9.81));
  *mag = quatRotate(qi, Eigen::Vector3d(0, 20, -40));
}

}  // namespace

TEST(AttitudeEstimation, VqfConvergesToStaticOrientationIn9D) {
  const Quat truth = axisAngle({0.2, -0.4, 1.0}, 50 * kDeg);
  Eigen::Vector3d acc, mag;
  staticEnu(truth, &acc, &mag);
  VQFParams p;
  p.sampling_rate = 100;
  VQF vqf(p);
  for (int i = 0; i < 6000; ++i) vqf.update(Eigen::Vector3d::Zero(), acc, mag);
  EXPECT_LT(angleBetween(vqf.quat9D(), truth), 0.1 * kDeg);
  // 6D: inclination only.
  const OrientationErrors e = broadErrors({vqf.quat6D()}, {truth}, {true});
  EXPECT_LT(e.inclination_rmse_deg, 0.1);
}

TEST(AttitudeEstimation, VqfRestBiasEstimationRecoversGyroBias) {
  Eigen::Vector3d acc, mag;
  staticEnu(Quat(1, 0, 0, 0), &acc, &mag);
  const Eigen::Vector3d bias = Eigen::Vector3d(0.3, -0.5, 0.2) * kDeg;  // within the 2 deg/s clip
  VQFParams p;
  p.sampling_rate = 100;
  VQF vqf(p);
  for (int i = 0; i < 3000; ++i) vqf.update(bias, acc, mag);
  EXPECT_TRUE(vqf.restDetected());
  EXPECT_LT((vqf.biasEstimate() - bias).norm(), 0.01 * kDeg);

  p.rest_bias_estimation = p.motion_bias_estimation = false;
  VQF basic(p);
  for (int i = 0; i < 3000; ++i) basic.update(bias, acc, mag);
  EXPECT_FALSE(basic.restDetected());
  EXPECT_EQ(basic.biasEstimate().norm(), 0.0);
}

TEST(AttitudeEstimation, VqfSixDofOutputIgnoresMagnetometer) {
  Eigen::Vector3d acc, mag;
  staticEnu(axisAngle({1, 1, 0}, 20 * kDeg), &acc, &mag);
  VQFParams p;
  p.sampling_rate = 200;
  VQF with_mag(p), without_mag(p);
  const Eigen::Vector3d gyr(0.05, -0.02, 0.1);
  for (int i = 0; i < 2000; ++i) {
    with_mag.update(gyr, acc, mag * (1 + 0.5 * std::sin(i * 0.01)));  // disturbed field
    without_mag.update(gyr, acc);
  }
  EXPECT_LT((with_mag.quat6D() - without_mag.quat6D()).norm(), 1e-12);
}

TEST(AttitudeEstimation, VqfFlagsAChangedFieldAsDisturbance) {
  Eigen::Vector3d acc, mag;
  staticEnu(Quat(1, 0, 0, 0), &acc, &mag);
  VQFParams p;
  p.sampling_rate = 100;
  VQF vqf(p);
  // A reference is only accepted after 5 s of motion (low-passed |gyro| >= 20 deg/s).
  const Eigen::Vector3d spin(0, 0, 40 * kDeg);
  Quat q(1, 0, 0, 0);
  for (int i = 0; i < 1500; ++i) {
    q = quatNormalized(quatMultiply(q, axisAngle({0, 0, 1}, 40 * kDeg / 100)));
    staticEnu(q, &acc, &mag);
    vqf.update(spin, acc, mag);
  }
  EXPECT_FALSE(vqf.magDisturbanceDetected());
  // A 30 % stronger field (e.g. a nearby magnet) is a disturbance.
  for (int i = 0; i < 50; ++i) vqf.update(Eigen::Vector3d::Zero(), acc, 1.3 * mag);
  EXPECT_TRUE(vqf.magDisturbanceDetected());
}
