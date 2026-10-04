#include "lf_gicp/lf_gicp.h"

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace localization_zoo {
namespace lf_gicp {

namespace {

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0.0, -v.z(), v.y(), v.z(), 0.0, -v.x(), -v.y(), v.x(), 0.0;
  return m;
}

Eigen::Matrix4d expTwist(const Eigen::Matrix<double, 6, 1>& xi) {
  // xi = [omega; rho], left-multiplied increment.
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  const Eigen::Vector3d omega = xi.head<3>();
  const double angle = omega.norm();
  if (angle > 1e-12) {
    T.block<3, 3>(0, 0) =
        Eigen::AngleAxisd(angle, omega / angle).toRotationMatrix();
  }
  T.block<3, 1>(0, 3) = xi.tail<3>();
  return T;
}

double median(std::deque<double> values) {
  if (values.empty()) return 0.0;
  const size_t mid = values.size() / 2;
  std::nth_element(values.begin(), values.begin() + mid, values.end());
  double m = values[mid];
  if (values.size() % 2 == 0) {
    m = 0.5 * (m + *std::max_element(values.begin(), values.begin() + mid));
  }
  return m;
}

Eigen::Vector3d transform(const Eigen::Matrix4d& T, const Eigen::Vector3d& p) {
  return T.block<3, 3>(0, 0) * p + T.block<3, 1>(0, 3);
}

}  // namespace

// ---------------------------------------------------------------- voxel map

Eigen::Vector3i GaussianVoxelMap::key(const Eigen::Vector3d& p) const {
  return Eigen::Vector3i(static_cast<int>(std::floor(p.x() / voxel_size_)),
                         static_cast<int>(std::floor(p.y() / voxel_size_)),
                         static_cast<int>(std::floor(p.z() / voxel_size_)));
}

VoxelGaussian GaussianVoxelMap::Accumulator::toGaussian() const {
  VoxelGaussian g;
  g.count = count;
  if (count == 0) return g;
  g.mean = sum / count;
  g.covariance = sum_sq / count - g.mean * g.mean.transpose();
  return g;
}

void GaussianVoxelMap::insert(const std::vector<Eigen::Vector3d>& world_points,
                              int frame) {
  for (const auto& p : world_points) {
    auto& acc = voxels_[key(p)];
    acc.sum += p;
    acc.sum_sq += p * p.transpose();
    ++acc.count;
    acc.last_frame = frame;
  }
}

void GaussianVoxelMap::evictOlderThan(int min_frame) {
  for (auto it = voxels_.begin(); it != voxels_.end();) {
    if (it->second.last_frame < min_frame) {
      it = voxels_.erase(it);
    } else {
      ++it;
    }
  }
}

std::vector<VoxelGaussian> GaussianVoxelMap::gaussians(int min_points) const {
  std::vector<VoxelGaussian> out;
  out.reserve(voxels_.size());
  for (const auto& [k, acc] : voxels_) {
    if (acc.count >= min_points) out.push_back(acc.toGaussian());
  }
  return out;
}

std::vector<std::pair<Eigen::Vector3i, VoxelGaussian>>
GaussianVoxelMap::entries(int min_points) const {
  std::vector<std::pair<Eigen::Vector3i, VoxelGaussian>> out;
  out.reserve(voxels_.size());
  for (const auto& [k, acc] : voxels_) {
    if (acc.count >= min_points) out.emplace_back(k, acc.toGaussian());
  }
  return out;
}

// ------------------------------------------------------ localizability field

LocalizabilityField computeLocalizabilityField(
    const std::vector<VoxelGaussian>& gaussians, int max_voxels,
    double covariance_regularization) {
  LocalizabilityField field;
  if (gaussians.empty()) return field;
  const size_t stride = std::max<size_t>(
      1, (gaussians.size() + static_cast<size_t>(max_voxels) - 1) /
             static_cast<size_t>(std::max(1, max_voxels)));
  for (size_t i = 0; i < gaussians.size(); i += stride) {
    // With covariance eigenvalues mu1 <= mu2 <= mu3, the information
    // Omega = (C + beta I)^-1 has eigenvalues 1/(mu1+beta) >= 1/(mu2+beta),
    // so the paper's planarity (lambda3 - lambda2) / lambda3 of Omega equals
    // 1 - (mu1 + beta) / (mu2 + beta); the normal is the eigenvector of mu1.
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eig(gaussians[i].covariance);
    const Eigen::Vector3d mu = eig.eigenvalues().cwiseMax(0.0);
    const double denom = mu(1) + covariance_regularization;
    if (!(denom > 1e-12)) continue;
    const double planarity =
        std::clamp(1.0 - (mu(0) + covariance_regularization) / denom, 0.0, 1.0);
    const Eigen::Vector3d n = eig.eigenvectors().col(0);
    field.M += planarity * n * n.transpose();
    ++field.sampled_voxels;
  }
  if (field.sampled_voxels == 0) return field;
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eig(field.M);
  const double trace = field.M.trace();
  const double lambda_min = std::max(0.0, eig.eigenvalues()(0));
  field.f0 = trace > 0.0 ? lambda_min / trace : 1.0 / 3.0;
  field.lambda0 = lambda_min / field.sampled_voxels;
  field.weak_axis = eig.eigenvectors().col(0);
  return field;
}

// --------------------------------------------------------- Fisher weighting

std::vector<double> fisherWeights(
    const std::vector<Eigen::Matrix<double, 3, 6>>& jacobians,
    const std::vector<Eigen::Matrix3d>& informations,
    const Eigen::Matrix<double, 6, 6>& hessian) {
  std::vector<double> weights(jacobians.size(), 1.0);
  if (jacobians.empty()) return weights;
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 6, 6>> eig(hessian);
  const Eigen::Matrix<double, 6, 1> v = eig.eigenvectors().col(0);
  double sum = 0.0;
  for (size_t m = 0; m < jacobians.size(); ++m) {
    const Eigen::Vector3d jv = jacobians[m] * v;
    weights[m] = jv.dot(informations[m] * jv);
    sum += weights[m];
  }
  const double mean = sum / static_cast<double>(jacobians.size());
  if (!(mean > 0.0)) {
    std::fill(weights.begin(), weights.end(), 1.0);
    return weights;
  }
  for (double& w : weights) w /= mean;
  return weights;
}

// ---------------------------------------------------------------- the gate

bool DegeneracyGate::update(double f0, double lambda0) {
  f0_window_.push_back(f0);
  lambda_window_.push_back(lambda0);
  while (static_cast<int>(f0_window_.size()) > params_.gate_window) {
    f0_window_.pop_front();
    lambda_window_.pop_front();
  }
  median_f0_ = median(f0_window_);
  median_lambda0_ = median(lambda_window_);
  if (!active_ && median_f0_ < params_.tau_on &&
      median_lambda0_ < params_.tau_absence) {
    active_ = true;
  } else if (active_ && (median_f0_ > params_.tau_off ||
                         median_lambda0_ >= params_.tau_absence)) {
    active_ = false;
  }
  return active_;
}

// ---------------------------------------------------------------- odometry

LFGICPOdometry::LFGICPOdometry(const LFGICPParams& params)
    : params_(params), map_(params.voxel_size), gate_(params) {}

std::vector<Eigen::Vector3d> LFGICPOdometry::preprocess(
    const std::vector<Eigen::Vector3d>& scan) const {
  std::vector<Eigen::Vector3d> out;
  out.reserve(scan.size());
  std::unordered_set<Eigen::Vector3i, GaussianVoxelMap::KeyHash,
                     GaussianVoxelMap::KeyEq>
      occupied;
  const double inv = 1.0 / params_.source_voxel_size;
  for (const auto& p : scan) {
    const double r = p.norm();
    if (!(r >= params_.min_range && r <= params_.max_range)) continue;
    const Eigen::Vector3i k(static_cast<int>(std::floor(p.x() * inv)),
                            static_cast<int>(std::floor(p.y() * inv)),
                            static_cast<int>(std::floor(p.z() * inv)));
    if (occupied.insert(k).second) out.push_back(p);
  }
  return out;
}

double LFGICPOdometry::adaptiveThreshold() const {
  // KISS-ICP style: 3 sigma of the model deviation, initial value until
  // a motion sample exists.
  if (model_error_count_ == 0) return params_.initial_threshold;
  return 3.0 * std::sqrt(model_error_sq_sum_ / model_error_count_);
}

LFGICPResult LFGICPOdometry::registerFrame(
    const std::vector<Eigen::Vector3d>& scan) {
  LFGICPResult result;
  const auto source = preprocess(scan);
  if (frame_ == 0 || map_.size() == 0) {
    std::vector<Eigen::Vector3d> world;
    world.reserve(source.size());
    for (const auto& p : source) world.push_back(transform(pose_, p));
    map_.insert(world, frame_);
    ++frame_;
    result.pose = pose_;
    return result;
  }

  // Snapshot of map Gaussians: lookup table and regularized information.
  const auto entries = map_.entries(params_.min_points_per_voxel);
  std::unordered_map<Eigen::Vector3i, int, GaussianVoxelMap::KeyHash,
                     GaussianVoxelMap::KeyEq>
      index;
  index.reserve(entries.size() * 2);
  std::vector<Eigen::Vector3d> means(entries.size());
  std::vector<Eigen::Matrix3d> infos(entries.size());
  std::vector<VoxelGaussian> raw(entries.size());
  for (size_t i = 0; i < entries.size(); ++i) {
    index.emplace(entries[i].first, static_cast<int>(i));
    means[i] = entries[i].second.mean;
    raw[i] = entries[i].second;
    infos[i] = (entries[i].second.covariance +
                params_.covariance_regularization * Eigen::Matrix3d::Identity())
                   .inverse();
  }

  // Localizability field and gate on the map before registration.
  result.field = computeLocalizabilityField(raw, params_.field_max_voxels,
                                            params_.field_regularization);
  const bool degenerate =
      params_.enable_mitigation &&
      gate_.update(result.field.f0, result.field.lambda0);
  result.degenerate = degenerate;
  if (degenerate) ++degenerate_frames_;

  const Eigen::Matrix4d prediction = pose_ * last_delta_;
  Eigen::Matrix4d T = prediction;
  const double threshold = adaptiveThreshold();

  std::vector<Eigen::Matrix<double, 3, 6>> jac;
  std::vector<Eigen::Matrix3d> info;
  std::vector<Eigen::Vector3d> resid;
  std::vector<double> huber;
  for (int it = 0; it < params_.max_iterations; ++it) {
    jac.clear();
    info.clear();
    resid.clear();
    huber.clear();
    for (const auto& p : source) {
      const Eigen::Vector3d q = transform(T, p);
      const Eigen::Vector3i k = map_.key(q);
      int best = -1;
      double best_d2 = threshold * threshold;
      for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
          for (int dz = -1; dz <= 1; ++dz) {
            const auto found = index.find(k + Eigen::Vector3i(dx, dy, dz));
            if (found == index.end()) continue;
            const double d2 = (q - means[found->second]).squaredNorm();
            if (d2 < best_d2) {
              best_d2 = d2;
              best = found->second;
            }
          }
      if (best < 0) continue;
      const Eigen::Vector3d d = q - means[best];
      const double maha = std::sqrt(std::max(0.0, d.dot(infos[best] * d)));
      Eigen::Matrix<double, 3, 6> J;
      J.block<3, 3>(0, 0) = -skew(q);
      J.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      jac.push_back(J);
      info.push_back(infos[best]);
      resid.push_back(d);
      huber.push_back(maha <= params_.huber_delta ? 1.0
                                                  : params_.huber_delta / maha);
    }
    result.correspondences = static_cast<int>(jac.size());
    if (jac.size() < 6) break;

    Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> b = Eigen::Matrix<double, 6, 1>::Zero();
    for (size_t m = 0; m < jac.size(); ++m) {
      const Eigen::Matrix<double, 6, 3> JtW = jac[m].transpose() * (huber[m] * info[m]);
      H += JtW * jac[m];
      b += JtW * resid[m];
    }
    if (degenerate) {
      std::vector<Eigen::Matrix3d> weighted_info(info.size());
      for (size_t m = 0; m < info.size(); ++m) weighted_info[m] = huber[m] * info[m];
      const auto w = fisherWeights(jac, weighted_info, H);
      H.setZero();
      b.setZero();
      for (size_t m = 0; m < jac.size(); ++m) {
        const Eigen::Matrix<double, 6, 3> JtW =
            jac[m].transpose() * (w[m] * huber[m] * info[m]);
        H += JtW * jac[m];
        b += JtW * resid[m];
      }
    }
    const Eigen::Matrix<double, 6, 1> dx = H.ldlt().solve(-b);
    if (!dx.allFinite()) break;
    T = expTwist(dx) * T;
    result.iterations = it + 1;
    if (dx.norm() < params_.convergence_epsilon) break;
  }

  // KISS-style model deviation for the adaptive threshold.
  const Eigen::Matrix4d deviation = prediction.inverse() * T;
  const double angle =
      Eigen::AngleAxisd(Eigen::Matrix3d(deviation.block<3, 3>(0, 0))).angle();
  const double model_error = deviation.block<3, 1>(0, 3).norm() +
                             2.0 * params_.max_range * std::sin(std::abs(angle) / 2.0);
  if (model_error > params_.min_motion_for_threshold) {
    model_error_sq_sum_ += model_error * model_error;
    ++model_error_count_;
  }

  last_delta_ = pose_.inverse() * T;
  pose_ = T;
  std::vector<Eigen::Vector3d> world;
  world.reserve(source.size());
  for (const auto& p : source) world.push_back(transform(pose_, p));
  map_.insert(world, frame_);
  map_.evictOlderThan(frame_ - params_.map_max_age_frames + 1);
  ++frame_;
  result.pose = pose_;
  return result;
}

}  // namespace lf_gicp
}  // namespace localization_zoo
