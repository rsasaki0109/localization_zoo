#include "kc_lo/kc_lo.h"

#include <gtest/gtest.h>

#include <vector>

using namespace localization_zoo::kc_lo;

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

KcLoParams baseParams() {
  KcLoParams p;
  p.voxel_size = 1.0;
  return p;
}

}  // namespace

// (1) ガウスカーネル親和度は距離 0 で 1、距離で指数減衰し、σ が広いほど寛容。
TEST(KcLo, KernelWeightDecaysWithDistance) {
  const double sigma = 0.5;
  EXPECT_NEAR(KcLoPipeline::kernelWeight(0.0, sigma), 1.0, 1e-9);
  const double w_near = KcLoPipeline::kernelWeight(0.25, sigma);  // d=0.5
  const double w_far = KcLoPipeline::kernelWeight(4.0, sigma);    // d=2.0
  EXPECT_GT(w_near, w_far);
  EXPECT_GT(w_far, 0.0);
  // σ を広げると同じ距離でも親和度が残る。
  EXPECT_GT(KcLoPipeline::kernelWeight(4.0, 2.0), w_far);
}

// (2) 親和度重み平均は近いモデル点が支配する (mean-shift 的ターゲット)。
//     クエリに近いクラスタへ ȳ が寄り、その重心付近になる。
TEST(KcLo, WeightedMeanPulledToNearbyDensity) {
  // 近いクラスタ (x≈0 付近に3点) と遠い1点 (x=5)。
  std::vector<Eigen::Vector3d> pts = {
      {0.1, 0, 0}, {-0.1, 0.1, 0}, {0.0, -0.1, 0.1}, {5.0, 0, 0}};
  double w = 0.0;
  const Eigen::Vector3d ym =
      KcLoPipeline::weightedMean(pts, Eigen::Vector3d(0, 0, 0), 0.5, &w);
  // 近いクラスタの重心 (~原点) 付近に来る (遠点 x=5 はほぼ無視)。
  EXPECT_LT(ym.x(), 0.5);
  EXPECT_GT(w, 0.0);
}

// (3) 対応点を取らずに前進並進を回復する (correspondence-free, σ アニーリング)。
TEST(KcLo, RecoversForwardTranslation) {
  KcLoPipeline pipe(baseParams());
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
  EXPECT_GT(res.num_correspondences, 0);
  EXPECT_GT(res.mean_weight, 0.0);
}
