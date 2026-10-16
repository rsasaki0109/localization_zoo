#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace mesh_loam {

/// Mesh-LOAM (Zhu, Zheng & Zhu, IEEE T-IV 2024, arXiv:2312.15630) —
/// simplified CPU port. Incremental voxel-hash IMLS-SDF map updated by
/// per-point scatter ("passive voxel" scheme), incremental zero-surface
/// extraction of the cells touched since the last extraction, and
/// point-to-mesh Gauss-Newton odometry with a normal cosine-similarity
/// correspondence gate.
struct MeshLoamParams {
  // Opt-in repository acceleration: conservative facet-bin normal bounds. Does not
  // alter the association gates or the scan/map parameters from the paper.
  bool accelerated = false;
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
  /// true: the radius prune drops voxels only; extracted facets stay as the
  /// global mesh (Eq. 3). Cells far from the sensor are never re-extracted
  /// (their voxels are gone), so their facets stay until the area is
  /// observed again.
  bool keep_mesh = false;
  /// keep_mesh only: facet bins farther than this (plus a 11 m hysteresis)
  /// from the current pose are moved to a compressed store (16-bit
  /// vertices, 22 B instead of ~70 B per facet) and restored when the pose
  /// comes back within the radius. <= 0 keeps every bin in the searchable
  /// store. Must exceed max_range plus the distance travelled in
  /// mesh_update_interval frames (frozen bins are not searched); 100 m for
  /// the 80 m KITTI setup.
  double mesh_freeze_radius = 0.0;

  // Incremental meshing.
  int block_size = 8;                ///< voxels per block edge (cubic blocks)
  int mesh_update_interval = 5;      ///< t_s: extract dirty cells every N frames
  /// true: re-extract only the cells with a corner voxel updated since the
  /// last extraction (cost scales with the scan); false: every cell of each
  /// block that holds an updated voxel (the earlier behaviour).
  bool mesh_dirty_cells = true;
  /// true: marching cubes (standard Lorensen-Cline / Bourke table, as in the
  /// paper); false: marching tetrahedra (6 tetrahedra per cell, more and
  /// smaller triangles).
  bool marching_cubes = false;

  // Point-to-mesh registration.
  double search_radius = 0.2;        ///< facet candidate search interval [m]
  double max_correspondence_dist = 0.5;
  double cos_gate = 0.98;            ///< |cos| similarity gate c_s
  /// false: among the gated facets of the 27 search cells take the smallest
  /// plane residual; true: take the facet nearest to the point (true
  /// point-to-triangle distance, within search_radius), as in the paper.
  bool nearest_facet = false;
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

/// Cumulative wall time per stage since construction / reset() [ms].
struct MeshLoamTimings {
  double preprocess_ms = 0.0;  ///< range filter + kNN PCA normals
  double icp_ms = 0.0;         ///< point-to-mesh Gauss-Newton
  double integrate_ms = 0.0;   ///< SDF scatter
  double extract_ms = 0.0;     ///< zero-surface extraction
  double prune_ms = 0.0;       ///< radius prune (+ mesh freeze/thaw)
  int frames = 0;
  int extractions = 0;
  long extracted_cells = 0;    ///< cells re-meshed over all extractions
};

class MeshLoamPipeline {
 public:
  explicit MeshLoamPipeline(const MeshLoamParams& params = MeshLoamParams());

  void reset();

  /// Register one scan given in the sensor frame; returns the world pose.
  MeshLoamResult registerFrame(const std::vector<Eigen::Vector3d>& points);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return num_voxels_; }  ///< observed voxels
  size_t meshSize() const { return num_facets_ + num_frozen_facets_; }  ///< all facets
  size_t liveFacets() const { return meshSize(); }
  size_t frozenFacets() const { return num_frozen_facets_; }
  /// Approximate heap bytes held by the voxel map and the mesh.
  size_t memoryBytes() const;
  const MeshLoamTimings& timings() const { return timings_; }

  /// Visit every facet (searchable and frozen) as world-frame vertices with
  /// the normal oriented toward positive SDF (exposed for tests / export).
  template <typename F>
  void forEachFacet(F&& f) const;

  /// Force zero-surface extraction of all dirty cells (exposed for tests).
  void extractMesh();
  /// Mark every observed voxel dirty and re-extract the whole mesh of the
  /// voxel map (exposed for tests; facets outside the voxel map are kept).
  void remeshAll();

 private:
  friend struct MeshLoamTestAccess;
  struct OrientedPoint {
    Eigen::Vector3d p = Eigen::Vector3d::Zero();
    Eigen::Vector3d n = Eigen::Vector3d::UnitZ();
    double curvature = 1.0;
    bool valid = false;
  };

  struct Voxel {
    float sdf = 0.0f;
    float weight = 0.0f;
    float n[3] = {0.0f, 0.0f, 0.0f};
  };

  struct VoxelBlock {
    std::vector<Voxel> voxels;      ///< block_size^3, x fastest
    std::vector<uint64_t> dirty;    ///< one bit per voxel, updated since last extraction
    int observed = 0;               ///< voxels with weight > 0
    int base[3] = {0, 0, 0};        ///< voxel index of the block's first voxel
    uint32_t tag = 0;               ///< hash used to assign blocks to threads
    bool queued = false;            ///< in dirty_blocks_
  };

