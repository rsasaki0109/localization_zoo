#pragma once

// IMLS-SLAM odometry as described in Deschaud, "IMLS-SLAM: scan-to-model
// matching based on 3D data" (ICRA 2018), separate from the harness-oriented
// ImlsSlam pipeline:
//
// - every raw scan point gets a PCA normal and a planarity a2D from the scan;
// - nine lists rank the points by a2D^2 (x x n).e_a (both signs, three axes)
//   and a2D^2 |n.e_a|; at each iteration the lists are walked from the top
//   and a point is kept when the model has a point within r of it, until s
//   points per list are kept (9 s samples, duplicates kept);
// - the model is the raw points of the last n localized scans (sliding
//   window, oldest scan dropped);
// - for each sample x the normal n_x is fitted to the model points within r
//   at every iteration, and I(x) = sum w_i (x - p_i).n_x / sum w_i with
//   w_i = exp(-|x - p_i|^2 / h^2) (eq. 1, one normal for the neighbourhood);
// - the scan is moved towards y = x - I(x) n_x by linear least squares
//   (small-angle), a fixed number of times;
// - the next pose is predicted with constant velocity.
//
// h = 0.06 m, r = 0.20 m, s = 100, n = 100 and 20 iterations are the
// paper's KITTI values (Sec. V-VI).

#include <Eigen/Core>

#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace imls_slam {

struct ImlsPaperParams {
  // Repository implementation acceleration, not an additional paper step:
  // parallel independent work and memoize repeated projections per iteration.
  // Opt-in; the sampling and normal-equation accumulation order are retained.
  bool accelerated = false;
  double h = 0.06;
  double r = 0.20;
  int samples_per_list = 100;
  int window_scans = 100;
  int iterations = 20;
  double scan_normal_radius = 0.5;  // PCA radius for the scan normals (not given in the paper)
  int scan_normal_max_neighbors = 30;
  int min_normal_points = 5;
  double min_range = 1.0;
  double max_range = 1e9;
  // Coarse start (not in the paper): a sample is kept only when the model has
  // a point within r of it, and the lists are walked until s samples are
  // kept, so from an initial guess more than about r off the samples that
  // would correct it (facades across the direction of travel) are skipped
  // and the scan barely moves. A sequence that starts in motion (KITTI 04,
  // 1.3 m per scan) with the constant-velocity guess at zero then stays
  // short for tens of scans. The first coarse_frames registrations run
  // coarse_iterations with coarse_r / coarse_h before the paper's
  // iterations; coarse_frames = 0 disables it.
  int coarse_frames = 0;
  double coarse_r = 1.0;
  double coarse_h = 0.3;
  int coarse_iterations = 10;
};

struct ImlsPaperResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();  // world <- sensor
  int samples = 0;                                      // last iteration
  size_t model_points = 0;
  bool coarse_start = false;                            // coarse stage ran
};

class ImlsPaperOdometry {
public:
  explicit ImlsPaperOdometry(const ImlsPaperParams& params = ImlsPaperParams());

  ImlsPaperResult process(const std::vector<Eigen::Vector3d>& scan);
  const Eigen::Matrix4d& pose() const { return pose_; }

  struct ScanPoint {
    Eigen::Vector3d p;
    Eigen::Vector3d n;
    double a2d = 0.0;
  };
  // Normals and planarity of one scan (sensor frame); points without a
  // normal are dropped. Exposed for tests.
  std::vector<ScanPoint> describe(const std::vector<Eigen::Vector3d>& scan) const;
  // The nine sorted index lists over `points`. Exposed for tests.
  std::vector<std::vector<int>> sampleLists(const std::vector<ScanPoint>& points) const;
  // IMLS projection of x onto the model: false if the model has no point
  // within r or too few for a normal.
  bool imls(const Eigen::Vector3d& x, Eigen::Vector3d* normal, double* distance) const {
    return imls(x, params_.r, params_.h, normal, distance);
  }
  bool imls(const Eigen::Vector3d& x, double r, double h, Eigen::Vector3d* normal,
            double* distance) const;
  void insertScan(const std::vector<Eigen::Vector3d>& scan, const Eigen::Matrix4d& pose);

private:
  struct KeyHash {
    size_t operator()(const Eigen::Vector3i& k) const {
      return (static_cast<size_t>(k.x()) * 73856093u) ^ (static_cast<size_t>(k.y()) * 19349663u) ^
             (static_cast<size_t>(k.z()) * 83492791u);
    }
  };
  using Grid = std::unordered_map<Eigen::Vector3i, std::vector<Eigen::Vector3d>, KeyHash>;
  struct Entry {
    int scan_id;
    Eigen::Vector3d p;
  };
  using ModelGrid = std::unordered_map<Eigen::Vector3i, std::vector<Entry>, KeyHash>;
  using ModelCells = std::vector<const std::vector<Entry>*>;

  Eigen::Vector3i key(const Eigen::Vector3d& p, double cell) const;
  bool hasModelPointWithin(const Eigen::Vector3d& x, double r,
                           const ModelCells* cells = nullptr) const;
  bool project(const Eigen::Vector3d& x, double r, double h, Eigen::Vector3d* normal,
               double* distance, const ModelCells* cells) const;
  // Gauss-Newton iterations at radius r and bandwidth h; returns the samples
  // of the last iteration (or of the first when count_only).
  int iterate(const std::vector<ScanPoint>& points, const std::vector<std::vector<int>>& lists,
              double r, double h, int iterations, bool count_only, Eigen::Matrix4d* T) const;

  ImlsPaperParams params_;
  ModelGrid model_;
  std::deque<std::pair<int, std::vector<Eigen::Vector3i>>> window_;  // scan id, its cells
  size_t model_points_ = 0;
  int next_scan_id_ = 0;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d previous_pose_ = Eigen::Matrix4d::Identity();
  int frame_ = 0;
};

}  // namespace imls_slam
}  // namespace localization_zoo
