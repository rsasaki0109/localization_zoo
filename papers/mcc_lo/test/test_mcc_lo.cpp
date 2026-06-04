#include "mcc_lo/mcc_lo.h"

#include <gtest/gtest.h>

#include <vector>

using namespace localization_zoo::mcc_lo;

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

MccLoParams baseParams() {
  MccLoParams p;
  p.voxel_size = 1.0;
  return p;
}

}  // namespace

// (1) 相関エントロピー (Welsch/Gauss) 重み: 残差 0 で 1、外れ値で指数減衰し、σ が
//     大きいほど同じ残差への重みが大きい (寛容)。
TEST(MccLo, CorrentropyWeightDownweightsOutliers) {
  const double sigma = 0.3;
  EXPECT_NEAR(MccLoPipeline::correntropyWeight(0.0, sigma), 1.0, 1e-9);
  // 大残差は強く減衰。
  const double w_far = MccLoPipeline::correntropyWeight(1.0, sigma);
  EXPECT_LT(w_far, 0.01);
  // σ を広げると同じ残差でも重みが残る (広いカーネル = 寛容)。
  const double w_wide = MccLoPipeline::correntropyWeight(1.0, 2.0);
  EXPECT_GT(w_wide, w_far);
  // 全重みは (0,1]。
  EXPECT_GT(w_far, 0.0);
  EXPECT_LE(w_far, 1.0);
}

// (2) 良条件で前進並進を回復する。
TEST(MccLo, RecoversForwardTranslation) {
  MccLoPipeline pipe(baseParams());
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
  EXPECT_GT(res.sigma_used, 0.0);
}

// (3) 高いアウトライア比でも相関エントロピー重みが前進を回復する (外れ値を指数棄却)。
TEST(MccLo, RobustToHighOutlierRatio) {
  MccLoPipeline pipe(baseParams());
  const auto box = makeBox();
  pipe.registerFrame(box);

  Eigen::Matrix4d m1 = Eigen::Matrix4d::Identity();
  m1(0, 3) = 0.5;
  pipe.registerFrame(transformAll(box, m1.inverse()));

  Eigen::Matrix4d m2 = Eigen::Matrix4d::Identity();
  m2(0, 3) = 1.0;
  auto frame = transformAll(box, m2.inverse());
  // 半数の点を大きくずらして外れ値にする。
  for (size_t i = 0; i < frame.size(); i += 2) frame[i].y() += (i % 4 ? 1.5 : -1.5);

  const auto res = pipe.registerFrame(frame);
  EXPECT_NEAR(res.pose(0, 3), 1.0, 0.2);
}
