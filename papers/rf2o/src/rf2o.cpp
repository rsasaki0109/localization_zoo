#include "rf2o/rf2o.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace rf2o {

namespace {

float signf(float x) { return x >= 0.f ? 1.f : -1.f; }

Eigen::Matrix3d toMatrix3d(const Eigen::Matrix3f& m) {
  Eigen::Matrix3d out = Eigen::Matrix3d::Identity();
  out.block<2, 2>(0, 0) = m.block<2, 2>(0, 0).cast<double>();
  out(0, 2) = m(0, 2);
  out(1, 2) = m(1, 2);
  return out;
}

}  // namespace

Eigen::Matrix3d pose2D(double x, double y, double yaw) {
  Eigen::Matrix3d T = Eigen::Matrix3d::Identity();
  T(0, 0) = std::cos(yaw);
  T(0, 1) = -std::sin(yaw);
  T(1, 0) = std::sin(yaw);
  T(1, 1) = std::cos(yaw);
  T(0, 2) = x;
  T(1, 2) = y;
  return T;
}

RF2OEstimator::RF2OEstimator(const RF2OParams& params) : params_(params) {}

void RF2OEstimator::reset() {
  initialized_ = false;
  pyramid_.clear();
  level_transforms_.clear();
  kai_loc_old_.setZero();
  cov_odo_.setIdentity();
  pose_ = Eigen::Matrix3d::Identity();
}

void RF2OEstimator::setInitialPose(const Eigen::Matrix3d& pose) { pose_ = pose; }

void RF2OEstimator::initialize(const LaserScan& scan) {
  width_ = static_cast<int>(scan.size());
  cols_ = params_.target_cols > 0 ? params_.target_cols : width_;
  fovh_ = static_cast<float>(scan.fov());
  const int pyr_levels =
      static_cast<int>(std::round(std::log2(std::round(static_cast<float>(width_) /
                                                       static_cast<float>(cols_))))) +
      params_.coarse_to_fine_levels;
  pyramid_.assign(static_cast<size_t>(pyr_levels), LevelData{});
  level_transforms_.assign(static_cast<size_t>(params_.coarse_to_fine_levels),
                           Eigen::Matrix3f::Identity());
  initialized_ = true;
}

void RF2OEstimator::createPyramid(const std::vector<float>& ranges) {
  const int pyr_levels = static_cast<int>(pyramid_.size());
  const float max_range_dif = static_cast<float>(params_.max_range_diff);

  for (int i = 0; i < pyr_levels; ++i) {
    const int s = 1 << i;
    const int cols_i = static_cast<int>(std::ceil(static_cast<float>(width_) / s));
    auto& lvl = pyramid_[static_cast<size_t>(i)];
    lvl.cols = cols_i;
    lvl.range.assign(static_cast<size_t>(cols_i), 0.f);
    lvl.xx.assign(static_cast<size_t>(cols_i), 0.f);
    lvl.yy.assign(static_cast<size_t>(cols_i), 0.f);

    if (i == 0) {
      for (int u = 0; u < cols_i; ++u) {
        const float dcenter = u < static_cast<int>(ranges.size()) ? ranges[static_cast<size_t>(u)]
                                                                  : 0.f;
        if (!std::isfinite(dcenter) || dcenter <= params_.min_range ||
            dcenter >= params_.max_range) {
          lvl.range[static_cast<size_t>(u)] = 0.f;
          continue;
        }
        float sum = 0.f, weight = 0.f;
        for (int l = -2; l <= 2; ++l) {
          const int idx = u + l;
          if (idx < 0 || idx >= static_cast<int>(ranges.size())) continue;
          const float v = ranges[static_cast<size_t>(idx)];
          if (!std::isfinite(v) || v <= 0.f) continue;
          const float abs_dif = std::abs(v - dcenter);
          if (abs_dif < max_range_dif) {
            const float w = g_mask_[static_cast<size_t>(l + 2)] * (max_range_dif - abs_dif);
            weight += w;
            sum += w * v;
          }
        }
        lvl.range[static_cast<size_t>(u)] = weight > 0.f ? sum / weight : 0.f;
      }
    } else {
      const auto& prev = pyramid_[static_cast<size_t>(i - 1)];
      for (int u = 0; u < cols_i; ++u) {
        const int u2 = 2 * u;
        const float dcenter =
            u2 < prev.cols ? prev.range[static_cast<size_t>(u2)] : 0.f;
        if (dcenter <= 0.f) {
          lvl.range[static_cast<size_t>(u)] = 0.f;
          continue;
        }
        float sum = 0.f, weight = 0.f;
        for (int l = -2; l <= 2; ++l) {
          const int idx = u2 + l;
          if (idx < 0 || idx >= prev.cols) continue;
          const float v = prev.range[static_cast<size_t>(idx)];
          if (v <= 0.f) continue;
          const float abs_dif = std::abs(v - dcenter);
          if (abs_dif < max_range_dif) {
            const float w = g_mask_[static_cast<size_t>(l + 2)] * (max_range_dif - abs_dif);
            weight += w;
            sum += w * v;
          }
        }
        lvl.range[static_cast<size_t>(u)] = weight > 0.f ? sum / weight : 0.f;
      }
    }

    const float kdtita = static_cast<float>(cols_i - 1) / fovh_;
    for (int u = 0; u < cols_i; ++u) {
      const float r = lvl.range[static_cast<size_t>(u)];
      if (r > 0.f) {
        const float tita = -0.5f * fovh_ + static_cast<float>(u) / kdtita;
        lvl.xx[static_cast<size_t>(u)] = r * std::cos(tita);
        lvl.yy[static_cast<size_t>(u)] = r * std::sin(tita);
      }
    }
  }
}

