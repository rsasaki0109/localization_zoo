// Full normal sequence, passive DOR snapshots. This executable never reads GT.
#include "native_loader.h"
#include "diagnostic_hooks.h"
#include <algorithm>
#include <stdexcept>

using namespace localization_zoo::inten_loam;
namespace serial = localization_zoo::evaluation::inten_trace;

int main(int argc, char** argv) {
  try {
    if (argc != 5) throw std::runtime_error("Expected scan directory, correspondences, masks and ranges paths");
    int last;
    if (!(std::cin >> last) || last < 1) throw std::runtime_error("Invalid last frame");
    int f;
    while (std::cin >> f) diagnostic::detailed.insert(f);
    if (!std::cin.eof()) throw std::runtime_error("Invalid detailed frames");
    std::ofstream correspondences(argv[2]), masks(argv[3]), ranges(argv[4]);
    if (!correspondences || !masks || !ranges) throw std::runtime_error("Output open failed");
    diagnostic::correspondences = &correspondences;
    diagnostic::masks = &masks; diagnostic::ranges = &ranges;
    InTenLoamPaperParams params;
    params.s2m_plane_max_dist = .05;
    params.accelerated = true;
    params.s2s_mapped_prior = true;
    params.s2s_translation_prior_ratio = .03;
    params.s2s_ground_pca = params.s2s_facade_pca = true;
    InTenLoamPaper reference(params);
    for (int i = 0; i <= last; ++i) {
      char name[32]; std::snprintf(name, sizeof(name), "%06d.bin", i);
      const auto path = fs::path(argv[1])/name;
      const auto raw = loadPCDXYZI(path.string(), 0.0);
      if (raw.empty()) throw std::runtime_error("Empty scan");
      std::vector<Eigen::Vector3d> points; std::vector<float> intensity;
      for (const auto& p : raw) { points.push_back(p.point); intensity.push_back(p.intensity); }
      if (*std::max_element(intensity.begin(), intensity.end()) > 1.f)
        for (auto& v : intensity) v /= 255.f;
      InTenLoamPaperTrace trace;
      const auto result = reference.process(points, intensity, nullptr, &trace);
      std::cout << serial::frame(i, path.string(), trace, result, trace.pose) << '\n' << std::flush;
      if (!std::cout || !correspondences || !masks || !ranges) throw std::runtime_error("Output write failed");
    }
    correspondences.close(); masks.close(); ranges.close();
    if (correspondences.fail() || masks.fail() || ranges.fail()) throw std::runtime_error("Output close failed");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
