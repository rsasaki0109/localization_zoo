#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace imls_slam {

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

/// 法線つき有向点 (oriented point) を格納するボクセルハッシュマップ。
/// IMLS の暗黙的曲面は過去スキャンの有向点群から定義されるため、点だけでなく
/// 各点の法線も保持する (PCA で都度計算する point-to-plane マップとは異なる)。
class ImlsModelMap {
public:
  struct Sample {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
  };

  /// query() の戻り値: 支持半径内の有向点集合 + 最近傍点情報。
  struct Neighbors {
    std::vector<Eigen::Vector3d> points;
    std::vector<Eigen::Vector3d> normals;
    Eigen::Vector3d nearest_normal = Eigen::Vector3d::Zero();
    double nearest_dist_sq = 0.0;
    bool found = false;  // 支持半径内に 1 点以上
  };

  explicit ImlsModelMap(double voxel_size, int max_points_per_voxel = 20)
      : voxel_size_(voxel_size), max_points_(max_points_per_voxel) {}

  void addSamples(const std::vector<Eigen::Vector3d>& points,
                  const std::vector<Eigen::Vector3d>& normals);
  Neighbors query(const Eigen::Vector3d& x, double support_radius) const;
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
    std::vector<Sample> samples;
  };
  std::unordered_map<Eigen::Vector3i, VoxelBlock, VoxelHash> map_;
};

struct ImlsSlamParams {
  double voxel_size = 1.0;
  double max_range = 100.0;
  double min_range = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  /// スキャン法線推定の近傍半径 [m]。<=0 なら voxel_size*1.5。
  double normal_radius = 0.0;

  // --- Implicit Moving Least Squares (IMLS) 暗黙的曲面 ---
  /// IMLS カーネルバンド幅 h [m]。重み W_i(x)=exp(−||x−p_i||²/h²)。
  /// 原論文は生 HDL64 密度向けに h=0.06 / r=0.20 だが、本実装はマップを
  /// ボクセルダウンサンプルするためマップ密度に合わせて h を拡大する。
  double imls_h = 0.5;
  /// IMLS 支持半径 [m] (この半径内の有向点で曲面を定義)。<=0 なら 3*imls_h。
  double imls_support_radius = 0.0;
  /// 最近傍点ゲート r [m] (原論文 0.20、マップ密度に合わせ拡大)。
  double max_correspondence_dist = 1.0;
  int max_iterations = 20;
  double convergence_criterion = 0.001;

  // --- 観測性ベースのサンプリング (paper の 9-score サンプリングを近似) ---
  /// true なら 6-DoF を最もよく拘束する点を選ぶ。false なら全有効点を使う。
  bool use_observability_sampling = true;
  /// ±並進3軸 + 回転3軸の各観測性リストから上位何点採るか (原論文 s=100)。
  int samples_per_axis = 100;

  double local_map_radius = 0.0;
  int map_cleanup_interval = 0;
};

struct ImlsSlamResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  /// 観測性サンプリングで選ばれた点数。
  int num_samples = 0;
  /// 最終反復の平均 |IMLS距離| [m] (曲面への残差)。
  double mean_abs_residual = 0.0;
};

/// IMLS-SLAM: 暗黙的移動最小二乗 (Implicit Moving Least Squares) 曲面への
/// scan-to-model マッチングによる LiDAR オドメトリ。
///
///   "IMLS-SLAM: scan-to-model matching based on 3D data"
///   (J.-E. Deschaud, ICRA 2018, arXiv:1802.08633) の from-paper 再構成。
///   著者は後発の CT-ICP は公開したが IMLS-SLAM の実装は未公開。
///
/// 過去スキャンの有向点群 {p_i, n_i} が暗黙的曲面
///   I^P(x) = Σ_i W_i(x) (x−p_i)·n_i / Σ_i W_i(x),   W_i(x)=exp(−||x−p_i||²/h²)
/// を定義する。新スキャンの各サンプル点 x の曲面距離 I^P(x) を最小化する姿勢を
/// Gauss-Newton で解く (point-to-implicit-surface)。単一最近傍平面への
/// point-to-plane (mcc_lo / 多くの既存手法) と異なり、複数の有向点を距離で
/// 重み付けした滑らかな高次曲面に当てる点が本機構の特徴。さらに 6-DoF を最も
/// 拘束する点を選ぶ観測性ベースのサンプリングで効率と安定性を得る。
class ImlsSlamPipeline {
public:
  explicit ImlsSlamPipeline(const ImlsSlamParams& params = ImlsSlamParams());

  void setInitialPose(const Eigen::Matrix4d& pose) { pose_ = pose; }
  ImlsSlamResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }
  size_t mapSize() const { return model_.size(); }

  // --- 機構ヘルパ (ユニットテスト用に公開) ---

  /// IMLS 暗黙的曲面距離 I^P(x) = Σ W_i (x−p_i)·n_i / Σ W_i,
  /// W_i = exp(−||x−p_i||²/h²)。支持点が無ければ 0 を返す。
  static double imlsDistance(const Eigen::Vector3d& x,
                             const std::vector<Eigen::Vector3d>& pts,
                             const std::vector<Eigen::Vector3d>& normals,
                             double h);

  /// 観測性サンプリング: 各点の法線 n と位置 p (センサ原点基準) から
  /// 並進拘束 |n·e_a| と回転拘束 |(p×n)·e_a| を評価し、3 並進軸 + 3 回転軸の
  /// 各リスト上位 samples_per_axis 点の和集合 (重複除去) を返す。
  static std::vector<int> selectObservabilityPoints(
      const std::vector<Eigen::Vector3d>& points,
      const std::vector<Eigen::Vector3d>& normals, int samples_per_axis);

private:
  std::vector<Eigen::Vector3d> rangeFilter(
      const std::vector<Eigen::Vector3d>& points) const;
  std::vector<Eigen::Vector3d> voxelDownsample(
      const std::vector<Eigen::Vector3d>& points, double voxel_size) const;
  void computeNormals(const std::vector<Eigen::Vector3d>& pts, double radius,
                      std::vector<Eigen::Vector3d>* normals,
                      std::vector<char>* valid) const;
  Eigen::Matrix4d predict() const;
  Eigen::Matrix4d runRegistration(
      const std::vector<Eigen::Vector3d>& source,
      const std::vector<int>& sample_idx, const Eigen::Matrix4d& base,
      ImlsSlamResult* result);

  ImlsSlamParams params_;
  ImlsModelMap model_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();
  std::deque<Eigen::Matrix4d> delta_window_;
  int frame_count_ = 0;
};

}  // namespace imls_slam
}  // namespace localization_zoo
