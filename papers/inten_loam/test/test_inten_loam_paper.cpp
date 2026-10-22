#include "inten_loam/inten_loam_paper.h"
#include "inten_loam/detail/feature_tree.h"
#include "inten_loam/detail/ground_pca.h"
#include "inten_loam/detail/mapped_motion_prior.h"
#include "inten_loam/detail/plane_row_support.h"
#include "inten_loam/detail/translation_prior.h"
#include "inten_loam/detail/s2s_consensus.h"

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

// Stationary sensor and static street, with one independently moving wall.
// Its radial motion passes existing DOR while remaining inside the S2S gate.
void movingWall(double wall_x, std::vector<Eigen::Vector3d>* pts, std::vector<float>* inten) {
  pts->clear(); inten->clear();
  for (int beam = 0; beam < 64; ++beam) {
    const double el = (2.0-beam*26.8/63.0)*M_PI/180.;
    for (int k = 0; k < 2000; ++k) {
      const double az = -M_PI+2.*M_PI*k/2000.;
      const Eigen::Vector3d d(std::cos(el)*std::cos(az),std::cos(el)*std::sin(az),std::sin(el));
      double t = castRay(Eigen::Vector3d::Zero(),d);
      if (d.x() > 0.) {
        const double candidate = wall_x/d.x();
        const Eigen::Vector3d p = candidate*d;
        if (std::abs(p.y()) < 3. && p.z() > -1.6 && p.z() < 2. && (t < 0. || candidate < t)) t = candidate;
      }
      if (t > 0.) { pts->push_back(t*d); inten->push_back(intensityAt(t*d)); }
    }
  }
}

}  // namespace

TEST(InTenLoamPaperConsensus, AmbiguityTieMajorityAndBoundaryFallback) {
  using detail::ClusterTranslationProfile;
  EXPECT_FALSE(InTenLoamPaperParams{}.s2s_geometry_consensus);
  auto profile = [](int id, const std::vector<int>& allowed) {
    ClusterTranslationProfile p{id, std::vector<double>(121, 1.)};
    for (int k : allowed) p.costs[k] = 0.;
    return p;
  };
  auto selected = detail::selectTranslationConsensus({profile(2, {40, 60}), profile(3, {40}), profile(4, {40})});
  ASSERT_TRUE(selected.usable);
  EXPECT_EQ(selected.best_index, 40);
  EXPECT_TRUE(selected.excluded_clusters.empty());
  selected = detail::selectTranslationConsensus({profile(2, {40}), profile(3, {80})});
  EXPECT_FALSE(selected.usable);
  EXPECT_EQ(selected.best_index, 40);  // nearest zero tie, then negative
  selected = detail::selectTranslationConsensus({profile(2, {0}), profile(3, {0})});
  EXPECT_FALSE(selected.usable);
  EXPECT_TRUE(selected.excluded_clusters.empty());
  selected = detail::selectTranslationConsensus({profile(2, {50}), profile(3, {50}), profile(4, {80})});
  ASSERT_TRUE(selected.usable);
  EXPECT_EQ(selected.excluded_clusters, std::vector<int>({4}));
  EXPECT_FALSE(detail::selectTranslationConsensus({}).usable);
}

TEST(InTenLoamPaperConsensus, FullPointProfilesPreserveSerialAndParallelBits) {
  PaperFrame current;
  std::vector<PaperFeature> previous;
  for (int c = 2; c < 7; ++c) {
    for (int k = 0; k < 35; ++k) {
      PaperFeature p;
      p.cluster = c;
      p.p = Eigen::Vector3d(.3 * std::sin(k * .71) + c * 4., .2 * std::cos(k * .47), .15 * std::sin(k * .19));
      previous.push_back(p);
      p.p.x() += 1.;
      current.all.push_back(p);
      if (k == 0) current.facade.push_back(p);
    }
  }
  Eigen::Matrix4d seed = Eigen::Matrix4d::Identity();
  seed(0, 3) = .6;
  const auto serial = detail::clusterTranslationProfiles(previous, current, seed, false);
#ifdef _OPENMP
  const int threads = omp_get_max_threads();
  omp_set_num_threads(4);
#endif
  const auto parallel = detail::clusterTranslationProfiles(previous, current, seed, true);
#ifdef _OPENMP
  omp_set_num_threads(threads);
#endif
  ASSERT_EQ(serial.size(), parallel.size());
  for (size_t c = 0; c < serial.size(); ++c) {
    EXPECT_EQ(serial[c].cluster, parallel[c].cluster);
    EXPECT_EQ(serial[c].costs, parallel[c].costs);
  }
  const auto decision = detail::selectTranslationConsensus(serial);
  ASSERT_TRUE(decision.usable);
  // The exact shape match is at-1.6m. Consensus deliberately admits an
  // uncertainty band and picks its nearest-zero common shift, not its minimum.
  EXPECT_DOUBLE_EQ(serial.front().costs[28], 0.);
  EXPECT_GT(decision.best_index, 0);
  EXPECT_LT(decision.best_index, 60);
  for (const auto& band : decision.admissions)
    EXPECT_TRUE(detail::admitted(band, decision.best_index));
  const auto unavailable = detail::clusterTranslationProfiles(previous, current, Eigen::Matrix4d::Identity(), false);
  EXPECT_FALSE(detail::selectTranslationConsensus(unavailable).usable);
}

