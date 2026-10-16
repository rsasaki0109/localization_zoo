#include "suma/suma_paper.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

using namespace localization_zoo::suma;

namespace {

struct Box {
  Eigen::Vector3d lo;
  Eigen::Vector3d hi;
};

// Street canyon: ground, two facades, closing walls and a few boxes, so all
// six degrees of freedom are constrained.
std::vector<Box> makeScene() {
  return {
      {{-200, -200, -3.0}, {200, 200, -1.73}},  // ground
      {{-60, 7.0, -3.0}, {60, 8.0, 6.0}},       // left facade
      {{-60, -8.0, -3.0}, {60, -7.0, 6.0}},     // right facade
      {{45, -8, -3}, {46, 8, 6}},               // front wall
      {{-46, -8, -3}, {-45, 8, 6}},             // back wall
      {{10, 2, -1.73}, {12, 4, 0.0}},
      {{-15, -5, -1.73}, {-12, -3, 1.5}},
      {{20, -6, -1.73}, {21, -4.5, 3.0}},
      {{4, 4.5, -1.73}, {6, 6.5, 2.0}},
      {{-6, 3, -1.73}, {-5, 4, 4.0}},
      {{30, 1, -1.73}, {33, 3, 1.0}},
  };
}

double rayBox(const Eigen::Vector3d& o, const Eigen::Vector3d& d, const Box& b) {
  double tmin = -std::numeric_limits<double>::infinity();
  double tmax = std::numeric_limits<double>::infinity();
  for (int k = 0; k < 3; ++k) {
    if (std::abs(d[k]) < 1e-12) {
      if (o[k] < b.lo[k] || o[k] > b.hi[k]) return -1.0;
      continue;
    }
    double t1 = (b.lo[k] - o[k]) / d[k];
    double t2 = (b.hi[k] - o[k]) / d[k];
    if (t1 > t2) std::swap(t1, t2);
    tmin = std::max(tmin, t1);
    tmax = std::min(tmax, t2);
  }
  if (tmax < tmin || tmax < 0.0) return -1.0;
  return tmin > 0.0 ? tmin : -1.0;
}

// 64-beam scan from +2 to -24.8 deg in the sensor frame at T_world_sensor.
std::vector<Eigen::Vector3d> simulateScan(const Eigen::Matrix4d& T, int columns = 1500) {
  static const std::vector<Box> scene = makeScene();
  const Eigen::Matrix3d R = T.topLeftCorner<3, 3>();
  const Eigen::Vector3d o = T.block<3, 1>(0, 3);
  std::vector<Eigen::Vector3d> points;
  points.reserve(64 * columns);
  for (int beam = 0; beam < 64; ++beam) {
    const double elev = (2.0 - beam * 26.8 / 63.0) * M_PI / 180.0;
    for (int c = 0; c < columns; ++c) {
      const double az = 2.0 * M_PI * (c + 0.25) / columns - M_PI;
      const Eigen::Vector3d dir_s(std::cos(elev) * std::cos(az),
                                  std::cos(elev) * std::sin(az), std::sin(elev));
      const Eigen::Vector3d dir_w = R * dir_s;
      double best = std::numeric_limits<double>::infinity();
      for (const Box& b : scene) {
        const double t = rayBox(o, dir_w, b);
        if (t > 0.0 && t < best) best = t;
      }
      if (best < 100.0) points.push_back(best * dir_s);
    }
  }
  return points;
}

Eigen::Matrix4d makePose(double x, double y, double z, double roll, double pitch,
                         double yaw) {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.topLeftCorner<3, 3>() = (Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                             Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                             Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX()))
                                .toRotationMatrix();
  T.block<3, 1>(0, 3) = Eigen::Vector3d(x, y, z);
  return T;
}

double rotationErrorDeg(const Eigen::Matrix4d& A, const Eigen::Matrix4d& B) {
  const Eigen::Matrix3d E = A.topLeftCorner<3, 3>().transpose() * B.topLeftCorner<3, 3>();
  return std::acos(std::clamp(0.5 * (E.trace() - 1.0), -1.0, 1.0)) * 180.0 / M_PI;
}

}  // namespace

TEST(SuMaPaperTest, ProjectionMatchesOfficialLayout) {
  SuMaPaper suma;
  double x = 0, y = 0;
  // Straight ahead at the horizon: centre column, row fov_up / fov * h.
  ASSERT_TRUE(suma.project(Eigen::Vector3d(10, 0, 0), &x, &y));
  EXPECT_NEAR(x, 450.0, 1e-9);
  EXPECT_NEAR(y, 3.0 / 28.0 * 64.0, 1e-9);
  // Columns grow clockwise (decreasing yaw).
  ASSERT_TRUE(suma.project(Eigen::Vector3d(10, -1, 0), &x, &y));
  EXPECT_GT(x, 450.0);
  // Outside the vertical field of view.
  EXPECT_FALSE(suma.project(Eigen::Vector3d(10, 0, 5), &x, &y));
}

