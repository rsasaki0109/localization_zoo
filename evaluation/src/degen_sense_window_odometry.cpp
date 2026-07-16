// GT-free window-odometry runner for the DegenSense pipeline
// (papers/degen_sense), with IMU support.
//
// Structurally this mirrors evaluation/src/xicp_window_odometry.cpp (CLI
// shape, PCD loading from the dogfooding directory layout, step gate,
// poses_xyyaw / pairs JSON output). It differs in one important way:
// DegenSensePipeline owns its own internal voxel map AND its own absolute
// pose (see papers/degen_sense/include/degen_sense/degen_sense.h). The
// runner therefore does not maintain a map or a "used" transform of its
// own -- it just feeds each frame (+ the IMU samples straddling it) to
// pipeline.registerFrame() and records whatever pose comes back.
//
// IMPORTANT: the step gate here is diagnostic-only. Because the pipeline
// is a black box that has already folded the new scan into its internal
// map and advanced its internal pose by the time registerFrame() returns,
// there is no way for this runner to "reject" a step and roll the
// pipeline back to a previous state. So when a per-frame delta fails the
// step gate we still record it as accepted=false / step_gate=false in the
// pair log (and it counts as "rejected" in the accepted-count stats an
// operator would compute from the JSON), but we keep using the pipeline's
// actual returned pose for poses_xyyaw and keep feeding it subsequent
// frames -- there is no alternative trajectory to fall back to.
//
// IMU handling mirrors runDegenSense() / selectImuWindow() in
// pcd_dogfooding.cpp: imu.csv has columns
//   stamp,angular_vel.x,angular_vel.y,angular_vel.z,linear_acc.x,linear_acc.y,linear_acc.z
// (epoch seconds, rad/s, m/s^2). For frame i (i>=1) we take every IMU
// sample with frame_timestamps[i-1] <= stamp <= frame_timestamps[i] from
// frame_timestamps.csv (matched by the PCD directory's global frame
// index, i.e. the zero-padded directory name), matching selectImuWindow's
// half-open-ish inclusive scan (skip earlier, break on first sample past
// end_time -- valid because imu.csv is monotonically increasing).

#include "degen_sense/degen_sense.h"
#include "imu_preintegration/imu_preintegration.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
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
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::high_resolution_clock;
using localization_zoo::degen_sense::DegenSenseParams;
using localization_zoo::degen_sense::DegenSensePipeline;
using localization_zoo::degen_sense::DegenSenseResult;
using localization_zoo::imu_preintegration::ImuSample;

