// Dump top-down trajectories (ground truth, pure dead reckoning, ESKF) for a
// dogfooding fixture, in the ground-truth world frame, for plotting.
//
// Estimated poses are relative (first = identity); they are anchored into the
// GT world frame exactly as pcd_dogfooding does, world = GT[0] * T_rel, so the
// XY view matches the map the ATE is computed against.
//
// Usage:
//   dump_trajectory <fixture_dir> <gt_csv> <out_csv> [--nhc]
// Writes columns: frame,gt_x,gt_y,dr_x,dr_y,eskf_x,eskf_y

#include "imu_dead_reckoning/imu_dead_reckoning.h"

#include <Eigen/Dense>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace localization_zoo::imu_dead_reckoning;

namespace {

std::vector<std::string> split(const std::string& s, char d) {
  std::vector<std::string> out;
  std::string tok;
  std::stringstream ss(s);
  while (std::getline(ss, tok, d)) out.push_back(tok);
  return out;
}

bool readColumns(const std::string& path, const std::vector<int>& cols,
                 std::vector<std::vector<double>>* rows) {
  std::ifstream f(path);
  if (!f) return false;
  std::string line;
  std::getline(f, line);  // header.
  while (std::getline(f, line)) {
    if (line.empty()) continue;
    const auto toks = split(line, ',');
    std::vector<double> r;
    bool ok = true;
    for (int c : cols) {
      if (c >= static_cast<int>(toks.size())) { ok = false; break; }
      r.push_back(std::stod(toks[c]));
    }
    if (ok) rows->push_back(std::move(r));
  }
  return true;
}

Eigen::Matrix3d rpyToR(double roll, double pitch, double yaw) {
  const Eigen::AngleAxisd rx(roll, Eigen::Vector3d::UnitX());
  const Eigen::AngleAxisd ry(pitch, Eigen::Vector3d::UnitY());
  const Eigen::AngleAxisd rz(yaw, Eigen::Vector3d::UnitZ());
  return (rz * ry * rx).toRotationMatrix();
}

// Run a pipeline over the fixture and return one relative pose per frame.
std::vector<Eigen::Matrix4d> runFrames(const std::vector<ImuReading>& imu,
                                       const std::vector<double>& frame_ts,
                                       const ImuDeadReckoningParams& params) {
  ImuDeadReckoningPipeline pipe(params);
  std::vector<Eigen::Matrix4d> out;
  out.reserve(frame_ts.size());
  size_t idx = 0;
  for (double q : frame_ts) {
    while (idx < imu.size() && imu[idx].stamp <= q + 1e-9) {
      pipe.processImu(imu[idx]);
      ++idx;
    }
    out.push_back(pipe.pose());
  }
  return out;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 4) {
    std::fprintf(stderr, "usage: %s <fixture_dir> <gt_csv> <out_csv> [--nhc]\n",
                 argv[0]);
    return 2;
  }
  const std::string dir = argv[1], gt_csv = argv[2], out_csv = argv[3];
  bool use_nhc = false;
  for (int i = 4; i < argc; ++i)
    if (std::string(argv[i]) == "--nhc") use_nhc = true;

  std::vector<std::vector<double>> imu_rows, ft_rows, gt_rows;
  if (!readColumns(dir + "/imu.csv", {0, 1, 2, 3, 4, 5, 6}, &imu_rows) ||
      !readColumns(dir + "/frame_timestamps.csv", {1}, &ft_rows) ||
      !readColumns(gt_csv, {1, 2, 3, 4, 5, 6}, &gt_rows)) {
    std::fprintf(stderr, "failed to read inputs\n");
    return 1;
  }
  if (gt_rows.size() != ft_rows.size()) {
    std::fprintf(stderr, "gt rows (%zu) != frame count (%zu)\n", gt_rows.size(),
                 ft_rows.size());
    return 1;
  }

  std::vector<ImuReading> imu;
  for (const auto& r : imu_rows) {
    ImuReading s;
    s.stamp = r[0];
    s.gyro = Eigen::Vector3d(r[1], r[2], r[3]);
    s.accel = Eigen::Vector3d(r[4], r[5], r[6]);
    imu.push_back(s);
  }
  std::vector<double> frame_ts;
  for (const auto& r : ft_rows) frame_ts.push_back(r[0]);

  const Eigen::Vector3d p0(gt_rows[0][0], gt_rows[0][1], gt_rows[0][2]);
  const Eigen::Matrix3d R0 = rpyToR(gt_rows[0][3], gt_rows[0][4], gt_rows[0][5]);

  // Pure dead reckoning: all aids off (repository default).
  ImuDeadReckoningParams dr_params;
  // ESKF: the best full-session aided config.
  ImuDeadReckoningParams eskf_params;
  eskf_params.enable_eskf = true;
  eskf_params.enable_zupt = true;
  eskf_params.enable_leveling = true;
  eskf_params.enable_nhc = use_nhc;

  const auto dr = runFrames(imu, frame_ts, dr_params);
  const auto eskf = runFrames(imu, frame_ts, eskf_params);

  std::ofstream out(out_csv);
  out << "frame,gt_x,gt_y,dr_x,dr_y,eskf_x,eskf_y\n";
  for (size_t i = 0; i < frame_ts.size(); ++i) {
    // Anchor relative estimates into the GT world frame: world = R0*t_rel + p0.
    const Eigen::Vector3d dr_w = R0 * dr[i].block<3, 1>(0, 3) + p0;
    const Eigen::Vector3d ek_w = R0 * eskf[i].block<3, 1>(0, 3) + p0;
    out << i << ',' << gt_rows[i][0] << ',' << gt_rows[i][1] << ',' << dr_w.x()
        << ',' << dr_w.y() << ',' << ek_w.x() << ',' << ek_w.y() << '\n';
  }
  std::printf("wrote %s (%zu frames)\n", out_csv.c_str(), frame_ts.size());
  return 0;
}
