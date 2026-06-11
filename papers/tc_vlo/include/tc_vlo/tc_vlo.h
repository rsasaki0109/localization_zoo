#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <vector>

#include "opl_lvio/opl_lvio.h"

namespace localization_zoo {
namespace tc_vlo {

struct TcVloParams {
  opl_lvio::OplLvioParams backend;
};

struct TcVloResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  opl_lvio::OplLvioResult backend;
};

class TcVloPipeline {
 public:
  explicit TcVloPipeline(const TcVloParams& params = TcVloParams());

  void reset();
  void setInitialPose(const Eigen::Matrix4d& pose);

  opl_lvio::FrameFeatures extractFeatures(
      const std::vector<Eigen::Vector3d>& frame) const;
  TcVloResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return backend_.pose(); }
  size_t mapSize() const { return backend_.mapSize(); }
  size_t lineMapSize() const { return backend_.lineMapSize(); }

  static opl_lvio::OplLvioParams makeBackendParams(const TcVloParams& params);

 private:
  TcVloParams params_;
  opl_lvio::OplLvioPipeline backend_;
};

}  // namespace tc_vlo
}  // namespace localization_zoo
