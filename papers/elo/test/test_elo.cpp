#include "elo/elo.h"

#include <gtest/gtest.h>

#include <Eigen/Geometry>

#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

using namespace localization_zoo::elo;

namespace {

std::vector<Eigen::Vector3d> makeWorldScene(double step = 0.08) {
  std::vector<Eigen::Vector3d> pts;
  for (double x = -6.0; x <= 8.0; x += step) {
    for (double y = -4.0; y <= 4.0; y += step) {
      pts.emplace_back(x, y, 0.0);
    }
  }
  for (double x = -6.0; x <= 8.0; x += step) {
    for (double z = 0.0; z <= 3.0; z += step) {
      pts.emplace_back(x, -4.0, z);
      pts.emplace_back(x, 4.0, z);
    }
  }
  for (double y = -4.0; y <= 4.0; y += step) {
    for (double z = 0.0; z <= 3.0; z += step) {
      pts.emplace_back(8.0, y, z);
    }
  }
  return pts;
}

std::vector<Eigen::Vector3d> scanFromPose(const Eigen::Vector3d& t,
                                          double yaw = 0.0) {
  const Eigen::Matrix3d R =
      Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  const auto world = makeWorldScene();
  std::vector<Eigen::Vector3d> scan;
  scan.reserve(world.size());
  for (const auto& p_w : world) {
    const Eigen::Vector3d p_s = R.transpose() * (p_w - t);
    const double r = p_s.norm();
    if (r > 0.2 && r < 30.0) scan.push_back(p_s);
  }
  return scan;
}

// The scan from a static pose with Gaussian range noise along each ray.
std::vector<Eigen::Vector3d> noisyStaticScan(std::mt19937* rng, double sigma) {
  std::normal_distribution<double> noise(0.0, sigma);
  auto scan = scanFromPose(Eigen::Vector3d(0.0, 0.0, 1.2));
  for (auto& p : scan) {
    const double r = p.norm();
    p *= (r + noise(*rng)) / r;
  }
  return scan;
}

EloParams testParams() {
  EloParams params;
  params.min_range = 0.2;
  params.max_range = 30.0;
  params.sri_width = 720;
  params.sri_height = 96;
  params.fov_up_deg = 35.0;
  params.fov_down_deg = -55.0;
  params.bev_x_min = -10.0;
  params.bev_x_max = 12.0;
  params.bev_y_min = -8.0;
  params.bev_y_max = 8.0;
  params.bev_resolution = 0.12;
  params.sensor_height = 1.2;
  params.ground_height_above = 0.15;
  params.ground_height_below = 0.2;
  params.max_iterations = 25;
  params.min_correspondences = 40;
  params.max_sri_correspondence_dist = 0.4;
  params.max_ground_correspondence_dist = 0.5;
  params.ground_search_radius_cells = 4;
  params.ground_normal_radius_cells = 5;
  params.max_alignment_points = 8000;
  return params;
}

}  // namespace

TEST(ELO, FirstFrameBuildsSphericalAndBevModels) {
  EloPipeline pipeline(testParams());
  const auto res = pipeline.registerFrame(scanFromPose(Eigen::Vector3d(0.0, 0.0, 1.2)));
  EXPECT_TRUE(res.converged);
  EXPECT_GT(res.num_ground_points, 100);
  EXPECT_GT(res.num_non_ground_points, 100);
  EXPECT_GT(pipeline.groundMapSize(), static_cast<size_t>(100));
  EXPECT_GT(pipeline.nonGroundMapSize(), static_cast<size_t>(100));
}

TEST(ELO, StaticSensorStaysPut) {
  EloPipeline pipeline(testParams());
  const auto scan = scanFromPose(Eigen::Vector3d(0.0, 0.0, 1.2));
  pipeline.registerFrame(scan);
  for (int i = 0; i < 3; ++i) {
    const auto res = pipeline.registerFrame(scan);
    EXPECT_TRUE(res.converged);
    EXPECT_GT(res.num_ground_correspondences, 20);
    EXPECT_GT(res.num_non_ground_correspondences, 20);
  }
  const Eigen::Vector3d est = pipeline.pose().block<3, 1>(0, 3);
  EXPECT_LT(est.norm(), 0.03);
}

