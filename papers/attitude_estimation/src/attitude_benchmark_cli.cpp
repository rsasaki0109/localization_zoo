// Runs one attitude filter on one BROAD-style trial CSV and prints the BROAD
// error metrics as JSON on stdout.
//
// CSV columns (header line required):
//   gx,gy,gz,ax,ay,az,mx,my,mz,qw,qx,qy,qz,movement
// gyr in rad/s; acc and mag in any unit; q = reference orientation (ENU);
// movement = 1 for samples that count towards the error.

#include "attitude_estimation/attitude_estimation.h"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace localization_zoo::attitude_estimation;

namespace {

struct Sample {
  Eigen::Vector3d gyr, acc, mag;
  Quat reference;
  bool movement = false;
};

std::vector<Sample> readCsv(const std::string& path) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("cannot open " + path);
  std::string line;
  std::getline(in, line);  // header
  std::vector<Sample> samples;
  while (std::getline(in, line)) {
    if (line.empty()) continue;
    std::stringstream ss(line);
    std::string cell;
    double v[14];
    for (int i = 0; i < 14; ++i) {
      if (!std::getline(ss, cell, ',')) throw std::runtime_error("short row in " + path);
      v[i] = std::stod(cell);  // accepts "nan"
    }
    Sample s;
    s.gyr = Eigen::Vector3d(v[0], v[1], v[2]);
    s.acc = Eigen::Vector3d(v[3], v[4], v[5]);
    s.mag = Eigen::Vector3d(v[6], v[7], v[8]);
    s.reference = Quat(v[9], v[10], v[11], v[12]);
    s.movement = v[13] != 0.0;
    samples.push_back(s);
  }
  return samples;
}

void usage() {
  std::cerr << "usage: attitude_benchmark_cli <trial.csv> --rate HZ --method madgwick|mahony\n"
               "  [--mode 9d|6d] [--beta B] [--legacy-xio-field-scale]\n"
               "  [--kp KP] [--ki KI] [--output-quat out.csv]\n";
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    usage();
    return 2;
  }
  const std::string csv = argv[1];
  std::string method, mode = "9d", output_quat;
  double rate = 0;
  MadgwickParams madgwick;
  MahonyParams mahony;
  for (int i = 2; i < argc; ++i) {
    const std::string arg = argv[i];
    auto next = [&]() -> std::string {
      if (i + 1 >= argc) throw std::runtime_error("missing value for " + arg);
      return argv[++i];
    };
    if (arg == "--rate") rate = std::stod(next());
    else if (arg == "--method") method = next();
    else if (arg == "--mode") mode = next();
    else if (arg == "--beta") madgwick.beta = std::stod(next());
    else if (arg == "--legacy-xio-field-scale") madgwick.legacy_xio_field_scale = true;
    else if (arg == "--kp") mahony.kp = std::stod(next());
    else if (arg == "--ki") mahony.ki = std::stod(next());
    else if (arg == "--output-quat") output_quat = next();
    else {
      std::cerr << "error: unknown argument " << arg << "\n";
      usage();
      return 2;
    }
  }
  if (rate <= 0 || (method != "madgwick" && method != "mahony") ||
      (mode != "9d" && mode != "6d")) {
    usage();
    return 2;
  }
  madgwick.sampling_rate = mahony.sampling_rate = rate;
  const bool use_mag = mode == "9d";

  const std::vector<Sample> samples = readCsv(csv);
  if (samples.empty()) {
    std::cerr << "error: no samples in " << csv << "\n";
    return 1;
  }
  // BROAD protocol: initial state from the first accelerometer and
  // magnetometer sample; output rotated into ENU.
  const Quat initial = quatFromAccMag(samples.front().acc, samples.front().mag);
  MadgwickFilter mad(madgwick);
  MahonyFilter mah(mahony);
  mad.setState(initial);
  mah.setState(initial);

  std::vector<Quat> estimate, reference;
  std::vector<bool> movement;
  estimate.reserve(samples.size());
  const auto t0 = std::chrono::steady_clock::now();
  for (const Sample& s : samples) {
    Quat q;
    if (method == "madgwick") {
      use_mag ? mad.update(s.gyr, s.acc, s.mag) : mad.updateImu(s.gyr, s.acc);
      q = mad.state();
    } else {
      use_mag ? mah.update(s.gyr, s.acc, s.mag) : mah.updateImu(s.gyr, s.acc);
      q = mah.state();
    }
    estimate.push_back(quatMultiply(enuCorrection(), q));
  }
  const double seconds =
      std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
  for (const Sample& s : samples) {
    reference.push_back(s.reference);
    movement.push_back(s.movement);
  }
  const OrientationErrors e = broadErrors(estimate, reference, movement);

  if (!output_quat.empty()) {
    std::ofstream out(output_quat);
    out.precision(17);
    out << "qw,qx,qy,qz\n";
    for (const Quat& q : estimate) out << q[0] << ',' << q[1] << ',' << q[2] << ',' << q[3] << '\n';
  }
  std::printf(
      "{\"total_rmse_deg\": %.10f, \"heading_rmse_deg\": %.10f, "
      "\"inclination_rmse_deg\": %.10f, \"samples\": %zu, \"movement_samples\": %d, "
      "\"update_us\": %.4f}\n",
      e.total_rmse_deg, e.heading_rmse_deg, e.inclination_rmse_deg, samples.size(), e.samples,
      1e6 * seconds / samples.size());
  return 0;
}
