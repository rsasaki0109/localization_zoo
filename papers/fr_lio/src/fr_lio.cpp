#include "fr_lio/fr_lio.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace localization_zoo {
namespace fr_lio {

namespace {

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0, -v.z(), v.y(), v.z(), 0, -v.x(), -v.y(), v.x(), 0;
  return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& w) {
  const double t = w.norm();
  if (t < 1e-10) return Eigen::Matrix3d::Identity() + skew(w);
  const Eigen::Matrix3d K = skew(w / t);
  return Eigen::Matrix3d::Identity() + std::sin(t) * K +
         (1.0 - std::cos(t)) * K * K;
}

Eigen::Matrix3d slerpSO3(const Eigen::Matrix3d& a, const Eigen::Matrix3d& b,
                         double t) {
  const Eigen::Quaterniond qa(a);
  const Eigen::Quaterniond qb(b);
  return qa.slerp(std::clamp(t, 0.0, 1.0), qb).toRotationMatrix();
}

std::vector<Eigen::Vector3d> transformPoints(
    const std::vector<Eigen::Vector3d>& pts, const Eigen::Matrix4d& T) {
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d tr = T.block<3, 1>(0, 3);
  std::vector<Eigen::Vector3d> out(pts.size());
  for (size_t i = 0; i < pts.size(); ++i) out[i] = R * pts[i] + tr;
  return out;
}

double stddevNorm(const std::vector<double>& xs) {
  if (xs.size() < 2) return 0.0;
  double mean = 0.0;
  for (double x : xs) mean += x;
  mean /= static_cast<double>(xs.size());
  double var = 0.0;
  for (double x : xs) {
    const double d = x - mean;
    var += d * d;
  }
  return std::sqrt(var / static_cast<double>(xs.size() - 1));
}

struct VoxelHash {
  std::size_t operator()(const Eigen::Vector3i& v) const {
    std::size_t seed = 0;
    auto hc = [&seed](int val) {
      seed ^= std::hash<int>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    hc(v.x());
    hc(v.y());
    hc(v.z());
    return seed;
  }
};

}  // namespace

// ============================================================
// RC-Vox
// ============================================================

RCVoxelMap::RCVoxelMap(double lidar_range, double grid_size, double voxel_size,
                       double lambda, int max_points_per_voxel)
    : lidar_range_(lidar_range),
      grid_size_(grid_size),
      voxel_size_(voxel_size),
      lambda_(lambda),
      max_points_per_voxel_(max_points_per_voxel),
      tla_dim_(std::max(4, static_cast<int>(std::ceil(
                               2.0 * lambda_ * lidar_range_ / grid_size_)))),
      bla_dim_(std::max(1, static_cast<int>(std::round(grid_size_ / voxel_size_)))) {}

void RCVoxelMap::clear() { grids_.clear(); }

int RCVoxelMap::wrapIndex(int v) const {
  v %= tla_dim_;
  if (v < 0) v += tla_dim_;
  return v;
}

Eigen::Vector3i RCVoxelMap::wrapVec(const Eigen::Vector3i& v) const {
  return Eigen::Vector3i(wrapIndex(v.x()), wrapIndex(v.y()), wrapIndex(v.z()));
}

int64_t RCVoxelMap::gridKey(const Eigen::Vector3i& it) const {
  const Eigen::Vector3i w = wrapVec(it);
  return (static_cast<int64_t>(w.x()) * tla_dim_ + w.y()) * tla_dim_ + w.z();
}

int RCVoxelMap::voxelKey(const Eigen::Vector3i& ib) const {
  return (ib.x() * bla_dim_ + ib.y()) * bla_dim_ + ib.z();
}

void RCVoxelMap::initialize(const Eigen::Vector3d& robot_position) {
  clear();
  t_init_ = robot_position - Eigen::Vector3d::Constant(lambda_ * lidar_range_);
  map_origin_tla_ = Eigen::Vector3i::Zero();
}

void RCVoxelMap::updateRobot(const Eigen::Vector3d& robot_position) {
  const Eigen::Vector3d rel = robot_position - t_init_;
  map_origin_tla_ = wrapVec((rel / grid_size_).cast<int>());
}

Eigen::Vector3i RCVoxelMap::tlaIndexForPoint(const Eigen::Vector3d& p) const {
  const Eigen::Vector3i rel = ((p - t_init_) / grid_size_).cast<int>();
  return wrapVec(rel);
}

Eigen::Vector3i RCVoxelMap::blaIndexForPoint(const Eigen::Vector3d& p,
                                             const Eigen::Vector3i& it) const {
  const Eigen::Vector3d grid_origin =
      it.cast<double>() * grid_size_ + t_init_;
  Eigen::Vector3i ib = ((p - grid_origin) / voxel_size_).cast<int>();
  ib = ib.cwiseMax(0).cwiseMin(Eigen::Vector3i::Constant(bla_dim_ - 1));
  return ib;
}

RCVoxelMap::BottomLevel& RCVoxelMap::gridAt(const Eigen::Vector3i& it) {
  auto& bl = grids_[gridKey(it)];
  if (bl.dim == 0) bl.dim = bla_dim_;
  return bl;
}

const RCVoxelMap::VoxelCell* RCVoxelMap::voxelAt(const Eigen::Vector3i& it,
                                                 const Eigen::Vector3i& ib) const {
  const auto git = grids_.find(gridKey(it));
  if (git == grids_.end()) return nullptr;
  const auto vit = git->second.voxels.find(voxelKey(ib));
  if (vit == git->second.voxels.end()) return nullptr;
  return &vit->second;
}

void RCVoxelMap::insertIntoVoxel(BottomLevel& bl, const Eigen::Vector3i& ib,
                                 const Eigen::Vector3d& p,
                                 bool cache_neighbors) {
  auto& cell = bl.voxels[voxelKey(ib)];
  if (static_cast<int>(cell.points.size()) < max_points_per_voxel_) {
    cell.points.push_back(p);
  }
  if (!cache_neighbors) return;
  static const Eigen::Vector3i dirs[6] = {{1, 0, 0},  {-1, 0, 0}, {0, 1, 0},
                                          {0, -1, 0}, {0, 0, 1},  {0, 0, -1}};
  for (const auto& d : dirs) {
    const Eigen::Vector3i nb = ib + d;
    if ((nb.array() < 0).any() || (nb.array() >= bla_dim_).any()) continue;
    auto& ncell = bl.voxels[voxelKey(nb)];
    if (static_cast<int>(ncell.neighbor_cache.size()) < max_points_per_voxel_) {
      ncell.neighbor_cache.push_back(p);
    }
  }
}

void RCVoxelMap::addPoints(const std::vector<Eigen::Vector3d>& world_points) {
  for (const auto& p : world_points) {
    const Eigen::Vector3i it = tlaIndexForPoint(p);
    BottomLevel& bl = gridAt(it);
    const Eigen::Vector3i ib = blaIndexForPoint(p, it);
    insertIntoVoxel(bl, ib, p, true);
  }
}

RCVoxelMap::PlaneMatch RCVoxelMap::matchPointToPlane(
    const Eigen::Vector3d& query, double max_dist, int k_neighbors) const {
  PlaneMatch out;
  const Eigen::Vector3i it = tlaIndexForPoint(query);
  const Eigen::Vector3i ib = blaIndexForPoint(query, it);
  const VoxelCell* cell = voxelAt(it, ib);
  if (!cell) return out;

  std::vector<std::pair<double, Eigen::Vector3d>> candidates;
  candidates.reserve(cell->points.size() + cell->neighbor_cache.size());
  const double max_dist_sq = max_dist * max_dist;
  auto collect = [&](const std::vector<Eigen::Vector3d>& pts) {
    for (const auto& pt : pts) {
      const double d2 = (pt - query).squaredNorm();
      if (d2 <= max_dist_sq) candidates.emplace_back(d2, pt);
    }
  };
  collect(cell->points);
  collect(cell->neighbor_cache);
  if (static_cast<int>(candidates.size()) < k_neighbors) return out;

  std::partial_sort(
      candidates.begin(), candidates.begin() + k_neighbors, candidates.end(),
      [](const auto& a, const auto& b) { return a.first < b.first; });

  Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  for (int i = 0; i < k_neighbors; ++i) mean += candidates[i].second;
  mean /= static_cast<double>(k_neighbors);
  for (int i = 0; i < k_neighbors; ++i) {
    const Eigen::Vector3d d = candidates[i].second - mean;
    cov += d * d.transpose();
  }
  cov /= static_cast<double>(k_neighbors);

  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
  if (es.info() != Eigen::Success) return out;
  if (es.eigenvalues()(0) > 0.05 * es.eigenvalues()(2)) return out;

  out.normal = es.eigenvectors().col(0).normalized();
  out.centroid = mean;
  out.valid = true;
  return out;
}

// ============================================================
// FR-LIO pipeline
// ============================================================

FRLIOPipeline::FRLIOPipeline(const FRLIOParams& params)
    : params_(params),
      map_(params.lidar_range, params.grid_size, params.voxel_size,
           params.map_lambda, params.max_points_per_voxel) {}

void FRLIOPipeline::setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }

