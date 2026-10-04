#include "l_lo/l_lo.h"

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace localization_zoo {
namespace l_lo {

namespace {

double cross(const Eigen::Vector2d& o, const Eigen::Vector2d& a, const Eigen::Vector2d& b) {
  return (a.x() - o.x()) * (b.y() - o.y()) - (a.y() - o.y()) * (b.x() - o.x());
}

Polygon transformPolygon(const Polygon& polygon, const Eigen::Vector3d& delta) {
  const double c = std::cos(delta.z()), s = std::sin(delta.z());
  Polygon out;
  out.reserve(polygon.size());
  for (const auto& p : polygon) {
    out.emplace_back(c * p.x() - s * p.y() + delta.x(), s * p.x() + c * p.y() + delta.y());
  }
  return out;
}

Eigen::Vector3d fitPlaneNormal(const std::vector<Eigen::Vector3d>& points) {
  if (points.size() < 10) return Eigen::Vector3d::UnitZ();
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  for (const auto& p : points) mean += p;
  mean /= static_cast<double>(points.size());
  Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
  for (const auto& p : points) cov += (p - mean) * (p - mean).transpose();
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eig(cov);
  Eigen::Vector3d n = eig.eigenvectors().col(0);
  if (n.z() < 0) n = -n;
  return n;
}

struct CellHash {
  size_t operator()(const Eigen::Vector3i& k) const {
    return (static_cast<size_t>(k.x()) * 73856093u) ^ (static_cast<size_t>(k.y()) * 19349663u) ^
           (static_cast<size_t>(k.z()) * 83492791u);
  }
};
struct CellEq {
  bool operator()(const Eigen::Vector3i& a, const Eigen::Vector3i& b) const { return a == b; }
};

}  // namespace

// ------------------------------------------------------------ geometry

Polygon convexHull(std::vector<Eigen::Vector2d> points) {
  std::sort(points.begin(), points.end(), [](const auto& a, const auto& b) {
    return a.x() < b.x() || (a.x() == b.x() && a.y() < b.y());
  });
  points.erase(std::unique(points.begin(), points.end(),
                           [](const auto& a, const auto& b) { return (a - b).norm() < 1e-9; }),
               points.end());
  if (points.size() < 3) return points;
  Polygon hull(2 * points.size());
  size_t k = 0;
  for (size_t i = 0; i < points.size(); ++i) {
    while (k >= 2 && cross(hull[k - 2], hull[k - 1], points[i]) <= 0) --k;
    hull[k++] = points[i];
  }
  for (size_t i = points.size() - 1, t = k + 1; i > 0; --i) {
    while (k >= t && cross(hull[k - 2], hull[k - 1], points[i - 1]) <= 0) --k;
    hull[k++] = points[i - 1];
  }
  hull.resize(k - 1);
  return hull;
}

double polygonArea(const Polygon& polygon) {
  // Shoelace formula, eq. (10).
  double area = 0.0;
  for (size_t i = 0; i < polygon.size(); ++i) {
    const auto& a = polygon[i];
    const auto& b = polygon[(i + 1) % polygon.size()];
    area += a.x() * b.y() - b.x() * a.y();
  }
  return 0.5 * std::abs(area);
}

Polygon convexIntersection(const Polygon& subject, const Polygon& clip) {
  if (subject.size() < 3 || clip.size() < 3) return {};
  Polygon output = subject;
  for (size_t i = 0; i < clip.size() && !output.empty(); ++i) {
    const Eigen::Vector2d& a = clip[i];
    const Eigen::Vector2d& b = clip[(i + 1) % clip.size()];
    Polygon input = output;
    output.clear();
    for (size_t j = 0; j < input.size(); ++j) {
      const Eigen::Vector2d& p = input[j];
      const Eigen::Vector2d& q = input[(j + 1) % input.size()];
      const double cp = cross(a, b, p), cq = cross(a, b, q);
      if (cp >= 0) output.push_back(p);
      if ((cp >= 0) != (cq >= 0)) {
        const double t = cp / (cp - cq);
        output.push_back(p + t * (q - p));
      }
    }
  }
  return output;
}

double turningFunctionArea(const Polygon& polygon) {
  const size_t n = polygon.size();
  if (n < 3) return 0.0;
  size_t start = 0;
  for (size_t i = 1; i < n; ++i) {
    if (polygon[i].norm() < polygon[start].norm()) start = i;
  }
  double perimeter = 0.0;
  for (size_t i = 0; i < n; ++i) perimeter += (polygon[(i + 1) % n] - polygon[i]).norm();
  if (!(perimeter > 0.0)) return 0.0;
  // Step function: on each side the value is the cumulative exterior angle
  // turned so far (starting with the exterior angle at the reference vertex).
  double area = 0.0, cumulative = 0.0;
  for (size_t k = 0; k < n; ++k) {
    const size_t i = (start + k) % n;
    const Eigen::Vector2d in = polygon[i] - polygon[(i + n - 1) % n];
    const Eigen::Vector2d out = polygon[(i + 1) % n] - polygon[i];
    cumulative += std::atan2(in.x() * out.y() - in.y() * out.x(), in.dot(out));
    area += cumulative * out.norm() / perimeter;
  }
  return area;
}

double hausdorffDistance(const Polygon& a, const Polygon& b) {
  auto directed = [](const Polygon& from, const Polygon& to) {
    double worst = 0.0;
    for (const auto& p : from) {
      double best = std::numeric_limits<double>::infinity();
      for (const auto& q : to) best = std::min(best, (p - q).norm());
      worst = std::max(worst, best);
    }
    return worst;
  };
  if (a.empty() || b.empty()) return std::numeric_limits<double>::infinity();
  return std::max(directed(a, b), directed(b, a));  // eqs. (6)-(8)
}

double hullSimilarity(const Polygon& a, const Polygon& b, double alpha, double beta) {
  return alpha * std::abs(turningFunctionArea(a) - turningFunctionArea(b)) +
         beta * hausdorffDistance(a, b);
}

// ------------------------------------------------------------ odometry

LLOOdometry::LLOOdometry(const LLOParams& params) : params_(params) {}

void LLOOdometry::splitGround(const std::vector<Eigen::Vector3d>& scan,
                              std::vector<Eigen::Vector3d>* ground,
                              std::vector<Eigen::Vector3d>* non_ground) const {
  // Line-fit ground segmentation in the spirit of [8]: per azimuth sector,
  // take the lowest point of each range bin and grow a ground line from the
  // sensor outward while the slope stays small.
  const int sectors = static_cast<int>(std::ceil(360.0 / params_.ground_sector_deg));
  const int bins = static_cast<int>(std::ceil(params_.max_range / params_.ground_bin_m));
  std::vector<double> lowest(static_cast<size_t>(sectors) * bins,
                             std::numeric_limits<double>::infinity());
  auto index = [&](const Eigen::Vector3d& p, int* s, int* b) {
    const double r = std::hypot(p.x(), p.y());
    double az = std::atan2(p.y(), p.x()) * 180.0 / M_PI;
    if (az < 0) az += 360.0;
    *s = std::min(sectors - 1, static_cast<int>(az / params_.ground_sector_deg));
    *b = std::min(bins - 1, static_cast<int>(r / params_.ground_bin_m));
  };
  for (const auto& p : scan) {
    const double r = p.head<2>().norm();
    if (r < params_.min_range || r > params_.max_range) continue;
    int s, b;
    index(p, &s, &b);
    double& z = lowest[static_cast<size_t>(s) * bins + b];
    z = std::min(z, p.z());
  }
  // Ground height per (sector, bin) following the local line.
  std::vector<double> ground_z(lowest.size(), std::numeric_limits<double>::quiet_NaN());
  for (int s = 0; s < sectors; ++s) {
    double prev_z = -params_.sensor_height, prev_r = 0.0;
    for (int b = 0; b < bins; ++b) {
      const double z = lowest[static_cast<size_t>(s) * bins + b];
      const double r = (b + 0.5) * params_.ground_bin_m;
      if (!std::isfinite(z)) continue;
      const double slope = (z - prev_z) / std::max(1e-3, r - prev_r);
      if (std::abs(slope) <= params_.ground_max_slope &&
          std::abs(z - prev_z) <= params_.ground_tolerance + params_.ground_max_slope * (r - prev_r)) {
        ground_z[static_cast<size_t>(s) * bins + b] = z;
        prev_z = z;
        prev_r = r;
      }
    }
  }
  for (const auto& p : scan) {
    const double r = p.head<2>().norm();
    if (r < params_.min_range || r > params_.max_range) continue;
    int s, b;
    index(p, &s, &b);
    const double gz = ground_z[static_cast<size_t>(s) * bins + b];
    if (std::isfinite(gz) && p.z() - gz < params_.ground_tolerance) {
      ground->push_back(p);
    } else {
      non_ground->push_back(p);
    }
  }
}

std::vector<Cluster> LLOOdometry::cluster(const std::vector<Eigen::Vector3d>& non_ground) const {
  // Voxel downsample, then Euclidean clustering as connected components of
  // occupied cells (26-neighbourhood).
  std::unordered_map<Eigen::Vector3i, std::vector<Eigen::Vector3d>, CellHash, CellEq> cells;
  std::unordered_map<Eigen::Vector3i, int, CellHash, CellEq> seen_voxel;
  for (const auto& p : non_ground) {
    const Eigen::Vector3i v = (p / params_.voxel_size).array().floor().cast<int>();
    if (!seen_voxel.emplace(v, 1).second) continue;
    const Eigen::Vector3i c = (p / params_.cluster_cell).array().floor().cast<int>();
    cells[c].push_back(p);
  }
  std::unordered_map<Eigen::Vector3i, int, CellHash, CellEq> label;
  std::vector<Cluster> clusters;
  for (const auto& [seed, pts] : cells) {
    if (label.count(seed)) continue;
    std::vector<Eigen::Vector3i> stack{seed};
    label[seed] = 1;
    Cluster cl;
    while (!stack.empty()) {
      const Eigen::Vector3i c = stack.back();
      stack.pop_back();
      const auto& members = cells[c];
      cl.points.insert(cl.points.end(), members.begin(), members.end());
      for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
          for (int dz = -1; dz <= 1; ++dz) {
            const Eigen::Vector3i n = c + Eigen::Vector3i(dx, dy, dz);
            if (cells.count(n) && !label.count(n)) {
              label[n] = 1;
              stack.push_back(n);
            }
          }
    }
    if (static_cast<int>(cl.points.size()) < params_.min_cluster_points ||
        static_cast<int>(cl.points.size()) > params_.max_cluster_points) {
      continue;
    }
    // Statistical outlier removal within the cluster, eqs. (1)-(2).
    const int k = std::min<int>(params_.sor_k, static_cast<int>(cl.points.size()) - 1);
    std::vector<double> avg(cl.points.size(), 0.0);
    for (size_t i = 0; i < cl.points.size(); ++i) {
      std::vector<double> d;
      d.reserve(cl.points.size());
      for (size_t j = 0; j < cl.points.size(); ++j) {
        if (i != j) d.push_back((cl.points[i] - cl.points[j]).norm());
      }
      std::nth_element(d.begin(), d.begin() + (k - 1), d.end());
      std::sort(d.begin(), d.begin() + k);
      for (int j = 0; j < k; ++j) avg[i] += d[j];
      avg[i] /= k;
    }
    double mean = 0.0, var = 0.0;
    for (double a : avg) mean += a;
    mean /= avg.size();
    for (double a : avg) var += (a - mean) * (a - mean);
    const double limit = mean + params_.sor_std_mul * std::sqrt(var / avg.size());
    std::vector<Eigen::Vector3d> kept;
    for (size_t i = 0; i < cl.points.size(); ++i) {
      if (avg[i] <= limit) kept.push_back(cl.points[i]);
    }
    if (static_cast<int>(kept.size()) < params_.min_cluster_points) continue;
    cl.points = std::move(kept);
    for (const auto& p : cl.points) cl.center += p;
    cl.center /= static_cast<double>(cl.points.size());  // eq. (3)
    cl.mean_height = cl.center.z();
    clusters.push_back(std::move(cl));
  }
  return clusters;
}

Eigen::Vector3d LLOOdometry::estimateHorizontal(const std::vector<Polygon>& previous,
                                                const std::vector<Polygon>& current,
                                                const Eigen::Vector3d& initial, int* rounds,
                                                double* overlap_ratio) const {
  double reference = 0.0;
  for (size_t i = 0; i < previous.size(); ++i) {
    reference += std::min(polygonArea(previous[i]), polygonArea(current[i]));
  }
  auto objective = [&](const Eigen::Vector3d& delta) {
    double g = 0.0;
    for (size_t i = 0; i < previous.size(); ++i) {
      g += polygonArea(convexIntersection(previous[i], transformPolygon(current[i], delta)));
    }
    return g;
  };
  Eigen::Vector3d delta = initial;
  double best = objective(delta);
  int round = 0;
  for (; round < params_.max_rounds; ++round) {
    const Eigen::Vector3d start = delta;
    for (int axis = 0; axis < 3; ++axis) {
      // Adaptive step, eq. (13): large while overlap is poor.
      const double coeff = axis < 2 ? params_.step_coeff_translation : params_.step_coeff_rotation;
      const double ratio = reference > 0.0 ? best / reference : 1.0;
      const double step = coeff * std::max(1.0, std::ceil(100.0 * (1.0 - ratio)));
      for (double direction : {1.0, -1.0}) {
        bool improved = true;
        while (improved) {
          Eigen::Vector3d trial = delta;
          trial(axis) += direction * step;
          const double value = objective(trial);
          improved = value > best + 1e-12;
          if (improved) {
            best = value;
            delta = trial;
          }
        }
      }
    }
    if ((delta - start).norm() < params_.termination) break;
  }
  if (rounds) *rounds = round + 1;
  if (overlap_ratio) *overlap_ratio = reference > 0.0 ? best / reference : 0.0;
  return delta;
}

LLOResult LLOOdometry::registerFrame(const std::vector<Eigen::Vector3d>& scan) {
  LLOResult result;
  std::vector<Eigen::Vector3d> ground, non_ground;
  splitGround(scan, &ground, &non_ground);
  auto clusters = cluster(non_ground);
  result.clusters = static_cast<int>(clusters.size());

  // Ground normal from the ground band around the vehicle (vertical pose).
  std::vector<Eigen::Vector3d> band;
  for (const auto& p : ground) {
    const double r = p.head<2>().norm();
    if (r >= params_.pitch_ground_min_range && r <= params_.pitch_ground_max_range) band.push_back(p);
  }
  const Eigen::Vector3d ground_normal = fitPlaneNormal(band);

  if (!initialized_) {
    initialized_ = true;
    previous_clusters_ = std::move(clusters);
    previous_ground_normal_ = ground_normal;
    result.pose = pose_;
    return result;
  }

  // Initial landmark matching on cluster centres (eq. 4) after applying the
  // constant-velocity prediction; accept matches closer than the mean of
  // all nearest-centre distances.
  const Eigen::Vector3d prediction = last_delta_;
  const double c = std::cos(prediction.z()), s = std::sin(prediction.z());
  auto toPrevious = [&](const Eigen::Vector3d& p) {
    return Eigen::Vector3d(c * p.x() - s * p.y() + prediction.x(),
                           s * p.x() + c * p.y() + prediction.y(), p.z());
  };
  std::vector<std::pair<int, int>> nearest;  // (previous, current)
  std::vector<double> distances;
  for (size_t r = 0; r < previous_clusters_.size(); ++r) {
    int best = -1;
    double best_d = std::numeric_limits<double>::infinity();
    for (size_t q = 0; q < clusters.size(); ++q) {
      const double d = (previous_clusters_[r].center.head<2>() - toPrevious(clusters[q].center).head<2>()).norm();
      if (d < best_d) {
        best_d = d;
        best = static_cast<int>(q);
      }
    }
    if (best >= 0) {
      nearest.emplace_back(static_cast<int>(r), best);
      distances.push_back(best_d);
    }
  }
  double mean_d = 0.0;
  for (double d : distances) mean_d += d;
  if (!distances.empty()) mean_d /= distances.size();

  // Layering at the shared height, hulls, and per-landmark layer selection.
  std::vector<Polygon> prev_hulls, curr_hulls;
  for (size_t m = 0; m < nearest.size(); ++m) {
    if (distances[m] > mean_d) continue;
    const Cluster& a = previous_clusters_[nearest[m].first];
    const Cluster& b = clusters[nearest[m].second];
    const double split = 0.5 * (a.mean_height + b.mean_height);
    std::vector<Eigen::Vector2d> a_up, a_low, b_up, b_low;
    for (const auto& p : a.points) (p.z() >= split ? a_up : a_low).push_back(p.head<2>());
    for (const auto& p : b.points) {
      const Eigen::Vector3d q = toPrevious(p);  // compare in a common frame
      (p.z() >= split ? b_up : b_low).push_back(q.head<2>());
    }
    const Polygon hulls[2][2] = {{convexHull(a_up), convexHull(b_up)},
                                 {convexHull(a_low), convexHull(b_low)}};
    int pick = -1;
    double pick_sim = std::numeric_limits<double>::infinity();
    for (int layer = 0; layer < 2; ++layer) {
      if (hulls[layer][0].size() < 3 || hulls[layer][1].size() < 3) continue;
      const double sim = hullSimilarity(hulls[layer][0], hulls[layer][1],
                                        params_.similarity_weight_shape, params_.similarity_weight_size);
      if (sim < pick_sim) {
        pick_sim = sim;
        pick = layer;
      }
    }
    if (pick < 0) continue;
    prev_hulls.push_back(hulls[pick][0]);
    // Hulls of frame k are kept in the predicted frame; express them back in
    // frame k so the search starts from the prediction.
    Polygon back;
    for (const auto& p : hulls[pick][1]) {
      const Eigen::Vector2d t = p - prediction.head<2>();
      back.emplace_back(c * t.x() + s * t.y(), -s * t.x() + c * t.y());
    }
    curr_hulls.push_back(back);
  }
  result.matches = static_cast<int>(prev_hulls.size());

  Eigen::Vector3d delta = prediction;
  if (result.matches >= params_.min_matches) {
    delta = estimateHorizontal(prev_hulls, curr_hulls, prediction, &result.rounds, &result.overlap_ratio);
  }

  // Vertical pose: roll assumed zero; pitch change from the change of the
  // fitted ground normal between frames (the paper's front/rear-plane text
  // read as consecutive frames, see README). z follows from composing the
  // pitched motion.
  const double pitch_k = std::atan2(ground_normal.x(), ground_normal.z());
  const double pitch_prev = std::atan2(previous_ground_normal_.x(), previous_ground_normal_.z());
  // R_y(dpitch) maps frame-k vectors into frame k-1, so the k-1 normal's tilt
  // equals the frame-k tilt plus dpitch.
  const double dpitch = pitch_prev - pitch_k;

  Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
  step.block<3, 3>(0, 0) = (Eigen::AngleAxisd(delta.z(), Eigen::Vector3d::UnitZ()) *
                            Eigen::AngleAxisd(dpitch, Eigen::Vector3d::UnitY()))
                               .toRotationMatrix();
  step(0, 3) = delta.x();
  step(1, 3) = delta.y();
  pose_ = pose_ * step;
  last_delta_ = delta;
  previous_clusters_ = std::move(clusters);
  previous_ground_normal_ = ground_normal;
  result.pose = pose_;
  return result;
}

}  // namespace l_lo
}  // namespace localization_zoo
