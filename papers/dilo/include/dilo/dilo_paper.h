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
// Edge key points (Sec. 3.5) are not implemented; the paper's plane-only
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
  double min_planarity = 0.0;
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
  double huber_delta = 0.25;
  double tukey_c = 1.0;
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

private:
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