int FRLIOPipeline::computeSubframeCount(
    const std::vector<imu_preintegration::ImuSample>& imu,
    const FRLIOParams& params) {
  if (imu.size() < 3) return 1;
  std::vector<double> acc_norms;
  std::vector<double> gyr_norms;
  acc_norms.reserve(imu.size());
  gyr_norms.reserve(imu.size());
  for (const auto& s : imu) {
    acc_norms.push_back(s.accel.norm());
    gyr_norms.push_back(s.gyro.norm());
  }
  const double ratio =
      std::max(stddevNorm(acc_norms) / std::max(params.sigma_acc_max, 1e-6),
               stddevNorm(gyr_norms) / std::max(params.sigma_gyr_max, 1e-6));
  const int n = static_cast<int>(std::ceil(params.max_subframes * ratio));
  return std::clamp(n, 1, params.max_subframes);
}

std::vector<Eigen::Vector3d> FRLIOPipeline::filterRange(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (r >= params_.min_range && r <= params_.max_range) out.push_back(p);
  }
  return out;
}

std::vector<Eigen::Vector3d> FRLIOPipeline::downsample(
    const std::vector<Eigen::Vector3d>& points, double leaf) const {
  std::unordered_map<Eigen::Vector3i, Eigen::Vector3d, VoxelHash> grid;
  for (const auto& p : points) {
    Eigen::Vector3i key(static_cast<int>(std::floor(p.x() / leaf)),
                        static_cast<int>(std::floor(p.y() / leaf)),
                        static_cast<int>(std::floor(p.z() / leaf)));
    grid.emplace(key, p);
  }
  std::vector<Eigen::Vector3d> out;
  out.reserve(grid.size());
  for (const auto& kv : grid) out.push_back(kv.second);
  return out;
}

