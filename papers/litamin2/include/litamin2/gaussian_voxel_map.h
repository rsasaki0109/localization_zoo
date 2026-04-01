#pragma once

#include <Eigen/Core>
#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

namespace localization_zoo {
namespace litamin2 {

/// 3D整数座標のハッシュ
struct Vector3iHash {
  std::size_t operator()(const Eigen::Vector3i& v) const {
    std::size_t seed = 0;
    auto hash_combine = [&seed](int val) {
      seed ^= std::hash<int>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    hash_combine(v.x());
    hash_combine(v.y());
    hash_combine(v.z());
    return seed;
  }
};

/// ボクセル内の正規分布を表す構造体
struct GaussianVoxel {
  int num_points = 0;
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();

  void addPoint(const Eigen::Vector3d& point) {
    num_points++;
    mean += point;
  }

  void addPointWithCov(const Eigen::Vector3d& point_mean,
                       const Eigen::Matrix3d& point_cov) {
    num_points++;
    mean += point_mean;
    cov += point_cov;
  }

  /// ボクセル内の全点を使って正規分布を確定
  void finalize() {
    if (num_points <= 0) return;
    mean /= num_points;
    cov /= num_points;
  }
};

/// 正規分布ボクセルマップ
class GaussianVoxelMap {
public:
  explicit GaussianVoxelMap(double resolution, int min_points_per_voxel = 3)
      : resolution_(resolution),
        min_points_per_voxel_(min_points_per_voxel) {}

  /// 点群と各点の共分散からボクセルマップを構築
  void create(const std::vector<Eigen::Vector3d>& points,
              const std::vector<Eigen::Matrix3d>& covariances) {
    voxels_.clear();

    for (size_t i = 0; i < points.size(); i++) {
      Eigen::Vector3i coord = pointToVoxel(points[i]);
      auto it = voxels_.find(coord);
      if (it == voxels_.end()) {
        auto voxel = std::make_shared<GaussianVoxel>();
        voxel->addPointWithCov(points[i], covariances[i]);
        voxels_[coord] = voxel;
      } else {
        it->second->addPointWithCov(points[i], covariances[i]);
      }
    }

    // finalize & 点数が少ないボクセルを除去
    for (auto it = voxels_.begin(); it != voxels_.end();) {
      it->second->finalize();
      if (it->second->num_points < min_points_per_voxel_) {
        it = voxels_.erase(it);
      } else {
        ++it;
      }
    }
  }

  /// 点群から直接ボクセルマップを構築 (各ボクセル内のサンプル共分散を計算)
  void createFromPoints(const std::vector<Eigen::Vector3d>& points,
                        int num_threads = 1) {
    // まずボクセルごとに点を集める
    std::unordered_map<Eigen::Vector3i, std::vector<int>, Vector3iHash>
        voxel_points;

    for (size_t i = 0; i < points.size(); i++) {
      Eigen::Vector3i coord = pointToVoxel(points[i]);
      voxel_points[coord].push_back(i);
    }

    voxels_.clear();
    std::vector<std::pair<Eigen::Vector3i, std::vector<int>>> voxel_entries;
    voxel_entries.reserve(voxel_points.size());
    for (auto& [coord, indices] : voxel_points) {
      voxel_entries.emplace_back(coord, std::move(indices));
    }

    std::vector<std::shared_ptr<GaussianVoxel>> built_voxels(voxel_entries.size());
    const int thread_count = std::max(1, num_threads);

#ifdef _OPENMP
#pragma omp parallel for num_threads(thread_count) if (thread_count > 1)
#endif
    for (int entry_idx = 0; entry_idx < static_cast<int>(voxel_entries.size());
         entry_idx++) {
      const auto& indices = voxel_entries[entry_idx].second;
      if (static_cast<int>(indices.size()) < min_points_per_voxel_) {
        continue;
      }

      auto voxel = std::make_shared<GaussianVoxel>();
      voxel->num_points = indices.size();

      Eigen::Vector3d sum = Eigen::Vector3d::Zero();
      for (int idx : indices) {
        sum += points[idx];
      }
      voxel->mean = sum / indices.size();

      Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
      for (int idx : indices) {
        const Eigen::Vector3d diff = points[idx] - voxel->mean;
        cov += diff * diff.transpose();
      }
      cov /= indices.size();

      Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
      Eigen::Vector3d eigenvalues = solver.eigenvalues();
      constexpr double kMinEigenvalue = 1e-3;
      for (int axis = 0; axis < 3; axis++) {
        eigenvalues(axis) = std::max(eigenvalues(axis), kMinEigenvalue);
      }
      voxel->cov = solver.eigenvectors() * eigenvalues.asDiagonal() *
                   solver.eigenvectors().transpose();
      built_voxels[entry_idx] = voxel;
    }

    for (size_t entry_idx = 0; entry_idx < voxel_entries.size(); entry_idx++) {
      if (built_voxels[entry_idx]) {
        voxels_[voxel_entries[entry_idx].first] = built_voxels[entry_idx];
      }
    }
  }

  /// 座標からボクセルを検索
  std::shared_ptr<GaussianVoxel> lookup(const Eigen::Vector3d& point) const {
    Eigen::Vector3i coord = pointToVoxel(point);
    auto it = voxels_.find(coord);
    if (it != voxels_.end()) {
      return it->second;
    }
    return nullptr;
  }

  /// 全ボクセルのイテレーション用
  const auto& voxels() const { return voxels_; }

  double resolution() const { return resolution_; }

  size_t size() const { return voxels_.size(); }

private:
  Eigen::Vector3i pointToVoxel(const Eigen::Vector3d& point) const {
    return Eigen::Vector3i(
        static_cast<int>(std::floor(point.x() / resolution_)),
        static_cast<int>(std::floor(point.y() / resolution_)),
        static_cast<int>(std::floor(point.z() / resolution_)));
  }

  double resolution_;
  int min_points_per_voxel_;
  std::unordered_map<Eigen::Vector3i, std::shared_ptr<GaussianVoxel>,
                     Vector3iHash>
      voxels_;
};

}  // namespace litamin2
}  // namespace localization_zoo
