#pragma once

// InTEn-LOAM as described in Li et al., "InTEn-LOAM: Intensity and Temporal
// Enhanced LiDAR Odometry and Mapping" (Remote Sensing 2023,
// arXiv:2209.05708), separate from the simplified InTenLoam pipeline:
//
// - FEF (Sec. 3.1): range image D (smallest range) and intensity image I
//   (largest intensity); segment image of the lowest z per azimuth column
//   and radial bin with per-column ground lines (Himmelsbach); angle-based
//   clustering with clusters under 15 points masked; curvature on D and
//   ground / facade / edge / reflector features (Th_E 0.3, Th_F 0.1,
//   Th_dI 80 of 255, 16 x 4 intensity blocks with the median threshold).
// - S2S (Sec. 3.4, Alg. 1): edge and plane residuals with neighbours from
//   the same and adjacent scan lines, a cubic B-spline intensity residual
//   (Sec. 3.2), Gauss-Newton on se(3), association redone with the current
//   estimate at every iteration, the previous relative motion as the guess.
// - S2M (Sec. 3.5, Alg. 2): prediction dT_{k-1} T_w'k, a local map of
//   keyframe features filtered by the temporal voxel filter (voxel mean
//   position, intensity averaged over a temporal window), 5-NN PCA lines and
//   planes, re-association every iteration.
// - DOR (Sec. 3.3): after S2M, the map and the previous scan are rendered
//   into a range image; an object (cluster) is dynamic when most of its
//   pixels disagree with it, ground never is; dynamic points are not
//   inserted into the map.
//
// Values the paper leaves open (image size, bins, thresholds of the ground
// fit, feature counts, kernel, voxel sizes, windows) are parameters below.

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstdint>
#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace inten_loam {

struct InTenLoamPaperParams {
  // Opt-in repository acceleration: independent feature stages, queries and
  // tree builds in parallel; residuals retain the original feature order.
  bool accelerated = false;
  // Motion compensation (Sec. 3.1.1, eq. 1) with the constant-velocity
  // motion and the point time read from the azimuth (KITTI scans carry no
  // per-point time): 0 off, +1 / -1 the two spin directions. Points are
  // moved to the forward-facing (azimuth 0) instant.
  int deskew = 0;
  // Range image (eq. 2).
  int width = 2048;
  int height = 64;
  double theta_down_deg = 24.9;
  double theta_up_deg = 2.0;
  double min_range = 1.0;
  double max_range = 120.0;
  // Segment image and ground lines (eqs. 3-4).
  int segment_columns = 512;        // azimuth segments
  double segment_bin = 0.5;         // radial bin [m]
  double ground_max_slope = 0.15;
  double ground_max_fit_error = 0.05;
  double ground_start_below = -1.2; // a ground line starts below this z [m]
  double ground_threshold = 0.2;    // Th_g [m]
  // Clustering (Bogoslavskyi & Stachniss).
  double cluster_angle_deg = 10.0;
  int min_cluster_points = 15;
  // Features (eq. 5, Sec. 3.1.3; thresholds from Sec. 4.1.1).
  int curvature_half_window = 5;
  // Th_E on |C|: the paper's 0.3 is for its 32-beam data (Sec. 4.1.1); on
  // KITTI 0.15 with 4 per sector was chosen on 07 (0.368 -> 0.352 %) and
  // holds on 00 (0.816 -> 0.769 %), mostly in the fast section.
  double edge_threshold = 0.15;
  double facade_threshold = 0.1;    // Th_F on |C|
  double reflector_delta = 80.0 / 255.0;  // Th_dI, intensity in [0, 1]
  int intensity_blocks_u = 16;
  int intensity_blocks_v = 4;
  int sectors_per_row = 8;
  int edges_per_sector = 4;
  int facades_per_sector = 4;
  int max_ground = 1000;
  int max_reflectors = 300;
  // Registration.
  bool use_intensity = true;
  double intensity_knot = 0.3;      // kappa [m]
  double intensity_weight = 1.0;
  double huber = 0.1;
  double s2s_gate_sq = 1.0;         // squared distance to the closest point [m^2]
  int s2s_iterations = 10;
  int s2m_iterations = 10;
  double convergence = 1e-4;
  // Mapping.
  bool use_mapping = true;
  int s2m_interval = 1;             // frames between S2M (keyframes)
  double map_edge_voxel = 0.4;
  double map_plane_voxel = 0.2;   // chosen on KITTI 07 (0.8: 0.803 %, 0.4: 0.444 %, 0.2: 0.368 %)
  double map_intensity_voxel = 0.2;
  double map_radius = 100.0;
  int temporal_window = 50;         // Th_t [frames]
  double s2m_gate_sq = 1.0;         // squared distance of the 5th neighbour
  // S2M plane fit: all 5 neighbours within this distance of the PCA
  // plane. The 0.2 m default admits curved and mixed neighbourhoods
  // (vegetation, kerbs, corners) whose normals pull yaw and pitch; 0.05 m
  // was chosen on KITTI 07 (see README, "Remaining KITTI gap").
  double s2m_plane_max_dist = 0.2;
  // ... and spread in two directions: middle / largest PCA eigenvalue at
  // least this (0: off). Five neighbours along one line (a scan line on a
  // wall, the end of a wall) pass any distance check while their normal
  // can turn freely about the line.
  double s2m_plane_min_spread = 0.0;
  // DOR.
  bool use_dor = true;
  double dor_relative_range = 0.1;  // s_d
  double dor_object_ratio = 0.5;    // Th_N
  // Opt-in repository extension, not in the paper: when the ten-neighbour
  // S2S plane query lacks same/adjacent-row support, search the required rows
  // under the existing distance gate. Existing selected points are retained.
  bool s2s_row_support = false;
  // Opt-in repository extension, not in the paper: seed the next S2S solve
  // with the preceding final mapped relative pose (normalized quaternion
  // rotation to prevent recursive non-rigid roundoff). Fallback and deskew still
  // use last accepted S2S motion; mapping-off/forced diagnostics are unchanged.
  bool s2s_mapped_prior = false;
};

