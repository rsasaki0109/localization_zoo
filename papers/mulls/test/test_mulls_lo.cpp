#include "mulls/mulls_lo.h"

#include <gtest/gtest.h>

#include <cmath>
#include <random>

using namespace localization_zoo::mulls;

namespace {

constexpr double kDeg = M_PI / 180.0;

// Ground at z = -1.8, walls at y = +-8, a few poles; scanner at the origin.
std::vector<Eigen::Vector4d> makeScene(std::mt19937& rng, bool with_box = false) {
  std::normal_distribution<double> n(0.0, 0.005);
  std::vector<Eigen::Vector4d> pts;
  for (double x = -30.0; x <= 30.0; x += 0.1)
    for (double y = -7.9; y <= 7.9; y += 0.1) pts.emplace_back(x, y, -1.8 + n(rng), 40.0);
  for (double x = -30.0; x <= 30.0; x += 0.12)
    for (double z = -1.7; z <= 3.0; z += 0.12) {
      pts.emplace_back(x, 8.0 + n(rng), z, 80.0);
      pts.emplace_back(x, -8.0 + n(rng), z, 80.0);
    }
  // A cross wall breaks the corridor's translation symmetry along x.
  for (double y = -7.9; y <= 7.9; y += 0.12)
    for (double z = -1.7; z <= 3.0; z += 0.12) pts.emplace_back(30.0 + n(rng), y, z, 80.0);
  for (double px : {-12.0, -4.0, 5.0, 13.0, 21.0})
    for (double py : {-5.0, 5.0})
      for (double z = -1.7; z <= 2.5; z += 0.04)
        for (int a = 0; a < 6; ++a)
          pts.emplace_back(px + 0.08 * std::cos(a * M_PI / 3), py + 0.08 * std::sin(a * M_PI / 3), z,
                           120.0);
  if (with_box) {
    // A 1.5 m high box (a parked car) on the ground.
    for (double x = 9.0; x <= 13.0; x += 0.1)
      for (double y = 2.0; y <= 4.0; y += 0.1) pts.emplace_back(x, y, -0.3, 60.0);
  }
  return pts;
}

std::vector<Eigen::Vector4d> transformScene(const std::vector<Eigen::Vector4d>& pts,
                                            const Eigen::Isometry3d& T) {
  std::vector<Eigen::Vector4d> out;
  for (const auto& p : pts) {
    const Eigen::Vector3d q = T * Eigen::Vector3d(p.head<3>());
    out.emplace_back(q.x(), q.y(), q.z(), p(3));
  }
  return out;
}

MullsLOParams noCorrection() {
  MullsLOParams params;
  params.vertical_angle_correction_deg = 0.0;
  return params;
}

}  // namespace

TEST(MullsLO, ElevationCorrectionKeepsRange) {
  MullsLOParams params;
  const Eigen::Vector4d p(20.0, 5.0, -1.0, 10.0);
  const LOCloud out = preprocessScan({p}, params);
  ASSERT_EQ(out.size(), 1u);
  EXPECT_NEAR(out[0].p.norm(), p.head<3>().norm(), 1e-9);
  const double before = std::asin(p.z() / p.head<3>().norm());
  const double after = std::asin(out[0].p.z() / out[0].p.norm());
  EXPECT_NEAR((after - before) / kDeg, 0.195, 1e-9);
  EXPECT_NEAR(std::atan2(out[0].p.y(), out[0].p.x()), std::atan2(p.y(), p.x()), 1e-12);
}

TEST(MullsLO, ScannerFilterDropsNearAndUnderground) {
  const MullsLOParams params = noCorrection();
  const LOCloud out = preprocessScan({Eigen::Vector4d(1.6, 0.0, 0.0, 0.0),    // inside 1.75 m
                                      Eigen::Vector4d(10.0, 0.0, -6.0, 0.0),  // below -5.8 near
                                      Eigen::Vector4d(30.0, 0.0, -6.0, 0.0),  // ok beyond 20 m
                                      Eigen::Vector4d(30.0, 0.0, -8.0, 0.0),  // below -7.8
                                      Eigen::Vector4d(130.0, 0.0, 0.0, 0.0),  // beyond 120 m
                                      Eigen::Vector4d(10.0, 0.0, -1.8, 0.0)},
                                     params);
  ASSERT_EQ(out.size(), 2u);
  EXPECT_DOUBLE_EQ(out[0].p.x(), 30.0);
  EXPECT_DOUBLE_EQ(out[1].p.z(), -1.8);
}

