// GT-free window-odometry runner for RELEAD's constrained-ESIKF frontend
// (papers/relead), with optional IMU support.
//
// Structurally this mirrors evaluation/src/xicp_window_odometry.cpp: same
// CLI shape, same dogfooding-directory PCD loading, same local voxel-map +
// KdTree correspondence building, same step/refinement gate contract, and
// the same poses_xyyaw / pairs JSON output shape. RELEAD's public API
// (papers/relead/include/relead/cesikf.h) differs from X-ICP's in two
// important ways that this runner has to account for:
//
//   1. CESIKF owns an internal 18-dim error state (R, t, v, b_g, b_a, g),
//      not just a pose. predict(imu, dt) advances that state from an IMU
//      measurement; update(correspondences) runs the constrained-ESIKF
//      LiDAR correction and mutates the state in place -- there is no
//      "try and discard" API, only a mutable state() accessor.
//   2. There is no dedicated "runRELEAD" integration in pcd_dogfooding.cpp
//      to mirror (grep confirms only ros2/localization_zoo_ros/src/
//      relead_node.cpp uses CESIKF, and it feeds dummy ground-plane
//      correspondences, not a real map). This runner therefore builds
//      real point-to-plane correspondences against a local world-frame
//      voxel map the same way xicp_window_odometry.cpp does (KdTree
//      nearest-neighbor + neighborhood-covariance normal), and feeds
//      real IMU samples from imu.csv the same way
//      degen_sense_window_odometry.cpp does.
//
// IMU handling: when imu.csv + frame_timestamps.csv are available and
// --no-imu is not passed, every IMU sample in (t_{i-1}, t_i] is fed to
// filter_.predict() sequentially (dt between consecutive samples), which
// advances the filter's own R/t/v prediction before the LiDAR update for
// frame i. When IMU is unavailable (or --no-imu), the seed pose is instead
// written directly into filter_.state() from a constant-velocity
// extrapolation of the last two accepted poses -- the same
// velocityModelPrediction() used by xicp_window_odometry.cpp -- since
// CESIKF exposes a mutable State& for exactly this kind of external
// seeding.
//
// Step/refinement gating and map bookkeeping follow xicp_window_odometry.cpp:
// a rejected pair does not advance T_prev_world/T_prev_prev_world and does
// not add points to the map. Because CESIKF's update() already wrote the
// LiDAR-corrected pose into its internal state (and predict() already
// inflated its internal covariance), a reject additionally calls
// filter_.reset() and then writes back a State snapshot taken after the
// last *accepted* update (full State: rotation, position, velocity,
// biases, gravity). Two earlier versions of this runner under-rolled-back
// on reject and both diverged on tunnel_full real data:
//   1. Resetting only rotation/position (not velocity/bias) let a single
//      bad ESIKF update poison the velocity estimate, which kept driving
//      position drift through predict() on every subsequent frame.
//   2. Restoring the full State but leaving CESIKF's internal covariance
//      P_ untouched (there is no public covariance mutator, only reset())
//      let repeated predict() calls across rejected frames keep inflating
//      P_ with fresh process noise every IMU sample, without the
//      counterbalancing shrinkage an *accepted* update's (I-KH)P step
//      would normally apply. An inflated P_ makes the next LiDAR update's
//      Kalman gain overconfident in the measurement, producing large
//      single-shot pose corrections (observed: ~11 deg yaw swings per
//      frame) even though the real VN100 gyro data integrates to <0.05
//      deg of rotation per 0.1 s frame (checked directly against imu.csv).
//      filter_.reset() clears P_ back to its tight 0.001*I prior, which
//      this runner then treats as consistent with "we trust the last
//      accepted pose," since state_ is immediately overwritten with that
//      pose right after.

#include "relead/cesikf.h"

#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Geometry>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::high_resolution_clock;
using localization_zoo::relead::CESIKF;
using localization_zoo::relead::CESIKFParams;
using localization_zoo::relead::CESIKFResult;
using localization_zoo::relead::DegeneracyInfo;
using localization_zoo::relead::ImuMeasurement;
using localization_zoo::relead::Localizability;
using localization_zoo::relead::PointWithNormal;