namespace {

// Runner-side source-cloud preprocessing constants. DegenSensePipeline does
// its own range filtering and voxel downsampling internally (using
// params.min_range/max_range/voxel_size), so these only exist to bound the
// point count handed to the pipeline per frame, mirroring the fixed
// r in (1.0, 80.0) clamp and source_voxel_size/max_source_points defaults
// that pcd_dogfooding.cpp's runDegenSense() uses (loadPCD() + the
// DegenSenseDogfoodingOptions defaults at pcd_dogfooding.cpp:2438).
constexpr double kSourceMinRange = 1.0;
constexpr double kSourceMaxRange = 80.0;
constexpr double kSourceVoxelSize = 0.5;
constexpr size_t kMaxSourcePoints = 4500;

struct ImuSampleCsv {
  double timestamp = 0.0;
  Eigen::Vector3d gyro = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel = Eigen::Vector3d::Zero();
};

struct PairLog {
  int index = 0;
  Eigen::Matrix4d raw = Eigen::Matrix4d::Identity();
  bool accepted = false;
  bool converged = false;
  bool step_gate = false;
  int iterations = 0;
  bool used_imu = false;
  size_t num_imu_samples = 0;
  double degeneracy_factor_rot = 1.0;
  double degeneracy_factor_trans = 1.0;
  bool degenerate = false;
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

struct FrameEntry {
  int global_index = 0;
  fs::path path;
};

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
                                        double min_range, double max_range) {
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
    if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) {
      continue;
    }
    const double r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    if (r < min_range || r > max_range) continue;
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

// Header format: stamp,angular_vel.x,angular_vel.y,angular_vel.z,
//                linear_acc.x,linear_acc.y,linear_acc.z
// (epoch seconds; rad/s; m/s^2). Mirrors loadImuCsv() in pcd_dogfooding.cpp.
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
// selectImuWindow() in pcd_dogfooding.cpp.
std::vector<ImuSample> selectImuWindow(const std::vector<ImuSampleCsv>& imu_samples,
                                       double start_time, double end_time) {
  std::vector<ImuSample> out;
  for (const auto& sample : imu_samples) {
    if (sample.timestamp < start_time) continue;
    if (sample.timestamp > end_time) break;
    ImuSample imu;
    imu.timestamp = sample.timestamp;
    imu.gyro = sample.gyro;
    imu.accel = sample.accel;
    out.push_back(imu);
  }
  return out;
}

// Loads frame_timestamps.csv (columns: frame_idx,timestamp,...) into a map
// keyed by the *global* frame index (the zero-padded PCD directory name),
// so it can be looked up regardless of --start-frame / max_frames slicing.
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

double yawDeg(const Eigen::Matrix4d& T) {
  return std::atan2(T(1, 0), T(0, 0)) * 180.0 / M_PI;
}

double rotationDeltaRad(const Eigen::Matrix4d& lhs, const Eigen::Matrix4d& rhs) {
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

void writeJsonNumber(std::ostream& out, double value) {
  if (std::isfinite(value)) {
    out << std::fixed << std::setprecision(6) << value;
  } else {
    out << "null";
  }
}

void writeOutput(const fs::path& out_path, const fs::path& pcd_dir,
                 const fs::path& gt_csv, const std::vector<Eigen::Matrix4d>& poses,
                 const std::vector<PairLog>& pairs, double runtime_s,
                 const DegenSenseParams& params, int start_frame, bool use_imu,
                 const fs::path& imu_csv_path, double max_step_translation,
                 double max_step_yaw_deg) {
  fs::create_directories(out_path.parent_path());
  std::ofstream out(out_path);

  size_t accepted_count = 0;
  size_t used_imu_count = 0;
  size_t degenerate_count = 0;
  for (const auto& p : pairs) {
    if (p.accepted) ++accepted_count;
    if (p.used_imu) ++used_imu_count;
    if (p.degenerate) ++degenerate_count;
  }

  out << "{\n";
  out << "  \"sequence_pcd_dir\": \"" << pcd_dir.string() << "\",\n";
  out << "  \"gt_csv\": \"" << gt_csv.string() << "\",\n";
  out << "  \"frames\": " << poses.size() << ",\n";
  out << "  \"method\": \"degen_sense\",\n";
  out << "  \"runtime_s\": ";
  writeJsonNumber(out, runtime_s);
  out << ",\n";
  out << "  \"parameters\": {\n";
  out << "    \"start_frame\": " << start_frame << ",\n";
  out << "    \"source_voxel_size\": " << kSourceVoxelSize << ",\n";
  out << "    \"max_source_points\": " << kMaxSourcePoints << ",\n";
  out << "    \"source_min_range\": " << kSourceMinRange << ",\n";
  out << "    \"source_max_range\": " << kSourceMaxRange << ",\n";
  out << "    \"voxel_size\": " << params.voxel_size << ",\n";
  out << "    \"max_range\": " << params.max_range << ",\n";
  out << "    \"min_range\": " << params.min_range << ",\n";
  out << "    \"max_points_per_voxel\": " << params.max_points_per_voxel << ",\n";
  out << "    \"normal_min_neighbors\": " << params.normal_min_neighbors << ",\n";
  out << "    \"planarity_threshold\": " << params.planarity_threshold << ",\n";
  out << "    \"max_icp_iterations\": " << params.max_icp_iterations << ",\n";
  out << "    \"convergence_criterion\": " << params.convergence_criterion << ",\n";
  out << "    \"initial_threshold\": " << params.initial_threshold << ",\n";
  out << "    \"warmup_frames\": " << params.warmup_frames << ",\n";
  out << "    \"buffer_size\": " << params.buffer_size << ",\n";
  out << "    \"mad_k\": " << params.mad_k << ",\n";
  out << "    \"min_fusion_weight\": " << params.min_fusion_weight << ",\n";
  out << "    \"gyro_bias_gain\": " << params.gyro_bias_gain << ",\n";
  out << "    \"max_gyro_bias\": " << params.max_gyro_bias << ",\n";
  out << "    \"local_map_radius\": " << params.local_map_radius << ",\n";
  out << "    \"map_cleanup_interval\": " << params.map_cleanup_interval << ",\n";
  out << "    \"use_imu\": " << (use_imu ? "true" : "false") << ",\n";
  out << "    \"imu_csv\": \"" << imu_csv_path.string() << "\",\n";
  out << "    \"max_step_translation\": " << max_step_translation << ",\n";
  out << "    \"max_step_yaw_deg\": " << max_step_yaw_deg << "\n";
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
    writeJsonNumber(out, p.raw(0, 3));
    out << ",\n";
    out << "      \"used_dy_curr_to_prev_m\": ";
    writeJsonNumber(out, p.raw(1, 3));
    out << ",\n";
    out << "      \"used_yaw_curr_to_prev_deg\": ";
    writeJsonNumber(out, yawDeg(p.raw));
    out << ",\n";
    out << "      \"converged\": " << (p.converged ? "true" : "false") << ",\n";
    out << "      \"step_gate\": " << (p.step_gate ? "true" : "false") << ",\n";
    out << "      \"iterations\": " << p.iterations << ",\n";
    out << "      \"used_imu\": " << (p.used_imu ? "true" : "false") << ",\n";
    out << "      \"num_imu_samples\": " << p.num_imu_samples << ",\n";
    out << "      \"degeneracy_factor_rot\": ";
    writeJsonNumber(out, p.degeneracy_factor_rot);
    out << ",\n";
    out << "      \"degeneracy_factor_trans\": ";
    writeJsonNumber(out, p.degeneracy_factor_trans);
    out << ",\n";
    out << "      \"degenerate\": " << (p.degenerate ? "true" : "false") << "\n";
    out << "    }";
    out << (i + 1 == pairs.size() ? "\n" : ",\n");
  }
  out << "  ]\n";
  out << "}\n";
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cerr
        << "Usage: degen_sense_window_odometry <pcd_dir> <gt_csv> "
           "<output_json> [max_frames] [--start-frame N] "
           "[--imu-csv PATH] [--no-imu] [--voxel-size X] "
           "[--max-icp-iterations N] [--warmup-frames N] [--mad-k X] "
           "[--min-fusion-weight X] [--local-map-radius X] "
           "[--map-cleanup-interval N] [--max-step-translation X] "
           "[--max-step-yaw-deg X]\n"
           "  gt_csv may be '-' for GT-free operation (no ATE is ever "
           "computed by this runner; gt_csv is recorded for bookkeeping "
           "only).\n"
           "  max_frames <= 0 (e.g. -1) means: use all available frames "
           "from start_frame onward.\n"
           "  --imu-csv defaults to <pcd_dir>/imu.csv if present. "
           "--no-imu disables IMU entirely (the pipeline still reports "
           "degeneracy factors; it just has nothing to fuse with).\n";
    return 1;
  }

