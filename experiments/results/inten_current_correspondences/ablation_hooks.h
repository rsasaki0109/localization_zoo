#pragma once
#include "hooks.h"
namespace diagnostic {
inline int exclude_mode = 0, exclude_cluster = -1;
inline bool exclude(const localization_zoo::inten_loam::PaperFeature& f) {
  if (exclude_mode == 1) return f.cluster == exclude_cluster;
  if (exclude_mode == 2) return true;
  if (exclude_mode == 3) return f.p.x() >= -12 && f.p.x() <= 15 &&
      f.p.y() >= -5 && f.p.y() <= -2 && f.p.z() >= -1.6 && f.p.z() <= .5;
  return false;
}
}
