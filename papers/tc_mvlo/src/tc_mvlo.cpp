#include "tc_mvlo/tc_mvlo.h"

#include <algorithm>

namespace localization_zoo {
namespace tc_mvlo {

opl_lvio::OplLvioParams TcMvloPipeline::makeBackendParams(
    const TcMvloParams& params) {
  auto p = params.backend;
  p.visual_point_weight = 0.065;
  p.line_weight = 0.12;
  p.direction_weight = 0.035;
  p.min_visual_points = 10;
  p.min_visual_lines = 4;
  p.max_visual_points = std::max(p.max_visual_points, 320);
  p.max_visual_lines = std::max(p.max_visual_lines, 140);
  p.visual_line_bonus = 1.8;
  p.helmert_min_scale = 0.35;
  p.helmert_max_scale = 3.0;
  p.motion_prior_weight = 8e-5;
  return p;
}

TcMvloPipeline::TcMvloPipeline(const TcMvloParams& params)
    : params_(params), backend_(makeBackendParams(params)) {}

void TcMvloPipeline::reset() { backend_.reset(); }

void TcMvloPipeline::setInitialPose(const Eigen::Matrix4d& pose) {
  backend_.setInitialPose(pose);
}

opl_lvio::FrameFeatures TcMvloPipeline::extractFeatures(
    const std::vector<Eigen::Vector3d>& frame) const {
  return backend_.extractFeatures(frame);
}

TcMvloResult TcMvloPipeline::registerFrame(
    const std::vector<Eigen::Vector3d>& frame) {
  TcMvloResult out;
  out.backend = backend_.registerFrame(frame);
  out.pose = out.backend.pose;
  return out;
}

}  // namespace tc_mvlo
}  // namespace localization_zoo
