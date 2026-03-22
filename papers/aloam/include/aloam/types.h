#pragma once

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <vector>

namespace localization_zoo {
namespace aloam {

using PointT = pcl::PointXYZI;
using PointCloud = pcl::PointCloud<PointT>;
using PointCloudPtr = PointCloud::Ptr;
using PointCloudConstPtr = PointCloud::ConstPtr;

/// 特徴抽出の結果
struct FeatureCloud {
  PointCloudPtr corner_sharp;       // エッジ特徴 (sharp)
  PointCloudPtr corner_less_sharp;  // エッジ特徴 (less sharp)
  PointCloudPtr surf_flat;          // 平面特徴 (flat)
  PointCloudPtr surf_less_flat;     // 平面特徴 (less flat)
  PointCloudPtr full_cloud;         // 全点群 (タイムスタンプ付き)

  FeatureCloud()
      : corner_sharp(new PointCloud),
        corner_less_sharp(new PointCloud),
        surf_flat(new PointCloud),
        surf_less_flat(new PointCloud),
        full_cloud(new PointCloud) {}
};

/// LiDARの点情報 (スキャンラインID + 相対時刻を分離)
struct AnnotatedPoint {
  float x, y, z;
  int scan_id;        // スキャンラインID
  float rel_time;     // スキャン内相対時刻 [0, 1]
  float curvature;    // 曲率
};

}  // namespace aloam
}  // namespace localization_zoo