TEST(InTenLoamPaperConsensus, PassiveTraceAndFixedRotationDuringNaturalTraversal) {
  InTenLoamPaperParams params;
  params.accelerated = true;
  params.s2s_geometry_consensus = true;
  params.s2s_ground_pca = params.s2s_facade_pca = true;
  params.s2s_translation_prior_ratio = .03;
  params.use_mapping = false;
  InTenLoamPaper passive(params), ordinary(params);
  std::vector<Eigen::Vector3d> points;
  std::vector<float> intensity;
  int applied = 0;
  for (int i = 0; i < 3; ++i) {
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose(0, 3) = i * .6;
    scanAt(pose, &points, &intensity);
    InTenLoamPaperTrace trace;
    const auto a = passive.process(points, intensity, nullptr, &trace);
    const auto b = ordinary.process(points, intensity);
    EXPECT_TRUE(a.pose == b.pose);
    if (trace.consensus.applied) {
      ++applied;
      EXPECT_TRUE((trace.accepted_relative.block<3, 3>(0, 0) == trace.s2s.output.block<3, 3>(0, 0)));
      for (const auto& it : trace.consensus.refinement.iterations) {
        EXPECT_TRUE((it.linearization_pose.block<3, 3>(0, 0) == trace.s2s.output.block<3, 3>(0, 0)));
        EXPECT_TRUE(it.delta.tail<3>().isZero(0.));
      }
    } else if (!trace.initialization && !trace.s2s_fallback) {
      EXPECT_TRUE(trace.accepted_relative == trace.s2s.output);
    }
  }
  EXPECT_GT(applied, 0);
}

TEST(InTenLoamPaper, CauchyFacadeDownweightsMovingSurfaceWithoutChangingOtherDataSystems) {
  InTenLoamPaperParams p;
  EXPECT_FALSE(p.s2s_facade_cauchy);
  p.accelerated=true;p.use_mapping=false;
  InTenLoamPaper reference(p);p.s2s_facade_cauchy=true;InTenLoamPaper candidate(p);
  std::vector<Eigen::Vector3d> points;std::vector<float> intensity;
  movingWall(6.,&points,&intensity);
  reference.process(points,intensity);candidate.process(points,intensity);
  movingWall(6.8,&points,&intensity);
  const Eigen::Matrix4d seed=Eigen::Matrix4d::Identity();
  InTenLoamPaperDiagnostics a,b;a.forced_pose=b.forced_pose=&seed;
  const auto x=reference.process(points,intensity,&a);
  const auto y=candidate.process(points,intensity,&b);
  EXPECT_EQ(x.num_ground,y.num_ground);EXPECT_EQ(x.num_facade,y.num_facade);
  EXPECT_EQ(x.num_edge,y.num_edge);EXPECT_EQ(x.num_reflector,y.num_reflector);
  for(int c=0;c<4;++c)EXPECT_EQ(a.s2s_n[c],b.s2s_n[c]);
  for(int c:{0,1,3}) {
    EXPECT_EQ((a.s2s_H[c]-b.s2s_H[c]).norm(),0.);
    EXPECT_EQ((a.s2s_g[c]-b.s2s_g[c]).norm(),0.);
  }
  EXPECT_GT(a.s2s_g[2].norm(),b.s2s_g[2].norm());
  EXPECT_GT((a.s2s_H[2]-b.s2s_H[2]).norm(),0.);
  EXPECT_TRUE(b.s2s_estimate.allFinite());
}

