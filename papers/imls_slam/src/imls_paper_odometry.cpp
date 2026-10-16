#include "imls_slam/imls_paper_odometry.h"

#include <Eigen/Dense>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <numeric>
#include <thread>
#include <unordered_set>

namespace localization_zoo {
namespace imls_slam {

namespace {

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0, -v.z(), v.y(), v.z(), 0, -v.x(), -v.y(), v.x(), 0;
  return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& w) {
  const double theta = w.norm();
  if (theta < 1e-12) return Eigen::Matrix3d::Identity() + skew(w);
  const Eigen::Matrix3d K = skew(w / theta);
  return Eigen::Matrix3d::Identity() + std::sin(theta) * K + (1.0 - std::cos(theta)) * K * K;
}

}  // namespace

ImlsPaperOdometry::ImlsPaperOdometry(const ImlsPaperParams& params) : params_(params) {}

Eigen::Vector3i ImlsPaperOdometry::key(const Eigen::Vector3d& p, double cell) const {
  return (p / cell).array().floor().cast<int>();
}

std::vector<ImlsPaperOdometry::ScanPoint> ImlsPaperOdometry::describe(
    const std::vector<Eigen::Vector3d>& scan) const {
  const double cell = params_.scan_normal_radius;
  Grid grid;
  std::vector<Eigen::Vector3d> pts;
  pts.reserve(scan.size());
  for (const auto& p : scan) {
    const double range = p.norm();
    if (range < params_.min_range || range > params_.max_range) continue;
    pts.push_back(p);
    grid[key(p, cell)].push_back(p);
  }
  const double r2 = cell * cell;
  using Neighbors = std::array<const std::vector<Eigen::Vector3d>*, 27>;
  std::unordered_map<Eigen::Vector3i, Neighbors, KeyHash> neighbor_cells;
  if (params_.accelerated) {
    neighbor_cells.reserve(grid.size());
    for (const auto& entry : grid) {
      Neighbors neighbors{};
      int i = 0;
      for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
          for (int dz = -1; dz <= 1; ++dz) {
            const auto it = grid.find(entry.first + Eigen::Vector3i(dx, dy, dz));
            neighbors[i++] = it == grid.end() ? nullptr : &it->second;
          }
      neighbor_cells.emplace(entry.first, neighbors);
    }
  }
  std::vector<ScanPoint> out;
  out.reserve(pts.size());
  auto describe_point = [&](const Eigen::Vector3d& p, std::vector<Eigen::Vector3d>& nb,
                            ScanPoint* result) {
    nb.clear();
    const Eigen::Vector3i c = key(p, cell);
    if (params_.accelerated) {
      // Cell topology is shared by every point in the cell. Its order is
      // exactly the legacy dx/dy/dz order, including the neighbour cap.
      for (const auto* bin : neighbor_cells.find(c)->second) {
        if (static_cast<int>(nb.size()) >= params_.scan_normal_max_neighbors) break;
        if (!bin) continue;
        for (const auto& q : *bin) {
          if ((q - p).squaredNorm() <= r2) nb.push_back(q);
          if (static_cast<int>(nb.size()) >= params_.scan_normal_max_neighbors) break;
        }
      }
    } else {
      for (int dx = -1; dx <= 1 && static_cast<int>(nb.size()) < params_.scan_normal_max_neighbors; ++dx)
        for (int dy = -1; dy <= 1 && static_cast<int>(nb.size()) < params_.scan_normal_max_neighbors; ++dy)
          for (int dz = -1; dz <= 1; ++dz) {
            auto it = grid.find(c + Eigen::Vector3i(dx, dy, dz));
            if (it == grid.end()) continue;
            for (const auto& q : it->second) {
              if ((q - p).squaredNorm() <= r2) nb.push_back(q);
              if (static_cast<int>(nb.size()) >= params_.scan_normal_max_neighbors) break;
            }
            if (static_cast<int>(nb.size()) >= params_.scan_normal_max_neighbors) break;
          }
    }
    if (static_cast<int>(nb.size()) < params_.min_normal_points) return false;
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    for (const auto& q : nb) mean += q;
    mean /= static_cast<double>(nb.size());
    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    for (const auto& q : nb) cov += (q - mean) * (q - mean).transpose();
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov / static_cast<double>(nb.size()));
    const Eigen::Vector3d ev = solver.eigenvalues().cwiseMax(0.0);  // ascending
    const double s1 = std::sqrt(ev(2)), s2 = std::sqrt(ev(1)), s3 = std::sqrt(ev(0));
    if (s1 <= 1e-12) return false;
    ScanPoint sp;
    sp.p = p;
    sp.n = solver.eigenvectors().col(0);
    sp.a2d = (s2 - s3) / s1;
    *result = sp;
    return true;
  };
  if (params_.accelerated) {
    std::vector<ScanPoint> described(pts.size());
    std::vector<char> valid(pts.size(), 0);
#pragma omp parallel
    {
      std::vector<Eigen::Vector3d> nb;
#pragma omp for schedule(static)
      for (long i = 0; i < static_cast<long>(pts.size()); ++i)
        valid[i] = describe_point(pts[i], nb, &described[i]);
    }
    // Preserve input order, including which points have insufficient support.
    for (size_t i = 0; i < pts.size(); ++i)
      if (valid[i]) out.push_back(described[i]);
  } else {
    std::vector<Eigen::Vector3d> nb;
    for (const auto& p : pts) {
      ScanPoint sp;
      if (describe_point(p, nb, &sp)) out.push_back(sp);
    }
  }
  return out;
}

