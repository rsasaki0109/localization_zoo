#include "dilo/dilo_paper.h"

#include <gtest/gtest.h>
#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <algorithm>
#include <cmath>
#include <random>

using namespace localization_zoo::dilo;

namespace {

// Ray-cast-like scan of a street: ground at z = -1.7, walls at y = +-8 with
// recesses, boxes; sampled along HDL-64-like beams.
double castRay(const Eigen::Vector3d& o, const Eigen::Vector3d& d) {
  double best = 1e9;
  auto plane = [&](const Eigen::Vector3d& n, double off, auto inside) {
    const double den = n.dot(d);
    if (std::abs(den) < 1e-9) return;
    const double t = -(n.dot(o) + off) / den;
    if (t <= 0.5 || t >= best) return;
    const Eigen::Vector3d p = o + t * d;
    if (inside(p)) best = t;
  };
  plane(Eigen::Vector3d::UnitZ(), 1.7, [](const Eigen::Vector3d&) { return true; });
  plane(Eigen::Vector3d::UnitY(), -8.0, [](const Eigen::Vector3d& p) { return std::fmod(std::abs(p.x()), 12.0) > 3.0; });
  plane(Eigen::Vector3d::UnitY(), 8.0, [](const Eigen::Vector3d& p) { return std::fmod(std::abs(p.x() + 5.0), 12.0) > 3.0; });
  plane(Eigen::Vector3d::UnitY(), -10.0, [](const Eigen::Vector3d&) { return true; });
  plane(Eigen::Vector3d::UnitY(), 10.0, [](const Eigen::Vector3d&) { return true; });
  plane(Eigen::Vector3d::UnitX(), -40.0, [](const Eigen::Vector3d&) { return true; });  // end walls
  plane(Eigen::Vector3d::UnitX(), 40.0, [](const Eigen::Vector3d&) { return true; });
  for (double bx = -60.0; bx <= 60.0; bx += 12.0) {  // side walls of the recesses
    plane(Eigen::Vector3d::UnitX(), -bx, [&](const Eigen::Vector3d& p) { return p.y() > 8.0 && p.y() < 10.0; });
    plane(Eigen::Vector3d::UnitX(), -(bx - 5.0), [&](const Eigen::Vector3d& p) { return p.y() < -8.0 && p.y() > -10.0; });
  }
  return best < 80.0 ? best : -1.0;
}

std::vector<Eigen::Vector3d> scanAt(const Eigen::Matrix4d& pose) {
  std::vector<Eigen::Vector3d> pts;
  const Eigen::Matrix3d R = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d o = pose.block<3, 1>(0, 3);
  for (int beam = 0; beam < 64; ++beam) {
    const double el = (2.0 - beam * 26.8 / 63.0) * M_PI / 180.0;
    for (int k = 0; k < 2000; ++k) {
      const double az = -M_PI + 2.0 * M_PI * k / 2000.0;
      const Eigen::Vector3d dl(std::cos(el) * std::cos(az), std::cos(el) * std::sin(az), std::sin(el));
      const double t = castRay(o, R * dl);
      if (t > 0) pts.push_back(t * dl);
    }
  }
  return pts;
}

}  // namespace

TEST(DiloPaper, GroundAndWallNormalsPointToTheSensor) {
  DiloPaperOdometry odo;
  const auto frame = odo.buildFrame(scanAt(Eigen::Matrix4d::Identity()));
  int ground = 0, wall = 0, bad = 0;
  for (size_t i = 0; i < frame->point.size(); ++i) {
    if (!frame->has_normal[i]) continue;
    const Eigen::Vector3f p = frame->point[i], n = frame->normal[i];
    if (std::abs(p.z() + 1.7f) < 0.02f && p.head<2>().norm() < 20.0f) {
      ++ground;
      if (n.z() < 0.98f) ++bad;
    } else if (std::abs(std::abs(p.y()) - 10.0f) < 0.02f && std::abs(p.x()) < 15.0f && p.z() > -1.0f) {
      ++wall;
      if (std::abs(n.y()) < 0.98f || n.y() * p.y() > 0) ++bad;
    }
  }
  EXPECT_GT(ground, 1000);
  EXPECT_GT(wall, 100);
  EXPECT_LT(bad, (ground + wall) / 20);
}

