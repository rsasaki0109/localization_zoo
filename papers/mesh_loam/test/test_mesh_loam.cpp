#include "mesh_loam/mesh_loam.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace localization_zoo::mesh_loam;

namespace localization_zoo {
namespace mesh_loam {
struct MeshLoamTestAccess {
  static void checkNormalBounds() {
    MeshLoamPipeline::FacetBin bin;
    bin.buildNormalBounds();
    EXPECT_TRUE(std::isinf(bin.normalUpper(Eigen::Vector3d::UnitZ())));
    for (int i = 0; i < 65; ++i) {
      MeshLoamPipeline::Facet f;
      const auto R = Eigen::AngleAxisf(.001f * i, Eigen::Vector3f::UnitX()).toRotationMatrix();
      f.a = Eigen::Vector3f::Zero();
      f.b = R * Eigen::Vector3f::UnitX();
      f.c = R * Eigen::Vector3f::UnitY();
      f.n = f.b.cross(f.c).normalized();
      if (i % 2) f.n = -f.n;  // |cos| gate must also cover reversed windings
      f.owner[0] = f.owner[1] = f.owner[2] = 0;
      bin.add(f);
    }
    bin.buildNormalBounds();
    EXPECT_LT(bin.normalUpper(Eigen::Vector3d::UnitX()), .98);
    EXPECT_GE(bin.normalUpper(Eigen::Vector3d::UnitZ()), 1.0);
    // Direct enumeration is an independent oracle for every query direction,
    // both signs, and nonunit vectors. Include queries at the cosine gate.
    for (int i = -100; i <= 100; ++i) {
      for (double scale : {1.0, -1.0, 1e6}) {
        const Eigen::Vector3d q = scale * Eigen::Vector3d(.01 * i, .007 * i, 1.0).normalized();
        double actual = 0;
        for (const auto& f : bin.facets)
          actual = std::max(actual, std::abs(f.n.cast<double>().dot(q)));
        EXPECT_GE(bin.normalUpper(q), actual);
      }
    }
    for (double cosine : {std::nextafter(.98, 0.0), .98, std::nextafter(.98, 1.0)}) {
      const Eigen::Vector3d q(std::sqrt(1.0 - cosine * cosine), 0.0, cosine);
      EXPECT_GE(bin.normalUpper(q), cosine);
    }
    auto copied = bin;
    // Removing/replacing facets must rebuild the bound, while a copied bin
    // keeps the normals that were present when it was copied.
    bin.facets.erase(bin.facets.begin() + 1, bin.facets.end());
    bin.facets.front().n = Eigen::Vector3f::UnitX();
    bin.buildNormalBounds();
    EXPECT_LT(bin.normalUpper(Eigen::Vector3d::UnitZ()), .98);
    EXPECT_GE(copied.normalUpper(Eigen::Vector3d::UnitZ()), 1.0);
  }
};
}  // namespace mesh_loam
}  // namespace localization_zoo

TEST(MeshLoam, BinNormalBoundsConservativelyCoverSignedFacetNormals) {
  MeshLoamTestAccess::checkNormalBounds();
}

namespace {

/// Points on the walls/floor/ceiling of a box room, sampled on a grid in the
/// sensor frame (sensor at `origin` inside the room, axis-aligned).
std::vector<Eigen::Vector3d> makeRoomScan(const Eigen::Vector3d& origin,
                                          double half_x = 4.0, double half_y = 3.0,
                                          double height = 2.5, double step = 0.06) {
  std::vector<Eigen::Vector3d> pts;
  const double x0 = -half_x - origin.x(), x1 = half_x - origin.x();
  const double y0 = -half_y - origin.y(), y1 = half_y - origin.y();
  const double z0 = -origin.z(), z1 = height - origin.z();
  for (double x = x0; x <= x1; x += step) {
    for (double z = z0; z <= z1; z += step) {
      pts.emplace_back(x, y0, z);
      pts.emplace_back(x, y1, z);
    }
  }
  for (double y = y0; y <= y1; y += step) {
    for (double z = z0; z <= z1; z += step) {
      pts.emplace_back(x0, y, z);
      pts.emplace_back(x1, y, z);
    }
  }
  for (double x = x0; x <= x1; x += step) {
    for (double y = y0; y <= y1; y += step) {
      pts.emplace_back(x, y, z0);
    }
  }
  return pts;
}

MeshLoamParams testParams() {
  MeshLoamParams params;
  params.min_range = 0.2;
  params.max_range = 30.0;
  params.voxel_size = 0.1;
  params.mesh_update_interval = 1;
  params.min_correspondences = 20;
  return params;
}

}  // namespace

