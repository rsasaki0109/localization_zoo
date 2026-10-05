#pragma once

// Causal fall confirmation from a VQF attitude estimate.
//
// An impact at t is confirmed when the body's up direction (6D VQF, Laidig and
// Seel 2023; papers/attitude_estimation) has turned by at least threshold_deg
// from its mean over [t - 2.0, t - 1.0] s and stayed there for dwell_s, within
// [t + start_s, t + 1.5] s. Otherwise the committed settled-window rule decides
// at t + 1.5 s: mean accelerometer direction over [t - 2.0, t - 1.0] s versus
// [t + 0.5, t + 1.5] s, at least 50 deg. Selection and evaluation are in
// evaluation/analyze_vqf_confirmation.py and evaluation/public_datasets.md.

#include "attitude_estimation/vqf.h"

#include <Eigen/Core>

#include <deque>
#include <optional>
#include <vector>

namespace localization_zoo {
namespace imu_motion_health {

struct PostureConfirmationParams {
  double threshold_deg = 50.0;
  double dwell_s = 0.1;
  double start_s = 0.0;
  double tau_acc_s = 3.0;  // VQF accelerometer time constant
  // Committed windows of the public benchmark policy.
  double pre_start_s = 2.0;
  double pre_end_s = 1.0;
  double max_wait_s = 1.5;
  double fallback_start_s = 0.5;
  double fallback_threshold_deg = 50.0;
};

class PostureConfirmer {
 public:
  explicit PostureConfirmer(const PostureConfirmationParams& params = PostureConfirmationParams());

  /// Feed every accepted sample in time order (raw gyro rad/s, accel m/s^2).
  void observe(double timestamp, const Eigen::Vector3d& gyro, const Eigen::Vector3d& accel);
  /// Register an impact that started at `timestamp` (usually the latest sample).
  void addImpact(double timestamp);
  /// Evaluate the pending impacts at the latest observed sample; returns the
  /// confirmation times reached there (each impact confirms at most once).
  std::vector<double> evaluate();

 private:
  struct Raw {
    double t;
    Eigen::Vector3d gyro, accel;
  };
  struct Record {
    double t;
    Eigen::Vector3d up;     // VQF body-frame up direction
    Eigen::Vector3d accel;  // raw accelerometer
  };
  struct Pending {
    double t;
    bool has_reference = false;
    Eigen::Vector3d reference_up = Eigen::Vector3d::Zero();
    bool above = false;
    double above_since = 0.0;
  };

  void push(const Raw& sample);
  bool meanDirection(bool up, double from, double to, Eigen::Vector3d* out) const;

  PostureConfirmationParams params_;
  // VQF needs a fixed rate: the first samples are buffered, the median
  // interval sets the rate, and the buffer is replayed.
  static constexpr std::size_t kRateSamples = 16;
  std::vector<Raw> startup_;
  std::optional<attitude_estimation::VQF> vqf_;
  std::deque<Record> history_;
  std::vector<Pending> pending_;
};

}  // namespace imu_motion_health
}  // namespace localization_zoo
