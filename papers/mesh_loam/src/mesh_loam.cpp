#include "mesh_loam/mesh_loam.h"

#include <Eigen/Dense>

#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <cmath>
#include <numeric>
#include <limits>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace localization_zoo {
namespace mesh_loam {

namespace {

using Clock = std::chrono::steady_clock;

double msSince(const Clock::time_point& t0) {
  return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

int maxThreads() {
#ifdef _OPENMP
  return std::max(1, omp_get_max_threads());
#else
  return 1;
#endif
}

int threadId() {
#ifdef _OPENMP
  return omp_get_thread_num();
#else
  return 0;
#endif
}

inline int floorDiv(int v, int b) { return v >= 0 ? v / b : -((-v + b - 1) / b); }

Eigen::Matrix3d expSo3(const Eigen::Vector3d& w) {
  const double theta = w.norm();
  if (theta < 1e-12) return Eigen::Matrix3d::Identity();
  return Eigen::AngleAxisd(theta, w / theta).toRotationMatrix();
}

// Squared distance from p to the triangle (a, b, c) (Ericson, Real-Time
// Collision Detection, 5.1.5).
double pointTriangleDist2(const Eigen::Vector3d& p, const Eigen::Vector3d& a,
                          const Eigen::Vector3d& b, const Eigen::Vector3d& c) {
  const Eigen::Vector3d ab = b - a, ac = c - a, ap = p - a;
  const double d1 = ab.dot(ap), d2 = ac.dot(ap);
  if (d1 <= 0.0 && d2 <= 0.0) return ap.squaredNorm();
  const Eigen::Vector3d bp = p - b;
  const double d3 = ab.dot(bp), d4 = ac.dot(bp);
  if (d3 >= 0.0 && d4 <= d3) return bp.squaredNorm();
  const double vc = d1 * d4 - d3 * d2;
  if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0) {
    const double v = d1 / (d1 - d3);
    return (p - (a + v * ab)).squaredNorm();
  }
  const Eigen::Vector3d cp = p - c;
  const double d5 = ab.dot(cp), d6 = ac.dot(cp);
  if (d6 >= 0.0 && d5 <= d6) return cp.squaredNorm();
  const double vb = d5 * d2 - d1 * d6;
  if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0) {
    const double w = d2 / (d2 - d6);
    return (p - (a + w * ac)).squaredNorm();
  }
  const double va = d3 * d6 - d5 * d4;
  if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0) {
    const double w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
    return (p - (b + w * (c - b))).squaredNorm();
  }
  const double denom = 1.0 / (va + vb + vc);
  const double v = vb * denom, w = vc * denom;
  return (p - (a + ab * v + ac * w)).squaredNorm();
}

uint32_t mixKey(const Eigen::Vector3i& k) {
  uint64_t h = static_cast<uint64_t>(static_cast<uint32_t>(k.x())) * 0x9E3779B97F4A7C15ULL;
  h ^= static_cast<uint64_t>(static_cast<uint32_t>(k.y())) * 0xC2B2AE3D27D4EB4FULL;
  h ^= static_cast<uint64_t>(static_cast<uint32_t>(k.z())) * 0x165667B19E3779F9ULL;
  h ^= h >> 29;
  h *= 0xBF58476D1CE4E5B9ULL;
  h ^= h >> 32;
  return static_cast<uint32_t>(h);
}

// Cell corners (voxel offsets) in the Bourke / Lorensen-Cline order.
const int kCorner[8][3] = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
                           {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}};
const int kTets[6][4] = {{0, 5, 1, 6}, {0, 1, 2, 6}, {0, 2, 3, 6},
                         {0, 3, 7, 6}, {0, 7, 4, 6}, {0, 4, 5, 6}};
