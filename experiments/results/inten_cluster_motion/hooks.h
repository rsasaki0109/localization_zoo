#pragma once
#include "inten_loam/inten_loam_paper.h"
#include "inten_trace_jsonl.h"
#include <set>
namespace diagnostic {
inline const localization_zoo::inten_loam::PaperFrame* frame = nullptr;
inline localization_zoo::inten_loam::PaperSolverTrace* terminal_trace = nullptr;
inline std::set<int> excluded;
}
