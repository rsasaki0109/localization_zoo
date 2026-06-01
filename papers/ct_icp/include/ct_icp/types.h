#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <unordered_map>
#include <vector>
#include <functional>

namespace localization_zoo {
namespace ct_icp {

/// タイムスタンプ付き3D点
struct TimedPoint {
  Eigen::Vector3d raw_point = Eigen::Vector3d::Zero();
  Eigen::Vector3d world_point = Eigen::Vector3d::Zero();
  double timestamp = 0.0;  // 相対タイムスタンプ [0, 1]
  double alpha = 0.0;      // 補間係数
};

/// SE3変換 (四元数 + 並進)
struct SE3 {
  Eigen::Quaterniond quat = Eigen::Quaterniond::Identity();
  Eigen::Vector3d trans = Eigen::Vector3d::Zero();

  /// 点を変換
  Eigen::Vector3d operator*(const Eigen::Vector3d& p) const {
    return quat * p + trans;
  }

  /// SE3合成
  SE3 operator*(const SE3& rhs) const {
    SE3 result;
    result.quat = quat * rhs.quat;
    result.trans = quat * rhs.trans + trans;
    return result;
  }

  /// 逆変換
  SE3 inverse() const {
    SE3 result;
    result.quat = quat.conjugate();
    result.trans = -(result.quat * trans);
    return result;
  }

  /// SLERP + 線形補間
  static SE3 interpolate(const SE3& a, const SE3& b, double alpha) {
    SE3 result;
    result.quat = a.quat.slerp(alpha, b.quat);
    result.trans = (1.0 - alpha) * a.trans + alpha * b.trans;
    return result;
  }
};

/// 1フレームの軌跡 (begin + end の2ポーズ)
struct TrajectoryFrame {
  SE3 begin_pose;
  SE3 end_pose;
  double begin_timestamp = 0.0;
  double end_timestamp = 1.0;

  /// 補間係数を計算
  double getAlpha(double timestamp) const {
    if (end_timestamp <= begin_timestamp) return 1.0;
    return (timestamp - begin_timestamp) / (end_timestamp - begin_timestamp);
  }

  /// タイムスタンプに応じた補間ポーズで点を変換
  Eigen::Vector3d transformPoint(const Eigen::Vector3d& raw_point,
                                  double timestamp) const {
    double alpha = getAlpha(timestamp);
    SE3 pose = SE3::interpolate(begin_pose, end_pose, alpha);
    return pose * raw_point;
  }
};

/// 3Dボクセル座標
struct Voxel {
  int x, y, z;

  bool operator==(const Voxel& other) const {
    return x == other.x && y == other.y && z == other.z;
  }
};

struct VoxelHash {
  std::size_t operator()(const Voxel& v) const {
    std::size_t seed = 0;
    auto hc = [&seed](int val) {
      seed ^= std::hash<int>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    hc(v.x);
    hc(v.y);
    hc(v.z);
    return seed;
  }
};

/// ボクセルブロック (ボクセル内の点群を保持)
struct VoxelBlock {
  // 20 を維持。paper は 30 だが、KITTI 07 実測で 30 にすると ms_chol baseline が
  // 1.61 -> 3.15m (+96%) と悪化することを確認 (2026-05-18)。点群密度が上がると
  // mean-of-knn reference が distort される副作用が大きい。Gap C
  // (min_distance_between_points) の方が voxel 内 redundancy を制御する適切な
  // mechanism。
  static constexpr int kMaxPoints = 20;
  Eigen::Vector3d points[kMaxPoints];
  int num_points = 0;

  // min_distance_sq > 0 で min-distance enforcement。同 voxel 内に既に
  // 距離未満の点があれば追加せず false を返す。0 だと従来通り (kMaxPoints 上限のみ)。
  // paper は 0.1 m (= min_distance_sq = 0.01)。
  bool addPoint(const Eigen::Vector3d& point, double min_distance_sq = 0.0) {
    if (min_distance_sq > 0.0) {
      for (int i = 0; i < num_points; ++i) {
        if ((points[i] - point).squaredNorm() < min_distance_sq) return false;
      }
    }
    if (num_points < kMaxPoints) {
      points[num_points++] = point;
      return true;
    }
    return false;
  }
};

/// ボクセルハッシュマップ
using VoxelHashMap = std::unordered_map<Voxel, VoxelBlock, VoxelHash>;

}  // namespace ct_icp
}  // namespace localization_zoo