  const fs::path pcd_dir = argv[1];
  const fs::path gt_csv = argv[2];
  const fs::path out_json = argv[3];
  const int max_frames = argc >= 5 && std::string(argv[4]).rfind("--", 0) != 0
                             ? std::stoi(argv[4])
                             : -1;

  // DegenSenseParams defaults tuned to match the dogfooding harness's
  // DegenSenseDogfoodingOptions base defaults (pcd_dogfooding.cpp:2438),
  // rather than the raw DegenSenseParams struct defaults in
  // degen_sense.h -- those raw defaults (e.g. max_icp_iterations=100,
  // initial_threshold=2.0, no map radius/cleanup) are tuned for small
  // bounded scenes and are both slower and prone to unbounded internal
  // map growth on a multi-thousand-frame outdoor sequence like the
  // degeneracy tunnel. Differences from degen_sense.h defaults:
  //   max_points_per_voxel   20   -> 12
  //   max_icp_iterations    100   -> 30
  //   initial_threshold      2.0  -> 1.5
  //   local_map_radius        0.0 -> 60.0  (0 = unbounded map)
  //   map_cleanup_interval    0   -> 4     (0 = never prune)
  DegenSenseParams params;
  params.max_points_per_voxel = 12;
  params.max_icp_iterations = 30;
  params.initial_threshold = 1.5;
  params.local_map_radius = 60.0;
  params.map_cleanup_interval = 4;