namespace {

// RELEAD requires at least this many point-to-plane correspondences before
// an ESIKF update is trusted. Mirrors kMinCorrespondences in
// xicp_window_odometry.cpp.
constexpr size_t kMinCorrespondences = 64;

struct FrameEntry {
  int global_index = 0;
  fs::path path;
};

struct ImuSampleCsv {
  double timestamp = 0.0;
  Eigen::Vector3d gyro = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel = Eigen::Vector3d::Zero();
};

struct PairLog {
  int index = 0;
  Eigen::Matrix4d raw = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d used = Eigen::Matrix4d::Identity();
  bool accepted = false;
  bool converged = false;
  bool step_gate = false;
  bool refinement_gate = false;
  int iterations = 0;
  size_t num_correspondences = 0;
  bool used_imu = false;
  size_t num_imu_samples = 0;
  std::array<Localizability, 3> rotation_localizability = {
      Localizability::FULL, Localizability::FULL, Localizability::FULL};
  std::array<Localizability, 3> translation_localizability = {
      Localizability::FULL, Localizability::FULL, Localizability::FULL};
  size_t num_degenerate_rotation_dirs = 0;
  size_t num_degenerate_translation_dirs = 0;
  bool has_degeneracy = false;
};

std::string trimCsvToken(std::string token) {
  while (!token.empty() &&
         (token.back() == '\r' || token.back() == '\n' || token.back() == ' ' ||
          token.back() == '\t')) {
    token.pop_back();
  }
  size_t begin = 0;
  while (begin < token.size() &&
         (token[begin] == ' ' || token[begin] == '\t')) {
    begin++;
  }
  return token.substr(begin);
}

std::vector<FrameEntry> collectPcds(const fs::path& pcd_dir, int start_frame,
                                    int max_frames) {
  std::vector<FrameEntry> entries;
  for (int offset = 0; max_frames <= 0 || offset < max_frames; ++offset) {
    const int i = start_frame + offset;
    std::ostringstream name;
    name << std::setw(8) << std::setfill('0') << i;
    fs::path path = pcd_dir / name.str() / "cloud.pcd";
    if (!fs::is_regular_file(path)) break;
    entries.push_back({i, path});
  }
  return entries;
}

std::vector<Eigen::Vector3d> loadPcdXYZ(const fs::path& path, double leaf,
                                        double min_range, double max_range,
                                        double z_min, double z_max) {
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
  if (pcl::io::loadPCDFile<pcl::PointXYZ>(path.string(), *cloud) == -1) {
    return {};
  }

  pcl::PointCloud<pcl::PointXYZ>::Ptr selected = cloud;
  pcl::PointCloud<pcl::PointXYZ> filtered;
  if (leaf > 1e-9) {
    pcl::VoxelGrid<pcl::PointXYZ> vg;
    vg.setInputCloud(cloud);
    vg.setLeafSize(leaf, leaf, leaf);
    vg.filter(filtered);
    selected.reset(new pcl::PointCloud<pcl::PointXYZ>(filtered));
  }

  std::vector<Eigen::Vector3d> points;
  points.reserve(selected->size());
  for (const auto& p : selected->points) {
    if (!std::isfinite(p.x) || !std::isfinite(p.y) ||
        !std::isfinite(p.z)) {
      continue;
    }
    const double r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    if (r < min_range || r > max_range || p.z < z_min || p.z > z_max) {
      continue;
    }
    points.emplace_back(p.x, p.y, p.z);
  }
  return points;
}

std::vector<Eigen::Vector3d> limitPoints(
    const std::vector<Eigen::Vector3d>& points, size_t max_points) {
  if (points.size() <= max_points) return points;
  std::vector<Eigen::Vector3d> limited;
  limited.reserve(max_points);
  const double step = static_cast<double>(points.size()) / max_points;
  for (size_t i = 0; i < max_points; ++i) {
    const size_t idx =
        std::min(static_cast<size_t>(i * step), points.size() - 1);
    limited.push_back(points[idx]);
  }
  return limited;
}

std::vector<Eigen::Vector3d> transformPoints(
    const std::vector<Eigen::Vector3d>& points, const Eigen::Matrix4d& pose) {
  std::vector<Eigen::Vector3d> out;
  out.reserve(points.size());
  const Eigen::Matrix3d R = pose.block<3, 3>(0, 0);
  const Eigen::Vector3d t = pose.block<3, 1>(0, 3);
  for (const auto& point : points) out.push_back(R * point + t);
  return out;
}

void compactPointMap(std::vector<Eigen::Vector3d>& map_points,
                     size_t max_points) {
  if (max_points == 0 || map_points.size() <= max_points) return;
  std::vector<Eigen::Vector3d> compacted;
  compacted.reserve(max_points);
  const double step = static_cast<double>(map_points.size()) / max_points;
  for (size_t i = 0; i < max_points; ++i) {
    const size_t idx =
        std::min(static_cast<size_t>(i * step), map_points.size() - 1);
    compacted.push_back(map_points[idx]);
  }
  map_points.swap(compacted);
}

void prunePointMapByRadius(std::vector<Eigen::Vector3d>& map_points,
                           const Eigen::Vector3d& center, double radius) {
  if (radius <= 0.0 || map_points.empty()) return;
  const double radius_sq = radius * radius;
  std::vector<Eigen::Vector3d> filtered;
  filtered.reserve(map_points.size());
  for (const auto& point : map_points) {
    if ((point - center).squaredNorm() <= radius_sq) filtered.push_back(point);
  }
  map_points.swap(filtered);
}

void addPointsToMap(std::vector<Eigen::Vector3d>& map_points,
                    const std::vector<Eigen::Vector3d>& local_points,
                    const Eigen::Matrix4d& pose, size_t max_points,
                    double map_radius) {
  auto points_world = transformPoints(local_points, pose);
  map_points.insert(map_points.end(), points_world.begin(), points_world.end());
  prunePointMapByRadius(map_points, pose.block<3, 1>(0, 3), map_radius);
  compactPointMap(map_points, max_points);
}

double yawDeg(const Eigen::Matrix4d& T) {
  return std::atan2(T(1, 0), T(0, 0)) * 180.0 / M_PI;
}

double rotationDeltaRad(const Eigen::Matrix4d& lhs,
                        const Eigen::Matrix4d& rhs) {
  const Eigen::Matrix3d dR =
      lhs.block<3, 3>(0, 0).transpose() * rhs.block<3, 3>(0, 0);
  const Eigen::AngleAxisd aa(dR);
  return std::isfinite(aa.angle()) ? std::abs(aa.angle())
                                   : std::numeric_limits<double>::infinity();
}

bool passesStepGate(const Eigen::Matrix4d& T, double max_translation,
                    double max_yaw_deg) {
  if (!T.array().isFinite().all()) return false;
  if (T.block<3, 1>(0, 3).norm() > max_translation) return false;
  if (std::abs(yawDeg(T)) > max_yaw_deg) return false;
  return true;
}

bool passesRefinementGate(const Eigen::Matrix4d& refined,
                          const Eigen::Matrix4d& seed,
                          double max_translation_delta,
                          double max_rotation_delta_rad) {
  if (!refined.array().isFinite().all()) return false;
  const double translation_delta =
      (refined.block<3, 1>(0, 3) - seed.block<3, 1>(0, 3)).norm();
  return translation_delta <= max_translation_delta &&
         rotationDeltaRad(refined, seed) <= max_rotation_delta_rad;
}

Eigen::Matrix4d velocityModelPrediction(const Eigen::Matrix4d& T_prev,
                                        const Eigen::Matrix4d& T_prev_prev) {
  const Eigen::Matrix4d delta_body = T_prev_prev.inverse() * T_prev;
  return T_prev * delta_body;
}

Eigen::Matrix4d composePose(const Eigen::Matrix3d& R, const Eigen::Vector3d& t) {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = R;
  T.block<3, 1>(0, 3) = t;
  return T;
}

const char* localizabilityToString(Localizability loc) {
  switch (loc) {
    case Localizability::FULL:
      return "FULL";
    case Localizability::PARTIAL:
      return "PARTIAL";
    case Localizability::NONE:
    default:
      return "NONE";
  }
}

// Builds RELEAD point-to-plane correspondences between the source scan
// (transformed into the map/world frame by the seed pose) and the local
// voxel map, using a KdTree nearest-neighbor search and neighborhood-
// covariance normals. This mirrors buildCorrespondences() in
// xicp_window_odometry.cpp; the only difference is the output type
// (relead::PointWithNormal instead of xicp::Correspondence) and field
// names (point/closest instead of source/target), matching what
// CESIKF::update() expects (see papers/relead/src/cesikf.cpp: it reads
// corr.point as the world-frame point and corr.closest/corr.normal as the
// map correspondence).
std::vector<PointWithNormal> buildCorrespondences(
    const std::vector<Eigen::Vector3d>& pts_local,
    const std::vector<Eigen::Vector3d>& map_points,
    const pcl::PointCloud<pcl::PointXYZ>::Ptr& map_cloud,
    pcl::KdTreeFLANN<pcl::PointXYZ>& kdtree, const Eigen::Matrix4d& T_seed,
    int k_neighbors, double max_correspondence_distance) {
  std::vector<PointWithNormal> correspondences;
  if (map_cloud->empty()) return correspondences;

  const Eigen::Matrix3d R = T_seed.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T_seed.block<3, 1>(0, 3);
  const int K = std::max(3, k_neighbors);
  const double max_dist_sq =
      max_correspondence_distance * max_correspondence_distance;

  std::vector<int> nn_indices(K);
  std::vector<float> nn_dists(K);
  correspondences.reserve(pts_local.size());

  for (const auto& p_local : pts_local) {
    const Eigen::Vector3d p_world = R * p_local + t;
    pcl::PointXYZ query;
    query.x = static_cast<float>(p_world.x());
    query.y = static_cast<float>(p_world.y());
    query.z = static_cast<float>(p_world.z());

    if (kdtree.nearestKSearch(query, K, nn_indices, nn_dists) < 3) continue;
    if (nn_dists[0] > max_dist_sq) continue;

    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
    const int valid_nn = std::min(K, static_cast<int>(nn_indices.size()));
    for (int k = 0; k < valid_nn; ++k) centroid += map_points[nn_indices[k]];
    centroid /= valid_nn;

    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    for (int k = 0; k < valid_nn; ++k) {
      const Eigen::Vector3d d = map_points[nn_indices[k]] - centroid;
      cov += d * d.transpose();
    }
    cov /= valid_nn;

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
    Eigen::Vector3d normal = solver.eigenvectors().col(0);  // smallest eigenvalue
    if (normal.dot(p_world - map_points[nn_indices[0]]) > 0) normal = -normal;

    PointWithNormal corr;
    corr.point = p_world;
    corr.normal = normal;
    corr.closest = map_points[nn_indices[0]];
    correspondences.push_back(corr);
  }
  return correspondences;
}

void rebuildKdTree(const std::vector<Eigen::Vector3d>& map_points,
                   pcl::PointCloud<pcl::PointXYZ>::Ptr& map_cloud,
                   pcl::KdTreeFLANN<pcl::PointXYZ>& kdtree) {
  map_cloud->clear();
  map_cloud->reserve(map_points.size());
  for (const auto& p : map_points) {
    pcl::PointXYZ pt;
    pt.x = static_cast<float>(p.x());
    pt.y = static_cast<float>(p.y());
    pt.z = static_cast<float>(p.z());
    map_cloud->push_back(pt);
  }
  if (!map_cloud->empty()) kdtree.setInputCloud(map_cloud);
}

// Header format: stamp,angular_vel.x,angular_vel.y,angular_vel.z,
//                linear_acc.x,linear_acc.y,linear_acc.z
// (epoch seconds; rad/s; m/s^2). Mirrors loadImuCsv() in
// degen_sense_window_odometry.cpp / pcd_dogfooding.cpp.
std::vector<ImuSampleCsv> loadImuCsv(const fs::path& csv_path) {
  std::vector<ImuSampleCsv> samples;
  std::ifstream file(csv_path);
  if (!file.is_open()) return samples;

  std::string line;
  if (!std::getline(file, line)) return samples;

  std::istringstream header_ss(line);
  std::string col;
  std::vector<std::string> cols;
  while (std::getline(header_ss, col, ',')) cols.push_back(trimCsvToken(col));

  int idx_stamp = -1;
  int idx_gx = -1, idx_gy = -1, idx_gz = -1;
  int idx_ax = -1, idx_ay = -1, idx_az = -1;
  for (size_t i = 0; i < cols.size(); i++) {
    if (cols[i] == "stamp") idx_stamp = static_cast<int>(i);
    if (cols[i] == "angular_vel.x") idx_gx = static_cast<int>(i);
    if (cols[i] == "angular_vel.y") idx_gy = static_cast<int>(i);
    if (cols[i] == "angular_vel.z") idx_gz = static_cast<int>(i);
    if (cols[i] == "linear_acc.x") idx_ax = static_cast<int>(i);
    if (cols[i] == "linear_acc.y") idx_ay = static_cast<int>(i);
    if (cols[i] == "linear_acc.z") idx_az = static_cast<int>(i);
  }

  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::vector<std::string> vals;
    std::string token;
    while (std::getline(ss, token, ',')) vals.push_back(trimCsvToken(token));

    if (idx_stamp < 0 || idx_az < 0 || idx_stamp >= static_cast<int>(vals.size()) ||
        idx_az >= static_cast<int>(vals.size())) {
      continue;
    }

    ImuSampleCsv sample;
    sample.timestamp = std::stod(vals[idx_stamp]);
    sample.gyro = Eigen::Vector3d(std::stod(vals[idx_gx]), std::stod(vals[idx_gy]),
                                  std::stod(vals[idx_gz]));
    sample.accel = Eigen::Vector3d(std::stod(vals[idx_ax]), std::stod(vals[idx_ay]),
                                   std::stod(vals[idx_az]));
    samples.push_back(sample);
  }
  return samples;
}

