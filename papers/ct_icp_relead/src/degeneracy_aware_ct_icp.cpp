#include "ct_icp_relead/degeneracy_aware_ct_icp.h"

#include <iostream>

namespace localization_zoo {
namespace ct_icp_relead {

DegeneracyAwareCTICP::DegeneracyAwareCTICP(const DegAwareCTICPParams& params)
    : ct_icp_reg_(params.ct_icp_params),
      deg_detector_(params.deg_params),
      params_(params) {}

void DegeneracyAwareCTICP::addPointsToMap(
    const std::vector<Eigen::Vector3d>& world_points) {
  ct_icp_reg_.addPointsToMap(world_points);
}

void DegeneracyAwareCTICP::clearMap() { ct_icp_reg_.clearMap(); }
size_t DegeneracyAwareCTICP::mapSize() const { return ct_icp_reg_.mapSize(); }

DegAwareCTICPResult DegeneracyAwareCTICP::registerFrame(
    const std::vector<ct_icp::TimedPoint>& timed_points,
    const ct_icp::TrajectoryFrame& initial_frame,
    const ct_icp::TrajectoryFrame* previous_frame) {
  DegAwareCTICPResult result;

  // Step 1: 通常のCT-ICPレジストレーション実行
  result.ct_result =
      ct_icp_reg_.registerFrame(timed_points, initial_frame, previous_frame);

  // Step 2: CT-ICPの結果からpoint-to-plane対応関係を再構築して退化検知
  // end_poseで変換した点と法線を使う
  std::vector<relead::PointWithNormal> correspondences;
  auto& frame = result.ct_result.frame;

  for (const auto& tp : timed_points) {
    Eigen::Vector3d world_pt =
        frame.transformPoint(tp.raw_point, tp.timestamp);

    // 簡易法線: z方向 (地面仮定)
    // 実際にはマップからの近傍探索+PCAが必要
    relead::PointWithNormal pn;
    pn.point = world_pt;
    pn.normal = Eigen::Vector3d(0, 0, 1);
    pn.closest = world_pt;
    pn.closest.z() = 0;
    correspondences.push_back(pn);

    if (correspondences.size() > 500) break;
  }

  result.degeneracy_info = deg_detector_.detect(correspondences);

  // Step 3: 退化が検知された場合、end_poseの退化方向成分を
  // previous_frameからの外挿値で置き換え
  if (result.degeneracy_info.hasDegeneracy() && previous_frame) {
    auto& end_pose = result.ct_result.frame.end_pose;

    for (const auto& v : result.degeneracy_info.degenerate_translation_dirs) {
      // 退化方向の並進をprevious_frameの等速仮定で置換
      Eigen::Vector3d prev_vel =
          previous_frame->end_pose.trans - previous_frame->begin_pose.trans;
      double proj_vel = v.dot(prev_vel);
      double proj_current = v.dot(end_pose.trans);
      double proj_expected =
          v.dot(previous_frame->end_pose.trans) + proj_vel;

      // 退化方向の更新を等速仮定値に置き換え
      end_pose.trans += v * (proj_expected - proj_current);
    }
  }

  return result;
}

}  // namespace ct_icp_relead
}  // namespace localization_zoo
