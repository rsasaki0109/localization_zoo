#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace icpsc {

struct PointI {
  Eigen::Vector3d p = Eigen::Vector3d::Zero();
  float intensity = 0.0f;
};

struct CylindricalParams {
  int width = 1024;
  int height = 64;
  double theta_down_deg = 24.8;
  double theta_up_deg = 2.0;
  double min_range = 1.0;
  double max_range = 80.0;
};

struct CylindricalImage {
  CylindricalParams params;
  std::vector<float> range;
  std::vector<float> intensity;
  std::vector<uint16_t> count;
  std::vector<Eigen::Vector3d> points;
};

/// ICPSC: 円筒強度画像上の shape-context (強度平均 + 点密度)。
struct IcpscDescriptor {
  Eigen::MatrixXd intensity_map;
  Eigen::MatrixXd density_map;
  Eigen::VectorXd ring_key;
  Eigen::VectorXd sector_key;
};

struct IcpscParams {
  CylindricalParams cylindrical;
  int num_rings = 20;
  int num_sectors = 60;
  double intensity_sigma = 0.15;
  double geom_residual_thresh = 0.2;
  double edge_curvature_thresh = 0.08;
  double intensity_grad_thresh = 0.04;
  int max_edge_features = 200;
  int max_surf_features = 400;
  int max_intensity_features = 300;
  double adaptive_alpha = 1.0;
  double geom_weight_floor = 0.2;
  double geom_weight_ceil = 0.95;
  int max_iterations = 15;
  double convergence_criterion = 0.001;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double max_correspondence_dist = 2.0;
  double initial_threshold = 2.0;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
  bool enable_intensity = true;
};

struct IcpscResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_geom_factors = 0;
  int num_intensity_factors = 0;
  double geom_weight = 0.5;
  double descriptor_distance = 0.0;
};

class IcpscCore {
 public:
  static CylindricalImage projectCylindrical(const std::vector<PointI>& points,
                                             const CylindricalParams& params);
  static IcpscDescriptor buildDescriptor(const CylindricalImage& image,
                                         int num_rings, int num_sectors);
  static double descriptorDistance(const IcpscDescriptor& a,
                                   const IcpscDescriptor& b);
  static double adaptiveGeomWeight(size_t num_geom, size_t num_intensity,
                                   double alpha, double floor_w, double ceil_w);
};

struct VoxelHash {
  std::size_t operator()(const Eigen::Vector3i& v) const {
    std::size_t seed = 0;
    auto hc = [&seed](int val) {
      seed ^= std::hash<int>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    hc(v.x()); hc(v.y()); hc(v.z());
    return seed;
  }
};

class VoxelHashMap {
 public:
  struct Correspondence {
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double mean_intensity = 0.0;
    double planarity = 0.0;
    bool found = false;
    bool has_normal = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<PointI>& points);
  std::vector<Correspondence> getCorrespondences(
      const std::vector<PointI>& queries, double max_dist,
      int normal_min_neighbors) const;
  void pruneFarVoxels(const Eigen::Vector3d& center, double max_distance);
  void clear();
  size_t size() const { return map_.size(); }

 private:
  Eigen::Vector3i toVoxel(const Eigen::Vector3d& p) const {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / voxel_size_)),
                           static_cast<int>(std::floor(p.y() / voxel_size_)),
                           static_cast<int>(std::floor(p.z() / voxel_size_)));
  }

  double voxel_size_;
  int max_points_;
  struct VoxelBlock {
    std::vector<Eigen::Vector3d> points;
    std::vector<double> intensity;
  };
  std::unordered_map<Eigen::Vector3i, VoxelBlock, VoxelHash> map_;
};

class IcpscPipeline {
 public:
  explicit IcpscPipeline(const IcpscParams& params = IcpscParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  IcpscResult registerFrame(const std::vector<PointI>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

 private:
  struct Features {
    std::vector<PointI> edges;
    std::vector<PointI> surfs;
    std::vector<PointI> intensity_pts;
  };

  Features extractFeatures(const CylindricalImage& image) const;
  std::vector<PointI> voxelDownsample(const std::vector<PointI>& points,
                                      double voxel_size) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(const Features& features,
                                    const Eigen::Matrix4d& base,
                                    IcpscResult* result);
  double meanAbsResidual(const Features& features,
                         const Eigen::Matrix4d& pose) const;

  IcpscParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  IcpscDescriptor last_descriptor_;
  bool has_last_descriptor_ = false;
  int frame_count_ = 0;
};

}  // namespace icpsc
}  // namespace localization_zoo
