#include "inten_loam/inten_loam_paper.h"
#include "inten_loam/detail/feature_tree.h"
#include "inten_loam/detail/mapped_motion_prior.h"
#include "inten_loam/detail/plane_row_support.h"

#include <gtest/gtest.h>
#include <Eigen/Geometry>
#include <Eigen/Cholesky>

#include <cmath>
#include <stdexcept>

#include <limits>
#include <random>
#include <utility>
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace localization_zoo::inten_loam;

namespace {

// Street: ground at z = -1.7, walls at y = +-8 with recesses, end walls, a
// pole; sampled along HDL-64-like beams. Intensity: lane-marking stripes on
// the ground, brighter wall bands.
double castRay(const Eigen::Vector3d& o, const Eigen::Vector3d& d) {
  double best = 1e9;
  auto plane = [&](const Eigen::Vector3d& n, double off, auto inside) {
    const double den = n.dot(d);
    if (std::abs(den) < 1e-9) return;
    const double t = -(n.dot(o) + off) / den;
    if (t <= 0.5 || t >= best) return;
    if (inside(o + t * d)) best = t;
  };
  plane(Eigen::Vector3d::UnitZ(), 1.7, [](const Eigen::Vector3d&) { return true; });
  plane(Eigen::Vector3d::UnitY(), -8.0, [](const Eigen::Vector3d& p) { return std::fmod(std::abs(p.x()), 12.0) > 3.0; });
  plane(Eigen::Vector3d::UnitY(), 8.0, [](const Eigen::Vector3d& p) { return std::fmod(std::abs(p.x() + 5.0), 12.0) > 3.0; });
  plane(Eigen::Vector3d::UnitY(), -10.0, [](const Eigen::Vector3d&) { return true; });
  plane(Eigen::Vector3d::UnitY(), 10.0, [](const Eigen::Vector3d&) { return true; });
  plane(Eigen::Vector3d::UnitX(), -60.0, [](const Eigen::Vector3d&) { return true; });
  plane(Eigen::Vector3d::UnitX(), 60.0, [](const Eigen::Vector3d&) { return true; });
  for (double bx = -60.0; bx <= 60.0; bx += 12.0) {
    plane(Eigen::Vector3d::UnitX(), -bx, [&](const Eigen::Vector3d& p) { return p.y() > 8.0 && p.y() < 10.0; });
    plane(Eigen::Vector3d::UnitX(), -(bx - 5.0), [&](const Eigen::Vector3d& p) { return p.y() < -8.0 && p.y() > -10.0; });
  }
  // A pole 0.3 m wide in front of the left wall.
  plane(Eigen::Vector3d::UnitX(), -3.0, [](const Eigen::Vector3d& p) { return p.y() > 2.0 && p.y() < 2.3; });
  plane(Eigen::Vector3d::UnitX(), -3.3, [](const Eigen::Vector3d& p) { return p.y() > 2.0 && p.y() < 2.3; });
  plane(Eigen::Vector3d::UnitY(), -2.0, [](const Eigen::Vector3d& p) { return p.x() > 3.0 && p.x() < 3.3; });
  plane(Eigen::Vector3d::UnitY(), -2.3, [](const Eigen::Vector3d& p) { return p.x() > 3.0 && p.x() < 3.3; });
  return best < 80.0 ? best : -1.0;
}

float intensityAt(const Eigen::Vector3d& p) {
  if (std::abs(p.z() + 1.7) < 0.01) return std::abs(std::fmod(std::abs(p.y()), 3.5) - 1.75) < 0.15 ? 0.9f : 0.2f;
  return std::fmod(std::abs(p.z() + 10.0), 1.0) < 0.3 ? 0.7f : 0.3f;
}

void scanAt(const Eigen::Matrix4d& pose, std::vector<Eigen::Vector3d>* pts, std::vector<float>* inten) {
  pts->clear();
  inten->clear();
  const Eigen::Matrix3d R = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d o = pose.block<3, 1>(0, 3);
  for (int beam = 0; beam < 64; ++beam) {
    const double el = (2.0 - beam * 26.8 / 63.0) * M_PI / 180.0;
    for (int k = 0; k < 2000; ++k) {
      const double az = -M_PI + 2.0 * M_PI * k / 2000.0;
      const Eigen::Vector3d dl(std::cos(el) * std::cos(az), std::cos(el) * std::sin(az), std::sin(el));
      const double t = castRay(o, R * dl);
      if (t <= 0) continue;
      pts->push_back(t * dl);
      inten->push_back(intensityAt(o + t * (R * dl)));
    }
  }
}

}  // namespace

