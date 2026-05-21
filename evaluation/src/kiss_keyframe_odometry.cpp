#include "kiss_icp/kiss_icp.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/io/pcd_io.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <algorithm>
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

namespace {

struct GTPose {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  double roll = 0.0;
  double pitch = 0.0;
  double yaw = 0.0;

  Eigen::Matrix4d matrix() const {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    const Eigen::AngleAxisd rx(roll, Eigen::Vector3d::UnitX());
    const Eigen::AngleAxisd ry(pitch, Eigen::Vector3d::UnitY());
    const Eigen::AngleAxisd rz(yaw, Eigen::Vector3d::UnitZ());
    T.block<3, 3>(0, 0) = (rz * ry * rx).toRotationMatrix();
    T.block<3, 1>(0, 3) = Eigen::Vector3d(x, y, z);
    return T;
  }
};

struct PairLog {
  int index = 0;
  Eigen::Matrix4d raw = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d used = Eigen::Matrix4d::Identity();
  bool accepted = false;
  bool converged = false;
  int correspondences = 0;
  int iterations = 0;
  double rmse = std::numeric_limits<double>::infinity();
  bool keyframe_attempted = false;
  bool keyframe_anchor_accepted = false;
  bool keyframe_accepted = false;
  int keyframe_index = 0;
  int keyframe_correspondences = 0;
  int keyframe_iterations = 0;
  double keyframe_rmse = std::numeric_limits<double>::infinity();
  double keyframe_correction_m = 0.0;
  double keyframe_correction_yaw_deg = 0.0;
  Eigen::Matrix4d keyframe_relative = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d keyframe_correction = Eigen::Matrix4d::Identity();
  std::string keyframe_decision_reason = "not_attempted";
};

std::string trim(std::string s) {
  while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
                        s.back() == ' ' || s.back() == '\t')) {
    s.pop_back();
  }
  size_t begin = 0;
  while (begin < s.size() && (s[begin] == ' ' || s[begin] == '\t')) ++begin;
  return s.substr(begin);
}

std::vector<std::string> splitCsvLine(const std::string& line) {
  std::vector<std::string> cols;
  std::istringstream ss(line);
  std::string token;
  while (std::getline(ss, token, ',')) cols.push_back(trim(token));
  return cols;
}

int indexOf(const std::vector<std::string>& cols, const std::string& name) {
  const auto it = std::find(cols.begin(), cols.end(), name);
  return it == cols.end() ? -1 : static_cast<int>(it - cols.begin());
}

std::vector<GTPose> loadGT(const fs::path& csv_path) {
  if (csv_path.string() == "-" || csv_path.empty()) return {};
  std::ifstream in(csv_path);
  if (!in.is_open()) throw std::runtime_error("Failed to open GT CSV");

  std::string line;
  if (!std::getline(in, line)) return {};
  const auto header = splitCsvLine(line);
  const int idx_x = indexOf(header, "lidar_pose.x");
  const int idx_y = indexOf(header, "lidar_pose.y");
  const int idx_z = indexOf(header, "lidar_pose.z");
  const int idx_roll = indexOf(header, "lidar_pose.roll");
  const int idx_pitch = indexOf(header, "lidar_pose.pitch");
  const int idx_yaw = indexOf(header, "lidar_pose.yaw");
  if (idx_x < 0 || idx_y < 0 || idx_z < 0 || idx_roll < 0 ||
      idx_pitch < 0 || idx_yaw < 0) {
    throw std::runtime_error("GT CSV is missing lidar_pose columns");
  }

  std::vector<GTPose> gt;
  while (std::getline(in, line)) {
    const auto vals = splitCsvLine(line);
    const int max_idx =
        std::max({idx_x, idx_y, idx_z, idx_roll, idx_pitch, idx_yaw});
    if (static_cast<int>(vals.size()) <= max_idx) continue;
    GTPose pose;
    pose.x = std::stod(vals[idx_x]);
    pose.y = std::stod(vals[idx_y]);
    pose.z = std::stod(vals[idx_z]);
    pose.roll = std::stod(vals[idx_roll]);
    pose.pitch = std::stod(vals[idx_pitch]);
    pose.yaw = std::stod(vals[idx_yaw]);
    gt.push_back(pose);
  }
  return gt;
}

