#include "inten_loam/inten_loam_paper.h"

#include <gtest/gtest.h>
#include <Eigen/Geometry>

#include <cmath>
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