TEST(InTenLoamPaper, RowSupportRecoversSameLineOutsideTenWithoutReplacingAdjacent) {
  detail::FeatureTree target;
  auto add = [&](double x, double y, int row) {
    PaperFeature p; p.p = Eigen::Vector3d(x, y, 0); p.row = row;
    target.pts.push_back(p);
  };
  add(0, 0, 10);
  add(0, .1, 11);
  for (int i = 0; i < 12; ++i) add(.02 * (i + 1), .02, 14);
  const int outside_ten = static_cast<int>(target.pts.size());
  add(.8, 0, 10);
  target.build();
  std::vector<int> ids; std::vector<float> distances;
  const Eigen::Vector3d query(0, 0, .04);
  ASSERT_EQ(target.knn(query, 10, &ids, &distances), 10);
  ASSERT_EQ(ids.front(), 0);
  for (int i : ids) ASSERT_NE(i, outside_ten);
  detail::PlaneRowSupport support; support.build(target.pts);
  int same = -1, adjacent = 1;
  support.complete(target.pts, query, 0, 1., &same, &adjacent);
  EXPECT_EQ(same, outside_ten);
  EXPECT_EQ(adjacent, 1);
  const Eigen::Vector3d normal = (target.pts[0].p - target.pts[adjacent].p)
      .cross(target.pts[0].p - target.pts[same].p).normalized();
  EXPECT_NEAR(std::abs(normal.dot(query - target.pts[0].p)), .04, 1e-12);
}

TEST(InTenLoamPaper, RowSupportRecoversAdjacentLineAndUsesNearestEligiblePoint) {
  std::vector<PaperFeature> points(5);
  points[0].row = points[1].row = 10;
  points[1].p.x() = .1;
  points[2].row = 8; points[2].p.y() = .7;
  points[3].row = 11; points[3].p.y() = .5;
  points[4].row = 13; points[4].p.y() = .01;  // closer but outside native row gap
  detail::PlaneRowSupport support; support.build(points);
  int same = 1, adjacent = -1;
  support.complete(points, Eigen::Vector3d::Zero(), 0, 1., &same, &adjacent);
  EXPECT_EQ(same, 1);
  EXPECT_EQ(adjacent, 3);
  // Existing support is never replaced, even by a closer row candidate.
  same = 1; adjacent = 2;
  support.complete(points, Eigen::Vector3d::Zero(), 0, 1., &same, &adjacent);
  EXPECT_EQ(adjacent, 2);
}

TEST(InTenLoamPaper, RowSupportPreservesDistanceGateExcludesClosestAndClearsOnRebuild) {
  std::vector<PaperFeature> points(3);
  points[0].row = points[1].row = 0;
  points[1].p.x() = 1.01;
  points[2].row = 2; points[2].p.y() = 1.;
  detail::PlaneRowSupport support; support.build(points);
  int same = -1, adjacent = -1;
  support.complete(points, Eigen::Vector3d::Zero(), 0, 1., &same, &adjacent);
  EXPECT_EQ(same, -1);  // cannot reuse closest or exceed the distance gate
  EXPECT_EQ(adjacent, 2);  // inclusive original squared-distance gate
  points.resize(1); support.build(points);
  same = adjacent = -1;
  support.complete(points, Eigen::Vector3d::Zero(), 0, 1., &same, &adjacent);
  EXPECT_EQ(same, -1);
  EXPECT_EQ(adjacent, -1);
  EXPECT_FALSE(InTenLoamPaperParams{}.s2s_row_support);
}

TEST(InTenLoamPaper, NativeSmallKQueriesMatchPclIncludingTiesAndRebuilds) {
  detail::FeatureTree legacy, native;
  std::mt19937 rng(43);
  std::uniform_real_distribution<double> uniform(-3.0, 3.0);
  for (int size : {0, 1, 3, 31, 512}) {
    legacy.pts.clear();
    for (int i = 0; i < size; ++i) {
      PaperFeature f;
      f.p = i < 20 ? Eigen::Vector3d(i % 2, (i / 2) % 2, (i / 4) % 2)
                   : Eigen::Vector3d(uniform(rng), uniform(rng), uniform(rng));
      legacy.pts.push_back(f);  // repeated lattice points exercise exact ties
    }
    native.pts = legacy.pts;
    legacy.build();
    native.build(true);
    for (int query = 0; query < 80; ++query) {
      const Eigen::Vector3d x = query < 10 ? Eigen::Vector3d(.5, .5, .5)
                                         : Eigen::Vector3d(uniform(rng), uniform(rng), uniform(rng));
      for (int k : {0, 1, 5, 10}) {
        std::vector<int> a, b;
        std::vector<float> da, db;
        EXPECT_EQ(legacy.knn(x, k, &a, &da), native.knn(x, k, &b, &db));
        EXPECT_EQ(a, b) << "size=" << size << " k=" << k;
        EXPECT_EQ(da, db);
      }
    }
  }
  // PCL filters nonfinite coordinates and maps indices back to the original
  // cloud. The native option deliberately falls back to PCL for this case.
  PaperFeature invalid;
  invalid.p.x() = std::numeric_limits<double>::quiet_NaN();
  legacy.pts.insert(legacy.pts.begin(), invalid);
  native.pts = legacy.pts;
  legacy.build();
  native.build(true);
  std::vector<int> a, b;
  std::vector<float> da, db;
  EXPECT_EQ(legacy.knn(Eigen::Vector3d::Zero(), 5, &a, &da),
            native.knn(Eigen::Vector3d::Zero(), 5, &b, &db));
  EXPECT_EQ(a, b);
  EXPECT_EQ(da, db);
}