TEST(InTenLoamPaper, CauchyFacadeTracePreservesTheIterativeTrajectory) {
  InTenLoamPaperParams p;p.accelerated=true;p.s2s_facade_cauchy=true;
  InTenLoamPaper plain(p),observed(p);
  std::vector<Eigen::Vector3d> points;std::vector<float> intensity;
  for(int i=0;i<3;++i) {
    movingWall(6.+.8*i,&points,&intensity);InTenLoamPaperTrace trace;
    const auto a=plain.process(points,intensity);
    const auto b=observed.process(points,intensity,nullptr,&trace);
    EXPECT_EQ((a.pose-b.pose).norm(),0.);EXPECT_EQ(a.s2s_pairs,b.s2s_pairs);EXPECT_EQ(a.s2m_pairs,b.s2m_pairs);
    EXPECT_EQ(a.dor_removed,b.dor_removed);
  }
}

TEST(InTenLoamPaper, CauchyFacadeBufferedReductionPreservesSerialResults) {
  InTenLoamPaperParams p;p.s2s_facade_cauchy=true;p.use_mapping=false;
  InTenLoamPaper serial(p);p.accelerated=true;InTenLoamPaper buffered(p);
  std::vector<Eigen::Vector3d> points;std::vector<float> intensity;
  for(int i=0;i<3;++i) {
    movingWall(6.+.8*i,&points,&intensity);InTenLoamPaperTrace a,b;
    const auto x=serial.process(points,intensity,nullptr,&a);
    const auto y=buffered.process(points,intensity,nullptr,&b);
    EXPECT_EQ((x.pose-y.pose).norm(),0.);ASSERT_EQ(a.s2s.iterations.size(),b.s2s.iterations.size());
    for(size_t k=0;k<a.s2s.iterations.size();++k) {
      EXPECT_EQ((a.s2s.iterations[k].H-b.s2s.iterations[k].H).norm(),0.);
      EXPECT_EQ((a.s2s.iterations[k].g-b.s2s.iterations[k].g).norm(),0.);
    }
  }
}

TEST(InTenLoamPaper, PredictedDorGatesFacadeQueriesWithoutRemovingExtractedFeatures) {
  InTenLoamPaperParams p;
  EXPECT_FALSE(p.s2s_predicted_dor_facade);
  p.accelerated = true;
  InTenLoamPaper reference(p);
  p.s2s_predicted_dor_facade = true;
  InTenLoamPaper candidate(p);
  std::vector<Eigen::Vector3d> points; std::vector<float> intensity;
  movingWall(6.,&points,&intensity);
  reference.process(points,intensity); candidate.process(points,intensity);
  movingWall(6.8,&points,&intensity);
  InTenLoamPaperTrace a,b;
  const auto x = reference.process(points,intensity,nullptr,&a);
  const auto y = candidate.process(points,intensity,nullptr,&b);
  ASSERT_TRUE(b.s2s_dor_called);
  ASSERT_GT(b.s2s_dor_masked_facades,0u);
  EXPECT_FALSE(a.s2s_dor_called);
  EXPECT_EQ(x.num_ground,y.num_ground); EXPECT_EQ(x.num_facade,y.num_facade);
  EXPECT_EQ(x.num_edge,y.num_edge); EXPECT_EQ(x.num_reflector,y.num_reflector);
  EXPECT_EQ((a.s2s.input-b.s2s.input).norm(),0.);
  ASSERT_FALSE(a.s2s.iterations.empty()); ASSERT_FALSE(b.s2s.iterations.empty());
  for (int c : {0,1,3}) EXPECT_EQ(a.s2s.iterations.front().class_pairs[c],b.s2s.iterations.front().class_pairs[c]);
  EXPECT_GT(a.s2s.iterations.front().class_pairs[2],b.s2s.iterations.front().class_pairs[2]);
  EXPECT_TRUE(b.s2s.output.allFinite()); EXPECT_TRUE(y.pose.allFinite());
}