TEST(ELO, RecoversSmallTranslation) {
  EloPipeline pipeline(testParams());
  pipeline.registerFrame(scanFromPose(Eigen::Vector3d(0.0, 0.0, 1.2)));
  const Eigen::Vector3d moved(0.18, 0.04, 0.0);
  const auto res = pipeline.registerFrame(scanFromPose(Eigen::Vector3d(0.0, 0.0, 1.2) + moved));
  EXPECT_TRUE(res.converged);
  const Eigen::Vector3d est = res.pose.block<3, 1>(0, 3);
  EXPECT_LT((est - moved).norm(), 0.06);
}

TEST(ELO, RecoversSmallYaw) {
  EloPipeline pipeline(testParams());
  pipeline.registerFrame(scanFromPose(Eigen::Vector3d(0.0, 0.0, 1.2)));
  const double yaw = 0.025;
  const auto res = pipeline.registerFrame(scanFromPose(Eigen::Vector3d(0.0, 0.0, 1.2), yaw));
  EXPECT_TRUE(res.converged);
  const Eigen::Matrix3d R = res.pose.block<3, 3>(0, 0);
  const double yaw_est = std::atan2(R(1, 0), R(0, 0));
  EXPECT_NEAR(yaw_est, yaw, 0.01);
}

TEST(ELO, TracksTranslationSequence) {
  EloPipeline pipeline(testParams());
  const Eigen::Vector3d base(0.0, 0.0, 1.2);
  pipeline.registerFrame(scanFromPose(base));
  const Eigen::Vector3d step(0.12, 0.0, 0.0);
  for (int i = 1; i <= 6; ++i) {
    const auto res = pipeline.registerFrame(scanFromPose(base + i * step));
    EXPECT_TRUE(res.converged) << "frame " << i;
  }
  const Eigen::Vector3d est = pipeline.pose().block<3, 1>(0, 3);
  EXPECT_LT((est - 6.0 * step).norm(), 0.12);
}

// Keeping the closer point on every SRI collision (the paper's update) turns
// range noise into a model that creeps towards the sensor. With only a front
// wall to fix x, that pulls a static sensor along x; fusing points on the
// same surface keeps it in place.
double staticNoisyDriftX(bool fuse_same_surface) {
  EloParams params = testParams();
  params.sri_fuse_same_surface = fuse_same_surface;
  EloPipeline pipeline(params);
  std::mt19937 rng(7);
  for (int i = 0; i < 12; ++i) pipeline.registerFrame(noisyStaticScan(&rng, 0.03));
  return pipeline.pose()(0, 3);
}

TEST(ELO, SameSurfaceFusionRemovesClosestPointCreep) {
  const double drift_closest = staticNoisyDriftX(false);
  const double drift_fused = staticNoisyDriftX(true);
  std::printf("static noisy drift x: closest %.4f m, fused %.4f m\n",
              drift_closest, drift_fused);
  EXPECT_LT(std::abs(drift_fused), 0.5 * std::abs(drift_closest));
  EXPECT_LT(std::abs(drift_fused), 0.01);
}

TEST(ELO, BiasOptionsStillTrackTranslationSequence) {
  EloParams params = testParams();
  params.sri_fuse_same_surface = true;
  params.ground_constrains_forward = false;
  EloPipeline pipeline(params);
  const Eigen::Vector3d base(0.0, 0.0, 1.2);
  pipeline.registerFrame(scanFromPose(base));
  const Eigen::Vector3d step(0.12, 0.0, 0.0);
  for (int i = 1; i <= 6; ++i) {
    const auto res = pipeline.registerFrame(scanFromPose(base + i * step));
    EXPECT_TRUE(res.converged) << "frame " << i;
  }
  const Eigen::Vector3d est = pipeline.pose().block<3, 1>(0, 3);
  EXPECT_LT((est - 6.0 * step).norm(), 0.12);
}