TEST(InTenLoamPaper, AcceleratedQueriesKeepNormalEquationsCountsAndPoses) {
#ifdef _OPENMP
  const int original_threads = omp_get_max_threads();
#endif
  for (int threads : {1, 4}) {
#ifdef _OPENMP
    omp_set_num_threads(threads);
#endif
    InTenLoamPaperParams params;
    params.s2m_plane_max_dist = 0.05;
    InTenLoamPaper legacy(params), legacy_forced(params);
    params.accelerated = true;
    InTenLoamPaper accelerated(params), accelerated_forced(params);
    for (int k = 0; k < 4; ++k) {
      Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
      pose.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.008 * k, Eigen::Vector3d::UnitZ()).toRotationMatrix();
      pose(0, 3) = 0.3 * k;
      std::vector<Eigen::Vector3d> points;
      std::vector<float> intensity;
      scanAt(pose, &points, &intensity);
      const auto fa = legacy.extractFeatures(points, intensity);
      const auto fb = accelerated.extractFeatures(points, intensity);
      for (const auto& pair : {std::make_pair(&fa.all, &fb.all),
                               std::make_pair(&fa.ground, &fb.ground),
                               std::make_pair(&fa.facade, &fb.facade),
                               std::make_pair(&fa.edge, &fb.edge),
                               std::make_pair(&fa.reflector, &fb.reflector)}) {
        ASSERT_EQ(pair.first->size(), pair.second->size());
        for (size_t j = 0; j < pair.first->size(); ++j) {
          const auto& x = (*pair.first)[j];
          const auto& y = (*pair.second)[j];
          EXPECT_TRUE((x.p.array() == y.p.array()).all());
          EXPECT_EQ(x.intensity, y.intensity);
          EXPECT_EQ(x.row, y.row);
          EXPECT_EQ(x.cluster, y.cluster);
        }
      }
      const auto a = legacy.process(points, intensity), b = accelerated.process(points, intensity);
      EXPECT_TRUE((a.pose.array() == b.pose.array()).all()) << "threads=" << threads << " frame=" << k;
      EXPECT_EQ(a.s2s_pairs, b.s2s_pairs);
      EXPECT_EQ(a.s2m_pairs, b.s2m_pairs);
      EXPECT_EQ(a.dor_removed, b.dor_removed);
      InTenLoamPaperDiagnostics da, db;
      da.forced_pose = db.forced_pose = &pose;
      legacy_forced.process(points, intensity, &da);
      accelerated_forced.process(points, intensity, &db);
      for (int c = 0; c < 4; ++c) {
        EXPECT_EQ(da.s2s_n[c], db.s2s_n[c]);
        EXPECT_EQ(da.s2m_n[c], db.s2m_n[c]);
        EXPECT_TRUE((da.s2s_H[c].array() == db.s2s_H[c].array()).all());
        EXPECT_TRUE((da.s2m_H[c].array() == db.s2m_H[c].array()).all());
        EXPECT_TRUE((da.s2s_g[c].array() == db.s2s_g[c].array()).all());
        EXPECT_TRUE((da.s2m_g[c].array() == db.s2m_g[c].array()).all());
      }
    }
  }
#ifdef _OPENMP
  omp_set_num_threads(original_threads);
#endif
}