TEST(InTenLoamPaper, PredictedDorIsInactiveWithoutMappingOrDor) {
  for (bool mapping : {false,true}) {
    InTenLoamPaperParams p;
    p.accelerated = true;
    p.use_mapping = mapping; p.use_dor = !mapping;
    InTenLoamPaper reference(p);
    p.s2s_predicted_dor_facade = true;
    InTenLoamPaper candidate(p);
    std::vector<Eigen::Vector3d> points; std::vector<float> intensity;
    for (int i = 0; i < 3; ++i) {
      movingWall(6.+.8*i,&points,&intensity);
      InTenLoamPaperTrace a,b;
      const auto x=reference.process(points,intensity,nullptr,&a);
      const auto y=candidate.process(points,intensity,nullptr,&b);
      EXPECT_FALSE(b.s2s_dor_called); EXPECT_EQ(b.s2s_dor_masked_facades,0u);
      EXPECT_EQ((x.pose-y.pose).norm(),0.);
      EXPECT_EQ(x.s2s_pairs,y.s2s_pairs); EXPECT_EQ(x.s2m_pairs,y.s2m_pairs);
      EXPECT_EQ(x.dor_removed,y.dor_removed);
      ASSERT_EQ(a.s2s.iterations.size(),b.s2s.iterations.size());
      for (size_t k=0;k<a.s2s.iterations.size();++k) {
        EXPECT_EQ((a.s2s.iterations[k].H-b.s2s.iterations[k].H).norm(),0.);
        EXPECT_EQ((a.s2s.iterations[k].g-b.s2s.iterations[k].g).norm(),0.);
      }
    }
  }
}

TEST(InTenLoamPaper, PredictedDorKeepsForcedDiagnosticEquationsAndOutputs) {
  InTenLoamPaperParams p; p.accelerated=true;
  InTenLoamPaper reference(p); p.s2s_predicted_dor_facade=true;
  InTenLoamPaper candidate(p);
  std::vector<Eigen::Vector3d> points; std::vector<float> intensity;
  movingWall(6.,&points,&intensity);
  reference.process(points,intensity); candidate.process(points,intensity);
  movingWall(6.8,&points,&intensity);
  const Eigen::Matrix4d seed=Eigen::Matrix4d::Identity();
  InTenLoamPaperDiagnostics a,b; a.forced_pose=b.forced_pose=&seed;
  reference.process(points,intensity,&a); candidate.process(points,intensity,&b);
  for(int c=0;c<4;++c) {
    EXPECT_EQ((a.s2s_H[c]-b.s2s_H[c]).norm(),0.); EXPECT_EQ((a.s2s_g[c]-b.s2s_g[c]).norm(),0.);
    EXPECT_EQ((a.s2m_H[c]-b.s2m_H[c]).norm(),0.); EXPECT_EQ((a.s2m_g[c]-b.s2m_g[c]).norm(),0.);
    EXPECT_EQ(a.s2s_n[c],b.s2s_n[c]); EXPECT_EQ(a.s2m_n[c],b.s2m_n[c]);
  }
  EXPECT_EQ((a.s2s_estimate-b.s2s_estimate).norm(),0.);
  EXPECT_EQ((a.s2m_estimate-b.s2m_estimate).norm(),0.);
}

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

TEST(InTenLoamPaper, NormalizedIntensityDenseParityAndRebuildDefault) {
  EXPECT_FALSE(InTenLoamPaperParams{}.s2s_intensity_normalized);
  std::vector<PaperFeature> cloud;
  for (int i = -2; i <= 3; ++i)
    for (int j = -2; j <= 3; ++j)
      for (int k = -2; k <= 3; ++k) {
        PaperFeature f;
        f.p = .3 * Eigen::Vector3d(i, j, k);
        f.intensity = static_cast<float>(.4 + .03 * i - .02 * j + .01 * k);
        cloud.push_back(f);
      }
  InTenLoamPaper::IntensityGrid legacy, normalized;
  legacy.build(cloud, .3);
  normalized.build(cloud, .3, true);
  for (const auto& x : {Eigen::Vector3d(.07, -.12, .21), Eigen::Vector3d(-.11, .02, -.09)}) {
    double a, b;
    Eigen::Vector3d ga, gb;
    ASSERT_TRUE(legacy.eval(x, &a, &ga));
    ASSERT_TRUE(normalized.eval(x, &b, &gb));
    EXPECT_NEAR(a, b, 1e-14);
    EXPECT_TRUE(ga.isApprox(gb, 1e-12));
  }
  normalized.build(cloud, .3);  // Reusing a grid must reset the opt-in mode.
  double a, b;
  Eigen::Vector3d ga, gb;
  const Eigen::Vector3d x(.07, -.12, .21);
  ASSERT_TRUE(legacy.eval(x, &a, &ga));
  ASSERT_TRUE(normalized.eval(x, &b, &gb));
  EXPECT_EQ(a, b);
  EXPECT_TRUE((ga.array() == gb.array()).all());
}

