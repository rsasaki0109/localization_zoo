#include "imu_preintegration/imu_preintegration.h"

#include <gtest/gtest.h>
#include <cmath>

using namespace localization_zoo::imu_preintegration;

TEST(ImuPreintegration, StationaryPrediction) {
  // 静止状態: predict() で重力と相殺されて動かないことを確認
  ImuPreintegrator preint;
  Eigen::Vector3d accel(0, 0, 9.81);  // センサは上向きgを感じる

  for (int i = 0; i < 100; i++) {
    preint.integrate(Eigen::Vector3d::Zero(), accel, 0.01);
  }

  // predict() で重力を加算すると静止
  Eigen::Matrix3d R_j;
  Eigen::Vector3d v_j, p_j;
  Eigen::Vector3d gravity(0, 0, -9.81);
  preint.predict(Eigen::Matrix3d::Identity(), Eigen::Vector3d::Zero(),
                 Eigen::Vector3d::Zero(), gravity, R_j, v_j, p_j);

  EXPECT_NEAR(p_j.norm(), 0.0, 0.01);
  EXPECT_NEAR(v_j.norm(), 0.0, 0.01);
  EXPECT_NEAR((R_j - Eigen::Matrix3d::Identity()).norm(), 0.0, 1e-6);
  EXPECT_NEAR(preint.result().delta_t, 1.0, 1e-6);
}

TEST(ImuPreintegration, ConstantAcceleration) {
  // x方向に1m/s²で加速 + 重力補償
  ImuPreintegrator preint;
  Eigen::Vector3d accel(1.0, 0.0, 9.81);  // 1m/s² forward + gravity

  for (int i = 0; i < 100; i++) {
    preint.integrate(Eigen::Vector3d::Zero(), accel, 0.01);
  }

  auto& r = preint.result();
  // 1秒後: Δv ≈ [1, 0, 0], Δp ≈ [0.5, 0, 0]
  EXPECT_NEAR(r.delta_v.x(), 1.0, 0.01);
  EXPECT_NEAR(r.delta_p.x(), 0.5, 0.01);
  EXPECT_NEAR(r.delta_v.y(), 0.0, 0.01);
}

TEST(ImuPreintegration, PureRotation) {
  // z軸周り90°/sで回転
  ImuPreintegrator preint;
  double omega_z = M_PI / 2.0;  // 90°/s
  Eigen::Vector3d gyro(0, 0, omega_z);
  Eigen::Vector3d accel(0, 0, 9.81);

  for (int i = 0; i < 100; i++) {
    preint.integrate(gyro, accel, 0.01);
  }

  auto& r = preint.result();
  // 1秒後: 90°回転
  Eigen::AngleAxisd aa(r.delta_R);
  EXPECT_NEAR(aa.angle(), M_PI / 2.0, 0.05);
  EXPECT_NEAR(std::abs(aa.axis().z()), 1.0, 0.05);
}

TEST(ImuPreintegration, PredictPose) {
  // 初期状態から1秒間x方向に等速1m/s
  ImuPreintegrator preint;
  Eigen::Vector3d accel(0, 0, 9.81);

  for (int i = 0; i < 100; i++) {
    preint.integrate(Eigen::Vector3d::Zero(), accel, 0.01);
  }

  Eigen::Matrix3d R_i = Eigen::Matrix3d::Identity();
  Eigen::Vector3d v_i(1.0, 0, 0);  // x方向 1m/s
  Eigen::Vector3d p_i(0, 0, 0);
  Eigen::Vector3d gravity(0, 0, -9.81);

  Eigen::Matrix3d R_j;
  Eigen::Vector3d v_j, p_j;
  preint.predict(R_i, v_i, p_i, gravity, R_j, v_j, p_j);

  // 等速直線運動: p_j ≈ [1, 0, 0]
  EXPECT_NEAR(p_j.x(), 1.0, 0.01);
  EXPECT_NEAR(p_j.y(), 0.0, 0.01);
  EXPECT_NEAR(p_j.z(), 0.0, 0.01);
}

TEST(ImuPreintegration, BiasCorrection) {
  // バイアスありで積分し、1次補正が機能するか確認
  Eigen::Vector3d true_bg(0.01, 0, 0);
  Eigen::Vector3d true_ba(0.1, 0, 0);

  // バイアスなしで積分
  ImuPreintegrator preint_no_bias;
  // バイアスありで積分
  ImuPreintegrator preint_with_bias(ImuNoiseParams(), true_bg, true_ba);

  Eigen::Vector3d gyro(0, 0, 0);
  Eigen::Vector3d accel(0, 0, 9.81);

  for (int i = 0; i < 100; i++) {
    preint_no_bias.integrate(gyro, accel, 0.01);
    preint_with_bias.integrate(gyro, accel, 0.01);
  }

  // バイアスなしの結果を1次補正
  auto corrected = preint_no_bias.result().correct(true_bg, true_ba);

  // 補正後はバイアスありの結果に近づくはず
  double v_err_before =
      (preint_no_bias.result().delta_v - preint_with_bias.result().delta_v).norm();
  double v_err_after =
      (corrected.delta_v - preint_with_bias.result().delta_v).norm();

  EXPECT_LT(v_err_after, v_err_before);
}

TEST(ImuPreintegration, CovarianceGrows) {
  ImuPreintegrator preint;

  for (int i = 0; i < 100; i++) {
    preint.integrate(Eigen::Vector3d::Zero(), Eigen::Vector3d(0, 0, 9.81), 0.01);
  }

  // 共分散は時間とともに増加するはず
  double trace = preint.result().covariance.trace();
  EXPECT_GT(trace, 0.0);
}
