#include "mesh_loam/mesh_loam.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::mesh_loam;

namespace {

/// Points on the walls/floor/ceiling of a box room, sampled on a grid in the
/// sensor frame (sensor at `origin` inside the room, axis-aligned).
std::vector<Eigen::Vector3d> makeRoomScan(const Eigen::Vector3d& origin,
                                          double half_x = 4.0, double half_y = 3.0,
                                          double height = 2.5, double step = 0.06) {
  std::vector<Eigen::Vector3d> pts;
  const double x0 = -half_x - origin.x(), x1 = half_x - origin.x();
  const double y0 = -half_y - origin.y(), y1 = half_y - origin.y();
  const double z0 = -origin.z(), z1 = height - origin.z();
  for (double x = x0; x <= x1; x += step) {
    for (double z = z0; z <= z1; z += step) {
      pts.emplace_back(x, y0, z);
      pts.emplace_back(x, y1, z);
    }
  }
  for (double y = y0; y <= y1; y += step) {
    for (double z = z0; z <= z1; z += step) {
      pts.emplace_back(x0, y, z);
      pts.emplace_back(x1, y, z);
    }
  }
  for (double x = x0; x <= x1; x += step) {
    for (double y = y0; y <= y1; y += step) {
      pts.emplace_back(x, y, z0);
    }
  }
  return pts;
}

MeshLoamParams testParams() {
  MeshLoamParams params;
  params.min_range = 0.2;
  params.max_range = 30.0;
  params.voxel_size = 0.1;
  params.mesh_update_interval = 1;
  params.min_correspondences = 20;
  return params;
}

}  // namespace

TEST(MeshLoam, FirstFrameBuildsSdfAndMesh) {
  MeshLoamPipeline pipeline(testParams());
  const auto res = pipeline.registerFrame(makeRoomScan(Eigen::Vector3d(0, 0, 1.2)));
  EXPECT_TRUE(res.converged);
  EXPECT_GT(pipeline.mapSize(), static_cast<size_t>(1000));
  EXPECT_GT(pipeline.meshSize(), static_cast<size_t>(500));
}

TEST(MeshLoam, StaticSensorStaysPut) {
  MeshLoamPipeline pipeline(testParams());
  const auto scan = makeRoomScan(Eigen::Vector3d(0, 0, 1.2));
  pipeline.registerFrame(scan);
  for (int i = 0; i < 3; ++i) {
    const auto res = pipeline.registerFrame(scan);
    EXPECT_TRUE(res.converged);
  }
  EXPECT_LT((pipeline.pose().block<3, 1>(0, 3)).norm(), 0.05);
}

TEST(MeshLoam, RecoversSmallTranslation) {
  MeshLoamPipeline pipeline(testParams());
  pipeline.registerFrame(makeRoomScan(Eigen::Vector3d(0, 0, 1.2)));
  const Eigen::Vector3d moved(0.15, 0.05, 0.0);
  const auto res =
      pipeline.registerFrame(makeRoomScan(Eigen::Vector3d(0, 0, 1.2) + moved));
  EXPECT_TRUE(res.converged);
  const Eigen::Vector3d est = res.pose.block<3, 1>(0, 3);
  EXPECT_LT((est - moved).norm(), 0.05);
}

TEST(MeshLoam, TracksTranslationSequence) {
  MeshLoamPipeline pipeline(testParams());
  const Eigen::Vector3d base(0, 0, 1.2);
  pipeline.registerFrame(makeRoomScan(base));
  const int steps = 8;
  const Eigen::Vector3d delta(0.15, 0.0, 0.0);
  for (int i = 1; i <= steps; ++i) {
    const auto res = pipeline.registerFrame(makeRoomScan(base + i * delta));
    EXPECT_TRUE(res.converged) << "frame " << i;
  }
  const Eigen::Vector3d expected = steps * delta;
  const Eigen::Vector3d est = pipeline.pose().block<3, 1>(0, 3);
  EXPECT_LT((est - expected).norm(), 0.1);
}

TEST(MeshLoam, RecoversSmallYaw) {
  MeshLoamPipeline pipeline(testParams());
  pipeline.registerFrame(makeRoomScan(Eigen::Vector3d(0, 0, 1.2)));
  // Rotate the same world points into a yawed sensor frame.
  const double yaw = 0.03;
  const Eigen::Matrix3d R_ws =
      Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  auto scan = makeRoomScan(Eigen::Vector3d(0, 0, 1.2));
  for (auto& p : scan) p = R_ws.transpose() * p;
  const auto res = pipeline.registerFrame(scan);
  EXPECT_TRUE(res.converged);
  const Eigen::Matrix3d R_est = res.pose.block<3, 3>(0, 0);
  const double yaw_est = std::atan2(R_est(1, 0), R_est(0, 0));
  EXPECT_NEAR(yaw_est, yaw, 0.01);
  EXPECT_LT((res.pose.block<3, 1>(0, 3)).norm(), 0.05);
}
