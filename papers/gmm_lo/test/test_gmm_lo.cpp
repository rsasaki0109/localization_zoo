#include "gmm_lo/gmm_lo.h"

#include <gtest/gtest.h>

#include <vector>

using namespace localization_zoo::gmm_lo;

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

GmmLoParams baseParams() {
  GmmLoParams p;
  p.voxel_size = 1.0;
  return p;
}

}  // namespace

// (1) ソフト重心: σ 小なら近いクラスタ寄り、σ 大なら両クラスタの中間へ寄る。
TEST(GmmLo, SoftCentroidBlendsWithSigma) {
  const Eigen::Vector3d query(0, 0, 0);
  // 近クラスタ (x≈1) と遠クラスタ (x≈5)。
  std::vector<Eigen::Vector3d> neighbors = {
      {1.0, 0, 0}, {1.1, 0.1, 0}, {0.9, -0.1, 0},
      {5.0, 0, 0}, {5.1, 0.1, 0}, {4.9, -0.1, 0}};

  const Eigen::Vector3d c_small = GmmLoPipeline::softCentroid(query, neighbors, 0.5);
  const Eigen::Vector3d c_large = GmmLoPipeline::softCentroid(query, neighbors, 5.0);

  // σ 小では近クラスタ (x≈1) に強く寄る。
  EXPECT_LT(c_small.x(), 1.5);
  // σ 大では遠クラスタの寄与が増え、より遠くへ引かれる。
  EXPECT_GT(c_large.x(), c_small.x());
}

// (2) 良条件で前進並進を回復する。
TEST(GmmLo, RecoversForwardTranslation) {
  GmmLoPipeline pipe(baseParams());
  const auto box = makeBox();
  pipe.registerFrame(box);

  Eigen::Matrix4d m1 = Eigen::Matrix4d::Identity();
  m1(0, 3) = 0.5;
  pipe.registerFrame(transformAll(box, m1.inverse()));

  Eigen::Matrix4d m2 = Eigen::Matrix4d::Identity();
  m2(0, 3) = 1.0;
  const auto res = pipe.registerFrame(transformAll(box, m2.inverse()));

  EXPECT_TRUE(res.converged);
  EXPECT_NEAR(res.pose(0, 3), 1.0, 0.12);
  EXPECT_NEAR(res.pose(1, 3), 0.0, 0.12);
  EXPECT_GT(res.mean_weight, 0.0);
}

// (3) アウトライア混入下で EM ソフト割当 (一様アウトライア項) が前進を回復する。
TEST(GmmLo, RobustToOutliers) {
  GmmLoPipeline pipe(baseParams());
  const auto box = makeBox();
  pipe.registerFrame(box);

  Eigen::Matrix4d m1 = Eigen::Matrix4d::Identity();
  m1(0, 3) = 0.5;
  pipe.registerFrame(transformAll(box, m1.inverse()));

  Eigen::Matrix4d m2 = Eigen::Matrix4d::Identity();
  m2(0, 3) = 1.0;
  auto frame = transformAll(box, m2.inverse());
  for (size_t i = 0; i < frame.size(); i += 3) frame[i].y() += (i % 2 ? 1.0 : -1.0);

  const auto res = pipe.registerFrame(frame);
  EXPECT_NEAR(res.pose(0, 3), 1.0, 0.2);
}
