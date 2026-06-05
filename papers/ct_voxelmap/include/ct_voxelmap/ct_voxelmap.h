#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace ct_voxelmap {

/// ボクセル座標ハッシュ (KISS-ICP と同じ方式)。
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

/// 確率的適応ボクセルマップ。
/// 各ボクセルは内包点の Gaussian (平均 μ, 共分散 Σ) を逐次蓄積し、
/// 平面的なら平面 (法線 n, 点 q=μ, 平面厚み分散 σ_plane²) を、
/// そうでなければ分布特徴 (μ, Σ) を保持する (CT-VoxelMap の hybrid 特徴)。
class ProbabilisticVoxelMap {
public:
  struct VoxelFeature {
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double plane_variance = 0.0;  ///< 平面厚み (= λ_min)
    int count = 0;
    bool is_planar = false;
    bool valid = false;
  };

  struct Query {
    VoxelFeature feature;
    bool found = false;
  };

  ProbabilisticVoxelMap(double voxel_size, int min_points, double planarity_ratio)
      : voxel_size_(voxel_size),
        min_points_(min_points),
        planarity_ratio_(planarity_ratio) {}

  /// 世界座標の点群をボクセル統計に加算し、特徴を更新する。
  void addPoints(const std::vector<Eigen::Vector3d>& points);

  /// クエリ点が属する (or 近傍) ボクセル特徴を返す。
  Query query(const Eigen::Vector3d& world_point) const;

  void pruneFarVoxels(const Eigen::Vector3d& center, double max_distance);
  void clear() { voxels_.clear(); }
  size_t size() const { return voxels_.size(); }

private:
  Eigen::Vector3i toVoxel(const Eigen::Vector3d& p) const {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / voxel_size_)),
                           static_cast<int>(std::floor(p.y() / voxel_size_)),
                           static_cast<int>(std::floor(p.z() / voxel_size_)));
  }

  struct Voxel {
    int count = 0;
    Eigen::Vector3d sum = Eigen::Vector3d::Zero();
    Eigen::Matrix3d sum_outer = Eigen::Matrix3d::Zero();
    VoxelFeature feature;
  };

  void refreshFeature(Voxel& v) const;

  double voxel_size_;
  int min_points_;
  double planarity_ratio_;
  std::unordered_map<Eigen::Vector3i, Voxel, VoxelHash> voxels_;
};

struct CTVoxelMapParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int voxel_min_points = 6;
  double planarity_ratio = 0.1;     ///< λ0 < ratio·λ2 で平面とみなす

  int max_icp_iterations = 100;
  double convergence_criterion = 0.001;
  double max_correspondence_dist = 2.0;

  // --- 点不確かさモデル (range/bearing 依存) ---
  double sigma_depth = 0.02;        ///< 距離方向の標準偏差 [m]
  double sigma_bearing = 0.0015;    ///< 角度方向の標準偏差 [rad]

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct CTVoxelMapResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  double planar_ratio = 0.0;   ///< 直近フレームの平面対応比率
};

/// CT-VoxelMap (arXiv:2604.03747) の確率的適応ボクセルマップ登録フロントエンド。
///
/// CT-VoxelMap の中核である確率的 hybrid ボクセル特徴と点不確かさ伝播を実装する:
///  - 各ボクセルが平面 or 分布特徴を保持 (適応的 hybrid)。
///  - 各測定点の共分散 Σ_p を range/bearing から構成し姿勢で伝播。
///  - 残差分散 = nᵀ R Σ_p Rᵀ n + σ_plane² の逆数で重み付けする
///    Mahalanobis point-to-plane / point-to-distribution 登録。
///
/// NOTE: 論文の B-spline + IEKF バックエンドおよび per-point time の
/// continuous-time deskew はこのフロントエンドには含まない (README 参照)。
/// 連続時間性は等速モデル予測で近似する。
class CTVoxelMapPipeline {
public:
  explicit CTVoxelMapPipeline(const CTVoxelMapParams& params = CTVoxelMapParams());

  CTVoxelMapResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return map_.size(); }

private:
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;

  /// 点 p (センサ系) の測定共分散を range/bearing モデルで構成する。
  Eigen::Matrix3d pointCovariance(const Eigen::Vector3d& p_sensor) const;

  Eigen::Matrix4d runICP(const std::vector<Eigen::Vector3d>& source,
                         const Eigen::Matrix4d& initial_guess,
                         double* planar_ratio_out);

  CTVoxelMapParams params_;
  ProbabilisticVoxelMap map_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();
  int frame_count_ = 0;
};

}  // namespace ct_voxelmap
}  // namespace localization_zoo
