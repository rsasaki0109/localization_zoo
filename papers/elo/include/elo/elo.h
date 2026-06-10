#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <vector>

namespace localization_zoo {
namespace elo {

/// ELO: Efficient LiDAR Odometry for Autonomous Driving
/// (Zheng & Zhu, IEEE RA-L 2021, arXiv:2104.10879).
///
/// CPU from-paper reimplementation of the core LiDAR-only front-end:
/// non-ground points are associated through a spherical range-image model,
/// ground points through a bird's-eye-view map, and both residual sets are
/// fused in point-to-plane Gauss-Newton odometry.  The model is kept in the
/// latest local LiDAR frame and updated by projectively fusing the previous
/// model with the current scan.
struct EloParams {
  double min_range = 1.0;
  double max_range = 80.0;

  int sri_width = 2048;
  int sri_height = 80;
  double fov_up_deg = 2.0;
  double fov_down_deg = -24.8;

  double bev_x_min = -40.0;
  double bev_x_max = 80.0;
  double bev_y_min = -30.0;
  double bev_y_max = 30.0;
  double bev_resolution = 0.2;

  double sensor_height = 1.73;
  double ground_height_above = 0.5;
  double ground_height_below = 1.0;
  double ground_angle_threshold_deg = 5.0;

  double normal_range_delta = 0.3;
  double normal_range_outlier_threshold = 0.5;
  double normal_plane_outlier_threshold = 0.5;
  int normal_lx_min = 5;
  int normal_lx_max = 13;
  int normal_ly_min = 3;
  int normal_ly_max = 7;
  int normal_min_neighbors = 5;
  double curvature_threshold = 0.1;

  int max_iterations = 20;
  double convergence_eps = 1e-4;
  int min_correspondences = 30;
  double max_sri_correspondence_dist = 1.0;
  double max_ground_correspondence_dist = 1.0;
  int sri_search_radius_px = 1;
  int ground_search_radius_cells = 3;
  int ground_normal_radius_cells = 4;
  double robust_scale = 0.5;
  double non_ground_weight = 1.0;
  double ground_weight = 1.5;
  int max_alignment_points = 12000;

  int model_window_frames = 100;
};

struct EloResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  int num_non_ground_correspondences = 0;
  int num_ground_correspondences = 0;
  int num_ground_points = 0;
  int num_non_ground_points = 0;
  double mean_abs_residual = 0.0;
};

class EloPipeline {
 public:
  explicit EloPipeline(const EloParams& params = EloParams());

  void reset();
  void setInitialPose(const Eigen::Matrix4d& pose);

  EloResult registerFrame(const std::vector<Eigen::Vector3d>& points);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t nonGroundMapSize() const;
  size_t groundMapSize() const;

 private:
  struct ScanCell {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::UnitZ();
    double range = 0.0;
    double curvature = 1.0;
    bool valid = false;
    bool ground = false;
    bool has_normal = false;
  };

  struct FramePoint {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::UnitZ();
    bool ground = false;
    bool has_normal = false;
  };

  struct FrameData {
    std::vector<ScanCell> sri;
    std::vector<FramePoint> points;
    int ground_points = 0;
    int non_ground_points = 0;
  };

  struct ModelCell {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::UnitZ();
    double range = 0.0;
    int stamp = 0;
    int count = 0;
    bool valid = false;
    bool has_normal = false;
  };

  struct Correspondence {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::UnitZ();
    bool ground = false;
  };

  int sriIndex(int row, int col) const;
  int bevWidth() const;
  int bevHeight() const;
  int bevIndex(int row, int col) const;
  bool projectSri(const Eigen::Vector3d& p, int* row, int* col) const;
  bool projectBev(const Eigen::Vector3d& p, int* row, int* col) const;

  FrameData buildFrame(const std::vector<Eigen::Vector3d>& points) const;
  void segmentGround(FrameData* frame) const;
  void computeRangeAdaptiveNormals(FrameData* frame) const;
  void collectFramePoints(FrameData* frame) const;

  bool lookupSri(const Eigen::Vector3d& p, Correspondence* corr) const;
  bool lookupGround(const Eigen::Vector3d& p, Correspondence* corr) const;
  bool estimateGroundNormal(int row, int col, Eigen::Vector3d* normal) const;
  Eigen::Matrix4d align(const FrameData& frame, const Eigen::Matrix4d& init,
                        EloResult* result) const;

  void initializeModel(const FrameData& frame);
  void updateModel(const FrameData& frame, const Eigen::Matrix4d& model_to_current);
  void insertSri(std::vector<ModelCell>* model, const Eigen::Vector3d& point,
                 const Eigen::Vector3d& normal, int stamp);
  void insertBev(std::vector<ModelCell>* model, const Eigen::Vector3d& point,
                 int stamp, int count = 1);

  EloParams params_;
  int frame_count_ = 0;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d model_pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  bool initialized_ = false;

  std::vector<ModelCell> sri_model_;
  std::vector<ModelCell> bev_model_;
};

}  // namespace elo
}  // namespace localization_zoo
