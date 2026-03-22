#include "xicp/xicp_registration.h"

#include <Eigen/Dense>
#include <Eigen/SVD>

#include <cmath>
#include <iostream>

namespace localization_zoo {
namespace xicp {

namespace {

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0, -v.z(), v.y(),
       v.z(), 0, -v.x(),
       -v.y(), v.x(), 0;
  return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& omega) {
  double theta = omega.norm();
  if (theta < 1e-10) return Eigen::Matrix3d::Identity() + skew(omega);
  Eigen::Matrix3d K = skew(omega / theta);
  return Eigen::Matrix3d::Identity() + std::sin(theta) * K +
         (1.0 - std::cos(theta)) * K * K;
}

}  // namespace

LocalizabilityInfo XICPRegistration::analyzeLocalizability(
    const std::vector<Correspondence>& correspondences,
    const Eigen::Matrix3d& R, const Eigen::Vector3d& t) const {
  LocalizabilityInfo info;
  int n = correspondences.size();
  if (n == 0) return info;

  // ヘシアン A' = A^T A (6x6) を構築
  Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();

  // 正規化された情報行列 F_r, F_t
  Eigen::MatrixXd F_r(n, 3);
  Eigen::MatrixXd F_t(n, 3);

  for (int i = 0; i < n; i++) {
    Eigen::Vector3d p = R * correspondences[i].source + t;
    Eigen::Vector3d u = correspondences[i].normal;

    Eigen::Vector3d cross = p.cross(u);
    Eigen::Matrix<double, 6, 1> a;
    a << cross, u;
    H += a * a.transpose();

    // 正規化 (moment-normalized)
    double cross_norm = cross.norm();
    if (cross_norm > 1.0) {
      F_r.row(i) = (cross / cross_norm).transpose();
    } else {
      F_r.row(i) = cross.transpose();
    }
    F_t.row(i) = u.transpose();
  }

  // 回転/並進ブロックのSVD
  Eigen::Matrix3d H_rr = H.block<3, 3>(0, 0);
  Eigen::Matrix3d H_tt = H.block<3, 3>(3, 3);

  Eigen::JacobiSVD<Eigen::Matrix3d> svd_r(
      H_rr, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Eigen::JacobiSVD<Eigen::Matrix3d> svd_t(
      H_tt, Eigen::ComputeFullU | Eigen::ComputeFullV);

  info.V_r = svd_r.matrixV();
  info.V_t = svd_t.matrixV();
  info.sigma_r = svd_r.singularValues();
  info.sigma_t = svd_t.singularValues();

  // ローカライズ可能性スコアを計算
  // I = |F · V| (各対応点の寄与を固有ベクトル方向に射影)
  Eigen::MatrixXd I_r = (F_r * info.V_r).cwiseAbs();
  Eigen::MatrixXd I_t = (F_t * info.V_t).cwiseAbs();

  // フィルタリング Stage 1: 低寄与を除去 (κ_f)
  // Stage 2: 強寄与を識別 (κ_s)
  for (int j = 0; j < 3; j++) {
    double Lc_r = 0, Ls_r = 0;
    double Lc_t = 0, Ls_t = 0;

    for (int i = 0; i < n; i++) {
      // 回転
      if (I_r(i, j) >= params_.kappa_f) {
        Lc_r += I_r(i, j);
        if (I_r(i, j) >= params_.kappa_s) {
          Ls_r += I_r(i, j);
        }
      }
      // 並進
      if (I_t(i, j) >= params_.kappa_f) {
        Lc_t += I_t(i, j);
        if (I_t(i, j) >= params_.kappa_s) {
          Ls_t += I_t(i, j);
        }
      }
    }

    info.L_c_rot(j) = Lc_r;
    info.L_s_rot(j) = Ls_r;
    info.L_c_trans(j) = Lc_t;
    info.L_s_trans(j) = Ls_t;

    // 分類
    auto classify = [&](double Lc, double Ls) -> Localizability {
      if (Lc >= params_.kappa_1 || Ls >= params_.kappa_2)
        return Localizability::FULL;
      if (Lc >= params_.kappa_2 || Ls >= params_.kappa_3)
        return Localizability::PARTIAL;
      return Localizability::NONE;
    };

    info.rotation[j] = classify(Lc_r, Ls_r);
    info.translation[j] = classify(Lc_t, Ls_t);
  }

  // Partial方向の制約値を計算 (再サンプリング)
  // 退化方向に強く整列した対応点のみでミニ最適化
  for (int j = 0; j < 3; j++) {
    if (info.translation[j] == Localizability::PARTIAL) {
      Eigen::Vector3d vj = info.V_t.col(j);
      // この方向に寄与する対応点のみ抽出
      Eigen::Matrix3d A_re = Eigen::Matrix3d::Zero();
      Eigen::Vector3d b_re = Eigen::Vector3d::Zero();
      for (int i = 0; i < n; i++) {
        if (I_t(i, j) >= params_.kappa_f) {
          Eigen::Vector3d u = correspondences[i].normal;
          Eigen::Vector3d p = R * correspondences[i].source + t;
          Eigen::Vector3d q = correspondences[i].target;
          A_re += u * u.transpose();
          b_re += u * u.dot(q - p);
        }
      }
      if (A_re.norm() > 1e-6) {
        info.t0 += vj * vj.dot(A_re.ldlt().solve(b_re));
      }
    }

    if (info.rotation[j] == Localizability::PARTIAL) {
      Eigen::Vector3d vj = info.V_r.col(j);
      Eigen::Matrix3d A_re = Eigen::Matrix3d::Zero();
      Eigen::Vector3d b_re = Eigen::Vector3d::Zero();
      for (int i = 0; i < n; i++) {
        if (I_r(i, j) >= params_.kappa_f) {
          Eigen::Vector3d u = correspondences[i].normal;
          Eigen::Vector3d p = R * correspondences[i].source + t;
          Eigen::Vector3d q = correspondences[i].target;
          Eigen::Vector3d cross = p.cross(u);
          A_re += cross * cross.transpose();
          b_re += cross * u.dot(q - p);
        }
      }
      if (A_re.norm() > 1e-6) {
        info.r0 += vj * vj.dot(A_re.ldlt().solve(b_re));
      }
    }
  }

  return info;
}

Eigen::Matrix<double, 6, 1> XICPRegistration::solveUnconstrained(
    const std::vector<Correspondence>& correspondences,
    const Eigen::Matrix3d& R, const Eigen::Vector3d& t,
    Eigen::Matrix<double, 6, 6>& H,
    Eigen::Matrix<double, 6, 1>& b) const {
  int n = correspondences.size();
  H.setZero();
  b.setZero();

  for (int i = 0; i < n; i++) {
    Eigen::Vector3d p = R * correspondences[i].source + t;
    Eigen::Vector3d q = correspondences[i].target;
    Eigen::Vector3d u = correspondences[i].normal;

    Eigen::Matrix<double, 6, 1> a;
    a << p.cross(u), u;

    double residual = u.dot(q - p);

    H += a * a.transpose();
    b += a * residual;
  }

  return H.ldlt().solve(b);
}

Eigen::Matrix<double, 6, 1> XICPRegistration::solveConstrained(
    const Eigen::Matrix<double, 6, 6>& H,
    const Eigen::Matrix<double, 6, 1>& b,
    const LocalizabilityInfo& loc_info) const {
  // 制約行列 C, d を構築
  std::vector<Eigen::Matrix<double, 1, 6>> C_rows;
  std::vector<double> d_vals;

  for (int j = 0; j < 3; j++) {
    if (loc_info.rotation[j] != Localizability::FULL) {
      Eigen::Matrix<double, 1, 6> c = Eigen::Matrix<double, 1, 6>::Zero();
      c.head<3>() = loc_info.V_r.col(j).transpose();
      C_rows.push_back(c);
      double dj = (loc_info.rotation[j] == Localizability::PARTIAL)
                      ? loc_info.V_r.col(j).dot(loc_info.r0)
                      : 0.0;
      d_vals.push_back(dj);
    }
    if (loc_info.translation[j] != Localizability::FULL) {
      Eigen::Matrix<double, 1, 6> c = Eigen::Matrix<double, 1, 6>::Zero();
      c.tail<3>() = loc_info.V_t.col(j).transpose();
      C_rows.push_back(c);
      double dj = (loc_info.translation[j] == Localizability::PARTIAL)
                      ? loc_info.V_t.col(j).dot(loc_info.t0)
                      : 0.0;
      d_vals.push_back(dj);
    }
  }

  if (C_rows.empty()) {
    return H.ldlt().solve(b);
  }

  int c = C_rows.size();
  Eigen::MatrixXd C(c, 6);
  Eigen::VectorXd d(c);
  for (int i = 0; i < c; i++) {
    C.row(i) = C_rows[i];
    d(i) = d_vals[i];
  }

  // 拡張正規方程式 (Lagrange乗数法):
  // [2H  C^T] [x*]   [2b]
  // [C   0  ] [λ*] = [d ]
  int sz = 6 + c;
  Eigen::MatrixXd A_aug = Eigen::MatrixXd::Zero(sz, sz);
  Eigen::VectorXd b_aug = Eigen::VectorXd::Zero(sz);

  A_aug.block<6, 6>(0, 0) = 2.0 * H;
  A_aug.block(0, 6, 6, c) = C.transpose();
  A_aug.block(6, 0, c, 6) = C;

  b_aug.head<6>() = 2.0 * b;
  b_aug.tail(c) = d;

  // SVDで解く
  Eigen::JacobiSVD<Eigen::MatrixXd> svd(
      A_aug, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Eigen::VectorXd x_aug = svd.solve(b_aug);

  return x_aug.head<6>();
}

XICPResult XICPRegistration::align(
    const std::vector<Correspondence>& correspondences,
    const Eigen::Matrix4d& initial_guess) {
  XICPResult result;
  result.transformation = initial_guess;

  Eigen::Matrix3d R = initial_guess.block<3, 3>(0, 0);
  Eigen::Vector3d t = initial_guess.block<3, 1>(0, 3);

  for (int iter = 0; iter < params_.max_iterations; iter++) {
    // ローカライズ可能性解析
    auto loc_info = analyzeLocalizability(correspondences, R, t);

    // 正規方程式を構築
    Eigen::Matrix<double, 6, 6> H;
    Eigen::Matrix<double, 6, 1> b;
    solveUnconstrained(correspondences, R, t, H, b);

    // 制約の有無で分岐
    Eigen::Matrix<double, 6, 1> delta;
    if (loc_info.hasDegeneracy()) {
      delta = solveConstrained(H, b, loc_info);
    } else {
      delta = H.ldlt().solve(b);
    }

    // 更新を適用
    R = expSO3(delta.head<3>()) * R;
    t = t + delta.tail<3>();

    result.num_iterations = iter + 1;
    result.localizability = loc_info;

    if (delta.norm() < params_.convergence_threshold) {
      result.converged = true;
      break;
    }
  }

  result.transformation.block<3, 3>(0, 0) = R;
  result.transformation.block<3, 1>(0, 3) = t;
  result.transformation.row(3) << 0, 0, 0, 1;

  return result;
}

}  // namespace xicp
}  // namespace localization_zoo
