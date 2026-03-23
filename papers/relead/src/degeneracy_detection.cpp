#include "relead/degeneracy_detection.h"

#include <Eigen/Dense>
#include <Eigen/SVD>

namespace localization_zoo {
namespace relead {

DegeneracyInfo DegeneracyDetection::detect(
    const std::vector<PointWithNormal>& correspondences) const {
  DegeneracyInfo info;

  if (correspondences.empty()) return info;

  int n = correspondences.size();

  // ヘシアン A' = A^T A を構築 (6x6)
  // A は Nx6 のヤコビアン行列
  // 各行: [p × n, n] (point-to-plane残差のヤコビアン)
  // ここで p は変換済み点、n は法線
  Eigen::MatrixXd A(n, 6);

  for (int i = 0; i < n; i++) {
    const auto& corr = correspondences[i];
    Eigen::Vector3d p = corr.point;
    Eigen::Vector3d u = corr.normal;

    // 回転部分: p × n
    Eigen::Vector3d cross = p.cross(u);
    A(i, 0) = cross.x();
    A(i, 1) = cross.y();
    A(i, 2) = cross.z();
    // 並進部分: n
    A(i, 3) = u.x();
    A(i, 4) = u.y();
    A(i, 5) = u.z();
  }

  // A' = A^T A (6x6)
  Eigen::Matrix<double, 6, 6> AtA = A.transpose() * A;

  // 回転と並進に分割
  // A'_rr (3x3): 回転部分
  // A'_tt (3x3): 並進部分
  Eigen::Matrix3d A_rr = AtA.block<3, 3>(0, 0);
  Eigen::Matrix3d A_tt = AtA.block<3, 3>(3, 3);

  // SVDで各成分を分解
  Eigen::JacobiSVD<Eigen::Matrix3d> svd_r(
      A_rr, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Eigen::JacobiSVD<Eigen::Matrix3d> svd_t(
      A_tt, Eigen::ComputeFullU | Eigen::ComputeFullV);

  Eigen::Vector3d sigma_r = svd_r.singularValues();
  Eigen::Vector3d sigma_t = svd_t.singularValues();
  Eigen::Matrix3d V_r = svd_r.matrixV();
  Eigen::Matrix3d V_t = svd_t.matrixV();

  // 各方向のローカライズ可能性を判定
  // 固有値比 = sigma_i / sigma_max
  auto classify = [&](double sigma, double sigma_max) -> Localizability {
    if (sigma_max < 1e-10) return Localizability::NONE;
    double ratio = sigma / sigma_max;
    if (ratio < params_.none_threshold) return Localizability::NONE;
    if (ratio < params_.partial_threshold) return Localizability::PARTIAL;
    return Localizability::FULL;
  };

  // 回転方向
  for (int i = 0; i < 3; i++) {
    info.rotation_localizability[i] = classify(sigma_r(i), sigma_r(0));
    if (info.rotation_localizability[i] == Localizability::NONE) {
      info.degenerate_rotation_dirs.push_back(V_r.col(i));
    }
  }

  // 並進方向
  for (int i = 0; i < 3; i++) {
    info.translation_localizability[i] = classify(sigma_t(i), sigma_t(0));
    if (info.translation_localizability[i] == Localizability::NONE) {
      info.degenerate_translation_dirs.push_back(V_t.col(i));
    }
  }

  return info;
}

}  // namespace relead
}  // namespace localization_zoo
