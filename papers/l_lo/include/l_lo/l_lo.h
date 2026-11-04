#pragma once

// L-LO: landmark-based LiDAR odometry (Li, Fu, Sun; arXiv:2312.16787).
// Reimplemented from the paper; no author code. The paper gives no numeric
// parameters, so every threshold below is this repository's choice.

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace l_lo {

using Polygon = std::vector<Eigen::Vector2d>;  // counter-clockwise, convex

struct LLOParams {
  // Pre-processing.
  double min_range = 2.0;
  double max_range = 60.0;
  double voxel_size = 0.2;            // non-ground downsampling
  double ground_sector_deg = 2.0;     // line-fit ground segmentation [8]
  double ground_bin_m = 1.0;
  double ground_max_slope = 0.15;
  double ground_tolerance = 0.25;     // |z - line| for ground
  double sensor_height = 1.73;        // KITTI HDL-64E mount
  double cluster_cell = 0.5;          // Euclidean clustering cell
  int min_cluster_points = 30;
  int max_cluster_points = 20000;
  int sor_k = 10;                     // statistical outlier removal
  double sor_std_mul = 1.0;
  // Registration.
  double similarity_weight_shape = 0.5;   // alpha, eq. (9)
  double similarity_weight_size = 0.5;    // beta
  double step_coeff_translation = 0.002;  // C in eq. (13), metres
  double step_coeff_rotation = 0.0002;    // C in eq. (13), radians
  double termination = 1e-4;
  int max_rounds = 300;
  int min_matches = 3;
  // Vertical pose: ground band used for the pitch estimate.
  double pitch_ground_min_range = 5.0;
  double pitch_ground_max_range = 25.0;
  // 0: change of the ground tilt between frames (historical; tracks neither
  //    road grade nor altitude, since the per-frame changes telescope).
  // 1: fit the same physical ground patch around the new position in both
  //    frames; the normal change gives the relative pitch and the plane
  //    offsets give the height change.
  // 2: pitch as in 0, height change as in 1.
  // 3: not in the paper (opt-in): height, pitch and roll from the current
  //    ground points against a world-frame ground-height map of the last
  //    ground_map_frames frames (scan-to-map, so per-frame tilt noise is not
  //    integrated); x, y and yaw are unchanged from the horizontal module.
  int pitch_mode = 0;
  double patch_half_length = 10.0;  // m, along x of frame k
  double patch_half_width = 6.0;    // m, along y of frame k
  double patch_blind_radius = 0.0;  // m, drop points this close to either sensor position
  // Raises every beam's elevation by this angle before processing. The KITTI
  // HDL-64 scans make flat ground look bowl-shaped; uncorrected, the
  // same-patch pitch picks up a systematic per-frame bias (mode 1).
  double vertical_angle_correction_deg = 0.0;
  // pitch_mode 3 only. The ground band is pitch_ground_min/max_range.
  int ground_map_frames = 30;        // window of the ground-height map
  double ground_map_cell = 0.5;      // m, height-map and point-thinning cell
  double ground_map_huber = 0.1;     // m
  int ground_map_iterations = 5;
  int ground_map_min_residuals = 50;
};

struct Cluster {
  std::vector<Eigen::Vector3d> points;
  Eigen::Vector3d center = Eigen::Vector3d::Zero();
  double mean_height = 0.0;
};

/// Graham-style convex hull (Andrew's monotone chain), counter-clockwise.
Polygon convexHull(std::vector<Eigen::Vector2d> points);
double polygonArea(const Polygon& polygon);
/// Intersection of two convex polygons (Sutherland-Hodgman).
Polygon convexIntersection(const Polygon& a, const Polygon& b);
/// Area under the turning function (normalized perimeter -> cumulative
/// exterior angle), starting at the vertex nearest the origin (paper Sec.
/// III-B-1-c).
double turningFunctionArea(const Polygon& polygon);
double hausdorffDistance(const Polygon& a, const Polygon& b);
/// Comprehensive similarity, eq. (9): alpha * |turning-area difference| +
/// beta * Hausdorff distance. Smaller is more similar.
double hullSimilarity(const Polygon& a, const Polygon& b, double alpha, double beta);

struct LLOResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  int clusters = 0;
  int matches = 0;
  int rounds = 0;
  double overlap_ratio = 0.0;
};

class LLOOdometry {
 public:
  explicit LLOOdometry(const LLOParams& params = LLOParams());
  LLOResult registerFrame(const std::vector<Eigen::Vector3d>& scan);
  const Eigen::Matrix4d& pose() const { return pose_; }

  // Exposed for tests.
  void splitGround(const std::vector<Eigen::Vector3d>& scan,
                   std::vector<Eigen::Vector3d>* ground,
                   std::vector<Eigen::Vector3d>* non_ground) const;
  std::vector<Cluster> cluster(const std::vector<Eigen::Vector3d>& non_ground) const;
  /// Maximize sum_i Area(A_i ∩ T B_i) over (dx, dy, dyaw) by the coordinate
  /// rotation method with the adaptive step of eq. (13).
  Eigen::Vector3d estimateHorizontal(const std::vector<Polygon>& previous,
                                     const std::vector<Polygon>& current,
                                     const Eigen::Vector3d& initial, int* rounds,
                                     double* overlap_ratio) const;

 private:
  LLOParams params_;
  bool initialized_ = false;
  std::vector<Cluster> previous_clusters_;
  Eigen::Vector3d previous_ground_normal_ = Eigen::Vector3d::UnitZ();
  std::vector<Eigen::Vector3d> previous_ground_;
  Eigen::Vector3d last_delta_ = Eigen::Vector3d::Zero();  // dx, dy, dyaw
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();

  // pitch_mode 3: thinned world ground points per frame and the summed
  // heights per (x, y) cell over the window.
  struct CellSum { double z = 0.0; int n = 0; };
  std::deque<std::vector<Eigen::Vector3d>> ground_map_frames_;
  std::unordered_map<long long, CellSum> ground_map_;
  long long groundCell(double x, double y) const;
  std::vector<Eigen::Vector3d> thinGround(const std::vector<Eigen::Vector3d>& ground) const;
  void insertGround(const std::vector<Eigen::Vector3d>& sensor_ground, const Eigen::Matrix4d& pose);
  Eigen::Matrix4d solveGroundMap(const std::vector<Eigen::Vector3d>& sensor_ground,
                                 const Eigen::Matrix4d& prediction) const;
};

}  // namespace l_lo
}  // namespace localization_zoo
