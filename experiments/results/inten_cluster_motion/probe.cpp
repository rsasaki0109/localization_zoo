// Cached full extracted frames only. This executable never reads GT or a region.
#include "hooks.h"
#include "profile.h"
#include <fstream>
#include <iostream>
#include <map>
#include <string>

using namespace localization_zoo::inten_loam;
namespace serial = localization_zoo::evaluation::inten_trace;

void selfTest() {
  std::vector<Eigen::Vector3d> shape;
  diagnostic::Cloud::Ptr target(new diagnostic::Cloud);
  for (int k = 0; k < 120; ++k) {
    const Eigen::Vector3d p(.3*std::sin(k*.71), .2*std::cos(k*.47), .15*std::sin(k*.19));
    shape.push_back(p); target->emplace_back(p.x(), p.y(), p.z());
  }
  pcl::KdTreeFLANN<pcl::PointXYZ> tree;
  tree.setInputCloud(target);
  const auto fixed = diagnostic::profile(shape, tree, Eigen::Vector3d::UnitX());
  if (fixed.excluded || fixed.best != 60) throw std::runtime_error("Static geometry was excluded");
  auto moving = shape;
  for (auto& p : moving) p.x() += 1.6;
  const auto shifted = diagnostic::profile(moving, tree, Eigen::Vector3d::UnitX());
  if (!shifted.excluded || shifted.best != 28) throw std::runtime_error("Moving geometry not detected");
  for (auto& p : moving) p.x() += 2.4;
  const auto boundary = diagnostic::profile(moving, tree, Eigen::Vector3d::UnitX());
  if (boundary.excluded || boundary.best != 0) throw std::runtime_error("Boundary optimum accepted");
  const auto unavailable = diagnostic::profile(shape, tree, Eigen::Vector3d::Zero());
  if (unavailable.excluded || !unavailable.costs.empty()) throw std::runtime_error("Zero prediction accepted");
  std::cout << "{\"static_kept\":true,\"moving_excluded\":true,\"boundary_kept\":true,"
               "\"zero_prediction_kept\":true,\"exit_code\":0}\n";
}

int main(int argc, char** argv) {
  try {
    if (argc == 2 && std::string(argv[1]) == "--self-test") { selfTest(); return 0; }
    if (argc != 2) throw std::runtime_error("Expected cached-feature text path");
    std::ifstream file(argv[1]);
    if (!file) throw std::runtime_error("Cannot open cached features");
    std::map<int, PaperFrame> frames;
    int frame;
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
    while (std::cin >> frame) {
      Eigen::Matrix4d seed;
      for (int k = 0; k < 16; ++k)
        if (!(std::cin >> seed(k/4, k%4))) throw std::runtime_error("Invalid natural seed");
      const auto& previous = frames.at(frame - 1);
      const auto& current = frames.at(frame);
      diagnostic::Cloud::Ptr target(new diagnostic::Cloud);
      for (const auto& p : previous.all)
        if (p.cluster >= 2) target->emplace_back(p.p.x(), p.p.y(), p.p.z());
      if (target->empty()) throw std::runtime_error("Empty non-ground target");
      pcl::KdTreeFLANN<pcl::PointXYZ> tree;
      tree.setInputCloud(target);
      Eigen::Vector3d direction = seed.block<3, 1>(0, 3);
      if (direction.norm() > 0.) direction.normalize();
      std::map<int, std::vector<Eigen::Vector3d>> clusters;
      for (const auto& p : current.facade) if (p.cluster >= 2) clusters[p.cluster];
      for (const auto& p : current.all) {
        const auto found = clusters.find(p.cluster);
        if (found != clusters.end()) found->second.push_back(seed.block<3, 3>(0, 0)*p.p + seed.block<3, 1>(0, 3));
      }
      std::set<int> mask;
      for (const auto& entry : clusters) {
        const auto profile = diagnostic::profile(entry.second, tree, direction);
        if (profile.excluded) mask.insert(entry.first);
        auto row = serial::record();
        row << "{\"type\":\"profile\",\"frame\":" << frame << ",\"cluster\":" << entry.first
            << ",\"points\":" << entry.second.size() << ",\"direction\":";
        serial::matrix(row, direction);
        row << ",\"best_index\":" << profile.best << ",\"excluded\":" << (profile.excluded ? "true" : "false")
            << ",\"costs_m2\":[";
        for (size_t k = 0; k < profile.costs.size(); ++k) { if (k) row << ','; row << profile.costs[k]; }
        row << "]}";
        std::cout << row.str() << '\n';
      }
      for (int mode = 0; mode < 2; ++mode) {
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
        if (mode) diagnostic::excluded = mask;
        PaperSolverTrace trace;
        diagnostic::terminal_trace = &trace;
        InTenLoamPaperDiagnostics d;
        d.forced_pose = &seed;
        core.process({}, {}, &d);
        diagnostic::terminal_trace = nullptr;
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
      }
      std::cout << std::flush;
      if (!std::cout) throw std::runtime_error("Output failed");
    }
    if (!std::cin.eof()) throw std::runtime_error("Invalid command input");
    return 0;
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
