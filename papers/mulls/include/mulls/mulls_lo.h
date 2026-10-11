#pragma once

// MULLS-LO: LiDAR odometry following Pan et al., "MULLS: Versatile LiDAR SLAM
// via Multi-metric Linear Least Square" (ICRA 2021), with the parameters of
// the authors' KITTI urban profile. Written from the paper and the profile's
// documented behaviour; it shares no code with the reference implementation.
//
// Per scan: a dual-threshold grid ground filter with per-cell plane fits,
// PCA classification of the remaining points into pillar / beam / facade,
// saliency-based thinning into a sparse source set, and multi-metric ICP
// (point-to-plane for ground and facade, point-to-line for pillars) solved
// as weighted linear least squares, scan-to-scan for the first frames and
// scan-to-map afterwards against a local map kept in the last frame.

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <array>
#include <deque>
#include <random>
#include <vector>

namespace localization_zoo {
namespace mulls {

enum LOClass { kLOGround = 0, kLOPillar = 1, kLOFacade = 2, kLOBeam = 3, kLONumClasses = 4 };

struct LOPoint {
  Eigen::Vector3d p = Eigen::Vector3d::Zero();
  // Plane normal (ground, facade) or line direction (pillar, beam).
  Eigen::Vector3d dir = Eigen::Vector3d::UnitZ();
  double saliency = 0.0;
  double intensity = 0.0;  // 0-255
};
using LOCloud = std::vector<LOPoint>;

struct MullsLOParams {
  // Input (scanner frame).
  double min_horizontal_range = 1.5;
  double max_horizontal_range = 120.0;
  double vertical_angle_correction_deg = 0.195;
  double scanner_height = 1.8;
  double scanner_filter_radius = 1.75;  // drop points this close horizontally
  double underground_depth = 6.0;       // drop z <= -(height + depth)
  double near_underground_depth = 4.0;  // within near_radius: z <= -(height + this)
  double near_radius = 20.0;

  // Ground filter.
  double gf_grid = 2.5;
  double gf_in_cell_height = 0.25;
  double gf_neighbor_height = 1.5;
  double gf_high_above_mean = 2.0;
  int gf_min_points = 6;
  double gf_ground_rate = 12.0;
  double gf_nonground_rate = 3.0;
  double gf_unit_distance = 10.0;
  double gf_bright_intensity = 150.0;
  double gf_plane_threshold = 0.075;
  int gf_plane_iterations = 20;
  double gf_min_normal_z = 0.8;
  int sparse_ground = 800;

  // Classification.
  int nonground_cap = 20000;
  int pca_stride = 2;
  double pca_radius = 0.7;
  double pca_radius_range = 30.0;  // radius grows as sqrt(range / this) beyond it
  int pca_max_neighbors = 25;
  int pca_min_neighbors = 8;
  double linearity = 0.62;
  double planarity = 0.62;
  double curvature = 0.08;
  double pillar_min_deg = 70.0;  // |e1_z| > sin(70)
  double beam_max_deg = 10.0;    // |e1_z| < sin(10)
  double beam_max_height = 0.5;
  double facade_max_deg = 20.0;  // |n_z| < sin(20)
  double vertex_labelled_share = 0.15;
  double nms_radius = 0.175;
  int nms_min_points = 10;
  int sparse_pillar = 400;
  int sparse_facade_per_sector = 300;
  int sparse_beam_per_sector = 50;
  bool use_beam = false;  // beams enter registration and the map (highway profile)

  // Multi-metric ICP.
  double gate_initial = 1.4;
  double gate_startup_bonus = 1.0;  // added while frame index <= 3
  double gate_min = 0.5;
  double gate_decay = 1.1;
  double search_gate_factor = 2.5;
  int prune_min_points = 500;
  double normal_max_deg = 20.0;
  int max_iterations = 20;
  double converge_translation = 5e-4;
  double converge_rotation_deg = 1e-3;
  double huber_delta = 0.05;
  int huber_start_iteration = 3;
  double distance_weight_range = 30.0;
  int min_pairs = 40;
  int min_pillar_facade_pairs = 20;
  double min_pillar_facade_ratio = 0.03;
  double crop_margin = 1.0;

