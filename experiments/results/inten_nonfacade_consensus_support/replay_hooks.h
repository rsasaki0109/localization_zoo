#pragma once
#include "inten_loam/detail/s2s_consensus.h"
namespace replay {
inline const localization_zoo::inten_loam::PaperFrame* frame = nullptr;
inline const Eigen::Matrix4d* seed = nullptr;
inline const std::vector<localization_zoo::inten_loam::detail::ClusterTranslationProfile>* profiles = nullptr;
}
