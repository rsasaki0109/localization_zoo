// Replay the actual normal prefix. GT is never read by this executable.
#include "native_loader.h"
#include "hooks.h"
#include <algorithm>
#include <set>
#include <stdexcept>

using namespace localization_zoo::inten_loam;
namespace serial = localization_zoo::evaluation::inten_trace;

int main(int argc, char** argv) {
  try {
    if (argc != 4) throw std::runtime_error("Expected scan directory, geometry and feature paths");
    std::set<int> frames;
    int endpoint;
    while (std::cin >> endpoint) frames.insert(endpoint);
    if (!std::cin.eof() || frames.empty()) throw std::runtime_error("Invalid endpoints");
    std::ofstream geometry(argv[2]), features(argv[3]);
    if (!geometry || !features) throw std::runtime_error("Output open failed");
    InTenLoamPaperParams params;
    params.s2m_plane_max_dist = .05;
    params.accelerated = true;
    params.s2s_mapped_prior = true;
    params.s2s_translation_prior_ratio = .03;
    params.s2s_ground_pca = params.s2s_facade_pca = true;
    InTenLoamPaper reference(params);
    for (int i = 0; i <= *frames.rbegin(); ++i) {
      char name[32];
      std::snprintf(name, sizeof(name), "%06d.bin", i);
      const auto raw = loadPCDXYZI((fs::path(argv[1]) / name).string(), 0.0);
      if (raw.empty()) throw std::runtime_error("Empty scan");
      std::vector<Eigen::Vector3d> points;
      std::vector<float> intensity;
      for (const auto& p : raw) { points.push_back(p.point); intensity.push_back(p.intensity); }
      if (*std::max_element(intensity.begin(), intensity.end()) > 1.0f)
        for (auto& value : intensity) value /= 255.0f;
      diagnostic::frame = i;
      diagnostic::output = frames.count(i) ? &geometry : nullptr;
      InTenLoamPaperTrace trace;
      reference.process(points, intensity, nullptr, &trace);
      auto row = serial::record();
      row << "{\"frame\":" << i << ",\"s2s\":"; serial::solver(row, trace.s2s);
      row << ",\"s2m\":"; serial::solver(row, trace.s2m);
      row << ",\"pose\":"; serial::matrix(row, trace.pose);
      row << '}';
      std::cout << row.str() << '\n' << std::flush;
      if (frames.count(i) || frames.count(i + 1)) {
        const auto extracted = reference.extractFeatures(points, intensity);
        auto frow = serial::record();
        frow << "{\"frame\":" << i << ",\"features\":[";
        int c = 0;
        for (const auto* set : {&extracted.edge, &extracted.ground, &extracted.facade,
                                &extracted.reflector, &extracted.all}) {
          if (c++) frow << ',';
          frow << '[';
          for (size_t k = 0; k < set->size(); ++k) {
            if (k) frow << ',';
            diagnostic::feature(frow, (*set)[k]);
          }
          frow << ']';
        }
        frow << "]}";
        features << frow.str() << '\n';
      }
      if (!std::cout || !geometry || !features) throw std::runtime_error("Output write failed");
    }
    geometry.close(); features.close();
    if (geometry.fail() || features.fail()) throw std::runtime_error("Output close failed");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
