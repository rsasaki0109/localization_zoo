#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace terrain_rbf_lio {

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

struct Cell2DHash {
  std::size_t operator()(const Eigen::Vector2i& v) const {
    std::size_t seed = 0;
    seed ^= std::hash<int>()(v.x()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<int>()(v.y()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
  }
};

/// 平面法線つきボクセルハッシュマップ (scan-to-map point-to-plane 用)。
class VoxelHashMap {
public:
  struct Correspondence {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    Eigen::Vector3d anchor = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double planarity = 0.0;
    bool found = false;
    bool has_normal = false;
  };

  explicit VoxelHashMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addPoints(const std::vector<Eigen::Vector3d>& points);
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
  };
  std::unordered_map<Eigen::Vector3i, VoxelBlock, VoxelHash> map_;
};

/// RBF 地形多様体 (terrain manifold)。地形高さ場 z = f(x,y) を Gaussian RBF で
/// 表現する。中心は水平グリッドセルで、各セル高さを逐次最小二乗 (RLS) 風に更新。
/// f(x,y) = Σ_i w_i φ_i / Σ_i φ_i,  φ_i = exp(-||(x,y)-c_i||²/2σ²)
/// (Nadaraya-Watson 形の Gaussian RBF 補間。RLS で更新したセル高さ w_i を補間)。
class TerrainRbfMap {
public:
  TerrainRbfMap(double cell_size, double sigma, double ground_band)
      : cell_size_(cell_size), sigma_(sigma), ground_band_(ground_band) {}

  /// world 系の点群で地形高さ場を更新 (各セルの ground 高さを RLS 更新)。
  void update(const std::vector<Eigen::Vector3d>& world_points);
  /// (x,y) における地形高さ f を評価。有効中心が無ければ valid=false。
  double evaluate(double x, double y, bool* valid,
                  double* dfdx = nullptr, double* dfdy = nullptr) const;
  size_t size() const { return cells_.size(); }

private:
  Eigen::Vector2i toCell(double x, double y) const {
    return Eigen::Vector2i(static_cast<int>(std::floor(x / cell_size_)),
                           static_cast<int>(std::floor(y / cell_size_)));
  }
  double cell_size_;
  double sigma_;
  double ground_band_;
  struct Cell {
    double cx = 0.0, cy = 0.0;  // セル中心 (水平)
    double h = 0.0;             // ground 高さ推定
    double n = 0.0;             // RLS 有効サンプル数
  };
  std::unordered_map<Eigen::Vector2i, Cell, Cell2DHash> cells_;
};

struct TerrainRbfParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double initial_threshold = 2.0;

  // --- RBF 地形 prior ---
  /// RBF 中心グリッドの水平解像度 (論文値 0.07 m は脚輪ロボ向け。車載は粗く)。
  double terrain_cell_size = 2.0;
  /// Gaussian カーネルのバンド幅 σ。
  double terrain_sigma = 3.0;
  /// ground 候補帯域。現セル推定からこの範囲内/以下の点だけ ground 更新に使う。
  double terrain_ground_band = 1.0;
  /// 地形ソフト拘束の重み λ_M (z-drift 抑制用、point-to-plane に加算)。
  double terrain_weight = 0.0;
  /// 地形拘束を有効化する最初のフレーム (warmup で h_sensor を推定)。
  int terrain_warmup = 10;
  /// h_sensor (地形面からのセンサ高) の EMA 係数。
  double sensor_height_alpha = 0.05;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct TerrainRbfResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  bool terrain_active = false;   ///< このフレームで地形拘束が発火したか
  double terrain_residual = 0.0; ///< 地形拘束残差 r_M = (t_z-h_sensor)-f(x,y)
  double sensor_height = 0.0;    ///< 現在の h_sensor 推定
};

/// Terrain-Aware RBF-LIO (arXiv:2509.26222, Liu & Zhang 2025) の from-paper
/// 再実装。原論文は脚輪ロボ (legged-wheel) 向けで、車輪中心 ξ_L が地形より
/// 半径 r だけ上にある接地拘束 r_M = ξ_z - r - f(ξ_x,ξ_y) を soft constraint に
/// 用いる。本実装は KITTI 車載 (純 LiDAR) 向けに、車輪接地拘束を「車両基準点が
/// 局所地形より一定高 h_sensor だけ上にある」soft prior へ翻案 (z-drift 抑制)。
/// RBF+RLS で滑らかな地形 manifold を維持する論文の核は保持する。
class TerrainRbfPipeline {
public:
  explicit TerrainRbfPipeline(const TerrainRbfParams& params = TerrainRbfParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  TerrainRbfResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return local_map_.size(); }
  size_t terrainSize() const { return terrain_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  Eigen::Matrix4d runRegistration(const std::vector<Eigen::Vector3d>& source,
                                  const Eigen::Matrix4d& initial_guess,
                                  TerrainRbfResult* result);
  Eigen::Matrix4d predict() const;

  TerrainRbfParams params_;
  VoxelHashMap local_map_;
  TerrainRbfMap terrain_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  double sensor_height_ = 0.0;
  bool sensor_height_init_ = false;
  int frame_count_ = 0;
};

}  // namespace terrain_rbf_lio
}  // namespace localization_zoo
