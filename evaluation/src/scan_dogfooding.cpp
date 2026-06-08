/// 2D laser scan dogfooding tool
///
/// Usage:
///   ./scan_dogfooding <scan_dir> <gt_csv> [max_frames] [--methods rf2o,pl_icp,csm,kinematic_icp,psm,ndt_2d,idc]
///
/// scan_dir layout:
///   scan_meta.json  (angle_min, angle_max, angle_increment, range_min, range_max, scan_rate_hz)
///   00000000/scan.csv  (comma-separated ranges)
///   00000001/scan.csv
/// gt_csv: timestamp,x,y,yaw

#include "csm/csm.h"
#include "kinematic_icp/kinematic_icp.h"
#include "pl_icp/pl_icp.h"
#include "idc/idc.h"
#include "ndt_2d/ndt_2d.h"
#include "psm/psm.h"
#include "rf2o/rf2o.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;

namespace {

struct GTPose2D {
  double timestamp = 0.0;
  double x = 0.0;
  double y = 0.0;
  double yaw = 0.0;

  Eigen::Matrix4d matrix4() const {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) =
        Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    T(0, 3) = x;
    T(1, 3) = y;
    return T;
  }
};

std::string trim(std::string s) {
  while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' '))
    s.pop_back();
  size_t b = 0;
  while (b < s.size() && s[b] == ' ') ++b;
  return s.substr(b);
}

std::vector<std::string> splitCsv(const std::string& line) {
  std::vector<std::string> out;
  std::istringstream ss(line);
  std::string tok;
  while (std::getline(ss, tok, ',')) out.push_back(trim(tok));
  return out;
}

std::vector<GTPose2D> loadGT(const fs::path& csv) {
  std::ifstream in(csv);
  if (!in) throw std::runtime_error("failed to open gt csv");
  std::string header;
  std::getline(in, header);
  const auto cols = splitCsv(header);
  auto idx = [&](const char* name) {
    const auto it = std::find(cols.begin(), cols.end(), name);
    return it == cols.end() ? -1 : static_cast<int>(it - cols.begin());
  };
  const int ix = idx("x"), iy = idx("y"), iyaw = idx("yaw"), its = idx("timestamp");
  std::vector<GTPose2D> poses;
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty()) continue;
    const auto c = splitCsv(line);
    GTPose2D p;
    if (its >= 0 && its < static_cast<int>(c.size())) p.timestamp = std::stod(c[its]);
    if (ix >= 0) p.x = std::stod(c[ix]);
    if (iy >= 0) p.y = std::stod(c[iy]);
    if (iyaw >= 0) p.yaw = std::stod(c[iyaw]);
    poses.push_back(p);
  }
  return poses;
}

localization_zoo::kinematic_icp::WheelOdom wheelOdomFromGT(const GTPose2D& prev,
                                                           const GTPose2D& cur) {
  localization_zoo::kinematic_icp::WheelOdom odom;
  const double dx = cur.x - prev.x;
  const double dy = cur.y - prev.y;
  const double c = std::cos(prev.yaw);
  const double s = std::sin(prev.yaw);
  odom.forward_m = c * dx + s * dy;
  odom.yaw_rad = cur.yaw - prev.yaw;
  return odom;
}

struct ScanMeta {
  double angle_min = -M_PI;
  double angle_max = M_PI;
  double angle_increment = 0.0;
  double range_min = 0.1;
  double range_max = 30.0;
  double scan_rate_hz = 10.0;
};