  // Local map.
  double map_radius = 90.0;
  int map_max_points = 20000;
  int pillar_refresh_period = 10;
  double pillar_refresh_radius = 1.8;
  int pillar_refresh_max_neighbors = 20;
  double pillar_refresh_linearity = 0.65;
  double pillar_refresh_min_z = 0.8;
  double dynamic_radius = 15.0;
  double dynamic_min_dist = 0.45;
  double dynamic_same_dist = 0.01;
  int dynamic_min_map_points = 4000;
  double frame_rate = 10.0;

  // Mechanism switches for ablation; all on in the profile.
  bool use_vertex_promotion = true;
  bool use_source_pruning = true;
  bool use_ground_balance = true;
  bool use_distance_weight = true;
  bool use_huber = true;
  bool use_intensity_weight = true;
  bool use_dynamic_removal = true;
  bool use_pillar_refresh = true;
  bool use_scan_to_map = true;  // false: scan-to-scan against the previous frame throughout

  unsigned seed = 42;

  // The authors' KITTI highway profile (sparse scenes such as sequence 01):
  // beams join the registration, a looser normal check, no ground-balance
  // weight and no dynamic-object removal.
  static MullsLOParams kittiHighway();
};

struct LOFeatures {
  std::array<LOCloud, kLONumClasses> dense;
  std::array<LOCloud, kLONumClasses> sparse;
  Eigen::AlignedBox3d bound;  // raw scan extent
};

struct LOICPResult {
  Eigen::Isometry3d T = Eigen::Isometry3d::Identity();  // source -> target
  int iterations = 0;
  bool converged = false;
  bool aborted = false;  // too few pairs or an implausible step
  std::array<int, kLONumClasses> pairs{};
};

// Steps of the pipeline, exposed for tests.
LOCloud preprocessScan(const std::vector<Eigen::Vector4d>& xyzi, const MullsLOParams& params);
// Splits into dense ground (with plane normals) and non-ground points.
void groundFilter(const LOCloud& cloud, const MullsLOParams& params, LOCloud* ground,
                  LOCloud* nonground);
// Labels non-ground points (dense pillar / beam / facade).
void classifyNonGround(const LOCloud& nonground, const MullsLOParams& params, std::mt19937* rng,
                       std::array<LOCloud, kLONumClasses>* dense);
LOCloud saliencyNms(const LOCloud& cloud, double radius, int min_points);
LOFeatures extractFeatures(const std::vector<Eigen::Vector4d>& xyzi, const MullsLOParams& params,
                           std::mt19937* rng);

// Distance weight b + (1 - b) |q| / range with b = min(0.7 + 0.05 k, 1.3).
double distanceWeight(int iteration, double target_norm, double range);
double huberWeight(double residual, double delta);
double groundBalanceWeight(const std::array<int, kLONumClasses>& pairs);

LOICPResult multiMetricICP(const std::array<LOCloud, kLONumClasses>& source,
                           const std::array<LOCloud, kLONumClasses>& target,
                           const Eigen::AlignedBox3d& target_bound,
                           const Eigen::Isometry3d& initial, double gate,
                           const MullsLOParams& params);

struct MullsLOFrameResult {
  Eigen::Isometry3d pose = Eigen::Isometry3d::Identity();  // first scan <- this scan
  LOICPResult icp;
  bool scan_to_map = false;
  std::array<int, kLONumClasses> feature_counts{};
  size_t map_points = 0;
};

class MullsLO {
public:
  explicit MullsLO(const MullsLOParams& params = MullsLOParams());

  // xyzi: scanner-frame points, intensity 0-255.
  MullsLOFrameResult process(const std::vector<Eigen::Vector4d>& xyzi);

  const std::array<LOCloud, kLONumClasses>& map() const { return map_; }

private:
  void updateMap();

  MullsLOParams params_;
  std::mt19937 rng_;
  int frame_index_ = 0;
  std::vector<Eigen::Isometry3d> poses_;
  LOFeatures previous_;
  Eigen::Isometry3d last_relative_ = Eigen::Isometry3d::Identity();  // prev <- current
  Eigen::Isometry3d guess_ = Eigen::Isometry3d::Identity();
  std::deque<double> recent_motion_;  // per-frame translation [m], last 20
  std::array<LOCloud, kLONumClasses> map_;  // in the previous frame
  Eigen::AlignedBox3d map_bound_;
};

}  // namespace mulls
}  // namespace localization_zoo