// [start_time, end_time] inclusive scan; imu.csv is monotonically
// increasing so we can break on the first sample past end_time. Mirrors
// selectImuWindow() in degen_sense_window_odometry.cpp / pcd_dogfooding.cpp.
std::vector<ImuSampleCsv> selectImuWindow(
    const std::vector<ImuSampleCsv>& imu_samples, double start_time,
    double end_time) {
  std::vector<ImuSampleCsv> out;
  for (const auto& sample : imu_samples) {
    if (sample.timestamp < start_time) continue;
    if (sample.timestamp > end_time) break;
    out.push_back(sample);
  }
  return out;
}

// Loads frame_timestamps.csv (columns: frame_idx,timestamp,...) into a map
// keyed by the *global* frame index (the zero-padded PCD directory name),
// so it can be looked up regardless of --start-frame / max_frames slicing.
// Mirrors loadFrameTimestamps() in degen_sense_window_odometry.cpp.
std::unordered_map<int, double> loadFrameTimestamps(const fs::path& pcd_dir) {
  std::unordered_map<int, double> out;
  fs::path csv_path = pcd_dir / "frame_timestamps.csv";
  std::ifstream file(csv_path);
  if (!file.is_open()) return out;

  std::string line;
  if (!std::getline(file, line)) return out;
  std::istringstream header_ss(line);
  std::string col;
  std::vector<std::string> cols;
  while (std::getline(header_ss, col, ',')) cols.push_back(trimCsvToken(col));

  int idx_frame = -1, idx_ts = -1;
  for (size_t i = 0; i < cols.size(); i++) {
    if (cols[i] == "frame_idx") idx_frame = static_cast<int>(i);
    if (cols[i] == "timestamp") idx_ts = static_cast<int>(i);
  }
  if (idx_ts < 0) return out;

  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::vector<std::string> vals;
    std::string token;
    while (std::getline(ss, token, ',')) vals.push_back(trimCsvToken(token));
    if (idx_ts >= static_cast<int>(vals.size())) continue;
    const int frame_idx = idx_frame >= 0 && idx_frame < static_cast<int>(vals.size())
                              ? std::stoi(vals[idx_frame])
                              : static_cast<int>(out.size());
    out[frame_idx] = std::stod(vals[idx_ts]);
  }
  return out;
}