// One feature: sensor-frame point, intensity in [0, 1], scan line (row).
struct PaperFeature {
  Eigen::Vector3d p = Eigen::Vector3d::Zero();
  float intensity = 0.0f;
  int row = 0;
  int cluster = 0;  // 1: ground, >= 2: object, 0: masked
};

struct PaperFrame {
  std::vector<PaperFeature> ground, facade, edge, reflector;
  std::vector<PaperFeature> all;  // every valid pixel (intensity target, DOR)
};

struct InTenLoamPaperResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();  // world <- sensor
  size_t num_ground = 0, num_facade = 0, num_edge = 0, num_reflector = 0;
  int s2s_pairs = 0;
  int s2m_pairs = 0;
  int s2m_class_pairs[4] = {0, 0, 0, 0};  // edge, ground, facade, intensity (last iteration)
  size_t dor_removed = 0;
};

// Diagnostics (analysis tools, not part of the method). With `forced_pose`
// set, S2S is solved from the relative motion to that pose and S2M from the
// pose itself; both results and the normal equations of each feature class
// at the forced pose (first iteration) are returned, and the frame is then
// placed at `forced_pose` (map insertion, next prediction) instead of the
// estimate. Fed with ground truth this gives per-frame bias against a map
// built from ground-truth poses. Without `forced_pose` nothing is filled.
struct InTenLoamPaperDiagnostics {
  using Mat6 = Eigen::Matrix<double, 6, 6>;
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  const Eigen::Matrix4d* forced_pose = nullptr;  // world <- sensor
  // Classes: 0 edge, 1 ground, 2 facade, 3 intensity. Jacobians are with
  // respect to a left perturbation [t, w] of the S2S relative pose (previous
  // frame) and of the S2M pose (world); the Gauss-Newton step is -H^-1 g
  // with H, g summed over the classes.
  Mat6 s2s_H[4], s2m_H[4];
  Vec6 s2s_g[4], s2m_g[4];
  int s2s_n[4] = {0, 0, 0, 0}, s2m_n[4] = {0, 0, 0, 0};
  Eigen::Matrix4d s2s_estimate = Eigen::Matrix4d::Identity();  // relative, from the forced relative
  Eigen::Matrix4d s2m_estimate = Eigen::Matrix4d::Identity();  // world <- sensor, from forced_pose
};

// Optional observation of the normal estimator, separate from forced-pose
// interventions above. No solver is rerun and no pose is supplied by this API.
enum class PaperSolverStop {
  NotRun, NoIterations, InsufficientPairs, NonFiniteStep, Converged, IterationLimit
};

