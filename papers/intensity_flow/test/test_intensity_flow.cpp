#include "intensity_flow/intensity_flow.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::intensity_flow;

namespace {

std::vector<PointI> makeCube(double half = 10.0, double step = 0.5,
                             double intensity = 0.5) {
  std::vector<PointI> pts;
  auto add = [&](double x, double y, double z) {
    PointI pi;
    pi.p = Eigen::Vector3d(x, y, z);
    pi.intensity = intensity;
    pts.push_back(pi);
  };
  for (double a = -half; a <= half; a += step)
    for (double b = -half; b <= half; b += step) {
      add(a, b, -half);
      add(a, b, half);
      add(a, -half, b);
      add(a, half, b);
      add(-half, a, b);
      add(half, a, b);
    }
  return pts;
}

std::vector<PointI> transformAll(const std::vector<PointI>& pts,
                                 const Eigen::Matrix4d& T) {
  std::vector<PointI> out(pts.size());
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  for (size_t i = 0; i < pts.size(); i++) {
    out[i].p = R * pts[i].p + t;
    out[i].intensity = pts[i].intensity;
  }
  return out;
}

}  // namespace

// (1) gradient-flow サンプリングは境界 (高 ∆p) 点を優先する。
TEST(IntensityFlow, GradientFlowPrefersBoundary) {
  // 平面グリッド z=0。境界 (|x| or |y| が端) は片側近傍 → ∆p 大。
  std::vector<PointI> pts;
  const double lim = 10.0;
  int boundary_total = 0;
  for (double x = -lim; x <= lim; x += 1.0)
    for (double y = -lim; y <= lim; y += 1.0) {
      PointI pi;
      pi.p = Eigen::Vector3d(x, y, 0.0);
      pi.intensity = 0.5;
      pts.push_back(pi);
      if (std::abs(x) > lim - 0.5 || std::abs(y) > lim - 0.5) boundary_total++;
    }
  const int N = static_cast<int>(pts.size());
  const double input_frac = static_cast<double>(boundary_total) / N;

  const auto idx = gradientFlowSample(pts, /*radius=*/2.0, /*bins=*/10,
                                      /*keep_ratio=*/0.3);
  ASSERT_FALSE(idx.empty());
  int boundary_sel = 0;
  for (int i : idx) {
    const auto& p = pts[i].p;
    if (std::abs(p.x()) > lim - 0.5 || std::abs(p.y()) > lim - 0.5)
      boundary_sel++;
  }
  const double sel_frac = static_cast<double>(boundary_sel) / idx.size();
  // 選択集合の境界割合が入力割合より高い (gradient-flow が縁を優先)。
  EXPECT_GT(sel_frac, input_frac);
}

// (2) keep_ratio に応じた点数で決定的にサンプリングする。
TEST(IntensityFlow, SampleCountAndDeterminism) {
  const auto cube = makeCube();
  const auto a = gradientFlowSample(cube, 1.0, 10, 0.5);
  const auto b = gradientFlowSample(cube, 1.0, 10, 0.5);
  EXPECT_EQ(a.size(), b.size());
  EXPECT_EQ(a, b);  // 決定的
  EXPECT_NEAR(static_cast<double>(a.size()) / cube.size(), 0.5, 0.05);
}

// (3) intensity 重み付き matching でも既知の並進を回復する。
TEST(IntensityFlow, RecoversKnownTranslation) {
  IntensityFlowParams params;
  params.voxel_size = 1.0;
  params.max_icp_iterations = 50;
  params.enable_gradient_flow = true;
  params.enable_intensity = true;
  IntensityFlowPipeline pipe(params);

  const auto room = makeCube();
  pipe.registerFrame(room);

  Eigen::Matrix4d move = Eigen::Matrix4d::Identity();
  move(0, 3) = 0.5;
  move(1, 3) = -0.3;
  const auto moved = transformAll(room, move.inverse());
  const auto res = pipe.registerFrame(moved);

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 0.5, 0.12);
  EXPECT_NEAR(res.pose(1, 3), -0.3, 0.12);
  EXPECT_LT(res.num_sampled, res.num_input);  // サンプリングで削減
}
