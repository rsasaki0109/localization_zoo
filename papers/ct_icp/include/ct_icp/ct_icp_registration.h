#pragma once

#include "ct_icp/types.h"

#include <deque>
#include <memory>
#include <vector>

namespace localization_zoo {
namespace ct_icp {

struct CTICPParams {
  int max_iterations = 30;           // ICPの最大外側反復
  int ceres_max_iterations = 5;      // Ceres内部反復
  double voxel_resolution = 1.0;     // ボクセルサイズ [m]
  int knn = 5;                       // 近傍点数
  double max_correspondence_dist = 100.0;  // 対応点の最大距離² [m²]
  double planarity_threshold = 0.3;  // 平面性閾値 (a2D)
  double convergence_threshold = 0.001;  // 収束閾値 [m]

  // ロバスト損失
  double cauchy_loss_param = 0.5;

  // 正則化重み
  double location_consistency_weight = 0.001;
  double orientation_consistency_weight = 0.001;
  double constant_velocity_weight = 0.001;

  // ボクセルサブサンプリング
  double keypoint_voxel_size = 0.5;  // キーポイントのボクセルサイズ [m]

  // スライディングウィンドウ
  int max_frames_in_map = 30;  // マップに保持する最新フレーム数

  // 拡張: 27 ボクセル近傍で knn 未満のときに 125 ボクセル (5x5x5) まで広げる
  bool multi_scale_correspondences = false;

  // Ceres linear solver. 既定 DENSE_QR と DENSE_NORMAL_CHOLESKY の選択肢。
  // CT-ICP の論文公式実装は DENSE_NORMAL_CHOLESKY を採用しているため、
  // paper 一致設定では true にする。
  bool use_normal_cholesky_solver = false;

  // Paper weight scheme (jedeschaud/ct_icp YAML 一致):
  // correspondence weight = weight_alpha * a2D^power_planarity
  //                       + weight_neighborhood * exp(-d_closest / d_max)
  // ここで d_closest は最近傍距離 [m]、d_max は sqrt(max_correspondence_dist) [m]。
  // weight_alpha=1.0, weight_neighborhood=0.0, power_planarity=1.0 で現状互換。
  // paper 既定: weight_alpha=0.9, weight_neighborhood=0.1, power_planarity=2.0
  double power_planarity = 1.0;
  double weight_alpha = 1.0;
  double weight_neighborhood = 0.0;

  // 平面性の soft floor。a2D < min_planarity_floor のときに対応点を rejection。
  // planarity_threshold (hard) と排他ではなく両方適用される。
  // paper 既定: 0.01 (planarity_threshold は 0 に近づけて soft weight に任せる)。
  double min_planarity_floor = 0.0;

  // 正則化重みを sqrt(N_corr · β) ではなく sqrt(β) フラットにする。
  // 既定 false は現状互換、true で paper 一致 (β=0.001 単独適用)。
  bool flat_regularizer_weight = false;

  // 正則化重みの N_corr 上限。<=0 で無効 (N_corr 全数使用、現状互換)。
  // 正値 cap で weight = sqrt(min(N_corr, cap) * β) になる。
  // 短軌跡では大きな N_corr が prior を 22-30x amplify するが、長軌跡では
  // この amplification が global drift を抑える load-bearing なので、cap で
  // 中間策を取る (KITTI 07: cap 小、KITTI 02: cap 大が好ましいかは未確認)。
  int regularizer_n_cap = 0;

  // Paper-aligned correspondence anchoring (Gap A):
  // 既定 false で reference = mean of knn neighbors (現状)。
  // true で reference = closest neighbor only。
  bool use_closest_neighbor_reference = false;
  // PCA に使う近傍点数。0 で knn と同じ (現状互換)。paper は 20。
  // pca_neighbor_count > knn のとき、PCA は dists ソート後の先頭 pca_neighbor_count を使う。
  int pca_neighbor_count = 0;

  // Paper-aligned voxel insertion (Gap C). 単位 m。
  // 0 で無効 (kMaxPoints 上限まで append、現状)。paper は 0.1 m。
  // VoxelBlock::addPoint に min_distance_sq として渡される。
  double min_distance_between_points = 0.0;

  // Coarse-to-fine 2-phase schedule。
  // false で従来通り (全 outer iteration で同一 params)。
  // true で iter < coarse_iterations は coarse phase: 探索半径 +1
  // (3x3x3 → 5x5x5)、planarity を coarse_planarity_threshold まで緩める、
  // Cauchy σ を coarse_cauchy_sigma_mult 倍。残り iter は fine phase で現状 params。
  bool coarse_to_fine = false;
  int coarse_iterations = 3;
  int coarse_search_radius = 2;
  double coarse_planarity_threshold = 0.05;
  double coarse_cauchy_sigma_mult = 2.0;

