#pragma once
#include "inten_loam/inten_loam_paper.h"
#include "inten_trace_jsonl.h"
#include <set>
namespace diagnostic {
inline const localization_zoo::inten_loam::PaperFrame* frame = nullptr;
inline localization_zoo::inten_loam::PaperSolverTrace* terminal_trace = nullptr;
inline std::set<int> excluded;
inline bool translation_only = false;
inline std::ostream* capture = nullptr;
inline int frame_id = -1, mode = -1, cls = -1, iteration = -1;
inline void residual(const Eigen::Vector3d& x, const Eigen::Vector3d& a, double r, double h) {
  if (!capture || iteration != 0) return;
  namespace serial = localization_zoo::evaluation::inten_trace;
  auto out = serial::record();
  out << "{\"frame\":" << frame_id << ",\"mode\":" << mode << ",\"class\":" << cls << ",\"x\":";
  serial::matrix(out, x); out << ",\"a\":"; serial::matrix(out, a);
  out << ",\"r\":" << r << ",\"weight\":" << (std::abs(r) <= h ? 1. : h/std::abs(r)) << '}';
  *capture << out.str() << '\n';
}
}