TEST(InTenLoamPaper, NormalizedSparseIntensityIsContinuousAndHasQuotientGradient) {
  for (int axis = 0; axis < 3; ++axis)
    for (int face : {-1, 1}) {
      std::vector<PaperFeature> cloud;
      for (int i = face - 2; i <= face + 2; ++i)
        for (int j : {0, 1}) {
          PaperFeature f;
          f.p = Eigen::Vector3d::Zero();
          f.p[axis] = .3 * i;
          f.p[(axis + 1) % 3] = .3 * j;
          f.intensity = static_cast<float>(.3 + .04 * i + .02 * j);
          cloud.push_back(f);
        }
      InTenLoamPaper::IntensityGrid grid;
      grid.build(cloud, .3, true);
      Eigen::Vector3d x = Eigen::Vector3d::Constant(.3 * .375);
      x[axis] = .3 * face;
      auto left = x, right = x;
      left[axis] -= 1e-9;
      right[axis] += 1e-9;
      double a, b;
      Eigen::Vector3d ga, gb;
      ASSERT_TRUE(grid.eval(left, &a, &ga));
      ASSERT_TRUE(grid.eval(right, &b, &gb));
      EXPECT_NEAR(a, b, 1e-8);
      EXPECT_LT((ga - gb).norm(), 1e-7);
      x[axis] -= .3 * .375;
      ASSERT_TRUE(grid.eval(x, &a, &ga));
      for (int d = 0; d < 3; ++d) {
        left = right = x;
        left[d] -= 3e-6;
        right[d] += 3e-6;
        double lo, hi;
        ASSERT_TRUE(grid.eval(left, &lo, &gb));
        ASSERT_TRUE(grid.eval(right, &hi, &gb));
        EXPECT_NEAR(ga[d], (hi - lo) / 6e-6, 1e-8);
      }
      for (auto& f : cloud) f.intensity = .4f;
      grid.build(cloud, .3, true);
      ASSERT_TRUE(grid.eval(x, &a, &ga));
      EXPECT_NEAR(a, .4f, 1e-14);
      EXPECT_LT(ga.norm(), 1e-12);
    }
}

TEST(InTenLoamPaper, NormalizedIntensityRejectsLowCountAndZeroBasisWeight) {
  std::vector<PaperFeature> cloud;
  for (int j : {0, 1})
    for (int k : {0, 1}) {
      PaperFeature f;
      f.p = .3 * Eigen::Vector3d(2, j, k);
      f.intensity = .4f;
      cloud.push_back(f);
    }
  InTenLoamPaper::IntensityGrid legacy, normalized;
  legacy.build(cloud, .3);
  normalized.build(cloud, .3, true);
  double value;
  Eigen::Vector3d gradient;
  const Eigen::Vector3d zero_weight(0, .1, .1);
  ASSERT_TRUE(legacy.eval(zero_weight, &value, &gradient));
  EXPECT_FALSE(normalized.eval(zero_weight, &value, &gradient));
  cloud.pop_back();
  normalized.build(cloud, .3, true);
  EXPECT_FALSE(normalized.eval(Eigen::Vector3d(.1, .1, .1), &value, &gradient));
  EXPECT_FALSE(normalized.eval(Eigen::Vector3d(10, 10, 10), &value, &gradient));
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

TEST(InTenLoamPaper, TranslationPriorRaisesOnlyWeakMarginalInformation) {
  using Mat6 = PaperIterationTrace::Mat6;
  using Vec6 = PaperIterationTrace::Vec6;
  Mat6 H = Mat6::Identity();
  H.diagonal() << .5, 100, 200, 10, 20, 30;
  const Eigen::Matrix3d information = detail::translationPriorInformation(H, Eigen::Vector3d::Zero(), .03);
  Eigen::Matrix3d expected = Eigen::Matrix3d::Zero(); expected(0, 0) = 5.5;
  EXPECT_LT((information-expected).norm(), 1e-12);
  Vec6 g; g << 1, 2, 3, 4, 5, 6;
  const Vec6 before = (H+1e-6*Mat6::Identity()).ldlt().solve(-g);
  Vec6 adjusted_g = g;
  Mat6 adjusted_H = H;
  EXPECT_EQ(detail::addTranslationPrior(information, Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), &adjusted_H, &adjusted_g), 0.0);
  const Vec6 after = (adjusted_H+1e-6*Mat6::Identity()).ldlt().solve(-adjusted_g);
  EXPECT_NEAR(after[0], -1.0/(6+1e-6), 1e-12);
  EXPECT_LT(std::abs(after[0]), std::abs(before[0]));
  EXPECT_EQ((after.tail<5>()-before.tail<5>()).norm(), 0.0);
  EXPECT_EQ(detail::translationPriorInformation(Mat6::Identity(), Eigen::Vector3d::Zero(), .03).norm(), 0.0);
}