void RF2OEstimator::createImagePyramid() {
  for (auto& lvl : pyramid_) {
    lvl.range_old = lvl.range;
    lvl.xx_old = lvl.xx;
    lvl.yy_old = lvl.yy;
  }
}

void RF2OEstimator::warpToPreviousLevel(int level, const Eigen::Matrix3f& cumulative) {
  auto& lvl = pyramid_[static_cast<size_t>(level)];
  const int cols_i = lvl.cols;
  const float kdtita = static_cast<float>(cols_i - 1) / fovh_;
  lvl.range_warped.assign(static_cast<size_t>(cols_i), 0.f);
  lvl.xx_warped.assign(static_cast<size_t>(cols_i), 0.f);
  lvl.yy_warped.assign(static_cast<size_t>(cols_i), 0.f);
  std::vector<float> wacu(static_cast<size_t>(cols_i), 0.f);

  for (int j = 0; j < cols_i; ++j) {
    if (lvl.range[static_cast<size_t>(j)] <= 0.f) continue;
    const float x_w = cumulative(0, 0) * lvl.xx[static_cast<size_t>(j)] +
                      cumulative(0, 1) * lvl.yy[static_cast<size_t>(j)] + cumulative(0, 2);
    const float y_w = cumulative(1, 0) * lvl.xx[static_cast<size_t>(j)] +
                      cumulative(1, 1) * lvl.yy[static_cast<size_t>(j)] + cumulative(1, 2);
    const float tita_w = std::atan2(y_w, x_w);
    const float range_w = std::hypot(x_w, y_w);
    const float uwarp = kdtita * (tita_w + 0.5f * fovh_);
    if (uwarp < 0.f || uwarp >= cols_i - 1) continue;
    const int uwarp_l = static_cast<int>(uwarp);
    const int uwarp_r = uwarp_l + 1;
    const float delta_r = static_cast<float>(uwarp_r) - uwarp;
    const float delta_l = uwarp - static_cast<float>(uwarp_l);
    if (std::abs(std::round(uwarp) - uwarp) < 0.05f) {
      const int idx = static_cast<int>(std::round(uwarp));
      lvl.range_warped[static_cast<size_t>(idx)] += range_w;
      wacu[static_cast<size_t>(idx)] += 1.f;
    } else {
      const float w_r = delta_l * delta_l;
      lvl.range_warped[static_cast<size_t>(uwarp_r)] += w_r * range_w;
      wacu[static_cast<size_t>(uwarp_r)] += w_r;
      const float w_l = delta_r * delta_r;
      lvl.range_warped[static_cast<size_t>(uwarp_l)] += w_l * range_w;
      wacu[static_cast<size_t>(uwarp_l)] += w_l;
    }
  }

  for (int u = 0; u < cols_i; ++u) {
    if (wacu[static_cast<size_t>(u)] > 0.f) {
      const float tita = -0.5f * fovh_ + static_cast<float>(u) / kdtita;
      lvl.range_warped[static_cast<size_t>(u)] /= wacu[static_cast<size_t>(u)];
      lvl.xx_warped[static_cast<size_t>(u)] =
          lvl.range_warped[static_cast<size_t>(u)] * std::cos(tita);
      lvl.yy_warped[static_cast<size_t>(u)] =
          lvl.range_warped[static_cast<size_t>(u)] * std::sin(tita);
    }
  }
}

