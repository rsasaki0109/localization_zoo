#pragma once

#include "aloam/types.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/kdtree/kdtree_flann.h>

#include <array>

namespace localization_zoo {
namespace aloam {

struct LaserMappingParams {
  double line_resolution = 0.4;      // エッジマップのボクセルサイズ [m]
  double plane_resolution = 0.8;     // 平面マップのボクセルサイズ [m]
  double cube_size = 50.0;           // キューブのサイズ [m]
  int cube_width = 21;
  int cube_height = 21;
  int cube_depth = 11;
  int num_optimization_iters = 2;
  int ceres_max_iterations = 4;
  double huber_loss_s = 0.1;
  int knn = 5;                       // 近傍点数
  double knn_max_dist_sq = 1.0;      // 近傍点の最大距離² [m²]
  double edge_eigenvalue_ratio = 3.0;  // 直線判定の固有値比
  double plane_threshold = 0.2;       // 平面判定の残差閾値
};

struct MappingResult {
  Eigen::Quaterniond q_w_curr = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr = Eigen::Vector3d::Zero();
  bool valid = false;
};

/// Scan-to-Map マッチング
class LaserMapping {
public:
  explicit LaserMapping(const LaserMappingParams& params =
                            LaserMappingParams());

  /// オドメトリ結果と特徴点群からマップマッチングを実行
  MappingResult process(const PointCloudPtr& corner_last,
                        const PointCloudPtr& surf_last,
                        const PointCloudPtr& full_res,
                        const Eigen::Quaterniond& q_odom,
                        const Eigen::Vector3d& t_odom);

  const Eigen::Quaterniond& orientation() const { return q_w_curr_; }
  const Eigen::Vector3d& position() const { return t_w_curr_; }

private:
  int toIndex(int i, int j, int k) const;
  void shiftCubeGrid();
  void extractSurroundingKeyFrames();
  void optimizeTransform(const PointCloudPtr& corner_from_map,
                         const PointCloudPtr& surf_from_map,
                         const PointCloudPtr& corner_last,
                         const PointCloudPtr& surf_last);
  void updateMap(const PointCloudPtr& corner_last,
                 const PointCloudPtr& surf_last);

  LaserMappingParams params_;

  // キューブグリッドマップ
  static constexpr int kMaxCubes = 21 * 21 * 11;
  std::array<PointCloudPtr, kMaxCubes> corner_cube_array_;
  std::array<PointCloudPtr, kMaxCubes> surf_cube_array_;

  // 中心キューブインデックス
  int center_cube_i_, center_cube_j_, center_cube_k_;

  // マップ座標系のポーズ
  Eigen::Quaterniond q_w_curr_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_w_curr_ = Eigen::Vector3d::Zero();

  // Odom→Map 補正
  Eigen::Quaterniond q_wmap_wodom_ = Eigen::Quaterniond::Identity();
  Eigen::Vector3d t_wmap_wodom_ = Eigen::Vector3d::Zero();

  // 周辺キューブの点群
  PointCloudPtr corner_from_map_;
  PointCloudPtr surf_from_map_;
  std::vector<int> surrounding_cube_ids_;

  bool initialized_ = false;
};

}  // namespace aloam
}  // namespace localization_zoo
