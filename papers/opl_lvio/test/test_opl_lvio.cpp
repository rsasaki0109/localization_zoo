#include "opl_lvio/opl_lvio.h"

#include <gtest/gtest.h>

#include <Eigen/Geometry>

#include <algorithm>
#include <random>
#include <vector>

using namespace localization_zoo::opl_lvio;

namespace {

OplLvioParams testParams() {
  OplLvioParams p;
  p.min_range = 0.2;
  p.max_range = 35.0;
  p.image_width = 360;
  p.image_height = 80;
  p.fov_up_deg = 30.0;
  p.fov_down_deg = -50.0;
  p.voxel_size = 0.7;
  p.registration_voxel_size = 0.45;
  p.max_registration_points = 3500;
  p.max_icp_iterations = 35;
  p.max_correspondence_dist = 1.5;
  p.min_correspondences = 15;
  p.min_visual_points = 8;
  p.min_visual_lines = 3;
  p.max_visual_points = 180;
  p.visual_point_curvature_threshold = 0.05;
  p.visual_point_weight = 0.08;
  p.visual_min_segment_pixels = 4;
  p.visual_range_jump = 1.2;
  p.line_linearity_threshold = 0.55;
  p.visual_linearity_threshold = 0.55;
  p.local_map_radius = 35.0;
  return p;
}

std::vector<Eigen::Vector3d> makeWorld(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-10.0, 10.0);
  std::uniform_real_distribution<double> z(-1.0, 3.0);
  std::normal_distribution<double> n(0.0, 0.01);

  for (int i = 0; i < 5000; ++i) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 3500; ++i) pts.emplace_back(xy(rng), 10.0 + n(rng), z(rng));
  for (int i = 0; i < 3500; ++i) pts.emplace_back(-10.0 + n(rng), xy(rng), z(rng));
  for (int i = 0; i < 3500; ++i) pts.emplace_back(xy(rng), -10.0 + n(rng), z(rng));

  for (double x = -8.0; x <= 8.0; x += 0.12) {
    pts.emplace_back(x, 4.0 + n(rng), 1.2 + n(rng));
    pts.emplace_back(x, -4.0 + n(rng), 1.8 + n(rng));
  }
  return pts;
}

std::vector<Eigen::Vector3d> scanFromPose(const std::vector<Eigen::Vector3d>& world,
                                          const Eigen::Vector3d& t,
                                          double yaw = 0.0) {
  const Eigen::Matrix3d R =
      Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  std::vector<Eigen::Vector3d> scan;
  scan.reserve(world.size());
  for (const auto& p_w : world) {
    const Eigen::Vector3d p_s = R.transpose() * (p_w - t);
    const double r = p_s.norm();
    if (r > 0.2 && r < 32.0) scan.push_back(p_s);
  }
  return scan;
}

}  // namespace

TEST(OplLvio, ProjectsToSphericalImage) {
  OplLvioPipeline pipe(testParams());
  int row = -1;
  int col = -1;
  double range = 0.0;
  ASSERT_TRUE(pipe.projectToSpherical({10.0, 0.0, 0.0}, &row, &col, &range));
  EXPECT_NEAR(range, 10.0, 1e-9);
  EXPECT_GE(row, 0);
  EXPECT_LT(row, testParams().image_height);
  EXPECT_NEAR(col, testParams().image_width / 2, 1);
}

TEST(OplLvio, ExtractsOptimizedPointAndLineFeatures) {
  std::mt19937 rng(8);
  OplLvioPipeline pipe(testParams());
  const auto features = pipe.extractFeatures(scanFromPose(makeWorld(rng), {0.0, 0.0, 0.0}));
  EXPECT_GT(features.visual_points.size(), static_cast<size_t>(8));
  EXPECT_GT(features.lidar_lines.size(), static_cast<size_t>(5));
  EXPECT_GT(features.visual_lines.size(), static_cast<size_t>(2));
  EXPECT_GT(features.fused_lines.size(), static_cast<size_t>(5));
  const auto supported = std::count_if(
      features.fused_lines.begin(), features.fused_lines.end(),
      [](const LineFeature& l) { return l.visual_supported; });
  EXPECT_GT(supported, 0);
}

TEST(OplLvio, TracksTranslationSequence) {
  std::mt19937 rng(9);
  const auto world = makeWorld(rng);
  OplLvioPipeline pipe(testParams());
  pipe.registerFrame(scanFromPose(world, {0.0, 0.0, 0.0}));
  for (int i = 1; i <= 4; ++i) {
    const auto r = pipe.registerFrame(scanFromPose(world, {0.25 * i, 0.0, 0.0}));
    EXPECT_TRUE(r.converged) << i;
    EXPECT_GT(r.num_point_correspondences, 0) << i;
    EXPECT_GT(r.point_weight_scale, 0.0) << i;
  }
  const Eigen::Vector3d est = pipe.pose().block<3, 1>(0, 3);
  EXPECT_LT((est - Eigen::Vector3d(1.0, 0.0, 0.0)).norm(), 0.35);
  EXPECT_GT(pipe.mapSize(), static_cast<size_t>(100));
  EXPECT_GT(pipe.lineMapSize(), static_cast<size_t>(3));
}

TEST(OplLvio, FallsBackToLidarWhenVisualLinesAreSparse) {
  OplLvioParams params = testParams();
  params.min_visual_lines = 50;
  params.min_visual_points = 50;
  OplLvioPipeline pipe(params);

  std::vector<Eigen::Vector3d> scan;
  for (double x = 3.0; x < 12.0; x += 0.5) {
    scan.emplace_back(x, 0.1 * std::sin(x), 0.0);
    scan.emplace_back(x, 2.0, 0.2);
  }
  const auto r0 = pipe.registerFrame(scan);
  EXPECT_TRUE(r0.lidar_fallback);
  const auto r1 = pipe.registerFrame(scan);
  EXPECT_TRUE(r1.lidar_fallback);
  EXPECT_FALSE(r1.visual_used);
}
