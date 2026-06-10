#include "mesh_loam/mesh_loam.h"

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>

namespace localization_zoo {
namespace mesh_loam {

namespace {

Eigen::Matrix3d expSo3(const Eigen::Vector3d& w) {
  const double theta = w.norm();
  if (theta < 1e-12) return Eigen::Matrix3d::Identity();
  return Eigen::AngleAxisd(theta, w / theta).toRotationMatrix();
}

}  // namespace

MeshLoamPipeline::MeshLoamPipeline(const MeshLoamParams& params) : params_(params) {}

void MeshLoamPipeline::reset() {
  frame_count_ = 0;
  pose_ = Eigen::Matrix4d::Identity();
  prev_pose_ = Eigen::Matrix4d::Identity();
  has_prev_ = false;
  voxels_.clear();
  block_facets_.clear();
  dirty_blocks_.clear();
  facets_.clear();
  facet_alive_.clear();
  facet_bins_.clear();
}

Eigen::Vector3i MeshLoamPipeline::toVoxel(const Eigen::Vector3d& p) const {
  const double r = params_.voxel_size;
  return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / r)),
                         static_cast<int>(std::floor(p.y() / r)),
                         static_cast<int>(std::floor(p.z() / r)));
}

Eigen::Vector3i MeshLoamPipeline::toBlock(const Eigen::Vector3i& voxel) const {
  const int b = params_.block_size;
  auto div = [b](int v) { return static_cast<int>(std::floor(static_cast<double>(v) / b)); };
  return Eigen::Vector3i(div(voxel.x()), div(voxel.y()), div(voxel.z()));
}

Eigen::Vector3i MeshLoamPipeline::toFacetCell(const Eigen::Vector3d& p) const {
  const double c = params_.search_radius;
  return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / c)),
                         static_cast<int>(std::floor(p.y() / c)),
                         static_cast<int>(std::floor(p.z() / c)));
}

Eigen::Vector3d MeshLoamPipeline::voxelCenter(const Eigen::Vector3i& v) const {
  const double r = params_.voxel_size;
  return Eigen::Vector3d((v.x() + 0.5) * r, (v.y() + 0.5) * r, (v.z() + 0.5) * r);
}

std::vector<MeshLoamPipeline::OrientedPoint> MeshLoamPipeline::preprocess(
    const std::vector<Eigen::Vector3d>& points) const {
  std::vector<OrientedPoint> out;
  out.reserve(points.size());
  std::vector<Eigen::Vector3d> kept;
  kept.reserve(points.size());
  for (const auto& p : points) {
    const double r = p.norm();
    if (!std::isfinite(r) || r < params_.min_range || r > params_.max_range) continue;
    kept.push_back(p);
  }
  if (kept.size() < static_cast<size_t>(params_.normal_neighbors + 1)) return out;

  // Neighbor grid for kNN PCA normals.
  const double cell = params_.normal_search_radius;
  std::unordered_map<Eigen::Vector3i, std::vector<size_t>, VoxelKeyHash, VoxelKeyEq> grid;
  grid.reserve(kept.size());
  auto cellOf = [cell](const Eigen::Vector3d& p) {
    return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / cell)),
                           static_cast<int>(std::floor(p.y() / cell)),
                           static_cast<int>(std::floor(p.z() / cell)));
  };
  for (size_t i = 0; i < kept.size(); ++i) grid[cellOf(kept[i])].push_back(i);

  const int k = std::max(5, params_.normal_neighbors);
  std::vector<std::pair<double, size_t>> nn;
  for (size_t i = 0; i < kept.size(); ++i) {
    const Eigen::Vector3d& p = kept[i];
    const Eigen::Vector3i c = cellOf(p);
    nn.clear();
    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dz = -1; dz <= 1; ++dz) {
          const auto it = grid.find(Eigen::Vector3i(c.x() + dx, c.y() + dy, c.z() + dz));
          if (it == grid.end()) continue;
          for (const size_t j : it->second) {
            if (j == i) continue;
            nn.emplace_back((kept[j] - p).squaredNorm(), j);
          }
        }
      }
    }
    OrientedPoint op;
    op.p = p;
    if (static_cast<int>(nn.size()) >= k) {
      std::partial_sort(nn.begin(), nn.begin() + k, nn.end());
      Eigen::Vector3d mean = p;
      for (int j = 0; j < k; ++j) mean += kept[nn[static_cast<size_t>(j)].second];
      mean /= static_cast<double>(k + 1);
      Eigen::Matrix3d cov = (p - mean) * (p - mean).transpose();
      for (int j = 0; j < k; ++j) {
        const Eigen::Vector3d d = kept[nn[static_cast<size_t>(j)].second] - mean;
        cov += d * d.transpose();
      }
      cov /= static_cast<double>(k + 1);
      Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
      const Eigen::Vector3d evals = es.eigenvalues();
      Eigen::Vector3d n = es.eigenvectors().col(0);
      const double sum = evals.sum();
      op.curvature = sum > 1e-12 ? evals(0) / sum : 1.0;
      // Orient toward the sensor (origin in the local frame) so that the SDF
      // is positive on the observed free-space side.
      if (n.dot(-p) < 0.0) n = -n;
      op.n = n;
      op.valid = true;
    }
    out.push_back(op);
  }
  return out;
}

