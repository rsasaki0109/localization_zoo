#include "vlom/vlom.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::vlom;
using namespace localization_zoo::pl_loam;

TEST(Vlom, ScaleCorrectionRejectsOutlier) {
  const std::vector<double> lidar = {10.0, 10.2, 9.8, 10.1, 50.0};
  const std::vector<double> tri = {5.0, 5.1, 4.9, 5.05, 5.0};
  const double s = VlomCore::scaleCorrectionFactor(lidar, tri, 2.5);
  EXPECT_NEAR(s, 2.0, 0.15);
}

TEST(Vlom, ScaleCorrectionRecoversRatio) {
  const std::vector<double> lidar = {12.0, 18.0, 24.0, 30.0};
  const std::vector<double> tri = {4.0, 6.0, 8.0, 10.0};
  EXPECT_NEAR(VlomCore::scaleCorrectionFactor(lidar, tri), 3.0, 1e-6);
}

TEST(Vlom, TriangulateDepthPositive) {
  const CameraModel cam = CameraModel::kittiHalfRes();
  const Eigen::Matrix3d R = Eigen::Matrix3d::Identity();
  const Eigen::Vector3d t(0.2, 0.0, 0.0);
  double depth = 0.0;
  ASSERT_TRUE(VlomCore::triangulateDepth(cam, Eigen::Vector2d(300, 90),
                                         Eigen::Vector2d(305, 90), R, t,
                                         &depth));
  EXPECT_GT(depth, 1.0);
}

TEST(Vlom, CamVeloRoundtrip) {
  const CameraModel cam = CameraModel::kittiHalfRes();
  Eigen::Matrix4d T_cam = Eigen::Matrix4d::Identity();
  T_cam(0, 3) = 0.3;
  const Eigen::Matrix4d T_velo =
      VlomCore::camMotionToVelo(cam, T_cam);
  const Eigen::Matrix4d T_cam2 =
      cam.T_velo_cam * T_velo * cam.T_velo_cam.inverse();
  EXPECT_NEAR(T_cam2(0, 3), T_cam(0, 3), 1e-3);
}

TEST(Vlom, DefaultsToIntensityPseudoImageForKittiOdometry) {
  const VlomParams params;
  EXPECT_TRUE(params.visual.use_intensity_pseudo_image);
  EXPECT_EQ(params.visual.intensity_dilation_radius, 2);
  EXPECT_FALSE(params.visual.use_rgb_features);
}

TEST(Vlom, ClearResetsState) {
  Vlom pipeline;
  pipeline.clear();
  EXPECT_EQ(pipeline.frameCount(), 0);
  EXPECT_NEAR(pipeline.position().norm(), 0.0, 1e-9);
}
