#pragma once

// SuMa frame-to-model odometry as described in Behley & Stachniss,
// "Efficient Surfel-Based SLAM using 3D Laser Range Data in Urban
// Environments" (RSS 2018), Sec. III-A to III-D and Table I, following the
// official implementation (jbehley/SuMa, config/default.xml) where the paper
// leaves details open. Everything the official code does on the GPU is done
// on the CPU here; loop closure and pose-graph optimisation (Sec. III-E) are
// not included, so this is the "Frame-to-Model" row of Table II.
//
// Per scan:
//  1. Vertex map V_D (nearest point per pixel, eq. 2, w x h = 900 x 64,
//     f_up 3 deg, f_down 25 deg, depth 2..75 m) and normal map N_D from
//     cross products of forward differences with horizontal wrap (eq. 3).
//  2. Model vertex / normal maps V_M, N_M rendered at the last pose T_WC(t-1)
//     from the active surfel map (created or updated in the last 100 scans)
//     with a stability above the threshold: every surfel is splatted as a
//     disk of its radius (CPU rasteriser, z-buffered, the pixel stores the
//     surfel centre and normal, as the official render_surfels shaders do).
//  3. Frame-to-model ICP (eqs. 5-9): projective association of every valid
//     data pixel through T_{C(t-1)C(t)}, bilinear model sampling, outliers
//     beyond delta_ICP = 2 m or theta_ICP = 30 deg, Huber weight k = 0.5,
//     Gauss-Newton on se(3) with left increments, at most 10 iterations,
//     warm start with the last increment. When the result differs from the
//     last increment by more than 0.4 m / 0.1 rad, the official code falls
//     back to frame-to-frame ICP against the previous scan (0.5 m / 30 deg).
//  4. Map update (Sec. III-D): radius r_s = sqrt(2) d p / clamp(cos, 0.5, 1)
//     (eq. 10, clamped to 0.05..1 m); every surfel visible from the new pose
//     is compared with the measurement in its pixel: compatible
//     (|n_s'(v_s - v_s')| < delta_M, |n_s x n_s'| < sin theta_M) surfels
//     gain stability (eq. 4) and are refined with gamma = 0.9 (eqs. 11-13)
//     when the measurement is more precise; the closest incompatible surfel
//     loses stability. Pixels without a compatible surfel create new
//     surfels. Surfels with l_s < odds(1 - p_stable), or unstable and not
//     updated for tau_O = 3 scans, are removed.
//
// Values only found in the official code (not in the paper) are marked
// "official" below.

#include "aloam/types.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstdint>
#include <vector>

namespace localization_zoo {
namespace suma {

struct SuMaPaperParams {
  // Vertex / normal map (Table I; official data_* parameters).
  int width = 900;
  int height = 64;
  double fov_up_deg = 3.0;
  double fov_down_deg = 25.0;  // magnitude
  double min_depth = 2.0;      // official min_depth
  double max_depth = 75.0;     // official max_depth
  // Rendered model view (official model_* parameters).
  double model_min_depth = 0.0;
  double model_max_depth = 100.0;

  // Frame-to-model ICP (Table I; official icp parameters).
  int max_iterations = 10;           // official "max iterations"
  double stop_delta = 1e-4;          // official "delta" (|dx|_inf)
  double stop_epsilon = 1e-4;        // official "stopping threshold"
  double icp_max_distance = 2.0;     // delta_ICP [m]
  double icp_max_angle_deg = 30.0;   // theta_ICP
  double huber_factor = 0.5;         // k
  bool bilinear_sampling = true;     // official
  // The official GL_LINEAR lookup blends invalid (zero) texels into the
  // model vertex and normal when at most 25 % of the weight is invalid,
  // pulling the vertex towards the sensor. true divides by the valid weight
  // (not in the official code). On KITTI use it together with the 0.22 deg
  // vertical-angle correction: in corridors where the forward direction is
  // nearly unconstrained (seq 02), the official lookup on raw scans drifts
  // forward (README, "Seq 02").
  bool normalize_bilinear = false;
  bool initialize_identity = false;  // false: warm start with last increment
  // Non-paper opt-in: project local translation updates out of weak normal-
  // equation directions, using common/degeneracy.h. 0 disables the remap.
  // This does not freeze the accumulated pose along those directions.
  double degenerate_ratio = 0.0;

  // Frame-to-frame fallback after a track loss (official).
  bool fallback_mode = true;
  double fallback_max_distance = 0.5;
  double fallback_max_angle_deg = 30.0;
  double fallback_trans_threshold = 0.4;  // [m] change w.r.t. last increment
  double fallback_rot_threshold = 0.1;    // [rad]

  // Surfel map (Table I; official map parameters).
  double min_radius = 0.05;
  double max_radius = 1.0;
  double max_angle_deg = 90.0;      // view angle limit for new surfels
  double map_max_distance = 0.2;    // delta_M [m]
  double map_max_angle_deg = 45.0;  // theta_M: official 45, Table I says 30
  int unstable_age = 3;             // tau_O
  double confidence_threshold = 0.0;  // render / keep threshold on l_s
  double p_stable = 0.6;
  double p_prior = 0.5;
  double sigma_angle = 1.0;
  double sigma_distance = 1.0;
  bool use_stability = true;
  double gamma = 0.9;               // weight of the old surfel (eqs. 11-12)
  int active_timestamps = 100;      // Delta_active
  double max_confidence = 20.0;     // official upper stability bound
  int time_init = 10;               // official ramp of the render threshold
  double p_unstable_init = 0.1;     // ... starting at odds(0.1)