void RF2OEstimator::calculateCoord(int level) {
  auto& lvl = pyramid_[static_cast<size_t>(level)];
  lvl.range_inter.resize(static_cast<size_t>(lvl.cols));
  lvl.xx_inter.resize(static_cast<size_t>(lvl.cols));
  lvl.yy_inter.resize(static_cast<size_t>(lvl.cols));
  for (int u = 0; u < lvl.cols; ++u) {
    if (lvl.range_old[static_cast<size_t>(u)] == 0.f ||
        lvl.range_warped[static_cast<size_t>(u)] == 0.f) {
      lvl.range_inter[static_cast<size_t>(u)] = 0.f;
      lvl.xx_inter[static_cast<size_t>(u)] = 0.f;
      lvl.yy_inter[static_cast<size_t>(u)] = 0.f;
    } else {
      lvl.range_inter[static_cast<size_t>(u)] =
          0.5f * (lvl.range_old[static_cast<size_t>(u)] +
                  lvl.range_warped[static_cast<size_t>(u)]);
      lvl.xx_inter[static_cast<size_t>(u)] =
          0.5f * (lvl.xx_old[static_cast<size_t>(u)] + lvl.xx_warped[static_cast<size_t>(u)]);
      lvl.yy_inter[static_cast<size_t>(u)] =
          0.5f * (lvl.yy_old[static_cast<size_t>(u)] + lvl.yy_warped[static_cast<size_t>(u)]);
    }
  }
}

void RF2OEstimator::calculateRangeDerivatives(int level, float fps) {
  auto& lvl = pyramid_[static_cast<size_t>(level)];
  const int cols_i = lvl.cols;
  lvl.rtita.assign(static_cast<size_t>(cols_i), 1.f);
  lvl.dtita.assign(static_cast<size_t>(cols_i), 0.f);
  lvl.dt.assign(static_cast<size_t>(cols_i), 0.f);

  for (int u = 0; u < cols_i - 1; ++u) {
    const float dx = lvl.xx_inter[static_cast<size_t>(u + 1)] -
                     lvl.xx_inter[static_cast<size_t>(u)];
    const float dy = lvl.yy_inter[static_cast<size_t>(u + 1)] -
                     lvl.yy_inter[static_cast<size_t>(u)];
    const float dist = dx * dx + dy * dy;
    if (dist > 0.f) lvl.rtita[static_cast<size_t>(u)] = std::sqrt(dist);
  }

  for (int u = 1; u < cols_i - 1; ++u) {
    lvl.dtita[static_cast<size_t>(u)] =
        (lvl.rtita[static_cast<size_t>(u - 1)] *
             (lvl.range_inter[static_cast<size_t>(u + 1)] -
              lvl.range_inter[static_cast<size_t>(u)]) +
         lvl.rtita[static_cast<size_t>(u)] *
             (lvl.range_inter[static_cast<size_t>(u)] -
              lvl.range_inter[static_cast<size_t>(u - 1)])) /
        (lvl.rtita[static_cast<size_t>(u)] + lvl.rtita[static_cast<size_t>(u - 1)] + 1e-6f);
  }
  if (cols_i >= 2) {
    lvl.dtita[0] = lvl.dtita[1];
    lvl.dtita[static_cast<size_t>(cols_i - 1)] = lvl.dtita[static_cast<size_t>(cols_i - 2)];
  }

  for (int u = 0; u < cols_i; ++u) {
    lvl.dt[static_cast<size_t>(u)] =
        fps * (lvl.range_warped[static_cast<size_t>(u)] - lvl.range_old[static_cast<size_t>(u)]);
  }
}

