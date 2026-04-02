#include "litamin2/litamin2_registration.h"

#include <Eigen/Dense>
#include <cmath>
#include <iostream>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace localization_zoo {
namespace litamin2 {

namespace {

/// so(3) の歪対称行列
Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0, -v.z(), v.y(),
       v.z(), 0, -v.x(),
       -v.y(), v.x(), 0;
  return m;
}

/// so(3) → SO(3) 指数写像
Eigen::Matrix3d expSO3(const Eigen::Vector3d& omega) {
  double theta = omega.norm();
  if (theta < 1e-10) {
    return Eigen::Matrix3d::Identity() + skew(omega);
  }
  Eigen::Matrix3d K = skew(omega / theta);
  return Eigen::Matrix3d::Identity() + std::sin(theta) * K +
         (1.0 - std::cos(theta)) * K * K;
}

}  // namespace

void LiTAMIN2Registration::setTarget(
    const std::vector<Eigen::Vector3d>& target_points) {
  target_voxel_map_ = std::make_unique<GaussianVoxelMap>(
      params_.voxel_resolution, params_.min_points_per_voxel);
  target_voxel_map_->createFromPoints(target_points, params_.num_threads);
}

void LiTAMIN2Registration::buildSourceVoxelMap(
    const std::vector<Eigen::Vector3d>& source_points) {
  source_voxel_map_ = std::make_unique<GaussianVoxelMap>(
      params_.voxel_resolution, params_.min_points_per_voxel);
  source_voxel_map_->createFromPoints(source_points, params_.num_threads);
}

std::vector<std::pair<std::shared_ptr<GaussianVoxel>,
                      std::shared_ptr<GaussianVoxel>>>
LiTAMIN2Registration::updateCorrespondences(const Eigen::Matrix3d& R,
                                            const Eigen::Vector3d& t) const {
  std::vector<
      std::pair<std::shared_ptr<GaussianVoxel>, std::shared_ptr<GaussianVoxel>>>
      corrs;
  corrs.reserve(source_voxel_map_->size());

  for (auto& [coord, src_voxel] : source_voxel_map_->voxels()) {
    // ソースボクセルの重心を変換
    Eigen::Vector3d transformed_mean = R * src_voxel->mean + t;

    // 変換後の座標でターゲットボクセルを検索
    auto tgt_voxel = target_voxel_map_->lookup(transformed_mean);
    if (tgt_voxel) {
      corrs.emplace_back(src_voxel, tgt_voxel);
    }
  }

  return corrs;
}

