#pragma once

// DiLO as described in Han et al., "DiLO: Direct light detection and ranging
// odometry based on spherical range images for autonomous driving" (ETRI
// Journal 2021), separate from the frame-to-keyframe DiloPipeline:
//
// - spherical range image (Sec. 3.2): 2048 columns, rows linear in
//   sin(elevation) at the same scale, nearest range per pixel, vertical gaps
//   filled by interpolation when the range changes slowly and with the far
//   range otherwise; points are back-projected from the pixel centres;
// - normals from the range gradient (eq. 11) with the Farid-Simoncelli 5-tap
//   filters, planarity a = 1 - |dn/dq|^2 (Sec. 3.3, 3.6);
// - plane key points: the top K of the IMLS-style lists a (x x n)_k (both
//   signs), a n_x, a n_y (both signs) and a n_z for the ground (Sec. 3.6);
// - residual (eq. 18): (n_r . R n_s) n_r . (x_r - T x_s), projective
//   association into the reference image;
// - Algorithm 1: a queue of the last N frames. A new frame is registered to
//   all of them (Huber); then every pair (i, j) in the window takes a Tukey
//   IRLS step and the window poses are re-estimated by averaging the
//   relative poses in the log map, with the oldest frame fixed.
//
// Edge key points (Sec. 3.5) are opt-in (use_edges); the paper's plane-only
// ablation (DiLO-plane, Table 1) is the matching reference.

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <deque>
#include <memory>
#include <vector>

namespace localization_zoo {
namespace dilo {

struct DiloPaperParams {
  int width = 2048;
  int height = 192;
  double top_elevation_deg = 3.0;   // elevation of the first row
  double min_range = 1.0;
  double max_range = 120.0;
  double fill_slope = 0.02;         // relative range change per row allowed for interpolation
  int max_fill_gap = 4;             // rows
  int keypoints_per_list = 100;
  // Key-point candidates need planarity a > min_planarity. a = 1 - |dn/dq|^2
  // is above 0.9 almost everywhere (vegetation, car bodies), so 0 lets the
  // lists fill up with such points; 0.98 keeps them out.
  double min_planarity = 0.0;
  // > 1: at most one key point of a list per keypoint_cell_px x
  // keypoint_cell_px block of the image (not in the paper).
  int keypoint_cell_px = 0;
  int window = 10;                  // N
  int init_iterations = 5;
  int update_iterations = 5;        // 0: no window refinement
  // true: measured pixels keep the exact scan point; false: every pixel is
  // back-projected from its centre ray (the paper stores ranges only).
  bool exact_points = true;
  // true: normals from the Farid gradients of the 3D point image; false:
  // eq. 11 (range gradient on pixel-centre rays).
  bool point_gradient_normals = true;
  bool planar_prediction = true;
  // Guards against failed registrations (not in the paper).
  double max_init_deviation_m = 1.0;
  double max_init_deviation_rad = 0.1;
  double max_pair_step_m = 0.5;
  double max_pair_step_rad = 0.05;
  // Edge key points (Sec. 3.5): Harris on dI_e/du, negative local minima;
  // the residual is the pixel distance along u to the nearest reference edge
  // in the same row (eq. 17); edge and plane steps are averaged.
  bool use_edges = false;
  int max_edge_points = 1000;
  double edge_search_px = 4.0;
  double harris_k = 0.04;
  double huber_delta = 0.25;
  double tukey_c = 1.0;
  // Second-pass edge options; the defaults reproduce the first edge
  // implementation above.
  // 0: Harris response of dI_e/du, negative local minima over 3x3;
  // 1: the same response, minima along u only (a vertical edge is kept on
  //    every row it crosses instead of being thinned to isolated points);
  // 2: vertical ridges of I_e: local maxima of I_e along u whose curvature
  //    along u exceeds the curvature along v;
  // 3: Harris on I_e itself (structure tensor of dI_e/du, dI_e/dv), negative
  //    response with the gradient mostly along u, minima along u.
  int edge_detector = 0;
  // Size of the reference edge set (the strongest edges of the image); 0:
  // max_edge_points. The source key points are the strongest max_edge_points.
  int edge_reference_points = 0;
  // Sub-pixel u of every edge (parabola through the detector score along u);
  // the source point is rotated about z to that azimuth.
  bool edge_subpixel = false;
  // The nearest reference edge is searched on rows within +-k of the
  // projected row.
  int edge_row_search = 0;
  // true: one Gauss-Newton system H = H_x + edge_weight H_e (pixels^2 against
  // metres^2); false: the paper's average of the plane and edge steps.
  bool edge_sum_normal_equations = false;
  double edge_weight = 1.0;
  // Directions the edge term acts on: 6 all; 3 yaw and horizontal
  // translation; 1 yaw only (the u residual carries little roll / pitch / z
  // information, and an edge-only step is free to wander there).
  int edge_dofs = 6;
  // Kernel thresholds of the edge residual in pixels; 0: huber_delta / tukey_c.
  double edge_huber_delta = 0.0;
  double edge_tukey_c = 0.0;
};

struct DiloPaperFrame {
  // Range image and per-pixel back-projected point, normal and planarity.
  std::vector<float> range;
  std::vector<Eigen::Vector3f> point;
  std::vector<Eigen::Vector3f> normal;
  std::vector<float> planarity;
  std::vector<unsigned char> has_normal;
  // Plane key points (sensor frame) with their normals.
  std::vector<Eigen::Vector3d> key_points;
  std::vector<Eigen::Vector3d> key_normals;
  // Edge key points (sensor frame) and, per row, the sorted u coordinates of
  // the reference edges (pixel centre c + 0.5, or the sub-pixel position).
  std::vector<Eigen::Vector3d> edge_points;
  std::vector<std::vector<double>> edge_cols;
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();  // world <- sensor
};

class DiloPaperOdometry {
public:
  explicit DiloPaperOdometry(const DiloPaperParams& params = DiloPaperParams());