void RF2OEstimator::computeWeights(int level, float fps) {
  auto& lvl = pyramid_[static_cast<size_t>(level)];
  const int cols_i = lvl.cols;
  lvl.weights.assign(static_cast<size_t>(cols_i), 0.f);
  const float kdtita = 1.f;
  const float kdt = kdtita / (fps * fps);
  const float k2d = 0.2f;

  for (int u = 1; u < cols_i - 1; ++u) {
    if (lvl.null_mask[static_cast<size_t>(u)] != 0) continue;
    const float ini_dtita = lvl.range_old[static_cast<size_t>(u + 1)] -
                            lvl.range_old[static_cast<size_t>(u - 1)];
    const float final_dtita = lvl.range_warped[static_cast<size_t>(u + 1)] -
                              lvl.range_warped[static_cast<size_t>(u - 1)];
    const float dtitat = ini_dtita - final_dtita;
    const float dtita2 =
        lvl.dtita[static_cast<size_t>(u + 1)] - lvl.dtita[static_cast<size_t>(u - 1)];
    const float w_der = kdt * lvl.dt[static_cast<size_t>(u)] * lvl.dt[static_cast<size_t>(u)] +
                        kdtita * lvl.dtita[static_cast<size_t>(u)] *
                            lvl.dtita[static_cast<size_t>(u)] +
                        k2d * (std::abs(dtitat) + std::abs(dtita2));
    lvl.weights[static_cast<size_t>(u)] = std::sqrt(1.f / std::max(w_der, 1e-6f));
  }
  float max_w = 0.f;
  for (float w : lvl.weights) max_w = std::max(max_w, w);
  if (max_w > 0.f) {
    const float inv = 1.f / max_w;
    for (float& w : lvl.weights) w *= inv;
  }
}

void RF2OEstimator::findNullPoints(int level) {
  auto& lvl = pyramid_[static_cast<size_t>(level)];
  lvl.null_mask.assign(static_cast<size_t>(lvl.cols), 1);
  for (int u = 1; u < lvl.cols - 1; ++u) {
    lvl.null_mask[static_cast<size_t>(u)] =
        lvl.range_inter[static_cast<size_t>(u)] == 0.f ? 1 : 0;
  }
}

bool RF2OEstimator::solveSystem(int level, float fps, Eigen::Vector3f* velocity) {
  auto& lvl = pyramid_[static_cast<size_t>(level)];
  const int cols_i = lvl.cols;
  int valid = 0;
  for (int u = 1; u < cols_i - 1; ++u)
    if (lvl.null_mask[static_cast<size_t>(u)] == 0) ++valid;
  if (valid < 10) return false;

  const float kdtita = static_cast<float>(cols_i - 1) / fovh_;
  Eigen::MatrixXf A(valid, 3);
  Eigen::VectorXf B(valid);
  int row = 0;
  for (int u = 1; u < cols_i - 1; ++u) {
    if (lvl.null_mask[static_cast<size_t>(u)] != 0) continue;
    const float tw = lvl.weights[static_cast<size_t>(u)];
    const float tita = -0.5f * fovh_ + static_cast<float>(u) / kdtita;
    const float r = lvl.range_inter[static_cast<size_t>(u)];
    A(row, 0) = tw * (std::cos(tita) + lvl.dtita[static_cast<size_t>(u)] * kdtita *
                                             std::sin(tita) / std::max(r, 1e-3f));
    A(row, 1) = tw * (std::sin(tita) - lvl.dtita[static_cast<size_t>(u)] * kdtita *
                                             std::cos(tita) / std::max(r, 1e-3f));
    A(row, 2) = tw * (-lvl.yy[static_cast<size_t>(u)] * std::cos(tita) +
                      lvl.xx[static_cast<size_t>(u)] * std::sin(tita) -
                      lvl.dtita[static_cast<size_t>(u)] * kdtita);
    B(row) = tw * (-lvl.dt[static_cast<size_t>(u)]);
    ++row;
  }

  Eigen::Vector3f var = (A.transpose() * A).ldlt().solve(A.transpose() * B);
  if (!var.allFinite()) return false;

  Eigen::VectorXf res = A * var - B;
  cov_odo_ = (1.f / std::max(valid - 3, 1)) * (A.transpose() * A).inverse() * res.squaredNorm();
  *velocity = var;
  return true;
}