TEST(MullsLO, GroundFilterSeparatesGroundFromObjects) {
  std::mt19937 rng(1);
  const MullsLOParams params = noCorrection();
  LOCloud ground, nonground;
  groundFilter(preprocessScan(makeScene(rng, true), params), params, &ground, &nonground);
  ASSERT_GT(ground.size(), 100u);
  // Ground candidates lie within 0.25 m of a cell's lowest point, so the base
  // of a wall can join the ground plane fit next to it.
  int flat = 0;
  for (const auto& pt : ground) {
    EXPECT_LT(std::abs(pt.p.z() + 1.8), 0.3);
    EXPECT_GT(pt.dir.z(), 0.8);
    flat += pt.dir.z() > 0.99 ? 1 : 0;
  }
  EXPECT_GT(flat, static_cast<int>(0.9 * ground.size()));
  // The box top (z = -0.3) and the walls are never ground.
  int box = 0;
  for (const auto& pt : nonground) box += std::abs(pt.p.z() + 0.3) < 0.01 ? 1 : 0;
  EXPECT_GT(box, 0);
  // Distance-inverse thinning keeps far ground sparser than near ground.
  int near_count = 0, far_count = 0;
  for (const auto& pt : ground) {
    const double r = std::hypot(pt.p.x(), pt.p.y());
    near_count += r < 10.0 ? 1 : 0;
    far_count += r > 20.0 ? 1 : 0;
  }
  EXPECT_GT(far_count, near_count);
}

TEST(MullsLO, RaisedCellIsNotGround) {
  // A 2 m step: the raised cells' lowest points are 2 m above their neighbours.
  const MullsLOParams params = noCorrection();
  LOCloud cloud;
  for (double x = 2.0; x < 22.0; x += 0.2)
    for (double y = -10.0; y < 10.0; y += 0.2) {
      LOPoint pt;
      pt.p = Eigen::Vector3d(x, y, (x > 11.0 && x < 14.0 && y > 1.0 && y < 4.0) ? 0.2 : -1.8);
      cloud.push_back(pt);
    }
  LOCloud ground, nonground;
  groundFilter(cloud, params, &ground, &nonground);
  for (const auto& pt : ground) EXPECT_LT(pt.p.z(), -1.0);
  int raised = 0;
  for (const auto& pt : nonground) raised += pt.p.z() > 0.0 ? 1 : 0;
  EXPECT_GT(raised, 0);
}

TEST(MullsLO, ClassifierFindsPillarsAndFacades) {
  std::mt19937 rng(2);
  const MullsLOParams params = noCorrection();
  const LOFeatures f = extractFeatures(makeScene(rng), params, &rng);
  ASSERT_GT(f.dense[kLOPillar].size(), 20u);
  ASSERT_GT(f.dense[kLOFacade].size(), 100u);
  for (const auto& pt : f.dense[kLOPillar]) EXPECT_GT(std::abs(pt.dir.z()), std::sin(70 * kDeg));
  for (const auto& pt : f.dense[kLOFacade]) EXPECT_LT(std::abs(pt.dir.z()), std::sin(20 * kDeg));
  // Sparse sets are capped.
  EXPECT_LE(f.sparse[kLOGround].size(), 800u);
  EXPECT_LE(f.sparse[kLOPillar].size(), 400u);
  EXPECT_LE(f.sparse[kLOFacade].size(), 1200u);
}

TEST(MullsLO, LowRailIsBeamHighRailIsNot) {
  MullsLOParams params = noCorrection();
  std::array<LOCloud, kLONumClasses> dense;
  LOCloud cloud;
  for (double z : {0.0, 1.5})
    for (double x = 5.0; x < 15.0; x += 0.02) {
      LOPoint pt;
      pt.p = Eigen::Vector3d(x, 3.0, z);
      cloud.push_back(pt);
    }
  std::mt19937 rng(3);
  classifyNonGround(cloud, params, &rng, &dense);
  ASSERT_FALSE(dense[kLOBeam].empty());
  for (const auto& pt : dense[kLOBeam]) EXPECT_LT(pt.p.z(), 0.5);
  EXPECT_TRUE(dense[kLOPillar].empty());
}

TEST(MullsLO, SaliencyNmsKeepsStrongestAndSpacing) {
  LOCloud cloud;
  for (int i = 0; i < 20; ++i) {
    LOPoint pt;
    pt.p = Eigen::Vector3d(0.05 * i, 0.0, 0.0);
    pt.saliency = (i == 7) ? 1.0 : 0.5;
    cloud.push_back(pt);
  }
  const LOCloud kept = saliencyNms(cloud, 0.175, 10);
  ASSERT_FALSE(kept.empty());
  EXPECT_DOUBLE_EQ(kept.front().p.x(), 0.35);
  for (size_t i = 0; i < kept.size(); ++i)
    for (size_t j = i + 1; j < kept.size(); ++j)
      EXPECT_GT((kept[i].p - kept[j].p).norm(), 0.175);
  cloud.resize(9);
  EXPECT_TRUE(saliencyNms(cloud, 0.175, 10).empty());
}

TEST(MullsLO, WeightSchedules) {
  EXPECT_NEAR(distanceWeight(0, 0.0, 30.0), 0.7, 1e-12);
  EXPECT_NEAR(distanceWeight(0, 30.0, 30.0), 1.0, 1e-12);
  EXPECT_NEAR(distanceWeight(20, 0.0, 30.0), 1.3, 1e-12);
  EXPECT_NEAR(distanceWeight(20, 90.0, 30.0), 0.4, 1e-12);
  EXPECT_DOUBLE_EQ(huberWeight(0.04, 0.05), 1.0);
  EXPECT_NEAR(huberWeight(0.1, 0.05), (2 * 0.1 * 0.05 - 0.05 * 0.05) / 0.01, 1e-12);
  std::array<int, kLONumClasses> pairs{};
  pairs[kLOGround] = 100;
  pairs[kLOFacade] = 50;
  pairs[kLOPillar] = 25;
  EXPECT_NEAR(groundBalanceWeight(pairs), 100.0 / 200.0001, 1e-6);
  pairs[kLOFacade] = 0;
  pairs[kLOPillar] = 0;
  EXPECT_DOUBLE_EQ(groundBalanceWeight(pairs), 0.01);
}