double LiTAMIN2Registration::linearize(
    const Eigen::Matrix3d& R, const Eigen::Vector3d& t,
    const std::vector<std::pair<std::shared_ptr<GaussianVoxel>,
                                std::shared_ptr<GaussianVoxel>>>& corrs,
    Eigen::Matrix<double, 6, 6>& H, Eigen::Matrix<double, 6, 1>& b) const {
  H.setZero();
  b.setZero();
  double total_error = 0.0;

  const double lambda = params_.lambda;
  const double sigma_icp_sq = params_.sigma_icp * params_.sigma_icp;
  const double sigma_cov_sq = params_.sigma_cov * params_.sigma_cov;
  const Eigen::Matrix3d I3 = Eigen::Matrix3d::Identity();
  const int thread_count = std::max(1, params_.num_threads);

#ifdef _OPENMP
  std::vector<Eigen::Matrix<double, 6, 6>> H_locals(
      thread_count, Eigen::Matrix<double, 6, 6>::Zero());
  std::vector<Eigen::Matrix<double, 6, 1>> b_locals(
      thread_count, Eigen::Matrix<double, 6, 1>::Zero());
  std::vector<double> error_locals(thread_count, 0.0);

#pragma omp parallel num_threads(thread_count) if (thread_count > 1)
  {
    const int tid = omp_get_thread_num();
    auto& H_local = H_locals[tid];
    auto& b_local = b_locals[tid];
    double& error_local = error_locals[tid];

#pragma omp for nowait
    for (int corr_idx = 0; corr_idx < static_cast<int>(corrs.size()); corr_idx++) {
      const auto& src = corrs[corr_idx].first;
      const auto& tgt = corrs[corr_idx].second;
#else
  for (size_t corr_idx = 0; corr_idx < corrs.size(); corr_idx++) {
    const auto& src = corrs[corr_idx].first;
    const auto& tgt = corrs[corr_idx].second;
    auto& H_local = H;
    auto& b_local = b;
    double& error_local = total_error;
#endif
      const Eigen::Vector3d& p = src->mean;
      const Eigen::Vector3d& q = tgt->mean;
      const Eigen::Matrix3d& Cp = src->cov;
      const Eigen::Matrix3d& Cq = tgt->cov;

      const Eigen::Vector3d Rp_t = R * p + t;
      const Eigen::Matrix3d RCpRT = R * Cp * R.transpose();
      const Eigen::Matrix3d C_sum = Cq + RCpRT + lambda * I3;
      const Eigen::Matrix3d C_sum_inv = C_sum.inverse();
      const double frob_norm = C_sum_inv.norm();
      const Eigen::Matrix3d C_qp = C_sum_inv / frob_norm;

      const Eigen::Vector3d error = q - Rp_t;
      const double E_icp = error.transpose() * C_qp * error;
      const double w_icp = 1.0 - E_icp / (E_icp + sigma_icp_sq);

      double w_cov = 0.0;
      double E_cov = 0.0;
      if (params_.use_cov_cost) {
        const Eigen::Matrix3d Cp_inv = Cp.inverse();
        const Eigen::Matrix3d Cq_inv = Cq.inverse();
        const double trace1 = (R * Cp_inv * R.transpose() * Cq).trace();
        const double trace2 = (Cq_inv * RCpRT).trace();
        const double cov_residual = trace1 + trace2 - 6.0;
        E_cov = cov_residual * cov_residual;
        w_cov = 1.0 - E_cov / (E_cov + sigma_cov_sq);
      }

      error_local += w_icp * E_icp + w_cov * E_cov;

      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = skew(Rp_t);
      J.block<3, 3>(0, 3) = -I3;

      const Eigen::Matrix<double, 3, 1> C_qp_error = C_qp * error;
      H_local += w_icp * J.transpose() * C_qp * J;
      b_local += w_icp * J.transpose() * C_qp_error;
    }
#ifdef _OPENMP
  }

  for (int tid = 0; tid < thread_count; tid++) {
    H += H_locals[tid];
    b += b_locals[tid];
    total_error += error_locals[tid];
  }
#endif

  return total_error;
}

bool LiTAMIN2Registration::isConverged(
    const Eigen::Matrix<double, 6, 1>& delta) const {
  // 回転部分と並進部分をそれぞれ閾値と比較
  double rotation_delta = delta.head<3>().norm();
  double translation_delta = delta.tail<3>().norm();

  return rotation_delta < params_.rotation_epsilon &&
         translation_delta < params_.translation_epsilon;
}

RegistrationResult LiTAMIN2Registration::align(
    const std::vector<Eigen::Vector3d>& source_points,
    const Eigen::Matrix4d& initial_guess) {
  if (!target_voxel_map_ || target_voxel_map_->size() == 0) {
    std::cerr << "[LiTAMIN2] Target not set or empty." << std::endl;
    return {};
  }

  // ソースのボクセルマップを構築
  buildSourceVoxelMap(source_points);

  if (source_voxel_map_->size() == 0) {
    std::cerr << "[LiTAMIN2] Source voxel map is empty." << std::endl;
    return {};
  }

  // 初期変換
  Eigen::Matrix3d R = initial_guess.block<3, 3>(0, 0);
  Eigen::Vector3d t = initial_guess.block<3, 1>(0, 3);

  RegistrationResult result;
  result.transformation = initial_guess;

  for (int iter = 0; iter < params_.max_iterations; iter++) {
    // 対応関係を更新
    auto corrs = updateCorrespondences(R, t);

    if (corrs.empty()) {
      std::cerr << "[LiTAMIN2] No correspondences found at iteration " << iter
                << std::endl;
      break;
    }

    // 線形化
    Eigen::Matrix<double, 6, 6> H;
    Eigen::Matrix<double, 6, 1> b_vec;
    double error = linearize(R, t, corrs, H, b_vec);

    // ガウスニュートン更新: H δ = -b
    Eigen::Matrix<double, 6, 1> delta = H.ldlt().solve(-b_vec);

    // 更新を適用
    Eigen::Matrix3d dR = expSO3(delta.head<3>());
    Eigen::Vector3d dt = delta.tail<3>();

    R = dR * R;
    t = dR * t + dt;

    result.num_iterations = iter + 1;
    result.final_error = error;

    // 収束判定
    if (isConverged(delta)) {
      result.converged = true;
      break;
    }
  }

  // 結果の変換行列を構築
  result.transformation.block<3, 3>(0, 0) = R;
  result.transformation.block<3, 1>(0, 3) = t;
  result.transformation.row(3) << 0, 0, 0, 1;

  return result;
}

}  // namespace litamin2
}  // namespace localization_zoo
