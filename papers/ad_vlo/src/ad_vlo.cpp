#include "ad_vlo/ad_vlo.h"

#include <algorithm>

namespace localization_zoo {
namespace ad_vlo {

opl_lvio::OplLvioParams AdVloPipeline::makeBackendParams(
    const AdVloParams& params) {
  auto p = params.backend;
  p.visual_point_weight = 0.025;
  p.line_weight = 0.0;
  p.direction_weight = 0.0;
  p.plane_weight = 1.15;
  p.min_visual_points = 16;
  p.min_visual_lines = 1000000;
  p.visual_point_curvature_threshold = 0.16;
  p.visual_range_jump = 0.55;
  p.max_visual_points = std::max(p.max_visual_points, 220);
  p.motion_prior_weight = 1e-4;
  return p;
}

AdVloPipeline::AdVloPipeline(const AdVloParams& params)
    : params_(params), backend_(makeBackendParams(params)) {}

void AdVloPipeline::reset() { backend_.reset(); }

void AdVloPipeline::setInitialPose(const Eigen::Matrix4d& pose) {
  backend_.setInitialPose(pose);
}

opl_lvio::FrameFeatures AdVloPipeline::extractFeatures(
    const std::vector<Eigen::Vector3d>& frame) const {
  return backend_.extractFeatures(frame);
}

AdVloResult AdVloPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  AdVloResult out;
  out.backend = backend_.registerFrame(frame);
  out.pose = out.backend.pose;
  return out;
}

}  // namespace ad_vlo
}  // namespace localization_zoo
