#include "icpsc/icpsc.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::icpsc;

namespace {

std::vector<PointI> makeBox(double half = 10.0, double step = 0.5,
                            float intensity = 0.3f) {
  std::vector<PointI> pts;
  for (double a = -half; a <= half; a += step)
    for (double b = -half; b <= half; b += step) {
      pts.push_back({Eigen::Vector3d(a, b, -half), intensity});
      pts.push_back({Eigen::Vector3d(a, b, half), intensity});
      pts.push_back({Eigen::Vector3d(a, -half, b), intensity});
      pts.push_back({Eigen::Vector3d(a, half, b), intensity});
      pts.push_back({Eigen::Vector3d(-half, a, b), intensity});
      pts.push_back({Eigen::Vector3d(half, a, b), intensity});
    }
  return pts;
}

CylindricalParams defaultCyl() {
  CylindricalParams p;
  p.width = 256;
  p.height = 32;
  return p;
}

}  // namespace

TEST(Icpsc, DescriptorDistanceSeparatesIntensity) {
  const auto cyl_a = IcpscCore::projectCylindrical(makeBox(6.0, 0.5, 0.2f),
                                                   defaultCyl());
  const auto cyl_b = IcpscCore::projectCylindrical(makeBox(6.0, 0.5, 0.9f),
                                                   defaultCyl());
  const auto desc_a = IcpscCore::buildDescriptor(cyl_a, 12, 36);
  const auto desc_b = IcpscCore::buildDescriptor(cyl_b, 12, 36);
  const auto desc_a2 = IcpscCore::buildDescriptor(cyl_a, 12, 36);
  EXPECT_LT(IcpscCore::descriptorDistance(desc_a, desc_a2), 0.05);
  EXPECT_GT(IcpscCore::descriptorDistance(desc_a, desc_b),
            IcpscCore::descriptorDistance(desc_a, desc_a2));
}

TEST(Icpsc, AdaptiveFusionPrefersGeometry) {
  const double w_rich = IcpscCore::adaptiveGeomWeight(400, 50, 1.0, 0.2, 0.95);
  const double w_poor = IcpscCore::adaptiveGeomWeight(20, 200, 1.0, 0.2, 0.95);
  EXPECT_GT(w_rich, w_poor);
  EXPECT_GT(w_rich, 0.7);
  EXPECT_LT(w_poor, 0.4);
}

std::vector<PointI> transformAll(const std::vector<PointI>& pts,
                                 const Eigen::Matrix4d& T) {
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  std::vector<PointI> out(pts.size());
  for (size_t i = 0; i < pts.size(); i++) {
    out[i].p = R * pts[i].p + t;
    out[i].intensity = pts[i].intensity;
  }
  return out;
}

TEST(Icpsc, RecoversForwardTranslation) {
  IcpscParams p;
  p.cylindrical.width = 256;
  p.cylindrical.height = 32;
  p.voxel_size = 1.0;
  p.planarity_threshold = 0.3;
  IcpscPipeline pipe(p);
  const auto box = makeBox();
  pipe.registerFrame(box);

  Eigen::Matrix4d m1 = Eigen::Matrix4d::Identity();
  m1(0, 3) = 0.5;
  pipe.registerFrame(transformAll(box, m1.inverse()));

  Eigen::Matrix4d m2 = Eigen::Matrix4d::Identity();
  m2(0, 3) = 1.0;
  const auto res = pipe.registerFrame(transformAll(box, m2.inverse()));

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 1.0, 0.25);
}
