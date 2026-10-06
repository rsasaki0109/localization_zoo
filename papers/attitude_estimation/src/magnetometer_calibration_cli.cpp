// Fit magnetometer hard/soft-iron calibration from a rotation recording.
//
//   magnetometer_calibration_cli <readings.csv> [--columns i,j,k]
//       [--profile-yaml out.yaml]
//
// The CSV has one header line; --columns picks the magnetometer columns
// (0-based, default 0,1,2). Prints JSON with the offset (hard iron), the
// correction matrix (soft iron), the fitted field norm, the RMS norm
// residual, and the direction coverage (of 26 sphere bins).
// --profile-yaml writes the imu_motion_health profile keys that apply it.

#include "attitude_estimation/magnetometer_calibration.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace localization_zoo::attitude_estimation;

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "usage: magnetometer_calibration_cli <readings.csv> [--columns i,j,k]\n"
                 "  [--profile-yaml out.yaml]\n";
    return 2;
  }
  int columns[3] = {0, 1, 2};
  std::string profile_yaml;
  for (int i = 2; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--profile-yaml" && i + 1 < argc) {
      profile_yaml = argv[++i];
      continue;
    }
    if (arg == "--columns" && i + 1 < argc &&
        std::sscanf(argv[++i], "%d,%d,%d", &columns[0], &columns[1], &columns[2]) == 3) {
      continue;
    }
    std::cerr << "error: bad argument " << arg << "\n";
    return 2;
  }
  std::ifstream in(argv[1]);
  if (!in) {
    std::cerr << "error: cannot open " << argv[1] << "\n";
    return 1;
  }
  std::string line;
  std::getline(in, line);  // header
  std::vector<Eigen::Vector3d> readings;
  while (std::getline(in, line)) {
    if (line.empty()) continue;
    std::vector<double> row;
    std::stringstream ss(line);
    std::string cell;
    while (std::getline(ss, cell, ',')) row.push_back(std::stod(cell));
    const int max_col = std::max(columns[0], std::max(columns[1], columns[2]));
    if (static_cast<int>(row.size()) <= max_col) {
      std::cerr << "error: short row\n";
      return 1;
    }
    const Eigen::Vector3d m(row[columns[0]], row[columns[1]], row[columns[2]]);
    if (m.allFinite()) readings.push_back(m);
  }
  const MagnetometerCalibration c = fitMagnetometerCalibration(readings);
  if (!c.valid) {
    std::cerr << "error: readings do not determine an ellipsoid (rotate through more directions)\n";
    return 1;
  }
  const Eigen::Vector3d& b = c.offset;
  const Eigen::Matrix3d& W = c.matrix;
  std::printf("{\"readings\": %zu, \"offset\": [%.9g, %.9g, %.9g], \"matrix\": [[%.9g, %.9g, %.9g], "
              "[%.9g, %.9g, %.9g], [%.9g, %.9g, %.9g]], \"field_norm\": %.9g, \"rms_residual\": %.9g, "
              "\"direction_bins\": %d}\n",
              readings.size(), b.x(), b.y(), b.z(), W(0, 0), W(0, 1), W(0, 2), W(1, 0), W(1, 1),
              W(1, 2), W(2, 0), W(2, 1), W(2, 2), c.field_norm, c.rms_residual, c.direction_bins);
  if (!profile_yaml.empty()) {
    std::ofstream out(profile_yaml);
    out.precision(9);
    out << "# Magnetometer calibration (hard/soft iron) from " << argv[1] << "\n"
        << "imu_motion_health:\n"
        << "  mag_offset_x: " << b.x() << "\n  mag_offset_y: " << b.y() << "\n  mag_offset_z: " << b.z() << "\n";
    const char* axes = "xyz";
    for (int r = 0; r < 3; ++r) {
      for (int col = 0; col < 3; ++col) {
        out << "  mag_matrix_" << axes[r] << axes[col] << ": " << W(r, col) << "\n";
      }
    }
  }
  if (c.direction_bins < 18) {
    std::cerr << "warning: only " << c.direction_bins
              << " of 26 directions covered; rotate the sensor through more orientations\n";
  }
  return 0;
}