void MeshLoamPipeline::integrateScan(const std::vector<OrientedPoint>& points_world) {
  const int r = params_.influence_radius;
  for (const auto& op : points_world) {
    if (!op.valid) continue;
    const Eigen::Vector3i center = toVoxel(op.p);
    for (int dx = -r; dx <= r; ++dx) {
      for (int dy = -r; dy <= r; ++dy) {
        for (int dz = -r; dz <= r; ++dz) {
          const Eigen::Vector3i key(center.x() + dx, center.y() + dy, center.z() + dz);
          const Eigen::Vector3d c = voxelCenter(key);
          auto& voxel = voxels_[key];
          const double dist2 = (c - op.p).squaredNorm();
          double w = std::exp(-dist2 / params_.imls_h);
          // Hybrid weight (Eq. 8); the dot is clamped at zero because the
          // paper leaves negative normal agreement unspecified and negative
          // weights destabilize the running average (Eq. 9).
          if (voxel.weight > 0.0) {
            w += params_.lambda_n * std::max(0.0, op.n.dot(voxel.normal));
          } else {
            w += params_.lambda_n;
          }
          const double d = op.n.dot(c - op.p);
          voxel.sdf += w * (d - voxel.sdf) / (voxel.weight + w);
          Eigen::Vector3d n_new = voxel.weight * voxel.normal + w * op.n;
          const double len = n_new.norm();
          voxel.normal = len > 1e-12 ? (n_new / len).eval() : op.n;
          voxel.weight += w;
          dirty_blocks_[toBlock(key)] = true;
        }
      }
    }
  }
}

bool MeshLoamPipeline::sampleSdf(const Eigen::Vector3i& v, double* sdf) const {
  const auto it = voxels_.find(v);
  if (it == voxels_.end() || it->second.weight < params_.min_voxel_weight) return false;
  *sdf = it->second.sdf;
  return true;
}

void MeshLoamPipeline::extractBlock(const Eigen::Vector3i& block) {
  // Drop facets previously extracted for this block.
  auto bf = block_facets_.find(block);
  if (bf != block_facets_.end()) {
    for (const size_t idx : bf->second) {
      if (!facet_alive_[idx]) continue;
      facet_alive_[idx] = 0;
      auto& bin = facet_bins_[toFacetCell(facets_[idx].v0)];
      bin.erase(std::remove(bin.begin(), bin.end(), idx), bin.end());
    }
    bf->second.clear();
  }

  const int b = params_.block_size;
  const Eigen::Vector3i base(block.x() * b, block.y() * b, block.z() * b);
  std::vector<size_t>& out = block_facets_[block];

  // Tetrahedral decomposition of each cell formed by 8 voxel centers.
  static const int kCorner[8][3] = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
                                    {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}};
  static const int kTets[6][4] = {{0, 5, 1, 6}, {0, 1, 2, 6}, {0, 2, 3, 6},
                                  {0, 3, 7, 6}, {0, 7, 4, 6}, {0, 4, 5, 6}};

  auto emit = [&](const Eigen::Vector3d& a, const Eigen::Vector3d& bb,
                  const Eigen::Vector3d& cc, const Eigen::Vector3d& inside) {
    const Eigen::Vector3d n = (bb - a).cross(cc - a);
    const double len = n.norm();
    if (len < 1e-12) return;
    Facet f;
    f.v0 = (a + bb + cc) / 3.0;
    f.normal = n / len;
    // Orient from the negative (inside) side toward positive SDF.
    if (f.normal.dot(f.v0 - inside) < 0.0) f.normal = -f.normal;
    const size_t idx = facets_.size();
    facets_.push_back(f);
    facet_alive_.push_back(1);
    facet_bins_[toFacetCell(f.v0)].push_back(idx);
    out.push_back(idx);
  };

  double sdf[8];
  Eigen::Vector3d pos[8];
  for (int x = 0; x < b; ++x) {
    for (int y = 0; y < b; ++y) {
      for (int z = 0; z < b; ++z) {
        const Eigen::Vector3i v0(base.x() + x, base.y() + y, base.z() + z);
        bool ok = true;
        for (int i = 0; i < 8 && ok; ++i) {
          const Eigen::Vector3i vi(v0.x() + kCorner[i][0], v0.y() + kCorner[i][1],
                                   v0.z() + kCorner[i][2]);
          ok = sampleSdf(vi, &sdf[i]);
          pos[i] = voxelCenter(vi);
        }
        if (!ok) continue;

        for (const auto& tet : kTets) {
          const double d[4] = {sdf[tet[0]], sdf[tet[1]], sdf[tet[2]], sdf[tet[3]]};
          const Eigen::Vector3d* q[4] = {&pos[tet[0]], &pos[tet[1]], &pos[tet[2]],
                                         &pos[tet[3]]};
          int neg[4], np = 0;
          int posi[4], pp = 0;
          for (int i = 0; i < 4; ++i) {
            if (d[i] < 0.0) {
              neg[np++] = i;
            } else {
              posi[pp++] = i;
            }
          }
          if (np == 0 || pp == 0) continue;
          auto lerp = [&](int i, int j) {
            const double t = d[i] / (d[i] - d[j]);
            return (*q[i] + t * (*q[j] - *q[i])).eval();
          };
          if (np == 1 || pp == 1) {
            const int lone = (np == 1) ? neg[0] : posi[0];
            const int* others = (np == 1) ? posi : neg;
            const Eigen::Vector3d inside = (np == 1) ? *q[neg[0]] : *q[others[0]];
            const Eigen::Vector3d e0 = lerp(lone, others[0]);
            const Eigen::Vector3d e1 = lerp(lone, others[1]);
            const Eigen::Vector3d e2 = lerp(lone, others[2]);
            const Eigen::Vector3d in_pt = (np == 1) ? *q[lone] : inside;
            emit(e0, e1, e2, in_pt);
          } else {
            // 2-2 split: quad from the four crossing edges.
            const Eigen::Vector3d e00 = lerp(neg[0], posi[0]);
            const Eigen::Vector3d e01 = lerp(neg[0], posi[1]);
            const Eigen::Vector3d e10 = lerp(neg[1], posi[0]);
            const Eigen::Vector3d e11 = lerp(neg[1], posi[1]);
            const Eigen::Vector3d inside = 0.5 * (*q[neg[0]] + *q[neg[1]]);
            emit(e00, e01, e11, inside);
            emit(e00, e11, e10, inside);
          }
        }
      }
    }
  }
}