TEST(MeshLoam, FirstFrameBuildsSdfAndMesh) {
  MeshLoamPipeline pipeline(testParams());
  const auto res = pipeline.registerFrame(makeRoomScan(Eigen::Vector3d(0, 0, 1.2)));
  EXPECT_TRUE(res.converged);
  EXPECT_GT(pipeline.mapSize(), static_cast<size_t>(1000));
  EXPECT_GT(pipeline.meshSize(), static_cast<size_t>(500));
}

TEST(MeshLoam, StaticSensorStaysPut) {
  MeshLoamPipeline pipeline(testParams());
  const auto scan = makeRoomScan(Eigen::Vector3d(0, 0, 1.2));
  pipeline.registerFrame(scan);
  for (int i = 0; i < 3; ++i) {
    const auto res = pipeline.registerFrame(scan);
    EXPECT_TRUE(res.converged);
  }
  EXPECT_LT((pipeline.pose().block<3, 1>(0, 3)).norm(), 0.05);
}

TEST(MeshLoam, RecoversSmallTranslation) {
  MeshLoamPipeline pipeline(testParams());
  pipeline.registerFrame(makeRoomScan(Eigen::Vector3d(0, 0, 1.2)));
  const Eigen::Vector3d moved(0.15, 0.05, 0.0);
  const auto res =
      pipeline.registerFrame(makeRoomScan(Eigen::Vector3d(0, 0, 1.2) + moved));
  EXPECT_TRUE(res.converged);
  const Eigen::Vector3d est = res.pose.block<3, 1>(0, 3);
  EXPECT_LT((est - moved).norm(), 0.05);
}

TEST(MeshLoam, TracksTranslationSequence) {
  MeshLoamPipeline pipeline(testParams());
  const Eigen::Vector3d base(0, 0, 1.2);
  pipeline.registerFrame(makeRoomScan(base));
  const int steps = 8;
  const Eigen::Vector3d delta(0.15, 0.0, 0.0);
  for (int i = 1; i <= steps; ++i) {
    const auto res = pipeline.registerFrame(makeRoomScan(base + i * delta));
    EXPECT_TRUE(res.converged) << "frame " << i;
  }
  const Eigen::Vector3d expected = steps * delta;
  const Eigen::Vector3d est = pipeline.pose().block<3, 1>(0, 3);
  EXPECT_LT((est - expected).norm(), 0.1);
}

TEST(MeshLoam, RecoversSmallYaw) {
  MeshLoamPipeline pipeline(testParams());
  pipeline.registerFrame(makeRoomScan(Eigen::Vector3d(0, 0, 1.2)));
  // Rotate the same world points into a yawed sensor frame.
  const double yaw = 0.03;
  const Eigen::Matrix3d R_ws =
      Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  auto scan = makeRoomScan(Eigen::Vector3d(0, 0, 1.2));
  for (auto& p : scan) p = R_ws.transpose() * p;
  const auto res = pipeline.registerFrame(scan);
  EXPECT_TRUE(res.converged);
  const Eigen::Matrix3d R_est = res.pose.block<3, 3>(0, 0);
  const double yaw_est = std::atan2(R_est(1, 0), R_est(0, 0));
  EXPECT_NEAR(yaw_est, yaw, 0.01);
  EXPECT_LT((res.pose.block<3, 1>(0, 3)).norm(), 0.05);
}

