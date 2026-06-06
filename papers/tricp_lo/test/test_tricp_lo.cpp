#include "tricp_lo/tricp_lo.h"

#include <gtest/gtest.h>

#include <vector>

using namespace localization_zoo::tricp_lo;

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

TricpLoParams baseParams() {
  TricpLoParams p;
  p.voxel_size = 1.0;
  return p;
}

}  // namespace

// (1) FRMSD によるオーバーラップ自動推定: 70% inlier (小残差) + 30% outlier
//     (大残差) の集合で、推定 ξ ≈ 0.7 (outlier を含み始める直前) になる。
TEST(TricpLo, FrmsdEstimatesOverlapFraction) {
  std::vector<double> sq;
  for (int i = 0; i < 70; i++) sq.push_back(0.01);  // inlier 二乗残差
  for (int i = 0; i < 30; i++) sq.push_back(10.0);  // outlier 二乗残差
  const double xi = TricpLoPipeline::estimateOverlap(sq, 0.3, 1.5);
  EXPECT_NEAR(xi, 0.70, 0.03);

  // FRMSD は ξ=0.7 で ξ=1.0 より小さい (outlier を含めると悪化)。
  std::vector<double> sorted = sq;
  std::sort(sorted.begin(), sorted.end());
  EXPECT_LT(TricpLoPipeline::frmsd(sorted, 0.70, 1.5),
            TricpLoPipeline::frmsd(sorted, 1.00, 1.5));
}

// (2) トリム平均はアウトライアを順位で無視する (LTS): ξ=0.7 の trimmed-mean は
//     inlier のみ (~0.01) を反映し、全体平均 (~3.0) とは大きく異なる。
TEST(TricpLo, TrimmedMeanIgnoresOutliers) {
  std::vector<double> sorted;
  for (int i = 0; i < 70; i++) sorted.push_back(0.01);
  for (int i = 0; i < 30; i++) sorted.push_back(10.0);
  std::sort(sorted.begin(), sorted.end());

  EXPECT_NEAR(TricpLoPipeline::trimmedMeanSqResidual(sorted, 0.70), 0.01, 1e-6);
  EXPECT_NEAR(TricpLoPipeline::trimmedMeanSqResidual(sorted, 1.00), 3.007, 0.01);
}

// (3) 良好な初期整合 (パイプラインでは CV 予測が供給) のもとでアウトライア群を
//     注入すると、順位トリミング + オーバーラップ自動推定が外れ値を棄却し、姿勢が
//     引きずられない。トリミング無し (ξ=1) は同じ外れ値で大きくずれる。
//     TrICP/FICP は粗整合を前提とする手法であり、本テストはその有効領域を検証する。
TEST(TricpLo, TrimmingRejectsOutliers) {
  const auto box = makeBox();
  // 既整合フレーム (動きなし) に 3 割の外れ値ブロックを足す。
  auto frame = box;
  for (size_t i = 0; i < frame.size(); i += 3)
    frame[i] += Eigen::Vector3d(3.0, -3.0, 2.0);

  // robust: FRMSD 自動オーバーラップ (このシーンは外れ値 33% なので ξ 下限を下げる)。
  TricpLoParams rp = baseParams();
  rp.min_overlap = 0.6;
  TricpLoPipeline robust(rp);
  robust.registerFrame(box);
  const auto rr = robust.registerFrame(frame);

  // non-robust: トリミング無し (ξ=1) で全点 (外れ値含む) を使う。
  TricpLoParams np = baseParams();
  np.auto_overlap = false;
  np.overlap_ratio = 1.0;
  TricpLoPipeline plain(np);
  plain.registerFrame(box);
  const auto pr = plain.registerFrame(frame);

  // robust は外れ値を順位で棄却し ξ<1 を推定、姿勢はほぼ動かない。
  EXPECT_LT(rr.overlap, 1.0);
  EXPECT_GT(rr.num_inliers, 0);
  const double robust_disp = rr.pose.block<3, 1>(0, 3).norm();
  const double plain_disp = pr.pose.block<3, 1>(0, 3).norm();
  EXPECT_LT(robust_disp, 0.3);          // 外れ値に引きずられない。
  EXPECT_LT(robust_disp, plain_disp);   // トリミング無しより明確に頑健。
}
