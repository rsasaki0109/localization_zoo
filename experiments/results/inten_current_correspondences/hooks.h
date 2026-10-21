// Isolated study producer only. No production API or estimator change.
#pragma once
#include "inten_loam/inten_loam_paper.h"
#include "inten_trace_jsonl.h"
#include <ostream>

namespace diagnostic {
using namespace localization_zoo::inten_loam;
namespace serial = localization_zoo::evaluation::inten_trace;
inline std::ostream* output = nullptr;
inline bool in_s2s = false;
inline int frame = -1, iteration = -1, cls = -1;
inline void feature(std::ostream& out, const PaperFeature& f) {
  out << '[' << f.p.x() << ',' << f.p.y() << ',' << f.p.z() << ','
      << f.row << ',' << f.cluster << ',' << f.intensity << ']';
}
inline void residual(const PaperFeature& f, const Eigen::Vector3d& x,
                     const Eigen::Vector3d& a, double r, double huber) {
  if (!output || !in_s2s) return;
  auto row = serial::record();
  row << "{\"type\":\"residual\",\"frame\":" << frame << ",\"iteration\":" << iteration
      << ",\"class\":" << cls << ",\"source\":";
  feature(row, f);
  row << ",\"x\":"; serial::matrix(row, x);
  row << ",\"a\":"; serial::matrix(row, a);
  row << ",\"r\":" << r << ",\"weight\":" << (std::abs(r) <= huber ? 1.0 : huber / std::abs(r)) << '}';
  *output << row.str() << '\n';
}
inline void plane(const PaperFeature& source, const std::vector<PaperFeature>& target,
                  const std::vector<int>& ids, const std::vector<float>& ds,
                  int same, int adj, const Eigen::Vector3d& x,
                  const Eigen::Vector3d& normal, double r) {
  if (!output || !in_s2s) return;
  auto row = serial::record();
  row << "{\"type\":\"plane\",\"frame\":" << frame << ",\"iteration\":" << iteration
      << ",\"class\":" << cls << ",\"source\":";
  feature(row, source);
  row << ",\"x\":"; serial::matrix(row, x);
  row << ",\"normal\":"; serial::matrix(row, normal);
  row << ",\"r\":" << r << ",\"same\":" << same << ",\"adjacent\":" << adj << ",\"neighbours\":[";
  for (size_t k = 0; k < ids.size(); ++k) {
    if (k) row << ',';
    row << '[' << ids[k] << ',' << ds[k] << ',';
    feature(row, target[static_cast<size_t>(ids[k])]);
    row << ']';
  }
  row << "]}";
  *output << row.str() << '\n';
}
}  // namespace diagnostic