TEST(InTenLoamPaper, SegmentsGroundAndFindsPlanarFeatures) {
  InTenLoamPaper odo;
  std::vector<Eigen::Vector3d> pts;
  std::vector<float> inten;
  scanAt(Eigen::Matrix4d::Identity(), &pts, &inten);
  const PaperFrame f = odo.extractFeatures(pts, inten);
  ASSERT_GT(f.ground.size(), 500u);
  int on_ground = 0;
  for (const auto& g : f.ground) on_ground += std::abs(g.p.z() + 1.7) < 0.05;
  EXPECT_GT(on_ground, static_cast<int>(f.ground.size() * 95 / 100));
  ASSERT_GT(f.facade.size(), 200u);
  int off_ground = 0;
  for (const auto& p : f.facade) off_ground += p.p.z() > -1.6;
  EXPECT_GT(off_ground, static_cast<int>(f.facade.size() * 9 / 10));
  EXPECT_GT(f.edge.size(), 10u);
  EXPECT_GT(f.reflector.size(), 100u);
}

TEST(InTenLoamPaper, BsplineIntensityFieldFollowsALinearRamp) {
  std::vector<PaperFeature> cloud;
  for (double x = -1.0; x <= 1.0; x += 0.05)
    for (double y = -1.0; y <= 1.0; y += 0.05)
      for (double z = -1.0; z <= 1.0; z += 0.05) {
        PaperFeature f;
        f.p = Eigen::Vector3d(x, y, z);
        f.intensity = static_cast<float>(0.5 + 0.2 * x - 0.1 * z);
        cloud.push_back(f);
      }
  InTenLoamPaper::IntensityGrid grid;
  grid.build(cloud, 0.3);
  double v;
  Eigen::Vector3d g;
  ASSERT_TRUE(grid.eval(Eigen::Vector3d(0.07, -0.12, 0.21), &v, &g));
  EXPECT_NEAR(v, 0.5 + 0.2 * 0.07 - 0.1 * 0.21, 0.01);
  EXPECT_NEAR(g.x(), 0.2, 0.02);
  EXPECT_NEAR(g.y(), 0.0, 0.02);
  EXPECT_NEAR(g.z(), -0.1, 0.02);
  EXPECT_FALSE(grid.eval(Eigen::Vector3d(5.0, 5.0, 5.0), &v, &g));
}

TEST(InTenLoamPaper, TracksForwardMotionAtFullScale) {
  InTenLoamPaper odo;
  std::vector<Eigen::Vector3d> pts;
  std::vector<float> inten;
  Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
  step.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.01, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  step(0, 3) = 1.0;  // about 1 m per scan, as on KITTI
  InTenLoamPaperResult r;
  for (int k = 0; k < 8; ++k) {
    if (k > 0) truth = truth * step;
    scanAt(truth, &pts, &inten);
    r = odo.process(pts, inten);
  }
  // The earlier pipeline came out at ~0.4x the true distance; allow 3 %
  // (platforms differ by a few cm on this corridor-like scene).
  const double scale = r.pose.block<3, 1>(0, 3).norm() / truth.block<3, 1>(0, 3).norm();
  EXPECT_NEAR(scale, 1.0, 0.03);
  EXPECT_LT((r.pose.block<3, 1>(0, 3) - truth.block<3, 1>(0, 3)).norm(), 0.25);
  EXPECT_LT(Eigen::AngleAxisd(r.pose.block<3, 3>(0, 0) * truth.block<3, 3>(0, 0).transpose()).angle(), 0.005);
}

TEST(InTenLoamPaper, StricterPlaneCheckStillTracks) {
  InTenLoamPaperParams params;
  params.s2m_plane_max_dist = 0.05;
  InTenLoamPaper odo(params);
  std::vector<Eigen::Vector3d> pts;
  std::vector<float> inten;
  Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
  step.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.01, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  step(0, 3) = 1.0;
  InTenLoamPaperResult r;
  for (int k = 0; k < 8; ++k) {
    if (k > 0) truth = truth * step;
    scanAt(truth, &pts, &inten);
    r = odo.process(pts, inten);
    if (k > 0) {
      EXPECT_GT(r.s2m_class_pairs[1], 100);  // ground planes pass the 5 cm check
      EXPECT_GT(r.s2m_class_pairs[2], 100);  // facades too
    }
  }
  EXPECT_LT((r.pose.block<3, 1>(0, 3) - truth.block<3, 1>(0, 3)).norm(), 0.25);
  EXPECT_LT(Eigen::AngleAxisd(r.pose.block<3, 3>(0, 0) * truth.block<3, 3>(0, 0).transpose()).angle(), 0.005);
}