TEST(DiloPaper, SlopeFillPutsTheFarGroundBelowTheGround) {
  // Between two beams the ground range changes by r^2 / h per radian of
  // elevation, more than fill_slope = 0.02 of the range per row beyond about
  // 11 m. The gap is then filled with the far range along the pixel-centre
  // rays, which pass below the far beam: points under the road. A larger
  // fill_slope interpolates the ground.
  auto below = [](double fill_slope) {
    DiloPaperParams p;
    p.fill_slope = fill_slope;
    DiloPaperOdometry odo(p);
    const auto frame = odo.buildFrame(scanAt(Eigen::Matrix4d::Identity()));
    int ground = 0, under = 0;
    for (const auto& q : frame->point) {
      if (std::abs(q.x()) < 15.0f || std::abs(q.x()) > 35.0f || std::abs(q.y()) > 6.0f || q.z() > -1.5f) continue;
      ++ground;
      if (q.z() < -1.75f) ++under;
    }
    EXPECT_GT(ground, 1000);
    return static_cast<double>(under) / ground;
  };
  EXPECT_GT(below(0.02), 0.2);
  EXPECT_LT(below(0.1), 0.01);
}

TEST(DiloPaper, KeypointCellsSpreadTheLists) {
  DiloPaperParams p;
  p.keypoint_cell_px = 8;
  DiloPaperOdometry odo(p);
  const auto frame = odo.buildFrame(scanAt(Eigen::Matrix4d::Identity()));
  ASSERT_EQ(frame->key_points.size(), 11u * p.keypoints_per_list);
  for (int list = 0; list < 11; ++list) {
    std::vector<std::pair<int, int>> cells;
    for (int i = 0; i < p.keypoints_per_list; ++i) {
      int row, col;
      ASSERT_TRUE(odo.project(frame->key_points[list * p.keypoints_per_list + i], &row, &col));
      cells.push_back({row / 8, col / 8});
    }
    std::sort(cells.begin(), cells.end());
    EXPECT_TRUE(std::adjacent_find(cells.begin(), cells.end()) == cells.end()) << "list " << list;
  }
  // The ground list (n_z) spreads sideways instead of piling up ahead.
  auto lateral_spread = [](const DiloPaperFrame& f, int K) {
    double lo = 1e9, hi = -1e9;
    for (int i = 10 * K; i < 11 * K; ++i) {
      lo = std::min(lo, f.key_points[i].y());
      hi = std::max(hi, f.key_points[i].y());
    }
    return hi - lo;
  };
  DiloPaperOdometry plain;
  const auto dense = plain.buildFrame(scanAt(Eigen::Matrix4d::Identity()));
  EXPECT_GT(lateral_spread(*frame, p.keypoints_per_list), 2.0 * lateral_spread(*dense, p.keypoints_per_list));
}

TEST(DiloPaper, PairwiseStepsRecoverASmallMotion) {
  DiloPaperOdometry odo;
  Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
  truth.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.02, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  truth(0, 3) = 0.3;
  truth(1, 3) = 0.1;
  const auto ref = odo.buildFrame(scanAt(Eigen::Matrix4d::Identity()));
  const auto src = odo.buildFrame(scanAt(truth));
  ASSERT_GT(src->key_points.size(), 500u);
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  for (int it = 0; it < 15; ++it) {
    Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> g = Eigen::Matrix<double, 6, 1>::Zero();
    ASSERT_GT(odo.accumulate(*src, *ref, T, 0, &H, &g), 50);
    const Eigen::Matrix<double, 6, 1> d = H.ldlt().solve(-g);
    Eigen::Matrix4d inc = Eigen::Matrix4d::Identity();
    inc.block<3, 3>(0, 0) = Eigen::AngleAxisd(d.head<3>().norm(), d.head<3>().normalized()).toRotationMatrix();
    if (d.head<3>().norm() < 1e-12) inc.block<3, 3>(0, 0).setIdentity();
    inc.block<3, 1>(0, 3) = d.tail<3>();
    T = T * inc;
  }
  EXPECT_LT((T.block<3, 1>(0, 3) - truth.block<3, 1>(0, 3)).norm(), 0.05);
  EXPECT_LT(Eigen::AngleAxisd(T.block<3, 3>(0, 0) * truth.block<3, 3>(0, 0).transpose()).angle(), 0.002);
}

