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
};

struct CTICPResult {
  TrajectoryFrame frame;
  int num_iterations = 0;
  bool converged = false;
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

  /// マップをクリア
  void clearMap();

  const VoxelHashMap& map() const { return voxel_map_; }
  size_t mapSize() const { return voxel_map_.size(); }

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
      const TrajectoryFrame& frame) const;

  CTICPParams params_;
  VoxelHashMap voxel_map_;
  std::deque<VoxelHashMap> frame_maps_;
};

}  // namespace ct_icp
}  // namespace localization_zoo