bool RF2OEstimator::filterLevelSolution(int ctf_level, float fps,
                                          const Eigen::Vector3f& velocity,
                                          Eigen::Matrix3f* increment) {
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> es(cov_odo_);
  if (es.info() != Eigen::Success) return false;

  const Eigen::Matrix3f Bii = es.eigenvectors();
  Eigen::Vector3f kai_b = Bii.colPivHouseholderQr().solve(velocity);

  Eigen::Matrix3f acu_trans = Eigen::Matrix3f::Identity();
  for (int i = 0; i < ctf_level; ++i) {
    acu_trans = level_transforms_[static_cast<size_t>(i)] * acu_trans;
  }
  Eigen::Vector3f kai_loc_sub = kai_loc_old_;
  kai_loc_sub(0) += -fps * acu_trans(0, 2);
  kai_loc_sub(1) += -fps * acu_trans(1, 2);
  if (acu_trans(0, 0) > 1.f) {
    kai_loc_sub(2) += 0.f;
  } else {
    kai_loc_sub(2) +=
        -fps * std::acos(std::clamp(acu_trans(0, 0), -1.f, 1.f)) * signf(acu_trans(1, 0));
  }
  Eigen::Vector3f kai_b_old = Bii.colPivHouseholderQr().solve(kai_loc_sub);

  const float cf = 15e3f * std::exp(-static_cast<float>(ctf_level));
  const float df = 0.05f * std::exp(-static_cast<float>(ctf_level));
  Eigen::Vector3f kai_b_fil;
  for (int i = 0; i < 3; ++i) {
    const float ev = es.eigenvalues()(i);
    kai_b_fil(i) = (kai_b(i) + (cf * ev + df) * kai_b_old(i)) / (1.f + cf * ev + df);
  }
  Eigen::Vector3f kai_loc_fil = Bii.inverse().colPivHouseholderQr().solve(kai_b_fil);

  const float incrx = kai_loc_fil(0) / fps;
  const float incry = kai_loc_fil(1) / fps;
  const float rot = kai_loc_fil(2) / fps;
  *increment = Eigen::Matrix3f::Identity();
  (*increment)(0, 0) = std::cos(rot);
  (*increment)(0, 1) = -std::sin(rot);
  (*increment)(1, 0) = std::sin(rot);
  (*increment)(1, 1) = std::cos(rot);
  (*increment)(0, 2) = incrx;
  (*increment)(1, 2) = incry;
  return true;
}

Eigen::Matrix3f RF2OEstimator::velocityToIncrement(const Eigen::Vector3f& vel, float fps) {
  Eigen::Matrix3f T = Eigen::Matrix3f::Identity();
  const float rot = vel(2) / fps;
  T(0, 0) = std::cos(rot);
  T(0, 1) = -std::sin(rot);
  T(1, 0) = std::sin(rot);
  T(1, 1) = std::cos(rot);
  T(0, 2) = vel(0) / fps;
  T(1, 2) = vel(1) / fps;
  return T;
}