std::vector<std::vector<int>> ImlsPaperOdometry::sampleLists(
    const std::vector<ScanPoint>& points) const {
  std::vector<std::vector<int>> lists(9);
  std::vector<std::vector<double>> scores(9, std::vector<double>(points.size()));
#pragma omp parallel for if(params_.accelerated) schedule(static)
  for (size_t i = 0; i < points.size(); ++i) {
    const auto& sp = points[i];
    const double w = sp.a2d * sp.a2d;
    const Eigen::Vector3d torque = sp.p.cross(sp.n);
    for (int a = 0; a < 3; ++a) {
      scores[2 * a][i] = w * torque(a);
      scores[2 * a + 1][i] = -w * torque(a);
      scores[6 + a][i] = w * std::abs(sp.n(a));
    }
  }
#pragma omp parallel for if(params_.accelerated) schedule(static)
  for (int l = 0; l < 9; ++l) {
    lists[l].resize(points.size());
    std::iota(lists[l].begin(), lists[l].end(), 0);
    std::sort(lists[l].begin(), lists[l].end(),
              [&](int a, int b) { return scores[l][a] > scores[l][b]; });
  }
  return lists;
}

bool ImlsPaperOdometry::hasModelPointWithin(const Eigen::Vector3d& x, double r,
                                           const ModelCells* cells) const {
  if (cells) {
    for (const auto* bin : *cells)
      for (const auto& e : *bin)
        if ((e.p - x).squaredNorm() <= r * r) return true;
    return false;
  }
  // The model grid has cells of params_.r; a larger radius visits more cells.
  const Eigen::Vector3i c = key(x, params_.r);
  const double r2 = r * r;
  const int reach = std::max(1, static_cast<int>(std::ceil(r / params_.r)));
  for (int dx = -reach; dx <= reach; ++dx)
    for (int dy = -reach; dy <= reach; ++dy)
      for (int dz = -reach; dz <= reach; ++dz) {
        auto it = model_.find(c + Eigen::Vector3i(dx, dy, dz));
        if (it == model_.end()) continue;
        for (const auto& e : it->second)
          if ((e.p - x).squaredNorm() <= r2) return true;
      }
  return false;
}

bool ImlsPaperOdometry::imls(const Eigen::Vector3d& x, double r, double h,
                             Eigen::Vector3d* normal, double* distance) const {
  return project(x, r, h, normal, distance, nullptr);
}