TEST(InTenLoamPaper, TranslationPriorCentersRotationAndAnchorsActualPosition) {
  using Mat6 = PaperIterationTrace::Mat6;
  using Vec6 = PaperIterationTrace::Vec6;
  const Eigen::Vector3d t(3, -2, 1), prediction(2.8, -1.9, 1);
  Mat6 B = Mat6::Identity(); B.block<3, 3>(0, 3) = detail::crossMatrix(t);
  Mat6 centered = Mat6::Identity(); centered.diagonal() << .5, 100, 200, 10, 20, 30;
  const Mat6 H = B.inverse().transpose()*centered*B.inverse();
  const Eigen::Matrix3d information = detail::translationPriorInformation(H, t, .03);
  Eigen::Matrix3d expected = Eigen::Matrix3d::Zero(); expected(0, 0) = 5.5;
  EXPECT_LT((information-expected).norm(), 1e-10);
  Mat6 prior_H = Mat6::Zero(); Vec6 prior_g = Vec6::Zero();
  const double cost = detail::addTranslationPrior(information, prediction, t, &prior_H, &prior_g);
  EXPECT_NEAR(cost, 5.5*.2*.2, 1e-12);
  // Independent finite differences of the quadratic residual under native
  // left rotation-plus-translation increments verify the rotational sign.
  for (int axis = 0; axis < 6; ++axis) {
    auto objective = [&](double amount) {
      Eigen::Vector3d point = t;
      if (axis < 3) point[axis] += amount;
      else point = Eigen::AngleAxisd(amount, Eigen::Vector3d::Unit(axis-3))*point;
      return (point-prediction).dot(information*(point-prediction))*.5;
    };
    EXPECT_NEAR((objective(1e-6)-objective(-1e-6))/2e-6, prior_g[axis], 1e-8);
  }
  Eigen::SelfAdjointEigenSolver<Mat6> eig(prior_H);
  EXPECT_GT(eig.eigenvalues().minCoeff(), -1e-10);
}

TEST(InTenLoamPaper, GroundPcaFitsSlopedSurfaceWithOriginalOrientation) {
  std::vector<PaperFeature> points;
  std::vector<int> ids;
  std::vector<float> distances;
  for (double x : {-0.4, 0.0, 0.4}) for (double y : {-0.3, 0.0, 0.3}) {
    PaperFeature f; f.p = Eigen::Vector3d(x, y, 0.2*x - 0.1*y - 1.7);
    ids.push_back(static_cast<int>(points.size())); points.push_back(f);
    distances.push_back(0.5f);
  }
  Eigen::Vector3d normal(0, 0, -1);
  ASSERT_TRUE(detail::groundPcaNormal(points, ids, distances, 1.0, &normal));
  EXPECT_LT((normal-Eigen::Vector3d(0.2, -0.1, -1).normalized()).norm(), 1e-12);
  // An out-of-gate point must not turn a road surface towards a wall.
  PaperFeature outlier; outlier.p = Eigen::Vector3d(100, 100, 100);
  ids.push_back(static_cast<int>(points.size())); points.push_back(outlier);
  distances.push_back(1.01f);
  Eigen::Vector3d with_outlier(0, 0, -1);
  ASSERT_TRUE(detail::groundPcaNormal(points, ids, distances, 1.0, &with_outlier));
  EXPECT_EQ((normal-with_outlier).norm(), 0.0);
}

