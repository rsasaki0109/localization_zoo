#include "imls_slam/imls_slam.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::imls_slam;

namespace {

std::vector<Eigen::Vector3d> makeBox(double half = 10.0, double step = 0.5) {
  std::vector<Eigen::Vector3d> pts;
  for (double a = -half; a <= half; a += step)
    for (double b = -half; b <= half; b += step) {
      pts.emplace_back(a, b, -half);
      pts.emplace_back(a, b, half);
      pts.emplace_back(a, -half, b);
      pts.emplace_back(a, half, b);
      pts.emplace_back(-half, a, b);
      pts.emplace_back(half, a, b);
    }
  return pts;
}

std::vector<Eigen::Vector3d> transformAll(const std::vector<Eigen::Vector3d>& pts,
                                          const Eigen::Matrix4d& T) {
  std::vector<Eigen::Vector3d> out(pts.size());
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  for (size_t i = 0; i < pts.size(); i++) out[i] = R * pts[i] + t;
  return out;
}

ImlsSlamParams baseParams() {
  ImlsSlamParams p;
  p.voxel_size = 1.0;
  p.imls_h = 0.5;
  return p;
}

}  // namespace

// (1) IMLS 暗黙的曲面距離: 平面モデル (法線 +z) への符号つき距離は高さに一致し、
//     かつ距離で重み付けされる (近い有向点が支配 = Moving Least Squares の局所性)。
TEST(ImlsSlam, ImplicitSurfaceDistanceMatchesPlane) {
  // z=0 平面上の有向点群、法線は +z。
  std::vector<Eigen::Vector3d> pts, normals;
  for (double a = -1.0; a <= 1.0; a += 0.2)
    for (double b = -1.0; b <= 1.0; b += 0.2) {
      pts.emplace_back(a, b, 0.0);
      normals.emplace_back(0.0, 0.0, 1.0);
    }
  const double h = 0.5;
  // 平面上 → 距離 0。
  EXPECT_NEAR(ImlsSlamPipeline::imlsDistance({0.0, 0.0, 0.0}, pts, normals, h),
              0.0, 1e-9);
  // 平面の 0.5 m 上 → 距離 0.5 (横位置に依らず point-to-plane 成分)。
  EXPECT_NEAR(ImlsSlamPipeline::imlsDistance({0.3, -0.4, 0.5}, pts, normals, h),
              0.5, 1e-6);

  // 距離重み付けの検証: 近い平面点 (z=0) と遠い平面点 (z=2) を混在させると、
  // クエリ z=0.5 では近い点が支配し距離 ~0.5 になる (遠点単独なら -1.5)。
  std::vector<Eigen::Vector3d> mixed_p = {{0, 0, 0}, {0, 0, 2.0}};
  std::vector<Eigen::Vector3d> mixed_n = {{0, 0, 1}, {0, 0, 1}};
  const double d = ImlsSlamPipeline::imlsDistance({0, 0, 0.5}, mixed_p, mixed_n, h);
  EXPECT_NEAR(d, 0.5, 0.05);
}

// (2) 観測性サンプリング: 1 方向に偏ったスキャンでも、各軸を拘束する稀少な点を
//     拾い上げ、3 並進軸すべてが代表される。
TEST(ImlsSlam, ObservabilitySamplingCoversAllAxes) {
  std::vector<Eigen::Vector3d> pts, normals;
  // 大多数 (900) は法線 +z (z 拘束過多)。
  for (int i = 0; i < 900; i++) {
    pts.emplace_back(0.1 * (i % 30), 0.1 * (i / 30), 0.0);
    normals.emplace_back(0.0, 0.0, 1.0);
  }
  // 稀少な x 法線 (40) と y 法線 (40)。
  for (int i = 0; i < 40; i++) {
    pts.emplace_back(0.0, 0.2 * i, 1.0);
    normals.emplace_back(1.0, 0.0, 0.0);
  }
  for (int i = 0; i < 40; i++) {
    pts.emplace_back(0.2 * i, 0.0, 2.0);
    normals.emplace_back(0.0, 1.0, 0.0);
  }

  const auto sel = ImlsSlamPipeline::selectObservabilityPoints(pts, normals, 10);
  // サンプリングは全点を返さず (絞り込みが効いている)。
  EXPECT_LT(sel.size(), pts.size());

  bool has_x = false, has_y = false, has_z = false;
  for (int i : sel) {
    if (std::abs(normals[i].x()) > 0.9) has_x = true;
    if (std::abs(normals[i].y()) > 0.9) has_y = true;
    if (std::abs(normals[i].z()) > 0.9) has_z = true;
  }
  EXPECT_TRUE(has_x);  // 稀少な x 拘束点が選ばれる。
  EXPECT_TRUE(has_y);  // 稀少な y 拘束点が選ばれる。
  EXPECT_TRUE(has_z);
}

// (3) 良条件で前進並進を回復する (scan-to-implicit-model の end-to-end)。
TEST(ImlsSlam, RecoversForwardTranslation) {
  ImlsSlamPipeline pipe(baseParams());
  const auto box = makeBox();
  pipe.registerFrame(box);

  Eigen::Matrix4d m1 = Eigen::Matrix4d::Identity();
  m1(0, 3) = 0.5;
  pipe.registerFrame(transformAll(box, m1.inverse()));

  Eigen::Matrix4d m2 = Eigen::Matrix4d::Identity();
  m2(0, 3) = 1.0;
  const auto res = pipe.registerFrame(transformAll(box, m2.inverse()));

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 1.0, 0.15);
  EXPECT_NEAR(res.pose(1, 3), 0.0, 0.15);
  EXPECT_GT(res.num_samples, 0);
  EXPECT_GT(res.num_correspondences, 0);
}