std::vector<std::vector<Eigen::Vector3d>> FRLIOPipeline::splitSubframes(
    const std::vector<Eigen::Vector3d>& points, int n_sub) const {
  std::vector<std::vector<Eigen::Vector3d>> out(n_sub);
  if (points.empty()) return out;
  struct AngPt {
    double ang;
    Eigen::Vector3d p;
  };
  std::vector<AngPt> sorted;
  sorted.reserve(points.size());
  for (const auto& p : points) {
    sorted.push_back({std::atan2(p.y(), p.x()), p});
  }
  std::sort(sorted.begin(), sorted.end(),
            [](const AngPt& a, const AngPt& b) { return a.ang < b.ang; });
  for (size_t i = 0; i < sorted.size(); ++i) {
    out[i % static_cast<size_t>(n_sub)].push_back(sorted[i].p);
  }
  return out;
}

std::vector<std::vector<imu_preintegration::ImuSample>> FRLIOPipeline::splitImu(
    const std::vector<imu_preintegration::ImuSample>& imu, int n_sub) const {
  std::vector<std::vector<imu_preintegration::ImuSample>> out(n_sub);
  if (imu.empty()) return out;
  const size_t chunk = std::max<size_t>(1, imu.size() / static_cast<size_t>(n_sub));
  for (size_t i = 0; i < imu.size(); ++i) {
    out[std::min(i / chunk, static_cast<size_t>(n_sub - 1))].push_back(imu[i]);
  }
  return out;
}

Eigen::Matrix4d FRLIOPipeline::predictMotion(
    const std::vector<imu_preintegration::ImuSample>& imu, bool* used_imu) const {
  Eigen::Matrix4d motion = last_delta_;
  if (imu.size() < 2) return motion;
  imu_preintegration::ImuPreintegrator preint;
  preint.integrateBatch(imu);
  const auto& pi = preint.result();
  if (!pi.delta_R.allFinite()) return motion;
  motion = Eigen::Matrix4d::Identity();
  motion.block<3, 3>(0, 0) = pi.delta_R;
  motion.block<3, 1>(0, 3) = last_delta_.block<3, 1>(0, 3);
  if (used_imu) *used_imu = true;
  return motion;
}

