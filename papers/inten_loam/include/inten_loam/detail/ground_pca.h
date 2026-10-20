#pragma once

#include <Eigen/Eigenvalues>
#include <vector>

namespace localization_zoo {
namespace inten_loam {
namespace detail {

// Repository-only S2S control: replace a valid ground triangle's normal using
// the same gated query neighbours. The plane still passes through the original
// nearest point. Do not change support, and retain the triangle on degeneracy.
template <typename Features>
bool groundPcaNormal(const Features& points, const std::vector<int>& ids,
                     const std::vector<float>& distances, double gate,
                     Eigen::Vector3d* normal) {
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  int count = 0;
  for (size_t k = 0; k < ids.size(); ++k) {
    if (distances[k] > gate) continue;
    mean += points[static_cast<size_t>(ids[k])].p;
    ++count;
  }
  if (count < 3) return false;
  mean /= count;
  Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
  for (size_t k = 0; k < ids.size(); ++k) {
    if (distances[k] > gate) continue;
    const Eigen::Vector3d d = points[static_cast<size_t>(ids[k])].p - mean;
    covariance.noalias() += d * d.transpose();
  }
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eig(covariance);
  if (eig.info() != Eigen::Success || !eig.eigenvalues().allFinite() ||
      eig.eigenvalues()[1] <= 1e-12 * eig.eigenvalues()[2]) return false;
  Eigen::Vector3d candidate = eig.eigenvectors().col(0);
  if (candidate.dot(*normal) < 0.0) candidate = -candidate;
  *normal = candidate;
  return true;
}

}  // namespace detail
}  // namespace inten_loam
}  // namespace localization_zoo