TEST(InTenLoamPaper, GroundPcaRetainsTriangleOnLineOrInsufficientSupport) {
  std::vector<PaperFeature> points(4);
  for (int i = 0; i < 4; ++i) points[i].p = Eigen::Vector3d(i, 2*i, -i);
  const Eigen::Vector3d original = Eigen::Vector3d(1, 2, 3).normalized();
  Eigen::Vector3d normal = original;
  EXPECT_FALSE(detail::groundPcaNormal(points, {0, 1, 2, 3}, {0.f, .1f, .2f, .3f}, 1., &normal));
  EXPECT_EQ((normal-original).norm(), 0.0);
  points[2].p.y() += 1.0;
  EXPECT_FALSE(detail::groundPcaNormal(points, {0, 1, 2, 3}, {0.f, .1f, 1.1f, 1.2f}, 1., &normal));
  EXPECT_EQ((normal-original).norm(), 0.0);
}

TEST(InTenLoamPaper, GroundPcaChangesOnlyGroundAtFixedInitialPose) {
  EXPECT_FALSE(InTenLoamPaperParams{}.s2s_ground_pca);
  InTenLoamPaperParams params; params.use_mapping = false;
  InTenLoamPaper baseline(params);
  params.s2s_ground_pca = true;
  InTenLoamPaper candidate(params);
  std::vector<Eigen::Vector3d> points; std::vector<float> intensity;
  Eigen::Matrix4d seed = Eigen::Matrix4d::Identity(), truth = seed;
  auto ripple = [&](double origin_x) {
    for (auto& point : points) if (std::abs(point.z()+1.7) < 1e-6)
      point.z() += .004*std::sin(3*(point.x()+origin_x)+2*point.y());
  };
  scanAt(seed, &points, &intensity); ripple(0.0);
  baseline.process(points, intensity); candidate.process(points, intensity);
  truth(0, 3) = .3;
  scanAt(truth, &points, &intensity); ripple(truth(0, 3));
  InTenLoamPaperDiagnostics a, b; a.forced_pose = &seed; b.forced_pose = &seed;
  baseline.process(points, intensity, &a); candidate.process(points, intensity, &b);
  ASSERT_GT(a.s2s_n[1], 0);
  for (int c = 0; c < 4; ++c) {
    EXPECT_EQ(a.s2s_n[c], b.s2s_n[c]);
    if (c == 1) continue;
    EXPECT_EQ((a.s2s_H[c]-b.s2s_H[c]).norm(), 0.0);
    EXPECT_EQ((a.s2s_g[c]-b.s2s_g[c]).norm(), 0.0);
  }
  EXPECT_GT((a.s2s_H[1]-b.s2s_H[1]).norm(), 1e-6);
}

TEST(InTenLoamPaper, FacadePcaChangesOnlyFacadeAtFixedInitialPose) {
  EXPECT_FALSE(InTenLoamPaperParams{}.s2s_facade_pca);
  InTenLoamPaperParams params; params.use_mapping = false; params.s2s_ground_pca = true;
  InTenLoamPaper baseline(params);
  params.s2s_facade_pca = true;
  InTenLoamPaper candidate(params);
  std::vector<Eigen::Vector3d> points; std::vector<float> intensity;
  Eigen::Matrix4d seed = Eigen::Matrix4d::Identity(), truth = seed;
  auto ripple = [&](double origin_x) {
    for (auto& point : points) if (std::abs(std::abs(point.y())-8.) < 1e-6)
      point.y() += .004*std::sin(3*(point.x()+origin_x)+2*point.z());
  };
  scanAt(seed, &points, &intensity); ripple(0.0);
  baseline.process(points, intensity); candidate.process(points, intensity);
  truth(0, 3) = .3;
  scanAt(truth, &points, &intensity); ripple(truth(0, 3));
  InTenLoamPaperDiagnostics a, b; a.forced_pose = &seed; b.forced_pose = &seed;
  baseline.process(points, intensity, &a); candidate.process(points, intensity, &b);
  ASSERT_GT(a.s2s_n[2], 0);
  for (int c = 0; c < 4; ++c) {
    EXPECT_EQ(a.s2s_n[c], b.s2s_n[c]);
    if (c == 2) continue;
    EXPECT_EQ((a.s2s_H[c]-b.s2s_H[c]).norm(), 0.0);
    EXPECT_EQ((a.s2s_g[c]-b.s2s_g[c]).norm(), 0.0);
  }
  EXPECT_GT((a.s2s_H[2]-b.s2s_H[2]).norm(), 1e-6);
}