  // Returns the causal pose of the new frame.
  Eigen::Matrix4d process(const std::vector<Eigen::Vector3d>& scan);
  // Poses of the frames that have left the window (smoothed), in order.
  const std::vector<Eigen::Matrix4d>& finalizedPoses() const { return finalized_; }
  // Poses still in the window, oldest first.
  std::vector<Eigen::Matrix4d> windowPoses() const;

  // Exposed for tests.
  std::shared_ptr<DiloPaperFrame> buildFrame(const std::vector<Eigen::Vector3d>& scan) const;
  bool project(const Eigen::Vector3d& p, int* row, int* col) const;
  // One weighted Gauss-Newton system of `source` (key points) against the
  // image of `reference`, for the relative pose T (reference <- source).
  // kernel 0: Huber(delta), 1: Tukey(c). Returns the number of pairs.
  int accumulate(const DiloPaperFrame& source, const DiloPaperFrame& reference,
                 const Eigen::Matrix4d& T, int kernel, Eigen::Matrix<double, 6, 6>* H,
                 Eigen::Matrix<double, 6, 1>* g) const;
  // Edge residual (eq. 17) of one source point p against the reference edges
  // for T (reference <- source): e = u_edge - u(T p) in pixels, with the
  // Jacobian for the increment T exp(xi), xi = [omega, v]. False if no
  // reference edge lies within edge_search_px on the searched rows.
  bool edgeResidual(const Eigen::Vector3d& p, const DiloPaperFrame& reference, const Eigen::Matrix4d& T,
                    double* e, Eigen::Matrix<double, 6, 1>* J) const;
  // u coordinate (pixels) of a point.
  double imageU(const Eigen::Vector3d& p) const;
  // Same for the edge residual (pixels along u).
  int accumulateEdges(const DiloPaperFrame& source, const DiloPaperFrame& reference,
                      const Eigen::Matrix4d& T, int kernel, Eigen::Matrix<double, 6, 6>* H,
                      Eigen::Matrix<double, 6, 1>* g) const;

private:
  // Step for one source / reference set: plane step, averaged with the edge
  // step when edges are on. False if neither has enough pairs.
  bool step(const DiloPaperFrame& source, const std::vector<const DiloPaperFrame*>& refs,
            const std::vector<Eigen::Matrix4d>& T, int kernel, Eigen::Matrix<double, 6, 1>* d) const;
  int index(int row, int col) const { return row * params_.width + col; }
  Eigen::Vector3d pixelDirection(int row, int col) const;

  DiloPaperParams params_;
  double scale_;   // pixels per radian (columns) and per unit sin(elevation) (rows)
  double row0_;    // c_v
  std::deque<std::shared_ptr<DiloPaperFrame>> window_;
  std::vector<Eigen::Matrix4d> finalized_;
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
};

}  // namespace dilo
}  // namespace localization_zoo
