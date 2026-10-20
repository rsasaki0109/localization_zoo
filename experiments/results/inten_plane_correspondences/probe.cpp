// Study-only executable. Native source, library, defaults and harness are untouched.
#include "native_loader.h"
#include "inten_trace_jsonl.h"
#include "inten_loam/detail/feature_tree.h"
#include <algorithm>
#include <stdexcept>

using namespace localization_zoo::inten_loam;
using Tree = detail::FeatureTree;
namespace serial = localization_zoo::evaluation::inten_trace;

struct Scan {
  std::vector<Eigen::Vector3d> points;
  std::vector<float> intensity;
};

Scan load(const std::string& path) {
  Scan scan;
  for (const auto& p : loadPCDXYZI(path, 0.0)) {
    scan.points.push_back(p.point);
    scan.intensity.push_back(p.intensity);
  }
  if (scan.points.empty()) throw std::runtime_error("empty scan");
  if (*std::max_element(scan.intensity.begin(), scan.intensity.end()) > 1.0f)
    for (auto& i : scan.intensity) i /= 255.0f;
  return scan;
}

void features(std::ostream& out, const std::vector<PaperFeature>& fs) {
  out << '[';
  for (size_t i = 0; i < fs.size(); ++i) {
    if (i) out << ',';
    out << '[' << fs[i].p.x() << ',' << fs[i].p.y() << ',' << fs[i].p.z()
        << ',' << fs[i].row << ']';
  }
  out << ']';
}

int main() {
  try {
    std::string id, mode, previous_path, current_path;
    while (std::cin >> id >> mode >> previous_path >> current_path) {
      Eigen::Matrix4d seed;
      for (int i = 0; i < 16; ++i)
        if (!(std::cin >> seed(i / 4, i % 4))) throw std::runtime_error("incomplete seed");
      if (mode != "oracle_current_gt" || !seed.allFinite()) throw std::runtime_error("invalid input");
      const auto previous = load(previous_path), current = load(current_path);
      InTenLoamPaperParams params;
      params.s2m_plane_max_dist = .05;
      params.use_mapping = false;
      InTenLoamPaper odometry(params);
      const auto prev = odometry.extractFeatures(previous.points, previous.intensity);
      const auto curr = odometry.extractFeatures(current.points, current.intensity);
      odometry.process(previous.points, previous.intensity);
      InTenLoamPaperDiagnostics diagnostic;
      diagnostic.forced_pose = &seed;
      const auto result = odometry.process(current.points, current.intensity, &diagnostic);
      auto out = serial::record();
      out << "{\"id\":\"" << id << "\",\"seed\":";
      serial::matrix(out, seed);
      out << ",\"output\":";
      serial::matrix(out, diagnostic.s2s_estimate);
      out << ",\"features\":[" << result.num_edge << ',' << result.num_ground << ','
          << result.num_facade << ',' << result.num_reflector << "],\"first_systems\":[";
      for (int c = 0; c < 4; ++c) {
        if (c) out << ',';
        out << "{\"n\":" << diagnostic.s2s_n[c] << ",\"H\":";
        serial::matrix(out, diagnostic.s2s_H[c]);
        out << ",\"g\":";
        serial::matrix(out, diagnostic.s2s_g[c]);
        out << '}';
      }
      out << "],\"planes\":[";
      for (int cls = 0; cls < 2; ++cls) {
        if (cls) out << ',';
        Tree tree;
        tree.pts = cls == 0 ? prev.ground : prev.facade;
        tree.build(false);
        const auto& src = cls == 0 ? curr.ground : curr.facade;
        const double gate = params.s2s_gate_sq;
        // Unmodified association and normal/residual construction.
#include "native_plane.h"
        out << "{\"target\":";
        features(out, tree.pts);
        out << ",\"source\":";
        features(out, src);
        out << ",\"queries\":[";
        for (size_t i = 0; i < src.size(); ++i) {
          if (i) out << ',';
          const Eigen::Vector3d x = seed.block<3, 3>(0, 0) * src[i].p + seed.block<3, 1>(0, 3);
          Eigen::Vector3d a = Eigen::Vector3d::Zero();
          double r = 0;
          std::vector<int> indices;
          std::vector<float> distances;
          const bool accepted = plane(src[i], x, &a, &r, &indices, &distances);
          out << '[' << (accepted ? "true" : "false") << ",[";
          for (size_t k = 0; k < indices.size(); ++k) {
            if (k) out << ',';
            out << indices[k];
          }
          out << "],[";
          for (size_t k = 0; k < distances.size(); ++k) {
            if (k) out << ',';
            out << distances[k];
          }
          out << "],";
          serial::matrix(out, a);
          out << ',' << r << ']';
        }
        out << "]}";
      }
      out << "]}";
      std::cout << out.str() << '\n';
      if (!std::cout) throw std::runtime_error("output stream failed");
    }
    if (!std::cin.eof()) throw std::runtime_error("input stream failed");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