void MeshLoamPipeline::extractMesh() {
  for (auto& entry : dirty_blocks_) {
    if (entry.second) extractBlock(entry.first);
  }
  dirty_blocks_.clear();

  // Compact the facet store once dead entries dominate.
  size_t alive = 0;
  for (const char a : facet_alive_) alive += static_cast<size_t>(a);
  if (facets_.size() > 1024 && alive * 2 < facets_.size()) {
    std::vector<Facet> next;
    next.reserve(alive);
    std::vector<size_t> remap(facets_.size(), SIZE_MAX);
    for (size_t i = 0; i < facets_.size(); ++i) {
      if (!facet_alive_[i]) continue;
      remap[i] = next.size();
      next.push_back(facets_[i]);
    }
    facets_ = std::move(next);
    facet_alive_.assign(facets_.size(), 1);
    facet_bins_.clear();
    for (size_t i = 0; i < facets_.size(); ++i) {
      facet_bins_[toFacetCell(facets_[i].v0)].push_back(i);
    }
    for (auto& entry : block_facets_) {
      std::vector<size_t> mapped;
      mapped.reserve(entry.second.size());
      for (const size_t idx : entry.second) {
        if (remap[idx] != SIZE_MAX) mapped.push_back(remap[idx]);
      }
      entry.second = std::move(mapped);
    }
  }
}

void MeshLoamPipeline::pruneMap() {
  const Eigen::Vector3d center = pose_.block<3, 1>(0, 3);
  const double r2 = params_.local_map_radius * params_.local_map_radius;
  for (auto it = voxels_.begin(); it != voxels_.end();) {
    if ((voxelCenter(it->first) - center).squaredNorm() > r2) {
      it = voxels_.erase(it);
    } else {
      ++it;
    }
  }
  const double block_span = params_.block_size * params_.voxel_size;
  for (auto it = block_facets_.begin(); it != block_facets_.end();) {
    const Eigen::Vector3d block_center =
        (it->first.cast<double>() + Eigen::Vector3d::Constant(0.5)) * block_span;
    if ((block_center - center).squaredNorm() > r2) {
      for (const size_t idx : it->second) {
        if (!facet_alive_[idx]) continue;
        facet_alive_[idx] = 0;
        auto& bin = facet_bins_[toFacetCell(facets_[idx].v0)];
        bin.erase(std::remove(bin.begin(), bin.end(), idx), bin.end());
      }
      it = block_facets_.erase(it);
    } else {
      ++it;
    }
  }
}

