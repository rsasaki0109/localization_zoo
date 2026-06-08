#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace inten_loam {

enum class FeatureLabel : uint8_t {
  kNone = 0,
  kGround = 1,
  kFacade = 2,
  kEdge = 3,
  kReflector = 4,
};

struct PointI {
  Eigen::Vector3d p = Eigen::Vector3d::Zero();
  float intensity = 0.0f;
  FeatureLabel label = FeatureLabel::kNone;
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
  std::vector<float> range;      // min range per pixel, 0=invalid
  std::vector<float> intensity;  // max intensity per pixel
  std::vector<uint8_t> label;
  std::vector<Eigen::Vector3d> points;  // one 3D point per valid pixel
};

struct InTenLoamParams {
  CylindricalParams cylindrical;
  size_t input_stride = 2;
  double ground_residual_thresh = 0.15;
  double edge_curvature_thresh = 0.12;
  double reflector_intensity_delta_thresh = 0.08;
  double reflector_intensity_abs_thresh = 0.55;
  int max_edge_features = 200;
  int max_surface_features = 400;
  int max_reflector_features = 120;
  double intensity_voxel_size = 0.5;
  double intensity_sigma = 0.12;
  double geom_distance_sq_threshold = 25.0;
  int num_optimization_iters = 2;
  int ceres_max_iterations = 6;
  double huber_loss_s = 0.1;
  double intensity_weight = 1.0;
  bool use_intensity_registration = true;
  /// Temporal voxel filter + dynamic object removal (paper §3.3–3.4).
  bool enable_tvf = true;
  bool enable_dor = true;
  double tvf_voxel_size = 1.0;
  int tvf_min_observations = 3;
  double dor_range_delta_thresh = 0.6;
  /// Scan-to-map mapping (local feature map in world frame).
  bool enable_mapping = true;
  double map_voxel_size = 1.0;
  double local_map_radius = 80.0;
  int mapping_keyframe_interval = 2;
};

struct InTenLoamResult {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  bool valid = false;
  int frame_count = 0;
  size_t num_ground = 0;
  size_t num_facade = 0;
  size_t num_edge = 0;
  size_t num_reflector = 0;
  size_t num_geom_factors = 0;
  size_t num_intensity_factors = 0;
  double mean_intensity_residual = 0.0;
  size_t num_dor_removed = 0;
  size_t num_tvf_removed = 0;
  size_t num_map_factors = 0;
  int mapping_updates = 0;
};

class InTenLoam {
 public:
  explicit InTenLoam(const InTenLoamParams& params = InTenLoamParams());

  InTenLoamResult process(const std::vector<PointI>& points);
  void clear();

  int frameCount() const { return frame_count_; }
  const Eigen::Quaterniond& orientation() const { return q_w_curr_; }
  const Eigen::Vector3d& position() const { return t_w_curr_; }

  // --- 機構ヘルパ (ユニットテスト用) ---

  static CylindricalImage projectCylindrical(const std::vector<PointI>& points,
                                             const CylindricalParams& params);
  static void computeCurvature(CylindricalImage* image);
  static void segmentGround(CylindricalImage* image, double residual_thresh);
  static void extractFeatureLabels(CylindricalImage* image,
                                   double edge_curvature_thresh,
                                   double reflector_intensity_delta_thresh,
                                   double reflector_intensity_abs_thresh);
  static std::vector<PointI> collectFeatures(const CylindricalImage& image,
                                             FeatureLabel label, int max_count);
  /// 8 近傍ボクセル強度の三線形補間 (B-spline 曲面の簡約版)。
  static double predictIntensityTrilinear(
      const std::vector<PointI>& target_reflectors, const Eigen::Vector3d& query,
      double voxel_size);
  static double intensityResidual(double predicted, double observed, double sigma);

 private:
  struct GeomMatch {
    Eigen::Vector3d curr = Eigen::Vector3d::Zero();
    Eigen::Vector3d last_j = Eigen::Vector3d::Zero();
    Eigen::Vector3d last_l = Eigen::Vector3d::Zero();
    Eigen::Vector3d last_m = Eigen::Vector3d::Zero();
    bool is_edge = false;
  };

  std::vector<GeomMatch> findGeomMatches(
      const std::vector<PointI>& curr,
      const std::vector<PointI>& last) const;
  std::vector<std::pair<size_t, size_t>> findReflectorMatches(
      const std::vector<PointI>& curr,
      const std::vector<PointI>& last) const;
  bool optimizeFrame(const std::vector<PointI>& curr_edges,
                     const std::vector<PointI>& curr_surfs,
                     const std::vector<PointI>& curr_reflectors,
                     const std::vector<PointI>& last_edges,
                     const std::vector<PointI>& last_surfs,
                     const std::vector<PointI>& last_reflectors,
                     InTenLoamResult* result);
  void applyTemporalVoxelFilter(std::vector<PointI>* points, size_t* removed);
  void applyDynamicObjectRemoval(CylindricalImage* image, size_t* removed);
  std::vector<PointI> transformPoints(const std::vector<PointI>& pts) const;
  std::vector<PointI> transformWorldToBody(const std::vector<PointI>& world_pts) const;
  void updateLocalMap(const std::vector<PointI>& edges,
                      const std::vector<PointI>& surfs,
                      const std::vector<PointI>& reflectors);
  void pruneLocalMap();

  struct VoxelKey {
    int x = 0;
    int y = 0;
    int z = 0;
    bool operator==(const VoxelKey& o) const {
      return x == o.x && y == o.y && z == o.z;
    }
  };
  struct VoxelKeyHash {
    size_t operator()(const VoxelKey& k) const {
      return static_cast<size_t>(k.x * 73856093 ^ k.y * 19349663 ^ k.z * 83492791);
    }
  };
  struct TemporalVoxel {
    int obs_count = 0;
    int last_frame = -1;
    float mean_range = 0.0f;
  };

  VoxelKey voxelKey(const Eigen::Vector3d& p, double voxel_size) const;
  std::unordered_map<VoxelKey, TemporalVoxel, VoxelKeyHash> temporal_voxels_;
  std::vector<PointI> map_edges_;
  std::vector<PointI> map_surfs_;
  std::vector<PointI> map_reflectors_;

  InTenLoamParams params_;
  std::vector<PointI> last_edges_;
  std::vector<PointI> last_surfs_;
  std::vector<PointI> last_reflectors_;
  Eigen::Quaterniond q_w_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr_ = Eigen::Vector3d::Zero();
  Eigen::Quaterniond q_last_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_last_curr_ = Eigen::Vector3d::Zero();
  bool initialized_ = false;
  int frame_count_ = 0;
};

}  // namespace inten_loam
}  // namespace localization_zoo