TEST(DiloPaper, EdgeJacobianMatchesFiniteDifferences) {
  DiloPaperParams params;
  params.edge_search_px = 50.0;
  params.edge_row_search = 1;
  DiloPaperOdometry odo(params);
  std::mt19937 rng(7);
  std::uniform_real_distribution<double> uni(-1.0, 1.0);
  // Increment on the right, T exp(xi), xi = [omega, v].
  auto perturbed = [](const Eigen::Matrix4d& T, const Eigen::Matrix<double, 6, 1>& xi) {
    Eigen::Matrix4d inc = Eigen::Matrix4d::Identity();
    const double th = xi.head<3>().norm();
    if (th > 0) inc.block<3, 3>(0, 0) = Eigen::AngleAxisd(th, xi.head<3>() / th).toRotationMatrix();
    inc.block<3, 1>(0, 3) = xi.tail<3>();
    return Eigen::Matrix4d(T * inc);
  };
  int checked = 0;
  for (int trial = 0; trial < 20; ++trial) {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    const Eigen::Vector3d axis = Eigen::Vector3d(uni(rng), uni(rng), uni(rng)).normalized();
    T.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.3 * uni(rng), axis).toRotationMatrix();
    T.block<3, 1>(0, 3) = Eigen::Vector3d(uni(rng), uni(rng), 0.2 * uni(rng));
    const Eigen::Vector3d p(15.0 * uni(rng), 15.0 * uni(rng), -1.0 + 0.5 * uni(rng));
    const Eigen::Vector3d x = T.block<3, 3>(0, 0) * p + T.block<3, 1>(0, 3);
    int row, col;
    if (!odo.project(x, &row, &col)) continue;
    // One reference edge on every row, 2.3 px from the projection.
    DiloPaperFrame ref;
    ref.edge_cols.assign(params.height, {});
    double u_edge = odo.imageU(x) + 2.3;
    if (u_edge >= params.width) u_edge -= params.width;
    for (auto& r : ref.edge_cols) r.push_back(u_edge);
    double e;
    Eigen::Matrix<double, 6, 1> J;
    ASSERT_TRUE(odo.edgeResidual(p, ref, T, &e, &J));
    EXPECT_NEAR(e, 2.3, 1e-6);
    const double h = 1e-6;
    for (int k = 0; k < 6; ++k) {
      Eigen::Matrix<double, 6, 1> d = Eigen::Matrix<double, 6, 1>::Zero();
      d(k) = h;
      double ep, em;
      Eigen::Matrix<double, 6, 1> unused;
      ASSERT_TRUE(odo.edgeResidual(p, ref, perturbed(T, d), &ep, &unused));
      ASSERT_TRUE(odo.edgeResidual(p, ref, perturbed(T, -d), &em, &unused));
      const double numeric = (ep - em) / (2.0 * h);
      EXPECT_NEAR(J(k), numeric, 1e-4 * std::max(1.0, std::abs(numeric))) << "trial " << trial << " k " << k;
    }
    ++checked;
  }
  EXPECT_GE(checked, 10);
}

TEST(DiloPaper, VerticalEdgesRecoverYaw) {
  // Noise-free street, 4 mrad (1.3 px) of yaw. The first detector (3x3
  // minima of the Harris response of dI_e/du) recovers only about 10 % of it:
  // the 2D suppression leaves isolated points, so a source edge rarely finds
  // its own edge on the same reference row. Minima along u only (1) or the
  // ridges of I_e (2) keep every row of a vertical edge.
  for (int detector : {1, 2}) {
    DiloPaperParams params;
    params.use_edges = true;
    params.edge_detector = detector;
    params.edge_subpixel = true;
    params.edge_reference_points = 3000;
    DiloPaperOdometry odo(params);
    Eigen::Matrix4d truth = Eigen::Matrix4d::Identity();
    truth.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.004, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    const auto ref = odo.buildFrame(scanAt(Eigen::Matrix4d::Identity()));
    const auto src = odo.buildFrame(scanAt(truth));
    ASSERT_GT(src->edge_points.size(), 50u) << detector;
    // Yaw only; translation and tilt at the truth.
    double yaw = 0.0;
    for (int it = 0; it < 10; ++it) {
      Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
      T.block<3, 3>(0, 0) = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
      Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
      Eigen::Matrix<double, 6, 1> g = Eigen::Matrix<double, 6, 1>::Zero();
      ASSERT_GT(odo.accumulateEdges(*src, *ref, T, 0, &H, &g), 30) << detector;
      yaw -= g(2) / H(2, 2);
    }
    EXPECT_NEAR(yaw, 0.004, detector == 2 ? 0.0005 : 0.001) << detector;
  }
}

TEST(DiloPaper, EdgeCombinationsTrackASyntheticDrive) {
  // Plane only, the edge step averaged on yaw only, and the summed system
  // restricted to yaw: all follow 4 frames of 0.4 m and 3 mrad per frame.
  for (int mode = 0; mode < 3; ++mode) {
    DiloPaperParams params;
    params.use_edges = mode > 0;
    params.edge_detector = 2;
    params.edge_subpixel = true;
    params.edge_dofs = 1;
    params.edge_sum_normal_equations = mode == 2;
    params.edge_weight = 0.003;
    DiloPaperOdometry odo(params);
    Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
    step.block<3, 3>(0, 0) = Eigen::AngleAxisd(0.003, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    step(0, 3) = 0.4;
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity(), est;
    for (int k = 0; k < 4; ++k) {
      est = odo.process(scanAt(pose));
      if (k < 3) pose = pose * step;
    }
    EXPECT_LT((est.block<3, 1>(0, 3) - pose.block<3, 1>(0, 3)).norm(), 0.05) << mode;
    EXPECT_LT(Eigen::AngleAxisd(est.block<3, 3>(0, 0) * pose.block<3, 3>(0, 0).transpose()).angle(), 0.002)
        << mode;
  }
}