TEST(InTenLoamPaper, ForcedPoseDiagnosticsStayAtTruthOnExactScans) {
  InTenLoamPaper odo;
  std::vector<Eigen::Vector3d> pts;
  std::vector<float> inten;
  Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
  step.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.01, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  step(0, 3) = 1.0;
  for (int k = 0; k < 5; ++k) {
    if (k > 0) truth = truth * step;
    scanAt(truth, &pts, &inten);
    InTenLoamPaperDiagnostics diag;
    diag.forced_pose = &truth;
    const InTenLoamPaperResult r = odo.process(pts, inten, &diag);
    // The frame is placed at the forced pose.
    EXPECT_LT((r.pose - truth).norm(), 1e-9);
    if (k == 0) continue;
    // Noise-free scans and a map from exact poses: S2M started at the truth
    // stays near it, and the class normal equations give only a small step.
    // (On this corridor the facade class pulls up to 3 cm backwards: x is
    // held by a few recess faces only.)
    EXPECT_LT((diag.s2m_estimate.block<3, 1>(0, 3) - truth.block<3, 1>(0, 3)).norm(), 0.05);
    EXPECT_LT(Eigen::AngleAxisd(diag.s2m_estimate.block<3, 3>(0, 0) * truth.block<3, 3>(0, 0).transpose()).angle(),
              0.002);
    EXPECT_GT(diag.s2m_n[1], 100);
    EXPECT_GT(diag.s2m_n[2], 100);
    EXPECT_GT(diag.s2s_n[1] + diag.s2s_n[2], 100);
    InTenLoamPaperDiagnostics::Mat6 H = InTenLoamPaperDiagnostics::Mat6::Zero();
    InTenLoamPaperDiagnostics::Vec6 g = InTenLoamPaperDiagnostics::Vec6::Zero();
    for (int c = 0; c < 4; ++c) {
      H += diag.s2m_H[c];
      g += diag.s2m_g[c];
    }
    const InTenLoamPaperDiagnostics::Vec6 d = H.ldlt().solve(-g);
    EXPECT_LT(d.head<3>().norm(), 0.05);
    EXPECT_LT(d.tail<3>().norm(), 0.002);
  }
}

TEST(InTenLoamPaper, PlaneSpreadCheckRemovesTheBackwardPullOfLineNeighbourhoods) {
  // On the street above, facade points on the recess faces find five map
  // neighbours along the end of a wall (a vertical line); every plane through
  // that line passes the distance check, and the arbitrary normal pulls S2M
  // backwards from the truth. Requiring a two-dimensional spread removes it.
  double worst[2] = {0.0, 0.0};
  for (int variant = 0; variant < 2; ++variant) {
    InTenLoamPaperParams params;
    params.s2m_plane_min_spread = variant == 0 ? 0.0 : 0.2;
    InTenLoamPaper odo(params);
    std::vector<Eigen::Vector3d> pts;
    std::vector<float> inten;
    Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
    Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
    step.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.01, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    step(0, 3) = 1.0;
    for (int k = 0; k < 5; ++k) {
      if (k > 0) truth = truth * step;
      scanAt(truth, &pts, &inten);
      InTenLoamPaperDiagnostics diag;
      diag.forced_pose = &truth;
      odo.process(pts, inten, &diag);
      if (k == 0) continue;
      const double forward = (truth.inverse() * diag.s2m_estimate)(0, 3);
      worst[variant] = std::max(worst[variant], std::abs(forward));
    }
  }
  // Measured: 3.2 cm without the check, 1.2 cm with it.
  EXPECT_GT(worst[0], 0.025);
  EXPECT_LT(worst[1], 0.5 * worst[0]);
}

TEST(InTenLoamPaper, MappedMotionPriorRejectsNonRigidRoundoffWithoutChangingTranslation) {
  Eigen::Matrix4d previous = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
  step.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.071, Eigen::Vector3d(1, 2, 3).normalized()).toRotationMatrix();
  step.block<3, 1>(0, 3) << 0.6, -0.2, 0.05;
  for (int i = 0; i < 4096; ++i) {
    Eigen::Matrix4d current = previous * step;
    // Controlled non-rigid roundoff, larger than ordinary double precision
    // so a missing rotation restoration fails reliably in a short test.
    current(0, 0) += 1e-9;
    const Eigen::Matrix4d raw = previous.inverse() * current;
    const Eigen::Matrix4d prior = detail::mappedMotionPrior(previous, current);
    const Eigen::Matrix3d rotation = prior.block<3, 3>(0, 0);
    EXPECT_LT((rotation.transpose() * rotation - Eigen::Matrix3d::Identity()).norm(), 2e-15);
    EXPECT_NEAR(rotation.determinant(), 1.0, 2e-15);
    EXPECT_EQ((prior.block<3, 1>(0, 3) - raw.block<3, 1>(0, 3)).norm(), 0.0);
    EXPECT_GT((raw.block<3, 3>(0, 0).transpose() * raw.block<3, 3>(0, 0) -
               Eigen::Matrix3d::Identity()).norm(), 1e-12);
    EXPECT_EQ(prior(3, 0), 0.0);
    EXPECT_EQ(prior(3, 1), 0.0);
    EXPECT_EQ(prior(3, 2), 0.0);
    EXPECT_EQ(prior(3, 3), 1.0);
    previous = current;
  }
}

