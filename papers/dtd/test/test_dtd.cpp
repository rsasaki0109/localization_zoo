#include "dtd/dtd.h"
#include <gtest/gtest.h>
#include <random>

using namespace localization_zoo::dtd;

namespace {
// 構造物 (柱・壁) のある都市様シーン。density 画像に強い特徴が出るよう
// 縦に伸びた点群クラスタ (柱) を散りばめる。
std::vector<Eigen::Vector3d> makeStructuredScene(std::mt19937& rng) {
  std::vector<Eigen::Vector3d> pts;
  std::normal_distribution<double> jit(0, 0.05);
  std::uniform_real_distribution<double> zc(0, 6);
  // 12 本の柱 (固定配置)。
  const double cols[12][2] = {{10, 5}, {-12, 8}, {15, -10}, {-8, -14},
                              {20, 18}, {-20, -5}, {5, 20}, {-15, 16},
                              {25, -3}, {-25, 10}, {8, -22}, {-3, 25}};
  for (auto& c : cols)
    for (int i = 0; i < 400; i++)
      pts.emplace_back(c[0] + jit(rng), c[1] + jit(rng), zc(rng));
  // 疎な地面。
  std::uniform_real_distribution<double> g(-30, 30);
  std::normal_distribution<double> gn(0, 0.01);
  for (int i = 0; i < 3000; i++) pts.emplace_back(g(rng), g(rng), gn(rng));
  return pts;
}
}  // namespace

TEST(DTD, ExtractsKeypoints) {
  std::mt19937 rng(1);
  DTDDetector det;
  const auto scene = makeStructuredScene(rng);
  const auto kps = det.extractKeypoints(scene);
  EXPECT_GE(kps.size(), 6u);  // 柱が keypoint として拾われる
}

TEST(DTD, DescribesTriangles) {
  std::mt19937 rng(1);
  DTDDetector det;
  const auto scene = makeStructuredScene(rng);
  const auto descs = det.describe(scene, 0);
  EXPECT_GT(descs.size(), 0u);
  // 辺長は昇順 (正準形)。
  for (const auto& d : descs) {
    EXPECT_LE(d.side[0], d.side[1] + 1e-9);
    EXPECT_LE(d.side[1], d.side[2] + 1e-9);
  }
}

TEST(DTD, DetectsLoopOnRevisit) {
  std::mt19937 rng(1);
  DTDDetector det;
  auto scene = makeStructuredScene(rng);

  // frame 0: 元の場所を登録。
  det.addToDatabase(det.describe(scene, 0));
  // frames 1..9: ダミー (遠くの別シーン) を登録して frame gap を作る。
  std::mt19937 rng2(99);
  for (int f = 1; f < 10; f++) {
    auto other = makeStructuredScene(rng2);
    for (auto& p : other) p += Eigen::Vector3d(500, 500, 0);  // 別の場所
    det.addToDatabase(det.describe(other, f));
  }

  // frame 10: 同じ場所を yaw 30° 回転して再訪 → ループ検出されるはず。
  const double yaw = 30.0 * M_PI / 180.0;
  Eigen::Matrix3d Rz;
  Rz << std::cos(yaw), -std::sin(yaw), 0, std::sin(yaw), std::cos(yaw), 0, 0, 0, 1;
  std::vector<Eigen::Vector3d> revisit;
  for (const auto& p : scene) revisit.push_back(Rz * p);
  const auto q = det.describe(revisit, 10);
  const auto loop = det.detectLoop(q, 10, 5);

  EXPECT_TRUE(loop.detected);
  EXPECT_EQ(loop.matched_frame, 0);  // 元の場所にマッチ
  EXPECT_GE(loop.inliers, 4);
}

TEST(DTD, NoFalseLoopOnDifferentScene) {
  std::mt19937 rng(1);
  DTDDetector det;
  auto scene = makeStructuredScene(rng);
  det.addToDatabase(det.describe(scene, 0));

  // 全く別の構造 (柱配置を変える)。
  std::mt19937 rng3(7);
  std::vector<Eigen::Vector3d> other;
  std::normal_distribution<double> jit(0, 0.05);
  std::uniform_real_distribution<double> pos(-28, 28), zc(0, 6);
  for (int c = 0; c < 12; c++) {
    double cx = pos(rng3), cy = pos(rng3);
    for (int i = 0; i < 400; i++) other.emplace_back(cx + jit(rng3), cy + jit(rng3), zc(rng3));
  }
  const auto q = det.describe(other, 10);
  const auto loop = det.detectLoop(q, 10, 5);
  EXPECT_FALSE(loop.detected);
}
