#include "inten_loam/inten_loam_paper.h"

#include <Eigen/Dense>
#include "inten_loam/detail/feature_tree.h"
#include "inten_loam/detail/ground_pca.h"
#include "inten_loam/detail/mapped_motion_prior.h"
#include "inten_loam/detail/plane_row_support.h"
#include "inten_loam/detail/translation_prior.h"
#include "inten_loam/detail/s2s_consensus.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace localization_zoo {
namespace inten_loam {

namespace {

using Vec6 = Eigen::Matrix<double, 6, 1>;
using Mat6 = Eigen::Matrix<double, 6, 6>;

using Tree = detail::FeatureTree;

Eigen::Matrix4d expSe3(const Vec6& d) {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  const Eigen::Vector3d w = d.tail<3>();
  if (w.norm() > 1e-12) T.block<3, 3>(0, 0) = Eigen::AngleAxisd(w.norm(), w.normalized()).toRotationMatrix();
  T.block<3, 1>(0, 3) = d.head<3>();
  return T;
}

// Accumulates one weighted residual r with Jacobian J = [a, x x a] (a: the
// gradient of r with respect to the point), Huber-weighted.
struct System {
  Mat6 H = Mat6::Zero();
  Vec6 g = Vec6::Zero();
  int n = 0;
  bool track_residual = false;
  double weighted_squared_residual = 0.0;
  void add(const Eigen::Vector3d& x, const Eigen::Vector3d& a, double r, double huber, System* also = nullptr,
           bool cauchy = false) {
    Vec6 J;
    J.head<3>() = a;
    J.tail<3>() = x.cross(a);
    const double w = cauchy ? 1.0 / (1.0 + (r / huber) * (r / huber))
                            : (std::abs(r) <= huber ? 1.0 : huber / std::abs(r));
    H += w * J * J.transpose();
    g += w * J * r;
    ++n;
    if (track_residual) weighted_squared_residual += w * r * r;
    if (also) {
      also->H += w * J * J.transpose();
      also->g += w * J * r;
      ++also->n;
    }
  }
};

void observeSystem(PaperSolverTrace* trace, const System& sys,
                   const Eigen::Matrix4d& T, const int counts[4]) {
  if (!trace) return;
  trace->iterations.emplace_back();
  auto& iteration = trace->iterations.back();
  iteration.linearization_pose = T;
  iteration.H = sys.H;
  iteration.g = sys.g;
  iteration.pairs = sys.n;
  iteration.weighted_squared_residual = sys.weighted_squared_residual;
  for (int c = 0; c < 4; ++c) iteration.class_pairs[c] = counts[c];
}

void observeStep(PaperSolverTrace* trace, const Vec6& delta) {
  if (!trace) return;
  auto& iteration = trace->iterations.back();
  iteration.delta = delta;
  iteration.step_attempted = true;
  iteration.step_finite = delta.allFinite();
  iteration.update_applied = iteration.step_finite;
}

// Querying immutable trees/fields is independent for every feature. Buffer the
// results before the serial reduction so Huber weighting, class diagnostics and
// every H/g addition keep the legacy order, irrespective of the worker count.
template <typename Query>
void accumulateFeatures(const std::vector<PaperFeature>& features,
                        const Eigen::Matrix3d& R, const Eigen::Vector3d& t,
                        const Query& query, double huber, bool accelerated,
                        System* system, System* also,
                        std::vector<int>* indices, std::vector<float>* distances, bool cauchy = false) {
  if (!accelerated) {
    for (const auto& f : features) {
      const Eigen::Vector3d x = R * f.p + t;
      Eigen::Vector3d gradient;
      double residual;
      if (query(f, x, &gradient, &residual, indices, distances))
        system->add(x, gradient, residual, huber, also, cauchy);
    }
    return;
  }
  struct Residual {
    Eigen::Vector3d x, gradient;
    double value = 0.0;
    bool valid = false;
  };
  std::vector<Residual> values(features.size());
#pragma omp parallel
  {
    std::vector<int> idx;
    std::vector<float> d2;
#pragma omp for schedule(static)
    for (long i = 0; i < static_cast<long>(features.size()); ++i) {
      Residual& value = values[i];
      value.x = R * features[i].p + t;
      value.valid = query(features[i], value.x, &value.gradient, &value.value, &idx, &d2);
    }
  }
  for (const auto& value : values)
    if (value.valid) system->add(value.x, value.gradient, value.value, huber, also, cauchy);
}

void bspline(double t, double B[4], double dB[4]) {
  const double t2 = t * t, t3 = t2 * t, u = 1.0 - t;
  B[0] = u * u * u / 6.0;
  B[1] = (3.0 * t3 - 6.0 * t2 + 4.0) / 6.0;
  B[2] = (-3.0 * t3 + 3.0 * t2 + 3.0 * t + 1.0) / 6.0;
  B[3] = t3 / 6.0;
  dB[0] = -0.5 * u * u;
  dB[1] = 1.5 * t2 - 2.0 * t;
  dB[2] = -1.5 * t2 + t + 0.5;
  dB[3] = 0.5 * t2;
}

struct VoxelKey {
  int x, y, z;
  bool operator==(const VoxelKey& o) const { return x == o.x && y == o.y && z == o.z; }
};
struct VoxelKeyHash {
  size_t operator()(const VoxelKey& k) const {
    return static_cast<size_t>((static_cast<int64_t>(k.x) * 73856093) ^ (static_cast<int64_t>(k.y) * 19349663) ^
                               (static_cast<int64_t>(k.z) * 83492791));
  }
};

// Temporal voxel filter cell: mean position of all points, intensity over
// the last observations (averaged within the temporal window at query time).
struct TemporalVoxel {
  Eigen::Vector3d sum = Eigen::Vector3d::Zero();
  int count = 0;
  int cluster = 0;
  std::deque<std::pair<int, float>> intensity;  // (frame, intensity)
};

}  // namespace

struct InTenLoamPaper::ClassSystems {
  System cls[4];  // edge, ground, facade, intensity
};

struct InTenLoamPaper::Index {
  Tree ground, facade, edge, all;
  IntensityGrid intensity;
  std::unique_ptr<detail::PlaneRowSupport> ground_rows, facade_rows;
};

struct InTenLoamPaper::Map {
  using Grid = std::unordered_map<VoxelKey, TemporalVoxel, VoxelKeyHash>;
  Grid edge, ground, facade, reflector;
  // Local map for the current S2M (world frame).
  Tree local_edge, local_ground, local_facade, local_reflector;
  IntensityGrid local_intensity;
};

InTenLoamPaper::InTenLoamPaper(const InTenLoamPaperParams& params)
    : params_(params), map_(std::make_shared<Map>()) {
  if (params_.s2s_facade_cauchy && (!std::isfinite(params_.huber) || params_.huber <= 0.0))
    throw std::invalid_argument("Cauchy facade influence requires a finite positive Huber scale");
  if (!std::isfinite(params_.s2s_translation_prior_ratio) ||
      params_.s2s_translation_prior_ratio < 0.0 || params_.s2s_translation_prior_ratio > 1.0)
    throw std::invalid_argument("s2s_translation_prior_ratio must be finite and in [0,1]");
}

// ---------------------------------------------------------------------------
// FEF
// ---------------------------------------------------------------------------

PaperFrame InTenLoamPaper::extractFeatures(const std::vector<Eigen::Vector3d>& points,
                                           const std::vector<float>& intensity) const {
  const int W = params_.width, H = params_.height;
  const double theta_d = params_.theta_down_deg * M_PI / 180.0;
  const double theta = (params_.theta_down_deg + params_.theta_up_deg) * M_PI / 180.0;
  std::vector<float> D(static_cast<size_t>(W) * H, 0.0f), I(static_cast<size_t>(W) * H, 0.0f);
  std::vector<int> src(static_cast<size_t>(W) * H, -1);
  for (size_t i = 0; i < points.size(); ++i) {
    const Eigen::Vector3d& p = points[i];
    const double r = p.norm();
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    int u = static_cast<int>(std::floor((1.0 - std::atan2(p.y(), p.x()) / M_PI) * W / 2.0));
    u = ((u % W) + W) % W;
    const int v = static_cast<int>(std::floor((std::asin(p.z() / r) + theta_d) / theta * H));
    if (v < 0 || v >= H) continue;
    const size_t k = static_cast<size_t>(v) * W + u;
    if (D[k] == 0.0f || r < D[k]) {
      D[k] = static_cast<float>(r);
      src[k] = static_cast<int>(i);
    }
    I[k] = std::max(I[k], i < intensity.size() ? intensity[i] : 0.0f);
  }
  auto at = [W](int u, int v) { return static_cast<size_t>(v) * W + ((u % W) + W) % W; };

  // Segment image (lowest z per azimuth segment and radial bin) and
  // per-segment ground lines.
  const int S = params_.segment_columns;
  const int NB = static_cast<int>(std::ceil(params_.max_range / params_.segment_bin)) + 1;
  std::vector<float> seg_z(static_cast<size_t>(S) * NB, std::numeric_limits<float>::infinity());
  std::vector<float> seg_rho(static_cast<size_t>(S) * NB, 0.0f);
  for (int v = 0; v < H; ++v)
    for (int u = 0; u < W; ++u) {
      const size_t k = at(u, v);
      if (src[k] < 0) continue;
      const Eigen::Vector3d& p = points[static_cast<size_t>(src[k])];
      const double rho = p.head<2>().norm();
      const int b = std::min(NB - 1, static_cast<int>(rho / params_.segment_bin));
      const size_t sk = static_cast<size_t>(u * S / W) * NB + b;
      if (p.z() < seg_z[sk]) {
        seg_z[sk] = static_cast<float>(p.z());
        seg_rho[sk] = static_cast<float>(rho);
      }
    }
  struct Line { double a, b, rho0, rho1; };
  std::vector<std::vector<Line>> lines(static_cast<size_t>(S));
  auto fit = [](const std::vector<std::pair<double, double>>& pts, double* a, double* b, double* rmse) {
    const double n = static_cast<double>(pts.size());
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (const auto& q : pts) { sx += q.first; sy += q.second; sxx += q.first * q.first; sxy += q.first * q.second; }
    const double den = n * sxx - sx * sx;
    *a = std::abs(den) < 1e-9 ? 0.0 : (n * sxy - sx * sy) / den;
    *b = (sy - *a * sx) / n;
    double e = 0;
    for (const auto& q : pts) e += std::pow(*a * q.first + *b - q.second, 2);
    *rmse = std::sqrt(e / n);
  };
#pragma omp parallel for if(params_.accelerated) schedule(static)
  for (int s = 0; s < S; ++s) {
    std::vector<std::pair<double, double>> cur;
    auto close = [&]() {
      if (cur.empty()) return;
      double a = 0, b = cur.front().second, e = 0;
      if (cur.size() >= 2) fit(cur, &a, &b, &e);
      lines[static_cast<size_t>(s)].push_back({a, b, cur.front().first, cur.back().first});
      cur.clear();
    };
    for (int bi = 0; bi < NB; ++bi) {
      const size_t sk = static_cast<size_t>(s) * NB + bi;
      if (!std::isfinite(seg_z[sk])) continue;
      const std::pair<double, double> q(seg_rho[sk], seg_z[sk]);
      if (cur.empty()) {
        bool start = q.second < params_.ground_start_below;
        if (!start && !lines[static_cast<size_t>(s)].empty()) {
          const Line& l = lines[static_cast<size_t>(s)].back();
          start = std::abs(l.a * q.first + l.b - q.second) < params_.ground_threshold;
        }
        if (start) cur.push_back(q);
        continue;
      }
      auto trial = cur;
      trial.push_back(q);
      double a, b, e;
      fit(trial, &a, &b, &e);
      if (std::abs(a) <= params_.ground_max_slope && e <= params_.ground_max_fit_error) {
        cur.push_back(q);
      } else {
        close();
        if (q.second < params_.ground_start_below) cur.push_back(q);
      }
    }
    close();
  }

  // Labels: 1 ground, >= 2 cluster id, 0 masked / invalid.
  std::vector<int> label(static_cast<size_t>(W) * H, -1);
#pragma omp parallel for if(params_.accelerated) schedule(static)
  for (int v = 0; v < H; ++v)
    for (int u = 0; u < W; ++u) {
      const size_t k = at(u, v);
      if (src[k] < 0) continue;
      const Eigen::Vector3d& p = points[static_cast<size_t>(src[k])];
      const double rho = p.head<2>().norm();
      const auto& ls = lines[static_cast<size_t>(u * S / W)];
      const Line* best = nullptr;
      double best_gap = 3.0;
      for (const auto& l : ls) {
        const double gap = rho < l.rho0 ? l.rho0 - rho : (rho > l.rho1 ? rho - l.rho1 : 0.0);
        if (gap < best_gap) { best_gap = gap; best = &l; }
      }
      if (best && std::abs(best->a * rho + best->b - p.z()) < params_.ground_threshold) label[k] = 1;
    }


  // Angle-based clustering of the non-ground pixels.
  const double alpha_u = 2.0 * M_PI / W, alpha_v = theta / H;
  const double beta_min = params_.cluster_angle_deg * M_PI / 180.0;
  double sin_alpha[2]{}, cos_alpha[2]{};
  if (params_.accelerated) {
    sin_alpha[0] = std::sin(alpha_u); sin_alpha[1] = std::sin(alpha_v);
    cos_alpha[0] = std::cos(alpha_u); cos_alpha[1] = std::cos(alpha_v);
  }
  int next = 2;
  std::vector<int> stack, members;
  for (int v = 0; v < H; ++v)
    for (int u = 0; u < W; ++u) {
      const size_t k0 = at(u, v);
      if (src[k0] < 0 || label[k0] != -1) continue;
      const int id = next++;
      members.clear();
      stack.assign(1, static_cast<int>(k0));
      label[k0] = id;
      while (!stack.empty()) {
        const int k = stack.back();
        stack.pop_back();
        members.push_back(k);
        const int cu = k % W, cv = k / W;
        const int du[4] = {1, -1, 0, 0}, dv[4] = {0, 0, 1, -1};
        for (int n = 0; n < 4; ++n) {
          const int nv = cv + dv[n];
          if (nv < 0 || nv >= H) continue;
          const size_t nk = at(cu + du[n], nv);
          if (src[nk] < 0 || label[nk] != -1) continue;
          const double d1 = std::max(D[static_cast<size_t>(k)], D[nk]);
          const double d2 = std::min(D[static_cast<size_t>(k)], D[nk]);
          const double a = n < 2 ? alpha_u : alpha_v;
          const double sa = params_.accelerated ? sin_alpha[n < 2 ? 0 : 1] : std::sin(a);
          const double ca = params_.accelerated ? cos_alpha[n < 2 ? 0 : 1] : std::cos(a);
          if (std::atan2(d2 * sa, d1 - d2 * ca) > beta_min) {
            label[nk] = id;
            stack.push_back(static_cast<int>(nk));
          }
        }
      }
      if (static_cast<int>(members.size()) < params_.min_cluster_points)
        for (const int m : members) label[static_cast<size_t>(m)] = 0;
    }

  // Curvature along the row; points behind an occluder are unreliable.
  const int hw = params_.curvature_half_window;
  std::vector<float> C(static_cast<size_t>(W) * H, std::numeric_limits<float>::quiet_NaN());
#pragma omp parallel for if(params_.accelerated) schedule(static)
  for (int v = 0; v < H; ++v)
    for (int u = 0; u < W; ++u) {
      const size_t k = at(u, v);
      if (src[k] < 0) continue;
      double sum = 0;
      int n = 0;
      bool occluded = false;
      for (int i = -hw; i <= hw; ++i) {
        if (i == 0) continue;
        const size_t nk = at(u + i, v);
        if (src[nk] < 0) continue;
        sum += D[k] - D[nk];
        ++n;
        if (std::abs(i) == 1 && D[k] - D[nk] > 0.1 * D[k]) occluded = true;
      }
      if (n >= hw && !occluded) C[k] = static_cast<float>(sum / (n * D[k]));
    }


  // Intensity block thresholds (median per block).
  std::vector<float> block_median(static_cast<size_t>(params_.intensity_blocks_u * params_.intensity_blocks_v), 1.0f);
  {
    std::vector<std::vector<float>> vals(block_median.size());
    for (int v = 0; v < H; ++v)
      for (int u = 0; u < W; ++u) {
        const size_t k = at(u, v);
        if (src[k] < 0) continue;
        const int bu = u * params_.intensity_blocks_u / W, bv = v * params_.intensity_blocks_v / H;
        vals[static_cast<size_t>(bv * params_.intensity_blocks_u + bu)].push_back(I[k]);
      }
    for (size_t b = 0; b < vals.size(); ++b) {
      if (vals[b].empty()) continue;
      std::nth_element(vals[b].begin(), vals[b].begin() + vals[b].size() / 2, vals[b].end());
      block_median[b] = vals[b][vals[b].size() / 2];
    }
  }

  PaperFrame frame;
  auto feature = [&](size_t k) {
    PaperFeature f;
    f.p = points[static_cast<size_t>(src[k])];
    f.intensity = I[k];
    f.row = static_cast<int>(k) / W;
    f.cluster = std::max(0, label[k]);
    return f;
  };
  std::vector<size_t> ground, reflector;
  for (int v = 0; v < H; ++v)
    for (int u = 0; u < W; ++u) {
      const size_t k = at(u, v);
      if (src[k] < 0) continue;
      frame.all.push_back(feature(k));
      if (label[k] == 0) continue;
      if (label[k] == 1) ground.push_back(k);
      bool refl = false;
      if (v + 1 < H && src[at(u, v + 1)] >= 0 && I[k] - I[at(u, v + 1)] > params_.reflector_delta) refl = true;
      const int bu = u * params_.intensity_blocks_u / W, bv = v * params_.intensity_blocks_v / H;
      if (I[k] > block_median[static_cast<size_t>(bv * params_.intensity_blocks_u + bu)]) refl = true;
      if (refl) reflector.push_back(k);
    }
  auto subsample = [&](const std::vector<size_t>& ks, int max_n, std::vector<PaperFeature>* out) {
    if (ks.empty() || max_n <= 0) return;
    const double step = std::max(1.0, static_cast<double>(ks.size()) / max_n);
    for (double i = 0; i < static_cast<double>(ks.size()) && static_cast<int>(out->size()) < max_n; i += step)
      out->push_back(feature(ks[static_cast<size_t>(i)]));
  };
  subsample(ground, params_.max_ground, &frame.ground);
  subsample(reflector, params_.max_reflectors, &frame.reflector);

  // Edges (largest |C|) and facades (smallest |C|) per row sector with
  // non-maximum suppression.
  std::vector<char> taken(static_cast<size_t>(W) * H, 0);
  std::vector<std::pair<float, int>> cand;
  for (int v = 0; v < H; ++v)
    for (int s = 0; s < params_.sectors_per_row; ++s) {
      const int u0 = s * W / params_.sectors_per_row, u1 = (s + 1) * W / params_.sectors_per_row;
      cand.clear();
      for (int u = u0; u < u1; ++u) {
        const size_t k = at(u, v);
        if (src[k] < 0 || label[k] < 2 || !std::isfinite(C[k])) continue;
        cand.emplace_back(std::abs(C[k]), u);
      }
      std::sort(cand.begin(), cand.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
      int picked = 0;
      for (const auto& c : cand) {
        if (picked >= params_.edges_per_sector || c.first <= params_.edge_threshold) break;
        const size_t k = at(c.second, v);
        if (taken[k]) continue;
        frame.edge.push_back(feature(k));
        ++picked;
        for (int i = -hw; i <= hw; ++i) taken[at(c.second + i, v)] = 1;
      }
      picked = 0;
      for (auto it = cand.rbegin(); it != cand.rend(); ++it) {
        if (picked >= params_.facades_per_sector || it->first >= params_.facade_threshold) break;
        const size_t k = at(it->second, v);
        if (taken[k]) continue;
        frame.facade.push_back(feature(k));
        ++picked;
        for (int i = -hw; i <= hw; ++i) taken[at(it->second + i, v)] = 1;
      }
    }
  return frame;
}

// ---------------------------------------------------------------------------
// Intensity field (eq. 6)
// ---------------------------------------------------------------------------

void InTenLoamPaper::IntensityGrid::build(const std::vector<PaperFeature>& cloud, double knot,
                                         bool normalized) {
  knot_ = knot;
  normalized_ = normalized;
  nodes_.clear();
  nodes_.reserve(cloud.size());
  for (const auto& f : cloud) {
    const Key k{static_cast<int>(std::lround(f.p.x() / knot)), static_cast<int>(std::lround(f.p.y() / knot)),
                static_cast<int>(std::lround(f.p.z() / knot))};
    auto& n = nodes_[k];
    n.first += f.intensity;
    ++n.second;
  }
}

bool InTenLoamPaper::IntensityGrid::eval(const Eigen::Vector3d& x, double* value,
                                         Eigen::Vector3d* gradient) const {
  const Eigen::Vector3d s = x / knot_;
  const Eigen::Vector3i c0(static_cast<int>(std::floor(s.x())), static_cast<int>(std::floor(s.y())),
                           static_cast<int>(std::floor(s.z())));
  const Eigen::Vector3d t = s - c0.cast<double>();
  double c[64];
  bool seen[64];
  double all = 0;
  int all_n = 0;
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      for (int k = 0; k < 4; ++k) {
        const int n = (i * 4 + j) * 4 + k;
        const auto it = nodes_.find(Key{c0.x() + i - 1, c0.y() + j - 1, c0.z() + k - 1});
        seen[n] = it != nodes_.end();
        if (seen[n]) {
          c[n] = it->second.first / it->second.second;
          all += c[n];
          ++all_n;
        }
      }
  if (all_n < 4) return false;
  const double mean = all / all_n;
  double Bx[4], By[4], Bz[4], dBx[4], dBy[4], dBz[4];
  bspline(t.x(), Bx, dBx);
  bspline(t.y(), By, dBy);
  bspline(t.z(), Bz, dBz);
  if (normalized_) {
    // If every occupied node shares an axis index, that basis factor
    // cancels exactly in numerator and denominator. Cancel before dividing:
    // near its zero, a quotient of tiny numbers loses gradient precision.
    int lower[3] = {4, 4, 4}, upper[3] = {-1, -1, -1};
    for (int i = 0; i < 4; ++i)
      for (int j = 0; j < 4; ++j)
        for (int k = 0; k < 4; ++k)
          if (seen[(i * 4 + j) * 4 + k]) {
            const int index[3] = {i, j, k};
            for (int a = 0; a < 3; ++a) {
              lower[a] = std::min(lower[a], index[a]);
              upper[a] = std::max(upper[a], index[a]);
            }
          }
    double* basis[3] = {Bx, By, Bz};
    double* derivative[3] = {dBx, dBy, dBz};
    for (int a = 0; a < 3; ++a)
      if (lower[a] == upper[a]) {
        if (!(basis[a][lower[a]] > 0.0)) return false;
        basis[a][lower[a]] = 1.0;
        derivative[a][lower[a]] = 0.0;
      }
    double numerator = 0.0, weight = 0.0;
    Eigen::Vector3d numerator_gradient = Eigen::Vector3d::Zero();
    Eigen::Vector3d weight_gradient = Eigen::Vector3d::Zero();
    for (int i = 0; i < 4; ++i)
      for (int j = 0; j < 4; ++j)
        for (int k = 0; k < 4; ++k) {
          const int n = (i * 4 + j) * 4 + k;
          if (!seen[n]) continue;
          const double w = Bx[i] * By[j] * Bz[k];
          const Eigen::Vector3d dw(dBx[i] * By[j] * Bz[k],
                                   Bx[i] * dBy[j] * Bz[k], Bx[i] * By[j] * dBz[k]);
          numerator += w * c[n];
          weight += w;
          numerator_gradient += dw * c[n];
          weight_gradient += dw;
        }
    if (!(weight > 0.0)) return false;
    *value = numerator / weight;
    *gradient = (numerator_gradient - *value * weight_gradient) / (weight * knot_);
    return std::isfinite(*value) && gradient->allFinite();
  }
  double val = 0;
  Eigen::Vector3d grad = Eigen::Vector3d::Zero();
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      for (int k = 0; k < 4; ++k) {
        const int n = (i * 4 + j) * 4 + k;
        const double cv = seen[n] ? c[n] : mean;
        val += Bx[i] * By[j] * Bz[k] * cv;
        grad += cv * Eigen::Vector3d(dBx[i] * By[j] * Bz[k], Bx[i] * dBy[j] * Bz[k], Bx[i] * By[j] * dBz[k]);
      }
  *value = val;
  *gradient = grad / knot_;
  return true;
}

// ---------------------------------------------------------------------------
// S2S (Alg. 1)
// ---------------------------------------------------------------------------

int InTenLoamPaper::solveS2S(const PaperFrame& curr, Eigen::Matrix4d* T, ClassSystems* first,
                             PaperSolverTrace* trace, const std::vector<char>* facade_dynamic,
                             bool translation_only) const {
  const Index& prev = *prev_;
  const double gate = params_.s2s_gate_sq;
  const double huber = params_.huber;
  const Eigen::Vector3d prediction = T->block<3, 1>(0, 3);
  Eigen::Matrix3d prior_information = Eigen::Matrix3d::Zero();
  std::vector<int> idx;
  std::vector<float> d2;
  int pairs = 0;
  if (trace) trace->stop = params_.s2s_iterations > 0 ? PaperSolverStop::IterationLimit : PaperSolverStop::NoIterations;
  for (int it = 0; it < params_.s2s_iterations; ++it) {
    System sys;
    sys.track_residual = trace != nullptr;
    auto cs = [&](int c) { return first && it == 0 ? &first->cls[c] : nullptr; };
    const Eigen::Matrix3d R = T->block<3, 3>(0, 0);
    const Eigen::Vector3d t = T->block<3, 1>(0, 3);
    auto edge = [&](const PaperFeature&, const Eigen::Vector3d& x, Eigen::Vector3d* a,
                    double* residual, std::vector<int>* indices, std::vector<float>* distances) {
      const int n = prev.edge.knn(x, 10, indices, distances);
      const auto& ids = *indices;
      const auto& ds = *distances;
      if (n < 2 || ds[0] > gate) return false;
      const PaperFeature& qj = prev.edge.pts[static_cast<size_t>(ids[0])];
      int m = -1;
      for (int k = 1; k < n && ds[static_cast<size_t>(k)] <= gate; ++k) {
        const int dr = std::abs(prev.edge.pts[static_cast<size_t>(ids[static_cast<size_t>(k)])].row - qj.row);
        if (dr >= 1 && dr <= 2) { m = ids[static_cast<size_t>(k)]; break; }
      }
      if (m < 0) return false;
      const Eigen::Vector3d dir = qj.p - prev.edge.pts[static_cast<size_t>(m)].p;
      if (dir.norm() < 1e-3) return false;
      const Eigen::Vector3d v = dir.normalized(), d = x - qj.p;
      const Eigen::Vector3d perp = d - v * v.dot(d);
      const double r = perp.norm();
      if (r < 1e-9) return false;
      *a = perp / r;
      *residual = r;
      return true;
    };
    accumulateFeatures(curr.edge, R, t, edge, huber, params_.accelerated, &sys, cs(0), &idx, &d2);
    int counts[4];
    counts[0] = sys.n;
    // Planes (eq. 12): q_j closest, q_n same scan line, q_m adjacent line.
    for (int cls = 0; cls < 2; ++cls) {
      const int before = sys.n;
      const auto& src = cls == 0 ? curr.ground : curr.facade;
      const Tree& tree = cls == 0 ? prev.ground : prev.facade;
      auto plane = [&](const PaperFeature& source, const Eigen::Vector3d& x, Eigen::Vector3d* a,
                       double* residual, std::vector<int>* indices, std::vector<float>* distances) {
        if (cls == 1 && facade_dynamic && source.cluster >= 0 &&
            static_cast<size_t>(source.cluster) < facade_dynamic->size() &&
            (*facade_dynamic)[static_cast<size_t>(source.cluster)]) return false;
        const int n = tree.knn(x, 10, indices, distances);
        const auto& ids = *indices;
        const auto& ds = *distances;
        if (n < 3 || ds[0] > gate) return false;
        const PaperFeature& qj = tree.pts[static_cast<size_t>(ids[0])];
        int same = -1, adj = -1;
        for (int k = 1; k < n && ds[static_cast<size_t>(k)] <= gate; ++k) {
          const int dr = std::abs(tree.pts[static_cast<size_t>(ids[static_cast<size_t>(k)])].row - qj.row);
          if (dr == 0 && same < 0) same = ids[static_cast<size_t>(k)];
          if (dr >= 1 && dr <= 2 && adj < 0) adj = ids[static_cast<size_t>(k)];
        }
        if (params_.s2s_row_support && (same < 0 || adj < 0)) {
          const auto& rows = cls == 0 ? prev.ground_rows : prev.facade_rows;
          rows->complete(tree.pts, x, ids[0], gate, &same, &adj);
        }
        if (same < 0 || adj < 0) return false;
        Eigen::Vector3d nrm = (qj.p - tree.pts[static_cast<size_t>(adj)].p)
                    .cross(qj.p - tree.pts[static_cast<size_t>(same)].p);
        if (nrm.norm() < 1e-6) return false;
        nrm.normalize();
        if ((cls == 0 && params_.s2s_ground_pca) || (cls == 1 && params_.s2s_facade_pca))
          detail::groundPcaNormal(tree.pts, ids, ds, gate, &nrm);
        *a = nrm;
        *residual = nrm.dot(x - qj.p);
        return true;
      };
      accumulateFeatures(src, R, t, plane, huber, params_.accelerated, &sys, cs(1 + cls), &idx, &d2,
                         cls == 1 && params_.s2s_facade_cauchy);
      counts[1 + cls] = sys.n - before;
    }
    // Reflectors (eq. 7) against the previous scan intensity field.
    const int before_intensity = sys.n;
    if (params_.use_intensity) {
      auto intensity = [&](const PaperFeature& f, const Eigen::Vector3d& x, Eigen::Vector3d* a,
                           double* residual, std::vector<int>*, std::vector<float>*) {
        double mu;
        Eigen::Vector3d grad;
        if (!prev.intensity.eval(x, &mu, &grad)) return false;
        const double w = params_.intensity_weight;
        *a = w * grad;
        *residual = w * (mu - f.intensity);
        return true;
      };
      accumulateFeatures(curr.reflector, R, t, intensity, huber, params_.accelerated, &sys, cs(3), &idx, &d2);
    }
    pairs = sys.n;
    counts[3] = sys.n - before_intensity;
    if (params_.s2s_translation_prior_ratio > 0.0 && sys.n >= 30) {
      if (it == 0) {
        prior_information = detail::translationPriorInformation(sys.H, t, params_.s2s_translation_prior_ratio);
        if (trace) trace->translation_prior_information = prior_information;
      }
      const double cost = detail::addTranslationPrior(prior_information, prediction, t, &sys.H, &sys.g);
      if (sys.track_residual) sys.weighted_squared_residual += cost;
    }
    observeSystem(trace, sys, *T, counts);
    if (sys.n < 30) {
      if (trace) trace->stop = PaperSolverStop::InsufficientPairs;
      return sys.n;
    }
    if (translation_only) {
      Vec6 delta = Vec6::Zero();
      delta.head<3>() = (sys.H.topLeftCorner<3, 3>() +
          1e-6 * Eigen::Matrix3d::Identity()).ldlt().solve(-sys.g.head<3>());
      observeStep(trace, delta);
      if (!delta.allFinite()) {
        if (trace) trace->stop = PaperSolverStop::NonFiniteStep;
        return 0;
      }
      T->block<3, 1>(0, 3) += delta.head<3>();
      if (delta.norm() < params_.convergence) {
        if (trace) trace->stop = PaperSolverStop::Converged;
        break;
      }
      continue;
    }
    const Vec6 delta = (sys.H + 1e-6 * Mat6::Identity()).ldlt().solve(-sys.g);
    observeStep(trace, delta);
    if (!delta.allFinite()) {
      if (trace) trace->stop = PaperSolverStop::NonFiniteStep;
      return 0;
    }
    *T = expSe3(delta) * *T;
    if (delta.norm() < params_.convergence) {
      if (trace) trace->stop = PaperSolverStop::Converged;
      break;
    }
  }
  return pairs;
}

int InTenLoamPaper::solveS2M(const PaperFrame& curr, Eigen::Matrix4d* T, int class_pairs[4],
                             ClassSystems* first, PaperSolverTrace* trace) const {
  const Map& map = *map_;
  const double gate = params_.s2m_gate_sq;
  const double huber = params_.huber;
  std::vector<int> idx;
  std::vector<float> d2;
  int pairs = 0;
  auto pca = [&](const Tree& tree, const std::vector<int>& ids, Eigen::Vector3d* c,
                 Eigen::Matrix3d* vec, Eigen::Vector3d* val) {
    *c = Eigen::Vector3d::Zero();
    for (int k = 0; k < 5; ++k) *c += tree.pts[static_cast<size_t>(ids[static_cast<size_t>(k)])].p;
    *c /= 5.0;
    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    for (int k = 0; k < 5; ++k) {
      const Eigen::Vector3d d = tree.pts[static_cast<size_t>(ids[static_cast<size_t>(k)])].p - *c;
      cov += d * d.transpose();
    }
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov / 5.0);
    *vec = es.eigenvectors();
    *val = es.eigenvalues();
  };
  if (trace) trace->stop = params_.s2m_iterations > 0 ? PaperSolverStop::IterationLimit : PaperSolverStop::NoIterations;
  for (int it = 0; it < params_.s2m_iterations; ++it) {
    System sys;
    sys.track_residual = trace != nullptr;
    auto cs = [&](int c) { return first && it == 0 ? &first->cls[c] : nullptr; };
    const Eigen::Matrix3d R = T->block<3, 3>(0, 0);
    const Eigen::Vector3d t = T->block<3, 1>(0, 3);
    auto edge = [&](const PaperFeature&, const Eigen::Vector3d& x, Eigen::Vector3d* a,
                    double* residual, std::vector<int>* indices, std::vector<float>* distances) {
      if (map.local_edge.knn(x, 5, indices, distances) < 5 || (*distances)[4] > gate) return false;
      Eigen::Vector3d c, val;
      Eigen::Matrix3d vec;
      pca(map.local_edge, *indices, &c, &vec, &val);
      if (val(2) < 3.0 * val(1)) return false;
      const Eigen::Vector3d v = vec.col(2), d = x - c;
      const Eigen::Vector3d perp = d - v * v.dot(d);
      const double r = perp.norm();
      if (r < 1e-9) return false;
      *a = perp / r;
      *residual = r;
      return true;
    };
    accumulateFeatures(curr.edge, R, t, edge, huber, params_.accelerated, &sys, cs(0), &idx, &d2);
    int counts[4];
    counts[0] = sys.n;
    for (int cls = 0; cls < 2; ++cls) {
      const auto& src = cls == 0 ? curr.ground : curr.facade;
      const Tree& tree = cls == 0 ? map.local_ground : map.local_facade;
      const int before = sys.n;
      auto plane = [&](const PaperFeature&, const Eigen::Vector3d& x, Eigen::Vector3d* a,
                       double* residual, std::vector<int>* indices, std::vector<float>* distances) {
        if (tree.knn(x, 5, indices, distances) < 5 || (*distances)[4] > gate) return false;
        Eigen::Vector3d c, val;
        Eigen::Matrix3d vec;
        pca(tree, *indices, &c, &vec, &val);
        const Eigen::Vector3d nrm = vec.col(0);
        bool planar = true;
        for (int k = 0; k < 5 && planar; ++k)
          planar = std::abs(nrm.dot(tree.pts[static_cast<size_t>((*indices)[static_cast<size_t>(k)])].p - c)) <
                   params_.s2m_plane_max_dist;
        if (!planar || val(1) < params_.s2m_plane_min_spread * val(2)) return false;
        *a = nrm;
        *residual = nrm.dot(x - c);
        return true;
      };
      accumulateFeatures(src, R, t, plane, huber, params_.accelerated, &sys, cs(1 + cls), &idx, &d2);
      counts[1 + cls] = sys.n - before;
    }
    const int before_intensity = sys.n;
    if (params_.use_intensity) {
      auto intensity = [&](const PaperFeature& f, const Eigen::Vector3d& x, Eigen::Vector3d* a,
                           double* residual, std::vector<int>*, std::vector<float>*) {
        double mu;
        Eigen::Vector3d grad;
        if (!map.local_intensity.eval(x, &mu, &grad)) return false;
        const double w = params_.intensity_weight;
        *a = w * grad;
        *residual = w * (mu - f.intensity);
        return true;
      };
      accumulateFeatures(curr.reflector, R, t, intensity, huber, params_.accelerated, &sys, cs(3), &idx, &d2);
    }
    pairs = sys.n;
    counts[3] = sys.n - before_intensity;
    if (class_pairs)
      for (int k = 0; k < 4; ++k) class_pairs[k] = counts[k];
    observeSystem(trace, sys, *T, counts);
    if (sys.n < 30) {
      if (trace) trace->stop = PaperSolverStop::InsufficientPairs;
      return sys.n;
    }
    const Vec6 delta = (sys.H + 1e-6 * Mat6::Identity()).ldlt().solve(-sys.g);
    observeStep(trace, delta);
    if (!delta.allFinite()) {
      if (trace) trace->stop = PaperSolverStop::NonFiniteStep;
      return 0;
    }
    *T = expSe3(delta) * *T;
    if (delta.norm() < params_.convergence) {
      if (trace) trace->stop = PaperSolverStop::Converged;
      break;
    }
  }
  return pairs;
}

// ---------------------------------------------------------------------------
// DOR (Sec. 3.3) and map insertion with the temporal voxel filter
// ---------------------------------------------------------------------------

std::vector<char> InTenLoamPaper::dynamicMask(const PaperFrame& curr, const Eigen::Matrix4d& T_w) const {
  int max_cluster = 1;
  for (const auto& f : curr.all) max_cluster = std::max(max_cluster, f.cluster);
  std::vector<char> dynamic(static_cast<size_t>(max_cluster) + 1, 0);
  const int W = params_.width, H = params_.height;
  const double theta_d = params_.theta_down_deg * M_PI / 180.0;
  const double theta = (params_.theta_down_deg + params_.theta_up_deg) * M_PI / 180.0;
  auto pixel = [&](const Eigen::Vector3d& p, int* k) {
    const double r = p.norm();
    if (r < params_.min_range || r > params_.max_range) return false;
    int u = static_cast<int>(std::floor((1.0 - std::atan2(p.y(), p.x()) / M_PI) * W / 2.0));
    u = ((u % W) + W) % W;
    const int v = static_cast<int>(std::floor((std::asin(p.z() / r) + theta_d) / theta * H));
    if (v < 0 || v >= H) return false;
    *k = v * W + u;
    return true;
  };
  // Map image: the local map in the current frame and the previous scan.
  std::vector<float> DM(static_cast<size_t>(W) * H, 0.0f);
  const Eigen::Matrix4d Tinv = T_w.inverse();
  auto render = [&](const Eigen::Vector3d& pw) {
    const Eigen::Vector3d p = Tinv.block<3, 3>(0, 0) * pw + Tinv.block<3, 1>(0, 3);
    int k;
    if (!pixel(p, &k)) return;
    const float r = static_cast<float>(p.norm());
    if (DM[static_cast<size_t>(k)] == 0.0f || r < DM[static_cast<size_t>(k)]) DM[static_cast<size_t>(k)] = r;
  };
  for (const Tree* tree : {&map_->local_edge, &map_->local_ground, &map_->local_facade})
    for (const auto& f : tree->pts) render(f.p);
  for (const auto& f : prev_->all.pts) render(prev_pose_.block<3, 3>(0, 0) * f.p + prev_pose_.block<3, 1>(0, 3));

  std::vector<int> total(dynamic.size(), 0), flagged(dynamic.size(), 0);
  for (const auto& f : curr.all) {
    if (f.cluster < 2) continue;
    ++total[static_cast<size_t>(f.cluster)];
    int k;
    if (!pixel(f.p, &k) || DM[static_cast<size_t>(k)] == 0.0f) continue;
    const double dp = f.p.norm();
    if (std::abs(DM[static_cast<size_t>(k)] - dp) > params_.dor_relative_range * dp) ++flagged[static_cast<size_t>(f.cluster)];
  }
  for (size_t c = 2; c < dynamic.size(); ++c)
    if (total[c] > 0 && flagged[c] >= params_.dor_object_ratio * total[c]) dynamic[c] = 1;
  return dynamic;
}

void InTenLoamPaper::insert(const PaperFrame& curr, const Eigen::Matrix4d& T_w,
                            const std::vector<char>& dynamic) {
  const Eigen::Matrix3d R = T_w.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T_w.block<3, 1>(0, 3);
  auto put = [&](Map::Grid* grid, double voxel, const std::vector<PaperFeature>& fs) {
    for (const auto& f : fs) {
      if (f.cluster >= 0 && static_cast<size_t>(f.cluster) < dynamic.size() && dynamic[static_cast<size_t>(f.cluster)]) continue;
      const Eigen::Vector3d p = R * f.p + t;
      const VoxelKey key{static_cast<int>(std::floor(p.x() / voxel)), static_cast<int>(std::floor(p.y() / voxel)),
                         static_cast<int>(std::floor(p.z() / voxel))};
      TemporalVoxel& v = (*grid)[key];
      v.sum += p;
      ++v.count;
      v.intensity.emplace_back(frame_, f.intensity);
      if (v.intensity.size() > 16) v.intensity.pop_front();
    }
  };
  put(&map_->edge, params_.map_edge_voxel, curr.edge);
  put(&map_->ground, params_.map_plane_voxel, curr.ground);
  put(&map_->facade, params_.map_plane_voxel, curr.facade);
  put(&map_->reflector, params_.map_intensity_voxel, curr.reflector);
}

// ---------------------------------------------------------------------------
// Pipeline
// ---------------------------------------------------------------------------

InTenLoamPaperResult InTenLoamPaper::process(const std::vector<Eigen::Vector3d>& points,
                                             const std::vector<float>& intensity,
                                             InTenLoamPaperDiagnostics* diag,
                                             InTenLoamPaperTrace* trace) {
  if (trace && diag && diag->forced_pose)
    throw std::invalid_argument("Passive InTEn trace cannot be combined with forced_pose");
  if (trace) {
    *trace = InTenLoamPaperTrace{};
    trace->frame_index = frame_;
  }
  InTenLoamPaperResult result;
  std::vector<Eigen::Vector3d> deskewed;
  if (params_.deskew != 0 && frame_ > 1) {
    // Sensor pose at scan fraction tau from the reference: Exp(tau xi) with
    // xi the previous relative motion.
    const Eigen::AngleAxisd aa(Eigen::Matrix3d(last_rel_.block<3, 3>(0, 0)));
    const Eigen::Vector3d t = last_rel_.block<3, 1>(0, 3);
    deskewed.reserve(points.size());
    for (const auto& p : points) {
      const double az = std::atan2(p.y(), p.x());  // (-pi, pi], 0 forward
      const double tau = params_.deskew * az / (2.0 * M_PI);
      deskewed.push_back(Eigen::AngleAxisd(tau * aa.angle(), aa.axis()) * p + tau * t);
    }
  }
  const PaperFrame frame = extractFeatures(deskewed.empty() ? points : deskewed, intensity);
  result.num_ground = frame.ground.size();
  result.num_facade = frame.facade.size();
  result.num_edge = frame.edge.size();
  result.num_reflector = frame.reflector.size();

  auto index = [this](const PaperFrame& f) {
    auto ix = std::make_shared<Index>();
    ix->ground.pts = f.ground;
    ix->facade.pts = f.facade;
    ix->edge.pts = f.edge;
    ix->all.pts = f.all;
    if (params_.s2s_row_support) {
      ix->ground_rows = std::make_unique<detail::PlaneRowSupport>();
      ix->facade_rows = std::make_unique<detail::PlaneRowSupport>();
      ix->ground_rows->build(f.ground);
      ix->facade_rows->build(f.facade);
    }
    // all.pts is rendered by DOR, never searched. The legacy path also
    // builds its tree; the opt-in path avoids that unused index entirely.
#pragma omp parallel sections if(params_.accelerated)
    {
#pragma omp section
      { ix->ground.build(params_.accelerated); }
#pragma omp section
      { ix->facade.build(params_.accelerated); }
#pragma omp section
      { ix->edge.build(params_.accelerated); }
#pragma omp section
      {
        if (!params_.accelerated) ix->all.build();
        if (params_.use_intensity)
          ix->intensity.build(f.all, params_.intensity_knot, params_.s2s_intensity_normalized);
      }
    }
      return ix;
  };
  // Local map around `center` from the voxel grids (TVF output).
  auto buildLocal = [&](const Eigen::Vector3d& center) {
    const double r2 = params_.map_radius * params_.map_radius;
    auto fill = [&](const Map::Grid& grid, Tree* tree, bool build_tree) {
      tree->pts.clear();
      for (const auto& kv : grid) {
        const TemporalVoxel& v = kv.second;
        const Eigen::Vector3d p = v.sum / v.count;
        if ((p - center).squaredNorm() > r2) continue;
        PaperFeature f;
        f.p = p;
        double s = 0;
        int n = 0;
        for (const auto& e : v.intensity)
          if (frame_ - e.first < params_.temporal_window) { s += e.second; ++n; }
        f.intensity = n > 0 ? static_cast<float>(s / n) : v.intensity.back().second;
        tree->pts.push_back(f);
      }
      if (build_tree) tree->build(params_.accelerated);
    };
    const Map::Grid* grids[] = {&map_->edge, &map_->ground, &map_->facade, &map_->reflector};
    Tree* trees[] = {&map_->local_edge, &map_->local_ground, &map_->local_facade, &map_->local_reflector};
#pragma omp parallel for if(params_.accelerated) schedule(static)
    for (int c = 0; c < 4; ++c) {
      // Reflector positions feed the spline grid, never a nearest-neighbour
      // query. Retain exactly the original voxel-iteration order and values.
      fill(*grids[c], trees[c], !params_.accelerated || c != 3);
    }
      if (params_.use_intensity) map_->local_intensity.build(map_->local_reflector.pts, params_.intensity_knot);
  };

  const bool forced = diag && diag->forced_pose;
  if (diag)
    for (int c = 0; c < 4; ++c) {
      diag->s2s_H[c].setZero(); diag->s2m_H[c].setZero();
      diag->s2s_g[c].setZero(); diag->s2m_g[c].setZero();
      diag->s2s_n[c] = diag->s2m_n[c] = 0;
    }
  if (frame_ == 0) {
    if (forced) pose_ = odom_ = prev_pose_ = *diag->forced_pose;
    insert(frame, pose_, {});
    prev_ = index(frame);
    ++frame_;
    result.pose = pose_;
    if (trace) {
      trace->initialization = true;
      trace->pose = pose_;
    }
    return result;
  }

  Eigen::Matrix4d rel = params_.s2s_mapped_prior && params_.use_mapping ? last_mapped_rel_ : last_rel_;
  std::vector<char> facade_dynamic;
  if (params_.s2s_predicted_dor_facade && params_.use_mapping && params_.use_dor && !forced) {
    const Eigen::Matrix4d prediction = drift_ * (odom_ * rel);
    facade_dynamic = dynamicMask(frame, prediction);
    if (trace) {
      trace->s2s_dor_called = true;
      for (const auto& f : frame.facade)
        if (f.cluster >= 0 && static_cast<size_t>(f.cluster) < facade_dynamic.size() &&
            facade_dynamic[static_cast<size_t>(f.cluster)]) ++trace->s2s_dor_masked_facades;
    }
  }
  if (forced) {
    // S2S from the forced relative motion; its normal equations there.
    rel = prev_pose_.inverse() * *diag->forced_pose;
    ClassSystems cs;
    solveS2S(frame, &rel, &cs);
    diag->s2s_estimate = rel;
    for (int c = 0; c < 4; ++c) {
      diag->s2s_H[c] = cs.cls[c].H; diag->s2s_g[c] = cs.cls[c].g; diag->s2s_n[c] = cs.cls[c].n;
    }
    rel = prev_pose_.inverse() * *diag->forced_pose;
    result.s2s_pairs = cs.cls[0].n + cs.cls[1].n + cs.cls[2].n + cs.cls[3].n;
  } else {
    if (trace) {
      trace->s2s.called = true;
      trace->s2s.input = rel;
    }
    // Preserve the original joint solve, including its unmasked estimated R.
    const Eigen::Matrix4d consensus_seed = rel;
    result.s2s_pairs = solveS2S(frame, &rel, nullptr, trace ? &trace->s2s : nullptr,
                              facade_dynamic.empty() ? nullptr : &facade_dynamic);
    if (trace) {
      trace->s2s.output = rel;
      trace->s2s.returned_pairs = result.s2s_pairs;
      trace->s2s_fallback = result.s2s_pairs < 30;
    }
    if (params_.s2s_geometry_consensus && result.s2s_pairs >= 30 && rel.allFinite()) {
      auto consensus = detail::selectTranslationConsensus(detail::clusterTranslationProfiles(
          prev_->all.pts, frame, consensus_seed, params_.accelerated));
      consensus.seed = consensus_seed;
      consensus.original_estimate = rel;
      if (consensus.usable) {
        Eigen::Matrix4d refined = consensus_seed;
        Eigen::Vector3d direction = consensus_seed.block<3, 1>(0, 3);
        if (direction.norm() > 0.) direction.normalize();
        refined.block<3, 1>(0, 3) += ((consensus.best_index - 60) * .05) * direction;
        refined.block<3, 3>(0, 0) = rel.block<3, 3>(0, 0);
        std::vector<char> mask = facade_dynamic;
        for (int cluster : consensus.excluded_clusters) {
          if (static_cast<size_t>(cluster) >= mask.size()) mask.resize(cluster + 1, false);
          mask[cluster] = true;
        }
        auto* observation = trace ? &consensus.refinement : nullptr;
        if (observation) {
          observation->called = true;
          observation->input = refined;
        }
        const int count = solveS2S(frame, &refined, nullptr, observation,
                                  mask.empty() ? nullptr : &mask, true);
        if (observation) {
          observation->output = refined;
          observation->returned_pairs = count;
        }
        if (count >= 30 && refined.allFinite()) {
          rel = refined;
          result.s2s_pairs = count;
          consensus.applied = true;
        }
      }
      if (trace) trace->consensus = std::move(consensus);
    }
    if (result.s2s_pairs < 30) rel = last_rel_;
  }
  last_rel_ = rel;
  odom_ = odom_ * rel;
  pose_ = drift_ * odom_;
  if (trace) {
    trace->accepted_relative = rel;
    trace->s2m_prediction = pose_;
  }

  if (params_.use_mapping && frame_ % std::max(1, params_.s2m_interval) == 0) {
    buildLocal(pose_.block<3, 1>(0, 3));
    Eigen::Matrix4d T = pose_;
    if (forced) {
      ClassSystems cs;
      T = *diag->forced_pose;
      result.s2m_pairs = solveS2M(frame, &T, result.s2m_class_pairs, &cs);
      diag->s2m_estimate = T;
      for (int c = 0; c < 4; ++c) {
        diag->s2m_H[c] = cs.cls[c].H; diag->s2m_g[c] = cs.cls[c].g; diag->s2m_n[c] = cs.cls[c].n;
      }
      T = *diag->forced_pose;
    } else {
      if (trace) {
        trace->s2m.called = true;
        trace->s2m.input = T;
      }
      result.s2m_pairs = solveS2M(frame, &T, result.s2m_class_pairs, nullptr, trace ? &trace->s2m : nullptr);
      if (trace) {
        trace->s2m.output = T;
        trace->s2m.returned_pairs = result.s2m_pairs;
        trace->s2m_accepted = result.s2m_pairs >= 30;
      }
    }
    if (result.s2m_pairs >= 30) {
      pose_ = T;
      drift_ = pose_ * odom_.inverse();
    }
    std::vector<char> dynamic;
    if (params_.use_dor) {
      dynamic = dynamicMask(frame, pose_);
      for (const auto& f : frame.all)
        if (f.cluster >= 2 && dynamic[static_cast<size_t>(f.cluster)]) ++result.dor_removed;
    }
    insert(frame, pose_, dynamic);
    if (frame_ % 50 == 0) {
      const double r2 = 2.25 * params_.map_radius * params_.map_radius;
      const Eigen::Vector3d c = pose_.block<3, 1>(0, 3);
      for (Map::Grid* grid : {&map_->edge, &map_->ground, &map_->facade, &map_->reflector})
        for (auto it = grid->begin(); it != grid->end();)
          it = (it->second.sum / it->second.count - c).squaredNorm() > r2 ? grid->erase(it) : std::next(it);
    }
  }

  if (forced) {
    pose_ = *diag->forced_pose;
    drift_ = pose_ * odom_.inverse();
  }
  prev_ = index(frame);
  // Only the next normal S2S seed changes. Keep the accepted S2S cache for
  // the existing low-count fallback and deskew, including after rejection.
  if (params_.s2s_mapped_prior && params_.use_mapping && !forced)
    last_mapped_rel_ = detail::mappedMotionPrior(prev_pose_, pose_);
  else
    last_mapped_rel_ = last_rel_;
  prev_pose_ = pose_;
  ++frame_;
  result.pose = pose_;
  if (trace) trace->pose = pose_;
  return result;
}

}  // namespace inten_loam
}  // namespace localization_zoo
