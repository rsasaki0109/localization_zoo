// Allan deviation of selected columns of a static IMU recording.
//
//   allan_variance_cli <data.npy|data.csv> --columns 1,2,3 [--time-column 0]
//       [--rate HZ] [--points-per-decade 20] [--names gx,gy,gz]
//
// .npy: 2-D little-endian float64, C order (e.g. the TUM VI imu-static file).
// .csv: one header line, comma-separated numbers.
// The sampling rate comes from --rate or from the time column (seconds):
// (N - 1) / (t_last - t_first). Prints JSON with the curve and the automatic
// noise parameters per column.

#include "allan_variance/allan_variance.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace localization_zoo::allan_variance;

namespace {

std::vector<int> parseInts(const std::string& s) {
  std::vector<int> out;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, ',')) out.push_back(std::stoi(item));
  return out;
}

std::vector<std::string> parseNames(const std::string& s) {
  std::vector<std::string> out;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, ',')) out.push_back(item);
  return out;
}

bool endsWith(const std::string& s, const std::string& suffix) {
  return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Reads the requested columns (and the time column, if any) into `out`.
void readNpy(const std::string& path, const std::vector<int>& columns,
             std::vector<std::vector<double>>* out) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("cannot open " + path);
  char magic[6];
  in.read(magic, 6);
  if (std::memcmp(magic, "\x93NUMPY", 6) != 0) throw std::runtime_error("not a .npy file");
  unsigned char version[2];
  in.read(reinterpret_cast<char*>(version), 2);
  uint32_t header_len = 0;
  if (version[0] == 1) {
    uint16_t h;
    in.read(reinterpret_cast<char*>(&h), 2);
    header_len = h;
  } else {
    in.read(reinterpret_cast<char*>(&header_len), 4);
  }
  std::string header(header_len, ' ');
  in.read(&header[0], header_len);
  if (header.find("'<f8'") == std::string::npos ||
      header.find("'fortran_order': False") == std::string::npos) {
    throw std::runtime_error("only C-order little-endian float64 .npy is supported");
  }
  const auto shape_pos = header.find("'shape': (");
  long rows = 0, cols = 0;
  if (shape_pos == std::string::npos ||
      std::sscanf(header.c_str() + shape_pos, "'shape': (%ld, %ld)", &rows, &cols) != 2) {
    throw std::runtime_error("expected a 2-D array");
  }
  for (int c : columns) {
    if (c < 0 || c >= cols) throw std::runtime_error("column out of range");
  }
  out->assign(columns.size(), std::vector<double>());
  for (auto& v : *out) v.reserve(rows);
  std::vector<double> buffer(static_cast<size_t>(cols) * 65536);
  long remaining = rows;
  while (remaining > 0) {
    const long chunk = std::min<long>(remaining, 65536);
    in.read(reinterpret_cast<char*>(buffer.data()), sizeof(double) * cols * chunk);
    if (!in) throw std::runtime_error("truncated .npy file");
    for (long r = 0; r < chunk; ++r) {
      for (size_t k = 0; k < columns.size(); ++k) (*out)[k].push_back(buffer[r * cols + columns[k]]);
    }
    remaining -= chunk;
  }
}

void readCsv(const std::string& path, const std::vector<int>& columns,
             std::vector<std::vector<double>>* out) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("cannot open " + path);
  std::string line;
  std::getline(in, line);  // header
  out->assign(columns.size(), std::vector<double>());
  std::vector<double> row;
  while (std::getline(in, line)) {
    if (line.empty()) continue;
    row.clear();
    std::stringstream ss(line);
    std::string cell;
    while (std::getline(ss, cell, ',')) row.push_back(std::stod(cell));
    for (size_t k = 0; k < columns.size(); ++k) {
      if (columns[k] >= static_cast<int>(row.size())) throw std::runtime_error("short row");
      (*out)[k].push_back(row[columns[k]]);
    }
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "usage: allan_variance_cli <data.npy|data.csv> --columns i,j,... "
                 "[--time-column t] [--rate HZ] [--points-per-decade P] [--names a,b,...]\n";
    return 2;
  }
  const std::string path = argv[1];
  std::vector<int> columns;
  std::vector<std::string> names;
  int time_column = -1;
  double rate = 0.0;
  int points_per_decade = 20;
  for (int i = 2; i < argc; ++i) {
    const std::string arg = argv[i];
    if (i + 1 >= argc) {
      std::cerr << "error: missing value for " << arg << "\n";
      return 2;
    }
    const std::string value = argv[++i];
    if (arg == "--columns") columns = parseInts(value);
    else if (arg == "--names") names = parseNames(value);
    else if (arg == "--time-column") time_column = std::stoi(value);
    else if (arg == "--rate") rate = std::stod(value);
    else if (arg == "--points-per-decade") points_per_decade = std::stoi(value);
    else {
      std::cerr << "error: unknown argument " << arg << "\n";
      return 2;
    }
  }
  if (columns.empty() || (rate <= 0 && time_column < 0)) {
    std::cerr << "error: need --columns and either --rate or --time-column\n";
    return 2;
  }
  if (!names.empty() && names.size() != columns.size()) {
    std::cerr << "error: --names must match --columns\n";
    return 2;
  }

  std::vector<int> wanted = columns;
  if (time_column >= 0) wanted.push_back(time_column);
  std::vector<std::vector<double>> data;
  try {
    endsWith(path, ".npy") ? readNpy(path, wanted, &data) : readCsv(path, wanted, &data);
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }
  const long samples = static_cast<long>(data.front().size());
  if (samples < 3) {
    std::cerr << "error: need at least 3 samples\n";
    return 1;
  }
  if (rate <= 0) {
    const std::vector<double>& t = data.back();
    rate = (samples - 1) / (t.back() - t.front());
  }

  const std::vector<long> sizes = logClusterSizes(samples, points_per_decade);
  std::vector<std::vector<AllanPoint>> curves(columns.size());
  std::vector<std::thread> workers;
  for (size_t k = 0; k < columns.size(); ++k) {
    workers.emplace_back([&, k]() { curves[k] = overlappingAllanDeviation(data[k], rate, sizes); });
  }
  for (auto& w : workers) w.join();

  std::printf("{\"samples\": %ld, \"sampling_rate\": %.9g, \"columns\": [", samples, rate);
  for (size_t k = 0; k < columns.size(); ++k) {
    const NoiseParameters p = extractNoiseParameters(curves[k]);
    const std::string name = names.empty() ? std::to_string(columns[k]) : names[k];
    std::printf("%s{\"name\": \"%s\", \"white_noise_density\": %.9g, \"bias_random_walk\": %.9g, "
                "\"bias_instability\": %.9g, \"bias_instability_tau\": %.9g, \"tau\": [",
                k ? ", " : "", name.c_str(), p.white_noise_density, p.bias_random_walk,
                p.bias_instability, p.bias_instability_tau);
    for (size_t i = 0; i < curves[k].size(); ++i) std::printf("%s%.9g", i ? ", " : "", curves[k][i].tau);
    std::printf("], \"adev\": [");
    for (size_t i = 0; i < curves[k].size(); ++i) std::printf("%s%.9g", i ? ", " : "", curves[k][i].adev);
    std::printf("]}");
  }
  std::printf("]}\n");
  return 0;
}