class InTenLoamPassiveTrace : public ::testing::TestWithParam<bool> {};

TEST_P(InTenLoamPassiveTrace, PreservesMovingPosesAndFutureMapState) {
  for (int options = 0; options < 4; ++options) {
    InTenLoamPaperParams params;
    params.s2s_row_support = options & 1;
    params.s2s_mapped_prior = options & 2;
    params.accelerated = GetParam();
    InTenLoamPaper plain(params), observed(params);
    std::vector<Eigen::Vector3d> pts;
    std::vector<float> inten;
    Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
    Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
    step(0, 3) = 1.0;
    step.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.01, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    InTenLoamPaperTrace trace;
    for (int k = 0; k < 6; ++k) {
      if (k) truth = truth * step;
      scanAt(truth, &pts, &inten);
      const auto a = plain.process(pts, inten);
      // Stop observing before the last frames, checking retained map/prediction.
      const auto b = observed.process(pts, inten, nullptr, k < 4 ? &trace : nullptr);
      EXPECT_EQ((a.pose - b.pose).norm(), 0.0);
      EXPECT_EQ(a.s2s_pairs, b.s2s_pairs);
      EXPECT_EQ(a.s2m_pairs, b.s2m_pairs);
      EXPECT_EQ(a.dor_removed, b.dor_removed);
      for (int c = 0; c < 4; ++c) EXPECT_EQ(a.s2m_class_pairs[c], b.s2m_class_pairs[c]);
      if (k >= 4) continue;
      EXPECT_EQ(trace.frame_index, k);
      EXPECT_EQ((trace.pose - b.pose).norm(), 0.0);
      if (!k) {
        EXPECT_TRUE(trace.initialization);
        EXPECT_FALSE(trace.s2s.called);
        EXPECT_FALSE(trace.s2m.called);
        continue;
      }
      EXPECT_FALSE(trace.initialization);
      EXPECT_TRUE(trace.s2s.called);
      EXPECT_TRUE(trace.s2m.called);
      EXPECT_FALSE(trace.s2s_fallback);
      EXPECT_TRUE(trace.s2m_accepted);
      EXPECT_EQ(trace.s2s.returned_pairs, b.s2s_pairs);
      EXPECT_EQ(trace.s2m.returned_pairs, b.s2m_pairs);
      EXPECT_EQ((trace.accepted_relative - trace.s2s.output).norm(), 0.0);
      EXPECT_EQ((trace.s2m.input - trace.s2m_prediction).norm(), 0.0);
      EXPECT_EQ((trace.pose - trace.s2m.output).norm(), 0.0);
      for (const auto* solver : {&trace.s2s, &trace.s2m}) {
        ASSERT_FALSE(solver->iterations.empty());
        EXPECT_EQ((solver->input - solver->iterations.front().linearization_pose).norm(), 0.0);
        for (const auto& it : solver->iterations) {
          EXPECT_EQ(it.pairs, it.class_pairs[0] + it.class_pairs[1] + it.class_pairs[2] + it.class_pairs[3]);
          EXPECT_GE(it.weighted_squared_residual, 0.0);
          EXPECT_TRUE(it.step_attempted);
          EXPECT_TRUE(it.step_finite);
          EXPECT_TRUE(it.update_applied);
          const PaperIterationTrace::Vec6 recomputed =
              (it.H + 1e-6 * PaperIterationTrace::Mat6::Identity()).ldlt().solve(-it.g);
          EXPECT_LT((recomputed - it.delta).norm(), 1e-12);
        }
      }
    }
  }
}