TEST(MeshLoam, NearestFacetRecoversTranslationAndYaw) {
  MeshLoamParams params = testParams();
  params.nearest_facet = true;
  MeshLoamPipeline pipeline(params);
  pipeline.registerFrame(makeRoomScan(Eigen::Vector3d(0, 0, 1.2)));
  const double yaw = 0.02;
  const Eigen::Vector3d moved(0.1, 0.05, 0.0);
  const Eigen::Matrix3d R_ws =
      Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  auto scan = makeRoomScan(Eigen::Vector3d(0, 0, 1.2) + moved);
  for (auto& p : scan) p = R_ws.transpose() * p;
  const auto res = pipeline.registerFrame(scan);
  EXPECT_TRUE(res.converged);
  const Eigen::Matrix3d R_est = res.pose.block<3, 3>(0, 0);
  EXPECT_NEAR(std::atan2(R_est(1, 0), R_est(0, 0)), yaw, 0.005);
  EXPECT_LT((res.pose.block<3, 1>(0, 3) - moved).norm(), 0.03);
}

TEST(MeshLoam, KeepMeshSurvivesThePrune) {
  MeshLoamParams params = testParams();
  params.local_map_radius = 1.0;  // prunes almost the whole room
  params.keep_mesh = true;
  MeshLoamPipeline kept(params);
  params.keep_mesh = false;
  MeshLoamPipeline pruned(params);
  const auto scan = makeRoomScan(Eigen::Vector3d(0, 0, 1.2));
  for (int i = 0; i < 2; ++i) {
    kept.registerFrame(scan);
    pruned.registerFrame(scan);
  }
  EXPECT_LT(kept.mapSize(), static_cast<size_t>(5000));
  EXPECT_GT(kept.liveFacets(), 2 * pruned.liveFacets());
}

namespace {

using FacetKey = std::array<float, 9>;

std::vector<FacetKey> facetSet(const MeshLoamPipeline& pipeline) {
  std::vector<FacetKey> out;
  pipeline.forEachFacet([&](const Eigen::Vector3d& a, const Eigen::Vector3d& b,
                            const Eigen::Vector3d& c, const Eigen::Vector3d&) {
    out.push_back({static_cast<float>(a.x()), static_cast<float>(a.y()), static_cast<float>(a.z()),
                   static_cast<float>(b.x()), static_cast<float>(b.y()), static_cast<float>(b.z()),
                   static_cast<float>(c.x()), static_cast<float>(c.y()), static_cast<float>(c.z())});
  });
  std::sort(out.begin(), out.end());
  return out;
}

/// Fraction of facets whose normal points toward `viewpoint` (the free-space
/// side of a room scanned from there).
double fractionFacingViewpoint(const MeshLoamPipeline& pipeline, const Eigen::Vector3d& viewpoint) {
  size_t good = 0, total = 0, bad_winding = 0;
  pipeline.forEachFacet([&](const Eigen::Vector3d& a, const Eigen::Vector3d& b,
                            const Eigen::Vector3d& c, const Eigen::Vector3d& n) {
    const Eigen::Vector3d centroid = (a + b + c) / 3.0;
    ++total;
    if (n.dot(viewpoint - centroid) > 0.0) ++good;
    // Winding agrees with the stored normal.
    if (n.dot((b - a).cross(c - a)) <= 0.0) ++bad_winding;
  });
  EXPECT_EQ(bad_winding, static_cast<size_t>(0));
  return total > 0 ? static_cast<double>(good) / total : 0.0;
}

/// Street-like scene: floor, two walls 4 m apart along x, and pillars every
/// 3 m, sampled within `range` of the sensor at `origin` (sensor frame).
std::vector<Eigen::Vector3d> makeStreetScan(const Eigen::Vector3d& origin, double range = 14.0,
                                            double step = 0.08) {
  std::vector<Eigen::Vector3d> pts;
  auto add = [&](const Eigen::Vector3d& w) {
    if ((w - origin).norm() < range) pts.push_back(w - origin);
  };
  const double x0 = origin.x() - range, x1 = origin.x() + range;
  for (double x = std::floor(x0 / step) * step; x <= x1; x += step) {
    for (double y = -2.0; y <= 2.0; y += step) add(Eigen::Vector3d(x, y, 0.0));
    for (double z = 0.0; z <= 2.5; z += step) {
      add(Eigen::Vector3d(x, -2.0, z));
      add(Eigen::Vector3d(x, 2.0, z));
    }
  }
  // Pillars (0.4 m squares) on both sides every 3 m.
  for (double px = std::floor(x0 / 3.0) * 3.0; px <= x1; px += 3.0) {
    for (const double py : {-1.6, 1.6}) {
      for (double z = 0.0; z <= 2.5; z += step) {
        for (double u = -0.2; u <= 0.2; u += step) {
          add(Eigen::Vector3d(px - 0.2, py + u, z));
          add(Eigen::Vector3d(px + 0.2, py + u, z));
          add(Eigen::Vector3d(px + u, py - 0.2, z));
          add(Eigen::Vector3d(px + u, py + 0.2, z));
        }
      }
    }
  }
  return pts;
}

}  // namespace