struct PaperIterationTrace {
  using Mat6 = Eigen::Matrix<double, 6, 6>;
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  Eigen::Matrix4d linearization_pose = Eigen::Matrix4d::Identity();
  Mat6 H = Mat6::Zero();  // unregularized, left perturbation [t, w]
  Vec6 g = Vec6::Zero();
  Vec6 delta = Vec6::Zero();  // solve uses H + 1e-6 I, as without tracing
  int class_pairs[4] = {0, 0, 0, 0};  // edge, ground, facade, intensity
  int pairs = 0;
  // Sum w*r*r at this linearization; w is the existing Huber IRLS weight.
  // This is not the Huber objective, a post-update score or a quality gate.
  double weighted_squared_residual = 0.0;
  bool step_attempted = false;
  bool step_finite = false;
  bool update_applied = false;
};

struct PaperSolverTrace {
  bool called = false;
  Eigen::Matrix4d input = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d output = Eigen::Matrix4d::Identity();
  int returned_pairs = 0;  // includes the existing nonfinite-step return of zero
  PaperSolverStop stop = PaperSolverStop::NotRun;
  std::vector<PaperIterationTrace> iterations;
};

struct InTenLoamPaperTrace {
  int frame_index = 0;
  bool initialization = false;
  PaperSolverTrace s2s, s2m;
  bool s2s_fallback = false;
  Eigen::Matrix4d accepted_relative = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d s2m_prediction = Eigen::Matrix4d::Identity();  // drift * odom
  bool s2m_accepted = false;
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();  // final world <- sensor
};

class InTenLoamPaper {
public:
  explicit InTenLoamPaper(const InTenLoamPaperParams& params = InTenLoamPaperParams());

  // points: sensor frame, intensity in [0, 1]. trace is reset on each call.
  // Tracing is off by default and cannot be combined with forced_pose:
  // that combination throws std::invalid_argument before estimator updates.
  InTenLoamPaperResult process(const std::vector<Eigen::Vector3d>& points,
                               const std::vector<float>& intensity,
                               InTenLoamPaperDiagnostics* diag = nullptr,
                               InTenLoamPaperTrace* trace = nullptr);

  // Exposed for tests.
  PaperFrame extractFeatures(const std::vector<Eigen::Vector3d>& points,
                             const std::vector<float>& intensity) const;
  // Cubic B-spline intensity field (eq. 6) on a lattice of spacing `knot`;
  // each control point is the mean intensity of the points nearest to it,
  // empty ones take the mean of the 4 x 4 x 4 support.
  class IntensityGrid {
  public:
    void build(const std::vector<PaperFeature>& cloud, double knot);
    // False when fewer than 4 control points of the support are observed.
    bool eval(const Eigen::Vector3d& x, double* value, Eigen::Vector3d* gradient) const;

  private:
    struct Key {
      int x, y, z;
      bool operator==(const Key& o) const { return x == o.x && y == o.y && z == o.z; }
    };
    struct KeyHash {
      size_t operator()(const Key& k) const {
        return static_cast<size_t>((static_cast<int64_t>(k.x) * 73856093) ^
                                   (static_cast<int64_t>(k.y) * 19349663) ^
                                   (static_cast<int64_t>(k.z) * 83492791));
      }
    };
    double knot_ = 0.3;
    std::unordered_map<Key, std::pair<double, int>, KeyHash> nodes_;
  };

private:
  struct Map;
  struct Index;
  struct ClassSystems;

  // Gauss-Newton on se(3) with re-association; T maps source into target.
  int solveS2S(const PaperFrame& curr, Eigen::Matrix4d* T, ClassSystems* first = nullptr,
               PaperSolverTrace* trace = nullptr) const;
  int solveS2M(const PaperFrame& curr, Eigen::Matrix4d* T, int class_pairs[4] = nullptr,
               ClassSystems* first = nullptr, PaperSolverTrace* trace = nullptr) const;
  std::vector<char> dynamicMask(const PaperFrame& curr, const Eigen::Matrix4d& T_w) const;
  void insert(const PaperFrame& curr, const Eigen::Matrix4d& T_w, const std::vector<char>& dynamic);

  InTenLoamPaperParams params_;
  std::shared_ptr<Index> prev_;   // previous frame and its kd-trees
  std::shared_ptr<Map> map_;
  Eigen::Matrix4d odom_ = Eigen::Matrix4d::Identity();    // T_w'k (S2S chain)
  Eigen::Matrix4d drift_ = Eigen::Matrix4d::Identity();   // dT
  Eigen::Matrix4d last_rel_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_mapped_rel_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d prev_pose_ = Eigen::Matrix4d::Identity();
  int frame_ = 0;
};

}  // namespace inten_loam
}  // namespace localization_zoo
