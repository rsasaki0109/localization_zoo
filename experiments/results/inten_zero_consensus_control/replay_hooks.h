#pragma once
#include "inten_loam/detail/s2s_consensus.h"
namespace replay {
inline const localization_zoo::inten_loam::PaperFrame* frame = nullptr;
inline const Eigen::Matrix4d* seed = nullptr;
inline const std::vector<localization_zoo::inten_loam::detail::ClusterTranslationProfile>* profiles = nullptr;
inline int mode = 0;
inline void select(localization_zoo::inten_loam::PaperConsensusTrace* c) {
  using namespace localization_zoo::inten_loam;
  if ((mode == 2 || mode == 5 || mode == 6) &&
      c->valid_clusters > 0 && 2*c->votes[60] > c->valid_clusters) {
    c->best_index = 60;
    c->usable = true;
    c->excluded_clusters.clear();
    for (const auto& band : c->admissions)
      if (!detail::admitted(band, 60)) c->excluded_clusters.push_back(band.cluster);
  }
  if (mode == 3 || mode == 5 || mode == 7) c->excluded_clusters.clear();
}
inline int seedIndex(int selected) {
  return mode == 4 || mode == 6 || mode == 7 ? 60 : selected;
}
}