TEST(MeshLoam, FacetNormalsPointToFreeSpace) {
  for (const bool mc : {false, true}) {
    MeshLoamParams params = testParams();
    params.marching_cubes = mc;
    MeshLoamPipeline pipeline(params);
    const Eigen::Vector3d sensor(0, 0, 1.2);
    pipeline.registerFrame(makeRoomScan(sensor));
    ASSERT_GT(pipeline.meshSize(), static_cast<size_t>(500));
    EXPECT_GT(fractionFacingViewpoint(pipeline, sensor), 0.97) << "marching_cubes=" << mc;
  }
}

TEST(MeshLoam, MarchingCubesUsesFewerFacets) {
  MeshLoamParams params = testParams();
  MeshLoamPipeline tets(params);
  params.marching_cubes = true;
  MeshLoamPipeline cubes(params);
  const auto scan = makeRoomScan(Eigen::Vector3d(0, 0, 1.2));
  tets.registerFrame(scan);
  cubes.registerFrame(scan);
  EXPECT_GT(cubes.meshSize(), static_cast<size_t>(500));
  EXPECT_LT(cubes.meshSize() * 3, tets.meshSize() * 2);
}

TEST(MeshLoam, MarchingCubesTracksTranslationAndYaw) {
  MeshLoamParams params = testParams();
  params.marching_cubes = true;
  params.nearest_facet = true;
  MeshLoamPipeline pipeline(params);
  pipeline.registerFrame(makeRoomScan(Eigen::Vector3d(0, 0, 1.2)));
  const double yaw = 0.02;
  const Eigen::Vector3d moved(0.1, 0.05, 0.0);
  const Eigen::Matrix3d R_ws =
      Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  auto scan = makeRoomScan(Eigen::Vector3d(0, 0, 1.2) + moved);
  for (auto& p : scan) p = R_ws.transpose() * p;
  const auto res = pipeline.registerFrame(scan);
  EXPECT_TRUE(res.converged);
  const Eigen::Matrix3d R_est = res.pose.block<3, 3>(0, 0);
  EXPECT_NEAR(std::atan2(R_est(1, 0), R_est(0, 0)), yaw, 0.005);
  EXPECT_LT((res.pose.block<3, 1>(0, 3) - moved).norm(), 0.03);
}

TEST(MeshLoam, IncrementalCellsMatchFullRemesh) {
  for (const bool mc : {false, true}) {
    MeshLoamParams params = testParams();
    params.marching_cubes = mc;
    params.mesh_update_interval = 2;
    MeshLoamPipeline pipeline(params);
    const Eigen::Vector3d base(0, 0, 1.2);
    for (int i = 0; i < 5; ++i) {
      pipeline.registerFrame(makeRoomScan(base + Eigen::Vector3d(0.07 * i, 0.03 * i, 0.0)));
    }
    pipeline.extractMesh();
    const auto incremental = facetSet(pipeline);
    pipeline.remeshAll();
    const auto full = facetSet(pipeline);
    ASSERT_GT(full.size(), static_cast<size_t>(500));
    EXPECT_EQ(incremental.size(), full.size()) << "marching_cubes=" << mc;
    EXPECT_TRUE(incremental == full) << "marching_cubes=" << mc;
  }
}

