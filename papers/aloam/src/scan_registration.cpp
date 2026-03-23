#include "aloam/scan_registration.h"

#include <pcl/filters/voxel_grid.h>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace localization_zoo {
namespace aloam {

int ScanRegistration::computeScanId(float x, float y, float z) const {
  float vertical_angle =
      std::atan2(z, std::sqrt(x * x + y * y)) * 180.0f / M_PI;

  int scan_id = 0;
  if (params_.n_scans == 16) {
    scan_id = static_cast<int>(vertical_angle + 15.0f) / 2 + 0.5f;
    if (scan_id > (params_.n_scans - 1) || scan_id < 0) return -1;
  } else if (params_.n_scans == 32) {
    scan_id = static_cast<int>((vertical_angle + 92.0f / 3.0f) * 3.0f / 4.0f);
    if (scan_id > (params_.n_scans - 1) || scan_id < 0) return -1;
  } else if (params_.n_scans == 64) {
    if (vertical_angle >= -8.83f) {
      scan_id = static_cast<int>((2.0f - vertical_angle) * 3.0f + 0.5f);
    } else {
      scan_id = params_.n_scans / 2 +
                static_cast<int>((-8.83f - vertical_angle) * 2.0f + 0.5f);
    }
    if (scan_id > 63 || scan_id < 0) return -1;
  } else {
    // 等間隔を仮定
    float fov = 30.0f;  // -15 ~ +15
    scan_id = static_cast<int>((vertical_angle + fov / 2.0f) /
                               (fov / params_.n_scans));
    if (scan_id > (params_.n_scans - 1) || scan_id < 0) return -1;
  }
  return scan_id;
}

FeatureCloud ScanRegistration::extract(const PointCloudConstPtr& cloud) {
  FeatureCloud result;

  // 1. NaN除去 + 近距離除去 + スキャンラインID付与
  std::vector<std::vector<int>> scan_indices(params_.n_scans);

  std::vector<AnnotatedPoint> annotated;
  annotated.reserve(cloud->size());

  for (size_t i = 0; i < cloud->size(); i++) {
    const auto& p = cloud->points[i];
    if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z))
      continue;

    float range = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    if (range < params_.minimum_range) continue;

    int scan_id = computeScanId(p.x, p.y, p.z);
    if (scan_id < 0 || scan_id >= params_.n_scans) continue;

    AnnotatedPoint ap;
    ap.x = p.x;
    ap.y = p.y;
    ap.z = p.z;
    ap.scan_id = scan_id;
    ap.curvature = 0.0f;

    // 水平角から相対時刻を計算
    float ori = -std::atan2(p.y, p.x);
    ap.rel_time = 0.0f;  // 後で計算