bool ImlsPaperOdometry::project(const Eigen::Vector3d& x, double r, double h,
                               Eigen::Vector3d* normal, double* distance,
                               const ModelCells* cells) const {
  const Eigen::Vector3i c = key(x, params_.r);
  const double r2 = r * r;
  const double inv_h2 = 1.0 / (h * h);
  const int reach = std::max(1, static_cast<int>(std::ceil(r / params_.r)));
  Eigen::Vector3d sum = Eigen::Vector3d::Zero();
  Eigen::Matrix3d sum_sq = Eigen::Matrix3d::Zero();
  Eigen::Vector3d weighted = Eigen::Vector3d::Zero();
  double weight_sum = 0.0;
  int n = 0;
  auto accumulate = [&](const std::vector<Entry>& bin) {
    for (const auto& e : bin) {
      const Eigen::Vector3d d = e.p - x;
      const double d2 = d.squaredNorm();
      if (d2 > r2) continue;
      ++n;
      sum += d;  // relative to x for conditioning
      sum_sq += d * d.transpose();
      const double w = std::exp(-d2 * inv_h2);
      weighted += w * d;
      weight_sum += w;
    }
  };
  if (cells) {
    for (const auto* bin : *cells) accumulate(*bin);
  } else {
    for (int dx = -reach; dx <= reach; ++dx)
      for (int dy = -reach; dy <= reach; ++dy)
        for (int dz = -reach; dz <= reach; ++dz) {
          auto it = model_.find(c + Eigen::Vector3i(dx, dy, dz));
          if (it != model_.end()) accumulate(it->second);
        }
  }
  if (n < params_.min_normal_points || weight_sum <= 0.0) return false;
  const Eigen::Vector3d mean = sum / n;
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(sum_sq / n - mean * mean.transpose());
  *normal = solver.eigenvectors().col(0);
  // I(x) = sum w (x - p).n / sum w = -n.(weighted mean of p - x).
  *distance = -normal->dot(weighted / weight_sum);
  return true;
}

void ImlsPaperOdometry::insertScan(const std::vector<Eigen::Vector3d>& scan,
                                   const Eigen::Matrix4d& pose) {
  const Eigen::Matrix3d R = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d t = pose.block<3, 1>(0, 3);
  const int id = next_scan_id_++;
  std::unordered_set<Eigen::Vector3i, KeyHash> cells;
  for (const auto& p : scan) {
    const double range = p.norm();
    if (range < params_.min_range || range > params_.max_range) continue;
    const Eigen::Vector3d w = R * p + t;
    const Eigen::Vector3i k = key(w, params_.r);
    model_[k].push_back({id, w});
    cells.insert(k);
    ++model_points_;
  }
  window_.emplace_back(id, std::vector<Eigen::Vector3i>(cells.begin(), cells.end()));
  while (static_cast<int>(window_.size()) > params_.window_scans) {
    const int old = window_.front().first;
    for (const auto& k : window_.front().second) {
      auto it = model_.find(k);
      if (it == model_.end()) continue;
      auto& v = it->second;
      const size_t before = v.size();
      v.erase(std::remove_if(v.begin(), v.end(), [old](const Entry& e) { return e.scan_id == old; }),
              v.end());
      model_points_ -= before - v.size();
      if (v.empty()) model_.erase(it);
    }
    window_.pop_front();
  }
}

