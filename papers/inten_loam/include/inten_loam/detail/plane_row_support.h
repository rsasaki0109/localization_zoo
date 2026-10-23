#pragma once

#include "inten_loam/inten_loam_paper.h"

#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace inten_loam {
namespace detail {

// Repository extension, used only when explicitly enabled. Complete missing
// scan-line support outside the original ten neighbours, keeping the original
// closest point, any already selected support, and the original distance gate.
// Rows are the extractor's elevation bins, not measured laser ring IDs.
class PlaneRowSupport {
 public:
  void build(const std::vector<PaperFeature>& points) {
    rows_.clear();
    for (size_t i = 0; i < points.size(); ++i)
      rows_[points[i].row].push_back(static_cast<int>(i));
  }

  void complete(const std::vector<PaperFeature>& points, const Eigen::Vector3d& x,
                int closest, double gate_sq, int* same, int* adjacent) const {
    const int row = points[static_cast<size_t>(closest)].row;
    if (*same < 0) *same = nearest(points, x, row, row, closest, gate_sq);
    if (*adjacent < 0) *adjacent = nearest(points, x, row - 2, row + 2, closest, gate_sq, row);
  }

 private:
  int nearest(const std::vector<PaperFeature>& points, const Eigen::Vector3d& x,
              int first_row, int last_row, int excluded, double gate_sq,
              int excluded_row = -1) const {
    const float q[3] = {static_cast<float>(x.x()), static_cast<float>(x.y()), static_cast<float>(x.z())};
    int best = -1;
    float best_distance = 0;
    for (int row = first_row; row <= last_row; ++row) {
      if (row == excluded_row) continue;
      const auto found = rows_.find(row);
      if (found == rows_.end()) continue;
      for (const int index : found->second) {
        if (index == excluded) continue;
        const auto& p = points[static_cast<size_t>(index)].p;
        const float dx = q[0] - static_cast<float>(p.x());
        const float dy = q[1] - static_cast<float>(p.y());
        const float dz = q[2] - static_cast<float>(p.z());
        const float distance = (dx * dx + dy * dy) + dz * dz;
        if (distance <= gate_sq &&
            (best < 0 || distance < best_distance || (distance == best_distance && index < best))) {
          best = index;
          best_distance = distance;
        }
      }
    }
    return best;
  }

  std::unordered_map<int, std::vector<int>> rows_;
};

}  // namespace detail
}  // namespace inten_loam
}  // namespace localization_zoo
