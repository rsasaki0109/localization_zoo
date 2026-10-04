#pragma once

// L-LO: landmark-based LiDAR odometry (Li, Fu, Sun; arXiv:2312.16787).
// Reimplemented from the paper; no author code. The paper gives no numeric
// parameters, so every threshold below is this repository's choice.

#include <Eigen/Core>
#include <Eigen/Geometry>

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
  Eigen::Vector3d last_delta_ = Eigen::Vector3d::Zero();  // dx, dy, dyaw
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
};

}  // namespace l_lo
}  // namespace localization_zoo
