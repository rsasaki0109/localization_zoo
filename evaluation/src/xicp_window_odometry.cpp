#include "xicp/xicp_registration.h"
#include "xicp/types.h"

#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Geometry>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::high_resolution_clock;
using localization_zoo::xicp::Correspondence;
using localization_zoo::xicp::Localizability;
using localization_zoo::xicp::LocalizabilityInfo;
using localization_zoo::xicp::XICPParams;
using localization_zoo::xicp::XICPRegistration;
using localization_zoo::xicp::XICPResult;

namespace {

// X-ICP requires at least this many point-to-plane correspondences before an
// alignment attempt is trusted (mirrors the dogfooding harness convention in
// pcd_dogfooding.cpp's runXICPDogfooding).
constexpr size_t kMinCorrespondences = 64;

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
  LocalizabilityInfo localizability;
};

std::vector<fs::path> collectPcds(const fs::path& pcd_dir, int start_frame,
                                  int max_frames) {
  std::vector<fs::path> paths;
  for (int offset = 0; max_frames <= 0 || offset < max_frames; ++offset) {
    const int i = start_frame + offset;
    std::ostringstream name;
    name << std::setw(8) << std::setfill('0') << i;
    fs::path path = pcd_dir / name.str() / "cloud.pcd";
    if (!fs::is_regular_file(path)) break;
    paths.push_back(path);
  }
  return paths;
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

// Builds point-to-plane correspondences between the source scan (transformed
// into the map frame by the initial guess) and the local voxel map, using a
// KdTree nearest-neighbor search and neighborhood-covariance normals. This
// mirrors runXICPDogfooding() in pcd_dogfooding.cpp so that the standalone
// window-odometry runner drives X-ICP the same way the dogfooding harness
// does.
std::vector<Correspondence> buildCorrespondences(
    const std::vector<Eigen::Vector3d>& pts_local,
    const std::vector<Eigen::Vector3d>& map_points,
    const pcl::PointCloud<pcl::PointXYZ>::Ptr& map_cloud,
    pcl::KdTreeFLANN<pcl::PointXYZ>& kdtree, const Eigen::Matrix4d& T_guess,
    int k_neighbors, double max_correspondence_distance) {
  std::vector<Correspondence> correspondences;
  if (map_cloud->empty()) return correspondences;

  const Eigen::Matrix3d R = T_guess.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T_guess.block<3, 1>(0, 3);
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

    Correspondence corr;
    corr.source = p_local;
    corr.target = map_points[nn_indices[0]];
    corr.normal = normal;
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

void writeJsonNumber(std::ostream& out, double value) {
  if (std::isfinite(value)) {
    out << std::fixed << std::setprecision(6) << value;
  } else {
    out << "null";
  }
}

void writeVec3(std::ostream& out, const Eigen::Vector3d& v) {
  out << "[";
  writeJsonNumber(out, v.x());
  out << ", ";
  writeJsonNumber(out, v.y());
  out << ", ";
  writeJsonNumber(out, v.z());
  out << "]";
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
    double runtime_s, const XICPParams& params, int start_frame,
    double source_voxel_size, size_t max_source_points, int k_neighbors,
    double max_correspondence_distance, size_t map_max_points,
    size_t refresh_interval, double map_radius, double max_step_translation,
    double max_step_yaw_deg, double max_seed_translation_delta,
    double max_seed_rotation_delta_rad, bool require_convergence) {
  fs::create_directories(out_path.parent_path());
  std::ofstream out(out_path);

  out << "{\n";
  out << "  \"sequence_pcd_dir\": \"" << pcd_dir.string() << "\",\n";
  out << "  \"gt_csv\": \"" << gt_csv.string() << "\",\n";
  out << "  \"frames\": " << poses.size() << ",\n";
  out << "  \"method\": \"xicp\",\n";
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
  out << "    \"kappa_f\": " << params.kappa_f << ",\n";
  out << "    \"kappa_s\": " << params.kappa_s << ",\n";
  out << "    \"kappa_1\": " << params.kappa_1 << ",\n";
  out << "    \"kappa_2\": " << params.kappa_2 << ",\n";
  out << "    \"kappa_3\": " << params.kappa_3 << ",\n";
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
      << (require_convergence ? "true" : "false") << "\n";
  out << "  },\n";
  out << "  \"metrics\": {\n";
  out << "    \"ate_xy_m\": null,\n";
  out << "    \"step_length_rmse_m\": null\n";
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
    const auto& loc = p.localizability;
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
    out << "      \"score\": null,\n";
    out << "      \"overlap\": null,\n";
    out << "      \"converged\": " << (p.converged ? "true" : "false") << ",\n";
    out << "      \"step_gate\": " << (p.step_gate ? "true" : "false") << ",\n";
    out << "      \"refinement_gate\": "
        << (p.refinement_gate ? "true" : "false") << ",\n";
    out << "      \"iterations\": " << p.iterations << ",\n";
    out << "      \"final_error\": null,\n";
    out << "      \"num_correspondences\": " << p.num_correspondences
        << ",\n";
    out << "      \"localizability_rotation\": ";
    writeLocalizabilityArray(out, loc.rotation);
    out << ",\n";
    out << "      \"localizability_translation\": ";
    writeLocalizabilityArray(out, loc.translation);
    out << ",\n";
    out << "      \"sigma_rotation\": ";
    writeVec3(out, loc.sigma_r);
    out << ",\n";
    out << "      \"sigma_translation\": ";
    writeVec3(out, loc.sigma_t);
    out << ",\n";
    out << "      \"has_degeneracy\": "
        << (loc.hasDegeneracy() ? "true" : "false") << ",\n";
    out << "      \"num_degenerate_directions\": "
        << loc.numDegenerateDirections() << "\n";
    out << "    }";
    out << (i + 1 == pairs.size() ? "\n" : ",\n");
  }
  out << "  ]\n";
  out << "}\n";
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cerr << "Usage: xicp_window_odometry <pcd_dir> <gt_csv> "
                 "<output_json> [max_frames] [--start-frame N] "
                 "[--source-voxel-size X] [--max-source-points N] "
                 "[--k-neighbors N] [--max-correspondence-distance X] "
                 "[--map-max-points N] [--refresh-interval N] "
                 "[--map-radius X] [--max-iterations N] "
                 "[--max-step-translation X] [--max-step-yaw-deg X] "
                 "[--max-seed-translation-delta X] "
                 "[--max-seed-rotation-delta-rad X] "
                 "[--require-convergence]\n"
                 "  gt_csv may be '-' for GT-free operation (no ATE is ever "
                 "computed by this runner; gt_csv is recorded for "
                 "bookkeeping only).\n"
                 "  max_frames <= 0 (e.g. -1) means: use all available "
                 "frames from start_frame onward.\n";
    return 1;
  }

  const fs::path pcd_dir = argv[1];
  const fs::path gt_csv = argv[2];
  const fs::path out_json = argv[3];
  const int max_frames = argc >= 5 && std::string(argv[4]).rfind("--", 0) != 0
                             ? std::stoi(argv[4])
                             : -1;

  XICPParams params;
  params.max_iterations = 30;
  params.convergence_threshold = 1e-5;

  int start_frame = 0;
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

  XICPRegistration reg(params);
  std::vector<Eigen::Vector3d> map_points;
  pcl::PointCloud<pcl::PointXYZ>::Ptr map_cloud(new pcl::PointCloud<pcl::PointXYZ>);
  pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;

  Eigen::Matrix4d T_prev_world = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d T_prev_prev_world = Eigen::Matrix4d::Identity();

  std::vector<Eigen::Matrix4d> poses;
  std::vector<PairLog> pairs;
  poses.reserve(pcds.size());
  pairs.reserve(pcds.size() - 1);
  poses.push_back(Eigen::Matrix4d::Identity());

  const auto t0 = Clock::now();
  for (size_t i = 0; i < pcds.size(); ++i) {
    auto points = limitPoints(
        loadPcdXYZ(pcds[i], source_voxel_size, min_range, max_range, z_min,
                   z_max),
        max_source_points);
    if (points.empty()) {
      std::cerr << "[XICP-window] empty scan at " << pcds[i] << std::endl;
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

    const Eigen::Matrix4d seed =
        i >= 2 ? velocityModelPrediction(T_prev_world, T_prev_prev_world)
               : T_prev_world;

    const auto correspondences = buildCorrespondences(
        points, map_points, map_cloud, kdtree, seed, k_neighbors,
        max_correspondence_distance);

    PairLog log;
    log.index = static_cast<int>(i);
    log.num_correspondences = correspondences.size();

    bool accepted = false;
    Eigen::Matrix4d T_refined = seed;
    if (correspondences.size() >= kMinCorrespondences) {
      const XICPResult result = reg.align(correspondences, seed);
      T_refined = result.transformation;
      const Eigen::Matrix4d raw_delta = T_prev_world.inverse() * T_refined;
      const bool step_gate =
          passesStepGate(raw_delta, max_step_translation, max_step_yaw_deg);
      const bool refinement_gate =
          passesRefinementGate(T_refined, seed, max_seed_translation_delta,
                               max_seed_rotation_delta_rad);
      accepted = step_gate && refinement_gate &&
                 (!require_convergence || result.converged);

      log.raw = raw_delta;
      log.converged = result.converged;
      log.step_gate = step_gate;
      log.refinement_gate = refinement_gate;
      log.iterations = result.num_iterations;
      log.localizability = result.localizability;
    } else {
      // Not enough map correspondences to trust an alignment; fall back to
      // the seed prediction without accepting it. LocalizabilityInfo stays
      // at its default (all-FULL, zero eigenvalues) since no analysis ran.
      log.raw = T_prev_world.inverse() * seed;
    }
    log.accepted = accepted;

    if (accepted) {
      log.used = log.raw;
      T_prev_prev_world = T_prev_world;
      T_prev_world = T_refined;
      if (i <= 1 || refresh_interval <= 1 || (i % refresh_interval) == 0) {
        addPointsToMap(map_points, points, T_prev_world, map_max_points,
                       map_radius);
        rebuildKdTree(map_points, map_cloud, kdtree);
      }
    } else {
      log.used = Eigen::Matrix4d::Identity();
    }

    pairs.push_back(log);
    poses.push_back(T_prev_world);

    if (i % 10 == 0) {
      std::cerr << "\r[XICP-window] " << i << "/" << (pcds.size() - 1)
                << " corr=" << log.num_correspondences
                << " conv=" << (log.converged ? "yes" : "no")
                << " accepted=" << (accepted ? "yes" : "no")
                << " degenerate=" << log.localizability.numDegenerateDirections();
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
              require_convergence);
  std::cout << "Wrote " << out_json << " frames=" << poses.size() << std::endl;
  return 0;
}
