#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "imu_preintegration/imu_preintegration.h"

namespace localization_zoo {
namespace pg_lio {

struct IntensityPoint {
  Eigen::Vector3d p = Eigen::Vector3d::Zero();
  double intensity = 0.0;
};

struct VoxelHash {
  std::size_t operator()(const Eigen::Vector3i& v) const {
    std::size_t seed = 0;
    auto hc = [&seed](int val) {
      seed ^= std::hash<int>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    hc(v.x());
    hc(v.y());
    hc(v.z());
    return seed;
  }
};

/// Spherical range image for PG-LIO photometric factors (PBID-style).
class RangeImage {
 public:
  RangeImage(int width, int height, double vfov_rad = 0.5235987756);

  void build(const std::vector<IntensityPoint>& points);
  void filterAndSmooth();
  void computeGradients();

  bool project(const Eigen::Vector3d& p_lidar, double& u, double& v) const;
  float sample(const std::vector<float>& img, double u, double v) const;
  float sampleFiltered(double u, double v) const;
  float gradU(double u, double v) const;
  float gradV(double u, double v) const;

  const std::vector<float>& filtered() const { return filtered_; }

 private:
  int width_;
  int height_;
  double fx_;
  double fy_;
  double cx_;
  double cy_;
  std::vector<float> intensity_;
  std::vector<float> filtered_;
  std::vector<float> grad_u_;
  std::vector<float> grad_v_;
};

/// NCC photometric patch stored in world frame.
struct PhotometricPatch {
  std::vector<Eigen::Vector3d> anchors_w;
  std::vector<float> ref_norm;
  double ref_mean = 0.0;
  double ref_sigma = 1.0;
};

class VoxelHashMap {
 public:
  struct Correspondence {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double planarity = 0.0;
    bool found = false;
    bool has_normal = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20);

  void addPoints(const std::vector<Eigen::Vector3d>& points);
  std::vector<Correspondence> getCorrespondences(
      const std::vector<Eigen::Vector3d>& points, double max_dist,
      int normal_min_neighbors) const;
  void pruneFarVoxels(const Eigen::Vector3d& center, double max_distance);

  void clear() { map_.clear(); }
  size_t size() const { return map_.size(); }

 private:
  Eigen::Vector3i toVoxel(const Eigen::Vector3d& p) const;

  double voxel_size_;
  int max_points_;
  struct VoxelBlock {
    std::vector<Eigen::Vector3d> points;
  };
  std::unordered_map<Eigen::Vector3i, VoxelBlock, VoxelHash> map_;
};

struct PGLIOParams {
  double voxel_size = 1.0;
  double min_range = 1.0;
  double max_range = 80.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 30;
  double max_correspondence_dist = 2.0;
  double convergence_criterion = 1e-4;

  int range_image_width = 512;
  int range_image_height = 64;
  int patch_radius = 2;
  int max_patches = 48;
  double min_patch_gradient = 0.02;
  double photometric_weight = 1.0;
  double degeneracy_ratio = 0.05;
  double ncc_outlier_threshold = 2.5;

  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct PGLIOResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  bool used_imu = false;
  int geometric_matches = 0;
  int photometric_patches = 0;
  int degenerate_trans_dirs = 0;
};

/// PG-LIO: photometric-geometric fusion LIO with NCC patches (simplified).
class PGLIOPipeline {
 public:
  explicit PGLIOPipeline(const PGLIOParams& params = PGLIOParams());

  void setInitialPose(const Eigen::Matrix4d& pose);

  PGLIOResult registerFrame(
      const std::vector<IntensityPoint>& frame,
      const std::vector<imu_preintegration::ImuSample>& imu);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }
  size_t patchCount() const { return patches_.size(); }

 private:
  std::vector<IntensityPoint> rangeFilter(
      const std::vector<IntensityPoint>& points) const;
  std::vector<IntensityPoint> voxelDownsample(
      const std::vector<IntensityPoint>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> toPoints(
      const std::vector<IntensityPoint>& points) const;

  void extractPatches(const RangeImage& image,
                      const std::vector<IntensityPoint>& points,
                      const Eigen::Matrix4d& pose_w);

  double patchNccError(const PhotometricPatch& patch, const RangeImage& image,
                       const Eigen::Matrix4d& pose_w,
                       Eigen::Matrix<double, 6, 1>* jacobian) const;

  int countDegenerateTransDirs(const Eigen::Matrix<double, 6, 6>& jtJ) const;

  Eigen::Matrix4d runJointOptimization(
      const std::vector<Eigen::Vector3d>& registration_points,
      const RangeImage& image, const Eigen::Matrix4d& initial_guess,
      const Eigen::Matrix4d& imu_prediction, bool used_imu,
      PGLIOResult* result);

  PGLIOParams params_;
  VoxelHashMap local_map_;
  std::vector<PhotometricPatch> patches_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  int frame_count_ = 0;
};

}  // namespace pg_lio
}  // namespace localization_zoo
