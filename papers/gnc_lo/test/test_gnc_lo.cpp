#include "gnc_lo/gnc_lo.h"

#include <gtest/gtest.h>

#include <vector>

using namespace localization_zoo::gnc_lo;

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

GncLoParams baseParams() {
  GncLoParams p;
  p.voxel_size = 1.0;
  return p;
}

}  // namespace

// (1) GNC-TLS 重み: μ→∞ で hard 判定 (inlier=1, outlier=0)、μ 小で far residual に
//     寛容 (継続法の凸スタート)。
TEST(GncLo, WeightGraduatesFromConvexToTls) {
  const double c = 0.3;
  // μ 大 (ほぼ TLS): 切断内=1、切断外=0。
  EXPECT_NEAR(GncLoPipeline::gncWeight(0.1, c, 100.0), 1.0, 1e-6);
  EXPECT_NEAR(GncLoPipeline::gncWeight(1.0, c, 100.0), 0.0, 1e-6);
  // μ 小 (凸寄り): 同じ大残差でも μ 大より重みが残る (広い収束域)。
  const double w_convex = GncLoPipeline::gncWeight(0.5, c, 0.05);
  const double w_tls = GncLoPipeline::gncWeight(0.5, c, 100.0);
  EXPECT_GT(w_convex, w_tls);
  // 全重みは [0,1]。
  EXPECT_GE(w_convex, 0.0);
  EXPECT_LE(w_convex, 1.0);
}

// (2) 良条件で前進並進を回復する。
TEST(GncLo, RecoversForwardTranslation) {
  GncLoPipeline pipe(baseParams());
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
  EXPECT_GT(res.inlier_ratio, 0.0);
}

// (3) 高いアウトライア比でも GNC が前進を回復する (TLS で外れ値を hard 棄却)。
TEST(GncLo, RobustToHighOutlierRatio) {
  GncLoPipeline pipe(baseParams());
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
