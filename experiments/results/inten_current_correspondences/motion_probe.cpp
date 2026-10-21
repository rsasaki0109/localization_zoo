// Offline full-point translation profiles, independent of native facade normals.
#include "native_loader.h"
#include "inten_trace_jsonl.h"
#include <pcl/kdtree/kdtree_flann.h>
#include <algorithm>
#include <stdexcept>

using namespace localization_zoo::inten_loam;
namespace serial = localization_zoo::evaluation::inten_trace;
using Cloud = pcl::PointCloud<pcl::PointXYZ>;

PaperFrame load(const std::string& path, InTenLoamPaper& core) {
  std::vector<Eigen::Vector3d> points; std::vector<float> intensity;
  for (const auto& p : loadPCDXYZI(path, 0.0)) { points.push_back(p.point); intensity.push_back(p.intensity); }
  if (points.empty()) throw std::runtime_error("Empty scan");
  if (*std::max_element(intensity.begin(), intensity.end()) > 1.0f)
    for (auto& v : intensity) v /= 255.0f;
  return core.extractFeatures(points, intensity);
}

bool inside(const Eigen::Vector3d& p, int region) {
  if (region == 3) return p.x() >= -30 && p.x() <= 30 && p.y() >= 2 && p.y() <= 25 && p.z() >= -1.6 && p.z() <= 2;
  return p.x() >= (region == 1 ? 3 : -12) && p.x() <= (region == 2 ? -2 : 15) &&
      p.y() >= -5 && p.y() <= -2 && p.z() >= -1.6 && p.z() <= .5;
}

std::pair<double, double> cost(const Cloud::Ptr& a, const Cloud::Ptr& b) {
  if (a->empty() || b->empty()) throw std::runtime_error("Empty geometric control region");
  pcl::KdTreeFLANN<pcl::PointXYZ> tree;
  std::vector<float> distances;
  for (int side = 0; side < 2; ++side) {
    tree.setInputCloud(side ? a : b);
    for (const auto& p : *(side ? b : a)) {
      std::vector<int> ids(1); std::vector<float> ds(1);
      if (tree.nearestKSearch(p, 1, ids, ds) != 1) throw std::runtime_error("NN failed");
      distances.push_back(ds[0]);
    }
  }
  std::sort(distances.begin(), distances.end());
  const size_t trimmed = std::max<size_t>(1, distances.size()*4/5);
  double all = 0, robust = 0;
  for (size_t k = 0; k < distances.size(); ++k) { all += distances[k]; if (k < trimmed) robust += distances[k]; }
  return {all/distances.size(), robust/trimmed};
}

int main() {
  try {
    int frame; std::string previous_path, current_path;
    InTenLoamPaperParams params; params.accelerated = true;
    InTenLoamPaper core(params);
    while (std::cin >> frame >> previous_path >> current_path) {
      Eigen::Matrix4d gt;
      for (int k = 0; k < 16; ++k) if (!(std::cin >> gt(k/4, k%4))) throw std::runtime_error("Invalid GT diagnostic transform");
      const auto previous = load(previous_path, core), current = load(current_path, core);
      for (int region = 0; region < 4; ++region) {
        Cloud::Ptr a(new Cloud), base(new Cloud);
        for (const auto& f : previous.all) if (inside(f.p, region)) a->emplace_back(f.p.x(), f.p.y(), f.p.z());
        for (const auto& f : current.all) {
          const Eigen::Vector3d p = gt.block<3, 3>(0, 0)*f.p+gt.block<3, 1>(0, 3);
          if (inside(p, region)) base->emplace_back(p.x(), p.y(), p.z());
        }
        auto out = serial::record();
        out << "{\"frame\":" << frame << ",\"region\":" << region << ",\"previous_points\":" << a->size()
            << ",\"current_points\":" << base->size() << ",\"profile\":[";
        if (a->empty() || base->empty()) {
          out << "],\"unavailable\":\"empty fixed region in at least one frame\"}";
          std::cout << out.str() << '\n' << std::flush;
          if (!std::cout) throw std::runtime_error("Output failed");
          continue;
        }
        for (int step = -60; step <= 60; ++step) {
          if (step > -60) out << ',';
          const double shift = step*.05;
          Cloud::Ptr b(new Cloud(*base));
          for (auto& p : *b) p.x += shift;
          const auto value = cost(a, b);
          out << '[' << shift << ',' << value.first << ',' << value.second << ']';
        }
        out << "]}";
        std::cout << out.str() << '\n' << std::flush;
        if (!std::cout) throw std::runtime_error("Output failed");
      }
    }
    if (!std::cin.eof()) throw std::runtime_error("Invalid input");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
