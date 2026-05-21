#include "ct_icp/ct_icp_registration.h"

#define PCL_NO_PRECOMPILE
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/PCLPointCloud2.h>
#include <pcl/conversions.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>
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

struct EIGEN_ALIGN16 PointXYZITime {
  PCL_ADD_POINT4D;
  float intensity;
  float time;
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

POINT_CLOUD_REGISTER_POINT_STRUCT(
    PointXYZITime,
    (float, x, x)
    (float, y, y)
    (float, z, z)
    (float, intensity, intensity)
    (float, time, time))

namespace {

struct LoadedScan {
  std::vector<Eigen::Vector3d> points;
  std::vector<double> relative_times;
  bool has_per_point_time = false;
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

std::vector<Eigen::Vector3d> loadPcdXYZ(const std::string& path, double leaf) {
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
  if (pcl::io::loadPCDFile<pcl::PointXYZ>(path, *cloud) == -1) return {};

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
    if (r > 1.0 && r < 80.0) points.emplace_back(p.x, p.y, p.z);
  }
  return points;
}

void normalizeRelativeTimes(std::vector<double>& relative_times) {
  if (relative_times.empty()) return;
  const auto [min_it, max_it] =
      std::minmax_element(relative_times.begin(), relative_times.end());
  const double min_time = *min_it;
  const double max_time = *max_it;
  if (max_time <= min_time + 1e-9) {
    const size_t denom = relative_times.size() > 1 ? relative_times.size() - 1 : 1;
    for (size_t i = 0; i < relative_times.size(); ++i) {
      relative_times[i] = static_cast<double>(i) / denom;
    }
    return;
  }
  for (double& t : relative_times) t = (t - min_time) / (max_time - min_time);
}

LoadedScan limitLoadedScan(const LoadedScan& scan, size_t max_points) {
  if (scan.points.size() <= max_points) return scan;

  LoadedScan limited;
  limited.has_per_point_time = scan.has_per_point_time;
  limited.points.reserve(max_points);
  if (scan.has_per_point_time) limited.relative_times.reserve(max_points);

  const double step = static_cast<double>(scan.points.size()) / max_points;
  for (size_t i = 0; i < max_points; ++i) {
    const size_t idx =
        std::min(static_cast<size_t>(i * step), scan.points.size() - 1);
    limited.points.push_back(scan.points[idx]);
    if (scan.has_per_point_time) limited.relative_times.push_back(scan.relative_times[idx]);
  }
  return limited;
}

LoadedScan loadTimedPcd(const fs::path& path, double leaf) {
  LoadedScan scan;
  pcl::PCLPointCloud2 raw_cloud;
  if (pcl::io::loadPCDFile(path.string(), raw_cloud) == -1) return scan;

  bool has_time_field = false;
  for (const auto& field : raw_cloud.fields) {
    if (field.name == "time") {
      has_time_field = true;
      break;
    }
  }
  if (!has_time_field) {
    scan.points = loadPcdXYZ(path.string(), leaf);
    return scan;
  }

  pcl::PointCloud<PointXYZITime>::Ptr cloud(new pcl::PointCloud<PointXYZITime>);
  pcl::fromPCLPointCloud2(raw_cloud, *cloud);

  pcl::PointCloud<PointXYZITime>::Ptr selected = cloud;
  pcl::PointCloud<PointXYZITime> filtered;
  if (leaf > 1e-9) {
    pcl::VoxelGrid<PointXYZITime> vg;
    vg.setInputCloud(cloud);
    vg.setLeafSize(leaf, leaf, leaf);
    vg.setDownsampleAllData(true);
    vg.filter(filtered);
    selected.reset(new pcl::PointCloud<PointXYZITime>(filtered));
  }

  scan.points.reserve(selected->size());
  scan.relative_times.reserve(selected->size());
  for (const auto& p : selected->points) {
    if (!std::isfinite(p.x) || !std::isfinite(p.y) ||
        !std::isfinite(p.z) || !std::isfinite(p.time)) {
      continue;
    }
    const double r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    if (r <= 1.0 || r >= 80.0) continue;
    scan.points.emplace_back(p.x, p.y, p.z);
    scan.relative_times.push_back(p.time);
  }

  scan.has_per_point_time = !scan.relative_times.empty();
  normalizeRelativeTimes(scan.relative_times);
  return scan;
}

std::vector<localization_zoo::ct_icp::TimedPoint> makeTimedPoints(
    const LoadedScan& scan) {
  std::vector<localization_zoo::ct_icp::TimedPoint> timed;
  timed.reserve(scan.points.size());
  const size_t denom = scan.points.size() > 1 ? scan.points.size() - 1 : 1;
  for (size_t j = 0; j < scan.points.size(); ++j) {
    localization_zoo::ct_icp::TimedPoint tp;
    tp.raw_point = scan.points[j];
    tp.timestamp = scan.has_per_point_time ? scan.relative_times[j]
                                           : static_cast<double>(j) / denom;
    timed.push_back(tp);
  }
  return timed;
}

Eigen::Matrix4d poseToMatrix(const localization_zoo::ct_icp::SE3& pose) {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = pose.quat.toRotationMatrix();
  T.block<3, 1>(0, 3) = pose.trans;
  return T;
}

Eigen::Matrix4d frameToMatrix(
    const localization_zoo::ct_icp::TrajectoryFrame& frame) {
  return poseToMatrix(frame.end_pose);
}

double yawDeg(const Eigen::Matrix4d& T) {
  return std::atan2(T(1, 0), T(0, 0)) * 180.0 / M_PI;
}

double rotationDeltaRad(const Eigen::Matrix4d& lhs, const Eigen::Matrix4d& rhs) {
  const Eigen::Matrix3d dR =
      lhs.block<3, 3>(0, 0).transpose() * rhs.block<3, 3>(0, 0);
  const Eigen::AngleAxisd aa(dR);
  if (!std::isfinite(aa.angle())) return std::numeric_limits<double>::infinity();
  return std::abs(aa.angle());
}

bool passesStepGate(const Eigen::Matrix4d& delta, double max_translation,
                    double max_yaw_deg) {
  return delta.array().isFinite().all() &&
         delta.block<3, 1>(0, 3).norm() <= max_translation &&
         std::abs(yawDeg(delta)) <= max_yaw_deg;
}

bool passesRefinementGate(const Eigen::Matrix4d& refined,
                          const Eigen::Matrix4d& seed,
                          double max_translation_delta,
                          double max_rotation_delta_rad) {
  return refined.array().isFinite().all() &&
         seed.array().isFinite().all() &&
         (refined.block<3, 1>(0, 3) - seed.block<3, 1>(0, 3)).norm() <=
             max_translation_delta &&
         rotationDeltaRad(refined, seed) <= max_rotation_delta_rad;
}

Eigen::Matrix4d velocityModelPrediction(const Eigen::Matrix4d& T_prev,
                                        const Eigen::Matrix4d& T_prev_prev) {
  const Eigen::Matrix4d delta_body = T_prev_prev.inverse() * T_prev;
  return T_prev * delta_body;
}

void writeJsonNumber(std::ostream& out, double value) {
  if (std::isfinite(value)) {
    out << std::fixed << std::setprecision(6) << value;
  } else {
    out << "null";
  }
}

void writeOutput(const fs::path& out_path, const fs::path& pcd_dir,
                 const fs::path& gt_csv,
                 const std::vector<Eigen::Matrix4d>& poses,
                 const std::vector<PairLog>& pairs, double runtime_s,
                 const localization_zoo::ct_icp::CTICPParams& params,
                 int start_frame, bool refinement_gate_enabled,
                 bool native_ct_icp_seed, double max_step_translation,
                 double max_step_yaw_deg,
                 double max_seed_translation_delta,
                 double max_seed_rotation_delta_rad,
                 bool require_convergence) {
  fs::create_directories(out_path.parent_path());
  std::ofstream out(out_path);

  out << "{\n";
  out << "  \"sequence_pcd_dir\": \"" << pcd_dir.string() << "\",\n";
  out << "  \"gt_csv\": \"" << gt_csv.string() << "\",\n";
  out << "  \"frames\": " << poses.size() << ",\n";
  out << "  \"method\": \"ct_icp_window_odometry\",\n";
  out << "  \"runtime_s\": ";
  writeJsonNumber(out, runtime_s);
  out << ",\n";
  out << "  \"parameters\": {\n";
  out << "    \"start_frame\": " << start_frame << ",\n";
  out << "    \"voxel_resolution\": " << params.voxel_resolution << ",\n";
  out << "    \"keypoint_voxel_size\": " << params.keypoint_voxel_size << ",\n";
  out << "    \"max_iterations\": " << params.max_iterations << ",\n";
  out << "    \"ceres_max_iterations\": " << params.ceres_max_iterations << ",\n";
  out << "    \"max_frames_in_map\": " << params.max_frames_in_map << ",\n";
  out << "    \"planarity_threshold\": " << params.planarity_threshold << ",\n";
  out << "    \"max_correspondence_dist\": " << params.max_correspondence_dist << ",\n";
  out << "    \"multi_scale_correspondences\": "
      << (params.multi_scale_correspondences ? "true" : "false") << ",\n";
  out << "    \"use_normal_cholesky_solver\": "
      << (params.use_normal_cholesky_solver ? "true" : "false") << ",\n";
  out << "    \"refinement_gate\": "
      << (refinement_gate_enabled ? "true" : "false") << ",\n";
  out << "    \"native_ct_icp_seed\": "
      << (native_ct_icp_seed ? "true" : "false") << ",\n";
  out << "    \"max_step_translation\": " << max_step_translation << ",\n";
  out << "    \"max_step_yaw_deg\": " << max_step_yaw_deg << ",\n";
  out << "    \"max_seed_translation_delta\": " << max_seed_translation_delta << ",\n";
  out << "    \"max_seed_rotation_delta_rad\": " << max_seed_rotation_delta_rad << ",\n";
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
    out << "      \"refinement_gate\": " << (p.refinement_gate ? "true" : "false") << ",\n";
    out << "      \"iterations\": " << p.iterations << ",\n";
    out << "      \"rmse_m\": null\n";
    out << "    }";
    out << (i + 1 == pairs.size() ? "\n" : ",\n");
  }
  out << "  ]\n";
  out << "}\n";
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cerr << "Usage: ct_icp_window_odometry <pcd_dir> <gt_csv> "
                 "<output_json> [max_frames] [--start-frame N] "
                 "[--source-voxel-size X] [--max-source-points N] "
                 "[--voxel-resolution X] [--keypoint-voxel-size X] "
                 "[--max-iterations N] [--ceres-max-iterations N] "
                 "[--max-frames-in-map N] [--planarity-threshold X] "
                 "[--max-correspondence-distance X] [--multi-scale] "
                 "[--normal-cholesky] [--refinement-gate] "
                 "[--native-ct-icp-seed] [--max-step-translation X] "
                 "[--max-step-yaw-deg X] [--max-seed-translation-delta X] "
                 "[--max-seed-rotation-delta-rad X] [--require-convergence]\n";
    return 1;
  }

