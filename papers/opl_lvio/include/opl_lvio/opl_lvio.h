#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace opl_lvio {

struct VoxelHash {
  std::size_t operator()(const Eigen::Vector3i& v) const {
    std::size_t seed = 0;
    auto mix = [&seed](int x) {
      seed ^= std::hash<int>()(x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    mix(v.x());
    mix(v.y());
    mix(v.z());
    return seed;
  }
};

struct LineFeature {
  Eigen::Vector3d center = Eigen::Vector3d::Zero();
  Eigen::Vector3d direction = Eigen::Vector3d::UnitX();
  double weight = 1.0;
  bool visual_supported = false;
};

struct PointFeature {
  Eigen::Vector3d point = Eigen::Vector3d::Zero();
  double weight = 1.0;
};

struct FrameFeatures {
  std::vector<Eigen::Vector3d> registration_points;
  std::vector<PointFeature> visual_points;
  std::vector<LineFeature> lidar_lines;
  std::vector<LineFeature> visual_lines;
  std::vector<LineFeature> fused_lines;
};

struct OplLvioParams {
  double min_range = 1.0;
  double max_range = 80.0;

  int image_width = 1024;
  int image_height = 64;
  double fov_up_deg = 3.0;
  double fov_down_deg = -25.0;

  double voxel_size = 1.0;
  int max_points_per_voxel = 10;
  int normal_min_neighbors = 5;
  int max_icp_iterations = 24;
  double convergence_criterion = 0.001;
  double max_correspondence_dist = 2.0;
  int min_correspondences = 20;

  double registration_voxel_size = 0.8;
  int max_registration_points = 5000;
  int max_visual_points = 320;
  int min_visual_points = 20;
  int max_lidar_lines = 180;
  int max_visual_lines = 120;
  int min_visual_lines = 8;

  int visual_point_neighbor_gap = 3;
  double visual_point_curvature_threshold = 0.12;
  double visual_point_weight = 0.08;
  int lidar_line_window_rows = 2;
  int lidar_line_window_cols = 5;
  int min_line_neighbors = 6;
  double line_linearity_threshold = 0.75;
  double visual_range_jump = 0.8;
  int visual_max_pixel_gap = 6;
  int visual_min_segment_pixels = 8;
  double visual_linearity_threshold = 0.7;
  double fusion_pixel_gate = 8.0;

  double plane_weight = 1.0;
  double line_weight = 0.0;
  double direction_weight = 0.0;
  double visual_line_bonus = 1.5;
  double helmert_min_scale = 0.25;
  double helmert_max_scale = 4.0;
  double motion_prior_weight = 1e-4;

  double map_update_radius = 0.5;
  double local_map_radius = 70.0;
  int map_cleanup_interval = 4;
};

struct OplLvioResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_plane_correspondences = 0;
  int num_point_correspondences = 0;
  int num_line_correspondences = 0;
  int num_visual_points = 0;
  int num_visual_lines = 0;
  int num_fused_lines = 0;
  bool visual_used = false;
  bool lidar_fallback = false;
  double point_weight_scale = 1.0;
  double line_weight_scale = 1.0;
  double mean_abs_residual = 0.0;
};

class OplLvioPipeline {
 public:
  explicit OplLvioPipeline(const OplLvioParams& params = OplLvioParams());

  void reset();
  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }

  bool projectToSpherical(const Eigen::Vector3d& point, int* row, int* col,
                          double* range = nullptr) const;
  FrameFeatures extractFeatures(const std::vector<Eigen::Vector3d>& frame) const;
  OplLvioResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const;
  size_t lineMapSize() const;

 private:
  struct RangeCell {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    double range = 0.0;
    bool valid = false;
  };

  struct PlaneMatch {
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::UnitZ();
    bool found = false;
  };

  struct PointMatch {
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();
    double weight = 1.0;
    bool found = false;
  };

  struct LineMatch {
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();
    Eigen::Vector3d direction = Eigen::Vector3d::UnitX();
    double weight = 1.0;
    bool found = false;
  };

  struct MapPoint {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    int stamp = 0;
    bool removed = false;
  };

  struct VisualMapPoint {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    int observations = 1;
    int stamp = 0;
    bool removed = false;
  };

  struct MapLine {
    Eigen::Vector3d center = Eigen::Vector3d::Zero();
    Eigen::Vector3d direction = Eigen::Vector3d::UnitX();
    int observations = 1;
    int stamp = 0;
    bool removed = false;
  };

  Eigen::Vector3i voxelKey(const Eigen::Vector3d& p) const;
  int imageIndex(int row, int col) const;

  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<RangeCell> buildRangeImage(
      const std::vector<Eigen::Vector3d>& points) const;
  std::vector<PointFeature> extractVisualPoints(
      const std::vector<RangeCell>& image) const;
  std::vector<LineFeature> extractLidarLines(
      const std::vector<RangeCell>& image) const;
  std::vector<LineFeature> extractVisualLines(
      const std::vector<RangeCell>& image) const;
  std::vector<LineFeature> fuseLines(
      const std::vector<LineFeature>& lidar_lines,
      const std::vector<LineFeature>& visual_lines) const;

  bool fitLine(const std::vector<Eigen::Vector3d>& points,
               double min_linearity, LineFeature* line) const;
  bool fitPlane(const std::vector<Eigen::Vector3d>& points,
                Eigen::Vector3d* center, Eigen::Vector3d* normal) const;

  PlaneMatch queryPlane(const Eigen::Vector3d& world_point) const;
  PointMatch queryVisualPoint(const Eigen::Vector3d& world_point) const;
  LineMatch queryLine(const Eigen::Vector3d& world_point,
                      const Eigen::Vector3d& source_direction_world) const;

  Eigen::Matrix4d align(const FrameFeatures& features,
                        const Eigen::Matrix4d& initial_guess,
                        OplLvioResult* result) const;

  void updateMap(const FrameFeatures& features, const Eigen::Matrix4d& pose,
                 int frame_index);
  void addPoint(const Eigen::Vector3d& point, int frame_index);
  void addVisualPoint(const PointFeature& point, int frame_index);
  void addLine(const LineFeature& line, int frame_index);
  void pruneFar(const Eigen::Vector3d& center);

  OplLvioParams params_;
  int frame_count_ = 0;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  std::vector<MapPoint> map_points_;
  std::vector<VisualMapPoint> visual_points_;
  std::vector<MapLine> map_lines_;
  std::unordered_map<Eigen::Vector3i, std::vector<size_t>, VoxelHash> point_voxels_;
  std::unordered_map<Eigen::Vector3i, std::vector<size_t>, VoxelHash> visual_point_voxels_;
  std::unordered_map<Eigen::Vector3i, std::vector<size_t>, VoxelHash> line_voxels_;
};

}  // namespace opl_lvio
}  // namespace localization_zoo
