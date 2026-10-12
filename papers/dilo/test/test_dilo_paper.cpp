#include "dilo/dilo_paper.h"

#include <gtest/gtest.h>
#include <Eigen/Dense>
#include <Eigen/Geometry>

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
