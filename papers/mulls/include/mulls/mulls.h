#pragma once

#include "aloam/laser_odometry.h"
#include "aloam/scan_registration.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <deque>

namespace localization_zoo {
namespace mulls {

struct MULLSMappingParams {
  double line_resolution = 0.4;
  double plane_resolution = 0.8;
  double point_resolution = 1.0;
  int num_optimization_iters = 2;
  int ceres_max_iterations = 4;
  double huber_loss_s = 0.1;
  int knn = 5;
  double knn_max_dist_sq = 1.0;
  double edge_eigenvalue_ratio = 3.0;
  double plane_threshold = 0.2;
  double point_neighbor_max_dist_sq = 4.0;
  double line_weight = 1.0;
  double plane_weight = 1.0;
  double point_weight = 0.3;
  int full_downsample_rate = 5;
  int max_frames_in_map = 30;
};

struct MULLSMappingResult {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  int num_line_constraints = 0;
  int num_plane_constraints = 0;
  int num_point_constraints = 0;
  size_t num_frames_in_map = 0;
  bool valid = false;
};

class MULLSMapping {
public:
  explicit MULLSMapping(const MULLSMappingParams& params = MULLSMappingParams());

  MULLSMappingResult process(const aloam::PointCloudPtr& corner_last,
                             const aloam::PointCloudPtr& surf_last,
                             const aloam::PointCloudPtr& full_res,
                             const Eigen::Quaterniond& q_init,
                             const Eigen::Vector3d& t_init);

  void clear();

  const Eigen::Quaterniond& orientation() const { return q_w_curr_; }
  const Eigen::Vector3d& position() const { return t_w_curr_; }
  size_t numFramesInMap() const { return frame_history_.size(); }

private:
  struct FrameClouds {
    aloam::PointCloudPtr corner;
    aloam::PointCloudPtr surf;
    aloam::PointCloudPtr points;
  };

  void rebuildMaps();
  FrameClouds makeFrameClouds(const aloam::PointCloudPtr& corner_last,
                              const aloam::PointCloudPtr& surf_last,
                              const aloam::PointCloudPtr& full_res) const;

  MULLSMappingParams params_;
  std::deque<FrameClouds> frame_history_;
  aloam::PointCloudPtr corner_map_;
  aloam::PointCloudPtr surf_map_;
  aloam::PointCloudPtr point_map_;
  Eigen::Quaterniond q_w_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr_ = Eigen::Vector3d::Zero();
  bool initialized_ = false;
};

struct MULLSParams {
  aloam::ScanRegistrationParams scan_registration;
  aloam::LaserOdometryParams odometry;
  MULLSMappingParams mapping;
};

struct MULLSResult {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  size_t num_corner_features = 0;
  size_t num_surface_features = 0;
  int num_line_constraints = 0;
  int num_plane_constraints = 0;
  int num_point_constraints = 0;
  size_t num_frames_in_map = 0;
  bool valid = false;
};

class MULLS {
public:
  explicit MULLS(const MULLSParams& params = MULLSParams());

  MULLSResult process(const aloam::PointCloudConstPtr& cloud);

  size_t numFramesInMap() const { return mapping_.numFramesInMap(); }

private:
  aloam::ScanRegistration scan_registration_;
  aloam::LaserOdometry odometry_;
  MULLSMapping mapping_;
};

}  // namespace mulls
}  // namespace localization_zoo
