#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <array>
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace cube_lio {

/// 強度つき点。
struct IntensityPoint {
  Eigen::Vector3d p = Eigen::Vector3d::Zero();
  double intensity = 0.0;
};

/// ボクセル座標ハッシュ。
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

/// キューブマップ強度投影 + IGM (intensity gradient magnitude)。
/// センサ中心から 6 面の画像平面に点を射影し、面ごとに強度画像をラスタ化、
/// ガウシアン平滑化 + Sobel で勾配ノルム (IGM) を計算する (CUBE-LIO スライド p.10-13)。
class CubeMap {
public:
  CubeMap(int size, double gaussian_sigma)
      : size_(size), gaussian_sigma_(gaussian_sigma) {}

  /// センサ系の強度点群から 6 面の強度画像と IGM 画像を構築する。
  void build(const std::vector<IntensityPoint>& points);

  /// 点が投影されるピクセルの IGM 値を返す (範囲外は 0)。
  double sampleIGM(const Eigen::Vector3d& p_sensor) const;

  int size() const { return size_; }

private:
  /// 点 → (face, u, v)。face<0 は無効。
  void project(const Eigen::Vector3d& d, int& face, double& u, double& v) const;

  int size_;
  double gaussian_sigma_;
  std::array<std::vector<float>, 6> intensity_;  ///< 面ごと size×size
  std::array<std::vector<float>, 6> igm_;        ///< 面ごと size×size
};

/// 強度つきボクセルハッシュマップ (平面法線 + ボクセル平均強度)。
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double planarity = 0.0;
    double intensity = 0.0;  ///< 近傍平均強度
    bool found = false;
    bool has_normal = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<IntensityPoint>& points);
  std::vector<Correspondence> getCorrespondences(
      const std::vector<Eigen::Vector3d>& points, double max_dist,
      int normal_min_neighbors) const;
  void pruneFarVoxels(const Eigen::Vector3d& center, double max_distance);

  void clear() { map_.clear(); }
  size_t size() const { return map_.size(); }

private:
  Eigen::Vector3i toVoxel(const Eigen::Vector3d& p) const {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / voxel_size_)),
                           static_cast<int>(std::floor(p.y() / voxel_size_)),
                           static_cast<int>(std::floor(p.z() / voxel_size_)));
  }

  double voxel_size_;
  int max_points_;
  struct VoxelBlock {
    std::vector<Eigen::Vector3d> points;
    std::vector<double> intensities;
  };
  std::unordered_map<Eigen::Vector3i, VoxelBlock, VoxelHash> map_;
};

struct CubeLIOParams {
  double voxel_size = 1.0;
  double max_range = 80.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;

  // --- キューブマップ / semi-dense 特徴 ---
  int cubemap_size = 256;            ///< 各面の解像度
  double gaussian_sigma = 1.0;       ///< IGM 平滑化シグマ
  double semi_dense_quantile = 0.6;  ///< IGM 上位分位を特徴に採用
  int min_semi_dense_points = 200;   ///< これ未満なら全点にフォールバック

  // --- 強度整合性重み ---
  double intensity_sigma = 30.0;     ///< 強度差の重みスケール

  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double initial_threshold = 2.0;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct CubeLIOResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int semi_dense_points = 0;  ///< 直近フレームで採用した semi-dense 特徴数
};

/// CUBE-LIO (劉 陽, ロボティクスシンポジア 2026 スライド準拠) の
/// intensity-cubemap semi-dense odometry フロントエンド。
///
/// 各フレームでキューブマップ強度画像から IGM を計算し、強度勾配が大きい
/// semi-dense 点を特徴として選ぶ。その特徴で scan-to-map point-to-plane を解き、
/// 対応の重みに幾何カーネルと強度整合性 (反射強度の一致) を掛ける
/// (測光 + 幾何の融合の近似)。
class CubeLIOPipeline {
public:
  explicit CubeLIOPipeline(const CubeLIOParams& params = CubeLIOParams());

  CubeLIOResult registerFrame(const std::vector<IntensityPoint>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }

private:
  std::vector<IntensityPoint> rangeFilter(
      const std::vector<IntensityPoint>& points) const;
  std::vector<IntensityPoint> voxelDownsample(
      const std::vector<IntensityPoint>& points, double voxel_size) const;
  /// IGM 上位分位の semi-dense 点を選ぶ。
  std::vector<IntensityPoint> selectSemiDense(
      const std::vector<IntensityPoint>& points, const CubeMap& cube) const;

  Eigen::Matrix4d runICP(const std::vector<IntensityPoint>& source,
                         const Eigen::Matrix4d& initial_guess,
                         double max_correspondence_dist);

  CubeLIOParams params_;
  VoxelHashMap local_map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  int frame_count_ = 0;
};

}  // namespace cube_lio
}  // namespace localization_zoo
