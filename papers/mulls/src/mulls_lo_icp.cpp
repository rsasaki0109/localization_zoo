#include "mulls/mulls_lo.h"
#include "mulls_lo_internal.h"

#include <Eigen/Cholesky>

#include <memory>

#include <cmath>

namespace localization_zoo {
namespace mulls {

namespace {

constexpr double kDegToRad = M_PI / 180.0;
constexpr int kUsedClasses[] = {kLOGround, kLOPillar, kLOFacade};

bool isLineClass(int c) { return c == kLOPillar || c == kLOBeam; }

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0.0, -v.z(), v.y(), v.z(), 0.0, -v.x(), -v.y(), v.x(), 0.0;
  return m;
}

void transformCloud(const Eigen::Isometry3d& T, LOCloud* cloud) {
  for (auto& pt : *cloud) {
    pt.p = T * pt.p;
    pt.dir = T.linear() * pt.dir;
  }
}

LOCloud cropCloud(const LOCloud& cloud, const Eigen::AlignedBox3d& box) {
  LOCloud out;
  for (const auto& pt : cloud)
    if (box.contains(pt.p)) out.push_back(pt);
  return out;
}

struct Pair {
  int source;
  int target;
};

}  // namespace

double distanceWeight(int iteration, double target_norm, double range) {
  const double b = std::min(0.7 + 0.05 * iteration, 1.3);
  return std::max(b + (1.0 - b) * target_norm / range, 0.01);
}

double huberWeight(double residual, double delta) {
  if (residual <= delta) return 1.0;
  return (2.0 * residual * delta - delta * delta) / (residual * residual);
}

double groundBalanceWeight(const std::array<int, kLONumClasses>& pairs) {
  const double num = pairs[kLOFacade] + 2.0 * pairs[kLOPillar] - pairs[kLOBeam];
  return std::max(0.01, num / (2.0 * pairs[kLOGround] + 1e-4));
}

LOICPResult multiMetricICP(const std::array<LOCloud, kLONumClasses>& source_in,
                           const std::array<LOCloud, kLONumClasses>& target_in,
                           const Eigen::AlignedBox3d& target_bound,
                           const Eigen::Isometry3d& initial, double gate,
                           const MullsLOParams& params) {
  LOICPResult result;
  result.T = initial;

  // Work only where both scans overlap: the target extent intersected with
  // the (initially transformed) source features, padded by a margin.
  std::array<LOCloud, kLONumClasses> src, tgt;
  Eigen::AlignedBox3d source_box;
  for (int c : kUsedClasses) {
    src[c] = source_in[c];
    transformCloud(initial, &src[c]);
    for (const auto& pt : src[c]) source_box.extend(pt.p);
  }
  Eigen::AlignedBox3d box = target_bound.intersection(source_box);
  if (box.isEmpty()) {
    result.aborted = true;
    return result;
  }
  box.min().array() -= params.crop_margin;
  box.max().array() += params.crop_margin;
  std::array<std::unique_ptr<PointTree>, kLONumClasses> trees;
  for (int c : kUsedClasses) {
    src[c] = cropCloud(src[c], box);
    tgt[c] = cropCloud(target_in[c], box);
    trees[c] = std::make_unique<PointTree>(tgt[c]);
  }

  const double gate0 = gate;
  const double cos_normal = std::cos(params.normal_max_deg * kDegToRad);
  Eigen::Isometry3d step = Eigen::Isometry3d::Identity();
  for (int k = 0; k < params.max_iterations; ++k) {
    result.iterations = k + 1;
    std::array<std::vector<Pair>, kLONumClasses> pairs;
    std::array<int, kLONumClasses> counts{};
    for (int c : kUsedClasses) {
      transformCloud(step, &src[c]);
      if (src[c].empty() || tgt[c].empty()) continue;
      // Large classes keep one source point per target point and drop the
      // source points that found no partner for the rest of the run.
      const bool prune =
          params.use_source_pruning && static_cast<int>(src[c].size()) >= params.prune_min_points;
      std::vector<char> taken(prune ? tgt[c].size() : 0, 0);
      LOCloud kept;
      for (size_t i = 0; i < src[c].size(); ++i) {
        int j;
        double d;
        if (!trees[c]->nearest(src[c][i].p, &j, &d) || d > params.search_gate_factor * gate) {
          continue;
        }
        if (prune) {
          if (taken[j]) continue;
          taken[j] = 1;
        }
        const int source_index = prune ? static_cast<int>(kept.size()) : static_cast<int>(i);
        if (prune) kept.push_back(src[c][i]);
        if (d > gate) continue;
        if (std::abs(src[c][i].dir.dot(tgt[c][j].dir)) < cos_normal) continue;
        pairs[c].push_back({source_index, j});
      }
      if (prune) src[c].swap(kept);
      counts[c] = static_cast<int>(pairs[c].size());
    }
    result.pairs = counts;

    const int line_plane = counts[kLOPillar] + counts[kLOFacade] + counts[kLOBeam];
    const int total = line_plane + counts[kLOGround];
    const double source_line_plane = static_cast<double>(src[kLOPillar].size() + src[kLOFacade].size());
    if (total < params.min_pairs || line_plane < params.min_pillar_facade_pairs ||
        (source_line_plane > 0 && line_plane / source_line_plane < params.min_pillar_facade_ratio)) {
      result.aborted = true;
      break;
    }
    gate = std::max(gate / params.gate_decay, params.gate_min);

    // Normal equations of the linearised multi-metric residuals.
    Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> g = Eigen::Matrix<double, 6, 1>::Zero();
    const double ground_weight = params.use_ground_balance ? groundBalanceWeight(counts) : 1.0;
    for (int c : kUsedClasses) {
      const double class_weight = c == kLOGround ? ground_weight : 1.0;
      for (const auto& pr : pairs[c]) {
        const LOPoint& s = src[c][pr.source];
        const LOPoint& t = tgt[c][pr.target];
        double w = class_weight;
        if (params.use_distance_weight)
          w *= distanceWeight(k, t.p.norm(), params.distance_weight_range);
        if (params.use_intensity_weight) w *= std::exp(-std::abs(s.intensity - t.intensity) / 255.0);
        const Eigen::Vector3d diff = t.p - s.p;
        if (isLineClass(c)) {
          const Eigen::Vector3d& v = t.dir;
          const Eigen::Vector3d rhs = v.cross(diff);
          if (params.use_huber && k >= params.huber_start_iteration)
            w *= huberWeight(rhs.norm(), params.huber_delta);
          Eigen::Matrix<double, 3, 6> A;
          A.leftCols<3>() = skew(v);
          A.rightCols<3>() = v.dot(s.p) * Eigen::Matrix3d::Identity() - s.p * v.transpose();
          H += w * A.transpose() * A;
          g += w * A.transpose() * rhs;
        } else {
          const Eigen::Vector3d& n = t.dir;
          const double rhs = n.dot(diff);
          if (params.use_huber && k >= params.huber_start_iteration)
            w *= huberWeight(std::abs(rhs), params.huber_delta);
          Eigen::Matrix<double, 6, 1> J;
          J.head<3>() = n;
          J.tail<3>() = s.p.cross(n);
          H += w * J * J.transpose();
          g += w * J * rhs;
        }
      }
    }
    const Eigen::Matrix<double, 6, 1> x = H.ldlt().solve(g);
    if (!x.allFinite()) {
      result.aborted = true;
      break;
    }
    const Eigen::Matrix3d R = (Eigen::AngleAxisd(x(5), Eigen::Vector3d::UnitZ()) *
                               Eigen::AngleAxisd(x(4), Eigen::Vector3d::UnitY()) *
                               Eigen::AngleAxisd(x(3), Eigen::Vector3d::UnitX()))
                                  .toRotationMatrix();
    const double angle_deg = Eigen::AngleAxisd(R).angle() / kDegToRad;
    const double shift = x.head<3>().norm();
    if (shift > 2.0 * gate0 || angle_deg > 45.0) {
      result.aborted = true;
      break;
    }
    step.linear() = R;
    step.translation() = x.head<3>();
    result.T = step * result.T;
    if (k > 2 && shift < params.converge_translation && angle_deg < params.converge_rotation_deg) {
      result.converged = true;
      break;
    }
  }
  return result;
}

}  // namespace mulls
}  // namespace localization_zoo
