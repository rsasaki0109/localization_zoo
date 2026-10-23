// Fixed-pair initialization interventions through the existing diagnostic API.
// This executable is outside the estimator and is never run by the harness.
#include "native_loader.h"
#include "inten_trace_jsonl.h"
#include <algorithm>
#include <iomanip>
#include <stdexcept>

using namespace localization_zoo::inten_loam;
namespace serial = localization_zoo::evaluation::inten_trace;

struct Scan {
  std::vector<Eigen::Vector3d> points;
  std::vector<float> intensity;
};

Scan load(const std::string& path) {
  Scan scan;
  const auto xyzi = loadPCDXYZI(path, 0.0);
  if (xyzi.empty()) throw std::runtime_error("empty input: " + path);
  for (const auto& point : xyzi) {
    scan.points.push_back(point.point);
    scan.intensity.push_back(point.intensity);
  }
  if (*std::max_element(scan.intensity.begin(), scan.intensity.end()) > 1.0f)
    for (auto& value : scan.intensity) value /= 255.0f;
  return scan;
}

int main() {
  try {
    std::string id, mode, previous_path, current_path, cached_previous, cached_current;
    Scan previous, current;
    while (std::cin >> id >> mode >> previous_path >> current_path) {
      Eigen::Matrix4d seed;
      for (int i = 0; i < 16; ++i)
        if (!(std::cin >> seed(i / 4, i % 4))) throw std::runtime_error("incomplete seed");
      if (!seed.allFinite()) throw std::runtime_error("nonfinite seed");
      if (previous_path != cached_previous || current_path != cached_current) {
        previous = load(previous_path);
        current = load(current_path);
        cached_previous = previous_path;
        cached_current = current_path;
      }
      InTenLoamPaperParams params;
      params.s2m_plane_max_dist = .05;
      params.use_mapping = false;  // Isolate the same S2S solver, not a new pipeline.
      InTenLoamPaper odometry(params);
      odometry.process(previous.points, previous.intensity);
      InTenLoamPaperDiagnostics diagnostic;
      diagnostic.forced_pose = &seed;
      const auto result = odometry.process(current.points, current.intensity, &diagnostic);
      auto row = serial::record();
      row << "{\"id\":\"" << id << "\",\"mode\":\"" << mode << "\",\"seed\":";
      serial::matrix(row, seed);
      row << ",\"output\":";
      serial::matrix(row, diagnostic.s2s_estimate);
      row << ",\"previous_points\":" << previous.points.size()
          << ",\"current_points\":" << current.points.size()
          << ",\"features\":[" << result.num_edge << ',' << result.num_ground << ','
          << result.num_facade << ',' << result.num_reflector << "],\"first_systems\":[";
      for (int c = 0; c < 4; ++c) {
        if (c) row << ',';
        row << "{\"n\":" << diagnostic.s2s_n[c] << ",\"H\":";
        serial::matrix(row, diagnostic.s2s_H[c]);
        row << ",\"g\":";
        serial::matrix(row, diagnostic.s2s_g[c]);
        row << '}';
      }
      row << "]}";
      std::cout << row.str() << '\n';
      if (!std::cout) throw std::runtime_error("output stream failed");
    }
    if (!std::cin.eof()) throw std::runtime_error("input stream failed");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