TEST_P(InTenLoamPassiveTrace, MappedPriorIsRecursiveAndPreservesS2SFallback) {
  EXPECT_FALSE(InTenLoamPaperParams{}.s2s_mapped_prior);
  for (bool mapped : {false, true}) {
    InTenLoamPaperParams params;
    params.accelerated = GetParam();
    params.s2s_mapped_prior = mapped;
    InTenLoamPaper odo(params);
    Eigen::Matrix4d previous = Eigen::Matrix4d::Identity();
    Eigen::Matrix4d expected_seed = Eigen::Matrix4d::Identity();
    Eigen::Matrix4d accepted = Eigen::Matrix4d::Identity();
    std::vector<Eigen::Vector3d> pts;
    std::vector<float> intensity;
    double correction = 0.0;
    for (int k = 0; k < 5; ++k) {
      Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
      truth(0, 3) = k;
      scanAt(truth, &pts, &intensity);
      InTenLoamPaperTrace trace;
      const auto result = odo.process(pts, intensity, nullptr, &trace);
      if (k) {
        EXPECT_LT((trace.s2s.input - expected_seed).norm(), mapped ? 1e-10 : 1e-30);
        ASSERT_TRUE(trace.s2m_accepted);
        ASSERT_FALSE(trace.s2s_fallback);
        const Eigen::Matrix4d final_rel = previous.inverse() * result.pose;
        correction = std::max(correction, (final_rel - trace.accepted_relative).norm());
        expected_seed = mapped ? final_rel : trace.accepted_relative;
        accepted = trace.accepted_relative;
      }
      previous = result.pose;
    }
    // This fixture must actually distinguish mapped and S2S motion.
    EXPECT_GT(correction, 1e-5);
    InTenLoamPaperTrace empty;
    const auto rejected = odo.process({}, {}, nullptr, &empty);
    EXPECT_LT((empty.s2s.input - expected_seed).norm(), mapped ? 1e-10 : 1e-30);
    EXPECT_TRUE(empty.s2s_fallback);
    EXPECT_FALSE(empty.s2m_accepted);
    EXPECT_EQ((empty.accepted_relative - accepted).norm(), 0.0);
    // After rejection, the recursive seed follows the actual final poses.
    const Eigen::Matrix4d next_seed = mapped ? (previous.inverse() * rejected.pose).eval() : accepted;
    odo.process({}, {}, nullptr, &empty);
    EXPECT_LT((empty.s2s.input - next_seed).norm(), mapped ? 1e-10 : 1e-30);
    InTenLoamPaper fresh(params);
    fresh.process(pts, intensity);
    fresh.process(pts, intensity, nullptr, &empty);
    EXPECT_EQ((empty.s2s.input - Eigen::Matrix4d::Identity()).norm(), 0.0);
  }
}

TEST_P(InTenLoamPassiveTrace, MappedPriorDoesNotAffectMappingOffOrForcedDiagnostics) {
  for (bool forced : {false, true}) {
    InTenLoamPaperParams params;
    params.accelerated = GetParam();
    params.use_mapping = forced;
    InTenLoamPaper baseline(params);
    params.s2s_mapped_prior = true;
    InTenLoamPaper candidate(params);
    std::vector<Eigen::Vector3d> pts;
    std::vector<float> intensity;
    for (int k = 0; k < 4; ++k) {
      Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
      truth(0, 3) = k;
      scanAt(truth, &pts, &intensity);
      InTenLoamPaperDiagnostics da, db;
      if (forced) da.forced_pose = db.forced_pose = &truth;
      const auto a = baseline.process(pts, intensity, &da);
      const auto b = candidate.process(pts, intensity, &db);
      EXPECT_EQ((a.pose - b.pose).norm(), 0.0);
      EXPECT_EQ(a.s2s_pairs, b.s2s_pairs);
      EXPECT_EQ(a.s2m_pairs, b.s2m_pairs);
      if (!forced || !k) continue;
      EXPECT_EQ((da.s2s_estimate - db.s2s_estimate).norm(), 0.0);
      EXPECT_EQ((da.s2m_estimate - db.s2m_estimate).norm(), 0.0);
      for (int c = 0; c < 4; ++c) {
        EXPECT_EQ((da.s2s_H[c] - db.s2s_H[c]).norm(), 0.0);
        EXPECT_EQ((da.s2s_g[c] - db.s2s_g[c]).norm(), 0.0);
        EXPECT_EQ((da.s2m_H[c] - db.s2m_H[c]).norm(), 0.0);
        EXPECT_EQ((da.s2m_g[c] - db.s2m_g[c]).norm(), 0.0);
      }
    }
  }
}

