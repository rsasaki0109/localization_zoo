#include "i_loam/i_loam.h"

#include <gtest/gtest.h>

#include <cmath>
#include <random>

using namespace localization_zoo::i_loam;

namespace {

using localization_zoo::aloam::PointCloud;
using localization_zoo::aloam::PointCloudPtr;
using localization_zoo::aloam::PointT;

// 構造ごとに固有の反射強度を持たせた合成スキャンを生成する。
// (地面 / 壁 / ポールで反射強度を変え、I-LOAM の強度経路を駆動する)
PointCloudPtr generateScan(const Eigen::Matrix4d& pose, std::mt19937& rng) {
  auto cloud = PointCloudPtr(new PointCloud);
  std::normal_distribution<float> noise(0.0f, 0.01f);
  std::normal_distribution<float> i_noise(0.0f, 0.02f);
  std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * M_PI);
  std::uniform_real_distribution<double> xy(-30.0, 30.0);
  std::uniform_real_distribution<double> z(0.0, 4.0);

  const Eigen::Matrix3d rotation = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d translation = pose.block<3, 1>(0, 3);

  auto add = [&](const Eigen::Vector3d& world_point, float reflectance) {
    const Eigen::Vector3d sensor_point =
        rotation.transpose() * (world_point - translation);
    PointT point;
    point.x = sensor_point.x() + noise(rng);
    point.y = sensor_point.y() + noise(rng);
    point.z = sensor_point.z() + noise(rng);
    point.intensity = std::min(1.0f, std::max(0.0f, reflectance + i_noise(rng)));
    cloud->push_back(point);
  };

  for (int i = 0; i < 3000; ++i) {
    add(Eigen::Vector3d(xy(rng), xy(rng), 0.0), 0.2f);  // 地面
  }
  for (int i = 0; i < 1000; ++i) {
    add(Eigen::Vector3d(xy(rng), 15.0, z(rng)), 0.6f);   // 壁A
    add(Eigen::Vector3d(-15.0, xy(rng), z(rng)), 0.55f);  // 壁B
  }
  for (int i = 0; i < 500; ++i) {
    const float theta = angle_dist(rng);
    add(Eigen::Vector3d(5.0 + 0.3 * std::cos(theta),
                        5.0 + 0.3 * std::sin(theta), z(rng)),
        0.9f);  // ポール(高反射)
  }
  return cloud;
}

}  // namespace

TEST(ILoam, IntensityWeightIsBoundedAndMonotone) {
  const double sigma = 0.15;
  EXPECT_DOUBLE_EQ(ILoam::intensityWeight(0.0, sigma), 1.0);
  const double w_small = ILoam::intensityWeight(0.1, sigma);
  const double w_large = ILoam::intensityWeight(0.4, sigma);
  EXPECT_GT(w_small, w_large);          // 差が大きいほど重みは小さい
  EXPECT_GT(w_large, 0.0);              // 正
  EXPECT_LE(w_small, 1.0);             // 1 以下
  // σ<=0 は重み無効化 (=1)
  EXPECT_DOUBLE_EQ(ILoam::intensityWeight(0.5, 0.0), 1.0);
  // 符号に対して対称
  EXPECT_DOUBLE_EQ(ILoam::intensityWeight(0.3, sigma),
                   ILoam::intensityWeight(-0.3, sigma));
}

TEST(ILoam, AugmentedDistanceAddsIntensityPenalty) {
  const double geom = 1.0;
  // Δ強度=0 または λ=0 なら幾何距離そのまま
  EXPECT_DOUBLE_EQ(ILoam::augmentedDistanceSq(geom, 0.0, 2.0), geom);
  EXPECT_DOUBLE_EQ(ILoam::augmentedDistanceSq(geom, 0.5, 0.0), geom);
  // 強度差はコストを増やす
  EXPECT_GT(ILoam::augmentedDistanceSq(geom, 0.5, 2.0), geom);
  EXPECT_DOUBLE_EQ(ILoam::augmentedDistanceSq(geom, 0.5, 2.0),
                   geom + 2.0 * 0.25);
}

TEST(ILoam, TracksShortSequence) {
  std::mt19937 rng(42);
  ILoam pipeline;
  ILoamResult last;
  for (int frame = 0; frame < 6; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = static_cast<double>(frame);
    last = pipeline.process(generateScan(pose, rng));
  }
  ASSERT_TRUE(last.valid);
  EXPECT_GT(last.num_edge_correspondences + last.num_plane_correspondences, 0u);
  EXPECT_GT(last.mean_intensity_weight, 0.0);
  EXPECT_LE(last.mean_intensity_weight, 1.0);
  EXPECT_LT((last.t_w_curr - Eigen::Vector3d(5.0, 0.0, 0.0)).norm(), 5.0);
}

TEST(ILoam, IntensityVariantsBothTrack) {
  // 強度経路の ON/OFF どちらでも破綻せず追従することを確認する。
  for (bool use_intensity : {false, true}) {
    std::mt19937 rng(7);
    ILoamParams params;
    params.use_intensity_weight = use_intensity;
    params.use_intensity_correspondence = use_intensity;
    ILoam pipeline(params);
    ILoamResult last;
    for (int frame = 0; frame < 6; ++frame) {
      Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
      pose(0, 3) = 0.8 * frame;
      last = pipeline.process(generateScan(pose, rng));
    }
    ASSERT_TRUE(last.valid);
    EXPECT_LT((last.t_w_curr - Eigen::Vector3d(4.0, 0.0, 0.0)).norm(), 5.0);
    if (!use_intensity) {
      EXPECT_DOUBLE_EQ(last.mean_intensity_weight, 1.0);
    }
  }
}

TEST(ILoam, ClearResetsState) {
  std::mt19937 rng(11);
  ILoam pipeline;
  for (int frame = 0; frame < 4; ++frame) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = 0.6 * frame;
    pipeline.process(generateScan(pose, rng));
  }
  EXPECT_GT(pipeline.frameCount(), 0);
  pipeline.clear();
  EXPECT_EQ(pipeline.frameCount(), 0);
  EXPECT_EQ(pipeline.position(), Eigen::Vector3d::Zero());
}