    scan_indices[scan_id].push_back(annotated.size());
    annotated.push_back(ap);
  }

  // 各スキャンラインを角度でソートし、相対時刻を計算
  for (int s = 0; s < params_.n_scans; s++) {
    auto& indices = scan_indices[s];
    if (indices.empty()) continue;

    // 角度でソート
    std::sort(indices.begin(), indices.end(),
              [&annotated](int a, int b) {
                float ori_a = -std::atan2(annotated[a].y, annotated[a].x);
                float ori_b = -std::atan2(annotated[b].y, annotated[b].x);
                return ori_a < ori_b;
              });

    // 相対時刻を割り当て [0, 1]
    int n = indices.size();
    for (int i = 0; i < n; i++) {
      annotated[indices[i]].rel_time = static_cast<float>(i) / n;
    }
  }

  // 2. 連結配列を構築 (スキャンライン順)
  std::vector<int> all_indices;
  std::vector<std::pair<int, int>> scan_start_end;  // (start, end) in all_indices

  for (int s = 0; s < params_.n_scans; s++) {
    int start = all_indices.size();
    for (int idx : scan_indices[s]) {
      all_indices.push_back(idx);
    }
    int end = all_indices.size();
    scan_start_end.emplace_back(start, end);
  }

  int cloud_size = all_indices.size();
  if (cloud_size < 20) return result;

  // 3. 曲率計算 (前後5点)
  for (int i = 5; i < cloud_size - 5; i++) {
    float diff_x = 0, diff_y = 0, diff_z = 0;
    for (int j = -5; j <= 5; j++) {
      if (j == 0) continue;
      int idx = all_indices[i + j];
      int idx_i = all_indices[i];
      diff_x += annotated[idx].x - annotated[idx_i].x;
      diff_y += annotated[idx].y - annotated[idx_i].y;
      diff_z += annotated[idx].z - annotated[idx_i].z;
    }
    annotated[all_indices[i]].curvature =
        diff_x * diff_x + diff_y * diff_y + diff_z * diff_z;
  }

  // 4. 特徴選択
  std::vector<int> label(annotated.size(), 0);  // 0=未分類, 2=sharp, 1=lessSharp, -1=flat
  std::vector<bool> neighbor_picked(annotated.size(), false);

  // 各スキャンラインの前後5点をpicked扱い (境界)
  for (int s = 0; s < params_.n_scans; s++) {
    auto [start, end] = scan_start_end[s];
    int n = end - start;
    if (n < 12) continue;

    for (int k = 0; k < 5; k++) {
      neighbor_picked[all_indices[start + k]] = true;
      neighbor_picked[all_indices[end - 1 - k]] = true;
    }
  }

  for (int s = 0; s < params_.n_scans; s++) {
    auto [start, end] = scan_start_end[s];
    int n = end - start;
    if (n < 12) continue;

    // 6サブリージョンに分割
    for (int sub = 0; sub < 6; sub++) {
      int sp = start + (n * sub) / 6;
      int ep = start + (n * (sub + 1)) / 6 - 1;

      // 曲率でインデックスをソート
      std::vector<int> sorted_idx;
      for (int k = sp; k <= ep; k++) {
        sorted_idx.push_back(k);
      }
      std::sort(sorted_idx.begin(), sorted_idx.end(),
                [&](int a, int b) {
                  return annotated[all_indices[a]].curvature >
                         annotated[all_indices[b]].curvature;
                });

      // エッジ特徴 (曲率大)
      int corner_count = 0;
      for (int k : sorted_idx) {
        int idx = all_indices[k];
        if (!neighbor_picked[idx] &&
            annotated[idx].curvature > params_.curvature_threshold) {
          corner_count++;

          if (corner_count <= params_.max_corner_sharp) {
            label[idx] = 2;
            PointT p;
            p.x = annotated[idx].x;
            p.y = annotated[idx].y;
            p.z = annotated[idx].z;
            p.intensity = annotated[idx].scan_id +
                          params_.scan_period * annotated[idx].rel_time;
            result.corner_sharp->push_back(p);
            result.corner_less_sharp->push_back(p);
          } else if (corner_count <= params_.max_corner_less_sharp) {
            label[idx] = 1;
            PointT p;
            p.x = annotated[idx].x;
            p.y = annotated[idx].y;
            p.z = annotated[idx].z;
            p.intensity = annotated[idx].scan_id +
                          params_.scan_period * annotated[idx].rel_time;
            result.corner_less_sharp->push_back(p);
          } else {
            break;
          }

          // 近傍をpicked
          neighbor_picked[idx] = true;
          for (int l = 1; l <= 5 && (k + l) <= ep; l++) {
            int nidx = all_indices[k + l];
            float dx = annotated[nidx].x - annotated[idx].x;
            float dy = annotated[nidx].y - annotated[idx].y;
            float dz = annotated[nidx].z - annotated[idx].z;
            if (dx * dx + dy * dy + dz * dz > 0.05f) break;
            neighbor_picked[nidx] = true;
          }
          for (int l = -1; l >= -5 && (k + l) >= sp; l--) {
            int nidx = all_indices[k + l];
            float dx = annotated[nidx].x - annotated[idx].x;
            float dy = annotated[nidx].y - annotated[idx].y;
            float dz = annotated[nidx].z - annotated[idx].z;
            if (dx * dx + dy * dy + dz * dz > 0.05f) break;
            neighbor_picked[nidx] = true;
          }
        }
      }

      // 平面特徴 (曲率小) - ソート順を逆に
      int flat_count = 0;
      for (auto it = sorted_idx.rbegin(); it != sorted_idx.rend(); ++it) {
        int k = *it;
        int idx = all_indices[k];
        if (!neighbor_picked[idx] &&
            annotated[idx].curvature < params_.curvature_threshold) {
          label[idx] = -1;
          flat_count++;

          PointT p;
          p.x = annotated[idx].x;
          p.y = annotated[idx].y;
          p.z = annotated[idx].z;
          p.intensity = annotated[idx].scan_id +
                        params_.scan_period * annotated[idx].rel_time;
          result.surf_flat->push_back(p);

          if (flat_count >= params_.max_surf_flat) break;

          neighbor_picked[idx] = true;
          for (int l = 1; l <= 5 && (k + l) <= ep; l++) {
            int nidx = all_indices[k + l];
            float dx = annotated[nidx].x - annotated[idx].x;
            float dy = annotated[nidx].y - annotated[idx].y;
            float dz = annotated[nidx].z - annotated[idx].z;
            if (dx * dx + dy * dy + dz * dz > 0.05f) break;
            neighbor_picked[nidx] = true;
          }
          for (int l = -1; l >= -5 && (k + l) >= sp; l--) {
            int nidx = all_indices[k + l];
            float dx = annotated[nidx].x - annotated[idx].x;
            float dy = annotated[nidx].y - annotated[idx].y;
            float dz = annotated[nidx].z - annotated[idx].z;
            if (dx * dx + dy * dy + dz * dz > 0.05f) break;
            neighbor_picked[nidx] = true;
          }
        }
      }
    }
  }

  // LessFlat = label <= 0 の全点をダウンサンプリング
  PointCloudPtr surf_less_flat_scan(new PointCloud);
  for (int s = 0; s < params_.n_scans; s++) {
    auto [start, end] = scan_start_end[s];
    for (int k = start; k < end; k++) {
      int idx = all_indices[k];
      if (label[idx] <= 0) {
        PointT p;
        p.x = annotated[idx].x;
        p.y = annotated[idx].y;
        p.z = annotated[idx].z;
        p.intensity = annotated[idx].scan_id +
                      params_.scan_period * annotated[idx].rel_time;
        surf_less_flat_scan->push_back(p);
      }
    }
  }

  pcl::VoxelGrid<PointT> downsize;
  downsize.setInputCloud(surf_less_flat_scan);
  downsize.setLeafSize(params_.less_flat_leaf_size, params_.less_flat_leaf_size,
                       params_.less_flat_leaf_size);
  downsize.filter(*result.surf_less_flat);

  // 全点群
  for (size_t i = 0; i < annotated.size(); i++) {
    PointT p;
    p.x = annotated[i].x;
    p.y = annotated[i].y;
    p.z = annotated[i].z;
    p.intensity =
        annotated[i].scan_id + params_.scan_period * annotated[i].rel_time;
    result.full_cloud->push_back(p);
  }

  return result;
}

}  // namespace aloam
}  // namespace localization_zoo
