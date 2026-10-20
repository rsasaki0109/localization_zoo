// Normal-prefix, actual-map S2M control through the existing forced diagnostic.
// Diagnostic endpoint is terminal and is never used as an odometry score.
#include "native_loader.h"
#include "inten_trace_jsonl.h"
#include <algorithm>
#include <stdexcept>

using namespace localization_zoo::inten_loam;
namespace serial = localization_zoo::evaluation::inten_trace;

int main(int argc, char** argv) {
  try {
    if (argc != 2) throw std::runtime_error("Expected owned KITTI scan directory");
    std::vector<int> frames;
    int endpoint;
    while (std::cin >> endpoint) frames.push_back(endpoint);
    if (!std::cin.eof() || frames.empty()) throw std::runtime_error("Invalid endpoints");
    for (int last : frames) {
      InTenLoamPaperParams params;
      params.s2m_plane_max_dist = .05;
      params.accelerated = true;
      params.s2s_mapped_prior = true;
      params.s2s_translation_prior_ratio = .03;
      params.s2s_ground_pca = params.s2s_facade_pca = true;
      InTenLoamPaper reference(params), control(params);
      for (int i = 0; i <= last; ++i) {
        char name[32];
        std::snprintf(name, sizeof(name), "%06d.bin", i);
        const auto raw = loadPCDXYZI((fs::path(argv[1]) / name).string(), 0.0);
        if (raw.empty()) throw std::runtime_error("Empty scan");
        std::vector<Eigen::Vector3d> points;
        std::vector<float> intensity;
        for (const auto& p : raw) { points.push_back(p.point); intensity.push_back(p.intensity); }
        if (*std::max_element(intensity.begin(), intensity.end()) > 1.0f)
          for (auto& value : intensity) value /= 255.0f;
        InTenLoamPaperTrace trace;
        const auto normal = reference.process(points, intensity, nullptr, &trace);
        if (i < last) {
          const auto paired = control.process(points, intensity);
          if (normal.pose != paired.pose) throw std::runtime_error("Normal prefix diverged");
        } else {
          InTenLoamPaperDiagnostics diagnostic;
          diagnostic.forced_pose = &trace.s2m.input;
          control.process(points, intensity, &diagnostic);
          auto row = serial::record();
          row << "{\"frame\":" << i << ",\"normal_prefix_exact\":true,\"reference\":{\"s2s\":";
          serial::solver(row, trace.s2s);
          row << ",\"s2m\":"; serial::solver(row, trace.s2m);
          row << ",\"pose\":"; serial::matrix(row, trace.pose);
          row << "},\"diagnostic_output\":"; serial::matrix(row, diagnostic.s2m_estimate);
          row << ",\"first_systems\":[";
          for (int c = 0; c < 4; ++c) {
            if (c) row << ',';
            row << "{\"n\":" << diagnostic.s2m_n[c] << ",\"H\":";
            serial::matrix(row, diagnostic.s2m_H[c]);
            row << ",\"g\":"; serial::matrix(row, diagnostic.s2m_g[c]);
            row << '}';
          }
          row << "]}";
          std::cout << row.str() << '\n' << std::flush;
          if (!std::cout) throw std::runtime_error("Output stream failed");
        }
      }
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