  const fs::path pcd_dir = argv[1];
  const fs::path gt_csv = argv[2];
  const fs::path out_json = argv[3];
  const int max_frames = argc >= 5 && std::string(argv[4]).rfind("--", 0) != 0
                             ? std::stoi(argv[4])
                             : 60;

  localization_zoo::ct_icp::CTICPParams params;
  params.voxel_resolution = 1.5;
  params.max_iterations = 8;
  params.ceres_max_iterations = 1;
  params.planarity_threshold = 0.08;
  params.keypoint_voxel_size = 1.25;
  params.max_frames_in_map = 8;

  int start_frame = 0;
  double source_voxel_size = 0.75;
  size_t max_source_points = 500;
  double max_step_translation = 2.0;
  double max_step_yaw_deg = 20.0;
  bool refinement_gate_enabled = false;
  bool native_ct_icp_seed = false;
  bool require_convergence = false;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;

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
    } else if (arg == "--voxel-resolution") {
      params.voxel_resolution = std::stod(needValue(arg));
    } else if (arg == "--keypoint-voxel-size") {
      params.keypoint_voxel_size = std::stod(needValue(arg));
    } else if (arg == "--max-iterations") {
      params.max_iterations = std::max(1, std::stoi(needValue(arg)));
    } else if (arg == "--ceres-max-iterations") {
      params.ceres_max_iterations = std::max(1, std::stoi(needValue(arg)));
    } else if (arg == "--max-frames-in-map") {
      params.max_frames_in_map = std::max(1, std::stoi(needValue(arg)));
    } else if (arg == "--planarity-threshold") {
      params.planarity_threshold = std::stod(needValue(arg));
    } else if (arg == "--max-correspondence-distance") {
      params.max_correspondence_dist = std::stod(needValue(arg));
    } else if (arg == "--multi-scale") {
      params.multi_scale_correspondences = true;
    } else if (arg == "--normal-cholesky") {
      params.use_normal_cholesky_solver = true;
    } else if (arg == "--refinement-gate") {
      refinement_gate_enabled = true;
    } else if (arg == "--native-ct-icp-seed") {
      native_ct_icp_seed = true;
    } else if (arg == "--require-convergence") {
      require_convergence = true;
    } else if (arg == "--max-step-translation") {
      max_step_translation = std::stod(needValue(arg));
    } else if (arg == "--max-step-yaw-deg") {
      max_step_yaw_deg = std::stod(needValue(arg));
    } else if (arg == "--max-seed-translation-delta") {
      max_seed_translation_delta = std::stod(needValue(arg));
    } else if (arg == "--max-seed-rotation-delta-rad") {
      max_seed_rotation_delta_rad = std::stod(needValue(arg));
    } else {
      throw std::runtime_error("Unknown option: " + arg);
    }
  }

  const auto pcds = collectPcds(pcd_dir, start_frame, max_frames);
  if (pcds.size() < 2) throw std::runtime_error("Need at least two PCD frames");

  localization_zoo::ct_icp::CTICPRegistration reg(params);
  localization_zoo::ct_icp::TrajectoryFrame prev;
  Eigen::Matrix4d T_prev_world = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d T_prev_prev_world = Eigen::Matrix4d::Identity();

  std::vector<Eigen::Matrix4d> poses;
  std::vector<PairLog> pairs;
  poses.reserve(pcds.size());
  pairs.reserve(pcds.size() - 1);
  poses.push_back(Eigen::Matrix4d::Identity());

  const auto t0 = Clock::now();
  for (size_t i = 0; i < pcds.size(); ++i) {
    const LoadedScan scan =
        limitLoadedScan(loadTimedPcd(pcds[i], source_voxel_size), max_source_points);
    if (scan.points.empty()) {
      std::cerr << "[CT-ICP-window] empty scan at " << pcds[i] << std::endl;
      if (i > 0) {
        PairLog log;
        log.index = static_cast<int>(i);
        pairs.push_back(log);
        poses.push_back(T_prev_world);
      }
      continue;
    }
    const auto timed = makeTimedPoints(scan);

    if (i == 0) {
      std::vector<Eigen::Vector3d> world;
      world.reserve(timed.size());
      for (const auto& tp : timed) world.push_back(prev.transformPoint(tp.raw_point, tp.timestamp));
      reg.addPointsToMap(world);
      continue;
    }

    localization_zoo::ct_icp::TrajectoryFrame init;
    Eigen::Matrix4d T_seed_world = T_prev_world;
    if (refinement_gate_enabled && i >= 2) {
      if (native_ct_icp_seed) {
        const Eigen::Matrix4d T_prev_begin = poseToMatrix(prev.begin_pose);
        const Eigen::Matrix4d T_prev_end = poseToMatrix(prev.end_pose);
        const Eigen::Matrix4d T_motion = T_prev_begin.inverse() * T_prev_end;
        T_seed_world = T_prev_end * T_motion;
      } else {
        T_seed_world = velocityModelPrediction(T_prev_world, T_prev_prev_world);
      }
      init.begin_pose = prev.end_pose;
      init.end_pose.trans = T_seed_world.block<3, 1>(0, 3);
      init.end_pose.quat = Eigen::Quaterniond(T_seed_world.block<3, 3>(0, 0));
    } else {
      init.begin_pose = init.end_pose = prev.end_pose;
    }

    const auto result = reg.registerFrame(timed, init, &prev);
    const Eigen::Matrix4d T_refined = frameToMatrix(result.frame);
    const Eigen::Matrix4d raw_delta = T_prev_world.inverse() * T_refined;
    const bool step_gate =
        passesStepGate(raw_delta, max_step_translation, max_step_yaw_deg);
    const bool refinement_gate =
        !refinement_gate_enabled || i < 2 ||
        passesRefinementGate(T_refined, T_seed_world, max_seed_translation_delta,
                             max_seed_rotation_delta_rad);
    const bool accepted =
        step_gate && refinement_gate && (!require_convergence || result.converged);

    PairLog log;
    log.index = static_cast<int>(i);
    log.raw = raw_delta;
    log.converged = result.converged;
    log.step_gate = step_gate;
    log.refinement_gate = refinement_gate;
    log.iterations = result.num_iterations;
    log.accepted = accepted;

    if (accepted) {
      std::vector<Eigen::Vector3d> world;
      world.reserve(timed.size());
      for (const auto& tp : timed) {
        world.push_back(result.frame.transformPoint(tp.raw_point, tp.timestamp));
      }
      reg.addPointsToMap(world);
      log.used = raw_delta;
      prev = result.frame;
      T_prev_prev_world = T_prev_world;
      T_prev_world = T_refined;
    } else {
      log.used = Eigen::Matrix4d::Identity();
    }

    pairs.push_back(log);
    poses.push_back(T_prev_world);

    if (i % 10 == 0) {
      std::cerr << "\r[CT-ICP-window] " << i << "/" << (pcds.size() - 1)
                << " conv=" << (result.converged ? "yes" : "no")
                << " accepted=" << (accepted ? "yes" : "no");
    }
  }
  std::cerr << std::endl;

  const double runtime_s =
      std::chrono::duration<double>(Clock::now() - t0).count();
  writeOutput(out_json, pcd_dir, gt_csv, poses, pairs, runtime_s, params,
              start_frame, refinement_gate_enabled, native_ct_icp_seed,
              max_step_translation, max_step_yaw_deg,
              max_seed_translation_delta, max_seed_rotation_delta_rad,
              require_convergence);
  std::cout << "Wrote " << out_json << " frames=" << poses.size() << std::endl;
  return 0;
}