  int start_frame = 0;
  bool use_imu = true;
  fs::path imu_csv_override;
  bool imu_csv_overridden = false;
  double max_step_translation = 2.0;
  double max_step_yaw_deg = 20.0;

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
    } else if (arg == "--voxel-size") {
      params.voxel_size = std::stod(needValue(arg));
    } else if (arg == "--max-icp-iterations") {
      params.max_icp_iterations = std::max(1, std::stoi(needValue(arg)));
    } else if (arg == "--warmup-frames") {
      params.warmup_frames = std::max(0, std::stoi(needValue(arg)));
    } else if (arg == "--mad-k") {
      params.mad_k = std::stod(needValue(arg));
    } else if (arg == "--min-fusion-weight") {
      params.min_fusion_weight = std::stod(needValue(arg));
    } else if (arg == "--local-map-radius") {
      params.local_map_radius = std::stod(needValue(arg));
    } else if (arg == "--map-cleanup-interval") {
      params.map_cleanup_interval = std::max(0, std::stoi(needValue(arg)));
    } else if (arg == "--max-step-translation") {
      max_step_translation = std::stod(needValue(arg));
    } else if (arg == "--max-step-yaw-deg") {
      max_step_yaw_deg = std::stod(needValue(arg));
    } else {
      throw std::runtime_error("Unknown option: " + arg);
    }
  }

  const auto pcds = collectPcds(pcd_dir, start_frame, max_frames);
  if (pcds.size() < 2) throw std::runtime_error("Need at least two PCD frames");

  fs::path imu_csv_path = imu_csv_overridden ? imu_csv_override : (pcd_dir / "imu.csv");
  std::vector<ImuSampleCsv> imu_samples;
  std::unordered_map<int, double> frame_timestamps;
  if (use_imu && fs::is_regular_file(imu_csv_path)) {
    imu_samples = loadImuCsv(imu_csv_path);
    frame_timestamps = loadFrameTimestamps(pcd_dir);
  }
  const bool imu_available = use_imu && !imu_samples.empty() &&
                             !frame_timestamps.empty();
  std::cerr << "[DegenSense-window] imu.csv=" << imu_csv_path
            << " samples=" << imu_samples.size()
            << " frame_timestamps=" << frame_timestamps.size()
            << " imu_available=" << (imu_available ? "yes" : "no")
            << std::endl;

  DegenSensePipeline pipeline(params);
  // GT-free: no setInitialPose() call, pipeline starts at identity like any
  // other window-odometry runner in this repo when gt_csv == "-".

  Eigen::Matrix4d T_prev_world = Eigen::Matrix4d::Identity();

  std::vector<Eigen::Matrix4d> poses;
  std::vector<PairLog> pairs;
  poses.reserve(pcds.size());
  pairs.reserve(pcds.size() - 1);

  const auto t0 = Clock::now();
  for (size_t i = 0; i < pcds.size(); ++i) {
    auto points = limitPoints(
        loadPcdXYZ(pcds[i].path, kSourceVoxelSize, kSourceMinRange, kSourceMaxRange),
        kMaxSourcePoints);
    if (points.empty()) {
      std::cerr << "[DegenSense-window] empty scan at " << pcds[i].path
                << std::endl;
      if (i > 0) {
        PairLog log;
        log.index = static_cast<int>(i);
        pairs.push_back(log);
        poses.push_back(T_prev_world);
      }
      continue;
    }

    std::vector<ImuSample> imu_batch;
    if (i > 0 && imu_available) {
      const auto it_prev = frame_timestamps.find(pcds[i - 1].global_index);
      const auto it_curr = frame_timestamps.find(pcds[i].global_index);
      if (it_prev != frame_timestamps.end() && it_curr != frame_timestamps.end()) {
        imu_batch = selectImuWindow(imu_samples, it_prev->second, it_curr->second);
      }
    }

    const DegenSenseResult result = pipeline.registerFrame(points, imu_batch);

    if (i == 0) {
      // First call to registerFrame() just seeds the internal map and
      // returns the (identity) initial pose -- no motion to log yet.
      poses.push_back(result.pose);
      T_prev_world = result.pose;
      continue;
    }

    PairLog log;
    log.index = static_cast<int>(i);
    log.raw = T_prev_world.inverse() * result.pose;
    log.converged = result.converged;
    log.iterations = result.iterations;
    log.used_imu = result.used_imu;
    log.num_imu_samples = imu_batch.size();
    log.degeneracy_factor_rot = result.degeneracy_factor_rot;
    log.degeneracy_factor_trans = result.degeneracy_factor_trans;
    log.degenerate = result.degenerate;

    // Diagnostic-only step gate: the pipeline has already committed this
    // frame to its internal map/pose, so a failing gate cannot roll it
    // back. We still record accepted=false / step_gate=false so a
    // downstream stats pass can count this frame as "rejected", but we
    // keep using the pipeline's actual pose for the trajectory and keep
    // feeding it subsequent frames (see file header comment).
    const bool step_gate =
        passesStepGate(log.raw, max_step_translation, max_step_yaw_deg);
    log.step_gate = step_gate;
    log.accepted = step_gate;

    pairs.push_back(log);
    T_prev_world = result.pose;
    poses.push_back(T_prev_world);

    if (i % 10 == 0) {
      std::cerr << "\r[DegenSense-window] " << i << "/" << (pcds.size() - 1)
                << " conv=" << (log.converged ? "yes" : "no")
                << " accepted=" << (log.accepted ? "yes" : "no")
                << " used_imu=" << (log.used_imu ? "yes" : "no")
                << " degenerate=" << (log.degenerate ? "yes" : "no")
                << " map=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;

  const double runtime_s =
      std::chrono::duration<double>(Clock::now() - t0).count();
  writeOutput(out_json, pcd_dir, gt_csv, poses, pairs, runtime_s, params,
              start_frame, use_imu, imu_csv_path, max_step_translation,
              max_step_yaw_deg);
  std::cout << "Wrote " << out_json << " frames=" << poses.size() << std::endl;
  return 0;
}