  // Map bookkeeping (stand-in for the official GPU submaps): surfels farther
  // than this from the sensor in x/y are dropped (official: 4 x 2 x 10 m +
  // 10 m around the current submap); inactive surfels are dropped unless
  // keep_inactive (they are only used for loop closures).
  double map_extent = 90.0;
  bool keep_inactive = false;
};

struct SuMaPaperResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();       // T_W_Ct
  Eigen::Matrix4d increment = Eigen::Matrix4d::Identity();  // T_C(t-1)_Ct
  int valid_pixels = 0;      // data pixels with vertex and normal
  int model_pixels = 0;      // rendered model pixels
  int inliers = 0;
  int outliers = 0;
  int iterations = 0;
  bool fallback = false;     // frame-to-frame fallback used
  size_t num_surfels = 0;
  int new_surfels = 0;
  int updated_surfels = 0;
};

class SuMaPaper {
public:
  // Image with a vertex and a normal per pixel (row 0 = upper fov limit).
  struct VertexNormalMap {
    int width = 0;
    int height = 0;
    std::vector<Eigen::Vector3f> vertex;
    std::vector<Eigen::Vector3f> normal;
    std::vector<uint8_t> vertex_valid;
    std::vector<uint8_t> normal_valid;

    void reset(int w, int h);
    int index(int u, int v) const { return v * width + u; }
    bool valid(int i) const { return vertex_valid[i] && normal_valid[i]; }
  };

  struct Surfel {
    Eigen::Vector3f position;  // world frame
    Eigen::Vector3f normal;    // world frame
    float radius = 0.0f;
    float confidence = 0.0f;   // stability log odds l_s
    int created = 0;           // t_c
    int updated = 0;           // t_u
  };

  struct IcpStats {
    int inliers = 0;
    int outliers = 0;
    int iterations = 0;
    double error = 0.0;                                       // sum w r^2
    Eigen::Matrix<double, 6, 6> JtJ = Eigen::Matrix<double, 6, 6>::Zero();  // last iteration
  };

  explicit SuMaPaper(const SuMaPaperParams& params = SuMaPaperParams());

  SuMaPaperResult process(const std::vector<Eigen::Vector3d>& points);
  SuMaPaperResult process(const aloam::PointCloudConstPtr& cloud);
  void clear();

  const Eigen::Matrix4d& pose() const { return pose_; }
  const std::vector<Surfel>& surfels() const { return surfels_; }
  const VertexNormalMap& lastData() const { return last_data_; }
  const VertexNormalMap& lastModel() const { return model_; }
  const SuMaPaperParams& params() const { return params_; }

  // Building blocks, public for tests.
  // Continuous image coordinates (x in [0, w), y in [0, h), y = 0 at the
  // upper fov limit) of a point; false outside the vertical fov.
  bool project(const Eigen::Vector3d& p, double* x, double* y) const;
  VertexNormalMap computeVertexNormalMap(
      const std::vector<Eigen::Vector3d>& points) const;
  // Render surfels (active, stability above the threshold) at T_W_C.
  VertexNormalMap renderModel(const Eigen::Matrix4d& T_world_sensor,
                              float confidence_threshold,
                              int* rendered_pixels = nullptr) const;
  // Projective point-to-plane Gauss-Newton of data against model; returns
  // T_model_data.
  Eigen::Matrix4d alignProjective(const VertexNormalMap& data,
                                  const VertexNormalMap& model,
                                  const Eigen::Matrix4d& initial,
                                  double max_distance, double max_angle_deg,
                                  IcpStats* stats = nullptr) const;
  // Insert / update surfels with a data frame at T_W_C.
  void updateMap(const VertexNormalMap& data, const Eigen::Matrix4d& T_world_sensor,
                 SuMaPaperResult* result = nullptr);
  float renderConfidenceThreshold() const;

private:
  struct Normal6 {
    Eigen::Matrix<double, 6, 6> JtJ;
    Eigen::Matrix<double, 6, 1> Jtf;
    double error = 0.0;
    int inliers = 0;
    int outliers = 0;
  };

  void accumulate(const VertexNormalMap& data, const VertexNormalMap& model,
                  const Eigen::Matrix4d& pose, double max_distance,
                  double cos_max_angle, Normal6* out) const;
  double pixelSize() const;
  double logOdds(double p) const;

  SuMaPaperParams params_;
  std::vector<Surfel> surfels_;
  VertexNormalMap last_data_;
  VertexNormalMap model_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_increment_ = Eigen::Matrix4d::Identity();
  int timestamp_ = 0;
};

// se(3) exponential (translation first), as in the official lie_algebra.cpp.
Eigen::Matrix4d se3Exp(const Eigen::Matrix<double, 6, 1>& x);

}  // namespace suma
}  // namespace localization_zoo
