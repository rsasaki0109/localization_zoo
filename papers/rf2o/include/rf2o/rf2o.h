#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <vector>

namespace localization_zoo {
namespace rf2o {

/// Planar laser scan (ROS LaserScan-compatible layout).
struct LaserScan {
  double angle_min = -3.141592653589793;
  double angle_max = 3.141592653589793;
  double angle_increment = 0.0;
  double range_min = 0.1;
  double range_max = 100.0;
  double timestamp = 0.0;
  std::vector<double> ranges;

  size_t size() const { return ranges.size(); }
  double fov() const { return angle_max - angle_min; }
};

struct RF2OParams {
  int coarse_to_fine_levels = 5;
  int target_cols = 0;  ///< 0 = use full scan width
  int irls_iterations = 5;
  double min_range = 0.1;
  double max_range = 30.0;
  double max_range_diff = 0.3;
  double default_dt = 0.1;
};

struct RF2OResult {
  Eigen::Matrix3d increment = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d pose = Eigen::Matrix3d::Identity();
  bool valid = false;
  int valid_beams = 0;
};

/// RF2O range-flow 2D laser odometry (Jaimez et al., ICRA 2016, simplified port).
class RF2OEstimator {
 public:
  explicit RF2OEstimator(const RF2OParams& params = RF2OParams());

  void reset();
  void setInitialPose(const Eigen::Matrix3d& pose);

  RF2OResult registerScan(const LaserScan& scan, double dt);

  const Eigen::Matrix3d& pose() const { return pose_; }

 private:
  struct LevelData {
    int cols = 0;
    std::vector<float> range;
    std::vector<float> range_old;
    std::vector<float> range_warped;
    std::vector<float> range_inter;
    std::vector<float> xx;
    std::vector<float> yy;
    std::vector<float> xx_inter;
    std::vector<float> yy_inter;
    std::vector<float> xx_old;
    std::vector<float> yy_old;
    std::vector<float> xx_warped;
    std::vector<float> yy_warped;
    std::vector<float> dt;
    std::vector<float> dtita;
    std::vector<float> weights;
    std::vector<int> null_mask;
    std::vector<float> rtita;
  };

  void initialize(const LaserScan& scan);
  void createPyramid(const std::vector<float>& ranges);
  void createImagePyramid();
  void warpToPreviousLevel(int level, const Eigen::Matrix3f& cumulative);
  void calculateCoord(int level);
  void calculateRangeDerivatives(int level, float fps);
  void computeWeights(int level, float fps);
  void findNullPoints(int level);
  bool solveSystem(int level, float fps, Eigen::Vector3f* velocity);
  bool filterLevelSolution(int ctf_level, float fps, const Eigen::Vector3f& velocity,
                           Eigen::Matrix3f* increment);
  static Eigen::Matrix3f velocityToIncrement(const Eigen::Vector3f& vel,
                                               float fps);

  RF2OParams params_;
  bool initialized_ = false;
  int width_ = 0;
  int cols_ = 0;
  float fovh_ = 0.0f;
  std::vector<LevelData> pyramid_;
  std::vector<Eigen::Matrix3f> level_transforms_;
  Eigen::Vector3f kai_loc_old_ = Eigen::Vector3f::Zero();
  Eigen::Matrix3f cov_odo_ = Eigen::Matrix3f::Identity();
  Eigen::Matrix3d pose_ = Eigen::Matrix3d::Identity();
  std::vector<float> g_mask_{1.f / 16.f, 0.25f, 6.f / 16.f, 0.25f, 1.f / 16.f};
};

Eigen::Matrix3d pose2D(double x, double y, double yaw);

}  // namespace rf2o
}  // namespace localization_zoo