int ImlsPaperOdometry::iterate(const std::vector<ScanPoint>& points,
                               const std::vector<std::vector<int>>& lists, double r, double h,
                               int iterations, bool count_only, Eigen::Matrix4d* T) const {
  struct Projection {
    std::atomic<unsigned char> state{0};  // empty, computing, ready
    bool valid = false;
    Eigen::Vector3d x, n;
    double distance = 0.0;
  };
  std::vector<Projection> cache(params_.accelerated ? points.size() : 0);
  // Each list owns a cache across iterations. The model is immutable for
  // this call; a new coarse/fine call or a new frame starts with empty caches.
  // Missing cells are cached too, and populated cells retain dx/dy/dz order.
  using CellCache = std::unordered_map<Eigen::Vector3i, ModelCells, KeyHash>;
  std::vector<CellCache> neighborhoods(params_.accelerated ? lists.size() : 0);
  const int reach = std::max(1, static_cast<int>(std::ceil(r / params_.r)));
  int samples = 0;
  for (int iter = 0; iter < iterations; ++iter) {
    const Eigen::Matrix3d R = T->block<3, 3>(0, 0);
    const Eigen::Vector3d t = T->block<3, 1>(0, 3);
    Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> g = Eigen::Matrix<double, 6, 1>::Zero();
    samples = 0;
    if (params_.accelerated) {
      for (auto& entry : cache) entry.state.store(0, std::memory_order_relaxed);
      std::vector<std::vector<int>> accepted(lists.size());
      // Each list has the same stopping rule. Repeated indices share one
      // projection, including failed queries, without dropping duplicate
      // residuals from the nine lists. The model is immutable in this stage.
#pragma omp parallel for schedule(dynamic, 1)
      for (long l = 0; l < static_cast<long>(lists.size()); ++l) {
        auto& kept = accepted[l];
        for (int idx : lists[l]) {
          if (static_cast<int>(kept.size()) >= params_.samples_per_list) break;
          Projection& entry = cache[idx];
          unsigned char empty = 0;
          if (entry.state.compare_exchange_strong(empty, 1, std::memory_order_acquire)) {
            entry.x = R * points[idx].p + t;
            const Eigen::Vector3i c = key(entry.x, params_.r);
            auto [cell, inserted] = neighborhoods[l].try_emplace(c);
            if (inserted) {
              for (int dx = -reach; dx <= reach; ++dx)
                for (int dy = -reach; dy <= reach; ++dy)
                  for (int dz = -reach; dz <= reach; ++dz) {
                    const auto it = model_.find(c + Eigen::Vector3i(dx, dy, dz));
                    if (it != model_.end()) cell->second.push_back(&it->second);
                  }
            }
            entry.valid = hasModelPointWithin(entry.x, r, &cell->second) &&
                          project(entry.x, r, h, &entry.n, &entry.distance, &cell->second);
            entry.state.store(2, std::memory_order_release);
          } else {
            while (entry.state.load(std::memory_order_acquire) != 2) std::this_thread::yield();
          }
          if (entry.valid) kept.push_back(idx);
        }
      }
      // Serial reduction matches the legacy list/point accumulation order.
      for (const auto& kept : accepted) {
        for (int idx : kept) {
          const Projection& entry = cache[idx];
          Eigen::Matrix<double, 6, 1> J;
          J.head<3>() = (entry.x - t).cross(entry.n);
          J.tail<3>() = entry.n;
          H += J * J.transpose();
          g += J * entry.distance;
        }
        samples += static_cast<int>(kept.size());
      }
    } else {
      for (const auto& list : lists) {
        int accepted = 0;
        for (int idx : list) {
          if (accepted >= params_.samples_per_list) break;
          const Eigen::Vector3d x = R * points[idx].p + t;
          if (!hasModelPointWithin(x, r)) continue;
          Eigen::Vector3d n;
          double I;
          if (!imls(x, r, h, &n, &I)) continue;
          ++accepted;
          // Residual n.(x' - y) with y = x - I n, linearised about the sensor
          // position: n.(x + w x (x - t) + v - y) = I + ((x - t) x n).w + n.v.
          Eigen::Matrix<double, 6, 1> J;
          J.head<3>() = (x - t).cross(n);
          J.tail<3>() = n;
          H += J * J.transpose();
          g += J * I;
        }
        samples += accepted;
      }
    }
    if (count_only || samples < 6) break;
    const Eigen::Matrix<double, 6, 1> delta = H.ldlt().solve(-g);
    if (!delta.allFinite()) break;
    const Eigen::Matrix3d dR = expSO3(delta.head<3>());
    T->block<3, 3>(0, 0) = dR * R;
    T->block<3, 1>(0, 3) = t + delta.tail<3>();
  }
  return samples;
}

ImlsPaperResult ImlsPaperOdometry::process(const std::vector<Eigen::Vector3d>& scan) {
  ImlsPaperResult result;
  if (frame_ == 0) {
    insertScan(scan, pose_);
    previous_pose_ = pose_;
    ++frame_;
    result.pose = pose_;
    result.model_points = model_points_;
    return result;
  }

  const std::vector<ScanPoint> points = describe(scan);
  const std::vector<std::vector<int>> lists = sampleLists(points);

  // Constant velocity: tau(k-1) tau(k-2)^-1 tau(k-1).
  Eigen::Matrix4d T = pose_ * (previous_pose_.inverse() * pose_);
  if (frame_ <= params_.coarse_frames && params_.coarse_iterations > 0) {
    iterate(points, lists, params_.coarse_r, params_.coarse_h, params_.coarse_iterations, false, &T);
    result.coarse_start = true;
  }
  result.samples = iterate(points, lists, params_.r, params_.h, params_.iterations, false, &T);

  previous_pose_ = pose_;
  pose_ = T;
  insertScan(scan, pose_);
  ++frame_;
  result.pose = pose_;
  result.model_points = model_points_;
  return result;
}

}  // namespace imls_slam
}  // namespace localization_zoo
