#include "student_t_lo/student_t_lo.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using namespace localization_zoo::student_t_lo;

namespace {

// 全方位を囲む箱 (良条件)。
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

std::vector<Eigen::Vector3d> transformAll(
    const std::vector<Eigen::Vector3d>& pts, const Eigen::Matrix4d& T) {
  std::vector<Eigen::Vector3d> out(pts.size());
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  for (size_t i = 0; i < pts.size(); i++) out[i] = R * pts[i] + t;
  return out;
}

StudentTLoParams baseParams() {
  StudentTLoParams p;
  p.voxel_size = 1.0;
  return p;
}

}  // namespace

// (1) Student's-t IRLS 重みの性質: r=0 で 1、|r| 増で単調減少、自由度 ν が小さいほど
//     大残差を強く減衰する (ν→∞ で Gaussian=減衰なし w≈1, ν 小 で頑健)。
TEST(StudentTLo, WeightDownweightsOutliers) {
  const double scale = 0.5;
  // r=0 で 1 (full inlier)。
  EXPECT_NEAR(StudentTLoPipeline::studentTWeight(0.0, scale, 5.0), 1.0, 1e-9);

  // 単調減少。
  const double w_small = StudentTLoPipeline::studentTWeight(0.3, scale, 5.0);
  const double w_large = StudentTLoPipeline::studentTWeight(1.5, scale, 5.0);
  EXPECT_LT(w_large, w_small);
  EXPECT_LT(w_small, 1.0);

  // 頑健性: 大残差では ν 小 (=2) の方が ν 大 (=100, ≈Gaussian) より強く減衰する。
  const double r = 2.0;
  const double w_robust = StudentTLoPipeline::studentTWeight(r, scale, 2.0);
  const double w_gaussian = StudentTLoPipeline::studentTWeight(r, scale, 100.0);
  EXPECT_LT(w_robust, w_gaussian);
  // ν→∞ 極限では大残差でも重み ≈ 1 (least-squares, 減衰しない)。
  EXPECT_GT(w_gaussian, 0.8);
}

// (2) 良条件で前進並進を回復する。
TEST(StudentTLo, RecoversForwardTranslation) {
  StudentTLoPipeline pipe(baseParams());
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
  EXPECT_GT(res.scale_used, 0.0);
}

// (3) アウトライア混入下で重尾重み付けが前進並進を頑健に回復する。
TEST(StudentTLo, RobustToOutliers) {
  auto run = [](bool heavy_tail) {
    StudentTLoParams p = baseParams();
    p.enable_student_t = heavy_tail;  // true=Student-t(重尾), false=Gaussian
    p.student_t_dof = 2.0;            // 強い重尾
    p.estimate_scale = false;         // 外れ値を取り込まないよう固定スケール
    p.scale_init = 0.2;
    StudentTLoPipeline pipe(p);
    const auto box = makeBox();
    pipe.registerFrame(box);

    Eigen::Matrix4d m1 = Eigen::Matrix4d::Identity();
    m1(0, 3) = 0.5;
    pipe.registerFrame(transformAll(box, m1.inverse()));

    Eigen::Matrix4d m2 = Eigen::Matrix4d::Identity();
    m2(0, 3) = 1.0;
    auto frame = transformAll(box, m2.inverse());
    // 1/3 の点を法線方向に ~1.0 m ずらして重尾アウトライアを混入 (決定的)。
    for (size_t i = 0; i < frame.size(); i += 3) {
      frame[i].y() += (i % 2 == 0 ? 1.0 : -1.0);
    }
    return pipe.registerFrame(frame).pose(0, 3);
  };

  const double x_heavy = run(true);
  // 重尾重み付けでアウトライアを減衰し前進量を回復する。
  EXPECT_NEAR(x_heavy, 1.0, 0.15);
}
