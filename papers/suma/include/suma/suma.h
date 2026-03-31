#pragma once

#include "aloam/types.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <deque>
#include <vector>

namespace localization_zoo {
namespace suma {

struct SuMaParams {
  int n_scans = 16;
  int num_columns = 1800;
  double min_range = 1.0;
  double max_range = 80.0;
  double surfel_resolution = 0.4;
  int surfel_stride = 1;
  int max_iterations = 4;
  int ceres_max_iterations = 6;
  int knn = 5;
  double max_correspondence_distance = 3.0;
  double plane_threshold = 0.25;
  double normal_alignment_threshold = 0.5;
  double huber_loss_s = 0.1;
  int max_frames_in_map = 20;
};

struct SuMaResult {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  size_t num_surfels = 0;
  int num_correspondences = 0;
  size_t num_frames_in_map = 0;
  bool initialized = false;
  bool valid = false;
};

class SuMa {
public:
  explicit SuMa(const SuMaParams& params = SuMaParams());

  SuMaResult process(const aloam::PointCloudConstPtr& cloud);
  void clear();

  const Eigen::Quaterniond& orientation() const { return q_w_curr_; }
  const Eigen::Vector3d& position() const { return t_w_curr_; }
  size_t numFramesInMap() const { return frame_history_.size(); }

private:
  struct SurfelFrame {
    aloam::PointCloudPtr points;
    std::vector<Eigen::Vector3d> normals;
  };

  struct PoseState {
    Eigen::Quaterniond q = Eigen::Quaterniond::Identity();
    Eigen::Vector3d t = Eigen::Vector3d::Zero();
  };

  int computeScanId(float x, float y, float z) const;
  SurfelFrame extractSurfels(const aloam::PointCloudConstPtr& cloud) const;
  SurfelFrame voxelizeSurfels(const SurfelFrame& frame) const;
  SurfelFrame transformFrame(const SurfelFrame& frame,
                             const Eigen::Quaterniond& quat,
                             const Eigen::Vector3d& trans) const;
  void rebuildMap();

  static PoseState composePose(const PoseState& a, const PoseState& b);
  static PoseState relativePose(const PoseState& from, const PoseState& to);

  SuMaParams params_;
  std::deque<SurfelFrame> frame_history_;
  aloam::PointCloudPtr map_points_;
  std::vector<Eigen::Vector3d> map_normals_;

  Eigen::Quaterniond q_w_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr_ = Eigen::Vector3d::Zero();
  PoseState last_motion_;
  bool initialized_ = false;
};

}  // namespace suma
}  // namespace localization_zoo