TEST(MullsLO, MultiMetricIcpRecoversPose) {
  std::mt19937 rng(4);
  const MullsLOParams params = noCorrection();
  const auto scene = makeScene(rng);
  Eigen::Isometry3d truth = Eigen::Isometry3d::Identity();  // target <- source
  truth.linear() = Eigen::AngleAxisd(2.0 * kDeg, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  truth.translation() = Eigen::Vector3d(0.3, -0.2, 0.0);
  const LOFeatures target = extractFeatures(scene, params, &rng);
  const LOFeatures source = extractFeatures(transformScene(scene, truth.inverse()), params, &rng);
  const LOICPResult r = multiMetricICP(source.sparse, target.dense, target.bound,
                                       Eigen::Isometry3d::Identity(), 2.4, params);
  EXPECT_FALSE(r.aborted);
  EXPECT_GT(r.pairs[kLOGround], 0);
  EXPECT_GT(r.pairs[kLOPillar] + r.pairs[kLOFacade], 20);
  const Eigen::Isometry3d err = truth.inverse() * r.T;
  EXPECT_LT(err.translation().norm(), 0.02);
  EXPECT_LT(Eigen::AngleAxisd(err.linear()).angle() / kDeg, 0.05);
}

TEST(MullsLO, IcpAbortsWithoutPairs) {
  MullsLOParams params = noCorrection();
  std::array<LOCloud, kLONumClasses> source, target;
  LOPoint pt;
  pt.p = Eigen::Vector3d(1.0, 0.0, 0.0);
  source[kLOFacade].push_back(pt);
  pt.p = Eigen::Vector3d(1.0, 0.0, 0.5);
  target[kLOFacade].push_back(pt);
  Eigen::AlignedBox3d bound(Eigen::Vector3d(-5, -5, -5), Eigen::Vector3d(5, 5, 5));
  Eigen::Isometry3d init = Eigen::Isometry3d::Identity();
  init.translation() = Eigen::Vector3d(0.1, 0.0, 0.0);
  const LOICPResult r = multiMetricICP(source, target, bound, init, 1.4, params);
  EXPECT_TRUE(r.aborted);
  EXPECT_TRUE(r.T.isApprox(init));
}

TEST(MullsLO, OdometryTracksConstantMotion) {
  std::mt19937 rng(5);
  MullsLOParams params = noCorrection();
  const auto scene = makeScene(rng);
  MullsLO lo(params);
  const Eigen::Vector3d velocity(0.8, 0.0, 0.0);  // per frame
  Eigen::Isometry3d last = Eigen::Isometry3d::Identity();
  for (int i = 0; i < 8; ++i) {
    Eigen::Isometry3d pose = Eigen::Isometry3d::Identity();
    pose.translation() = velocity * i;
    const auto r = lo.process(transformScene(scene, pose.inverse()));
    last = r.pose;
    if (i >= 3) EXPECT_TRUE(r.scan_to_map);
  }
  EXPECT_LT((last.translation() - velocity * 7).norm(), 0.1);
}

TEST(MullsLO, HighwayProfileRegistersBeams) {
  MullsLOParams params = MullsLOParams::kittiHighway();
  params.vertical_angle_correction_deg = 0.0;
  EXPECT_TRUE(params.use_beam);
  EXPECT_FALSE(params.use_ground_balance);
  EXPECT_FALSE(params.use_dynamic_removal);
  // Guard rails below the sensor along the road.
  std::mt19937 rng(6);
  auto scene = makeScene(rng);
  for (double x = -30.0; x <= 30.0; x += 0.03)
    for (double y : {-6.5, 6.5}) scene.emplace_back(x, y, -0.9, 90.0);
  const LOFeatures f = extractFeatures(scene, params, &rng);
  ASSERT_FALSE(f.sparse[kLOBeam].empty());
  EXPECT_LE(f.sparse[kLOBeam].size(), 400u);
  for (const auto& pt : f.sparse[kLOBeam]) EXPECT_LT(std::abs(pt.dir.z()), std::sin(15 * kDeg));

  Eigen::Isometry3d truth = Eigen::Isometry3d::Identity();
  truth.translation() = Eigen::Vector3d(0.4, 0.1, 0.0);
  const LOFeatures source = extractFeatures(transformScene(scene, truth.inverse()), params, &rng);
  const LOICPResult r = multiMetricICP(source.sparse, f.dense, f.bound,
                                       Eigen::Isometry3d::Identity(), 2.4, params);
  EXPECT_GT(r.pairs[kLOBeam], 0);
  EXPECT_LT((truth.inverse() * r.T).translation().norm(), 0.03);
}
