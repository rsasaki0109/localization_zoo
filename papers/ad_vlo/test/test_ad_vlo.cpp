#include "ad_vlo/ad_vlo.h"

#include <gtest/gtest.h>

#include <Eigen/Geometry>

#include <cmath>
#include <random>
#include <vector>

using namespace localization_zoo::ad_vlo;

namespace {

AdVloParams testParams() {
  AdVloParams params;
  auto& p = params.backend;
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
  p.max_visual_points = 180;
  p.max_lidar_lines = 90;
  p.max_visual_lines = 80;
  p.visual_point_curvature_threshold = 0.05;
  p.visual_min_segment_pixels = 4;
  p.visual_range_jump = 1.2;
  p.line_linearity_threshold = 0.55;
  p.visual_linearity_threshold = 0.55;
  p.local_map_radius = 35.0;
  return params;
}

std::vector<Eigen::Vector3d> makeWorld(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::uniform_real_distribution<double> xy(-10.0, 10.0);
  std::uniform_real_distribution<double> z(-1.0, 3.0);
  std::normal_distribution<double> n(0.0, 0.01);
  for (int i = 0; i < 4500; ++i) pts.emplace_back(xy(rng), xy(rng), n(rng));
  for (int i = 0; i < 2500; ++i) pts.emplace_back(xy(rng), 9.0 + n(rng), z(rng));
  for (int i = 0; i < 2500; ++i) pts.emplace_back(-9.0 + n(rng), xy(rng), z(rng));
  for (double x = -8.0; x <= 8.0; x += 0.12) {
    pts.emplace_back(x, 4.0 + n(rng), 1.2 + n(rng));
    pts.emplace_back(x, -4.0 + n(rng), 1.8 + n(rng));
  }
  return pts;
}

std::vector<Eigen::Vector3d> scanFromPose(const std::vector<Eigen::Vector3d>& world,
                                          const Eigen::Vector3d& t) {
  std::vector<Eigen::Vector3d> scan;
  scan.reserve(world.size());
  for (const auto& p_w : world) {
    const Eigen::Vector3d p_s = p_w - t;
    const double r = p_s.norm();
    if (r > 0.2 && r < 32.0) scan.push_back(p_s);
  }
  return scan;
}

}  // namespace

TEST(AdVlo, AppliesDirectPlaneProfile) {
  const auto p = AdVloPipeline::makeBackendParams(testParams());
  EXPECT_GT(p.plane_weight, 1.0);
  EXPECT_GT(p.visual_point_curvature_threshold, 0.1);
  EXPECT_EQ(p.line_weight, 0.0);
}

TEST(AdVlo, ExtractsOcclusionAwareVisualPoints) {
  std::mt19937 rng(16);
  AdVloPipeline pipe(testParams());
  const auto features = pipe.extractFeatures(scanFromPose(makeWorld(rng), {0.0, 0.0, 0.0}));
  EXPECT_GT(features.visual_points.size(), static_cast<size_t>(8));
  EXPECT_GT(features.registration_points.size(), static_cast<size_t>(100));
}

TEST(AdVlo, TracksShortTranslation) {
  std::mt19937 rng(17);
  const auto world = makeWorld(rng);
  AdVloPipeline pipe(testParams());
  pipe.registerFrame(scanFromPose(world, {0.0, 0.0, 0.0}));
  for (int i = 1; i <= 4; ++i) {
    const auto r = pipe.registerFrame(scanFromPose(world, {0.2 * i, 0.0, 0.0}));
    EXPECT_GT(r.backend.num_plane_correspondences, 0) << i;
  }
  EXPECT_LT((pipe.pose().block<3, 1>(0, 3) - Eigen::Vector3d(0.8, 0.0, 0.0)).norm(), 0.35);
}
