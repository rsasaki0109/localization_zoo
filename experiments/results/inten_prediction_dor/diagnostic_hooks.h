// Study-only passive records. Production source and public API stay unchanged.
#pragma once
#include "inten_loam/inten_loam_paper.h"
#include "inten_trace_jsonl.h"
#include <set>
#include <ostream>
#include <cmath>
#include <algorithm>

namespace diagnostic {
using namespace localization_zoo::inten_loam;
namespace serial = localization_zoo::evaluation::inten_trace;
inline std::ostream* masks = nullptr;
inline std::ostream* correspondences = nullptr;
inline std::ostream* ranges = nullptr;
inline std::set<int> detailed;
inline int stage = 2;

inline void feature(std::ostream& out, const PaperFeature& f) {
  out << '[' << f.p.x() << ',' << f.p.y() << ',' << f.p.z() << ','
      << f.row << ',' << f.cluster << ',' << f.intensity << ']';
}

inline void facade(int frame, const PaperFeature& source,
                   const std::vector<PaperFeature>& target,
                   const std::vector<int>& ids, const std::vector<float>& ds,
                   int same, int adj, const Eigen::Vector3d& x,
                   const Eigen::Vector3d& normal, double residual) {
  if (!correspondences) return;
  auto out = serial::record();
  out << "{\"frame\":" << frame << ",\"source\":"; feature(out, source);
  out << ",\"x\":"; serial::matrix(out, x);
  out << ",\"normal\":"; serial::matrix(out, normal);
  out << ",\"r\":" << residual << ",\"same\":" << same << ",\"adjacent\":" << adj << ",\"neighbours\":[";
  for (size_t k = 0; k < ids.size(); ++k) {
    if (k) out << ',';
    out << '[' << ids[k] << ',' << ds[k] << ',';
    feature(out, target[static_cast<size_t>(ids[k])]); out << ']';
  }
  out << "]}";
  *correspondences << out.str() << '\n';
}

inline void mask(int frame, const InTenLoamPaperParams& params, const PaperFrame& current,
                 const Eigen::Matrix4d& pose, const std::vector<float>& rendered,
                 const std::vector<int>& total, const std::vector<int>& covered,
                 const std::vector<int>& flagged, const std::vector<char>& dynamic,
                 int map_pixels) {
  if (!masks) return;
  std::vector<int> facade_count(total.size(), 0), edge_count(total.size(), 0);
  for (const auto& f : current.facade) ++facade_count[static_cast<size_t>(f.cluster)];
  for (const auto& f : current.edge) ++edge_count[static_cast<size_t>(f.cluster)];
  auto out = serial::record();
  out << "{\"frame\":" << frame << ",\"stage\":" << stage << ",\"pose\":";
  serial::matrix(out, pose);
  out << ",\"map_pixels_before_previous_render\":" << map_pixels
      << ",\"total_rendered_pixels\":" << std::count_if(rendered.begin(), rendered.end(), [](float r) { return r > 0; })
      << ",\"clusters\":[";
  bool first = true;
  for (size_t c = 2; c < total.size(); ++c) {
    if (!total[c]) continue;
    if (!first) out << ','; first = false;
    out << '[' << c << ',' << total[c] << ',' << covered[c] << ',' << flagged[c]
        << ',' << static_cast<int>(dynamic[c]) << ',' << facade_count[c] << ',' << edge_count[c] << ']';
  }
  out << "]}";
  *masks << out.str() << '\n';
  if (!ranges || !detailed.count(frame)) return;
  const double down = params.theta_down_deg*M_PI/180.;
  const double theta = (params.theta_down_deg+params.theta_up_deg)*M_PI/180.;
  for (const auto& f : current.all) {
    if (f.cluster < 2) continue;
    const double r = f.p.norm();
    int u = static_cast<int>(std::floor((1.-std::atan2(f.p.y(), f.p.x())/M_PI)*params.width/2.));
    u = ((u%params.width)+params.width)%params.width;
    const int v = static_cast<int>(std::floor((std::asin(f.p.z()/r)+down)/theta*params.height));
    const bool valid = r >= params.min_range && r <= params.max_range && v >= 0 && v < params.height;
    const float dm = valid ? rendered[static_cast<size_t>(v*params.width+u)] : 0.f;
    auto row = serial::record();
    row << "[" << frame << ',' << stage << ',';
    feature(row, f);
    row << ',' << (valid ? v*params.width+u : -1) << ',' << r << ',' << dm << "]";
    *ranges << row.str() << '\n';
  }
}
}  // namespace diagnostic