TEST(SuMaPaperTest, VertexAndNormalMapsOfPlanes) {
  SuMaPaper suma;
  const auto scan = simulateScan(Eigen::Matrix4d::Identity());
  const auto map = suma.computeVertexNormalMap(scan);
  int valid = 0, ground = 0, ground_ok = 0, facade = 0, facade_ok = 0;
  for (int i = 0; i < map.width * map.height; ++i) {
    if (!map.valid(i)) continue;
    ++valid;
    const Eigen::Vector3f& v = map.vertex[i];
    const Eigen::Vector3f& n = map.normal[i];
    if (std::abs(v.z() + 1.73f) < 1e-3f && std::abs(v.y()) < 6.0f &&
        std::abs(v.x()) < 3.5f) {
      ++ground;
      ground_ok += n.z() > 0.99f;
    }
    if (std::abs(v.y() - 7.0f) < 1e-3f && std::abs(v.x()) < 2.0f && v.z() > -1.0f) {
      ++facade;
      facade_ok += n.y() < -0.99f;
    }
  }
  EXPECT_GT(valid, 900 * 64 / 2);
  ASSERT_GT(ground, 50);
  ASSERT_GT(facade, 20);
  EXPECT_GT(static_cast<double>(ground_ok) / ground, 0.9);
  EXPECT_GT(static_cast<double>(facade_ok) / facade, 0.9);
}

TEST(SuMaPaperTest, RenderedModelReproducesTheScan) {
  SuMaPaper suma;
  const auto scan = simulateScan(Eigen::Matrix4d::Identity());
  suma.process(scan);
  ASSERT_GT(suma.surfels().size(), 10000u);
  int rendered = 0;
  const auto model =
      suma.renderModel(Eigen::Matrix4d::Identity(), suma.renderConfidenceThreshold(), &rendered);
  const auto& data = suma.lastData();
  int valid = 0, covered = 0;
  std::vector<double> distances;
  int normal_ok = 0;
  for (int i = 0; i < data.width * data.height; ++i) {
    if (!data.valid(i)) continue;
    ++valid;
    if (!model.valid(i)) continue;
    ++covered;
    const Eigen::Vector3d vd = data.vertex[i].cast<double>();
    const Eigen::Vector3d vm = model.vertex[i].cast<double>();
    const Eigen::Vector3d nm = model.normal[i].cast<double>();
    distances.push_back(std::abs(nm.dot(vd - vm)));
    normal_ok += nm.dot(data.normal[i].cast<double>()) > 0.95;
  }
  ASSERT_GT(valid, 0);
  EXPECT_GT(static_cast<double>(covered) / valid, 0.8);
  std::nth_element(distances.begin(), distances.begin() + distances.size() / 2,
                   distances.end());
  EXPECT_LT(distances[distances.size() / 2], 0.02);
  EXPECT_GT(static_cast<double>(normal_ok) / covered, 0.9);

  // Rendering from a pose 1 m ahead moves the image content: an object at
  // x = 10 m now appears closer.
  const auto shifted = suma.renderModel(makePose(1, 0, 0, 0, 0, 0),
                                        suma.renderConfidenceThreshold());
  double x = 0, y = 0;
  ASSERT_TRUE(suma.project(Eigen::Vector3d(9.5, 3.0, -0.5), &x, &y));
  // The 64 beams do not fill all 64 rows (0.425 vs 0.4375 deg), so look in
  // the 3 x 3 neighbourhood.
  int i = -1;
  for (int dv = -1; dv <= 1 && i < 0; ++dv)
    for (int du = -1; du <= 1 && i < 0; ++du) {
      const int j = shifted.index(static_cast<int>(x) + du, static_cast<int>(y) + dv);
      if (shifted.valid(j)) i = j;
    }
  ASSERT_GE(i, 0);
  EXPECT_NEAR(shifted.vertex[i].x(), 9.0, 0.3);  // box face at x = 10 (world)
}

