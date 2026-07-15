#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cstddef>
#include <deque>
#include <vector>

namespace localization_zoo {
namespace dilo {

/// 球面レンジ画像 (Spherical Range Image, SRI)。点群を (仰角→行, 方位角→列) で
/// 2D 画像に投影し、画素ごとに最近 range・3D 点・面法線を保持する。投影による
/// データ対応付け (projective data association) と画像ベース法線推定に使う。
class SphericalRangeImage {
public:
  struct Params {
    int height = 64;          // 行数 (リング/仰角ビン)
    int width = 900;          // 列数 (方位角ビン)
    double fov_up_deg = 2.0;  // 仰角上限
    double fov_down_deg = -24.8;  // 仰角下限 (KITTI HDL-64E 既定)
    double min_range = 1.0;
    double max_range = 100.0;
    int lookup_radius = 1;    // projective association の近傍探索半径 [px]
    double max_lookup_point_distance = 1.5;  // 近傍画素候補の 3D 距離 gate [m]
  };

  struct Pixel {
    Eigen::Vector3d point = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal = Eigen::Vector3d::Zero();
    double range = 0.0;
    bool valid = false;
    bool has_normal = false;
  };

  SphericalRangeImage() = default;

  /// 点群から SRI を構築 (各画素は最近 range の点を採用)。法線も計算する。
  void build(const std::vector<Eigen::Vector3d>& points, const Params& params);

  /// 3D 点 (この SRI の座標系) を画素に投影し対応を返す。範囲外/無効なら valid=false。
  Pixel lookup(const Eigen::Vector3d& p) const;

  bool empty() const { return pixels_.empty(); }
  const Params& params() const { return params_; }

private:
  bool project(const Eigen::Vector3d& p, int* row, int* col) const;
  void computeNormals();

  Params params_;
  std::vector<Pixel> pixels_;  // height*width
  int idx(int r, int c) const { return r * params_.width + c; }
};

struct DiloParams {
  double max_range = 100.0;
  double min_range = 1.0;
  double source_voxel_size = 0.0;  // >0 で GN 用ソース点を間引く
  int sri_height = 64;
  int sri_width = 900;
  double fov_up_deg = 2.0;
  double fov_down_deg = -24.8;
  int lookup_radius = 1;
  double max_lookup_point_distance = 1.5;
  int max_iterations = 30;
  double convergence_criterion = 0.001;
  double initial_threshold = 1.0;  // point-to-plane 残差の外れ値しきい値 (m)
  double robust_scale = 0.5;       // Cauchy 核スケール
  double planarity_min = 0.0;      // 法線有効性のみ (将来拡張用)
  /// キーフレーム切替しきい値 (これを超える相対運動でキーフレーム更新)。
  double keyframe_translation = 2.0;
  double keyframe_rotation_deg = 10.0;
};

struct DiloResult {
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
  bool converged = false;
  int iterations = 0;
  int num_correspondences = 0;
  bool keyframe_updated = false;
};

/// DiLO: Direct LiDAR Odometry based on spherical range images
/// (Han et al., ETRI Journal 2021, doi:10.4218/etrij.2021-0088) の from-paper
/// 再実装。著者コード未公開。
///
///  明示的な最近傍探索 (kd-tree/voxel) を使わず、現在スキャン点をキーフレームの
///  球面レンジ画像 (SRI) へ投影して対応を得る (projective data association)。各対応で
///  画像ベース法線による point-to-plane 残差 e=n·(p'-q) を計算し、相対姿勢を
///  Gauss-Newton で直接 (direct) 最適化する。相対運動がしきい値を超えると現在
///  スキャンを新キーフレームとする (frame-to-keyframe direct odometry)。
///
/// 純 LiDAR scope。IMU 緩結合は範囲外 (KITTI は IMU 無し→等速予測)。
class DiloPipeline {
public:
  explicit DiloPipeline(const DiloParams& params = DiloParams());

  void setInitialPose(const Eigen::Matrix4d& pose);
  DiloResult registerFrame(const std::vector<Eigen::Vector3d>& frame);

  const Eigen::Matrix4d& pose() const { return pose_; }

private:
  std::vector<Eigen::Vector3d> prepare(
      const std::vector<Eigen::Vector3d>& points) const;
  /// 現在点群をキーフレーム SRI へ direct alignment し相対姿勢 T_rel を返す。
  Eigen::Matrix4d align(const std::vector<Eigen::Vector3d>& source,
                        const Eigen::Matrix4d& init_rel, DiloResult* result);

  DiloParams params_;
  SphericalRangeImage keyframe_sri_;
  Eigen::Matrix4d pose_ = Eigen::Matrix4d::Identity();           // 現在の世界姿勢
  Eigen::Matrix4d keyframe_pose_ = Eigen::Matrix4d::Identity();  // キーフレーム世界姿勢
  Eigen::Matrix4d last_delta_ = Eigen::Matrix4d::Identity();     // 等速予測用
  int frame_count_ = 0;
};

}  // namespace dilo
}  // namespace localization_zoo