  // Upstream mechanics (jedeschaud/ct_icp, config/odometry/driving_config.yaml).
  // Every default below keeps the historical behaviour.
  // insertPoints() のボクセル当たり上限 (upstream 30, VoxelBlock::kCapacity まで)。
  int max_points_per_voxel = VoxelBlock::kMaxPoints;
  // >0: 半径 [m] 以内の点だけを近傍候補にし、その中の最近傍 knn 点を必須とする
  // (upstream default_radius 0.75, 20 neighbors)。
  double neighbor_search_radius = 0.0;
  // reference = 選ばれた knn 点のうち最遠点。upstream は max-heap を先頭から
  // 取り出すため neighborhood.points[0] が最遠点になり、これを平面上の点に使う。
  bool farthest_neighbor_reference = false;
  // a2D を固有値ではなく標準偏差 (sqrt(固有値)) で計算する (upstream)。
  bool a2d_from_std = false;
  // >0: 近傍重み exp(-|reference - x| / scale) を使い、weight_alpha と
  // weight_neighborhood を和で正規化する (upstream: scale = 0.3 * 20 = 6 m)。
  double neighborhood_weight_scale = 0.0;
  // >0: キーポイント順で最初の N 個の対応だけを使う (upstream max_num_residuals 900)。
  int max_num_residuals = 0;
  // 収束判定を upstream 形式にする: begin/end の並進変化の和 < 0.01 m かつ
  // 回転変化の和 < 0.1 deg。
  bool upstream_convergence = false;
  // キーポイントのボクセル座標を floor ではなく static_cast<short>(p / size)
  // (0 方向への切り捨て) で求める (upstream sub_sample_frame)。
  bool truncate_voxel_coords = false;
};

struct CTICPResult {
  TrajectoryFrame frame;
  int num_iterations = 0;
  bool converged = false;
  int num_residuals = 0;  // 最後の反復で使った幾何残差数
};

/// CT-ICP レジストレーション
class CTICPRegistration {
public:
  explicit CTICPRegistration(const CTICPParams& params = CTICPParams())
      : params_(params) {}

  /// マップにフレームの点群を追加
  void addPointsToMap(const std::vector<Eigen::Vector3d>& world_points);

  /// レジストレーション実行
  /// @param timed_points タイムスタンプ付き点群 (センサ座標系)
  /// @param initial_frame 初期推定の軌跡フレーム
  /// @param previous_frame 前フレーム (正則化用, nullptr可)
  CTICPResult registerFrame(
      const std::vector<TimedPoint>& timed_points,
      const TrajectoryFrame& initial_frame,
      const TrajectoryFrame* previous_frame = nullptr);

  /// 点を 1 点ずつ永続マップへ直接追加する (upstream の挿入規則:
  /// ボクセル上限 max_points_per_voxel、min_distance_between_points)。
  /// スライディングウィンドウは使わない。
  void insertPoints(const std::vector<Eigen::Vector3d>& world_points);

  /// 先頭点が location から distance [m] より遠いボクセルを削除する。
  void removeVoxelsFarFrom(const Eigen::Vector3d& location, double distance);

  /// マップをクリア
  void clearMap();

  CTICPParams& mutableParams() { return params_; }

  const VoxelHashMap& map() const { return voxel_map_; }
  size_t mapSize() const { return voxel_map_.size(); }

  void setRegularizerNCap(int cap) { params_.regularizer_n_cap = cap; }

private:
  /// ボクセルキーポイントにダウンサンプリング
  std::vector<TimedPoint> subsampleKeypoints(
      const std::vector<TimedPoint>& points) const;

  /// 1フレーム分の点群をボクセル化
  VoxelHashMap buildFrameMap(
      const std::vector<Eigen::Vector3d>& world_points) const;

  /// スライディングウィンドウから集約マップを再構築
  void rebuildMapFromWindow();

  /// 近傍探索して法線を計算
  struct Correspondence {
    int point_idx;
    Eigen::Vector3d reference;
    Eigen::Vector3d normal;
    double weight;
  };

  std::vector<Correspondence> findCorrespondences(
      const std::vector<TimedPoint>& keypoints,
      const TrajectoryFrame& frame,
      int outer_iter = 0) const;

  CTICPParams params_;
  VoxelHashMap voxel_map_;
  std::deque<VoxelHashMap> frame_maps_;
};

}  // namespace ct_icp
}  // namespace localization_zoo
