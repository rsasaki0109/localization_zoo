#include "v_loam15/v_loam15.h"

#include <algorithm>

namespace localization_zoo {
namespace v_loam15 {

opl_lvio::OplLvioParams VLoam15Pipeline::makeBackendParams(
    const VLoam15Params& params) {
  auto p = params.backend;
  p.visual_point_weight = 0.035;
  p.line_weight = 0.0;
  p.direction_weight = 0.0;
  p.min_visual_points = 12;
  p.min_visual_lines = 1000000;
  p.max_visual_points = std::max(p.max_visual_points, 260);
  p.motion_prior_weight = 3e-4;
  p.visual_point_curvature_threshold = 0.10;
  return p;
}

VLoam15Pipeline::VLoam15Pipeline(const VLoam15Params& params)
    : params_(params), backend_(makeBackendParams(params)) {}

void VLoam15Pipeline::reset() { backend_.reset(); }

void VLoam15Pipeline::setInitialPose(const Eigen::Matrix4d& pose) {
  backend_.setInitialPose(pose);
}

opl_lvio::FrameFeatures VLoam15Pipeline::extractFeatures(
    const std::vector<Eigen::Vector3d>& frame) const {
  return backend_.extractFeatures(frame);
}

VLoam15Result VLoam15Pipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  VLoam15Result out;
  out.backend = backend_.registerFrame(frame);
  out.pose = out.backend.pose;
  return out;
}

}  // namespace v_loam15
}  // namespace localization_zoo
