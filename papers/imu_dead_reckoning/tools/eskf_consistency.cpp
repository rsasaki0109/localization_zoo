// ESKF consistency (NEES) analysis for the IMU dead-reckoning filter.
//
// Runs the --imu-dr-eskf filter over a real dogfooding fixture and checks
// whether its reported position covariance is statistically consistent with the
// actual position error against ground truth. For a consistent 3-DOF filter the
// per-frame Normalized Estimation Error Squared,
//     NEES_i = e_i^T P_pos_i^-1 e_i,   e_i = p_filter_i - p_gt_i,
// is chi-square distributed with 3 DOF, so E[NEES] = 3 and ~95% of frames fall
// at or below the chi2_3 95th percentile (7.815). ANEES = mean(NEES)/3: ~1 is
// consistent, >>1 is overconfident (covariance too small), <<1 pessimistic.
//
// Usage:
//   eskf_consistency <fixture_dir> <gt_csv> [--nhc] [--no-leveling]
// Emits a summary to stdout and a per-frame CSV (frame,err,nees,sigma) to
// <fixture_dir>/eskf_consistency.csv.

#include "imu_dead_reckoning/imu_dead_reckoning.h"

#include <Eigen/Dense>

#include <cmath>
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

// Reads a CSV with a header, returns rows as vectors of doubles for the
// requested 0-based column indices (in order).
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
    r.reserve(cols.size());
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
  return (rz * ry * rx).toRotationMatrix();  // matches pcd_dogfooding GTPose.
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 3) {
    std::fprintf(stderr,
                 "usage: %s <fixture_dir> <gt_csv> [--nhc] [--no-leveling]\n",
                 argv[0]);
    return 2;
  }
  const std::string dir = argv[1];
  const std::string gt_csv = argv[2];
  bool use_nhc = false, use_leveling = true;
  double zupt_sigma = -1, sigma_accel = -1, sigma_gyro = -1, leveling_sigma = -1;
  for (int i = 3; i < argc; ++i) {
    const std::string a = argv[i];
    if (a == "--nhc") use_nhc = true;
    else if (a == "--no-leveling") use_leveling = false;
    else if (a == "--zupt-sigma" && i + 1 < argc) zupt_sigma = std::stod(argv[++i]);
    else if (a == "--sigma-accel" && i + 1 < argc) sigma_accel = std::stod(argv[++i]);
    else if (a == "--sigma-gyro" && i + 1 < argc) sigma_gyro = std::stod(argv[++i]);
    else if (a == "--leveling-sigma" && i + 1 < argc) leveling_sigma = std::stod(argv[++i]);
  }

  // imu.csv: stamp, gyro xyz, accel xyz (cols 0..6).
  std::vector<std::vector<double>> imu_rows;
  if (!readColumns(dir + "/imu.csv", {0, 1, 2, 3, 4, 5, 6}, &imu_rows) ||
      imu_rows.empty()) {
    std::fprintf(stderr, "failed to read %s/imu.csv\n", dir.c_str());
    return 1;
  }
  std::vector<ImuReading> imu;
  imu.reserve(imu_rows.size());
  for (const auto& r : imu_rows) {
    ImuReading s;
    s.stamp = r[0];
    s.gyro = Eigen::Vector3d(r[1], r[2], r[3]);
    s.accel = Eigen::Vector3d(r[4], r[5], r[6]);
    imu.push_back(s);
  }

  // frame_timestamps.csv: frame_idx, timestamp, points (want col 1).
  std::vector<std::vector<double>> ft_rows;
  if (!readColumns(dir + "/frame_timestamps.csv", {1}, &ft_rows) ||
      ft_rows.empty()) {
    std::fprintf(stderr, "failed to read %s/frame_timestamps.csv\n",
                 dir.c_str());
    return 1;
  }
  std::vector<double> frame_ts;
  for (const auto& r : ft_rows) frame_ts.push_back(r[0]);

  // gt csv: timestamp,x,y,z,roll,pitch,yaw (cols 0..6).
  std::vector<std::vector<double>> gt_rows;
  if (!readColumns(gt_csv, {1, 2, 3, 4, 5, 6}, &gt_rows) ||
      gt_rows.size() != frame_ts.size()) {
    std::fprintf(stderr,
                 "gt rows (%zu) must match frame count (%zu)\n",
                 gt_rows.size(), frame_ts.size());
    return 1;
  }

  // Ground truth expressed in the filter's world frame (anchored at GT[0]):
  // p_gt_filter[i] = R0^T * (pos_i - pos_0).
  const Eigen::Vector3d p0(gt_rows[0][0], gt_rows[0][1], gt_rows[0][2]);
  const Eigen::Matrix3d R0 =
      rpyToR(gt_rows[0][3], gt_rows[0][4], gt_rows[0][5]);
  const Eigen::Matrix3d R0t = R0.transpose();

  ImuDeadReckoningParams params;
  params.enable_eskf = true;
  params.enable_zupt = true;
  params.enable_leveling = use_leveling;
  params.enable_nhc = use_nhc;
  if (zupt_sigma > 0) params.eskf_zupt_sigma = zupt_sigma;
  if (sigma_accel > 0) params.eskf_sigma_accel = sigma_accel;
  if (sigma_gyro > 0) params.eskf_sigma_gyro = sigma_gyro;
  if (leveling_sigma > 0) params.eskf_leveling_sigma = leveling_sigma;
  ImuDeadReckoningPipeline pipe(params);

  std::ofstream out(dir + "/eskf_consistency.csv");
  out << "frame,err_m,nees,sigma_pos_m\n";

  const double kChi2_3_95 = 7.815;  // 95th percentile, chi-square 3 DOF.
  double nees_sum = 0.0, err2_sum = 0.0;
  long n_valid = 0, n_within = 0;
  size_t imu_idx = 0;
  for (size_t fi = 0; fi < frame_ts.size(); ++fi) {
    const double q = frame_ts[fi];
    while (imu_idx < imu.size() && imu[imu_idx].stamp <= q + 1e-9) {
      pipe.processImu(imu[imu_idx]);
      ++imu_idx;
    }
    if (!pipe.staticInitialized()) continue;
    const Eigen::Matrix3d P_pos = pipe.covariance().block<3, 3>(0, 0);
    if (P_pos.norm() < 1e-12) continue;  // not yet seeded.

    const Eigen::Vector3d p_filter = pipe.pose().block<3, 1>(0, 3);
    const Eigen::Vector3d pos_i(gt_rows[fi][0], gt_rows[fi][1], gt_rows[fi][2]);
    const Eigen::Vector3d p_gt_filter = R0t * (pos_i - p0);
    const Eigen::Vector3d e = p_filter - p_gt_filter;

    // Regularize P a touch for a stable inverse (position covariance can be
    // tiny right after a ZUPT); 1e-6 m^2 = 1 mm floor.
    const Eigen::Matrix3d Pr = P_pos + 1e-6 * Eigen::Matrix3d::Identity();
    const double nees = e.transpose() * Pr.inverse() * e;
    const double sigma_pos = std::sqrt(P_pos.trace() / 3.0);

    nees_sum += nees;
    err2_sum += e.squaredNorm();
    if (nees <= kChi2_3_95) ++n_within;
    ++n_valid;
    out << fi << ',' << e.norm() << ',' << nees << ',' << sigma_pos << '\n';
  }
  out.close();

  if (n_valid == 0) {
    std::fprintf(stderr, "no valid frames\n");
    return 1;
  }
  const double mean_nees = nees_sum / n_valid;
  const double anees = mean_nees / 3.0;
  const double rms_err = std::sqrt(err2_sum / n_valid);
  std::printf("ESKF consistency: %s (zupt+%s%s)\n", dir.c_str(),
              use_leveling ? "leveling" : "-", use_nhc ? "+nhc" : "");
  std::printf("  frames evaluated : %ld\n", n_valid);
  std::printf("  RMS position err : %.3f m\n", rms_err);
  std::printf("  mean NEES        : %.2f   (ideal 3.0)\n", mean_nees);
  std::printf("  ANEES            : %.2f   (ideal 1.0; >>1 overconfident)\n",
              anees);
  std::printf("  frames <= chi2_95: %.1f%% (ideal ~95%%)\n",
              100.0 * n_within / n_valid);
  std::printf("  per-frame CSV    : %s/eskf_consistency.csv\n", dir.c_str());
  return 0;
}
