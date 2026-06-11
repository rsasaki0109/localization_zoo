#include "tc_vlo/tc_vlo.h"

#include <algorithm>

namespace localization_zoo {
namespace tc_vlo {

opl_lvio::OplLvioParams TcVloPipeline::makeBackendParams(
    const TcVloParams& params) {
  auto p = params.backend;
  p.visual_point_weight = 0.055;
  p.line_weight = 0.10;
  p.direction_weight = 0.025;
  p.min_visual_points = 12;
  p.min_visual_lines = 4;
  p.max_visual_points = std::max(p.max_visual_points, 300);
  p.max_visual_lines = std::max(p.max_visual_lines, 120);
  p.visual_line_bonus = 1.3;
  p.motion_prior_weight = 1e-4;
  return p;
}

TcVloPipeline::TcVloPipeline(const TcVloParams& params)
    : params_(params), backend_(makeBackendParams(params)) {}

void TcVloPipeline::reset() { backend_.reset(); }

void TcVloPipeline::setInitialPose(const Eigen::Matrix4d& pose) {
  backend_.setInitialPose(pose);
}

opl_lvio::FrameFeatures TcVloPipeline::extractFeatures(
    const std::vector<Eigen::Vector3d>& frame) const {
  return backend_.extractFeatures(frame);
}

TcVloResult TcVloPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  TcVloResult out;
  out.backend = backend_.registerFrame(frame);
  out.pose = out.backend.pose;
  return out;
}

}  // namespace tc_vlo
}  // namespace localization_zoo