TEST(InTenLoamPaper, AcceptedPlanePcaFitsVerticalFacadeAndExcludesOutOfGatePoint) {
  std::vector<PaperFeature> points(5);
  points[0].p = Eigen::Vector3d(0, 8, 0);
  points[1].p = Eigen::Vector3d(.5, 8.1, 0);
  points[2].p = Eigen::Vector3d(0, 7.95, .5);
  points[3].p = Eigen::Vector3d(.5, 8.05, .5);
  points[4].p = Eigen::Vector3d(0, 20, 0);  // outside the unchanged gate
  const Eigen::Vector3d expected = Eigen::Vector3d(-.2, 1., .1).normalized();
  Eigen::Vector3d normal = Eigen::Vector3d::UnitY();
  ASSERT_TRUE(detail::groundPcaNormal(points, {0,1,2,3,4}, {0.f,.3f,.3f,.6f,4.f}, 1., &normal));
  EXPECT_LT((normal-expected).norm(), 1e-12);
}

TEST(InTenLoamPaper, TranslationPriorIsDefaultOffAndValidatesItsRange) {
  EXPECT_EQ(InTenLoamPaperParams{}.s2s_translation_prior_ratio, 0.0);
  for (double invalid : {-1.0, 1.01, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
    InTenLoamPaperParams p; p.s2s_translation_prior_ratio = invalid;
    EXPECT_THROW({ InTenLoamPaper odometry(p); }, std::invalid_argument);
  }
}

TEST(InTenLoamPaper, TranslationPriorAppearsInTraceButNotFeatureCounts) {
  InTenLoamPaperParams p; p.use_mapping = false; p.s2s_translation_prior_ratio = 1.0;
  InTenLoamPaper observed(p), forced(p);
  std::vector<Eigen::Vector3d> points; std::vector<float> intensity;
  Eigen::Matrix4d origin = Eigen::Matrix4d::Identity(), truth = origin;
  scanAt(origin, &points, &intensity);
  observed.process(points, intensity); forced.process(points, intensity);
  truth(0, 3) = .3;
  scanAt(truth, &points, &intensity);
  InTenLoamPaperTrace trace; observed.process(points, intensity, nullptr, &trace);
  InTenLoamPaperDiagnostics diagnostic; diagnostic.forced_pose = &origin;
  forced.process(points, intensity, &diagnostic);
  ASSERT_FALSE(trace.s2s.iterations.empty());
  const auto& first = trace.s2s.iterations.front();
  PaperIterationTrace::Mat6 H = PaperIterationTrace::Mat6::Zero();
  PaperIterationTrace::Vec6 g = PaperIterationTrace::Vec6::Zero();
  int n = 0;
  for (int c = 0; c < 4; ++c) { H += diagnostic.s2s_H[c]; g += diagnostic.s2s_g[c]; n += diagnostic.s2s_n[c]; }
  const Eigen::Matrix3d information = detail::translationPriorInformation(H, Eigen::Vector3d::Zero(), 1.0);
  EXPECT_GT(information.norm(), 1.0);
  detail::addTranslationPrior(information, Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), &H, &g);
  // Class-wise accumulation changes summation order relative to the serial
  // per-feature total; compare at double-precision relative roundoff.
  EXPECT_LT((H-first.H).norm(), 1e-13*H.norm());
  EXPECT_EQ(n, first.pairs);
  EXPECT_LT((g-first.g).norm(), 1e-13*g.norm());
  EXPECT_LT((trace.s2s.translation_prior_information-information).norm(), 1e-10);
  EXPECT_EQ((trace.s2s.output-diagnostic.s2s_estimate).norm(), 0.0);
}

TEST_P(InTenLoamPassiveTrace, PreservesMovingPosesAndFutureMapState) {
  for (int options = 0; options < 32; ++options) {
    InTenLoamPaperParams params;
    params.s2s_row_support = options & 1;
    params.s2s_mapped_prior = options & 2;
    params.s2s_translation_prior_ratio = options & 4 ? .03 : 0.0;
    params.s2s_ground_pca = options & 8;
    params.s2s_facade_pca = options & 16;
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