TEST(MeshLoam, DirtyCellsTouchFewerCellsThanBlocks) {
  MeshLoamParams params = testParams();
  MeshLoamPipeline cells(params);
  params.mesh_dirty_cells = false;
  MeshLoamPipeline blocks(params);
  const auto scan = makeRoomScan(Eigen::Vector3d(0, 0, 1.2));
  for (int i = 0; i < 2; ++i) {
    cells.registerFrame(scan);
    blocks.registerFrame(scan);
  }
  EXPECT_LT(cells.timings().extracted_cells, blocks.timings().extracted_cells);
  // Same surface either way (block mode can only miss boundary updates).
  const double ratio = static_cast<double>(cells.meshSize()) / blocks.meshSize();
  EXPECT_NEAR(ratio, 1.0, 0.02);
}

TEST(MeshLoam, BoundedAssociationRetainsPosesAcrossMeshUpdatesAndPruning) {
#ifdef _OPENMP
  const int original_threads = omp_get_max_threads();
  omp_set_num_threads(1);  // isolate association from the legacy parallel sums
#endif
  for (bool marching_cubes : {false, true}) {
    MeshLoamParams params = testParams();
    params.nearest_facet = true;
    params.marching_cubes = marching_cubes;
    params.mesh_update_interval = 2;
    params.local_map_radius = 4.0;
    MeshLoamPipeline legacy(params);
    params.accelerated = true;
    MeshLoamPipeline indexed(params);
    for (int i = 0; i < 7; ++i) {
      const auto scan = makeRoomScan(Eigen::Vector3d(.08 * i, .02 * i, 1.2));
      const auto a = legacy.registerFrame(scan), b = indexed.registerFrame(scan);
      EXPECT_EQ(a.num_correspondences, b.num_correspondences);
      EXPECT_EQ(legacy.meshSize(), indexed.meshSize());
      EXPECT_TRUE((a.pose.array() == b.pose.array()).all()) << "frame=" << i;
    }
  }
#ifdef _OPENMP
  omp_set_num_threads(original_threads);
#endif
}

TEST(MeshLoam, FrozenMeshIsKeptAndRestored) {
  MeshLoamParams params = testParams();
  params.accelerated = true;  // compare the same cache footprint with/without freeze
  params.max_range = 10.0;
  params.local_map_radius = 11.0;
  params.keep_mesh = true;
  params.marching_cubes = true;
  params.nearest_facet = true;
  params.mesh_update_interval = 2;
  MeshLoamParams frozen_params = params;
  frozen_params.mesh_freeze_radius = 2.0;  // bins beyond ~13 m are frozen
  MeshLoamPipeline plain(params);
  MeshLoamPipeline frozen(frozen_params);

  // Drive along the street, turn around and come back.
  std::vector<double> xs = {0.0};
  double v = 0.0;
  for (int i = 0; i < 64; ++i) {
    if (i < 8) {
      v += 0.1;  // accelerate to 0.8 m/frame
    } else if (i >= 26 && i < 42) {
      v -= 0.1;  // decelerate and reverse to -0.8 m/frame
    }
    xs.push_back(xs.back() + v);
  }
  ASSERT_GT(*std::max_element(xs.begin(), xs.end()), 20.0);
  ASSERT_LT(xs.back(), 6.0);
  size_t max_frozen = 0;
  for (size_t i = 0; i < xs.size(); ++i) {
    const Eigen::Vector3d origin(xs[i], 0.0, 1.0);
    const auto scan = makeStreetScan(origin, params.max_range, 0.15);
    plain.registerFrame(scan);
    const auto res = frozen.registerFrame(scan);
    EXPECT_LT((res.pose.block<3, 1>(0, 3) - Eigen::Vector3d(xs[i], 0.0, 0.0)).norm(), 0.15)
        << "frame " << i;
    max_frozen = std::max(max_frozen, frozen.frozenFacets());
  }
  EXPECT_GT(max_frozen, static_cast<size_t>(1000));
  // Coming back thawed the area near the start again.
  EXPECT_LT(frozen.frozenFacets(), max_frozen);
  // Same global mesh size up to quantization-collapsed slivers, and the
  // compressed store is smaller.
  const double ratio = static_cast<double>(frozen.meshSize()) / plain.meshSize();
  EXPECT_NEAR(ratio, 1.0, 0.03);
  EXPECT_LT(frozen.memoryBytes(), plain.memoryBytes());
}
