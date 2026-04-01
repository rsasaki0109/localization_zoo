#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>

namespace localization_zoo {
namespace ndt {

struct VoxelHash {
  std::size_t operator()(const Eigen::Vector3i& v) const {
    std::size_t seed = 0;
    auto hc = [&seed](int val) {
      seed ^= std::hash<int>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    hc(v.x()); hc(v.y()); hc(v.z());
    return seed;
  }
};

/// NDTボクセル (正規分布)
struct NDTCell {
  int num_points = 0;
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
  Eigen::Matrix3d cov_inv = Eigen::Matrix3d::Zero();
  bool valid = false;
};

struct NDTParams {
  double resolution = 1.0;       // ボクセルサイズ [m]
  int max_iterations = 30;
  double step_size = 0.1;        // ニュートン法のステップサイズ
  double convergence_threshold = 0.01;
  double outlier_ratio = 0.55;   // 外れ値比率 (ガウスの混合)
  int min_points_per_cell = 5;
};

struct NDTResult {
  Eigen::Matrix4d transformation = Eigen::Matrix4d::Identity();
  double score = 0.0;
  int iterations = 0;
  bool converged = false;
};

/// NDT マップ (正規分布ボクセルグリッド)
class NDTMap {
public:
  explicit NDTMap(double resolution, int min_points = 5);

  void setInputCloud(const std::vector<Eigen::Vector3d>& points);
  const NDTCell* lookup(const Eigen::Vector3d& point) const;
  size_t size() const { return cells_.size(); }

private:
  Eigen::Vector3i toVoxel(const Eigen::Vector3d& p) const;
  double resolution_;
  int min_points_;
  std::unordered_map<Eigen::Vector3i, NDTCell, VoxelHash> cells_;
};

/// NDT レジストレーション
class NDTRegistration {
public:
  explicit NDTRegistration(const NDTParams& params = NDTParams())
      : params_(params) {}

  /// ターゲット点群を設定 (NDTマップ構築)
  void setTarget(const std::vector<Eigen::Vector3d>& target);

  /// ソース点群をアライン
  NDTResult align(const std::vector<Eigen::Vector3d>& source,
                  const Eigen::Matrix4d& initial_guess =
                      Eigen::Matrix4d::Identity());

private:
  /// NDTスコアとヤコビアン・ヘシアンの計算
  double computeDerivatives(const std::vector<Eigen::Vector3d>& source,
                             const Eigen::Matrix4d& T,
                             Eigen::Matrix<double, 6, 1>& gradient,
                             Eigen::Matrix<double, 6, 6>& hessian) const;
  double computeScore(const std::vector<Eigen::Vector3d>& source,
                      const Eigen::Matrix4d& T) const;
  double lineSearchStep(const std::vector<Eigen::Vector3d>& source,
                        const Eigen::Matrix4d& T,
                        const Eigen::Matrix<double, 6, 1>& delta,
                        double current_score) const;
  Eigen::Matrix4d makeInitialGuess(
      const std::vector<Eigen::Vector3d>& source,
      const Eigen::Matrix4d& initial_guess) const;

  NDTParams params_;
  std::unique_ptr<NDTMap> target_map_;
  Eigen::Vector3d target_centroid_ = Eigen::Vector3d::Zero();
  double target_xy_yaw_ = 0.0;
};

}  // namespace ndt
}  // namespace localization_zoo