std::vector<Eigen::Vector3d> loadPcd(const fs::path& path) {
  pcl::PointCloud<pcl::PointXYZI> cloud;
  if (pcl::io::loadPCDFile<pcl::PointXYZI>(path.string(), cloud) != 0) {
    throw std::runtime_error("Failed to load PCD: " + path.string());
  }
  std::vector<Eigen::Vector3d> points;
  points.reserve(cloud.size());
  for (const auto& p : cloud.points) {
    if (std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z)) {
      points.emplace_back(p.x, p.y, p.z);
    }
  }
  return points;
}

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

double yawDeg(const Eigen::Matrix4d& T) {
  return std::atan2(T(1, 0), T(0, 0)) * 180.0 / M_PI;
}

bool passesGate(const Eigen::Matrix4d& T, double max_translation,
                double max_yaw_deg) {
  if (T.block<3, 1>(0, 3).norm() > max_translation) return false;
  if (std::abs(yawDeg(T)) > max_yaw_deg) return false;
  return T.array().isFinite().all();
}

std::string keyframeDecisionReason(
    bool attempted, const localization_zoo::kiss_icp::KISSMatcherResult& result,
    const Eigen::Matrix4d& correction, double correction_m,
    double correction_yaw_deg, double max_keyframe_correction,
    double max_keyframe_yaw_deg, double max_keyframe_rmse,
    int min_keyframe_correspondences) {
  if (!attempted) return "not_attempted";
  if (!result.converged) return "anchor_rejected";
  if (correction_m > max_keyframe_correction) {
    return "correction_translation_gate";
  }
  if (correction_yaw_deg > max_keyframe_yaw_deg) return "correction_yaw_gate";
  if (!std::isfinite(result.rmse) || result.rmse > max_keyframe_rmse) {
    return "rmse_gate";
  }
  if (result.num_correspondences < min_keyframe_correspondences) {
    return "correspondence_gate";
  }
  if (!correction.array().isFinite().all()) return "missing_correction_transform";
  return "accepted";
}

double computeAteXY(const std::vector<Eigen::Matrix4d>& poses,
                    const std::vector<GTPose>& gt) {
  const int n = std::min(poses.size(), gt.size());
  if (n == 0) return std::numeric_limits<double>::quiet_NaN();
  const Eigen::Matrix4d gt0_inv = gt.front().matrix().inverse();
  double sum_sq = 0.0;
  for (int i = 0; i < n; ++i) {
    const Eigen::Matrix4d gt_rel = gt0_inv * gt[i].matrix();
    const Eigen::Vector2d err =
        poses[i].block<2, 1>(0, 3) - gt_rel.block<2, 1>(0, 3);
    sum_sq += err.squaredNorm();
  }
  return std::sqrt(sum_sq / n);
}