bool MeshLoamPipeline::solveIcp(const std::vector<OrientedPoint>& features_world,
                                Eigen::Matrix4d* icp_correction,
                                MeshLoamResult* result) const {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  const double max_dist = params_.max_correspondence_dist;
  int used = 0;
  double abs_residual = 0.0;

  int iter = 0;
  for (; iter < params_.max_iterations; ++iter) {
    Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> g = Eigen::Matrix<double, 6, 1>::Zero();
    used = 0;
    abs_residual = 0.0;
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);

    for (const auto& op : features_world) {
      const Eigen::Vector3d p = R * op.p + t;
      const Eigen::Vector3d n_pt = R * op.n;
      const Eigen::Vector3i cell = toFacetCell(p);

      double best = max_dist;
      const Facet* best_facet = nullptr;
      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          for (int dz = -1; dz <= 1; ++dz) {
            const auto it =
                facet_bins_.find(Eigen::Vector3i(cell.x() + dx, cell.y() + dy, cell.z() + dz));
            if (it == facet_bins_.end()) continue;
            for (const size_t idx : it->second) {
              const Facet& f = facets_[idx];
              if (std::abs(f.normal.dot(n_pt)) < params_.cos_gate) continue;
              const double dist = std::abs(f.normal.dot(p - f.v0));
              if (dist < best) {
                best = dist;
                best_facet = &f;
              }
            }
          }
        }
      }
      if (best_facet == nullptr) continue;

      const double r_i = best_facet->normal.dot(p - best_facet->v0);
      Eigen::Matrix<double, 6, 1> J;
      J.head<3>() = best_facet->normal;
      J.tail<3>() = p.cross(best_facet->normal);
      H += J * J.transpose();
      g += J * r_i;
      ++used;
      abs_residual += std::abs(r_i);
    }

    if (used < params_.min_correspondences) break;

    const Eigen::Matrix<double, 6, 1> delta = H.ldlt().solve(-g);
    if (!delta.allFinite()) break;

    Eigen::Matrix4d step = Eigen::Matrix4d::Identity();
    step.block<3, 3>(0, 0) = expSo3(delta.tail<3>());
    step.block<3, 1>(0, 3) = delta.head<3>();
    T = step * T;

    if (delta.norm() < params_.convergence_eps) {
      ++iter;
      break;
    }
  }

  result->iterations = iter;
  result->num_correspondences = used;
  result->mean_abs_residual = used > 0 ? abs_residual / used : 0.0;
  if (used < params_.min_correspondences) return false;
  *icp_correction = T;
  return true;
}

MeshLoamResult MeshLoamPipeline::registerFrame(const std::vector<Eigen::Vector3d>& points) {
  MeshLoamResult result;
  const auto oriented = preprocess(points);

  if (frame_count_ == 0) {
    std::vector<OrientedPoint> world = oriented;  // pose is identity
    integrateScan(world);
    extractMesh();
    frame_count_ = 1;
    result.pose = pose_;
    result.converged = true;
    return result;
  }

  // Constant-velocity prediction.
  Eigen::Matrix4d prediction = pose_;
  if (has_prev_) {
    prediction = pose_ * (prev_pose_.inverse() * pose_);
  }

  // Planar features in the predicted world frame.
  std::vector<OrientedPoint> features;
  features.reserve(oriented.size());
  const Eigen::Matrix3d Rp = prediction.block<3, 3>(0, 0);
  const Eigen::Vector3d tp = prediction.block<3, 1>(0, 3);
  for (const auto& op : oriented) {
    if (!op.valid || op.curvature >= params_.curvature_threshold) continue;
    OrientedPoint w = op;
    w.p = Rp * op.p + tp;
    w.n = Rp * op.n;
    features.push_back(w);
  }

  Eigen::Matrix4d icp = Eigen::Matrix4d::Identity();
  result.converged = solveIcp(features, &icp, &result);
  const Eigen::Matrix4d new_pose = result.converged ? (icp * prediction).eval() : prediction;

  prev_pose_ = pose_;
  pose_ = new_pose;
  has_prev_ = true;

  // Integrate the full oriented scan at the refined pose.
  std::vector<OrientedPoint> world;
  world.reserve(oriented.size());
  const Eigen::Matrix3d R = pose_.block<3, 3>(0, 0);
  const Eigen::Vector3d t = pose_.block<3, 1>(0, 3);
  for (const auto& op : oriented) {
    if (!op.valid) continue;
    OrientedPoint w = op;
    w.p = R * op.p + t;
    w.n = R * op.n;
    world.push_back(w);
  }
  integrateScan(world);

  ++frame_count_;
  if (facets_.empty() || frame_count_ % params_.mesh_update_interval == 0) {
    extractMesh();
    pruneMap();
  }

  result.pose = pose_;
  return result;
}

}  // namespace mesh_loam
}  // namespace localization_zoo