const int kEdge[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6},
                          {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

// Standard marching-cubes triangle table (Lorensen & Cline 1987, as
// published by Paul Bourke, "Polygonising a scalar field"). Bit i of the case
// index is set when corner i is inside (SDF < 0). Triangles are listed as
// edge indices; their winding gives a normal toward the inside.
const int8_t kTriTable[256][16] = {
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,8,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,1,9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {1,8,3,9,8,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {1,2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,8,3,1,2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {9,2,10,0,2,9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {2,8,3,2,10,8,10,9,8,-1,-1,-1,-1,-1,-1,-1},
    {3,11,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,11,2,8,11,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {1,9,0,2,3,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {1,11,2,1,9,11,9,8,11,-1,-1,-1,-1,-1,-1,-1},
    {3,10,1,11,10,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,10,1,0,8,10,8,11,10,-1,-1,-1,-1,-1,-1,-1},
    {3,9,0,3,11,9,11,10,9,-1,-1,-1,-1,-1,-1,-1},
    {9,8,10,10,8,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {4,7,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {4,3,0,7,3,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,1,9,8,4,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {4,1,9,4,7,1,7,3,1,-1,-1,-1,-1,-1,-1,-1},
    {1,2,10,8,4,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {3,4,7,3,0,4,1,2,10,-1,-1,-1,-1,-1,-1,-1},
    {9,2,10,9,0,2,8,4,7,-1,-1,-1,-1,-1,-1,-1},
    {2,10,9,2,9,7,2,7,3,7,9,4,-1,-1,-1,-1},
    {8,4,7,3,11,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {11,4,7,11,2,4,2,0,4,-1,-1,-1,-1,-1,-1,-1},
    {9,0,1,8,4,7,2,3,11,-1,-1,-1,-1,-1,-1,-1},
    {4,7,11,9,4,11,9,11,2,9,2,1,-1,-1,-1,-1},
    {3,10,1,3,11,10,7,8,4,-1,-1,-1,-1,-1,-1,-1},
    {1,11,10,1,4,11,1,0,4,7,11,4,-1,-1,-1,-1},
    {4,7,8,9,0,11,9,11,10,11,0,3,-1,-1,-1,-1},
    {4,7,11,4,11,9,9,11,10,-1,-1,-1,-1,-1,-1,-1},
    {9,5,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {9,5,4,0,8,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,5,4,1,5,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {8,5,4,8,3,5,3,1,5,-1,-1,-1,-1,-1,-1,-1},
    {1,2,10,9,5,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {3,0,8,1,2,10,4,9,5,-1,-1,-1,-1,-1,-1,-1},
    {5,2,10,5,4,2,4,0,2,-1,-1,-1,-1,-1,-1,-1},
    {2,10,5,3,2,5,3,5,4,3,4,8,-1,-1,-1,-1},
    {9,5,4,2,3,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,11,2,0,8,11,4,9,5,-1,-1,-1,-1,-1,-1,-1},
    {0,5,4,0,1,5,2,3,11,-1,-1,-1,-1,-1,-1,-1},
    {2,1,5,2,5,8,2,8,11,4,8,5,-1,-1,-1,-1},
    {10,3,11,10,1,3,9,5,4,-1,-1,-1,-1,-1,-1,-1},
    {4,9,5,0,8,1,8,10,1,8,11,10,-1,-1,-1,-1},
    {5,4,0,5,0,11,5,11,10,11,0,3,-1,-1,-1,-1},
    {5,4,8,5,8,10,10,8,11,-1,-1,-1,-1,-1,-1,-1},
    {9,7,8,5,7,9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {9,3,0,9,5,3,5,7,3,-1,-1,-1,-1,-1,-1,-1},
    {0,7,8,0,1,7,1,5,7,-1,-1,-1,-1,-1,-1,-1},
    {1,5,3,3,5,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {9,7,8,9,5,7,10,1,2,-1,-1,-1,-1,-1,-1,-1},
    {10,1,2,9,5,0,5,3,0,5,7,3,-1,-1,-1,-1},
    {8,0,2,8,2,5,8,5,7,10,5,2,-1,-1,-1,-1},
    {2,10,5,2,5,3,3,5,7,-1,-1,-1,-1,-1,-1,-1},
    {7,9,5,7,8,9,3,11,2,-1,-1,-1,-1,-1,-1,-1},
    {9,5,7,9,7,2,9,2,0,2,7,11,-1,-1,-1,-1},
    {2,3,11,0,1,8,1,7,8,1,5,7,-1,-1,-1,-1},
    {11,2,1,11,1,7,7,1,5,-1,-1,-1,-1,-1,-1,-1},
    {9,5,8,8,5,7,10,1,3,10,3,11,-1,-1,-1,-1},
    {5,7,0,5,0,9,7,11,0,1,0,10,11,10,0,-1},
    {11,10,0,11,0,3,10,5,0,8,0,7,5,7,0,-1},
    {11,10,5,7,11,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {10,6,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,8,3,5,10,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {9,0,1,5,10,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {1,8,3,1,9,8,5,10,6,-1,-1,-1,-1,-1,-1,-1},
    {1,6,5,2,6,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {1,6,5,1,2,6,3,0,8,-1,-1,-1,-1,-1,-1,-1},
    {9,6,5,9,0,6,0,2,6,-1,-1,-1,-1,-1,-1,-1},
    {5,9,8,5,8,2,5,2,6,3,2,8,-1,-1,-1,-1},
    {2,3,11,10,6,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {11,0,8,11,2,0,10,6,5,-1,-1,-1,-1,-1,-1,-1},
    {0,1,9,2,3,11,5,10,6,-1,-1,-1,-1,-1,-1,-1},
    {5,10,6,1,9,2,9,11,2,9,8,11,-1,-1,-1,-1},
    {6,3,11,6,5,3,5,1,3,-1,-1,-1,-1,-1,-1,-1},
    {0,8,11,0,11,5,0,5,1,5,11,6,-1,-1,-1,-1},
    {3,11,6,0,3,6,0,6,5,0,5,9,-1,-1,-1,-1},
    {6,5,9,6,9,11,11,9,8,-1,-1,-1,-1,-1,-1,-1},
    {5,10,6,4,7,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {4,3,0,4,7,3,6,5,10,-1,-1,-1,-1,-1,-1,-1},
    {1,9,0,5,10,6,8,4,7,-1,-1,-1,-1,-1,-1,-1},
    {10,6,5,1,9,7,1,7,3,7,9,4,-1,-1,-1,-1},
    {6,1,2,6,5,1,4,7,8,-1,-1,-1,-1,-1,-1,-1},
    {1,2,5,5,2,6,3,0,4,3,4,7,-1,-1,-1,-1},
    {8,4,7,9,0,5,0,6,5,0,2,6,-1,-1,-1,-1},
    {7,3,9,7,9,4,3,2,9,5,9,6,2,6,9,-1},
    {3,11,2,7,8,4,10,6,5,-1,-1,-1,-1,-1,-1,-1},
    {5,10,6,4,7,2,4,2,0,2,7,11,-1,-1,-1,-1},
    {0,1,9,4,7,8,2,3,11,5,10,6,-1,-1,-1,-1},
    {9,2,1,9,11,2,9,4,11,7,11,4,5,10,6,-1},
    {8,4,7,3,11,5,3,5,1,5,11,6,-1,-1,-1,-1},
    {5,1,11,5,11,6,1,0,11,7,11,4,0,4,11,-1},
    {0,5,9,0,6,5,0,3,6,11,6,3,8,4,7,-1},
    {6,5,9,6,9,11,4,7,9,7,11,9,-1,-1,-1,-1},
    {10,4,9,6,4,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {4,10,6,4,9,10,0,8,3,-1,-1,-1,-1,-1,-1,-1},
    {10,0,1,10,6,0,6,4,0,-1,-1,-1,-1,-1,-1,-1},
    {8,3,1,8,1,6,8,6,4,6,1,10,-1,-1,-1,-1},
    {1,4,9,1,2,4,2,6,4,-1,-1,-1,-1,-1,-1,-1},
    {3,0,8,1,2,9,2,4,9,2,6,4,-1,-1,-1,-1},
    {0,2,4,4,2,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {8,3,2,8,2,4,4,2,6,-1,-1,-1,-1,-1,-1,-1},
    {10,4,9,10,6,4,11,2,3,-1,-1,-1,-1,-1,-1,-1},
    {0,8,2,2,8,11,4,9,10,4,10,6,-1,-1,-1,-1},
    {3,11,2,0,1,6,0,6,4,6,1,10,-1,-1,-1,-1},
    {6,4,1,6,1,10,4,8,1,2,1,11,8,11,1,-1},
    {9,6,4,9,3,6,9,1,3,11,6,3,-1,-1,-1,-1},
    {8,11,1,8,1,0,11,6,1,9,1,4,6,4,1,-1},
    {3,11,6,3,6,0,0,6,4,-1,-1,-1,-1,-1,-1,-1},
    {6,4,8,11,6,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {7,10,6,7,8,10,8,9,10,-1,-1,-1,-1,-1,-1,-1},
    {0,7,3,0,10,7,0,9,10,6,7,10,-1,-1,-1,-1},
    {10,6,7,1,10,7,1,7,8,1,8,0,-1,-1,-1,-1},
    {10,6,7,10,7,1,1,7,3,-1,-1,-1,-1,-1,-1,-1},
    {1,2,6,1,6,8,1,8,9,8,6,7,-1,-1,-1,-1},
    {2,6,9,2,9,1,6,7,9,0,9,3,7,3,9,-1},
    {7,8,0,7,0,6,6,0,2,-1,-1,-1,-1,-1,-1,-1},
    {7,3,2,6,7,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {2,3,11,10,6,8,10,8,9,8,6,7,-1,-1,-1,-1},
    {2,0,7,2,7,11,0,9,7,6,7,10,9,10,7,-1},
    {1,8,0,1,7,8,1,10,7,6,7,10,2,3,11,-1},
    {11,2,1,11,1,7,10,6,1,6,7,1,-1,-1,-1,-1},
    {8,9,6,8,6,7,9,1,6,11,6,3,1,3,6,-1},
    {0,9,1,11,6,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {7,8,0,7,0,6,3,11,0,11,6,0,-1,-1,-1,-1},
    {7,11,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {7,6,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {3,0,8,11,7,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,1,9,11,7,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {8,1,9,8,3,1,11,7,6,-1,-1,-1,-1,-1,-1,-1},
    {10,1,2,6,11,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {1,2,10,3,0,8,6,11,7,-1,-1,-1,-1,-1,-1,-1},
    {2,9,0,2,10,9,6,11,7,-1,-1,-1,-1,-1,-1,-1},
    {6,11,7,2,10,3,10,8,3,10,9,8,-1,-1,-1,-1},
    {7,2,3,6,2,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {7,0,8,7,6,0,6,2,0,-1,-1,-1,-1,-1,-1,-1},
    {2,7,6,2,3,7,0,1,9,-1,-1,-1,-1,-1,-1,-1},
    {1,6,2,1,8,6,1,9,8,8,7,6,-1,-1,-1,-1},
    {10,7,6,10,1,7,1,3,7,-1,-1,-1,-1,-1,-1,-1},
    {10,7,6,1,7,10,1,8,7,1,0,8,-1,-1,-1,-1},
    {0,3,7,0,7,10,0,10,9,6,10,7,-1,-1,-1,-1},
    {7,6,10,7,10,8,8,10,9,-1,-1,-1,-1,-1,-1,-1},
    {6,8,4,11,8,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {3,6,11,3,0,6,0,4,6,-1,-1,-1,-1,-1,-1,-1},
    {8,6,11,8,4,6,9,0,1,-1,-1,-1,-1,-1,-1,-1},
    {9,4,6,9,6,3,9,3,1,11,3,6,-1,-1,-1,-1},
    {6,8,4,6,11,8,2,10,1,-1,-1,-1,-1,-1,-1,-1},
    {1,2,10,3,0,11,0,6,11,0,4,6,-1,-1,-1,-1},
    {4,11,8,4,6,11,0,2,9,2,10,9,-1,-1,-1,-1},
    {10,9,3,10,3,2,9,4,3,11,3,6,4,6,3,-1},
    {8,2,3,8,4,2,4,6,2,-1,-1,-1,-1,-1,-1,-1},
    {0,4,2,4,6,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {1,9,0,2,3,4,2,4,6,4,3,8,-1,-1,-1,-1},
    {1,9,4,1,4,2,2,4,6,-1,-1,-1,-1,-1,-1,-1},
    {8,1,3,8,6,1,8,4,6,6,10,1,-1,-1,-1,-1},
    {10,1,0,10,0,6,6,0,4,-1,-1,-1,-1,-1,-1,-1},
    {4,6,3,4,3,8,6,10,3,0,3,9,10,9,3,-1},
    {10,9,4,6,10,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {4,9,5,7,6,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,8,3,4,9,5,11,7,6,-1,-1,-1,-1,-1,-1,-1},
    {5,0,1,5,4,0,7,6,11,-1,-1,-1,-1,-1,-1,-1},
    {11,7,6,8,3,4,3,5,4,3,1,5,-1,-1,-1,-1},
    {9,5,4,10,1,2,7,6,11,-1,-1,-1,-1,-1,-1,-1},
    {6,11,7,1,2,10,0,8,3,4,9,5,-1,-1,-1,-1},
    {7,6,11,5,4,10,4,2,10,4,0,2,-1,-1,-1,-1},
    {3,4,8,3,5,4,3,2,5,10,5,2,11,7,6,-1},
    {7,2,3,7,6,2,5,4,9,-1,-1,-1,-1,-1,-1,-1},
    {9,5,4,0,8,6,0,6,2,6,8,7,-1,-1,-1,-1},
    {3,6,2,3,7,6,1,5,0,5,4,0,-1,-1,-1,-1},
    {6,2,8,6,8,7,2,1,8,4,8,5,1,5,8,-1},
    {9,5,4,10,1,6,1,7,6,1,3,7,-1,-1,-1,-1},
    {1,6,10,1,7,6,1,0,7,8,7,0,9,5,4,-1},
    {4,0,10,4,10,5,0,3,10,6,10,7,3,7,10,-1},
    {7,6,10,7,10,8,5,4,10,4,8,10,-1,-1,-1,-1},
    {6,9,5,6,11,9,11,8,9,-1,-1,-1,-1,-1,-1,-1},
    {3,6,11,0,6,3,0,5,6,0,9,5,-1,-1,-1,-1},
    {0,11,8,0,5,11,0,1,5,5,6,11,-1,-1,-1,-1},
    {6,11,3,6,3,5,5,3,1,-1,-1,-1,-1,-1,-1,-1},
    {1,2,10,9,5,11,9,11,8,11,5,6,-1,-1,-1,-1},
    {0,11,3,0,6,11,0,9,6,5,6,9,1,2,10,-1},
    {11,8,5,11,5,6,8,0,5,10,5,2,0,2,5,-1},
    {6,11,3,6,3,5,2,10,3,10,5,3,-1,-1,-1,-1},
    {5,8,9,5,2,8,5,6,2,3,8,2,-1,-1,-1,-1},
    {9,5,6,9,6,0,0,6,2,-1,-1,-1,-1,-1,-1,-1},
    {1,5,8,1,8,0,5,6,8,3,8,2,6,2,8,-1},
    {1,5,6,2,1,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {1,3,6,1,6,10,3,8,6,5,6,9,8,9,6,-1},
    {10,1,0,10,0,6,9,5,0,5,6,0,-1,-1,-1,-1},
    {0,3,8,5,6,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {10,5,6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {11,5,10,7,5,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {11,5,10,11,7,5,8,3,0,-1,-1,-1,-1,-1,-1,-1},
    {5,11,7,5,10,11,1,9,0,-1,-1,-1,-1,-1,-1,-1},
    {10,7,5,10,11,7,9,8,1,8,3,1,-1,-1,-1,-1},
    {11,1,2,11,7,1,7,5,1,-1,-1,-1,-1,-1,-1,-1},
    {0,8,3,1,2,7,1,7,5,7,2,11,-1,-1,-1,-1},
    {9,7,5,9,2,7,9,0,2,2,11,7,-1,-1,-1,-1},
    {7,5,2,7,2,11,5,9,2,3,2,8,9,8,2,-1},
    {2,5,10,2,3,5,3,7,5,-1,-1,-1,-1,-1,-1,-1},
    {8,2,0,8,5,2,8,7,5,10,2,5,-1,-1,-1,-1},
    {9,0,1,5,10,3,5,3,7,3,10,2,-1,-1,-1,-1},
    {9,8,2,9,2,1,8,7,2,10,2,5,7,5,2,-1},
    {1,3,5,3,7,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,8,7,0,7,1,1,7,5,-1,-1,-1,-1,-1,-1,-1},
    {9,0,3,9,3,5,5,3,7,-1,-1,-1,-1,-1,-1,-1},
    {9,8,7,5,9,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {5,8,4,5,10,8,10,11,8,-1,-1,-1,-1,-1,-1,-1},
    {5,0,4,5,11,0,5,10,11,11,3,0,-1,-1,-1,-1},
    {0,1,9,8,4,10,8,10,11,10,4,5,-1,-1,-1,-1},
    {10,11,4,10,4,5,11,3,4,9,4,1,3,1,4,-1},
    {2,5,1,2,8,5,2,11,8,4,5,8,-1,-1,-1,-1},
    {0,4,11,0,11,3,4,5,11,2,11,1,5,1,11,-1},
    {0,2,5,0,5,9,2,11,5,4,5,8,11,8,5,-1},
    {9,4,5,2,11,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {2,5,10,3,5,2,3,4,5,3,8,4,-1,-1,-1,-1},
    {5,10,2,5,2,4,4,2,0,-1,-1,-1,-1,-1,-1,-1},
    {3,10,2,3,5,10,3,8,5,4,5,8,0,1,9,-1},
    {5,10,2,5,2,4,1,9,2,9,4,2,-1,-1,-1,-1},
    {8,4,5,8,5,3,3,5,1,-1,-1,-1,-1,-1,-1,-1},
    {0,4,5,1,0,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {8,4,5,8,5,3,9,0,5,0,3,5,-1,-1,-1,-1},
    {9,4,5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {4,11,7,4,9,11,9,10,11,-1,-1,-1,-1,-1,-1,-1},
    {0,8,3,4,9,7,9,11,7,9,10,11,-1,-1,-1,-1},
    {1,10,11,1,11,4,1,4,0,7,4,11,-1,-1,-1,-1},
    {3,1,4,3,4,8,1,10,4,7,4,11,10,11,4,-1},
    {4,11,7,9,11,4,9,2,11,9,1,2,-1,-1,-1,-1},
    {9,7,4,9,11,7,9,1,11,2,11,1,0,8,3,-1},
    {11,7,4,11,4,2,2,4,0,-1,-1,-1,-1,-1,-1,-1},
    {11,7,4,11,4,2,8,3,4,3,2,4,-1,-1,-1,-1},
    {2,9,10,2,7,9,2,3,7,7,4,9,-1,-1,-1,-1},
    {9,10,7,9,7,4,10,2,7,8,7,0,2,0,7,-1},
    {3,7,10,3,10,2,7,4,10,1,10,0,4,0,10,-1},
    {1,10,2,8,7,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {4,9,1,4,1,7,7,1,3,-1,-1,-1,-1,-1,-1,-1},
    {4,9,1,4,1,7,0,8,1,8,7,1,-1,-1,-1,-1},
    {4,0,3,7,4,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {4,8,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {9,10,8,10,11,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {3,0,9,3,9,11,11,9,10,-1,-1,-1,-1,-1,-1,-1},
    {0,1,10,0,10,8,8,10,11,-1,-1,-1,-1,-1,-1,-1},
    {3,1,10,11,3,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {1,2,11,1,11,9,9,11,8,-1,-1,-1,-1,-1,-1,-1},
    {3,0,9,3,9,11,1,2,9,2,11,9,-1,-1,-1,-1},
    {0,2,11,8,0,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {3,2,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {2,3,8,2,8,10,10,8,9,-1,-1,-1,-1,-1,-1,-1},
    {9,10,2,0,9,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {2,3,8,2,8,10,0,1,8,1,10,8,-1,-1,-1,-1},
    {1,10,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {1,3,8,9,1,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,9,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {0,3,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
};

struct Triangle {
  Eigen::Vector3d a, b, c, n;  ///< n: unit normal toward positive SDF
};

// Marching tetrahedra on one cell (6 tetrahedra sharing the 0-6 diagonal).
template <typename Emit>
void marchTetrahedra(const double sdf[8], const Eigen::Vector3d pos[8], Emit&& emit) {
  auto emitOriented = [&](const Eigen::Vector3d& a, const Eigen::Vector3d& b,
                          const Eigen::Vector3d& c, const Eigen::Vector3d& inside) {
    Eigen::Vector3d n = (b - a).cross(c - a);
    const double len = n.norm();
    if (len < 1e-12) return;
    n /= len;
    // Orient from the negative (inside) side toward positive SDF.
    const Eigen::Vector3d centroid = (a + b + c) / 3.0;
    if (n.dot(centroid - inside) < 0.0) n = -n;
    emit(a, b, c, n);
  };
  for (const auto& tet : kTets) {
    const double d[4] = {sdf[tet[0]], sdf[tet[1]], sdf[tet[2]], sdf[tet[3]]};
    const Eigen::Vector3d* q[4] = {&pos[tet[0]], &pos[tet[1]], &pos[tet[2]], &pos[tet[3]]};
    int neg[4], np = 0;
    int posi[4], pp = 0;
    for (int i = 0; i < 4; ++i) {
      if (d[i] < 0.0) {
        neg[np++] = i;
      } else {
        posi[pp++] = i;
      }
    }
    if (np == 0 || pp == 0) continue;
    auto lerp = [&](int i, int j) {
      const double t = d[i] / (d[i] - d[j]);
      return (*q[i] + t * (*q[j] - *q[i])).eval();
    };
    if (np == 1 || pp == 1) {
      const int lone = (np == 1) ? neg[0] : posi[0];
      const int* others = (np == 1) ? posi : neg;
      const Eigen::Vector3d inside = (np == 1) ? *q[lone] : *q[others[0]];
      emitOriented(lerp(lone, others[0]), lerp(lone, others[1]), lerp(lone, others[2]), inside);
    } else {
      // 2-2 split: quad from the four crossing edges.
      const Eigen::Vector3d e00 = lerp(neg[0], posi[0]);
      const Eigen::Vector3d e01 = lerp(neg[0], posi[1]);
      const Eigen::Vector3d e10 = lerp(neg[1], posi[0]);
      const Eigen::Vector3d e11 = lerp(neg[1], posi[1]);
      const Eigen::Vector3d inside = 0.5 * (*q[neg[0]] + *q[neg[1]]);
      emitOriented(e00, e01, e11, inside);
      emitOriented(e00, e11, e10, inside);
    }
  }
}

// Marching cubes on one cell with the standard table.
template <typename Emit>
void marchCube(const double sdf[8], const Eigen::Vector3d pos[8], Emit&& emit) {
  int index = 0;
  for (int i = 0; i < 8; ++i) {
    if (sdf[i] < 0.0) index |= 1 << i;
  }
  if (index == 0 || index == 255) return;
  Eigen::Vector3d ev[12];
  int done = 0;
  const int8_t* tri = kTriTable[index];
  for (int k = 0; k < 16 && tri[k] >= 0; k += 3) {
    for (int m = 0; m < 3; ++m) {
      const int e = tri[k + m];
      if (done & (1 << e)) continue;
      const int i = kEdge[e][0], j = kEdge[e][1];
      const double t = sdf[i] / (sdf[i] - sdf[j]);
      ev[e] = pos[i] + t * (pos[j] - pos[i]);
      done |= 1 << e;
    }
    // Table winding points inside; swap two vertices to point outside.
    const Eigen::Vector3d& a = ev[tri[k]];
    const Eigen::Vector3d& b = ev[tri[k + 2]];
    const Eigen::Vector3d& c = ev[tri[k + 1]];
    Eigen::Vector3d n = (b - a).cross(c - a);
    const double len = n.norm();
    if (len < 1e-12) continue;
    emit(a, b, c, (n / len).eval());
  }
}

}  // namespace

void MeshLoamPipeline::FacetBin::add(const Facet& f) {
  facets.push_back(f);
  expand(f);
}

void MeshLoamPipeline::FacetBin::expand(const Facet& f) {
  lo = lo.cwiseMin(f.a).cwiseMin(f.b).cwiseMin(f.c);
  hi = hi.cwiseMax(f.a).cwiseMax(f.b).cwiseMax(f.c);
  const Eigen::Vector3f c = (f.a + f.b + f.c) / 3.0f;
  const float r2 = std::max({(f.a - c).squaredNorm(), (f.b - c).squaredNorm(),
                             (f.c - c).squaredNorm()});
  // Slightly inflated so that the float bound stays conservative.
  max_rad = std::max(max_rad, std::sqrt(r2) * 1.0001f + 1e-6f);
}

void MeshLoamPipeline::FacetBin::recomputeBounds() {
  lo = Eigen::Vector3f::Constant(1e30f);
  hi = Eigen::Vector3f::Constant(-1e30f);
  max_rad = 0.0f;
  for (const Facet& f : facets) expand(f);
}

void MeshLoamPipeline::FacetBin::buildFilters() {
  // A copied pipeline may share this read-only cache. Detach before rebuilding.
  if (!filters || !filters.unique()) filters = std::make_shared<Filters>();
  filters->stride = (facets.size() + 3) / 4 * 4;
  filters->values.resize(6 * filters->stride);
  for (size_t i = 0; i < facets.size(); ++i) {
    const Facet& f = facets[i];
    const Eigen::Vector3f sum = f.a + f.b + f.c;
    for (int axis = 0; axis < 3; ++axis) {
      filters->values[axis * filters->stride + i] = f.n(axis);
      filters->values[(axis + 3) * filters->stride + i] = sum(axis);
    }
  }
}

const MeshLoamPipeline::Facet* MeshLoamPipeline::FacetBin::nearest(
    const Eigen::Vector3d& relative, const Eigen::Vector3d& normal,
    double cos_gate, double* best, Eigen::Vector3d* best_relative) const {
  if (!filters || facets.empty()) return nullptr;
  const Eigen::Vector3f normal_f = normal.cast<float>();
  const Eigen::Vector3f relative_f = relative.cast<float>();
  const float cos_gate_f = static_cast<float>(cos_gate) - 1e-5f;
  float bound = static_cast<float>(std::sqrt(*best)) + 1e-5f;
  float bound2 = bound * bound;
  float centroid_bound = bound + max_rad;
  float centroid_bound2 = centroid_bound * centroid_bound;
  const float* v = filters->values.data();
  const size_t stride = filters->stride;
  // Scale the additional broad-filter slack for non-KITTI search-bin sizes.
  // The scalar tests below, with their original fixed slack, remain decisive.
  const float scale = std::max(1.0f, relative_f.cwiseAbs().maxCoeff() +
      lo.cwiseAbs().cwiseMax(hi.cwiseAbs()).maxCoeff() + max_rad);
  const float extra_slack = 1e-5f + 32.0f * std::numeric_limits<float>::epsilon() * scale;
  const Facet* found = nullptr;
  for (size_t first = 0; first < facets.size(); first += 4) {
    using Packet = Eigen::Array4f;
    const Eigen::Map<const Packet> nx(v + first), ny(v + stride + first),
        nz(v + 2 * stride + first);
    const Eigen::Array<bool, 4, 1> normal_match =
        (nx * normal_f.x() + ny * normal_f.y() + nz * normal_f.z()).abs() >=
        static_cast<float>(cos_gate) - 2e-5f;
    // Nearby facets usually share an orientation. Avoid loading centroids or
    // computing distance bounds when every lane fails the cheap normal gate.
    if (!normal_match.any()) continue;
    const Eigen::Map<const Packet> sx(v + 3 * stride + first),
        sy(v + 4 * stride + first), sz(v + 5 * stride + first);
    const Packet dx = relative_f.x() - sx * (1.0f / 3.0f);
    const Packet dy = relative_f.y() - sy * (1.0f / 3.0f);
    const Packet dz = relative_f.z() - sz * (1.0f / 3.0f);
    const Packet plane = nx * dx + ny * dy + nz * dz;
    // Recheck survivors with the original arithmetic and the latest bound;
    // packet rounding never decides an association or changes the tie order.
    const float broad = bound + extra_slack;
    const float broad_centroid = broad + max_rad;
    const Eigen::Array<bool, 4, 1> possible = normal_match &&
        (plane.square() <= broad * broad) &&
        (dx.square() + dy.square() + dz.square() <= broad_centroid * broad_centroid);
    for (size_t lane = 0; lane < 4 && first + lane < facets.size(); ++lane) {
      if (!possible(static_cast<int>(lane))) continue;
      const size_t i = first + lane;
      const Facet& f = facets[i];
      if (std::abs(f.n.dot(normal_f)) < cos_gate_f) continue;
      const Eigen::Vector3f sum(v[3 * stride + i], v[4 * stride + i], v[5 * stride + i]);
      const Eigen::Vector3f d0f = relative_f - sum * (1.0f / 3.0f);
      const float scalar_plane = f.n.dot(d0f);
      if (scalar_plane * scalar_plane > bound2 || d0f.squaredNorm() > centroid_bound2) continue;
      const Eigen::Vector3d fn = f.n.cast<double>();
      if (std::abs(fn.dot(normal)) < cos_gate) continue;
      const double distance = pointTriangleDist2(relative, f.a.cast<double>(), f.b.cast<double>(),
                                                f.c.cast<double>());
      // Original facet order and strict comparison retain the legacy tie rule.
      if (distance < *best) {
        *best = distance;
        found = &f;
        *best_relative = relative - sum.cast<double>() / 3.0;
        bound = static_cast<float>(std::sqrt(*best)) + 1e-5f;
        bound2 = bound * bound;
        centroid_bound = bound + max_rad;
        centroid_bound2 = centroid_bound * centroid_bound;
      }
    }
  }
  return found;
}

MeshLoamPipeline::MeshLoamPipeline(const MeshLoamParams& params) : params_(params) {
  params_.block_size = std::max(1, params_.block_size);
  params_.mesh_update_interval = std::max(1, params_.mesh_update_interval);
}

void MeshLoamPipeline::reset() {
  frame_count_ = 0;
  pose_ = Eigen::Matrix4d::Identity();
  prev_pose_ = Eigen::Matrix4d::Identity();
  has_prev_ = false;
  timings_ = MeshLoamTimings();
  blocks_.clear();
  dirty_blocks_.clear();
  num_voxels_ = 0;
  facet_bins_.clear();
  num_facets_ = 0;
  frozen_.clear();
  num_frozen_facets_ = 0;
}

Eigen::Vector3i MeshLoamPipeline::toVoxel(const Eigen::Vector3d& p) const {
  const double r = params_.voxel_size;
  return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / r)),
                         static_cast<int>(std::floor(p.y() / r)),
                         static_cast<int>(std::floor(p.z() / r)));
}

Eigen::Vector3i MeshLoamPipeline::toBlock(const Eigen::Vector3i& voxel) const {
  const int b = params_.block_size;
  return Eigen::Vector3i(floorDiv(voxel.x(), b), floorDiv(voxel.y(), b), floorDiv(voxel.z(), b));
}

Eigen::Vector3i MeshLoamPipeline::toFacetCell(const Eigen::Vector3d& p) const {
  const double c = params_.search_radius;
  return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / c)),
                         static_cast<int>(std::floor(p.y() / c)),
                         static_cast<int>(std::floor(p.z() / c)));
}

Eigen::Vector3d MeshLoamPipeline::voxelCenter(const Eigen::Vector3i& v) const {
  const double r = params_.voxel_size;
  return Eigen::Vector3d((v.x() + 0.5) * r, (v.y() + 0.5) * r, (v.z() + 0.5) * r);
}

Eigen::Vector3d MeshLoamPipeline::binOrigin(const Eigen::Vector3i& bin) const {
  return bin.cast<double>() * params_.search_radius;
}

Eigen::Vector3i MeshLoamPipeline::binBaseVoxel(const Eigen::Vector3i& bin) const {
  const double ratio = params_.search_radius / params_.voxel_size;
  return Eigen::Vector3i(static_cast<int>(std::floor(bin.x() * ratio + 1e-6)),
                         static_cast<int>(std::floor(bin.y() * ratio + 1e-6)),
                         static_cast<int>(std::floor(bin.z() * ratio + 1e-6)));
}

Eigen::Vector3i MeshLoamPipeline::toFreezeChunk(const Eigen::Vector3i& bin) const {
  constexpr int kChunk = 32;
  return Eigen::Vector3i(floorDiv(bin.x(), kChunk), floorDiv(bin.y(), kChunk),
                         floorDiv(bin.z(), kChunk));
}

size_t MeshLoamPipeline::memoryBytes() const {
  const size_t b3 = static_cast<size_t>(params_.block_size) * params_.block_size *
                    params_.block_size;
  constexpr size_t kNode = 48;  // hash node + bucket, approximate
  size_t bytes = blocks_.size() * (kNode + sizeof(VoxelBlock) + b3 * sizeof(Voxel) +
                                   ((b3 + 63) / 64) * 8);
  bytes += facet_bins_.bucket_count() * sizeof(void*);
  for (const auto& entry : facet_bins_) {
    bytes += kNode + sizeof(FacetBin) + entry.second.facets.capacity() * sizeof(Facet);
    bytes += (entry.second.filters ? sizeof(FacetBin::Filters) +
                 entry.second.filters->values.capacity() * sizeof(float) : 0);
  }
  for (const auto& entry : frozen_) {
    const FrozenChunk& c = entry.second;
    bytes += kNode + sizeof(FrozenChunk) + c.bins.capacity() * sizeof(Eigen::Vector3i) +
             c.offsets.capacity() * sizeof(uint32_t) + c.facets.capacity() * sizeof(FrozenFacet);
  }
  return bytes;
}

std::vector<MeshLoamPipeline::OrientedPoint> MeshLoamPipeline::preprocess(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<OrientedPoint> out;
  std::vector<Eigen::Vector3d> kept;
  kept.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    kept.push_back(p);
  }
  if (kept.size() < static_cast<size_t>(params_.normal_neighbors + 1)) return out;

  // Neighbor grid for kNN PCA normals: points sorted by cell into one flat
  // array (index order within a cell).
  const double cell = params_.normal_search_radius;
  auto cellOf = [cell](const Eigen::Vector3d& p) {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / cell)),
                           static_cast<int>(std::floor(p.y() / cell)),
                           static_cast<int>(std::floor(p.z() / cell)));
  };
  KeyMap<uint32_t> cell_id;
  cell_id.reserve(kept.size());
  std::vector<Eigen::Vector3i> cells;
  std::vector<uint32_t> point_cell(kept.size());
  for (size_t i = 0; i < kept.size(); ++i) {
    auto [it, inserted] = cell_id.try_emplace(cellOf(kept[i]), static_cast<uint32_t>(cells.size()));
    if (inserted) cells.push_back(it->first);
    point_cell[i] = it->second;
  }
  std::vector<uint32_t> cell_start(cells.size() + 1, 0);
  for (const uint32_t c : point_cell) ++cell_start[c + 1];
  for (size_t c = 0; c < cells.size(); ++c) cell_start[c + 1] += cell_start[c];
  std::vector<Eigen::Vector3d> flat_p(kept.size());
  std::vector<uint32_t> flat_i(kept.size());
  {
    std::vector<uint32_t> fill(cell_start.begin(), cell_start.end() - 1);
    for (size_t i = 0; i < kept.size(); ++i) {
      const uint32_t slot = fill[point_cell[i]]++;
      flat_p[slot] = kept[i];
      flat_i[slot] = static_cast<uint32_t>(i);
    }
  }

  // Exact k nearest neighbors among the 27 cells around the point's cell
  // (ties broken by index): cells are visited nearest first and the search
  // stops once the next cell is farther than the current k-th neighbor.
  const int k = std::max(5, params_.normal_neighbors);
  out.resize(kept.size());
#pragma omp parallel
  {
    std::vector<std::pair<double, size_t>> nn;
    std::array<std::pair<double, int>, 27> order;
    std::array<int, 27> neighbor;
#pragma omp for schedule(dynamic, 16)
    for (long ci = 0; ci < static_cast<long>(cells.size()); ++ci) {
      const Eigen::Vector3i& c = cells[static_cast<size_t>(ci)];
      int m = 0;
      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          for (int dz = -1; dz <= 1; ++dz) {
            const auto it = cell_id.find(Eigen::Vector3i(c.x() + dx, c.y() + dy, c.z() + dz));
            neighbor[m++] = it == cell_id.end() ? -1 : static_cast<int>(it->second);
          }
        }
      }
      const Eigen::Vector3d lo = c.cast<double>() * cell;
      for (uint32_t slot = cell_start[ci]; slot < cell_start[ci + 1]; ++slot) {
        const size_t i = flat_i[slot];
        const Eigen::Vector3d& p = kept[i];
        // Squared distance from p to each neighbor cell box.
        int n_order = 0;
        for (int q = 0; q < 27; ++q) {
          if (neighbor[q] < 0) continue;
          const int d[3] = {q / 9 - 1, (q / 3) % 3 - 1, q % 3 - 1};
          double d2 = 0.0;
          for (int ax = 0; ax < 3; ++ax) {
            const double t = p(ax) - lo(ax);
            if (d[ax] < 0) d2 += t * t;
            else if (d[ax] > 0) d2 += (cell - t) * (cell - t);
          }
          order[n_order++] = {d2, neighbor[q]};
        }
        std::sort(order.begin(), order.begin() + n_order);
        nn.clear();
        for (int o = 0; o < n_order; ++o) {
          if (static_cast<int>(nn.size()) == k && order[o].first > nn.front().first) break;
          const uint32_t cb = cell_start[order[o].second], ce = cell_start[order[o].second + 1];
          for (uint32_t m2 = cb; m2 < ce; ++m2) {
            const size_t j = flat_i[m2];
            if (j == i) continue;
            const std::pair<double, size_t> cand((flat_p[m2] - p).squaredNorm(), j);
            if (static_cast<int>(nn.size()) < k) {
              nn.push_back(cand);
              std::push_heap(nn.begin(), nn.end());
            } else if (cand < nn.front()) {
              std::pop_heap(nn.begin(), nn.end());
              nn.back() = cand;
              std::push_heap(nn.begin(), nn.end());
            }
          }
        }
        std::sort_heap(nn.begin(), nn.end());
        OrientedPoint op;
        op.p = p;
        if (static_cast<int>(nn.size()) >= k) {
          Eigen::Vector3d mean = p;
          for (int j = 0; j < k; ++j) mean += kept[nn[static_cast<size_t>(j)].second];
          mean /= static_cast<double>(k + 1);
          Eigen::Matrix3d cov = (p - mean) * (p - mean).transpose();
          for (int j = 0; j < k; ++j) {
            const Eigen::Vector3d d = kept[nn[static_cast<size_t>(j)].second] - mean;
            cov += d * d.transpose();
          }
          cov /= static_cast<double>(k + 1);
          Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
          const Eigen::Vector3d evals = es.eigenvalues();
          Eigen::Vector3d n = es.eigenvectors().col(0);
          const double sum = evals.sum();
          op.curvature = sum > 1e-12 ? evals(0) / sum : 1.0;
          // Orient toward the sensor (origin in the local frame) so that the SDF
          // is positive on the observed free-space side.
          if (n.dot(-p) < 0.0) n = -n;
          op.n = n;
          op.valid = true;
        }
        out[i] = op;
      }
    }
  }
  return out;
}

void MeshLoamPipeline::integrateScan(const std::vector<OrientedPoint>& points_world) {
  const int r = params_.influence_radius;
  const int b = params_.block_size;
  const int b3 = b * b * b;
  const int words = (b3 + 63) / 64;
  // Blocks a (2r+1)^3 neighborhood can touch per axis.
  const int per_axis = (2 * r + b - 1) / b + 1;
  const int max_blocks = per_axis * per_axis * per_axis;
  const size_t n = points_world.size();

  // Serial pass: allocate every block the scan touches, so the parallel pass
  // below never modifies the hash map.
  std::vector<VoxelBlock*> ptrs(n * static_cast<size_t>(max_blocks), nullptr);
  std::vector<uint8_t> counts(n, 0);
  Eigen::Vector3i last_key(INT_MIN, INT_MIN, INT_MIN);
  VoxelBlock* last_block = nullptr;
  for (size_t i = 0; i < n; ++i) {
    const OrientedPoint& op = points_world[i];
    if (!op.valid) continue;
    const Eigen::Vector3i c = toVoxel(op.p);
    const Eigen::Vector3i lo = toBlock((c.array() - r).matrix());
    const Eigen::Vector3i hi = toBlock((c.array() + r).matrix());
    int m = 0;
    for (int bz = lo.z(); bz <= hi.z(); ++bz) {
      for (int by = lo.y(); by <= hi.y(); ++by) {
        for (int bx = lo.x(); bx <= hi.x(); ++bx) {
          const Eigen::Vector3i key(bx, by, bz);
          VoxelBlock* blk;
          if (key == last_key) {
            blk = last_block;
          } else {
            auto [it, inserted] = blocks_.try_emplace(key);
            blk = &it->second;
            if (inserted) {
              blk->voxels.resize(static_cast<size_t>(b3));
              blk->dirty.assign(static_cast<size_t>(words), 0);
              blk->tag = mixKey(key);
              blk->base[0] = bx * b;
              blk->base[1] = by * b;
              blk->base[2] = bz * b;
            }
            last_key = key;
            last_block = blk;
          }
          if (!blk->queued) {
            blk->queued = true;
            dirty_blocks_.push_back(key);
          }
          ptrs[i * max_blocks + m++] = blk;
        }
      }
    }
    counts[i] = static_cast<uint8_t>(m);
  }

  // Parallel pass: each thread owns the blocks whose tag maps to it and
  // applies the updates of every point to them in scan order, so each voxel
  // sees exactly the serial update sequence (deterministic for any thread
  // count).
  const int threads = maxThreads();
  std::vector<size_t> added(static_cast<size_t>(threads), 0);
  const double inv_h = 1.0 / params_.imls_h;
  const double lambda = params_.lambda_n;
  const double vs = params_.voxel_size;
#pragma omp parallel num_threads(threads)
  {
    const int t = threadId();
    size_t added_local = 0;
    for (size_t i = 0; i < n; ++i) {
      const int m = counts[i];
      if (m == 0) continue;
      const OrientedPoint& op = points_world[i];
      const Eigen::Vector3i c = toVoxel(op.p);
      for (int j = 0; j < m; ++j) {
        VoxelBlock* blk = ptrs[i * max_blocks + j];
        if (static_cast<int>(blk->tag % static_cast<uint32_t>(threads)) != t) continue;
        const Eigen::Vector3i base(blk->base[0], blk->base[1], blk->base[2]);
        const int x0 = std::max(c.x() - r, base.x()), x1 = std::min(c.x() + r, base.x() + b - 1);
        const int y0 = std::max(c.y() - r, base.y()), y1 = std::min(c.y() + r, base.y() + b - 1);
        const int z0 = std::max(c.z() - r, base.z()), z1 = std::min(c.z() + r, base.z() + b - 1);
        for (int z = z0; z <= z1; ++z) {
          for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
              const int li = (x - base.x()) + b * ((y - base.y()) + b * (z - base.z()));
              Voxel& vx = blk->voxels[static_cast<size_t>(li)];
              const Eigen::Vector3d center((x + 0.5) * vs, (y + 0.5) * vs, (z + 0.5) * vs);
              const Eigen::Vector3d diff = center - op.p;
              double w = std::exp(-diff.squaredNorm() * inv_h);
              const double weight = vx.weight;
              const Eigen::Vector3d vn(vx.n[0], vx.n[1], vx.n[2]);
              // Hybrid weight (Eq. 8); the dot is clamped at zero because the
              // paper leaves negative normal agreement unspecified and
              // negative weights destabilize the running average (Eq. 9).
              if (weight > 0.0) {
                w += lambda * std::max(0.0, op.n.dot(vn));
              } else {
                w += lambda;
                ++added_local;
                ++blk->observed;
              }
              const double d = op.n.dot(diff);
              const double sdf = vx.sdf;
              vx.sdf = static_cast<float>(sdf + w * (d - sdf) / (weight + w));
              const Eigen::Vector3d n_new = weight * vn + w * op.n;
              const double len = n_new.norm();
              const Eigen::Vector3d nn = len > 1e-12 ? (n_new / len).eval() : op.n;
              vx.n[0] = static_cast<float>(nn.x());
              vx.n[1] = static_cast<float>(nn.y());
              vx.n[2] = static_cast<float>(nn.z());
              vx.weight = static_cast<float>(weight + w);
              blk->dirty[static_cast<size_t>(li >> 6)] |= uint64_t{1} << (li & 63);
            }
          }
        }
      }
    }
    added[static_cast<size_t>(t)] = added_local;
  }
  for (const size_t a : added) num_voxels_ += a;
}

void MeshLoamPipeline::extractMesh() {
  const int b = params_.block_size;
  const int b1 = b + 1;
  const int b3 = b * b * b;
  const int words = (b3 + 63) / 64;
  const double vs = params_.voxel_size;
  const float min_w = static_cast<float>(params_.min_voxel_weight);
  const int threads = maxThreads();

  if (!frozen_.empty()) {
    // A re-observed frozen area must be searchable again before its cells
    // are replaced (normally freezeAndThaw already restored it).
    std::vector<Eigen::Vector3i> chunks;
    for (const auto& key : dirty_blocks_) {
      const Eigen::Vector3i lo =
          (toFreezeChunk(toFacetCell(voxelCenter(key * b))).array() - 1).matrix();
      for (int dz = 0; dz < 3; ++dz) {
        for (int dy = 0; dy < 3; ++dy) {
          for (int dx = 0; dx < 3; ++dx) {
            const Eigen::Vector3i ch(lo.x() + dx, lo.y() + dy, lo.z() + dz);
            if (frozen_.count(ch)) chunks.push_back(ch);
          }
        }
      }
    }
    for (const auto& ch : chunks) thawChunk(ch);
  }

  // 1. Cells to re-extract, as per-block bit masks over the cells whose
  //    origin (lowest corner) voxel lies in the block.
  std::vector<Eigen::Vector3i> order;
  std::vector<std::vector<uint64_t>> masks;
  if (!params_.mesh_dirty_cells) {
    for (const auto& key : dirty_blocks_) {
      if (!blocks_.count(key)) continue;
      order.push_back(key);
      std::vector<uint64_t> all(static_cast<size_t>(words), ~uint64_t{0});
      if (b3 % 64) all.back() = (uint64_t{1} << (b3 % 64)) - 1;
      masks.push_back(std::move(all));
    }
  } else {
    // A cell is dirty when any of its 8 corners was updated; corners at the
    // upper faces live in the +x/+y/+z neighbors, so the lower neighbors of
    // every dirty block are candidates too.
    KeyMap<char> seen;
    seen.reserve(dirty_blocks_.size() * 4);
    std::vector<Eigen::Vector3i> candidates;
    for (const auto& key : dirty_blocks_) {
      for (int d = 0; d < 8; ++d) {
        const Eigen::Vector3i c(key.x() - (d & 1), key.y() - ((d >> 1) & 1),
                                key.z() - ((d >> 2) & 1));
        if (seen.emplace(c, 1).second) candidates.push_back(c);
      }
    }
    std::vector<std::vector<uint64_t>> cand_masks(candidates.size());
#pragma omp parallel num_threads(threads)
    {
      std::vector<uint8_t> dv(static_cast<size_t>(b1 * b1 * b1));
#pragma omp for schedule(dynamic, 64)
      for (long ci = 0; ci < static_cast<long>(candidates.size()); ++ci) {
        const Eigen::Vector3i& key = candidates[static_cast<size_t>(ci)];
        const VoxelBlock* nb[8];
        bool any = false;
        for (int d = 0; d < 8; ++d) {
          const auto it = blocks_.find(Eigen::Vector3i(key.x() + (d & 1), key.y() + ((d >> 1) & 1),
                                                       key.z() + ((d >> 2) & 1)));
          nb[d] = (it != blocks_.end() && it->second.queued) ? &it->second : nullptr;
          any |= nb[d] != nullptr;
        }
        if (!any) continue;
        for (int z = 0; z < b1; ++z) {
          for (int y = 0; y < b1; ++y) {
            for (int x = 0; x < b1; ++x) {
              const int d = (x == b ? 1 : 0) | (y == b ? 2 : 0) | (z == b ? 4 : 0);
              uint8_t flag = 0;
              if (nb[d] != nullptr) {
                const int li = (x % b) + b * ((y % b) + b * (z % b));
                flag = (nb[d]->dirty[static_cast<size_t>(li >> 6)] >> (li & 63)) & 1;
              }
              dv[static_cast<size_t>(x + b1 * (y + b1 * z))] = flag;
            }
          }
        }
        std::vector<uint64_t> mask(static_cast<size_t>(words), 0);
        bool nonempty = false;
        for (int z = 0; z < b; ++z) {
          for (int y = 0; y < b; ++y) {
            for (int x = 0; x < b; ++x) {
              uint8_t f = 0;
              for (int k = 0; k < 8 && !f; ++k) {
                f = dv[static_cast<size_t>((x + kCorner[k][0]) +
                                           b1 * ((y + kCorner[k][1]) + b1 * (z + kCorner[k][2])))];
              }
              if (f) {
                const int li = x + b * (y + b * z);
                mask[static_cast<size_t>(li >> 6)] |= uint64_t{1} << (li & 63);
                nonempty = true;
              }
            }
          }
        }
        if (nonempty) cand_masks[static_cast<size_t>(ci)] = std::move(mask);
      }
    }
    for (size_t ci = 0; ci < candidates.size(); ++ci) {
      if (cand_masks[ci].empty()) continue;
      order.push_back(candidates[ci]);
      masks.push_back(std::move(cand_masks[ci]));
    }
  }
  KeyMap<size_t> mask_index;
  mask_index.reserve(order.size());
  for (size_t i = 0; i < order.size(); ++i) mask_index.emplace(order[i], i);

  // 2. Bins that can hold facets of the dirty cells (those overlapping the
  //    cell boxes; a facet's centroid lies in its cell), listed per block in
  //    parallel and merged in block order. They are created up front so that
  //    the update below runs in parallel without touching the hash map.
  std::vector<std::vector<Eigen::Vector3i>> block_bins(order.size());
#pragma omp parallel num_threads(threads)
  {
    std::vector<std::pair<Eigen::Vector3i, Eigen::Vector3i>> ranges;
    std::vector<char> local;
    const double eps = 1e-6;
#pragma omp for schedule(dynamic, 16)
    for (long oi = 0; oi < static_cast<long>(order.size()); ++oi) {
      const size_t i = static_cast<size_t>(oi);
      const Eigen::Vector3i base = order[i] * b;
      Eigen::Vector3i lo_bin(INT_MAX, INT_MAX, INT_MAX), hi_bin(INT_MIN, INT_MIN, INT_MIN);
      ranges.clear();
      for (int li = 0; li < b3; ++li) {
        if (!((masks[i][static_cast<size_t>(li >> 6)] >> (li & 63)) & 1)) continue;
        const Eigen::Vector3i cell(base.x() + li % b, base.y() + (li / b) % b, base.z() + li / (b * b));
        const Eigen::Vector3d lo = (cell.cast<double>().array() + 0.5).matrix() * vs;
        const Eigen::Vector3d hi = (cell.cast<double>().array() + 1.5).matrix() * vs;
        const Eigen::Vector3i blo = toFacetCell((lo.array() - eps).matrix());
        const Eigen::Vector3i bhi = toFacetCell((hi.array() + eps).matrix());
        ranges.emplace_back(blo, bhi);
        lo_bin = lo_bin.cwiseMin(blo);
        hi_bin = hi_bin.cwiseMax(bhi);
      }
      const Eigen::Vector3i ext = hi_bin - lo_bin + Eigen::Vector3i::Ones();
      local.assign(static_cast<size_t>(ext.prod()), 0);
      for (const auto& rg : ranges) {
        for (int z = rg.first.z(); z <= rg.second.z(); ++z) {
          for (int y = rg.first.y(); y <= rg.second.y(); ++y) {
            for (int x = rg.first.x(); x <= rg.second.x(); ++x) {
              char& f = local[static_cast<size_t>((x - lo_bin.x()) +
                                                  ext.x() * ((y - lo_bin.y()) +
                                                             ext.y() * (z - lo_bin.z())))];
              if (f) continue;
              f = 1;
              block_bins[i].emplace_back(x, y, z);
            }
          }
        }
      }
    }
  }
  KeyMap<uint32_t> bin_index;
  std::vector<FacetBin*> bin_ptr;
  std::vector<Eigen::Vector3i> bin_keys;
  {
    size_t total = 0;
    for (const auto& list : block_bins) total += list.size();
    bin_index.reserve(total);
    bin_ptr.reserve(total);
    bin_keys.reserve(total);
    for (const auto& list : block_bins) {
      for (const auto& key : list) {
        if (!bin_index.emplace(key, static_cast<uint32_t>(bin_ptr.size())).second) continue;
        bin_ptr.push_back(&facet_bins_[key]);
        bin_keys.push_back(key);
      }
    }
    block_bins.clear();
  }

  // 3. March the dirty cells of each block (parallel).
  struct NewFacet {
    uint32_t bin;
    Facet f;
  };
  std::vector<std::vector<NewFacet>> produced(order.size());
  long cells_done = 0;
#pragma omp parallel num_threads(threads) reduction(+ : cells_done)
  {
    std::vector<float> grid_sdf(static_cast<size_t>(b1 * b1 * b1));
    std::vector<uint8_t> grid_ok(static_cast<size_t>(b1 * b1 * b1));
#pragma omp for schedule(dynamic, 16)
    for (long oi = 0; oi < static_cast<long>(order.size()); ++oi) {
      const Eigen::Vector3i& key = order[static_cast<size_t>(oi)];
      const std::vector<uint64_t>& mask = masks[static_cast<size_t>(oi)];
      const VoxelBlock* nb[8];
      for (int d = 0; d < 8; ++d) {
        const auto it = blocks_.find(Eigen::Vector3i(key.x() + (d & 1), key.y() + ((d >> 1) & 1),
                                                     key.z() + ((d >> 2) & 1)));
        nb[d] = it != blocks_.end() ? &it->second : nullptr;
      }
      if (nb[0] == nullptr) continue;  // origin voxels unobserved: no facets
      for (int z = 0; z < b1; ++z) {
        for (int y = 0; y < b1; ++y) {
          for (int x = 0; x < b1; ++x) {
            const int d = (x == b ? 1 : 0) | (y == b ? 2 : 0) | (z == b ? 4 : 0);
            const size_t gi = static_cast<size_t>(x + b1 * (y + b1 * z));
            grid_ok[gi] = 0;
            if (nb[d] == nullptr) continue;
            const Voxel& v =
                nb[d]->voxels[static_cast<size_t>((x % b) + b * ((y % b) + b * (z % b)))];
            if (v.weight < min_w) continue;
            grid_ok[gi] = 1;
            grid_sdf[gi] = v.sdf;
          }
        }
      }
      const Eigen::Vector3i base = key * b;
      std::vector<NewFacet>& out = produced[static_cast<size_t>(oi)];
      Eigen::Vector3i cell;
      Eigen::Vector3i last_bin(INT_MIN, INT_MIN, INT_MIN);
      uint32_t last_index = 0;
      auto emit = [&](const Eigen::Vector3d& a, const Eigen::Vector3d& bb, const Eigen::Vector3d& cc,
                      const Eigen::Vector3d& n) {
        // Winding consistent with n (frozen facets recover n from it).
        const bool swap = n.dot((bb - a).cross(cc - a)) < 0.0;
        const Eigen::Vector3d& v1 = swap ? cc : bb;
        const Eigen::Vector3d& v2 = swap ? bb : cc;
        const Eigen::Vector3i bin = toFacetCell((a + v1 + v2) / 3.0);
        const Eigen::Vector3d o = binOrigin(bin);
        NewFacet nf;
        nf.f.a = (a - o).cast<float>();
        nf.f.b = (v1 - o).cast<float>();
        nf.f.c = (v2 - o).cast<float>();
        // Normal of the stored (float) triangle, so that plane-distance
        // bounds in the search hold exactly.
        Eigen::Vector3d nn = (nf.f.b - nf.f.a).cast<double>().cross((nf.f.c - nf.f.a).cast<double>());
        const double len = nn.norm();
        if (len < 1e-12 || nn.dot(n) <= 0.0) return;
        nf.f.n = (nn / len).cast<float>();
        const Eigen::Vector3i off = cell - binBaseVoxel(bin);
        nf.f.owner[0] = static_cast<int8_t>(off.x());
        nf.f.owner[1] = static_cast<int8_t>(off.y());
        nf.f.owner[2] = static_cast<int8_t>(off.z());
        if (bin != last_bin) {
          // Always listed: the centroid lies in the cell box.
          last_bin = bin;
          last_index = bin_index.at(bin);
        }
        nf.bin = last_index;
        out.push_back(nf);
      };
      double sdf[8];
      Eigen::Vector3d pos[8];
      for (int li = 0; li < b3; ++li) {
        if (!((mask[static_cast<size_t>(li >> 6)] >> (li & 63)) & 1)) continue;
        ++cells_done;
        const int x = li % b, y = (li / b) % b, z = li / (b * b);
        bool ok = true;
        for (int k = 0; k < 8 && ok; ++k) {
          const size_t gi = static_cast<size_t>((x + kCorner[k][0]) +
                                                b1 * ((y + kCorner[k][1]) + b1 * (z + kCorner[k][2])));
          ok = grid_ok[gi] != 0;
          sdf[k] = grid_sdf[gi];
        }
        if (!ok) continue;
        cell = Eigen::Vector3i(base.x() + x, base.y() + y, base.z() + z);
        for (int k = 0; k < 8; ++k) {
          pos[k] = voxelCenter(Eigen::Vector3i(cell.x() + kCorner[k][0], cell.y() + kCorner[k][1],
                                               cell.z() + kCorner[k][2]));
        }
        if (params_.marching_cubes) {
          marchCube(sdf, pos, emit);
        } else {
          marchTetrahedra(sdf, pos, emit);
        }
      }
    }
  }
  timings_.extracted_cells += cells_done;

  // New facets bucketed per bin, in block order (deterministic).
  std::vector<uint32_t> bucket_start(bin_ptr.size() + 1, 0);
  for (const auto& list : produced) {
    for (const NewFacet& nf : list) ++bucket_start[nf.bin + 1];
  }
  for (size_t j = 0; j < bin_ptr.size(); ++j) bucket_start[j + 1] += bucket_start[j];
  std::vector<Facet> bucketed(bucket_start.back());
  {
    std::vector<uint32_t> fill(bucket_start.begin(), bucket_start.end() - 1);
    for (const auto& list : produced) {
      for (const NewFacet& nf : list) bucketed[fill[nf.bin]++] = nf.f;
    }
    produced.clear();
  }

  // 4. Per bin (parallel): drop the facets owned by re-extracted cells,
  //    append the new ones.
  std::vector<long> delta(static_cast<size_t>(threads), 0);
#pragma omp parallel num_threads(threads)
  {
    long delta_local = 0;
#pragma omp for schedule(dynamic, 256)
    for (long bj = 0; bj < static_cast<long>(bin_ptr.size()); ++bj) {
      const size_t j = static_cast<size_t>(bj);
      FacetBin& fb = *bin_ptr[j];
      std::vector<Facet>& bin = fb.facets;
      const size_t before = bin.size();
      const uint32_t s0 = bucket_start[j], s1 = bucket_start[j + 1];
      if (before == 0 && s0 == s1) continue;
      if (before > 0) {
        const Eigen::Vector3i base = binBaseVoxel(bin_keys[j]);
        Eigen::Vector3i last_block(INT_MIN, INT_MIN, INT_MIN);
        const std::vector<uint64_t>* last_mask = nullptr;
        bin.erase(std::remove_if(bin.begin(), bin.end(),
                                 [&](const Facet& f) {
                                   const Eigen::Vector3i cell(base.x() + f.owner[0],
                                                              base.y() + f.owner[1],
                                                              base.z() + f.owner[2]);
                                   const Eigen::Vector3i blk = toBlock(cell);
                                   if (blk != last_block) {
                                     last_block = blk;
                                     const auto mi = mask_index.find(blk);
                                     last_mask = mi == mask_index.end() ? nullptr : &masks[mi->second];
                                   }
                                   if (last_mask == nullptr) return false;
                                   const Eigen::Vector3i l = cell - blk * b;
                                   const int li = l.x() + b * (l.y() + b * l.z());
                                   return (((*last_mask)[static_cast<size_t>(li >> 6)] >> (li & 63)) & 1) != 0;
                                 }),
                  bin.end());
      }
      const bool removed_any = bin.size() != before;
      // Exact capacity (no doubling slack): most bins are rebuilt at every
      // extraction anyway, and kept-mesh bins stay as they are.
      const size_t needed = bin.size() + (s1 - s0);
      if (needed > bin.capacity()) {
        bin.reserve(needed);
      } else if (bin.capacity() > needed + needed / 4 + 4) {
        std::vector<Facet> tight;
        tight.reserve(needed);
        tight.assign(bin.begin(), bin.end());
        bin.swap(tight);
      }
      bin.insert(bin.end(), bucketed.begin() + s0, bucketed.begin() + s1);
      if (removed_any) {
        fb.recomputeBounds();
      } else {
        for (uint32_t m = s0; m < s1; ++m) fb.expand(bucketed[m]);
      }
      if (params_.accelerated && params_.nearest_facet) fb.buildFilters();
      delta_local += static_cast<long>(bin.size()) - static_cast<long>(before);
    }
    delta[static_cast<size_t>(threadId())] += delta_local;
  }
  for (const long d : delta) num_facets_ = static_cast<size_t>(static_cast<long>(num_facets_) + d);
  for (size_t j = 0; j < bin_ptr.size(); ++j) {
    if (bin_ptr[j]->facets.empty()) facet_bins_.erase(bin_keys[j]);
  }

  // 5. Clear the dirty state.
  for (const auto& key : dirty_blocks_) {
    const auto it = blocks_.find(key);
    if (it == blocks_.end()) continue;
    std::fill(it->second.dirty.begin(), it->second.dirty.end(), 0);
    it->second.queued = false;
  }
  dirty_blocks_.clear();
}

void MeshLoamPipeline::remeshAll() {
  const int b3 = params_.block_size * params_.block_size * params_.block_size;
  for (auto& entry : blocks_) {
    VoxelBlock& blk = entry.second;
    for (int li = 0; li < b3; ++li) {
      if (blk.voxels[static_cast<size_t>(li)].weight > 0.0f) {
        blk.dirty[static_cast<size_t>(li >> 6)] |= uint64_t{1} << (li & 63);
      }
    }
    if (!blk.queued) {
      blk.queued = true;
      dirty_blocks_.push_back(entry.first);
    }
  }
  extractMesh();
}

void MeshLoamPipeline::pruneMap() {
  const Eigen::Vector3d center = pose_.block<3, 1>(0, 3);
  const double R = params_.local_map_radius;
  const double r2 = R * R;
  const int b = params_.block_size;
  const double vs = params_.voxel_size;

  // Voxels: drop whole blocks outside the radius, reset single voxels on
  // the boundary (same per-voxel-center criterion as before).
  for (auto it = blocks_.begin(); it != blocks_.end();) {
    VoxelBlock& blk = it->second;
    const Eigen::Vector3d lo = ((it->first * b).cast<double>().array() + 0.5).matrix() * vs;
    const Eigen::Vector3d hi = lo + Eigen::Vector3d::Constant((b - 1) * vs);
    const Eigen::Vector3d nearest = center.cwiseMax(lo).cwiseMin(hi);
    const Eigen::Vector3d far = (center - lo).cwiseAbs().cwiseMax((center - hi).cwiseAbs());
    if ((nearest - center).squaredNorm() > r2) {
      num_voxels_ -= static_cast<size_t>(blk.observed);
      it = blocks_.erase(it);
      continue;
    }
    if (far.squaredNorm() > r2) {
      const Eigen::Vector3i base = it->first * b;
      for (int li = 0; li < b * b * b; ++li) {
        Voxel& v = blk.voxels[static_cast<size_t>(li)];
        if (!(v.weight > 0.0f)) continue;
        const Eigen::Vector3i key(base.x() + li % b, base.y() + (li / b) % b, base.z() + li / (b * b));
        if ((voxelCenter(key) - center).squaredNorm() > r2) {
          v = Voxel();
          --blk.observed;
          --num_voxels_;
        }
      }
      if (blk.observed == 0 && !blk.queued) {
        it = blocks_.erase(it);
        continue;
      }
    }
    ++it;
  }

  if (params_.keep_mesh) {
    if (params_.mesh_freeze_radius > 0.0) freezeAndThaw();
    return;
  }
  // Facets: drop those whose owning block's center is outside the radius.
  const double s = params_.search_radius;
  const double block_span = b * vs;
  const double margin = std::sqrt(3.0) * (s + 3.0 * vs + block_span);
  for (auto it = facet_bins_.begin(); it != facet_bins_.end();) {
    const Eigen::Vector3d bc = binOrigin(it->first) + Eigen::Vector3d::Constant(0.5 * s);
    const double dc = (bc - center).norm();
    if (dc + margin < R) {
      ++it;
      continue;
    }
    std::vector<Facet>& bin = it->second.facets;
    if (dc - margin > R) {
      num_facets_ -= bin.size();
      it = facet_bins_.erase(it);
      continue;
    }
    const Eigen::Vector3i base = binBaseVoxel(it->first);
    const size_t before = bin.size();
    bin.erase(std::remove_if(bin.begin(), bin.end(),
                             [&](const Facet& f) {
                               const Eigen::Vector3i cell(base.x() + f.owner[0], base.y() + f.owner[1],
                                                          base.z() + f.owner[2]);
                               const Eigen::Vector3d block_center =
                                   (toBlock(cell).cast<double>() + Eigen::Vector3d::Constant(0.5)) *
                                   block_span;
                               return (block_center - center).squaredNorm() > r2;
                             }),
              bin.end());
    num_facets_ -= before - bin.size();
    if (bin.empty()) {
      it = facet_bins_.erase(it);
    } else {
      if (bin.size() != before) {
        it->second.recomputeBounds();
        if (params_.accelerated && params_.nearest_facet) it->second.buildFilters();
      }
      ++it;
    }
  }
}

void MeshLoamPipeline::thawChunk(const Eigen::Vector3i& chunk) {
  const auto it = frozen_.find(chunk);
  if (it == frozen_.end()) return;
  const float q = static_cast<float>(frozenQuantum());
  const FrozenChunk& c = it->second;
  for (size_t i = 0; i < c.bins.size(); ++i) {
    FacetBin& bin = facet_bins_[c.bins[i]];
    bin.facets.reserve(bin.facets.size() + (c.offsets[i + 1] - c.offsets[i]));
    for (uint32_t j = c.offsets[i]; j < c.offsets[i + 1]; ++j) {
      const FrozenFacet& ff = c.facets[j];
      Facet f;
      f.a = Eigen::Vector3f(ff.v[0], ff.v[1], ff.v[2]) * q;
      f.b = Eigen::Vector3f(ff.v[3], ff.v[4], ff.v[5]) * q;
      f.c = Eigen::Vector3f(ff.v[6], ff.v[7], ff.v[8]) * q;
      const Eigen::Vector3d nn = (f.b - f.a).cast<double>().cross((f.c - f.a).cast<double>());
      const double len = nn.norm();
      --num_frozen_facets_;
      if (len < 1e-12) continue;  // collapsed by quantization
      f.n = (nn / len).cast<float>();
      std::copy(ff.owner, ff.owner + 3, f.owner);
      bin.add(f);
      ++num_facets_;
    }
    if (bin.facets.empty()) facet_bins_.erase(c.bins[i]);
    else if (params_.accelerated && params_.nearest_facet) bin.buildFilters();
  }
  frozen_.erase(it);
}

void MeshLoamPipeline::freezeAndThaw() {
  const double Rf = params_.mesh_freeze_radius;
  if (!(Rf > 0.0)) return;
  const Eigen::Vector3d center = pose_.block<3, 1>(0, 3);
  const double s = params_.search_radius;
  constexpr int kChunk = 32;
  const double chunk_span = kChunk * s;
  const double chunk_diag = std::sqrt(3.0) * chunk_span;
  const double q = frozenQuantum();

  // Thaw chunks that come within Rf.
  for (auto it = frozen_.begin(); it != frozen_.end();) {
    const Eigen::Vector3d lo = it->first.cast<double>() * chunk_span;
    const Eigen::Vector3d hi = lo + Eigen::Vector3d::Constant(chunk_span);
    const Eigen::Vector3d nearest = center.cwiseMax(lo).cwiseMin(hi);
    if ((nearest - center).norm() >= Rf) {
      ++it;
      continue;
    }
    const Eigen::Vector3i key = it->first;
    ++it;
    thawChunk(key);
  }

  // Freeze bins beyond Rf plus a chunk diagonal (hysteresis: a thawed chunk
  // is not frozen again right away).
  const double freeze_r = Rf + chunk_diag;
  std::vector<Eigen::Vector3i> to_freeze;
  for (const auto& entry : facet_bins_) {
    const Eigen::Vector3d bc = binOrigin(entry.first) + Eigen::Vector3d::Constant(0.5 * s);
    if ((bc - center).norm() > freeze_r) to_freeze.push_back(entry.first);
  }
  std::sort(to_freeze.begin(), to_freeze.end(), [](const Eigen::Vector3i& a, const Eigen::Vector3i& b) {
    return std::lexicographical_compare(a.data(), a.data() + 3, b.data(), b.data() + 3);
  });
  for (const auto& key : to_freeze) {
    auto it = facet_bins_.find(key);
    FrozenChunk& c = frozen_[toFreezeChunk(key)];
    if (c.offsets.empty()) c.offsets.push_back(0);
    c.bins.push_back(key);
    for (const Facet& f : it->second.facets) {
      FrozenFacet ff;
      const Eigen::Vector3f* v[3] = {&f.a, &f.b, &f.c};
      for (int k = 0; k < 3; ++k) {
        for (int m = 0; m < 3; ++m) {
          const double val = std::round((*v[k])(m) / q);
          ff.v[3 * k + m] = static_cast<int16_t>(std::clamp(val, -32767.0, 32767.0));
        }
      }
      std::copy(f.owner, f.owner + 3, ff.owner);
      c.facets.push_back(ff);
    }
    c.offsets.push_back(static_cast<uint32_t>(c.facets.size()));
    num_facets_ -= it->second.facets.size();
    num_frozen_facets_ += it->second.facets.size();
    facet_bins_.erase(it);
  }
  for (auto& entry : frozen_) {
    entry.second.facets.shrink_to_fit();
    entry.second.bins.shrink_to_fit();
    entry.second.offsets.shrink_to_fit();
  }
}

bool MeshLoamPipeline::solveIcp(const std::vector<OrientedPoint>& features_world,
                                Eigen::Matrix4d* icp_correction,
                                MeshLoamResult* result) const {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  const double max_dist = params_.max_correspondence_dist;
  const double s = params_.search_radius;
  const double search2 = s * s;
  // Search bins nearest first (center, faces, edges, corners) so that the
  // nearest-facet bound tightens early.
  static const std::array<int, 27> kVisitOrder = [] {
    std::array<int, 27> order{};
    for (int k = 0; k < 27; ++k) order[k] = k;
    std::stable_sort(order.begin(), order.end(), [](int a, int b) {
      auto l1 = [](int k) { return std::abs(k / 9 - 1) + std::abs((k / 3) % 3 - 1) + std::abs(k % 3 - 1); };
      return l1(a) < l1(b);
    });
    return order;
  }();
  const double cos_gate = params_.cos_gate;
  const float cos_gate_f = static_cast<float>(cos_gate) - 1e-5f;  // float prefilter
  const bool nearest = params_.nearest_facet;
  int used = 0;
  double abs_residual = 0.0;

  // Per-feature cache of the 27 search bins (valid while the feature stays
  // in the same bin; the map is not modified during registration).
  const size_t nfeat = features_world.size();
  std::vector<Eigen::Vector3i> cache_cell(nfeat, Eigen::Vector3i(INT_MIN, INT_MIN, INT_MIN));
  std::vector<std::array<const FacetBin*, 27>> cache_bins(nfeat);

  const int threads = maxThreads();
  int iter = 0;
  for (; iter < params_.max_iterations; ++iter) {
    Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> g = Eigen::Matrix<double, 6, 1>::Zero();
    used = 0;
    abs_residual = 0.0;
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);

    // Per-thread sums over static chunks, added in thread order so that the
    // result does not depend on scheduling.
    std::vector<Eigen::Matrix<double, 6, 6>> H_part(threads, Eigen::Matrix<double, 6, 6>::Zero());
    std::vector<Eigen::Matrix<double, 6, 1>> g_part(threads, Eigen::Matrix<double, 6, 1>::Zero());
    std::vector<int> used_part(threads, 0);
    std::vector<double> abs_part(threads, 0.0);
#pragma omp parallel num_threads(threads)
    {
      const int tid = threadId();
      Eigen::Matrix<double, 6, 6>& H_local = H_part[tid];
      Eigen::Matrix<double, 6, 1>& g_local = g_part[tid];
      int& used_local = used_part[tid];
      double& abs_local = abs_part[tid];
#pragma omp for schedule(static)
      for (long fi = 0; fi < static_cast<long>(nfeat); ++fi) {
        const size_t i = static_cast<size_t>(fi);
        const auto& op = features_world[i];
        const Eigen::Vector3d p = R * op.p + t;
        const Eigen::Vector3d n_pt = R * op.n;
        const Eigen::Vector3i cell = toFacetCell(p);
        std::array<const FacetBin*, 27>& bins = cache_bins[i];
        if (cell != cache_cell[i]) {
          cache_cell[i] = cell;
          int k = 0;
          for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
              for (int dz = -1; dz <= 1; ++dz) {
                const auto it =
                    facet_bins_.find(Eigen::Vector3i(cell.x() + dx, cell.y() + dy, cell.z() + dz));
                bins[k++] = it == facet_bins_.end() ? nullptr : &it->second;
              }
            }
          }
        }

        const Eigen::Vector3f n_ptf = n_pt.cast<float>();
        double best = nearest ? search2 : max_dist;
        const Facet* best_facet = nullptr;
        Eigen::Vector3d best_rel = Eigen::Vector3d::Zero();
        for (const int k : kVisitOrder) {
          const FacetBin* bin = bins[k];
          if (bin == nullptr) continue;
          const Eigen::Vector3i key(cell.x() + k / 9 - 1, cell.y() + (k / 3) % 3 - 1,
                                    cell.z() + k % 3 - 1);
          const Eigen::Vector3d rel = p - binOrigin(key);
          if (!nearest) {
            for (const Facet& f : bin->facets) {
              const Eigen::Vector3d fn = f.n.cast<double>();
              if (std::abs(fn.dot(n_pt)) < cos_gate) continue;
              const Eigen::Vector3d d0 = rel - (f.a + f.b + f.c).cast<double>() / 3.0;
              const double dist = std::abs(fn.dot(d0));
              if (dist < best) {
                best = dist;
                best_facet = &f;
                best_rel = d0;
              }
            }
            continue;
          }
          // Every triangle of the bin lies in its vertex AABB.
          const Eigen::Vector3d gap = (bin->lo.cast<double>() - rel)
                                          .cwiseMax(rel - bin->hi.cast<double>())
                                          .cwiseMax(0.0);
          if (gap.squaredNorm() >= best) continue;
          if (params_.accelerated && bin->filters) {
            const Facet* found = bin->nearest(rel, n_pt, cos_gate, &best, &best_rel);
            if (found) best_facet = found;
            continue;
          }
          // Lower bounds of the point-triangle distance evaluated in float
          // with a margin above the float rounding (|rel| < 1 m), then the
          // exact gate and distance in double for the survivors.
          constexpr float kSlack = 1e-5f;
          const Eigen::Vector3f relf = rel.cast<float>();
          float plane_bound = static_cast<float>(std::sqrt(best)) + kSlack;
          float centroid_bound = plane_bound + bin->max_rad;
          float plane_bound2 = plane_bound * plane_bound;
          float centroid_bound2 = centroid_bound * centroid_bound;
          for (const Facet& f : bin->facets) {
            if (std::abs(f.n.dot(n_ptf)) < cos_gate_f) continue;
            const Eigen::Vector3f d0f = relf - (f.a + f.b + f.c) * (1.0f / 3.0f);
            const float plane = f.n.dot(d0f);
            if (plane * plane > plane_bound2) continue;
            if (d0f.squaredNorm() > centroid_bound2) continue;
            const Eigen::Vector3d fn = f.n.cast<double>();
            if (std::abs(fn.dot(n_pt)) < cos_gate) continue;
            const double dist = pointTriangleDist2(rel, f.a.cast<double>(), f.b.cast<double>(),
                                                   f.c.cast<double>());
            if (dist < best) {
              best = dist;
              best_facet = &f;
              best_rel = rel - (f.a + f.b + f.c).cast<double>() / 3.0;
              plane_bound = static_cast<float>(std::sqrt(best)) + kSlack;
              centroid_bound = plane_bound + bin->max_rad;
              plane_bound2 = plane_bound * plane_bound;
              centroid_bound2 = centroid_bound * centroid_bound;
            }
          }
        }
        if (best_facet == nullptr) continue;

        const Eigen::Vector3d fn = best_facet->n.cast<double>();
        const double r_i = fn.dot(best_rel);
        Eigen::Matrix<double, 6, 1> J;
        J.head<3>() = fn;
        J.tail<3>() = p.cross(fn);
        H_local += J * J.transpose();
        g_local += J * r_i;
        ++used_local;
        abs_local += std::abs(r_i);
      }
    }
    for (int k = 0; k < threads; ++k) {
      H += H_part[k];
      g += g_part[k];
      used += used_part[k];
      abs_residual += abs_part[k];
    }

    if (used < params_.min_correspondences) break;

    const Eigen::Matrix<double, 6, 1> delta = H.ldlt().solve(-g);
    if (!delta.allFinite()) break;

    Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
    step.block<3, 3>(0, 0) = expSo3(delta.tail<3>());
    step.block<3, 1>(0, 3) = delta.head<3>();
    T = step * T;

    if (delta.norm() < params_.convergence_eps) {
      ++iter;
      break;
    }
  }

  result->iterations = iter;
  result->num_correspondences = used;
  result->mean_abs_residual = used > 0 ? abs_residual / used : 0.0;
  if (used < params_.min_correspondences) return false;
  *icp_correction = T;
  return true;
}

MeshLoamResult MeshLoamPipeline::registerFrame(const std::vector<Eigen::Vector3d>& points) {
  MeshLoamResult result;
  ++timings_.frames;
  auto t0 = Clock::now();
  const auto oriented = preprocess(points);
  timings_.preprocess_ms += msSince(t0);

  if (frame_count_ == 0) {
    std::vector<OrientedPoint> world = oriented;  // pose is identity
    t0 = Clock::now();
    integrateScan(world);
    timings_.integrate_ms += msSince(t0);
    t0 = Clock::now();
    extractMesh();
    timings_.extract_ms += msSince(t0);
    ++timings_.extractions;
    frame_count_ = 1;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  // Constant-velocity prediction.
  Eigen::Matrix4d prediction = pose_;
  if (has_prev_) {
    prediction = pose_ * (prev_pose_.inverse() * pose_);
  }

  // Planar features in the predicted world frame.
  std::vector<OrientedPoint> features;
  features.reserve(oriented.size());
  const Eigen::Matrix3d Rp = prediction.block<3, 3>(0, 0);
  const Eigen::Vector3d tp = prediction.block<3, 1>(0, 3);
  for (const auto& op : oriented) {
    if (!op.valid || op.curvature >= params_.curvature_threshold) continue;
    OrientedPoint w = op;
    w.p = Rp * op.p + tp;
    w.n = Rp * op.n;
    features.push_back(w);
  }

  Eigen::Matrix4d icp = Eigen::Matrix4d::Identity();
  t0 = Clock::now();
  result.converged = solveIcp(features, &icp, &result);
  timings_.icp_ms += msSince(t0);
  const Eigen::Matrix4d new_pose = result.converged ? (icp * prediction).eval() : prediction;

  prev_pose_ = pose_;
  pose_ = new_pose;
  has_prev_ = true;

  // Integrate the full oriented scan at the refined pose.
  std::vector<OrientedPoint> world;
  world.reserve(oriented.size());
  const Eigen::Matrix3d R = pose_.block<3, 3>(0, 0);
  const Eigen::Vector3d t = pose_.block<3, 1>(0, 3);
  for (const auto& op : oriented) {
    if (!op.valid) continue;
    OrientedPoint w = op;
    w.p = R * op.p + t;
    w.n = R * op.n;
    world.push_back(w);
  }
  t0 = Clock::now();
  integrateScan(world);
  timings_.integrate_ms += msSince(t0);

  ++frame_count_;
  if (num_facets_ == 0 || frame_count_ % params_.mesh_update_interval == 0) {
    t0 = Clock::now();
    extractMesh();
    timings_.extract_ms += msSince(t0);
    ++timings_.extractions;
    t0 = Clock::now();
    pruneMap();
    timings_.prune_ms += msSince(t0);
  }

  result.pose = pose_;
  return result;
}

}  // namespace mesh_loam
}  // namespace localization_zoo