double computeStepRmseXY(const std::vector<Eigen::Matrix4d>& poses,
                         const std::vector<GTPose>& gt) {
  const int n = std::min(poses.size(), gt.size());
  if (n < 2) return std::numeric_limits<double>::quiet_NaN();
  const Eigen::Matrix4d gt0_inv = gt.front().matrix().inverse();
  double sum_sq = 0.0;
  for (int i = 1; i < n; ++i) {
    const double est_step =
        (poses[i].block<2, 1>(0, 3) - poses[i - 1].block<2, 1>(0, 3)).norm();
    const Eigen::Matrix4d gt_prev = gt0_inv * gt[i - 1].matrix();
    const Eigen::Matrix4d gt_curr = gt0_inv * gt[i].matrix();
    const double gt_step =
        (gt_curr.block<2, 1>(0, 3) - gt_prev.block<2, 1>(0, 3)).norm();
    const double err = est_step - gt_step;
    sum_sq += err * err;
  }
  return std::sqrt(sum_sq / (n - 1));
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
                 const std::vector<PairLog>& pairs,
                 const std::vector<GTPose>& gt,
                 const localization_zoo::kiss_icp::KISSMatcherParams& params,
                 int keyframe_interval, double max_step_translation,
                 double max_step_yaw_deg, double max_keyframe_correction,
                 double max_keyframe_yaw_deg, double max_keyframe_rmse,
                 int min_keyframe_correspondences) {
  fs::create_directories(out_path.parent_path());
  std::ofstream out(out_path);
  const double ate_xy = computeAteXY(poses, gt);
  const double step_rmse = computeStepRmseXY(poses, gt);

  out << "{\n";
  out << "  \"sequence_pcd_dir\": \"" << pcd_dir.string() << "\",\n";
  out << "  \"gt_csv\": \"" << gt_csv.string() << "\",\n";
  out << "  \"frames\": " << poses.size() << ",\n";
  out << "  \"method\": \"kiss_keyframe_anchor_odometry\",\n";
  out << "  \"parameters\": {\n";
  out << "    \"keyframe_interval\": " << keyframe_interval << ",\n";
  out << "    \"target_voxel_size\": " << params.target_voxel_size << ",\n";
  out << "    \"source_voxel_size\": " << params.source_voxel_size << ",\n";
  out << "    \"max_correspondence_distance\": "
      << params.max_correspondence_distance << ",\n";
  out << "    \"max_icp_iterations\": " << params.max_icp_iterations << ",\n";
  out << "    \"min_correspondences\": " << params.min_correspondences << ",\n";
  out << "    \"max_step_translation\": " << max_step_translation << ",\n";
  out << "    \"max_step_yaw_deg\": " << max_step_yaw_deg << ",\n";
  out << "    \"max_keyframe_correction\": " << max_keyframe_correction << ",\n";
  out << "    \"max_keyframe_yaw_deg\": " << max_keyframe_yaw_deg << ",\n";
  out << "    \"max_keyframe_rmse\": " << max_keyframe_rmse << ",\n";
  out << "    \"min_keyframe_correspondences\": "
      << min_keyframe_correspondences << "\n";
  out << "  },\n";
  out << "  \"metrics\": {\n";
  out << "    \"ate_xy_m\": ";
  writeJsonNumber(out, ate_xy);
  out << ",\n";
  out << "    \"step_length_rmse_m\": ";
  writeJsonNumber(out, step_rmse);
  out << "\n";
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
    out << "      \"score\": ";
    writeJsonNumber(out, std::isfinite(p.rmse) ? 1.0 / (1.0 + p.rmse) : -1.0);
    out << ",\n";
    out << "      \"overlap\": " << p.correspondences << ",\n";
    out << "      \"converged\": " << (p.converged ? "true" : "false")
        << ",\n";
    out << "      \"iterations\": " << p.iterations << ",\n";
    out << "      \"rmse_m\": ";
    writeJsonNumber(out, p.rmse);
    out << ",\n";
    out << "      \"keyframe_attempted\": "
        << (p.keyframe_attempted ? "true" : "false") << ",\n";
    out << "      \"keyframe_anchor_accepted\": "
        << (p.keyframe_anchor_accepted ? "true" : "false") << ",\n";
    out << "      \"keyframe_accepted\": "
        << (p.keyframe_accepted ? "true" : "false") << ",\n";
    out << "      \"keyframe_decision_reason\": \""
        << p.keyframe_decision_reason << "\",\n";
    out << "      \"keyframe_index\": " << p.keyframe_index << ",\n";
    out << "      \"keyframe_correspondences\": " << p.keyframe_correspondences
        << ",\n";
    out << "      \"keyframe_iterations\": " << p.keyframe_iterations << ",\n";
    out << "      \"keyframe_rmse_m\": ";
    writeJsonNumber(out, p.keyframe_rmse);
    out << ",\n";
    out << "      \"keyframe_correction_m\": ";
    writeJsonNumber(out, p.keyframe_correction_m);
    out << ",\n";
    out << "      \"keyframe_correction_yaw_deg\": ";
    writeJsonNumber(out, p.keyframe_correction_yaw_deg);
    out << ",\n";
    out << "      \"keyframe_relative_dx_m\": ";
    writeJsonNumber(out, p.keyframe_relative(0, 3));
    out << ",\n";
    out << "      \"keyframe_relative_dy_m\": ";
    writeJsonNumber(out, p.keyframe_relative(1, 3));
    out << ",\n";
    out << "      \"keyframe_relative_yaw_deg\": ";
    writeJsonNumber(out, yawDeg(p.keyframe_relative));
    out << ",\n";
    out << "      \"keyframe_correction_dx_m\": ";
    writeJsonNumber(out, p.keyframe_correction(0, 3));
    out << ",\n";
    out << "      \"keyframe_correction_dy_m\": ";
    writeJsonNumber(out, p.keyframe_correction(1, 3));
    out << ",\n";
    out << "      \"keyframe_correction_transform_yaw_deg\": ";
    writeJsonNumber(out, yawDeg(p.keyframe_correction));
    out << "\n";
    out << "    }";
    out << (i + 1 == pairs.size() ? "\n" : ",\n");
  }
  out << "  ]\n";
  out << "}\n";
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cerr << "Usage: kiss_keyframe_odometry <pcd_dir> <gt_csv> "
                 "<output_json> [max_frames] [--keyframe-interval N] "
                 "[--start-frame N] "
                 "[--max-keyframe-correction X] [--max-keyframe-yaw-deg X] "
                 "[--max-keyframe-rmse X] [--min-keyframe-correspondences N] "
                 "[--target-voxel-size X] "
                 "[--source-voxel-size X] [--max-correspondence-distance X] "
                 "[--max-step-translation X] [--max-step-yaw-deg X] "
                 "[--max-iterations N] [--min-correspondences N]\n";
    return 1;
  }

  const fs::path pcd_dir = argv[1];
  const fs::path gt_csv = argv[2];
  const fs::path out_json = argv[3];
  const int max_frames = argc >= 5 && std::string(argv[4]).rfind("--", 0) != 0
                             ? std::stoi(argv[4])
                             : 60;

  localization_zoo::kiss_icp::KISSMatcherParams params;
  int keyframe_interval = 10;
  int start_frame = 0;
  double max_step_translation = 0.3;
  double max_step_yaw_deg = 6.0;
  double max_keyframe_correction = 1.0;
  double max_keyframe_yaw_deg = 5.0;
  double max_keyframe_rmse = 1.2;
  int min_keyframe_correspondences = 6000;

  const int opt_begin =
      argc >= 5 && std::string(argv[4]).rfind("--", 0) != 0 ? 5 : 4;
  for (int i = opt_begin; i < argc; ++i) {
    const std::string arg = argv[i];
    auto needValue = [&](const std::string& name) {
      if (i + 1 >= argc) throw std::runtime_error(name + " requires a value");
      return std::string(argv[++i]);
    };
    if (arg == "--keyframe-interval") {
      keyframe_interval = std::max(2, std::stoi(needValue(arg)));
    } else if (arg == "--start-frame") {
      start_frame = std::max(0, std::stoi(needValue(arg)));
    } else if (arg == "--max-keyframe-correction") {
      max_keyframe_correction = std::stod(needValue(arg));
    } else if (arg == "--max-keyframe-yaw-deg") {
      max_keyframe_yaw_deg = std::stod(needValue(arg));
    } else if (arg == "--max-keyframe-rmse") {
      max_keyframe_rmse = std::stod(needValue(arg));
    } else if (arg == "--min-keyframe-correspondences") {
      min_keyframe_correspondences = std::stoi(needValue(arg));
    } else if (arg == "--target-voxel-size") {
      params.target_voxel_size = std::stod(needValue(arg));
    } else if (arg == "--source-voxel-size") {
      params.source_voxel_size = std::stod(needValue(arg));
    } else if (arg == "--max-correspondence-distance") {
      params.max_correspondence_distance = std::stod(needValue(arg));
    } else if (arg == "--max-step-translation") {
      max_step_translation = std::stod(needValue(arg));
    } else if (arg == "--max-step-yaw-deg") {
      max_step_yaw_deg = std::stod(needValue(arg));
    } else if (arg == "--max-iterations") {
      params.max_icp_iterations = std::stoi(needValue(arg));
    } else if (arg == "--min-correspondences") {
      params.min_correspondences = std::stoi(needValue(arg));
    } else {
      throw std::runtime_error("Unknown argument: " + arg);
    }
  }

  const auto pcds = collectPcds(pcd_dir, start_frame, max_frames);
  if (pcds.size() < 2) throw std::runtime_error("Need at least two PCD frames");
  const auto gt = loadGT(gt_csv);

  localization_zoo::kiss_icp::KISSMatcher matcher(params);
  std::vector<Eigen::Vector3d> prev = loadPcd(pcds.front());
  std::vector<Eigen::Vector3d> keyframe_cloud = prev;
  int keyframe_index = 0;
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d keyframe_pose = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d local_prior = Eigen::Matrix4d::Identity();
  std::vector<Eigen::Matrix4d> poses{pose};
  std::vector<PairLog> pairs;

  for (size_t i = 1; i < pcds.size(); ++i) {
    const auto curr = loadPcd(pcds[i]);
    matcher.setTarget(prev);
    const auto local_result = matcher.align(curr, local_prior);

    PairLog log;
    log.index = static_cast<int>(i);
    log.raw = local_result.transform;
    log.converged = local_result.converged;
    log.correspondences = local_result.num_correspondences;
    log.iterations = local_result.iterations;
    log.rmse = local_result.rmse;
    log.accepted = local_result.converged &&
                   passesGate(local_result.transform, max_step_translation,
                              max_step_yaw_deg);

    if (log.accepted) {
      log.used = local_result.transform;
      local_prior = local_result.transform;
    } else if (passesGate(local_prior, max_step_translation, max_step_yaw_deg)) {
      log.used = local_prior;
    } else {
      log.used = Eigen::Matrix4d::Identity();
      local_prior = Eigen::Matrix4d::Identity();
    }

    pose = pose * log.used;

    if (static_cast<int>(i) - keyframe_index >= keyframe_interval) {
      log.keyframe_attempted = true;
      log.keyframe_index = keyframe_index;
      const Eigen::Matrix4d relative_prior = keyframe_pose.inverse() * pose;
      matcher.setTarget(keyframe_cloud);
      const auto keyframe_result = matcher.align(curr, relative_prior);
      const Eigen::Matrix4d anchored_pose =
          keyframe_pose * keyframe_result.transform;
      const Eigen::Matrix4d correction = anchored_pose * pose.inverse();
      log.keyframe_relative = keyframe_result.transform;
      log.keyframe_correction = correction;
      log.keyframe_anchor_accepted = keyframe_result.converged;
      log.keyframe_correspondences = keyframe_result.num_correspondences;
      log.keyframe_iterations = keyframe_result.iterations;
      log.keyframe_rmse = keyframe_result.rmse;
      log.keyframe_correction_m = correction.block<3, 1>(0, 3).norm();
      log.keyframe_correction_yaw_deg = std::abs(yawDeg(correction));
      log.keyframe_decision_reason = keyframeDecisionReason(
          log.keyframe_attempted, keyframe_result, correction,
          log.keyframe_correction_m, log.keyframe_correction_yaw_deg,
          max_keyframe_correction, max_keyframe_yaw_deg, max_keyframe_rmse,
          min_keyframe_correspondences);
      log.keyframe_accepted = log.keyframe_decision_reason == "accepted";
      if (log.keyframe_accepted) {
        pose = anchored_pose;
      }
      keyframe_cloud = curr;
      keyframe_pose = pose;
      keyframe_index = static_cast<int>(i);
    }

    poses.push_back(pose);
    pairs.push_back(log);
    prev = curr;

    std::cout << "\r[KISS-keyframe] " << i << "/" << pcds.size() - 1
              << " local_rmse=" << std::fixed << std::setprecision(3)
              << log.rmse << " kf="
              << (log.keyframe_attempted
                      ? (log.keyframe_accepted ? "accepted" : "rejected")
                      : "skip")
              << std::flush;
  }
  std::cout << std::endl;

  writeOutput(out_json, pcd_dir, gt_csv, poses, pairs, gt, params,
              keyframe_interval, max_step_translation, max_step_yaw_deg,
              max_keyframe_correction, max_keyframe_yaw_deg, max_keyframe_rmse,
              min_keyframe_correspondences);
  std::cout << "Wrote " << out_json << " ATE_xy="
            << computeAteXY(poses, gt) << " step_rmse="
            << computeStepRmseXY(poses, gt) << std::endl;
  return 0;
}