TEST(SuMaPaperTest, ProjectiveAssociationRecoversPerturbation) {
  SuMaPaper suma;
  suma.process(simulateScan(Eigen::Matrix4d::Identity()));
  const auto model =
      suma.renderModel(Eigen::Matrix4d::Identity(), suma.renderConfidenceThreshold());
  const auto& data = suma.lastData();

  SuMaPaper::IcpStats stats;
  const Eigen::Matrix4d same =
      suma.alignProjective(data, model, Eigen::Matrix4d::Identity(), 2.0, 30.0, &stats);
  const double same_t = same.block<3, 1>(0, 3).norm();
  EXPECT_LT(same_t, 0.01);
  EXPECT_LT(rotationErrorDeg(same, Eigen::Matrix4d::Identity()), 0.05);
  EXPECT_GT(stats.inliers, 10000);

  // A scan taken at a known pose aligns to the model of the first scan.
  const Eigen::Matrix4d truth = makePose(0.8, 0.15, 0.03, 0.01, -0.008, 0.03);
  const auto moved = suma.computeVertexNormalMap(simulateScan(truth));
  const Eigen::Matrix4d est =
      suma.alignProjective(moved, model, Eigen::Matrix4d::Identity(), 2.0, 30.0, &stats);
  const double est_t = (est.block<3, 1>(0, 3) - truth.block<3, 1>(0, 3)).norm();
  EXPECT_LT(est_t, 0.03);
  EXPECT_LT(rotationErrorDeg(est, truth), 0.1);
}

TEST(SuMaPaperTest, OptInRemapProjectsTranslationRatherThanRotation) {
  SuMaPaperParams params;
  params.max_iterations = 1;
  SuMaPaper reference(params);
  reference.process(simulateScan(Eigen::Matrix4d::Identity()));
  const auto model = reference.renderModel(Eigen::Matrix4d::Identity(), reference.renderConfidenceThreshold());
  const auto data = reference.computeVertexNormalMap(simulateScan(makePose(0.3, 0.1, 0.02, 0, 0, 0.01)));
  SuMaPaper::IcpStats stats;
  const auto control = reference.alignProjective(data, model, Eigen::Matrix4d::Identity(), 2, 30, &stats);
  ASSERT_GT(stats.inliers, 1000);
  ASSERT_GT((control.block<3, 1>(0, 3).norm()), 0.05);
  // An intentionally all-direction threshold isolates the vector ordering:
  // SuMa uses [translation, rotation], unlike DiLO's [rotation, translation].
  params.degenerate_ratio = 2.0;
  SuMaPaper remapped(params);
  const auto result = remapped.alignProjective(data, model, Eigen::Matrix4d::Identity(), 2, 30);
  EXPECT_TRUE(result.allFinite());
  EXPECT_LT((result.block<3, 1>(0, 3).norm()), 1e-12);
  EXPECT_GT(rotationErrorDeg(result, Eigen::Matrix4d::Identity()), 0.01);
  EXPECT_TRUE((result.topLeftCorner<3, 3>().isApprox(control.topLeftCorner<3, 3>(), 1e-14)));
  params.degenerate_ratio = 0.0;
  SuMaPaper disabled(params);
  const auto unchanged = disabled.alignProjective(data, model, Eigen::Matrix4d::Identity(), 2, 30);
  EXPECT_EQ(unchanged, control);
}

TEST(SuMaPaperTest, MapUpdateIntegratesRepeatedObservations) {
  SuMaPaper suma;
  const auto scan = simulateScan(Eigen::Matrix4d::Identity());
  const auto first = suma.process(scan);
  const size_t after_first = suma.surfels().size();
  EXPECT_EQ(static_cast<size_t>(first.new_surfels), after_first);
  const auto second = suma.process(scan);
  // Almost every measurement is compatible with an existing surfel.
  EXPECT_GT(second.updated_surfels, static_cast<int>(0.8 * after_first));
  const int max_new = static_cast<int>(0.2 * after_first);
  EXPECT_LT(second.new_surfels, max_new);
  int stable = 0;
  for (const auto& s : suma.surfels()) stable += s.confidence > 0.0f;
  EXPECT_GT(stable, static_cast<int>(0.8 * after_first));
  // A surfel radius follows eq. 10 (clamped to [0.05, 1] m).
  for (const auto& s : suma.surfels()) {
    EXPECT_GE(s.radius, 0.05f - 1e-6f);
    EXPECT_LE(s.radius, 1.0f + 1e-6f);
  }
}

TEST(SuMaPaperTest, RecoversKnownMotion) {
  SuMaPaper suma;
  // 1 m per scan forward with a slow turn and a slight climb.
  std::vector<Eigen::Matrix4d> truth;
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  const Eigen::Matrix4d step = makePose(1.0, 0.0, 0.01, 0.0, 0.0, 0.004);
  for (int k = 0; k < 15; ++k) {
    truth.push_back(T);
    T = T * step;
  }
  SuMaPaperResult last;
  for (const auto& pose : truth) last = suma.process(simulateScan(pose));
  const Eigen::Matrix4d& end = truth.back();
  const double end_t = (last.pose.block<3, 1>(0, 3) - end.block<3, 1>(0, 3)).norm();
  EXPECT_LT(end_t, 0.1);
  EXPECT_LT(rotationErrorDeg(last.pose, end), 0.2);
  EXPECT_GT(last.inliers, 5000);
  EXPECT_FALSE(last.fallback);
}