ScanMeta loadMeta(const fs::path& scan_dir) {
  ScanMeta meta;
  std::ifstream in(scan_dir / "scan_meta.json");
  if (!in) return meta;
  std::string content((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
  auto findNum = [&](const char* key) -> double {
    const std::string pat = std::string("\"") + key + "\"";
    const size_t pos = content.find(pat);
    if (pos == std::string::npos) return 0.0;
    const size_t colon = content.find(':', pos);
    return std::stod(content.substr(colon + 1));
  };
  meta.angle_min = findNum("angle_min");
  meta.angle_max = findNum("angle_max");
  meta.angle_increment = findNum("angle_increment");
  meta.range_min = findNum("range_min");
  meta.range_max = findNum("range_max");
  meta.scan_rate_hz = findNum("scan_rate_hz");
  if (meta.scan_rate_hz <= 0.0) meta.scan_rate_hz = 10.0;
  return meta;
}

localization_zoo::kinematic_icp::LaserScan loadScanKinematicICP(const fs::path& frame_dir,
                                                              const ScanMeta& meta) {
  localization_zoo::kinematic_icp::LaserScan scan;
  scan.angle_min = meta.angle_min;
  scan.angle_max = meta.angle_max;
  scan.angle_increment = meta.angle_increment;
  scan.range_min = meta.range_min;
  scan.range_max = meta.range_max;
  std::ifstream in(frame_dir / "scan.csv");
  if (!in) return scan;
  std::string line;
  std::getline(in, line);
  for (const auto& tok : splitCsv(line)) {
    if (tok.empty()) continue;
    scan.ranges.push_back(std::stod(tok));
  }
  return scan;
}

localization_zoo::csm::LaserScan loadScanCSM(const fs::path& frame_dir, const ScanMeta& meta) {
  localization_zoo::csm::LaserScan scan;
  scan.angle_min = meta.angle_min;
  scan.angle_max = meta.angle_max;
  scan.angle_increment = meta.angle_increment;
  scan.range_min = meta.range_min;
  scan.range_max = meta.range_max;
  std::ifstream in(frame_dir / "scan.csv");
  if (!in) return scan;
  std::string line;
  std::getline(in, line);
  for (const auto& tok : splitCsv(line)) {
    if (tok.empty()) continue;
    scan.ranges.push_back(std::stod(tok));
  }
  return scan;
}

localization_zoo::idc::LaserScan loadScanIDC(const fs::path& frame_dir, const ScanMeta& meta) {
  localization_zoo::idc::LaserScan scan;
  scan.angle_min = meta.angle_min;
  scan.angle_max = meta.angle_max;
  scan.angle_increment = meta.angle_increment;
  scan.range_min = meta.range_min;
  scan.range_max = meta.range_max;
  std::ifstream in(frame_dir / "scan.csv");
  if (!in) return scan;
  std::string line;
  std::getline(in, line);
  for (const auto& tok : splitCsv(line)) {
    if (tok.empty()) continue;
    scan.ranges.push_back(std::stod(tok));
  }
  return scan;
}

localization_zoo::ndt_2d::LaserScan loadScanNDT2D(const fs::path& frame_dir,
                                                    const ScanMeta& meta) {
  localization_zoo::ndt_2d::LaserScan scan;
  scan.angle_min = meta.angle_min;
  scan.angle_max = meta.angle_max;
  scan.angle_increment = meta.angle_increment;
  scan.range_min = meta.range_min;
  scan.range_max = meta.range_max;
  std::ifstream in(frame_dir / "scan.csv");
  if (!in) return scan;
  std::string line;
  std::getline(in, line);
  for (const auto& tok : splitCsv(line)) {
    if (tok.empty()) continue;
    scan.ranges.push_back(std::stod(tok));
  }
  return scan;
}

localization_zoo::psm::LaserScan loadScanPSM(const fs::path& frame_dir, const ScanMeta& meta) {
  localization_zoo::psm::LaserScan scan;
  scan.angle_min = meta.angle_min;
  scan.angle_max = meta.angle_max;
  scan.angle_increment = meta.angle_increment;
  scan.range_min = meta.range_min;
  scan.range_max = meta.range_max;
  std::ifstream in(frame_dir / "scan.csv");
  if (!in) return scan;
  std::string line;
  std::getline(in, line);
  for (const auto& tok : splitCsv(line)) {
    if (tok.empty()) continue;
    scan.ranges.push_back(std::stod(tok));
  }
  return scan;
}

localization_zoo::pl_icp::LaserScan loadScanPLICP(const fs::path& frame_dir,
                                                  const ScanMeta& meta) {
  localization_zoo::pl_icp::LaserScan scan;
  scan.angle_min = meta.angle_min;
  scan.angle_max = meta.angle_max;
  scan.angle_increment = meta.angle_increment;
  scan.range_min = meta.range_min;
  scan.range_max = meta.range_max;
  std::ifstream in(frame_dir / "scan.csv");
  if (!in) return scan;
  std::string line;
  std::getline(in, line);
  for (const auto& tok : splitCsv(line)) {
    if (tok.empty()) continue;
    scan.ranges.push_back(std::stod(tok));
  }
  return scan;
}

localization_zoo::rf2o::LaserScan loadScan(const fs::path& frame_dir,
                                           const ScanMeta& meta) {
  localization_zoo::rf2o::LaserScan scan;
  scan.angle_min = meta.angle_min;
  scan.angle_max = meta.angle_max;
  scan.angle_increment = meta.angle_increment;
  scan.range_min = meta.range_min;
  scan.range_max = meta.range_max;
  std::ifstream in(frame_dir / "scan.csv");
  if (!in) return scan;
  std::string line;
  std::getline(in, line);
  for (const auto& tok : splitCsv(line)) {
    if (tok.empty()) continue;
    scan.ranges.push_back(std::stod(tok));
  }
  return scan;
}

std::vector<fs::path> listFrames(const fs::path& scan_dir, size_t max_frames) {
  std::vector<fs::path> frames;
  for (const auto& ent : fs::directory_iterator(scan_dir)) {
    if (!ent.is_directory()) continue;
    const auto name = ent.path().filename().string();
    if (name.size() == 8 && std::all_of(name.begin(), name.end(), ::isdigit))
      frames.push_back(ent.path());
  }
  std::sort(frames.begin(), frames.end());
  if (max_frames > 0 && frames.size() > max_frames)
    frames.resize(max_frames);
  return frames;
}

double computeATE2D(const std::vector<Eigen::Matrix4d>& est,
                    const std::vector<Eigen::Matrix4d>& gt) {
  const int n = std::min(est.size(), gt.size());
  if (n == 0) return 0.0;
  double sum = 0.0;
  for (int i = 0; i < n; ++i) {
    const double dx = est[static_cast<size_t>(i)](0, 3) - gt[static_cast<size_t>(i)](0, 3);
    const double dy = est[static_cast<size_t>(i)](1, 3) - gt[static_cast<size_t>(i)](1, 3);
    sum += dx * dx + dy * dy;
  }
  return std::sqrt(sum / n);
}

double trajectoryLength2D(const std::vector<Eigen::Matrix4d>& poses) {
  double len = 0.0;
  for (size_t i = 1; i < poses.size(); ++i) {
    const Eigen::Vector2d a(poses[i - 1](0, 3), poses[i - 1](1, 3));
    const Eigen::Vector2d b(poses[i](0, 3), poses[i](1, 3));
    len += (b - a).norm();
  }
  return len;
}

struct RPEMetrics {
  bool available = false;
  double trans_pct = 0.0;
  double rot_deg_per_m = 0.0;
};

double rpeSegmentLengthM(double trajectory_length_m) {
  if (trajectory_length_m <= 0.0) return 0.0;
  constexpr double kKitBench = 100.0;
  if (trajectory_length_m >= kKitBench) return kKitBench;
  const double half = 0.5 * trajectory_length_m;
  constexpr double kMinSeg = 3.0;
  double seg = std::max(half, kMinSeg);
  seg = std::min(seg, trajectory_length_m * 0.95);
  return seg;
}

RPEMetrics computeRPE(const std::vector<Eigen::Matrix4d>& est,
                      const std::vector<Eigen::Matrix4d>& gt,
                      double segment_length_m = 100.0) {
  const int n = std::min(static_cast<int>(est.size()), static_cast<int>(gt.size()));
  if (n < 2 || segment_length_m <= 0.0) return {};

  std::vector<double> trans_errs;
  std::vector<double> rot_errs;
  for (int i = 0; i < n; i++) {
    double dist = 0.0;
    int j = i + 1;
    while (j < n) {
      dist += (gt[static_cast<size_t>(j)].block<3, 1>(0, 3) -
               gt[static_cast<size_t>(j - 1)].block<3, 1>(0, 3))
                  .norm();
      if (dist >= segment_length_m) break;
      ++j;
    }
    if (j >= n || dist <= 1e-9) break;

    const Eigen::Matrix4d dT_est = est[static_cast<size_t>(i)].inverse() * est[static_cast<size_t>(j)];
    const Eigen::Matrix4d dT_gt = gt[static_cast<size_t>(i)].inverse() * gt[static_cast<size_t>(j)];
    const Eigen::Matrix4d T_err = dT_gt.inverse() * dT_est;

    const double t_err = T_err.block<3, 1>(0, 3).norm();
    const Eigen::Matrix3d R_err = T_err.block<3, 3>(0, 0);
    const double trace_term =
        std::clamp((R_err.trace() - 1.0) / 2.0, -1.0, 1.0);
    const double r_err_rad = std::acos(trace_term);

    trans_errs.push_back(t_err / dist * 100.0);
    rot_errs.push_back(r_err_rad / dist * 180.0 / M_PI);
  }

  if (trans_errs.empty()) return {};

  RPEMetrics metrics;
  metrics.available = true;
  for (double value : trans_errs) metrics.trans_pct += value;
  for (double value : rot_errs) metrics.rot_deg_per_m += value;
  metrics.trans_pct /= trans_errs.size();
  metrics.rot_deg_per_m /= rot_errs.size();
  return metrics;
}

void writeJsonNumberOrNull(std::ostream& out, double value) {
  if (std::isfinite(value)) {
    out << std::fixed << std::setprecision(3) << value;
  } else {
    out << "null";
  }
}

struct MethodResult {
  std::string name;
  std::vector<Eigen::Matrix4d> poses;
  double time_ms = 0.0;
  bool skipped = false;
  std::string note;
  double ate_m = 0.0;
  bool has_rpe = false;
  double rpe_trans_pct = 0.0;
  double rpe_rot_deg_per_m = 0.0;
};

MethodResult runRF2O(const std::vector<fs::path>& frames, const ScanMeta& meta,
                     const std::vector<GTPose2D>& gt, bool no_gt_seed) {
  using namespace localization_zoo::rf2o;
  MethodResult res;
  res.name = "RF2O";
  RF2OParams params;
  RF2OEstimator est(params);
  if (!gt.empty() && !no_gt_seed) {
    est.setInitialPose(localization_zoo::rf2o::pose2D(gt.front().x, gt.front().y,
                                                     gt.front().yaw));
  }
  const double dt = 1.0 / meta.scan_rate_hz;
  auto t0 = Clock::now();
  for (size_t i = 0; i < frames.size(); ++i) {
    auto scan = loadScan(frames[i], meta);
    if (scan.size() < 10) continue;
    scan.timestamp = static_cast<double>(i) * dt;
    const auto out = est.registerScan(scan, dt);
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<2, 2>(0, 0) = out.pose.block<2, 2>(0, 0);
    T(0, 3) = out.pose(0, 2);
    T(1, 3) = out.pose(1, 2);
    res.poses.push_back(T);
    if (i % 10 == 0) {
      std::cerr << "\r  [RF2O] " << i << "/" << frames.size();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Range-flow 2D laser odometry (Jaimez ICRA 2016, simplified port).";
  return res;
}

MethodResult runPLICP(const std::vector<fs::path>& frames, const ScanMeta& meta,
                      const std::vector<GTPose2D>& gt, bool no_gt_seed) {
  using namespace localization_zoo::pl_icp;
  MethodResult res;
  res.name = "PL-ICP";
  PLICPParams params;
  params.max_correspondence_distance = 1.5;
  params.max_neighbor_gap = 2.0;
  PLICPEstimator est(params);
  if (!gt.empty() && !no_gt_seed) {
    est.setInitialPose(pose2D(gt.front().x, gt.front().y, gt.front().yaw));
  }
  auto t0 = Clock::now();
  for (size_t i = 0; i < frames.size(); ++i) {
    const auto scan = loadScanPLICP(frames[i], meta);
    if (scan.size() < 10) continue;
    const auto out = est.registerScan(scan);
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<2, 2>(0, 0) = out.pose.block<2, 2>(0, 0);
    T(0, 3) = out.pose(0, 2);
    T(1, 3) = out.pose(1, 2);
    res.poses.push_back(T);
    if (i % 10 == 0) {
      std::cerr << "\r  [PL-ICP] " << i << "/" << frames.size();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Point-to-line ICP 2D scan odometry (Censi IROS 2008, simplified port).";
  return res;
}

MethodResult runCSM(const std::vector<fs::path>& frames, const ScanMeta& meta,
                    const std::vector<GTPose2D>& gt, bool no_gt_seed) {
  using namespace localization_zoo::csm;
  MethodResult res;
  res.name = "CSM";
  CSMParams params;
  params.search_xy_range = 0.8;
  params.search_yaw_range = 0.25;
  params.coarse_xy_steps = 11;
  params.coarse_yaw_steps = 11;
  params.fine_xy_steps = 5;
  params.fine_yaw_steps = 5;
  CSMEstimator est(params);
  if (!gt.empty() && !no_gt_seed) {
    est.setInitialPose(pose2D(gt.front().x, gt.front().y, gt.front().yaw));
  }
  auto t0 = Clock::now();
  for (size_t i = 0; i < frames.size(); ++i) {
    const auto scan = loadScanCSM(frames[i], meta);
    if (scan.size() < 10) continue;
    const auto out = est.registerScan(scan);
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<2, 2>(0, 0) = out.pose.block<2, 2>(0, 0);
    T(0, 3) = out.pose(0, 2);
    T(1, 3) = out.pose(1, 2);
    res.poses.push_back(T);
    if (i % 10 == 0) {
      std::cerr << "\r  [CSM] " << i << "/" << frames.size();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Correlative scan matching 2D odometry (Olson ICRA 2009, simplified port).";
  return res;
}

MethodResult runIDC(const std::vector<fs::path>& frames, const ScanMeta& meta,
                  const std::vector<GTPose2D>& gt, bool no_gt_seed) {
  using namespace localization_zoo::idc;
  MethodResult res;
  res.name = "IDC";
  IDCParams params;
  params.max_cp_distance = 0.8;
  params.max_range_diff = 0.4;
  params.range_match_beam_window = 8;
  IDCEstimator est(params);
  if (!gt.empty() && !no_gt_seed) {
    est.setInitialPose(pose2D(gt.front().x, gt.front().y, gt.front().yaw));
  }
  auto t0 = Clock::now();
  for (size_t i = 0; i < frames.size(); ++i) {
    const auto scan = loadScanIDC(frames[i], meta);
    if (scan.size() < 10) continue;
    const auto out = est.registerScan(scan);
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<2, 2>(0, 0) = out.pose.block<2, 2>(0, 0);
    T(0, 3) = out.pose(0, 2);
    T(1, 3) = out.pose(1, 2);
    res.poses.push_back(T);
    if (i % 10 == 0) {
      std::cerr << "\r  [IDC] " << i << "/" << frames.size();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Iterative dual correspondence 2D odometry (Lu & Milios 1997, simplified port).";
  return res;
}

MethodResult runNDT2D(const std::vector<fs::path>& frames, const ScanMeta& meta,
                    const std::vector<GTPose2D>& gt, bool no_gt_seed) {
  using namespace localization_zoo::ndt_2d;
  MethodResult res;
  res.name = "NDT-2D";
  NDT2DParams params;
  params.cell_size = 0.5;
  params.cov_regularization = 0.05;
  params.max_iterations = 30;
  NDT2DEstimator est(params);
  if (!gt.empty() && !no_gt_seed) {
    est.setInitialPose(pose2D(gt.front().x, gt.front().y, gt.front().yaw));
  }
  auto t0 = Clock::now();
  for (size_t i = 0; i < frames.size(); ++i) {
    const auto scan = loadScanNDT2D(frames[i], meta);
    if (scan.size() < 10) continue;
    const auto out = est.registerScan(scan);
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<2, 2>(0, 0) = out.pose.block<2, 2>(0, 0);
    T(0, 3) = out.pose(0, 2);
    T(1, 3) = out.pose(1, 2);
    res.poses.push_back(T);
    if (i % 10 == 0) {
      std::cerr << "\r  [NDT-2D] " << i << "/" << frames.size();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "2D NDT scan matching odometry (Biber & Straßer IROS 2003, simplified port).";
  return res;
}

MethodResult runPSM(const std::vector<fs::path>& frames, const ScanMeta& meta,
                  const std::vector<GTPose2D>& gt, bool no_gt_seed) {
  using namespace localization_zoo::psm;
  MethodResult res;
  res.name = "PSM";
  PSMParams params;
  params.search_xy_range = 0.8;
  params.search_yaw_range = 0.25;
  params.coarse_xy_steps = 11;
  params.coarse_yaw_steps = 11;
  params.fine_xy_steps = 5;
  params.fine_yaw_steps = 5;
  PSMEstimator est(params);
  if (!gt.empty() && !no_gt_seed) {
    est.setInitialPose(pose2D(gt.front().x, gt.front().y, gt.front().yaw));
  }
  auto t0 = Clock::now();
  for (size_t i = 0; i < frames.size(); ++i) {
    const auto scan = loadScanPSM(frames[i], meta);
    if (scan.size() < 10) continue;
    const auto out = est.registerScan(scan);
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<2, 2>(0, 0) = out.pose.block<2, 2>(0, 0);
    T(0, 3) = out.pose(0, 2);
    T(1, 3) = out.pose(1, 2);
    res.poses.push_back(T);
    if (i % 10 == 0) {
      std::cerr << "\r  [PSM] " << i << "/" << frames.size();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Polar scan matching 2D odometry (Hähnel polar range correlation, simplified port).";
  return res;
}

MethodResult runKinematicICP(const std::vector<fs::path>& frames, const ScanMeta& meta,
                           const std::vector<GTPose2D>& gt, bool no_gt_seed,
                           bool wheel_odom_from_gt) {
  using namespace localization_zoo::kinematic_icp;
  MethodResult res;
  res.name = "Kinematic-ICP";
  KinematicICPParams params;
  params.max_correspondence_distance = 1.5;
  params.wheel_odom_weight = 8.0;
  params.use_last_increment_as_wheel_odom = !wheel_odom_from_gt;
  KinematicICPEstimator est(params);
  if (!gt.empty() && !no_gt_seed) {
    est.setInitialPose(pose2D(gt.front().x, gt.front().y, gt.front().yaw));
  }
  auto t0 = Clock::now();
  for (size_t i = 0; i < frames.size(); ++i) {
    const auto scan = loadScanKinematicICP(frames[i], meta);
    if (scan.size() < 10) continue;
    std::optional<WheelOdom> wheel;
    if (wheel_odom_from_gt && i > 0 && gt.size() > 1) {
      const size_t gi = std::min(i, gt.size() - 1);
      const size_t gp = std::min(i - 1, gt.size() - 1);
      wheel = wheelOdomFromGT(gt[gp], gt[gi]);
    }
    const auto out = est.registerScan(scan, wheel);
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<2, 2>(0, 0) = out.pose.block<2, 2>(0, 0);
    T(0, 3) = out.pose(0, 2);
    T(1, 3) = out.pose(1, 2);
    res.poses.push_back(T);
    if (i % 10 == 0) {
      std::cerr << "\r  [Kinematic-ICP] " << i << "/" << frames.size();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Unicycle PL-ICP + wheel odom prior (Guadagnino ICRA 2025, simplified 2D port).";
  if (wheel_odom_from_gt) {
    res.note += " GT-derived wheel odometry.";
  }
  return res;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << " <scan_dir> <gt_csv> [max_frames] [--methods rf2o,pl_icp,csm,kinematic_icp,psm,ndt_2d,idc]"
              << " [--no-gt-seed] [--wheel-odom-from-gt] [--summary-json path]\n";
    return 1;
  }

  const fs::path scan_dir = argv[1];
  const fs::path gt_csv = argv[2];
  size_t max_frames = 0;
  bool no_gt_seed = false;
  bool wheel_odom_from_gt = false;
  std::string methods = "rf2o";
  std::string summary_json_path;
  for (int i = 3; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--no-gt-seed") {
      no_gt_seed = true;
    } else if (arg == "--wheel-odom-from-gt") {
      wheel_odom_from_gt = true;
    } else if (arg == "--methods" && i + 1 < argc) {
      methods = argv[++i];
    } else if (arg == "--summary-json" && i + 1 < argc) {
      summary_json_path = argv[++i];
    } else if (arg.rfind("--summary-json=", 0) == 0) {
      summary_json_path = arg.substr(std::string("--summary-json=").size());
    } else if (std::all_of(arg.begin(), arg.end(), ::isdigit)) {
      max_frames = static_cast<size_t>(std::stoul(arg));
    }
  }

  const auto frames = listFrames(scan_dir, max_frames);
  const auto gt_raw = loadGT(gt_csv);
  const auto meta = loadMeta(scan_dir);
  std::vector<Eigen::Matrix4d> gt;
  gt.reserve(frames.size());
  for (size_t i = 0; i < frames.size(); ++i) {
    const size_t gi = std::min(i, gt_raw.size() - 1);
    gt.push_back(gt_raw[gi].matrix4());
  }

  std::cout << "Scan frames: " << frames.size() << "\n";
  std::cout << "GT poses: " << gt.size() << "\n";
  const double traj_len = trajectoryLength2D(gt);
  std::cout << "Trajectory length: " << traj_len << " m\n";
  const double rpe_seg = rpeSegmentLengthM(traj_len);

  std::vector<MethodResult> results;
  if (methods.find("idc") != std::string::npos) {
    std::cout << "Running IDC...\n";
    results.push_back(runIDC(frames, meta, gt_raw, no_gt_seed));
  }
  if (methods.find("ndt_2d") != std::string::npos) {
    std::cout << "Running NDT-2D...\n";
    results.push_back(runNDT2D(frames, meta, gt_raw, no_gt_seed));
  }
  if (methods.find("psm") != std::string::npos) {
    std::cout << "Running PSM...\n";
    results.push_back(runPSM(frames, meta, gt_raw, no_gt_seed));
  }
  if (methods.find("kinematic_icp") != std::string::npos) {
    std::cout << "Running Kinematic-ICP...\n";
    results.push_back(runKinematicICP(frames, meta, gt_raw, no_gt_seed, wheel_odom_from_gt));
  }
  if (methods.find("csm") != std::string::npos) {
    std::cout << "Running CSM...\n";
    results.push_back(runCSM(frames, meta, gt_raw, no_gt_seed));
  }
  if (methods.find("pl_icp") != std::string::npos) {
    std::cout << "Running PL-ICP...\n";
    results.push_back(runPLICP(frames, meta, gt_raw, no_gt_seed));
  }
  if (methods.find("rf2o") != std::string::npos) {
    std::cout << "Running RF2O...\n";
    results.push_back(runRF2O(frames, meta, gt_raw, no_gt_seed));
  }

  std::cout << "\n========================================\n";
  std::cout << "  RESULTS (2D Scan Data)\n";
  std::cout << "========================================\n";
  std::cout << std::left << std::setw(20) << "Method" << std::setw(12) << "ATE [m]"
            << std::setw(14) << "Drift [%]" << std::setw(12) << "Frames"
            << std::setw(12) << "Time [ms]" << std::setw(10) << "FPS\n";
  std::cout << std::string(80, '-') << "\n";

  for (auto& r : results) {
    if (r.skipped) continue;
    r.ate_m = computeATE2D(r.poses, gt);
    const RPEMetrics rpe = computeRPE(r.poses, gt, rpe_seg);
    r.has_rpe = rpe.available;
    r.rpe_trans_pct = rpe.trans_pct;
    r.rpe_rot_deg_per_m = rpe.rot_deg_per_m;
    const double fps =
        r.time_ms > 0 ? r.poses.size() / (r.time_ms / 1000.0) : 0.0;
    std::ostringstream drift_ss;
    if (r.has_rpe && std::isfinite(r.rpe_trans_pct)) {
      drift_ss << std::fixed << std::setprecision(2) << r.rpe_trans_pct;
    } else {
      drift_ss << "-";
    }
    std::cout << std::setw(20) << r.name << std::setw(12) << std::fixed
              << std::setprecision(3) << r.ate_m << std::setw(14) << drift_ss.str()
              << std::setw(12) << r.poses.size() << std::setw(12) << r.time_ms
              << std::setw(10) << fps << "\n";
    if (!r.note.empty()) std::cout << "  note: " << r.note << "\n";
  }

  if (!summary_json_path.empty()) {
    std::ofstream out(summary_json_path);
    if (!out) {
      std::cerr << "Failed to write summary json: " << summary_json_path << "\n";
      return 1;
    }
    out << "{\n";
    out << "  \"scan_dir\": \"" << scan_dir.string() << "\",\n";
    out << "  \"gt_csv\": \"" << gt_csv.string() << "\",\n";
    out << "  \"num_frames\": " << frames.size() << ",\n";
    out << "  \"trajectory_length_m\": ";
    writeJsonNumberOrNull(out, traj_len);
    out << ",\n";
    out << "  \"rpe_segment_length_m\": ";
    writeJsonNumberOrNull(out, rpe_seg);
    out << ",\n";
    out << "  \"gt_seed\": " << (no_gt_seed ? "false" : "true") << ",\n";
    out << "  \"methods\": [\n";
    bool first = true;
    for (const auto& r : results) {
      if (r.skipped) continue;
      if (!first) out << ",\n";
      first = false;
      out << "    {\n";
      out << "      \"name\": \"" << r.name << "\",\n";
      out << "      \"status\": \"ok\",\n";
      out << "      \"ate_m\": ";
      writeJsonNumberOrNull(out, r.ate_m);
      out << ",\n";
      out << "      \"drift_pct\": ";
      if (r.has_rpe && std::isfinite(r.rpe_trans_pct)) {
        writeJsonNumberOrNull(out, r.rpe_trans_pct);
      } else {
        out << "null";
      }
      out << ",\n";
      out << "      \"rpe_rot_deg_per_m\": ";
      if (r.has_rpe && std::isfinite(r.rpe_rot_deg_per_m)) {
        writeJsonNumberOrNull(out, r.rpe_rot_deg_per_m);
      } else {
        out << "null";
      }
      out << ",\n";
      out << "      \"frames\": " << r.poses.size() << ",\n";
      out << "      \"time_ms\": ";
      writeJsonNumberOrNull(out, r.time_ms);
      out << ",\n";
      out << "      \"fps\": ";
      if (r.time_ms > 0) {
        writeJsonNumberOrNull(out, r.poses.size() / (r.time_ms / 1000.0));
      } else {
        out << "null";
      }
      out << ",\n";
      out << "      \"note\": \"" << r.note << "\"\n";
      out << "    }";
    }
    out << "\n  ]\n";
    out << "}\n";
    std::cout << "Wrote summary: " << summary_json_path << "\n";
  }

  return 0;
}
