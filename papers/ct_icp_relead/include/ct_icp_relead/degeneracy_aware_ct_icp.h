#pragma once

#include "ct_icp/ct_icp_registration.h"
#include "relead/degeneracy_detection.h"

namespace localization_zoo {
namespace ct_icp_relead {

struct DegAwareCTICPParams {
  ct_icp::CTICPParams ct_icp_params;
  relead::DegeneracyDetectionParams deg_params;
};

struct DegAwareCTICPResult {
  ct_icp::CTICPResult ct_result;
  relead::DegeneracyInfo degeneracy_info;
};

/// 退化検知付き CT-ICP
class DegeneracyAwareCTICP {
public:
  explicit DegeneracyAwareCTICP(
      const DegAwareCTICPParams& params = DegAwareCTICPParams());

  void addPointsToMap(const std::vector<Eigen::Vector3d>& world_points);

  DegAwareCTICPResult registerFrame(
      const std::vector<ct_icp::TimedPoint>& timed_points,
      const ct_icp::TrajectoryFrame& initial_frame,
      const ct_icp::TrajectoryFrame* previous_frame = nullptr);

  void clearMap();
  size_t mapSize() const;

private:
  ct_icp::CTICPRegistration ct_icp_reg_;
  relead::DegeneracyDetection deg_detector_;
  DegAwareCTICPParams params_;
};

}  // namespace ct_icp_relead
}  // namespace localization_zoo