void writeJsonNumber(std::ostream& out, double value) {
  if (std::isfinite(value)) {
    out << std::fixed << std::setprecision(6) << value;
  } else {
    out << "null";
  }
}

void writeLocalizabilityArray(std::ostream& out,
                              const std::array<Localizability, 3>& cats) {
  out << "[\"" << localizabilityToString(cats[0]) << "\", \""
      << localizabilityToString(cats[1]) << "\", \""
      << localizabilityToString(cats[2]) << "\"]";
}

void writeOutput(
    const fs::path& out_path, const fs::path& pcd_dir, const fs::path& gt_csv,
    const std::vector<Eigen::Matrix4d>& poses, const std::vector<PairLog>& pairs,
    double runtime_s, const CESIKFParams& params, int start_frame,
    double source_voxel_size, size_t max_source_points, int k_neighbors,
    double max_correspondence_distance, size_t map_max_points,
    size_t refresh_interval, double map_radius, double max_step_translation,
    double max_step_yaw_deg, double max_seed_translation_delta,
    double max_seed_rotation_delta_rad, bool require_convergence, bool use_imu,
    const fs::path& imu_csv_path) {
  fs::create_directories(out_path.parent_path());
  std::ofstream out(out_path);

  size_t accepted_count = 0;
  size_t used_imu_count = 0;
  size_t degenerate_count = 0;
  for (const auto& p : pairs) {
    if (p.accepted) ++accepted_count;
    if (p.used_imu) ++used_imu_count;
    if (p.has_degeneracy) ++degenerate_count;
  }

  out << "{\n";
  out << "  \"sequence_pcd_dir\": \"" << pcd_dir.string() << "\",\n";
  out << "  \"gt_csv\": \"" << gt_csv.string() << "\",\n";
  out << "  \"frames\": " << poses.size() << ",\n";
  out << "  \"method\": \"relead\",\n";
  out << "  \"runtime_s\": ";
  writeJsonNumber(out, runtime_s);
  out << ",\n";
  out << "  \"parameters\": {\n";
  out << "    \"start_frame\": " << start_frame << ",\n";
  out << "    \"source_voxel_size\": " << source_voxel_size << ",\n";
  out << "    \"max_source_points\": " << max_source_points << ",\n";
  out << "    \"k_neighbors\": " << k_neighbors << ",\n";
  out << "    \"max_correspondence_distance\": "
      << max_correspondence_distance << ",\n";
  out << "    \"min_correspondences\": " << kMinCorrespondences << ",\n";
  out << "    \"max_iterations\": " << params.max_iterations << ",\n";
  out << "    \"convergence_threshold\": " << params.convergence_threshold
      << ",\n";
  out << "    \"lidar_noise_std\": " << params.lidar_noise_std << ",\n";
  out << "    \"none_threshold\": " << params.degeneracy_params.none_threshold
      << ",\n";
  out << "    \"partial_threshold\": "
      << params.degeneracy_params.partial_threshold << ",\n";
  out << "    \"map_max_points\": " << map_max_points << ",\n";
  out << "    \"refresh_interval\": " << refresh_interval << ",\n";
  out << "    \"map_radius\": " << map_radius << ",\n";
  out << "    \"max_step_translation\": " << max_step_translation << ",\n";
  out << "    \"max_step_yaw_deg\": " << max_step_yaw_deg << ",\n";
  out << "    \"max_seed_translation_delta\": " << max_seed_translation_delta
      << ",\n";
  out << "    \"max_seed_rotation_delta_rad\": "
      << max_seed_rotation_delta_rad << ",\n";
  out << "    \"require_convergence\": "
      << (require_convergence ? "true" : "false") << ",\n";
  out << "    \"use_imu\": " << (use_imu ? "true" : "false") << ",\n";
  out << "    \"imu_csv\": \"" << imu_csv_path.string() << "\"\n";
  out << "  },\n";
  out << "  \"metrics\": {\n";
  out << "    \"ate_xy_m\": null,\n";
  out << "    \"step_length_rmse_m\": null,\n";
  out << "    \"num_pairs\": " << pairs.size() << ",\n";
  out << "    \"num_accepted\": " << accepted_count << ",\n";
  out << "    \"num_used_imu\": " << used_imu_count << ",\n";
  out << "    \"num_degenerate\": " << degenerate_count << "\n";
  out << "  },\n";
  out << "  \"poses_xyyaw\": [\n";
  for (size_t i = 0; i < poses.size(); ++i) {
    out << "    {\"index\": " << i << ", \"x_m\": ";
    writeJsonNumber(out, poses[i](0, 3));
    out << ", \"y_m\": ";
    writeJsonNumber(out, poses[i](1, 3));
    out << ", \"yaw_deg\": ";
    writeJsonNumber(out, yawDeg(poses[i]));
    out << "}";
    out << (i + 1 == poses.size() ? "\n" : ",\n");
  }
  out << "  ],\n";
  out << "  \"pairs\": [\n";
  for (size_t i = 0; i < pairs.size(); ++i) {
    const auto& p = pairs[i];
    out << "    {\n";
    out << "      \"index\": " << p.index << ",\n";
    out << "      \"dx_curr_to_prev_m\": ";
    writeJsonNumber(out, p.raw(0, 3));
    out << ",\n";
    out << "      \"dy_curr_to_prev_m\": ";
    writeJsonNumber(out, p.raw(1, 3));
    out << ",\n";
    out << "      \"yaw_curr_to_prev_deg\": ";
    writeJsonNumber(out, yawDeg(p.raw));
    out << ",\n";
    out << "      \"accepted\": " << (p.accepted ? "true" : "false") << ",\n";
    out << "      \"used_dx_curr_to_prev_m\": ";
    writeJsonNumber(out, p.used(0, 3));
    out << ",\n";
    out << "      \"used_dy_curr_to_prev_m\": ";
    writeJsonNumber(out, p.used(1, 3));
    out << ",\n";
    out << "      \"used_yaw_curr_to_prev_deg\": ";
    writeJsonNumber(out, yawDeg(p.used));
    out << ",\n";
    out << "      \"converged\": " << (p.converged ? "true" : "false") << ",\n";
    out << "      \"step_gate\": " << (p.step_gate ? "true" : "false") << ",\n";
    out << "      \"refinement_gate\": "
        << (p.refinement_gate ? "true" : "false") << ",\n";
    out << "      \"iterations\": " << p.iterations << ",\n";
    out << "      \"num_correspondences\": " << p.num_correspondences
        << ",\n";
    out << "      \"used_imu\": " << (p.used_imu ? "true" : "false") << ",\n";
    out << "      \"num_imu_samples\": " << p.num_imu_samples << ",\n";
    out << "      \"localizability_rotation\": ";
    writeLocalizabilityArray(out, p.rotation_localizability);
    out << ",\n";
    out << "      \"localizability_translation\": ";
    writeLocalizabilityArray(out, p.translation_localizability);
    out << ",\n";
    out << "      \"has_degeneracy\": "
        << (p.has_degeneracy ? "true" : "false") << ",\n";
    out << "      \"num_degenerate_rotation_dirs\": "
        << p.num_degenerate_rotation_dirs << ",\n";
    out << "      \"num_degenerate_translation_dirs\": "
        << p.num_degenerate_translation_dirs << "\n";
    out << "    }";
    out << (i + 1 == pairs.size() ? "\n" : ",\n");
  }
  out << "  ]\n";
  out << "}\n";
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cerr << "Usage: relead_window_odometry <pcd_dir> <gt_csv> "
                 "<output_json> [max_frames] [--start-frame N] "
                 "[--imu-csv PATH] [--no-imu] "
                 "[--source-voxel-size X] [--max-source-points N] "
                 "[--k-neighbors N] [--max-correspondence-distance X] "
                 "[--map-max-points N] [--refresh-interval N] "
                 "[--map-radius X] [--max-iterations N] "
                 "[--lidar-noise-std X] "
                 "[--max-step-translation X] [--max-step-yaw-deg X] "
                 "[--max-seed-translation-delta X] "
                 "[--max-seed-rotation-delta-rad X] "
                 "[--require-convergence]\n"
                 "  gt_csv may be '-' for GT-free operation (no ATE is ever "
                 "computed by this runner; gt_csv is recorded for "
                 "bookkeeping only).\n"
                 "  max_frames <= 0 (e.g. -1) means: use all available "
                 "frames from start_frame onward.\n"
                 "  --imu-csv defaults to <pcd_dir>/imu.csv if present. "
                 "--no-imu disables IMU predict() entirely; the frame seed "
                 "then falls back to a constant-velocity extrapolation of "
                 "the last two accepted poses, written directly into "
                 "CESIKF's mutable state().\n";
    return 1;
  }

  const fs::path pcd_dir = argv[1];
  const fs::path gt_csv = argv[2];
  const fs::path out_json = argv[3];
  const int max_frames = argc >= 5 && std::string(argv[4]).rfind("--", 0) != 0
                             ? std::stoi(argv[4])
                             : -1;

  CESIKFParams params;
  params.max_iterations = 10;
  params.convergence_threshold = 1e-4;

  int start_frame = 0;
  bool use_imu = true;
  fs::path imu_csv_override;
  bool imu_csv_overridden = false;
  double source_voxel_size = 1.0;
  size_t max_source_points = 2500;
  int k_neighbors = 10;
  double max_correspondence_distance = 3.0;
  size_t map_max_points = 40000;
  size_t refresh_interval = 4;
  double map_radius = 45.0;
  const double min_range = 0.2;
  const double max_range = 30.0;
  const double z_min = -5.0;
  const double z_max = 5.0;
  double max_step_translation = 2.0;
  double max_step_yaw_deg = 20.0;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;
  bool require_convergence = false;

  const int opt_begin =
      argc >= 5 && std::string(argv[4]).rfind("--", 0) != 0 ? 5 : 4;
  for (int i = opt_begin; i < argc; ++i) {
    const std::string arg = argv[i];
    auto needValue = [&](const std::string& name) {
      if (i + 1 >= argc) throw std::runtime_error(name + " requires a value");
      return std::string(argv[++i]);
    };
    if (arg == "--start-frame") {
      start_frame = std::max(0, std::stoi(needValue(arg)));
    } else if (arg == "--imu-csv") {
      imu_csv_override = needValue(arg);
      imu_csv_overridden = true;
    } else if (arg == "--no-imu") {
      use_imu = false;
    } else if (arg == "--source-voxel-size") {
      source_voxel_size = std::stod(needValue(arg));
    } else if (arg == "--max-source-points") {
      max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(needValue(arg))));
    } else if (arg == "--k-neighbors") {
      k_neighbors = std::max(3, std::stoi(needValue(arg)));
    } else if (arg == "--max-correspondence-distance") {
      max_correspondence_distance = std::stod(needValue(arg));
    } else if (arg == "--map-max-points") {
      map_max_points =
          static_cast<size_t>(std::max(1, std::stoi(needValue(arg))));
    } else if (arg == "--refresh-interval") {
      refresh_interval =
          static_cast<size_t>(std::max(1, std::stoi(needValue(arg))));
    } else if (arg == "--map-radius") {
      map_radius = std::stod(needValue(arg));
    } else if (arg == "--max-iterations") {
      params.max_iterations = std::max(1, std::stoi(needValue(arg)));
    } else if (arg == "--lidar-noise-std") {
      params.lidar_noise_std = std::stod(needValue(arg));
    } else if (arg == "--max-step-translation") {
      max_step_translation = std::stod(needValue(arg));
    } else if (arg == "--max-step-yaw-deg") {
      max_step_yaw_deg = std::stod(needValue(arg));
    } else if (arg == "--max-seed-translation-delta") {
      max_seed_translation_delta = std::stod(needValue(arg));
    } else if (arg == "--max-seed-rotation-delta-rad") {
      max_seed_rotation_delta_rad = std::stod(needValue(arg));
    } else if (arg == "--require-convergence") {
      require_convergence = true;
    } else {
      throw std::runtime_error("Unknown option: " + arg);
    }
  }

  const auto pcds = collectPcds(pcd_dir, start_frame, max_frames);
  if (pcds.size() < 2) throw std::runtime_error("Need at least two PCD frames");

  fs::path imu_csv_path =
      imu_csv_overridden ? imu_csv_override : (pcd_dir / "imu.csv");
  std::vector<ImuSampleCsv> imu_samples;
  std::unordered_map<int, double> frame_timestamps;
  if (use_imu && fs::is_regular_file(imu_csv_path)) {
    imu_samples = loadImuCsv(imu_csv_path);
    frame_timestamps = loadFrameTimestamps(pcd_dir);
  }
  const bool imu_available =
      use_imu && !imu_samples.empty() && !frame_timestamps.empty();
  std::cerr << "[RELEAD-window] imu.csv=" << imu_csv_path
            << " samples=" << imu_samples.size()
            << " frame_timestamps=" << frame_timestamps.size()
            << " imu_available=" << (imu_available ? "yes" : "no")
            << std::endl;

  CESIKF filter(params);
  std::vector<Eigen::Vector3d> map_points;
  pcl::PointCloud<pcl::PointXYZ>::Ptr map_cloud(new pcl::PointCloud<pcl::PointXYZ>);
  pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;

  Eigen::Matrix4d T_prev_world = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d T_prev_prev_world = Eigen::Matrix4d::Identity();
  // Full-state snapshot taken after the last accepted update (rotation,
  // position, velocity, biases, gravity), restored verbatim on reject. See
  // the file header comment for why this must be the whole State and not
  // just rotation/position.
  localization_zoo::relead::State last_accepted_state = filter.state();

  std::vector<Eigen::Matrix4d> poses;
  std::vector<PairLog> pairs;
  poses.reserve(pcds.size());
  pairs.reserve(pcds.size() - 1);
  poses.push_back(Eigen::Matrix4d::Identity());

  const auto t0 = Clock::now();
  for (size_t i = 0; i < pcds.size(); ++i) {
    auto points = limitPoints(
        loadPcdXYZ(pcds[i].path, source_voxel_size, min_range, max_range,
                   z_min, z_max),
        max_source_points);
    if (points.empty()) {
      std::cerr << "[RELEAD-window] empty scan at " << pcds[i].path
                << std::endl;
      if (i > 0) {
        PairLog log;
        log.index = static_cast<int>(i);
        pairs.push_back(log);
        poses.push_back(T_prev_world);
      }
      continue;
    }

    if (i == 0) {
      addPointsToMap(map_points, points, T_prev_world, map_max_points,
                     map_radius);
      rebuildKdTree(map_points, map_cloud, kdtree);
      continue;
    }

    // Seed the filter's own state before building correspondences: either
    // by IMU-predicting forward from the last accepted pose, or -- if IMU
    // is unavailable -- by writing a constant-velocity extrapolation
    // directly into CESIKF's mutable state().
    bool used_imu_step = false;
    size_t n_imu_samples = 0;
    if (imu_available) {
      const auto it_prev = frame_timestamps.find(pcds[i - 1].global_index);
      const auto it_curr = frame_timestamps.find(pcds[i].global_index);
      if (it_prev != frame_timestamps.end() && it_curr != frame_timestamps.end()) {
        const auto imu_batch =
            selectImuWindow(imu_samples, it_prev->second, it_curr->second);
        double last_t = it_prev->second;
        for (const auto& s : imu_batch) {
          const double dt = s.timestamp - last_t;
          if (dt > 0.0 && dt < 0.5) {
            ImuMeasurement m;
            m.timestamp = s.timestamp;
            m.angular_velocity = s.gyro;
            m.linear_acceleration = s.accel;
            filter.predict(m, dt);
          }
          last_t = s.timestamp;
        }
        if (!imu_batch.empty()) {
          used_imu_step = true;
          n_imu_samples = imu_batch.size();
        }
      }
    }
    if (!used_imu_step) {
      const Eigen::Matrix4d seed =
          i >= 2 ? velocityModelPrediction(T_prev_world, T_prev_prev_world)
                 : T_prev_world;
      filter.state().rotation = seed.block<3, 3>(0, 0);
      filter.state().position = seed.block<3, 1>(0, 3);
    }

    const Eigen::Matrix4d T_seed =
        composePose(filter.state().rotation, filter.state().position);

    const auto correspondences = buildCorrespondences(
        points, map_points, map_cloud, kdtree, T_seed, k_neighbors,
        max_correspondence_distance);

    PairLog log;
    log.index = static_cast<int>(i);
    log.num_correspondences = correspondences.size();
    log.used_imu = used_imu_step;
    log.num_imu_samples = n_imu_samples;

    bool accepted = false;
    Eigen::Matrix4d T_refined = T_seed;
    if (correspondences.size() >= kMinCorrespondences) {
      const CESIKFResult result = filter.update(correspondences);
      T_refined = composePose(result.state.rotation, result.state.position);
      const Eigen::Matrix4d raw_delta = T_prev_world.inverse() * T_refined;
      const bool step_gate =
          passesStepGate(raw_delta, max_step_translation, max_step_yaw_deg);
      const bool refinement_gate =
          passesRefinementGate(T_refined, T_seed, max_seed_translation_delta,
                               max_seed_rotation_delta_rad);
      accepted = step_gate && refinement_gate &&
                 (!require_convergence || result.converged);

      log.raw = raw_delta;
      log.converged = result.converged;
      log.step_gate = step_gate;
      log.refinement_gate = refinement_gate;
      log.iterations = result.iterations;
      log.rotation_localizability = result.degeneracy_info.rotation_localizability;
      log.translation_localizability =
          result.degeneracy_info.translation_localizability;
      log.num_degenerate_rotation_dirs =
          result.degeneracy_info.degenerate_rotation_dirs.size();
      log.num_degenerate_translation_dirs =
          result.degeneracy_info.degenerate_translation_dirs.size();
      log.has_degeneracy = result.degeneracy_info.hasDegeneracy();
    } else {
      // Not enough map correspondences to trust an ESIKF update; fall back
      // to the seed prediction without accepting it.
      log.raw = T_prev_world.inverse() * T_seed;
    }
    log.accepted = accepted;

    if (accepted) {
      log.used = log.raw;
      T_prev_prev_world = T_prev_world;
      T_prev_world = T_refined;
      // filter.state() already holds T_refined (update() wrote into it).
      // Snapshot the full state now that it is trusted, so a future reject
      // has a clean state to restore.
      last_accepted_state = filter.state();
      if (i <= 1 || refresh_interval <= 1 || (i % refresh_interval) == 0) {
        addPointsToMap(map_points, points, T_prev_world, map_max_points,
                       map_radius);
        rebuildKdTree(map_points, map_cloud, kdtree);
      }
    } else {
      log.used = Eigen::Matrix4d::Identity();
      // Reset the filter (state_ back to defaults, P_ back to its tight
      // 0.001*I prior) and then restore the last accepted State snapshot,
      // so neither a corrupted velocity/bias estimate nor an
      // predict()-inflated covariance can keep driving drift through
      // subsequent frames (see file header comment).
      filter.reset();
      filter.state() = last_accepted_state;
    }

    pairs.push_back(log);
    poses.push_back(T_prev_world);

    if (i % 10 == 0) {
      std::cerr << "\r[RELEAD-window] " << i << "/" << (pcds.size() - 1)
                << " corr=" << log.num_correspondences
                << " conv=" << (log.converged ? "yes" : "no")
                << " accepted=" << (accepted ? "yes" : "no")
                << " used_imu=" << (log.used_imu ? "yes" : "no")
                << " degenerate=" << (log.has_degeneracy ? "yes" : "no");
    }
  }
  std::cerr << std::endl;

  const double runtime_s =
      std::chrono::duration<double>(Clock::now() - t0).count();
  writeOutput(out_json, pcd_dir, gt_csv, poses, pairs, runtime_s, params,
              start_frame, source_voxel_size, max_source_points, k_neighbors,
              max_correspondence_distance, map_max_points, refresh_interval,
              map_radius, max_step_translation, max_step_yaw_deg,
              max_seed_translation_delta, max_seed_rotation_delta_rad,
              require_convergence, use_imu, imu_csv_path);
  std::cout << "Wrote " << out_json << " frames=" << poses.size() << std::endl;
  return 0;
}