  /// Triangle stored inside its 0.2 m search bin: vertices relative to the
  /// bin origin (float), unit normal toward positive SDF, and the owning
  /// marching cell as an offset from the bin's base voxel.
  struct Facet {
    Eigen::Vector3f a, b, c;
    Eigen::Vector3f n;
    int8_t owner[3];
  };
  /// Facets of one search bin plus exact bounds for pruning the
  /// nearest-facet search: the AABB of their vertices and the largest
  /// centroid-to-vertex distance (bin-relative).
  struct FacetBin {
    std::vector<Facet> facets;
    Eigen::Vector3f lo = Eigen::Vector3f::Constant(1e30f);
    Eigen::Vector3f hi = Eigen::Vector3f::Constant(-1e30f);
    float max_rad = 0.0f;
    Eigen::Vector3f normal_ref = Eigen::Vector3f::Zero();
    float normal_radius = std::numeric_limits<float>::infinity();
    void add(const Facet& f);
    void expand(const Facet& f);  ///< grow the bounds by f (already stored)
    void recomputeBounds();
    void buildNormalBounds();
    double normalUpper(const Eigen::Vector3d& normal) const;
  };

  /// Compressed facet: 16-bit vertices relative to the bin origin; the
  /// normal is recomputed from the winding.
  struct FrozenFacet {
    int16_t v[9];
    int8_t owner[3];
  };
  struct FrozenChunk {
    std::vector<Eigen::Vector3i> bins;       ///< bin keys
    std::vector<uint32_t> offsets;           ///< bins.size()+1 offsets into facets
    std::vector<FrozenFacet> facets;
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
  template <typename V>
  using KeyMap = std::unordered_map<Eigen::Vector3i, V, VoxelKeyHash, VoxelKeyEq>;

  Eigen::Vector3i toVoxel(const Eigen::Vector3d& p) const;
  Eigen::Vector3i toBlock(const Eigen::Vector3i& voxel) const;
  Eigen::Vector3i toFacetCell(const Eigen::Vector3d& p) const;
  Eigen::Vector3d voxelCenter(const Eigen::Vector3i& v) const;
  Eigen::Vector3d binOrigin(const Eigen::Vector3i& bin) const;
  Eigen::Vector3i binBaseVoxel(const Eigen::Vector3i& bin) const;
  Eigen::Vector3i toFreezeChunk(const Eigen::Vector3i& bin) const;

  std::vector<OrientedPoint> preprocess(const std::vector<Eigen::Vector3d>& points) const;
  void integrateScan(const std::vector<OrientedPoint>& points_world);
  void pruneMap();
  void freezeAndThaw();
  void thawChunk(const Eigen::Vector3i& chunk);
  double frozenQuantum() const {
    return (params_.search_radius + 3.0 * params_.voxel_size) / 32000.0;
  }
  bool solveIcp(const std::vector<OrientedPoint>& features_world,
                Eigen::Matrix4d* icp_correction, MeshLoamResult* result) const;

  MeshLoamParams params_;
  MeshLoamTimings timings_;
  int frame_count_ = 0;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d prev_pose_ = Eigen::Matrix4d::Identity();
  bool has_prev_ = false;

  KeyMap<VoxelBlock> blocks_;
  std::vector<Eigen::Vector3i> dirty_blocks_;  ///< blocks with dirty voxels, first-touch order
  size_t num_voxels_ = 0;

  KeyMap<FacetBin> facet_bins_;                ///< search bin -> facets
  size_t num_facets_ = 0;
  KeyMap<FrozenChunk> frozen_;                 ///< keep_mesh far store
  size_t num_frozen_facets_ = 0;
};

template <typename F>
void MeshLoamPipeline::forEachFacet(F&& f) const {
  for (const auto& entry : facet_bins_) {
    const Eigen::Vector3d o = binOrigin(entry.first);
    for (const Facet& t : entry.second.facets) {
      f(Eigen::Vector3d(o + t.a.cast<double>()), Eigen::Vector3d(o + t.b.cast<double>()),
        Eigen::Vector3d(o + t.c.cast<double>()), Eigen::Vector3d(t.n.cast<double>()));
    }
  }
  const double q = frozenQuantum();
  for (const auto& chunk : frozen_) {
    const FrozenChunk& c = chunk.second;
    for (size_t i = 0; i < c.bins.size(); ++i) {
      const Eigen::Vector3d o = binOrigin(c.bins[i]);
      for (uint32_t j = c.offsets[i]; j < c.offsets[i + 1]; ++j) {
        const FrozenFacet& t = c.facets[j];
        const Eigen::Vector3d a = o + q * Eigen::Vector3d(t.v[0], t.v[1], t.v[2]);
        const Eigen::Vector3d b = o + q * Eigen::Vector3d(t.v[3], t.v[4], t.v[5]);
        const Eigen::Vector3d cc = o + q * Eigen::Vector3d(t.v[6], t.v[7], t.v[8]);
        Eigen::Vector3d n = (b - a).cross(cc - a);
        const double len = n.norm();
        if (len > 0.0) n /= len;
        f(a, b, cc, n);
      }
    }
  }
}

}  // namespace mesh_loam
}  // namespace localization_zoo
