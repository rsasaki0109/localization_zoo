#include "imu_motion_health/posture_confirmation.h"

#include <algorithm>
#include <cmath>

namespace localization_zoo {
namespace imu_motion_health {

namespace {

constexpr double kRadToDeg = 180.0 / M_PI;

double angleDeg(const Eigen::Vector3d& unit, const Eigen::Vector3d& v) {
  const double n = v.norm();
  if (n <= 0.0) return 0.0;
  return std::acos(std::max(-1.0, std::min(1.0, unit.dot(v) / n))) * kRadToDeg;
}

}  // namespace

PostureConfirmer::PostureConfirmer(const PostureConfirmationParams& params) : params_(params) {}

void PostureConfirmer::observe(double timestamp, const Eigen::Vector3d& gyro,
                               const Eigen::Vector3d& accel) {
  const Raw sample{timestamp, gyro, accel};
  if (vqf_) {
    push(sample);
    return;
  }
  startup_.push_back(sample);
  if (startup_.size() < kRateSamples) return;
  std::vector<double> dts;
  for (std::size_t i = 1; i < startup_.size(); ++i) dts.push_back(startup_[i].t - startup_[i - 1].t);
  std::nth_element(dts.begin(), dts.begin() + dts.size() / 2, dts.end());
  double dt = dts[dts.size() / 2];
  if (dts.size() % 2 == 0) {  // median of an even count, as statistics.median
    const double upper = dt;
    dt = 0.5 * (upper + *std::max_element(dts.begin(), dts.begin() + dts.size() / 2));
  }
  attitude_estimation::VQFParams vqf;
  vqf.sampling_rate = 1.0 / dt;
  vqf.tau_acc = params_.tau_acc_s;
  vqf_ = std::make_unique<attitude_estimation::VQF>(vqf);
  for (const Raw& s : startup_) push(s);
  startup_.clear();
}

void PostureConfirmer::push(const Raw& sample) {
  vqf_->update(sample.gyro, sample.accel);
  const attitude_estimation::Quat q = vqf_->quat6D();
  const double w = q[0], x = q[1], y = q[2], z = q[3];
  // Body-frame up direction R(q)^T e_z.
  const Eigen::Vector3d up(2 * (x * z - w * y), 2 * (y * z + w * x), 1 - 2 * (x * x + y * y));
  history_.push_back({sample.t, up, sample.accel});
  const double keep = params_.pre_start_s + params_.max_wait_s + 1.0;
  while (!history_.empty() && history_.front().t < sample.t - keep) history_.pop_front();
}

void PostureConfirmer::addImpact(double timestamp) { pending_.push_back({timestamp}); }

bool PostureConfirmer::meanDirection(bool up, double from, double to, Eigen::Vector3d* out) const {
  Eigen::Vector3d sum = Eigen::Vector3d::Zero();
  int n = 0;
  for (const Record& r : history_) {
    if (r.t < from || r.t > to) continue;
    sum += up ? r.up : r.accel;
    ++n;
  }
  if (n == 0) return false;
  const Eigen::Vector3d mean = sum / n;
  if (mean.norm() <= 0.0) return false;
  *out = mean.normalized();
  return true;
}

std::vector<double> PostureConfirmer::evaluate() {
  std::vector<double> confirmed;
  if (history_.empty()) return confirmed;
  const Record& latest = history_.back();
  const double s = latest.t;
  std::vector<Pending> keep;
  for (Pending p : pending_) {
    if (!p.has_reference) {
      // Needs the whole reference window; without it the impact is skipped
      // (as the analysis does), including the fallback.
      if (!meanDirection(true, p.t - params_.pre_start_s, p.t - params_.pre_end_s,
                         &p.reference_up)) {
        continue;
      }
      p.has_reference = true;
    }
    const double window_end = p.t + params_.max_wait_s;
    if (s >= p.t + params_.start_s && s <= window_end) {
      if (angleDeg(p.reference_up, latest.up) >= params_.threshold_deg) {
        if (!p.above) {
          p.above = true;
          p.above_since = s;
        }
        if (s - p.above_since >= params_.dwell_s) {
          confirmed.push_back(s);
          continue;
        }
      } else {
        p.above = false;
      }
    }
    if (s >= window_end) {
      // Committed settled-window rule at t + max_wait.
      Eigen::Vector3d before, after;
      if (meanDirection(false, p.t - params_.pre_start_s, p.t - params_.pre_end_s, &before) &&
          meanDirection(false, p.t + params_.fallback_start_s, window_end, &after) &&
          angleDeg(before, after) >= params_.fallback_threshold_deg) {
        confirmed.push_back(window_end);
      }
      continue;
    }
    keep.push_back(p);
  }
  pending_ = std::move(keep);
  return confirmed;
}

}  // namespace imu_motion_health
}  // namespace localization_zoo
