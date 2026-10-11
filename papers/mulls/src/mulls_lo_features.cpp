#include "mulls/mulls_lo.h"
#include "mulls_lo_internal.h"

#include <Eigen/Eigenvalues>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace localization_zoo {
namespace mulls {

namespace {

constexpr double kDegToRad = M_PI / 180.0;

struct Pca {
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  Eigen::Vector3d values = Eigen::Vector3d::Zero();  // descending
  Eigen::Vector3d principal = Eigen::Vector3d::UnitX();
  Eigen::Vector3d normal = Eigen::Vector3d::UnitZ();
};

Pca computePca(const LOCloud& cloud, const std::vector<int>& indices, size_t count) {
  Pca pca;
  for (size_t k = 0; k < count; ++k) pca.mean += cloud[indices[k]].p;
  pca.mean /= static_cast<double>(count);
  Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
  for (size_t k = 0; k < count; ++k) {
    const Eigen::Vector3d d = cloud[indices[k]].p - pca.mean;
    cov += d * d.transpose();
  }
  cov /= static_cast<double>(count);
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
  // Eigen sorts ascending.
  pca.values = Eigen::Vector3d(solver.eigenvalues()(2), solver.eigenvalues()(1),
                               solver.eigenvalues()(0));
  pca.principal = solver.eigenvectors().col(2);
  pca.normal = solver.eigenvectors().col(0);
  return pca;
}

// Plane through three points or the least-squares plane of a set: n.p + d = 0.
bool planeFromPoints(const LOCloud& cloud, const std::vector<int>& indices,
                     Eigen::Vector3d* normal, double* offset) {
  if (indices.size() < 3) return false;
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  for (int i : indices) mean += cloud[i].p;
  mean /= static_cast<double>(indices.size());
  Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
  for (int i : indices) {
    const Eigen::Vector3d d = cloud[i].p - mean;
    cov += d * d.transpose();
  }
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
  if (solver.eigenvalues()(1) < 1e-12) return false;  // degenerate (collinear)
  *normal = solver.eigenvectors().col(0);
  *offset = -normal->dot(mean);
  return true;
}

}  // namespace

std::vector<int> sampleIndices(size_t n, size_t k, std::mt19937* rng) {
  std::vector<int> out;
  if (k >= n) {
    out.resize(n);
    std::iota(out.begin(), out.end(), 0);
    return out;
  }
  // Selection sampling: keeps the input order.
  out.reserve(k);
  size_t needed = k;
  for (size_t i = 0; i < n && needed > 0; ++i) {
    std::uniform_real_distribution<double> u(0.0, 1.0);
    if (u(*rng) * static_cast<double>(n - i) < static_cast<double>(needed)) {
      out.push_back(static_cast<int>(i));
      --needed;
    }
  }
  return out;
}

LOCloud sampleCloud(const LOCloud& cloud, size_t k, std::mt19937* rng) {
  LOCloud out;
  for (int i : sampleIndices(cloud.size(), k, rng)) out.push_back(cloud[i]);
  return out;
}

PointTree::PointTree(const LOCloud& cloud) {
  pcl::PointCloud<pcl::PointXYZ>::Ptr pc(new pcl::PointCloud<pcl::PointXYZ>);
  pc->reserve(cloud.size());
  for (const auto& pt : cloud)
    pc->push_back(pcl::PointXYZ(static_cast<float>(pt.p.x()), static_cast<float>(pt.p.y()),
                                static_cast<float>(pt.p.z())));
  empty_ = pc->empty();
  if (!empty_) tree_.setInputCloud(pc);
}

int PointTree::radius(const Eigen::Vector3d& q, double r, std::vector<int>* idx,
                      std::vector<float>* d2) const {
  idx->clear();
  d2->clear();
  if (empty_) return 0;
  pcl::PointXYZ p(static_cast<float>(q.x()), static_cast<float>(q.y()), static_cast<float>(q.z()));
  return tree_.radiusSearch(p, r, *idx, *d2);  // sorted by distance
}

bool PointTree::nearest(const Eigen::Vector3d& q, int* index, double* dist) const {
  if (empty_) return false;
  std::vector<int> idx(1);
  std::vector<float> d2(1);
  pcl::PointXYZ p(static_cast<float>(q.x()), static_cast<float>(q.y()), static_cast<float>(q.z()));
  if (tree_.nearestKSearch(p, 1, idx, d2) < 1) return false;
  *index = idx[0];
  *dist = std::sqrt(static_cast<double>(d2[0]));
  return true;
}

LOCloud preprocessScan(const std::vector<Eigen::Vector4d>& xyzi, const MullsLOParams& params) {
  LOCloud out;
  out.reserve(xyzi.size());
  const double dv = params.vertical_angle_correction_deg * kDegToRad;
  for (const auto& v : xyzi) {
    Eigen::Vector3d p = v.head<3>();
    const double rho = std::hypot(p.x(), p.y());
    if (!(rho > params.min_horizontal_range && rho < params.max_horizontal_range)) continue;
    if (dv != 0.0) {
      // Rotate the point up by dv in its vertical plane, keeping the range.
      const double r = p.norm();
      const double elev = std::asin(std::clamp(p.z() / r, -1.0, 1.0));
      const double scale = std::cos(elev + dv) / std::cos(elev);
      p = Eigen::Vector3d(p.x() * scale, p.y() * scale, r * std::sin(elev + dv));
    }
    const double rho2 = std::hypot(p.x(), p.y());
    if (rho2 <= params.scanner_filter_radius) continue;
    if (p.z() <= -(params.scanner_height + params.underground_depth)) continue;
    if (rho2 < params.near_radius &&
        p.z() <= -(params.scanner_height + params.near_underground_depth))
      continue;
    LOPoint pt;
    pt.p = p;
    pt.intensity = v(3);
    out.push_back(pt);
  }
  return out;
}

void groundFilter(const LOCloud& cloud, const MullsLOParams& params, LOCloud* ground,
                  LOCloud* nonground) {
  ground->clear();
  nonground->clear();
  if (cloud.empty()) return;

  auto rate = [&](double distance, double base) {
    const double w = params.gf_unit_distance / std::max(distance, 1e-3);
    return std::max(1, static_cast<int>(std::floor(w * w * base + 1.0)));
  };

  double mean_z = 0.0;
  int sampled = 0;
  for (size_t i = 0; i < cloud.size(); i += 100) {
    mean_z += cloud[i].p.z();
    ++sampled;
  }
  mean_z /= std::max(sampled, 1);

  // Points well above the mean height cannot be ground.
  std::vector<int> low;
  int high_index = 0;
  for (size_t i = 0; i < cloud.size(); ++i) {
    const LOPoint& pt = cloud[i];
    if (pt.p.z() > mean_z + params.gf_high_above_mean) {
      if (high_index % rate(pt.p.norm(), params.gf_nonground_rate) == 0 ||
          pt.intensity > params.gf_bright_intensity)
        nonground->push_back(pt);
      ++high_index;
    } else {
      low.push_back(static_cast<int>(i));
    }
  }
  if (low.empty()) return;

  double min_x = 1e18, min_y = 1e18, max_x = -1e18, max_y = -1e18;
  for (int i : low) {
    min_x = std::min(min_x, cloud[i].p.x());
    min_y = std::min(min_y, cloud[i].p.y());
    max_x = std::max(max_x, cloud[i].p.x());
    max_y = std::max(max_y, cloud[i].p.y());
  }
  const int nx = static_cast<int>(std::floor((max_x - min_x) / params.gf_grid)) + 1;
  const int ny = static_cast<int>(std::floor((max_y - min_y) / params.gf_grid)) + 1;
  std::vector<std::vector<int>> cells(static_cast<size_t>(nx) * ny);
  for (int i : low) {
    const int cx = std::min(nx - 1, static_cast<int>((cloud[i].p.x() - min_x) / params.gf_grid));
    const int cy = std::min(ny - 1, static_cast<int>((cloud[i].p.y() - min_y) / params.gf_grid));
    cells[static_cast<size_t>(cx) * ny + cy].push_back(i);
  }

  // Per cell: drop points below mean - 3 sigma, then record the lowest height.
  std::vector<double> cell_min(cells.size(), 1e18);
  std::vector<double> cell_range(cells.size(), 0.0);
  for (size_t c = 0; c < cells.size(); ++c) {
    auto& pts = cells[c];
    if (pts.empty()) continue;
    cell_range[c] = cloud[pts.front()].p.norm();
    if (static_cast<int>(pts.size()) >= params.gf_min_points) {
      double m = 0.0, m2 = 0.0;
      for (int i : pts) {
        m += cloud[i].p.z();
        m2 += cloud[i].p.z() * cloud[i].p.z();
      }
      m /= pts.size();
      const double sigma = std::sqrt(std::max(0.0, m2 / pts.size() - m * m));
      const double floor_z = m - 3.0 * sigma;
      pts.erase(std::remove_if(pts.begin(), pts.end(),
                               [&](int i) { return cloud[i].p.z() < floor_z; }),
                pts.end());
    }
    if (static_cast<int>(pts.size()) < params.gf_min_points) {
      pts.clear();  // too sparse to judge: neither ground nor non-ground
      continue;
    }
    for (int i : pts) cell_min[c] = std::min(cell_min[c], cloud[i].p.z());
  }

  std::mt19937 plane_rng(params.seed);
  for (int cx = 0; cx < nx; ++cx) {
    for (int cy = 0; cy < ny; ++cy) {
      const size_t c = static_cast<size_t>(cx) * ny + cy;
      const auto& pts = cells[c];
      if (pts.empty()) continue;
      double neighbor_min = cell_min[c];
      if (cx > 0 && cy > 0 && cx < nx - 1 && cy < ny - 1) {
        for (int dx = -1; dx <= 1; ++dx)
          for (int dy = -1; dy <= 1; ++dy)
            neighbor_min =
                std::min(neighbor_min, cell_min[static_cast<size_t>(cx + dx) * ny + cy + dy]);
      }
      const bool ground_cell = cell_min[c] - neighbor_min < params.gf_neighbor_height;
      const int ground_rate = rate(cell_range[c], params.gf_ground_rate);
      const int nonground_rate = rate(cell_range[c], params.gf_nonground_rate);

      std::vector<int> candidates;
      int nonground_index = 0;
      for (int i : pts) {
        if (ground_cell && cloud[i].p.z() - cell_min[c] < params.gf_in_cell_height) {
          candidates.push_back(i);
        } else {
          if (nonground_index % nonground_rate == 0 ||
              cloud[i].intensity > params.gf_bright_intensity)
            nonground->push_back(cloud[i]);
          ++nonground_index;
        }
      }
      if (static_cast<int>(candidates.size()) < params.gf_min_points) continue;

      // RANSAC plane, refined by least squares on its inliers.
      std::vector<int> best;
      std::uniform_int_distribution<size_t> pick(0, candidates.size() - 1);
      for (int it = 0; it < params.gf_plane_iterations; ++it) {
        std::vector<int> triple{candidates[pick(plane_rng)], candidates[pick(plane_rng)],
                                candidates[pick(plane_rng)]};
        Eigen::Vector3d n;
        double d;
        if (!planeFromPoints(cloud, triple, &n, &d)) continue;
        std::vector<int> inliers;
        for (int i : candidates)
          if (std::abs(n.dot(cloud[i].p) + d) < params.gf_plane_threshold) inliers.push_back(i);
        if (inliers.size() > best.size()) best.swap(inliers);
      }
      Eigen::Vector3d n;
      double d;
      if (best.size() < 3 || !planeFromPoints(cloud, best, &n, &d)) continue;
      std::vector<int> inliers;
      for (int i : candidates)
        if (std::abs(n.dot(cloud[i].p) + d) < params.gf_plane_threshold) inliers.push_back(i);
      if (std::abs(n.z()) <= params.gf_min_normal_z) continue;
      if (n.z() < 0.0) n = -n;
      for (size_t k = 0; k < inliers.size(); ++k) {
        if (static_cast<int>(k) % ground_rate != 0) continue;
        LOPoint pt = cloud[inliers[k]];
        pt.dir = n;
        ground->push_back(pt);
      }
    }
  }
}

void classifyNonGround(const LOCloud& input, const MullsLOParams& params, std::mt19937* rng,
                       std::array<LOCloud, kLONumClasses>* dense) {
  for (int c : {kLOPillar, kLOFacade, kLOBeam}) (*dense)[c].clear();
  const LOCloud cloud = sampleCloud(input, params.nonground_cap, rng);
  if (cloud.empty()) return;
  const PointTree tree(cloud);
  const double sin_pillar = std::sin(params.pillar_min_deg * kDegToRad);
  const double sin_beam = std::sin(params.beam_max_deg * kDegToRad);
  const double sin_facade = std::sin(params.facade_max_deg * kDegToRad);

  struct Query {
    int index;
    std::vector<int> neighbors;
    Pca pca;
  };
  std::vector<Query> queries;
  std::vector<int> label(cloud.size(), -1);
  std::vector<double> saliency(cloud.size(), 0.0);
  std::vector<Eigen::Vector3d> dir(cloud.size(), Eigen::Vector3d::UnitZ());
  std::vector<int> idx;
  std::vector<float> d2;

  auto line_class = [&](const Eigen::Vector3d& e1, double z) {
    if (std::abs(e1.z()) > sin_pillar) return static_cast<int>(kLOPillar);
    if (std::abs(e1.z()) < sin_beam && z < params.beam_max_height) return static_cast<int>(kLOBeam);
    return -1;
  };

  for (size_t i = 0; i < cloud.size(); i += params.pca_stride) {
    const Eigen::Vector3d& p = cloud[i].p;
    const double range = p.norm();
    const double r = range > params.pca_radius_range
                         ? params.pca_radius * std::sqrt(range / params.pca_radius_range)
                         : params.pca_radius;
    tree.radius(p, r, &idx, &d2);
    const size_t count = std::min(idx.size(), static_cast<size_t>(params.pca_max_neighbors));
    if (static_cast<int>(count) < params.pca_min_neighbors) continue;
    Query q;
    q.index = static_cast<int>(i);
    q.neighbors.assign(idx.begin(), idx.begin() + count);
    q.pca = computePca(cloud, q.neighbors, count);
    const Eigen::Vector3d& l = q.pca.values;
    if (l(0) <= 0.0) continue;
    const double linearity = (l(0) - l(1)) / l(0);
    const double planarity = (l(1) - l(2)) / l(0);
    if (linearity > params.linearity) {
      const int c = line_class(q.pca.principal, p.z());
      if (c >= 0) {
        label[i] = c;
        saliency[i] = linearity;
        dir[i] = q.pca.principal;
      }
    } else if (planarity > params.planarity) {
      if (std::abs(q.pca.normal.z()) < sin_facade) {
        label[i] = kLOFacade;
        saliency[i] = planarity;
        dir[i] = q.pca.normal;
      }
    }
    queries.push_back(std::move(q));
  }

  // Corner-like points among labelled neighbours join the line classes.
  for (const auto& q : queries) {
    if (label[q.index] >= 0) continue;
    const Eigen::Vector3d& l = q.pca.values;
    const double curvature = l(2) / l.sum();
    if (curvature <= params.curvature) continue;
    int labelled = 0;
    for (int j : q.neighbors) labelled += label[j] >= 0 ? 1 : 0;
    if (static_cast<double>(labelled) / q.neighbors.size() <= params.vertex_labelled_share)
      continue;
    const int c = line_class(q.pca.principal, cloud[q.index].p.z());
    if (c < 0) continue;
    label[q.index] = c;
    saliency[q.index] = 5.0 * curvature;
    dir[q.index] = q.pca.principal;
  }

  for (size_t i = 0; i < cloud.size(); ++i) {
    if (label[i] < 0) continue;
    LOPoint pt = cloud[i];
    pt.dir = dir[i];
    pt.saliency = saliency[i];
    (*dense)[label[i]].push_back(pt);
  }
}

LOCloud saliencyNms(const LOCloud& cloud, double radius, int min_points) {
  LOCloud out;
  if (static_cast<int>(cloud.size()) < min_points) return out;
  std::vector<int> order(cloud.size());
  std::iota(order.begin(), order.end(), 0);
  std::stable_sort(order.begin(), order.end(),
                   [&](int a, int b) { return cloud[a].saliency > cloud[b].saliency; });
  const PointTree tree(cloud);
  std::vector<char> suppressed(cloud.size(), 0);
  std::vector<int> idx;
  std::vector<float> d2;
  for (int i : order) {
    if (suppressed[i]) continue;
    out.push_back(cloud[i]);
    tree.radius(cloud[i].p, radius, &idx, &d2);
    for (int j : idx) suppressed[j] = 1;
  }
  return out;
}

LOFeatures extractFeatures(const std::vector<Eigen::Vector4d>& xyzi, const MullsLOParams& params,
                           std::mt19937* rng) {
  LOFeatures f;
  for (const auto& v : xyzi) f.bound.extend(Eigen::Vector3d(v.head<3>()));
  const LOCloud cloud = preprocessScan(xyzi, params);
  LOCloud nonground;
  groundFilter(cloud, params, &f.dense[kLOGround], &nonground);
  classifyNonGround(nonground, params, rng, &f.dense);
  f.sparse[kLOGround] = sampleCloud(f.dense[kLOGround], params.sparse_ground, rng);

  const LOCloud pillars = saliencyNms(f.dense[kLOPillar], params.nms_radius, params.nms_min_points);
  f.sparse[kLOPillar] = sampleCloud(pillars, params.sparse_pillar, rng);

  // Facades: balance the normal directions over four azimuth sectors.
  const LOCloud facades = saliencyNms(f.dense[kLOFacade], params.nms_radius, params.nms_min_points);
  if (static_cast<int>(facades.size()) <= params.sparse_facade_per_sector) {
    f.sparse[kLOFacade] = facades;
  } else {
    std::array<LOCloud, 4> sectors;
    for (const auto& pt : facades) {
      double az = std::atan2(pt.dir.y(), pt.dir.x());
      if (az < 0.0) az += 2.0 * M_PI;
      sectors[std::min(3, static_cast<int>(az / (0.5 * M_PI)))].push_back(pt);
    }
    for (const auto& s : sectors) {
      const LOCloud kept = sampleCloud(s, params.sparse_facade_per_sector, rng);
      f.sparse[kLOFacade].insert(f.sparse[kLOFacade].end(), kept.begin(), kept.end());
    }
  }
  return f;
}

}  // namespace mulls
}  // namespace localization_zoo
