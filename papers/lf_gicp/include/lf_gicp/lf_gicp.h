#pragma once

// LF-GICP: parameter-free degeneracy handling for point-to-distribution
// LiDAR odometry via a voxel-normal localizability field.
// Reimplemented from the paper (arXiv:2608.19522, Im 2026); no author code.

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstdint>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace lf_gicp {

struct LFGICPParams {
  // Sensor-class constants (paper Table SI, >= 64 beams).
  double voxel_size = 1.0;          // map voxel
  double source_voxel_size = 0.3;   // scan downsampling
  double min_range = 1.0;           // not specified in the paper
  double max_range = 80.0;
  int max_iterations = 12;          // GN iterations
  double huber_delta = 1.0;
  double covariance_regularization = 2.0;  // beta in C + beta I (Table IV)
  int min_points_per_voxel = 3;
  int map_max_age_frames = 500;     // age-windowed local map
  double initial_threshold = 2.0;   // KISS-style adaptive correspondence threshold
  double min_motion_for_threshold = 0.1;
  double convergence_epsilon = 1e-4;

  // Degeneracy gate (paper Table SI, "global" constants).
  bool enable_mitigation = true;    // false = vanilla VGICP backend
  double tau_on = 0.165;            // enter: median f0 below
  double tau_off = 0.185;           // exit: median f0 above
  double tau_absence = 0.0143;      // median lambda0 below
  int gate_window = 20;             // frames
  int field_max_voxels = 4096;      // L_max
  // beta used for the field's planarity only (see README: the paper's lambda0
  // scale is matched on its KITTI 00 calibration trace).
  double field_regularization = 0.25;
};

/// Localizability statistics of the current local map (paper Eqs. 5-6).
struct LocalizabilityField {
  Eigen::Matrix3d M = Eigen::Matrix3d::Zero();
  double f0 = 1.0 / 3.0;       // lambda_min(M) / tr(M)
  double lambda0 = 0.0;        // lambda_min(M) / |V|
  Eigen::Vector3d weak_axis = Eigen::Vector3d::UnitX();
  int sampled_voxels = 0;
};

struct VoxelGaussian {
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
  int count = 0;
};

/// Voxel map of per-voxel Gaussians with age-based eviction.
class GaussianVoxelMap {
 public:
  explicit GaussianVoxelMap(double voxel_size) : voxel_size_(voxel_size) {}

  void insert(const std::vector<Eigen::Vector3d>& world_points, int frame);
  void evictOlderThan(int min_frame);
  std::vector<VoxelGaussian> gaussians(int min_points) const;
  /// Voxel keys and Gaussians with at least min_points points.
  std::vector<std::pair<Eigen::Vector3i, VoxelGaussian>> entries(int min_points) const;
  Eigen::Vector3i key(const Eigen::Vector3d& p) const;
  size_t size() const { return voxels_.size(); }

  struct KeyHash {
    size_t operator()(const Eigen::Vector3i& k) const {
      return (static_cast<size_t>(k.x()) * 73856093u) ^
             (static_cast<size_t>(k.y()) * 19349663u) ^
             (static_cast<size_t>(k.z()) * 83492791u);
    }
  };
  struct KeyEq {
    bool operator()(const Eigen::Vector3i& a, const Eigen::Vector3i& b) const {
      return a == b;
    }
  };

 private:
  struct Accumulator {
    Eigen::Vector3d sum = Eigen::Vector3d::Zero();
    Eigen::Matrix3d sum_sq = Eigen::Matrix3d::Zero();
    int count = 0;
    int last_frame = 0;
    VoxelGaussian toGaussian() const;
  };

  double voxel_size_;
  std::unordered_map<Eigen::Vector3i, Accumulator, KeyHash, KeyEq> voxels_;
};

/// Build the localizability field from map Gaussians (stride-subsampled to
/// max_voxels). Planarity uses the regularized information (C + beta I)^-1:
/// the paper's Table IV shows f0 changing with beta, which only happens if
/// rho is computed after regularization (its Sec. III-C text says "before").
LocalizabilityField computeLocalizabilityField(
    const std::vector<VoxelGaussian>& gaussians, int max_voxels,
    double covariance_regularization = 0.0);

/// Soft Fisher-information weights (paper Eq. 7): w_m = q_m / mean(q), with
/// q_m = v^T J_m^T Omega_m J_m v for the weakest Hessian eigenvector v.
std::vector<double> fisherWeights(
    const std::vector<Eigen::Matrix<double, 3, 6>>& jacobians,
    const std::vector<Eigen::Matrix3d>& informations,
    const Eigen::Matrix<double, 6, 6>& hessian);

/// Trailing-median gate with hysteresis (paper Sec. III-F, Alg. S1).
class DegeneracyGate {
 public:
  explicit DegeneracyGate(const LFGICPParams& params) : params_(params) {}
  bool update(double f0, double lambda0);
  bool active() const { return active_; }
  double medianF0() const { return median_f0_; }
  double medianLambda0() const { return median_lambda0_; }

 private:
  LFGICPParams params_;
  std::deque<double> f0_window_;
  std::deque<double> lambda_window_;
  bool active_ = false;
  double median_f0_ = 1.0 / 3.0;
  double median_lambda0_ = 1.0;
};

struct LFGICPResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  int iterations = 0;
  int correspondences = 0;
  bool degenerate = false;
  LocalizabilityField field;
};

class LFGICPOdometry {
 public:
  explicit LFGICPOdometry(const LFGICPParams& params = LFGICPParams());

  /// Register one scan given in the sensor frame; returns the world pose.
  LFGICPResult registerFrame(const std::vector<Eigen::Vector3d>& scan);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return map_.size(); }
  int degenerateFrames() const { return degenerate_frames_; }
  int frames() const { return frame_; }

 private:
  std::vector<Eigen::Vector3d> preprocess(
      const std::vector<Eigen::Vector3d>& scan) const;
  double adaptiveThreshold() const;

  LFGICPParams params_;
  GaussianVoxelMap map_;
  DegeneracyGate gate_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  double model_error_sq_sum_ = 0.0;
  int model_error_count_ = 0;
  int frame_ = 0;
  int degenerate_frames_ = 0;
};

}  // namespace lf_gicp
}  // namespace localization_zoo
