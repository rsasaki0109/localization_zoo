#pragma once
#include "inten_loam/inten_loam_paper.h"
namespace replay {
inline const localization_zoo::inten_loam::PaperFrame* frame=nullptr;
inline const Eigen::Matrix4d* seed=nullptr;
inline bool reciprocal=false;
inline bool common_prior=false;
inline const Eigen::Matrix3d* reference_joint=nullptr;
inline const Eigen::Matrix3d* reference_refinement=nullptr;
inline int frame_number=-1,mode=-1;
}