Eigen::Matrix4d FRLIOPipeline::alignSubframe(
    const std::vector<Eigen::Vector3d>& points,
    const Eigen::Matrix4d& initial_guess, int* valid_matches) const {
  Eigen::Matrix4d T = initial_guess;
  int total_valid = 0;
  for (int iter = 0; iter < params_.max_icp_iterations; ++iter) {
    const auto src = transformPoints(points, T);
    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> Jtb = Eigen::Matrix<double, 6, 1>::Zero();
    int valid = 0;
    for (const auto& q : src) {
      const auto plane =
          map_.matchPointToPlane(q, params_.max_correspondence_dist, 5);
      if (!plane.valid) continue;
      const double res = plane.normal.dot(q - plane.centroid);
      Eigen::Matrix<double, 1, 6> J;
      J.block<1, 3>(0, 0) = (-plane.normal.transpose() * skew(q));
      J.block<1, 3>(0, 3) = plane.normal.transpose();
      JtJ += J.transpose() * J;
      Jtb += J.transpose() * res;
      ++valid;
    }
    total_valid = std::max(total_valid, valid);
    if (valid < params_.min_plane_points) break;
    const Eigen::Matrix<double, 6, 1> delta = JtJ.ldlt().solve(-Jtb);
    if (!delta.allFinite()) break;
    Eigen::Matrix4d dT = Eigen::Matrix4d::Identity();
    dT.block<3, 3>(0, 0) = expSO3(delta.head<3>());
    dT.block<3, 1>(0, 3) = delta.tail<3>();
    T = dT * T;
    if (delta.norm() < 1e-4) break;
  }
  if (valid_matches) *valid_matches = total_valid;
  return T;
}

void FRLIOPipeline::backwardSmooth(
    std::vector<Eigen::Matrix4d>& poses,
    const std::vector<Eigen::Matrix4d>& predictions) const {
  if (poses.size() < 2 || predictions.size() != poses.size()) return;
  std::vector<Eigen::Matrix4d> smoothed = poses;
  for (int k = static_cast<int>(poses.size()) - 2; k >= 0; --k) {
    const Eigen::Vector3d t_corr =
        params_.esks_gain *
        (smoothed[k + 1].block<3, 1>(0, 3) -
         predictions[k + 1].block<3, 1>(0, 3));
    smoothed[k].block<3, 1>(0, 3) += t_corr;
    smoothed[k].block<3, 3>(0, 0) =
        slerpSO3(smoothed[k].block<3, 3>(0, 0), smoothed[k + 1].block<3, 3>(0, 0),
                 0.5 * params_.esks_gain);
  }
  poses.swap(smoothed);
}

bool FRLIOPipeline::constraintOk(int valid_matches) const {
  return valid_matches >= params_.min_valid_matches;
}

FRLIOResult FRLIOPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame,
    const std::vector<imu_preintegration::ImuSample>& imu) {
  FRLIOResult result;
  auto filtered = filterRange(frame);
  auto down = downsample(filtered, params_.source_voxel_size);
  auto reg_pts = downsample(down, params_.voxel_size);
  if (reg_pts.empty()) {
    if (frame_count_ == 0) {
      map_.initialize(pose_.block<3, 1>(0, 3));
      frame_count_++;
      result.pose = pose_;
      result.converged = true;
    }
    return result;
  }

  if (frame_count_ == 0) {
    map_.initialize(pose_.block<3, 1>(0, 3));
    map_.addPoints(transformPoints(down, pose_));
    frame_count_++;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  map_.updateRobot(pose_.block<3, 1>(0, 3));
  const int n_sub = computeSubframeCount(imu, params_);
  result.subframes = n_sub;
  const auto sub_pts = splitSubframes(reg_pts, n_sub);
  const auto sub_imu = splitImu(imu, n_sub);

  std::vector<Eigen::Matrix4d> sub_poses;
  std::vector<Eigen::Matrix4d> predictions;
  sub_poses.reserve(static_cast<size_t>(n_sub));
  predictions.reserve(static_cast<size_t>(n_sub));

  Eigen::Matrix4d running = pose_;
  for (int s = 0; s < n_sub; ++s) {
    if (sub_pts[static_cast<size_t>(s)].empty()) continue;
    bool used = false;
    const Eigen::Matrix4d motion = predictMotion(sub_imu[static_cast<size_t>(s)], &used);
    if (used) result.used_imu = true;
    const Eigen::Matrix4d pred = running * motion;
    predictions.push_back(pred);
    int valid = 0;
    const Eigen::Matrix4d aligned =
        alignSubframe(sub_pts[static_cast<size_t>(s)], pred, &valid);
    sub_poses.push_back(aligned);
    running = aligned;
  }

  if (sub_poses.empty()) {
    result.pose = pose_;
    return result;
  }

  backwardSmooth(sub_poses, predictions);
  result.smoothed_subframes = static_cast<int>(sub_poses.size());
  result.pose = sub_poses.back();
  result.converged = true;

  int valid_last = 0;
  alignSubframe(sub_pts.back(), result.pose, &valid_last);
  result.map_updated = constraintOk(valid_last);
  if (result.map_updated) {
    last_delta_ = pose_.inverse() * result.pose;
    pose_ = result.pose;
    map_.addPoints(transformPoints(down, pose_));
  } else {
    last_delta_ = pose_.inverse() * result.pose;
    pose_ = result.pose;
  }

  frame_count_++;
  return result;
}

}  // namespace fr_lio
}  // namespace localization_zoo