TEST_P(InTenLoamPassiveTrace, ShowsFallbackAndMappingRejectionWithoutStaleIterations) {
  InTenLoamPaperParams params;
  params.accelerated = GetParam();
  InTenLoamPaper plain(params), observed(params);
  std::vector<Eigen::Vector3d> pts;
  std::vector<float> inten;
  InTenLoamPaperTrace trace;
  Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
  for (int k = 0; k < 3; ++k) {
    truth(0, 3) = k;
    scanAt(truth, &pts, &inten);
    plain.process(pts, inten);
    observed.process(pts, inten, nullptr, &trace);
  }
  ASSERT_GT((trace.s2s.input.block<3, 1>(0, 3).norm()), 0.1);
  const auto a = plain.process({}, {});
  const auto b = observed.process({}, {}, nullptr, &trace);
  EXPECT_EQ((a.pose - b.pose).norm(), 0.0);
  EXPECT_TRUE(trace.s2s_fallback);
  EXPECT_FALSE(trace.s2m_accepted);
  EXPECT_EQ((trace.accepted_relative - trace.s2s.input).norm(), 0.0);
  EXPECT_EQ((trace.pose - trace.s2m_prediction).norm(), 0.0);
  for (const auto* solver : {&trace.s2s, &trace.s2m}) {
    EXPECT_EQ(solver->stop, PaperSolverStop::InsufficientPairs);
    EXPECT_EQ(solver->returned_pairs, 0);
    ASSERT_EQ(solver->iterations.size(), 1u);
    EXPECT_EQ(solver->iterations[0].pairs, 0);
    EXPECT_FALSE(solver->iterations[0].step_attempted);
    EXPECT_FALSE(solver->iterations[0].update_applied);
    EXPECT_EQ((solver->output - solver->input).norm(), 0.0);
  }
}

TEST_P(InTenLoamPassiveTrace, DistinguishesSkippedMappingAndZeroIterationBudget) {
  for (bool mapping : {false, true}) {
    InTenLoamPaperParams params;
    params.accelerated = GetParam();
    params.use_mapping = mapping;
    params.s2m_interval = 2;
    params.s2s_iterations = 0;
    params.s2m_iterations = 0;
    InTenLoamPaper odo(params);
    InTenLoamPaperTrace trace;
    for (int k = 0; k < 4; ++k) {
      odo.process({}, {}, nullptr, &trace);
      EXPECT_EQ(trace.frame_index, k);
      if (!k) continue;
      EXPECT_EQ(trace.s2s.stop, PaperSolverStop::NoIterations);
      EXPECT_TRUE(trace.s2s.iterations.empty());
      EXPECT_TRUE(trace.s2s_fallback);
      EXPECT_EQ(trace.s2m.called, mapping && k % 2 == 0);
      EXPECT_EQ(trace.s2m.stop, trace.s2m.called ? PaperSolverStop::NoIterations : PaperSolverStop::NotRun);
      EXPECT_TRUE(trace.s2m.iterations.empty());
      EXPECT_FALSE(trace.s2m_accepted);
    }
  }
}

TEST_P(InTenLoamPassiveTrace, ReportsIterationBudgetAndConvergenceSeparately) {
  std::vector<Eigen::Vector3d> pts;
  std::vector<float> inten;
  for (bool converge : {false, true}) {
    InTenLoamPaperParams params;
    params.accelerated = GetParam();
    params.s2s_iterations = params.s2m_iterations = 1;
    params.convergence = converge ? 1e6 : 0.0;
    InTenLoamPaper odo(params);
    scanAt(Eigen::Matrix4d::Identity(), &pts, &inten);
    odo.process(pts, inten);
    Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
    truth(0, 3) = 1.0;
    scanAt(truth, &pts, &inten);
    InTenLoamPaperTrace trace;
    odo.process(pts, inten, nullptr, &trace);
    for (const auto* solver : {&trace.s2s, &trace.s2m}) {
      ASSERT_EQ(solver->iterations.size(), 1u);
      EXPECT_TRUE(solver->iterations[0].update_applied);
      EXPECT_EQ(solver->stop, converge ? PaperSolverStop::Converged : PaperSolverStop::IterationLimit);
    }
  }
}

TEST_P(InTenLoamPassiveTrace, RejectsForcedPoseBeforeChangingEstimatorState) {
  InTenLoamPaperParams params;
  params.accelerated = GetParam();
  InTenLoamPaper plain(params), observed(params);
  Eigen::Matrix4d intervention = Eigen::Matrix4d::Identity();
  intervention(0, 3) = 100.0;
  InTenLoamPaperDiagnostics diag;
  diag.forced_pose = &intervention;
  InTenLoamPaperTrace trace;
  EXPECT_THROW(observed.process({}, {}, &diag, &trace), std::invalid_argument);
  const auto a = plain.process({}, {});
  const auto b = observed.process({}, {}, nullptr, &trace);
  EXPECT_TRUE(trace.initialization);
  EXPECT_EQ(trace.frame_index, 0);
  EXPECT_EQ((a.pose - b.pose).norm(), 0.0);
}

INSTANTIATE_TEST_SUITE_P(AccelerationModes, InTenLoamPassiveTrace,
                         ::testing::Values(false, true));
