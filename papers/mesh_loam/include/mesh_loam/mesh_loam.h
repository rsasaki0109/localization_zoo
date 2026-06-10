#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace mesh_loam {

/// Mesh-LOAM (Zhu, Zheng & Zhu, IEEE T-IV 2024, arXiv:2312.15630) —
/// simplified CPU port. Incremental voxel-hash IMLS-SDF map updated by
/// per-point scatter ("passive voxel" scheme), block-wise incremental
/// zero-surface extraction, and point-to-mesh Gauss-Newton odometry with a
/// normal cosine-similarity correspondence gate.
struct MeshLoamParams {
  // Preprocessing.
  double min_range = 1.0;
  double max_range = 80.0;
  int normal_neighbors = 8;          ///< kNN for PCA normals (paper: k >= 5)
  double normal_search_radius = 1.0; ///< neighbor grid radius for PCA [m]
  double curvature_threshold = 0.1;  ///< c_th: planar-feature gate

  // IMLS-SDF voxel map.
  double voxel_size = 0.1;          ///< r [m]
  int influence_radius = 1;          ///< scatter radius in voxels (paper l=3 => 1)
  double imls_h = 0.05;              ///< Gaussian range parameter h [m^2]
  double lambda_n = 0.2;             ///< normal-similarity weight in hybrid weight
  double min_voxel_weight = 0.3;     ///< W(v) floor for surface extraction
  double local_map_radius = 120.0;   ///< prune voxels/mesh beyond this [m]

  // Incremental meshing.
  int block_size = 8;                ///< voxels per block edge (cubic blocks)
  int mesh_update_interval = 5;      ///< t_s: extract dirty blocks every N frames

  // Point-to-mesh registration.
  double search_radius = 0.2;        ///< facet candidate search interval [m]
  double max_correspondence_dist = 0.5;
  double cos_gate = 0.98;            ///< |cos| similarity gate c_s
  int max_iterations = 30;
  double convergence_eps = 1e-4;
  int min_correspondences = 30;
};

struct MeshLoamResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  double mean_abs_residual = 0.0;
};

class MeshLoamPipeline {
 public:
  explicit MeshLoamPipeline(const MeshLoamParams& params = MeshLoamParams());

  void reset();

  /// Register one scan given in the sensor frame; returns the world pose.
  MeshLoamResult registerFrame(const std::vector<Eigen::Vector3d>& points);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return voxels_.size(); }
  size_t meshSize() const { return facets_.size(); }

  /// Force zero-surface extraction of all dirty blocks (exposed for tests).
  void extractMesh();

 private:
  struct OrientedPoint {
    Eigen::Vector3d p = Eigen::Vector3d::Zero();
    Eigen::Vector3d n = Eigen::Vector3d::UnitZ();
    double curvature = 1.0;
    bool valid = false;
  };

  struct Voxel {
    double sdf = 0.0;
    double weight = 0.0;
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
  };

  struct Facet {
    Eigen::Vector3d v0 = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::UnitZ();
  };

  struct VoxelKeyHash {
    size_t operator()(const Eigen::Vector3i& v) const {
      return static_cast<size_t>((static_cast<int64_t>(v.x()) * 73856093) ^
                                 (static_cast<int64_t>(v.y()) * 19349663) ^
                                 (static_cast<int64_t>(v.z()) * 83492791));
    }
  };
  struct VoxelKeyEq {
    bool operator()(const Eigen::Vector3i& a, const Eigen::Vector3i& b) const {
      return a == b;
    }
  };

  Eigen::Vector3i toVoxel(const Eigen::Vector3d& p) const;
  Eigen::Vector3i toBlock(const Eigen::Vector3i& voxel) const;
  Eigen::Vector3i toFacetCell(const Eigen::Vector3d& p) const;
  Eigen::Vector3d voxelCenter(const Eigen::Vector3i& v) const;

  std::vector<OrientedPoint> preprocess(const std::vector<Eigen::Vector3d>& points) const;
  void integrateScan(const std::vector<OrientedPoint>& points_world);
  bool sampleSdf(const Eigen::Vector3i& v, double* sdf) const;
  void extractBlock(const Eigen::Vector3i& block);
  void pruneMap();
  bool solveIcp(const std::vector<OrientedPoint>& features_world,
                Eigen::Matrix4d* icp_correction, MeshLoamResult* result) const;

  MeshLoamParams params_;
  int frame_count_ = 0;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d prev_pose_ = Eigen::Matrix4d::Identity();
  bool has_prev_ = false;

  std::unordered_map<Eigen::Vector3i, Voxel, VoxelKeyHash, VoxelKeyEq> voxels_;
  std::unordered_map<Eigen::Vector3i, std::vector<size_t>, VoxelKeyHash, VoxelKeyEq>
      block_facets_;  ///< block -> indices into facets_ (rebuilt per extraction)
  std::unordered_map<Eigen::Vector3i, bool, VoxelKeyHash, VoxelKeyEq> dirty_blocks_;
  std::vector<Facet> facets_;
  std::vector<char> facet_alive_;
  std::unordered_map<Eigen::Vector3i, std::vector<size_t>, VoxelKeyHash, VoxelKeyEq>
      facet_bins_;  ///< search-interval grid -> facet indices
};

}  // namespace mesh_loam
}  // namespace localization_zoo
