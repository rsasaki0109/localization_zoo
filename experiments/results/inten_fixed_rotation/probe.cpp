// Cached full extracted frames only. This executable never reads GT or a region.
#include "hooks.h"
#include <fstream>
#include <iostream>
#include <map>
#include <string>

using namespace localization_zoo::inten_loam;
namespace serial = localization_zoo::evaluation::inten_trace;

int main(int argc, char** argv) {
  try {
    if (argc != 3) throw std::runtime_error("Expected cached-feature text path");
    std::ifstream file(argv[1]);
    if (!file) throw std::runtime_error("Cannot open cached features");
    std::map<int, PaperFrame> frames;
    std::ofstream residuals(argv[2]);
    if (!residuals) throw std::runtime_error("Cannot open query observer");
    int frame, mode, count;
    while (file >> frame) {
      if (frames.count(frame)) throw std::runtime_error("Duplicate cached frame");
      auto& f = frames[frame];
      for (auto* set : {&f.edge, &f.ground, &f.facade, &f.reflector, &f.all}) {
        size_t count;
        if (!(file >> count)) throw std::runtime_error("Invalid cached count");
        for (size_t k = 0; k < count; ++k) {
          PaperFeature p;
          if (!(file >> p.p.x() >> p.p.y() >> p.p.z() >> p.row >> p.cluster >> p.intensity))
            throw std::runtime_error("Invalid cached feature");
          set->push_back(p);
        }
      }
    }
    if (!file.eof() || frames.empty()) throw std::runtime_error("Invalid feature input");
    while (std::cin >> frame >> mode >> count) {
      std::set<int> mask;
      for (int k = 0; k < count; ++k) {
        int cluster;
        if (!(std::cin >> cluster) || cluster < 2 || !mask.insert(cluster).second)
          throw std::runtime_error("Invalid fixed mask");
      }
      Eigen::Matrix4d seed;
      for (int k = 0; k < 16; ++k)
        if (!(std::cin >> seed(k/4, k%4))) throw std::runtime_error("Invalid natural-derived seed");
      const auto& previous = frames.at(frame - 1);
      const auto& current = frames.at(frame);
        InTenLoamPaperParams params;
        params.s2m_plane_max_dist = .05;
        params.accelerated = true;
        params.s2s_mapped_prior = true;
        params.s2s_translation_prior_ratio = .03;
        params.s2s_ground_pca = params.s2s_facade_pca = true;
        params.use_mapping = false;
        InTenLoamPaper core(params);
        diagnostic::excluded.clear();
        diagnostic::terminal_trace = nullptr;
        diagnostic::frame = &previous;
        core.process({}, {});
        diagnostic::frame = &current;
        diagnostic::excluded = mask;
        diagnostic::translation_only = mode >= 2;
        diagnostic::frame_id = frame; diagnostic::mode = mode;
        diagnostic::capture = &residuals;
        PaperSolverTrace trace;
        diagnostic::terminal_trace = &trace;
        InTenLoamPaperDiagnostics d;
        d.forced_pose = &seed;
        core.process({}, {}, &d);
        diagnostic::terminal_trace = nullptr;
        diagnostic::capture = nullptr;
        auto row = serial::record();
        row << "{\"type\":\"terminal\",\"frame\":" << frame << ",\"mode\":" << mode << ",\"output\":";
        serial::matrix(row, d.s2s_estimate);
        row << ",\"solver\":"; serial::solver(row, trace);
        row << ",\"first_systems\":[";
        for (int c = 0; c < 4; ++c) {
          if (c) row << ',';
          row << "{\"n\":" << d.s2s_n[c] << ",\"H\":"; serial::matrix(row, d.s2s_H[c]);
          row << ",\"g\":"; serial::matrix(row, d.s2s_g[c]); row << '}';
        }
        row << "]}";
        std::cout << row.str() << '\n';
      std::cout << std::flush;
      if (!std::cout) throw std::runtime_error("Output failed");
    }
    residuals.close();
    if (residuals.fail()) throw std::runtime_error("Query output failed");
    if (!std::cin.eof()) throw std::runtime_error("Invalid command input");
    return 0;
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
