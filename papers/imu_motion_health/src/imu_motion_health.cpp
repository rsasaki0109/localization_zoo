#include "imu_motion_health/imu_motion_health.h"

#include <Eigen/SVD>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

namespace localization_zoo {
namespace imu_motion_health {
namespace {

constexpr double kEpsilon = 1e-12;
constexpr double kRadToDeg = 180.0 / 3.14159265358979323846;

double safePositive(double value, double fallback) {
  return std::isfinite(value) && value > 0.0 ? value : fallback;
}

double clamp01(double value) {
  return std::max(0.0, std::min(1.0, value));
}

double rms(const std::deque<double>& values) {
  if (values.empty()) return 0.0;
  double sum = 0.0;
  for (const double value : values) sum += value * value;
  return std::sqrt(sum / static_cast<double>(values.size()));
}

double mean(const std::deque<double>& values) {
  if (values.empty()) return 0.0;
  double sum = 0.0;
  for (const double value : values) sum += value;
  return sum / static_cast<double>(values.size());
}

double stddev(const std::deque<double>& values) {
  if (values.empty()) return 0.0;
  const double m = mean(values);
  double sum = 0.0;
  for (const double value : values) {
    const double delta = value - m;
    sum += delta * delta;
  }
  return std::sqrt(sum / static_cast<double>(values.size()));
}

Eigen::Quaterniond integrateQuaternion(const Eigen::Quaterniond& q,
                                        const Eigen::Vector3d& omega,
                                        double dt) {
  const double angle = omega.norm() * dt;
  if (!(angle > kEpsilon) || !std::isfinite(angle)) return q;
  const Eigen::AngleAxisd delta(angle, omega.normalized());
  return (q * Eigen::Quaterniond(delta)).normalized();
}

std::string jsonEscape(const std::string& value) {
  std::string escaped;
  escaped.reserve(value.size() + 8);
  for (const char c : value) {
    switch (c) {
      case '\\':
        escaped += "\\\\";
        break;
      case '"':
        escaped += "\\\"";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      default:
        escaped += c;
        break;
    }
  }
  return escaped;
}

void writeNumber(std::ostringstream& stream, double value) {
  // JSON has no NaN/Infinity literals.  Invalid input samples can still be
  // reported by the streaming API, so keep the serializer valid JSON by
  // representing non-finite measurements as null.
  if (std::isfinite(value)) {
    stream << value;
  } else {
    stream << "null";
  }
}

void writeVector(std::ostringstream& stream, const Eigen::Vector3d& value) {
  stream << '[';
  writeNumber(stream, value.x());
  stream << ',';
  writeNumber(stream, value.y());
  stream << ',';
  writeNumber(stream, value.z());
  stream << ']';
}

void writeQuaternion(std::ostringstream& stream,
                     const Eigen::Quaterniond& value) {
  // Eigen stores (w,x,y,z); retaining that order avoids ambiguity in a log.
  stream << '[';
  writeNumber(stream, value.w());
  stream << ',';
  writeNumber(stream, value.x());
  stream << ',';
  writeNumber(stream, value.y());
  stream << ',';
  writeNumber(stream, value.z());
  stream << ']';
}

}  // namespace

const char* motionStateName(MotionState state) {
  switch (state) {
    case MotionState::kInitializing:
      return "initializing";
    case MotionState::kStationary:
      return "stationary";
    case MotionState::kMoving:
      return "moving";
    case MotionState::kImpact:
      return "impact";
    case MotionState::kFall:
      return "fall";
    case MotionState::kVibration:
      return "vibration";
    case MotionState::kUnknown:
      return "unknown";
  }
  return "unknown";
}

const char* healthStateName(HealthState state) {
  switch (state) {
    case HealthState::kInitializing:
      return "initializing";
    case HealthState::kReady:
      return "ready";
    case HealthState::kDegraded:
      return "degraded";
    case HealthState::kInvalid:
      return "invalid";
  }
  return "invalid";
}

std::string toJson(const ImuMotionHealthState& state) {
  std::ostringstream stream;
  stream << std::setprecision(12);
  stream << '{';
  stream << "\"timestamp\":";
  writeNumber(stream, state.timestamp);
  stream << ",\"dt\":";
  writeNumber(stream, state.dt);
  stream << ",\"sample_rate_hz\":";
  writeNumber(stream, state.sample_rate_hz);
  stream << ",\"sample_accepted\":" << (state.sample_accepted ? "true" : "false");
  stream << ",\"integrated\":" << (state.integrated ? "true" : "false");
  stream << ",\"nonfinite\":" << (state.nonfinite ? "true" : "false");
  stream << ",\"nonmonotonic\":" << (state.nonmonotonic ? "true" : "false");
  stream << ",\"gap\":" << (state.gap ? "true" : "false");
  stream << ",\"gyro_saturated\":"
         << (state.gyro_saturated ? "true" : "false");
  stream << ",\"accel_saturated\":"
         << (state.accel_saturated ? "true" : "false");
  stream << ",\"startup_complete\":"
         << (state.startup_complete ? "true" : "false");
  stream << ",\"startup_quality_ok\":"
         << (state.startup_quality_ok ? "true" : "false");
  stream << ",\"startup_bias_trusted\":"
         << (state.startup_bias_trusted ? "true" : "false");
  stream << ",\"startup_sample_count\":" << state.startup_sample_count;
  stream << ",\"startup_gyro_std\":";
  writeNumber(stream, state.startup_gyro_std);
  stream << ",\"startup_accel_norm_std\":";
  writeNumber(stream, state.startup_accel_norm_std);
  stream << ",\"startup_gravity_norm\":";
  writeNumber(stream, state.startup_gravity_norm);
  stream << ",\"motion_state\":\"" << motionStateName(state.motion_state)
         << "\"";
  stream << ",\"health_state\":\"" << healthStateName(state.health_state)
         << "\"";
  stream << ",\"stationary\":" << (state.stationary ? "true" : "false");
  stream << ",\"moving\":" << (state.moving ? "true" : "false");
  stream << ",\"impact\":" << (state.impact ? "true" : "false");
  stream << ",\"fall\":" << (state.fall ? "true" : "false");
  stream << ",\"vibration\":" << (state.vibration ? "true" : "false");
  stream << ",\"degraded\":" << (state.degraded ? "true" : "false");
  stream << ",\"confidence\":";
  writeNumber(stream, state.confidence);
  stream << ",\"gyro_norm\":";
  writeNumber(stream, state.gyro_norm);
  stream << ",\"accel_norm\":";
  writeNumber(stream, state.accel_norm);
  stream << ",\"linear_accel_norm\":";
  writeNumber(stream, state.linear_accel_norm);
  stream << ",\"vibration_rms\":";
  writeNumber(stream, state.vibration_rms);
  stream << ",\"freefall_duration_s\":";
  writeNumber(stream, state.freefall_duration_s);
  stream << ",\"tilt_angle_rad\":";
  writeNumber(stream, state.tilt_angle_rad);
  stream << ",\"tilt_angle_deg\":";
  writeNumber(stream, state.tilt_angle_deg);
  stream << ",\"tilted\":" << (state.tilted ? "true" : "false");
  stream << ",\"gyro_bias\":";
  writeVector(stream, state.gyro_bias);
  stream << ",\"accel_bias\":";
  writeVector(stream, state.accel_bias);
  stream << ",\"gravity_world\":";
  writeVector(stream, state.gravity_world);
  stream << ",\"gravity_magnitude\":";
  writeNumber(stream, state.gravity_magnitude);
  stream << ",\"orientation_wxyz\":";
  writeQuaternion(stream, state.orientation);
  stream << ",\"velocity\":";
  writeVector(stream, state.velocity);
  stream << ",\"position\":";
  writeVector(stream, state.position);
  stream << ",\"relative_velocity\":";
  writeVector(stream, state.relative_velocity);
  stream << ",\"relative_position\":";
  writeVector(stream, state.relative_position);
  stream << ",\"diagnostic\":\"" << jsonEscape(state.diagnostic) << "\"";
  stream << '}';
  return stream.str();
}

ImuMotionHealth::ImuMotionHealth(const ImuMotionHealthParams& params)
    : params_(params) {
  params_.startup_duration_s =
      std::max(0.0, std::isfinite(params_.startup_duration_s)
                         ? params_.startup_duration_s
                         : 1.0);
  params_.startup_min_samples = std::max<std::size_t>(1, params_.startup_min_samples);
  params_.gravity_magnitude =
      safePositive(params_.gravity_magnitude, 9.80665);
  params_.max_gap_s = safePositive(params_.max_gap_s, 0.25);
  params_.min_dt_s = safePositive(params_.min_dt_s, 1e-7);
  params_.max_integration_dt_s =
      safePositive(params_.max_integration_dt_s, 0.10);
  params_.tilt_angle_threshold_deg =
      safePositive(params_.tilt_angle_threshold_deg, 55.0);
  params_.tilt_angle_threshold_deg =
      std::min(179.0, std::max(1.0, params_.tilt_angle_threshold_deg));
  params_.tilt_min_duration_s =
      std::max(0.0, std::isfinite(params_.tilt_min_duration_s)
                         ? params_.tilt_min_duration_s
                         : 0.15);
  params_.motion_window_samples =
      std::max<std::size_t>(1, params_.motion_window_samples);
  params_.gyro_saturation_rad_s =
      safePositive(params_.gyro_saturation_rad_s, 34.0);
  params_.accel_saturation_mps2 =
      safePositive(params_.accel_saturation_mps2, 156.9);
  params_.stationary_bias_gain =
      clamp01(std::isfinite(params_.stationary_bias_gain)
                  ? params_.stationary_bias_gain
                  : 0.01);
  params_.leveling_gain =
      clamp01(std::isfinite(params_.leveling_gain) ? params_.leveling_gain
                                                   : 0.03);
  reset();
}

void ImuMotionHealth::clearRuntimeState() {
  state_ = ImuMotionHealthState();
  state_.gravity_magnitude = params_.gravity_magnitude;
  state_.gravity_world = Eigen::Vector3d(0.0, 0.0, params_.gravity_magnitude);
  counters_ = ImuMotionHealthCounters();
  have_timestamp_ = false;
  last_timestamp_ = 0.0;
  first_timestamp_ = 0.0;
  last_integration_timestamp_ = 0.0;
  anomaly_until_ = 0.0;
  have_anomaly_timestamp_ = false;
  startup_complete_ = false;
  startup_quality_ok_ = false;
  startup_had_data_error_ = false;
  startup_samples_.clear();
  freefall_start_timestamp_ = 0.0;
  freefall_active_ = false;
  impact_until_ = 0.0;
  fall_until_ = 0.0;
  vibration_until_ = 0.0;
  tilt_active_ = false;
  tilt_start_timestamp_ = 0.0;
  gyro_norm_window_.clear();
  accel_norm_window_.clear();
  linear_accel_norm_window_.clear();
  accel_window_.clear();
  sample_rate_ema_hz_ = 0.0;
  orientation_ = Eigen::Quaterniond::Identity();
  velocity_.setZero();
  position_.setZero();
  gyro_bias_.setZero();
  accel_bias_.setZero();
  gravity_world_ = Eigen::Vector3d(0.0, 0.0, params_.gravity_magnitude);
  gravity_magnitude_ = params_.gravity_magnitude;
}

void ImuMotionHealth::reset() { clearRuntimeState(); }

void ImuMotionHealth::finalizeStartup() {
  if (startup_complete_ || startup_samples_.empty()) return;

  Eigen::Vector3d gyro_mean = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel_mean = Eigen::Vector3d::Zero();
  for (const ImuSample& sample : startup_samples_) {
    gyro_mean += sample.gyro;
    accel_mean += sample.accel;
  }
  const double count = static_cast<double>(startup_samples_.size());
  gyro_mean /= count;
  accel_mean /= count;

  double gyro_variance = 0.0;
  double accel_norm_mean = 0.0;
  for (const ImuSample& sample : startup_samples_) {
    gyro_variance += (sample.gyro - gyro_mean).squaredNorm();
    accel_norm_mean += sample.accel.norm();
  }
  gyro_variance /= count;
  accel_norm_mean /= count;
  double accel_norm_variance = 0.0;
  for (const ImuSample& sample : startup_samples_) {
    const double delta = sample.accel.norm() - accel_norm_mean;
    accel_norm_variance += delta * delta;
  }
  accel_norm_variance /= count;

  state_.startup_sample_count = startup_samples_.size();
  state_.startup_gyro_std = std::sqrt(std::max(0.0, gyro_variance));
  state_.startup_accel_norm_std =
      std::sqrt(std::max(0.0, accel_norm_variance));
  state_.startup_gravity_norm = accel_norm_mean;

  const bool enough_samples =
      startup_samples_.size() >= params_.startup_min_samples;
  const bool gyro_quiet =
      state_.startup_gyro_std <= params_.startup_max_gyro_std;
  const bool accel_quiet =
      state_.startup_accel_norm_std <= params_.startup_max_accel_norm_std;
  const bool gravity_plausible =
      std::abs(accel_norm_mean - params_.gravity_magnitude) <=
      params_.startup_gravity_tolerance;
  startup_quality_ok_ = enough_samples && gyro_quiet && accel_quiet &&
                        gravity_plausible && !startup_had_data_error_ &&
                        accel_norm_mean > kEpsilon;
  startup_complete_ = true;
  state_.startup_quality_ok = startup_quality_ok_;
  state_.startup_bias_trusted = startup_quality_ok_;

  // A noisy/moving startup is still allowed to transition to the streaming
  // mode, but its means are not silently promoted to calibration constants.
  gyro_bias_ = (params_.estimate_gyro_bias && startup_quality_ok_)
                   ? gyro_mean
                   : Eigen::Vector3d::Zero();

  const double mean_norm = accel_mean.norm();
  Eigen::Vector3d accel_for_alignment = accel_mean;
  if (params_.estimate_accel_bias && startup_quality_ok_ &&
      mean_norm > kEpsilon) {
    gravity_magnitude_ = params_.gravity_magnitude;
    const Eigen::Vector3d direction = accel_mean / mean_norm;
    accel_bias_ = accel_mean - direction * gravity_magnitude_;
    accel_for_alignment = accel_mean - accel_bias_;
  } else {
    accel_bias_.setZero();
    gravity_magnitude_ = mean_norm > kEpsilon ? mean_norm
                                             : params_.gravity_magnitude;
  }

  if (accel_for_alignment.norm() > kEpsilon) {
    const Eigen::Vector3d direction = accel_for_alignment.normalized();
    orientation_ = Eigen::Quaterniond::FromTwoVectors(
                        direction, Eigen::Vector3d::UnitZ())
                        .normalized();
  } else {
    orientation_ = Eigen::Quaterniond::Identity();
  }
  gravity_world_ = Eigen::Vector3d(0.0, 0.0, gravity_magnitude_);
  velocity_.setZero();
  position_.setZero();
  last_integration_timestamp_ = last_timestamp_;

  state_.startup_complete = true;
  state_.startup_quality_ok = startup_quality_ok_;
  state_.startup_bias_trusted = startup_quality_ok_;
}

void ImuMotionHealth::classify(double timestamp, double /*dt*/,
                               bool had_data_error,
                               const Eigen::Vector3d& corrected_accel) {
  const std::size_t max_samples =
      std::max<std::size_t>(1, params_.motion_window_samples);
  while (gyro_norm_window_.size() > max_samples) gyro_norm_window_.pop_front();
  while (accel_norm_window_.size() > max_samples)
    accel_norm_window_.pop_front();
  while (linear_accel_norm_window_.size() > max_samples)
    linear_accel_norm_window_.pop_front();
  while (accel_window_.size() > max_samples) accel_window_.pop_front();

  const double gyro_rms = rms(gyro_norm_window_);
  const double accel_mean = mean(accel_norm_window_);
  const double accel_std = stddev(accel_norm_window_);
  const double linear_rms = rms(linear_accel_norm_window_);

  Eigen::Vector3d accel_vector_mean = Eigen::Vector3d::Zero();
  for (const Eigen::Vector3d& value : accel_window_) accel_vector_mean += value;
  if (!accel_window_.empty()) {
    accel_vector_mean /= static_cast<double>(accel_window_.size());
  }
  double vibration_sum = 0.0;
  for (const Eigen::Vector3d& value : accel_window_)
    vibration_sum += (value - accel_vector_mean).squaredNorm();
  const double vibration_rms = accel_window_.empty()
                                   ? 0.0
                                   : std::sqrt(vibration_sum /
                                               static_cast<double>(accel_window_.size()));
  state_.vibration_rms = std::max(vibration_rms, accel_std);

  const bool freefall_now =
      state_.accel_norm <= params_.fall_freefall_threshold;
  if (freefall_now) {
    if (!freefall_active_) freefall_start_timestamp_ = timestamp;
    freefall_active_ = true;
  } else if (freefall_active_) {
    const double duration = timestamp - freefall_start_timestamp_;
    if (duration >= params_.fall_min_duration_s) {
      fall_until_ = std::max(fall_until_,
                             timestamp + params_.event_hold_duration_s);
    }
    freefall_active_ = false;
  }
  state_.freefall_duration_s =
      freefall_active_ ? std::max(0.0, timestamp - freefall_start_timestamp_)
                       : 0.0;
  if (freefall_active_ && state_.freefall_duration_s >=
                              params_.fall_min_duration_s) {
    fall_until_ = std::max(fall_until_,
                           timestamp + params_.event_hold_duration_s);
  }

  const bool impact_now =
      state_.accel_norm >= params_.impact_accel_threshold ||
      state_.gyro_norm >= params_.impact_gyro_threshold;
  if (impact_now) {
    impact_until_ = std::max(impact_until_,
                             timestamp + params_.event_hold_duration_s);
  }

  const bool vibration_now =
      state_.vibration_rms >= params_.vibration_rms_threshold &&
      accel_window_.size() >= std::min<std::size_t>(
                                  3, std::max<std::size_t>(1, max_samples));
  if (vibration_now) {
    vibration_until_ = std::max(vibration_until_,
                                timestamp + params_.event_hold_duration_s);
  }

  // Tilt is evaluated from two complementary signals.  The nominal
  // orientation captures a pose change when the gyro was integrated; the
  // gravity direction catches a post-impact pose change when the gyro sample
  // was missed or the device simply comes to rest before the orientation can
  // move.  The latter is gated by a quiet, near-|g| window so translational
  // acceleration is not mislabeled as a fall.
  const auto angleToUp = [](const Eigen::Vector3d& direction) {
    if (direction.norm() <= kEpsilon) return 0.0;
    const double cosine = std::max(-1.0, std::min(1.0,
                                                   direction.normalized().z()));
    return std::acos(cosine);
  };
  const double orientation_tilt =
      angleToUp(orientation_ * Eigen::Vector3d::UnitZ());
  const double gravity_tilt = angleToUp(orientation_ * corrected_accel);
  state_.tilt_angle_rad = std::max(orientation_tilt, gravity_tilt);
  state_.tilt_angle_deg = state_.tilt_angle_rad * kRadToDeg;

  const bool quiet_for_tilt =
      !had_data_error && !impact_now && !freefall_now &&
      gyro_rms <= std::max(0.10, 2.0 * params_.stationary_gyro_threshold) &&
      std::abs(accel_mean - gravity_magnitude_) <=
          2.0 * params_.stationary_accel_tolerance &&
      state_.vibration_rms < params_.vibration_rms_threshold;
  const bool tilt_candidate =
      quiet_for_tilt &&
      state_.tilt_angle_deg >= params_.tilt_angle_threshold_deg;
  if (tilt_candidate) {
    if (!tilt_active_) tilt_start_timestamp_ = timestamp;
    tilt_active_ = true;
    const double tilt_duration =
        std::max(0.0, timestamp - tilt_start_timestamp_);
    if (tilt_duration >= params_.tilt_min_duration_s) {
      fall_until_ = std::max(fall_until_,
                             timestamp + params_.event_hold_duration_s);
    }
    state_.tilted = tilt_duration >= params_.tilt_min_duration_s;
  } else {
    tilt_active_ = false;
    state_.tilted = false;
  }

  const bool stationary_now =
      !had_data_error && !impact_now && !freefall_now && !state_.tilted &&
      gyro_rms <= params_.stationary_gyro_threshold &&
      std::abs(accel_mean - gravity_magnitude_) <=
          params_.stationary_accel_tolerance &&
      linear_rms <= params_.stationary_accel_tolerance &&
      state_.vibration_rms < params_.vibration_rms_threshold;
  const bool fall_now = fall_until_ >= timestamp;
  const bool impact = impact_until_ >= timestamp;
  const bool vibration = vibration_until_ >= timestamp;
  const bool moving_now =
      !stationary_now && !impact && !fall_now && !vibration &&
      (linear_rms >= params_.moving_accel_threshold ||
       gyro_rms >= params_.moving_gyro_threshold ||
       state_.linear_accel_norm >= params_.moving_accel_threshold);

  if (!startup_complete_) {
    state_.motion_state = MotionState::kInitializing;
  } else if (fall_now) {
    state_.motion_state = MotionState::kFall;
  } else if (impact) {
    state_.motion_state = MotionState::kImpact;
  } else if (vibration) {
    state_.motion_state = MotionState::kVibration;
  } else if (stationary_now) {
    state_.motion_state = MotionState::kStationary;
  } else if (moving_now) {
    state_.motion_state = MotionState::kMoving;
  } else {
    state_.motion_state = MotionState::kUnknown;
  }

  if (state_.motion_state == MotionState::kStationary)
    ++counters_.stationary_samples;
  if (state_.motion_state == MotionState::kMoving)
    ++counters_.moving_samples;
  if (state_.motion_state == MotionState::kImpact)
    ++counters_.impact_samples;
  if (state_.motion_state == MotionState::kFall)
    ++counters_.fall_samples;
  if (state_.motion_state == MotionState::kVibration)
    ++counters_.vibration_samples;

  state_.startup_complete = startup_complete_;
  state_.startup_quality_ok = startup_quality_ok_;
  state_.startup_bias_trusted = startup_quality_ok_;
  state_.gyro_bias = gyro_bias_;
  state_.accel_bias = accel_bias_;
  state_.gravity_world = gravity_world_;
  state_.gravity_magnitude = gravity_magnitude_;
  state_.orientation = orientation_;
  state_.velocity = velocity_;
  state_.position = position_;
  state_.relative_velocity = velocity_;
  state_.relative_position = position_;
}

void ImuMotionHealth::updateStateAliases() {
  state_.stationary = state_.motion_state == MotionState::kStationary;
  state_.moving = state_.motion_state == MotionState::kMoving;
  state_.impact = state_.motion_state == MotionState::kImpact;
  state_.fall = state_.motion_state == MotionState::kFall;
  state_.vibration = state_.motion_state == MotionState::kVibration;
  state_.degraded = state_.health_state != HealthState::kReady;
  state_.gyro_bias = gyro_bias_;
  state_.accel_bias = accel_bias_;
  state_.gravity_world = gravity_world_;
  state_.gravity_magnitude = gravity_magnitude_;
  state_.orientation = orientation_;
  state_.velocity = velocity_;
  state_.position = position_;
  state_.relative_velocity = velocity_;
  state_.relative_position = position_;
  state_.sample_rate_hz = sample_rate_ema_hz_;
}

void ImuMotionHealth::updateConfidence(bool had_data_error) {
  double confidence = startup_complete_ ? (startup_quality_ok_ ? 0.92 : 0.42)
                                        : 0.15;
  if (state_.motion_state == MotionState::kUnknown) confidence -= 0.08;
  if (state_.motion_state == MotionState::kMoving) confidence -= 0.08;
  if (state_.motion_state == MotionState::kVibration) confidence -= 0.18;
  if (state_.motion_state == MotionState::kImpact ||
      state_.motion_state == MotionState::kFall)
    confidence -= 0.25;
  if (had_data_error) confidence -= 0.35;
  if (state_.health_state == HealthState::kInvalid) confidence = 0.0;
  state_.confidence = clamp01(confidence);
}

void ImuMotionHealth::makeDiagnostic(bool had_data_error) {
  std::vector<std::string> messages;
  if (state_.nonfinite) messages.emplace_back("nonfinite sample");
  if (state_.nonmonotonic) messages.emplace_back("nonmonotonic timestamp");
  if (state_.gap) messages.emplace_back("timestamp gap");
  if (state_.gyro_saturated) messages.emplace_back("gyro saturation");
  if (state_.accel_saturated) messages.emplace_back("accel saturation");
  if (had_data_error && messages.empty()) messages.emplace_back("data error");
  if (startup_complete_ && !startup_quality_ok_)
    messages.emplace_back("startup quality gate failed");
  if (state_.motion_state == MotionState::kImpact)
    messages.emplace_back("impact detected");
  if (state_.motion_state == MotionState::kFall)
    messages.emplace_back(state_.tilted ? "fall/tilt detected"
                                        : "fall/freefall detected");
  if (state_.motion_state == MotionState::kVibration)
    messages.emplace_back("vibration detected");

  state_.diagnostic.clear();
  for (std::size_t i = 0; i < messages.size(); ++i) {
    if (i != 0) state_.diagnostic += "; ";
    state_.diagnostic += messages[i];
  }
}

ImuMotionHealthState ImuMotionHealth::process(const ImuSample& sample) {
  ++counters_.samples;

  // Per-sample flags are not sticky.  Counters provide the historical view;
  // a consumer can therefore treat state_ as an edge-triggered diagnostic.
  state_.sample_accepted = false;
  state_.integrated = false;
  state_.nonfinite = false;
  state_.nonmonotonic = false;
  state_.gap = false;
  state_.gyro_saturated = false;
  state_.accel_saturated = false;
  state_.saturated = false;
  state_.dt = 0.0;
  state_.timestamp = sample.timestamp;
  state_.diagnostic.clear();

  const bool finite = std::isfinite(sample.timestamp) &&
                      sample.gyro.allFinite() && sample.accel.allFinite();
  if (!finite) {
    ++counters_.nonfinite_samples;
    ++counters_.rejected_samples;
    state_.nonfinite = true;
    state_.health_state =
        startup_complete_ ? HealthState::kInvalid : HealthState::kInitializing;
    startup_had_data_error_ = true;
    makeDiagnostic(true);
    updateStateAliases();
    updateConfidence(true);
    return state_;
  }

  if (have_timestamp_ && sample.timestamp <= last_timestamp_) {
    ++counters_.nonmonotonic_timestamps;
    ++counters_.rejected_samples;
    state_.nonmonotonic = true;
    state_.health_state = startup_complete_ ? HealthState::kDegraded
                                            : HealthState::kInitializing;
    startup_had_data_error_ = true;
    anomaly_until_ = last_timestamp_ + params_.event_hold_duration_s;
    have_anomaly_timestamp_ = true;
    makeDiagnostic(true);
    updateStateAliases();
    updateConfidence(true);
    return state_;
  }

  double dt = 0.0;
  if (have_timestamp_) {
    dt = sample.timestamp - last_timestamp_;
    state_.dt = dt;
    if (dt > params_.max_gap_s) {
      ++counters_.timestamp_gaps;
      state_.gap = true;
      startup_had_data_error_ = true;
      anomaly_until_ = sample.timestamp + params_.event_hold_duration_s;
      have_anomaly_timestamp_ = true;
    }
    if (dt > params_.min_dt_s && std::isfinite(dt)) {
      const double hz = 1.0 / dt;
      sample_rate_ema_hz_ =
          sample_rate_ema_hz_ == 0.0 ? hz : 0.9 * sample_rate_ema_hz_ + 0.1 * hz;
    }
  } else {
    have_timestamp_ = true;
    first_timestamp_ = sample.timestamp;
  }
  last_timestamp_ = sample.timestamp;

  state_.gyro_saturated =
      sample.gyro.cwiseAbs().maxCoeff() >= params_.gyro_saturation_rad_s;
  state_.accel_saturated =
      sample.accel.cwiseAbs().maxCoeff() >= params_.accel_saturation_mps2;
  state_.saturated = state_.gyro_saturated || state_.accel_saturated;
  if (state_.gyro_saturated) ++counters_.gyro_saturated_samples;
  if (state_.accel_saturated) ++counters_.accel_saturated_samples;
  if (state_.saturated) {
    ++counters_.rejected_samples;
    startup_had_data_error_ = true;
    anomaly_until_ = sample.timestamp + params_.event_hold_duration_s;
    have_anomaly_timestamp_ = true;
  } else {
    ++counters_.accepted_samples;
    state_.sample_accepted = true;
  }

  // Startup samples are retained as raw values because the bias and leveling
  // frame do not exist yet.  Saturated samples stay in the evidence window,
  // but startup_had_data_error_ prevents them from becoming calibration.
  if (!startup_complete_ && !state_.saturated) startup_samples_.push_back(sample);

  const Eigen::Vector3d corrected_gyro = sample.gyro - gyro_bias_;
  const Eigen::Vector3d corrected_accel = sample.accel - accel_bias_;
  state_.gyro_norm = corrected_gyro.norm();
  state_.accel_norm = corrected_accel.norm();
  const Eigen::Vector3d world_specific = orientation_ * corrected_accel;
  const Eigen::Vector3d world_linear = world_specific - gravity_world_;
  state_.linear_accel_norm = world_linear.norm();
  gyro_norm_window_.push_back(state_.gyro_norm);
  accel_norm_window_.push_back(state_.accel_norm);
  linear_accel_norm_window_.push_back(state_.linear_accel_norm);
  accel_window_.push_back(corrected_accel);

  if (!startup_complete_) {
    const bool enough_time =
        params_.startup_duration_s <= 0.0 ||
        sample.timestamp - first_timestamp_ >= params_.startup_duration_s;
    const bool enough_samples =
        startup_samples_.size() >= params_.startup_min_samples;
    if (!state_.saturated && enough_time && enough_samples) finalizeStartup();
    state_.motion_state = startup_complete_ ? MotionState::kUnknown
                                            : MotionState::kInitializing;
    state_.health_state = startup_complete_
                              ? (startup_quality_ok_ ? HealthState::kReady
                                                     : HealthState::kDegraded)
                              : HealthState::kInitializing;
    updateStateAliases();
    updateConfidence(state_.gap || state_.saturated);
    makeDiagnostic(state_.gap || state_.saturated);
    return state_;
  }

  const bool had_data_error = state_.gap || state_.saturated;
  classify(sample.timestamp, dt, had_data_error, corrected_accel);

  // A stationary sample is the only time at which the gyro bias is
  // observable without an external reference.  Apply the update before
  // propagation so the current sample benefits from it as well.
  if (state_.motion_state == MotionState::kStationary &&
      params_.estimate_gyro_bias && params_.stationary_bias_gain > 0.0) {
    gyro_bias_ = (1.0 - params_.stationary_bias_gain) * gyro_bias_ +
                 params_.stationary_bias_gain * sample.gyro;
  }

  const Eigen::Vector3d corrected_gyro_now = sample.gyro - gyro_bias_;
  const Eigen::Vector3d corrected_accel_now = sample.accel - accel_bias_;
  const bool can_integrate =
      state_.sample_accepted && !had_data_error && dt > params_.min_dt_s &&
      dt <= params_.max_integration_dt_s;
  if (can_integrate) {
    orientation_ = integrateQuaternion(orientation_, corrected_gyro_now, dt);
    const Eigen::Vector3d world_accel =
        orientation_ * corrected_accel_now - gravity_world_;
    if (state_.motion_state == MotionState::kStationary &&
        params_.zero_velocity_when_stationary) {
      // Keep the relative anchor stable while parked.  The orientation still
      // propagates (and is gravity-levelled below), so a stationary turn is
      // not confused with a frozen attitude.
      velocity_.setZero();
    } else {
      position_ += velocity_ * dt + 0.5 * world_accel * dt * dt;
      velocity_ += world_accel * dt;
    }
    state_.integrated = true;
  }

  // Complementary gravity leveling: rotate measured world gravity toward +Z
  // by a small horizontal correction.  Left multiplication preserves yaw in
  // the small-error limit and never pretends acceleration observes heading.
  if (state_.motion_state == MotionState::kStationary &&
      params_.leveling_gain > 0.0 && corrected_accel_now.norm() > kEpsilon) {
    const Eigen::Vector3d measured =
        (orientation_ * corrected_accel_now).normalized();
    const Eigen::Vector3d target = Eigen::Vector3d::UnitZ();
    const Eigen::Vector3d axis = measured.cross(target);
    const double sine = axis.norm();
    const double cosine = std::max(-1.0, std::min(1.0, measured.dot(target)));
    if (sine > kEpsilon) {
      const double angle = std::atan2(sine, cosine);
      orientation_ =
          (Eigen::Quaterniond(Eigen::AngleAxisd(
               params_.leveling_gain * angle, axis / sine)) * orientation_)
              .normalized();
    }
  }

  // A gap or saturation intentionally leaves the nominal state unchanged.
  // A too-large-but-not-gap interval also gets discarded by the integration
  // guard; the health state below explains that the stream is degraded only
  // for the explicit gap/saturation cases.
  state_.health_state = HealthState::kReady;
  if (!startup_quality_ok_ || had_data_error ||
      (have_anomaly_timestamp_ && sample.timestamp < anomaly_until_)) {
    state_.health_state = HealthState::kDegraded;
  }
  state_.timestamp = sample.timestamp;
  state_.dt = dt;
  updateStateAliases();
  updateConfidence(had_data_error);
  makeDiagnostic(had_data_error);
  return state_;
}

}  // namespace imu_motion_health
}  // namespace localization_zoo