RF2OResult RF2OEstimator::registerScan(const LaserScan& scan, double dt) {
  RF2OResult result;
  result.pose = pose_;
  if (scan.size() < 10) return result;

  std::vector<float> ranges(scan.size());
  for (size_t i = 0; i < scan.size(); ++i) {
    const double r = scan.ranges[i];
    ranges[i] = (!std::isfinite(r) || r < scan.range_min || r > scan.range_max)
                    ? 0.f
                    : static_cast<float>(r);
  }

  if (!initialized_ || width_ != static_cast<int>(scan.size())) {
    initialize(scan);
    createPyramid(ranges);
    pyramid_[0].range_old = pyramid_[0].range;
    pyramid_[0].xx_old = pyramid_[0].xx;
    pyramid_[0].yy_old = pyramid_[0].yy;
    result.valid = true;
    return result;
  }

  const float fps = static_cast<float>(1.0 / std::max(dt, 1e-3));
  createImagePyramid();
  createPyramid(ranges);

  const int log_offset = static_cast<int>(std::round(
      std::log2(std::round(static_cast<float>(width_) / static_cast<float>(cols_)))));
  std::fill(level_transforms_.begin(), level_transforms_.end(), Eigen::Matrix3f::Identity());

  for (int ctf_idx = 0; ctf_idx < params_.coarse_to_fine_levels; ++ctf_idx) {
    const int image_level = params_.coarse_to_fine_levels - ctf_idx - 1 + log_offset;
    if (image_level < 0 || image_level >= static_cast<int>(pyramid_.size())) {
      continue;
    }
    auto& lvl = pyramid_[static_cast<size_t>(image_level)];

    if (ctf_idx == 0) {
      lvl.range_warped = lvl.range;
      lvl.xx_warped = lvl.xx;
      lvl.yy_warped = lvl.yy;
    } else {
      Eigen::Matrix3f acu = Eigen::Matrix3f::Identity();
      for (int j = 1; j <= ctf_idx; ++j) {
        acu = level_transforms_[static_cast<size_t>(j - 1)] * acu;
      }
      warpToPreviousLevel(image_level, acu);
    }

    calculateCoord(image_level);
    calculateRangeDerivatives(image_level, fps);
    findNullPoints(image_level);
    computeWeights(image_level, fps);

    Eigen::Vector3f velocity = Eigen::Vector3f::Zero();
    bool solved = false;
    for (int iter = 0; iter < params_.irls_iterations; ++iter) {
      if (!solveSystem(image_level, fps, &velocity)) break;
      solved = true;
    }
    if (!solved) continue;

    Eigen::Matrix3f increment = Eigen::Matrix3f::Identity();
    if (!filterLevelSolution(ctf_idx, fps, velocity, &increment)) {
      increment = velocityToIncrement(velocity, fps);
    }
    level_transforms_[static_cast<size_t>(ctf_idx)] = increment;
  }

  Eigen::Matrix3f acu_trans = Eigen::Matrix3f::Identity();
  for (int i = 1; i <= params_.coarse_to_fine_levels; ++i) {
    acu_trans = level_transforms_[static_cast<size_t>(i - 1)] * acu_trans;
  }

  const float kai_loc_x = fps * acu_trans(0, 2);
  const float kai_loc_y = fps * acu_trans(1, 2);
  const float kai_loc_w = (acu_trans(0, 0) > 1.f)
                              ? 0.f
                              : fps * std::acos(std::clamp(acu_trans(0, 0), -1.f, 1.f)) *
                                    signf(acu_trans(1, 0));

  const float phi_before = std::atan2(static_cast<float>(pose_(1, 0)),
                                      static_cast<float>(pose_(0, 0)));
  const float kai_abs_x =
      kai_loc_x * std::cos(phi_before) - kai_loc_y * std::sin(phi_before);
  const float kai_abs_y =
      kai_loc_x * std::sin(phi_before) + kai_loc_y * std::cos(phi_before);
  const float kai_abs_w = kai_loc_w;

  Eigen::Matrix3d pose_aux = Eigen::Matrix3d::Identity();
  const float rot = kai_loc_w / fps;
  pose_aux(0, 0) = std::cos(rot);
  pose_aux(0, 1) = -std::sin(rot);
  pose_aux(1, 0) = std::sin(rot);
  pose_aux(1, 1) = std::cos(rot);
  pose_aux(0, 2) = acu_trans(0, 2);
  pose_aux(1, 2) = acu_trans(1, 2);

  pose_ = pose_ * pose_aux;
  result.increment = pose_aux;
  result.pose = pose_;
  result.valid = true;

  const float phi_after = std::atan2(static_cast<float>(pose_(1, 0)),
                                     static_cast<float>(pose_(0, 0)));
  kai_loc_old_(0) = kai_abs_x * std::cos(phi_after) + kai_abs_y * std::sin(phi_after);
  kai_loc_old_(1) = -kai_abs_x * std::sin(phi_after) + kai_abs_y * std::cos(phi_after);
  kai_loc_old_(2) = kai_abs_w;
  return result;
}

}  // namespace rf2o
}  // namespace localization_zoo
