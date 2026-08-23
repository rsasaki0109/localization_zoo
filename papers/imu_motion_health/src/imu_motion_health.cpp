#include "imu_motion_health/imu_motion_health.h"

#include <Eigen/SVD>

#include <algorithm>
#include <cmath>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
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

void writeNullableNumber(std::ostringstream& stream, bool present,
                         double value) {
  if (present) {
    writeNumber(stream, value);
  } else {
    stream << "null";
  }
}

std::string trimCopy(const std::string& value) {
  std::size_t first = 0;
  while (first < value.size() &&
         std::isspace(static_cast<unsigned char>(value[first]))) {
    ++first;
  }
  std::size_t last = value.size();
  while (last > first &&
         std::isspace(static_cast<unsigned char>(value[last - 1]))) {
    --last;
  }
  return value.substr(first, last - first);
}

std::string lowerCopy(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) {
                   return static_cast<char>(std::tolower(c));
                 });
  return value;
}

bool parseYamlNumber(const std::string& text, double* value) {
  const std::string token = trimCopy(text);
  if (token.empty()) return false;
  std::size_t consumed = 0;
  try {
    *value = std::stod(token, &consumed);
  } catch (const std::exception&) {
    return false;
  }
  return consumed == token.size() && std::isfinite(*value);
}

bool parseYamlSize(const std::string& text, std::size_t* value) {
  const std::string token = trimCopy(text);
  if (token.empty() || token[0] == '-') return false;
  std::size_t consumed = 0;
  try {
    const unsigned long long parsed = std::stoull(token, &consumed);
    if (consumed != token.size() ||
        parsed > static_cast<unsigned long long>(
                     std::numeric_limits<std::size_t>::max())) {
      return false;
    }
    *value = static_cast<std::size_t>(parsed);
  } catch (const std::exception&) {
    return false;
  }
  return true;
}

bool parseYamlBool(const std::string& text, bool* value) {
  const std::string token = lowerCopy(trimCopy(text));
  if (token == "true") {
    *value = true;
    return true;
  }
  if (token == "false") {
    *value = false;
    return true;
  }
  return false;
}

std::string unquoteYamlScalar(const std::string& text) {
  const std::string token = trimCopy(text);
  if (token.size() >= 2 &&
      ((token.front() == '\'' && token.back() == '\'') ||
       (token.front() == '"' && token.back() == '"'))) {
    return token.substr(1, token.size() - 2);
  }
  return token;
}

bool profileKeyIsBool(const std::string& key) {
  return key == "estimate_gyro_bias" || key == "estimate_accel_bias" ||
         key == "zero_velocity_when_stationary" ||
         key == "emit_moving_events";
}

bool profileKeyIsSize(const std::string& key) {
  return key == "startup_min_samples" || key == "motion_window_samples" ||
         key == "event_queue_capacity";
}

// Assign one scalar profile field.  Return false for an unknown key so that
// typos in a deployment file cannot silently change detector behaviour.
bool assignProfileField(const std::string& raw_key, const std::string& raw_value,
                        ImuMotionHealthParams* params, std::string* error) {
  std::string key = lowerCopy(trimCopy(raw_key));
  std::replace(key.begin(), key.end(), '-', '_');
  const std::string value = unquoteYamlScalar(raw_value);

  // A few natural spellings are accepted, but all map to one documented
  // parameter and remain strict (there is no catch-all option).
  if (key == "startup_duration" || key == "startup_duration_s")
    key = "startup_duration_s";
  else if (key == "gravity" || key == "gravity_magnitude")
    key = "gravity_magnitude";
  else if (key == "max_gap" || key == "max_gap_s")
    key = "max_gap_s";
  else if (key == "max_integration_dt" || key == "max_integration_dt_s")
    key = "max_integration_dt_s";
  else if (key == "gyro_saturation" || key == "gyro_saturation_rad_s")
    key = "gyro_saturation_rad_s";
  else if (key == "accel_saturation" || key == "accel_saturation_mps2")
    key = "accel_saturation_mps2";
  else if (key == "stationary_gyro")
    key = "stationary_gyro_threshold";
  else if (key == "stationary_accel")
    key = "stationary_accel_tolerance";
  else if (key == "moving_gyro")
    key = "moving_gyro_threshold";
  else if (key == "moving_accel")
    key = "moving_accel_threshold";
  else if (key == "impact_accel")
    key = "impact_accel_threshold";
  else if (key == "impact_gyro")
    key = "impact_gyro_threshold";
  else if (key == "fall_freefall")
    key = "fall_freefall_threshold";
  else if (key == "fall_min_duration")
    key = "fall_min_duration_s";
  else if (key == "tilt_angle" || key == "tilt_angle_threshold")
    key = "tilt_angle_threshold_deg";
  else if (key == "tilt_min_duration")
    key = "tilt_min_duration_s";
  else if (key == "event_hold" || key == "event_hold_duration")
    key = "event_hold_duration_s";
  else if (key == "vibration_rms")
    key = "vibration_rms_threshold";
  else if (key == "stationary_bias")
    key = "stationary_bias_gain";
  else if (key == "leveling")
    key = "leveling_gain";
  else if (key == "bias_jump_threshold" ||
           key == "gyro_bias_jump_threshold_rad_s")
    key = "gyro_bias_jump_threshold";
  else if (key == "bias_jump_min_duration" || key == "bias_jump_min_duration_s" ||
           key == "gyro_bias_jump_min_duration")
    key = "gyro_bias_jump_min_duration_s";

  if (profileKeyIsBool(key)) {
    bool parsed = false;
    if (!parseYamlBool(value, &parsed)) {
      if (error) *error = "profile key '" + raw_key + "' expects true or false";
      return false;
    }
    if (key == "estimate_gyro_bias") params->estimate_gyro_bias = parsed;
    if (key == "estimate_accel_bias") params->estimate_accel_bias = parsed;
    if (key == "zero_velocity_when_stationary")
      params->zero_velocity_when_stationary = parsed;
    if (key == "emit_moving_events") params->emit_moving_events = parsed;
    return true;
  }

  if (profileKeyIsSize(key)) {
    std::size_t parsed = 0;
    if (!parseYamlSize(value, &parsed) || parsed == 0) {
      if (error) *error = "profile key '" + raw_key + "' expects a positive integer";
      return false;
    }
    if (key == "startup_min_samples") params->startup_min_samples = parsed;
    if (key == "motion_window_samples") params->motion_window_samples = parsed;
    if (key == "event_queue_capacity") params->event_queue_capacity = parsed;
    return true;
  }

  const std::map<std::string, double*> fields = {
      {"startup_duration_s", &params->startup_duration_s},
      {"gravity_magnitude", &params->gravity_magnitude},
      {"startup_max_gyro_std", &params->startup_max_gyro_std},
      {"startup_max_accel_norm_std", &params->startup_max_accel_norm_std},
      {"startup_gravity_tolerance", &params->startup_gravity_tolerance},
      {"max_gap_s", &params->max_gap_s},
      {"min_dt_s", &params->min_dt_s},
      {"gyro_saturation_rad_s", &params->gyro_saturation_rad_s},
      {"accel_saturation_mps2", &params->accel_saturation_mps2},
      {"stationary_gyro_threshold", &params->stationary_gyro_threshold},
      {"stationary_accel_tolerance", &params->stationary_accel_tolerance},
      {"moving_gyro_threshold", &params->moving_gyro_threshold},
      {"moving_accel_threshold", &params->moving_accel_threshold},
      {"impact_accel_threshold", &params->impact_accel_threshold},
      {"impact_gyro_threshold", &params->impact_gyro_threshold},
      {"fall_freefall_threshold", &params->fall_freefall_threshold},
      {"fall_min_duration_s", &params->fall_min_duration_s},
      {"event_hold_duration_s", &params->event_hold_duration_s},
      {"tilt_angle_threshold_deg", &params->tilt_angle_threshold_deg},
      {"tilt_min_duration_s", &params->tilt_min_duration_s},
      {"vibration_rms_threshold", &params->vibration_rms_threshold},
      {"stationary_bias_gain", &params->stationary_bias_gain},
      {"leveling_gain", &params->leveling_gain},
      {"gyro_bias_jump_threshold", &params->gyro_bias_jump_threshold},
      {"gyro_bias_jump_min_duration_s",
       &params->gyro_bias_jump_min_duration_s},
      {"max_integration_dt_s", &params->max_integration_dt_s},
  };
  const auto found = fields.find(key);
  if (found == fields.end()) {
    if (error) *error = "unknown profile key: " + raw_key;
    return false;
  }
  double parsed = 0.0;
  if (!parseYamlNumber(value, &parsed)) {
    if (error) *error = "profile key '" + raw_key + "' expects a finite number";
    return false;
  }
  *found->second = parsed;
  return true;
}

bool validateProfileParams(const ImuMotionHealthParams& params,
                           std::string* error) {
  const auto positive = [&](double value, const char* key) {
    if (std::isfinite(value) && value > 0.0) return true;
    if (error) *error = std::string("profile key '") + key + "' must be positive";
    return false;
  };
  const auto nonnegative = [&](double value, const char* key) {
    if (std::isfinite(value) && value >= 0.0) return true;
    if (error)
      *error = std::string("profile key '") + key + "' must be non-negative";
    return false;
  };
  if (!nonnegative(params.startup_duration_s, "startup_duration_s") ||
      params.startup_min_samples == 0 ||
      !positive(params.gravity_magnitude, "gravity_magnitude") ||
      !positive(params.startup_max_gyro_std, "startup_max_gyro_std") ||
      !positive(params.startup_max_accel_norm_std,
                "startup_max_accel_norm_std") ||
      !positive(params.startup_gravity_tolerance,
                "startup_gravity_tolerance") ||
      !positive(params.max_gap_s, "max_gap_s") ||
      !positive(params.min_dt_s, "min_dt_s") ||
      !positive(params.gyro_saturation_rad_s, "gyro_saturation_rad_s") ||
      !positive(params.accel_saturation_mps2, "accel_saturation_mps2") ||
      !positive(params.stationary_gyro_threshold,
                "stationary_gyro_threshold") ||
      !positive(params.stationary_accel_tolerance,
                "stationary_accel_tolerance") ||
      !positive(params.moving_gyro_threshold, "moving_gyro_threshold") ||
      !positive(params.moving_accel_threshold, "moving_accel_threshold") ||
      !positive(params.impact_accel_threshold, "impact_accel_threshold") ||
      !positive(params.impact_gyro_threshold, "impact_gyro_threshold") ||
      !positive(params.fall_freefall_threshold, "fall_freefall_threshold") ||
      !nonnegative(params.fall_min_duration_s, "fall_min_duration_s") ||
      !nonnegative(params.event_hold_duration_s, "event_hold_duration_s") ||
      !positive(params.tilt_angle_threshold_deg,
                "tilt_angle_threshold_deg") ||
      params.tilt_angle_threshold_deg >= 180.0 ||
      !nonnegative(params.tilt_min_duration_s, "tilt_min_duration_s") ||
      !positive(params.vibration_rms_threshold,
                "vibration_rms_threshold") ||
      params.motion_window_samples == 0 ||
      !nonnegative(params.stationary_bias_gain, "stationary_bias_gain") ||
      params.stationary_bias_gain > 1.0 ||
      !nonnegative(params.leveling_gain, "leveling_gain") ||
      params.leveling_gain > 1.0 ||
      !positive(params.gyro_bias_jump_threshold,
                "gyro_bias_jump_threshold") ||
      !nonnegative(params.gyro_bias_jump_min_duration_s,
                   "gyro_bias_jump_min_duration_s") ||
      !positive(params.max_integration_dt_s, "max_integration_dt_s") ||
      params.event_queue_capacity == 0) {
    if (error && error->empty()) *error = "invalid profile parameter";
    return false;
  }
  return true;
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

const char* eventTypeName(ImuEventType type) {
  switch (type) {
    case ImuEventType::kImpact:
      return "impact";
    case ImuEventType::kFall:
      return "fall";
    case ImuEventType::kVibration:
      return "vibration";
    case ImuEventType::kMoving:
      return "moving";
    case ImuEventType::kBiasJump:
      return "bias_jump";
  }
  return "unknown";
}

const char* eventPhaseName(ImuEventPhase phase) {
  switch (phase) {
    case ImuEventPhase::kStarted:
      return "started";
    case ImuEventPhase::kEnded:
      return "ended";
    case ImuEventPhase::kUpdated:
      return "updated";
  }
  return "unknown";
}

std::string toJson(const ImuMotionEvent& event) {
  std::ostringstream out;
  out << std::setprecision(12);
  out << '{';
  out << "\"schema\":\"imu_motion_health_event_v1\"";
  out << ",\"id\":" << event.id;
  out << ",\"type\":\"" << eventTypeName(event.type) << '"';
  out << ",\"phase\":\"" << eventPhaseName(event.phase) << '"';
  out << ",\"timestamp\":";
  writeNumber(out, event.timestamp);
  out << ",\"start_timestamp\":";
  writeNumber(out, event.start_timestamp);
  out << ",\"end_timestamp\":";
  writeNullableNumber(out, event.has_end_timestamp, event.end_timestamp);
  out << ",\"duration_s\":";
  writeNumber(out, event.duration_s);
  out << ",\"peak_accel_norm\":";
  writeNumber(out, event.peak_accel_norm);
  out << ",\"peak_gyro_norm\":";
  writeNumber(out, event.peak_gyro_norm);
  out << ",\"peak_vibration_rms\":";
  writeNumber(out, event.peak_vibration_rms);
  out << ",\"peak_confidence\":";
  writeNumber(out, event.peak_confidence);
  out << ",\"peak_accel\":";
  writeNumber(out, event.peak_accel);
  out << ",\"peak_gyro\":";
  writeNumber(out, event.peak_gyro);
  out << ",\"peak_vibration\":";
  writeNumber(out, event.peak_vibration);
  out << ",\"confidence\":";
  writeNumber(out, event.confidence);
  out << '}';
  return out.str();
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
  stream << ",\"gyro_bias_jump\":"
         << (state.gyro_bias_jump ? "true" : "false");
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
  stream << ",\"gyro_bias_delta_norm\":";
  writeNumber(stream, state.gyro_bias_delta_norm);
  stream << ",\"gyro_bias_jump_duration_s\":";
  writeNumber(stream, state.gyro_bias_jump_duration_s);
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

const char* profileName(const std::string& name) {
  const std::string normalized = lowerCopy(trimCopy(name));
  if (normalized == "default" || normalized == "" ||
      normalized == "imu_motion_health")
    return "default";
  if (normalized == "wearable" || normalized == "wearables") return "wearable";
  if (normalized == "vehicle" || normalized == "vehicles") return "vehicle";
  if (normalized == "machine" || normalized == "machines") return "machine";
  if (normalized == "cargo") return "cargo";
  if (normalized == "drone" || normalized == "drones" ||
      normalized == "uav")
    return "drone";
  return nullptr;
}

bool applyProfile(const std::string& name, ImuMotionHealthParams* params,
                  std::string* error) {
  if (params == nullptr) {
    if (error) *error = "profile destination is null";
    return false;
  }
  const char* canonical = profileName(name);
  if (canonical == nullptr) {
    if (error) *error = "unknown IMU profile: " + name;
    return false;
  }

  // The built-ins intentionally change only detector policy, leaving sensor
  // full-scale and integration settings at caller/default values.  This makes
  // a profile safe to layer onto a board-specific configuration.
  if (std::string(canonical) == "wearable") {
    params->startup_duration_s = 0.80;
    params->startup_min_samples = 20;
    params->stationary_gyro_threshold = 0.10;
    params->stationary_accel_tolerance = 0.65;
    params->moving_gyro_threshold = 0.22;
    params->moving_accel_threshold = 0.85;
    params->impact_accel_threshold = 18.0;
    params->impact_gyro_threshold = 7.0;
    params->fall_freefall_threshold = 2.8;
    params->fall_min_duration_s = 0.08;
    params->tilt_angle_threshold_deg = 45.0;
    params->tilt_min_duration_s = 0.20;
    params->vibration_rms_threshold = 1.25;
    params->motion_window_samples = 16;
    params->event_hold_duration_s = 0.25;
    params->emit_moving_events = false;
  } else if (std::string(canonical) == "vehicle") {
    params->startup_duration_s = 1.20;
    params->startup_min_samples = 30;
    params->stationary_gyro_threshold = 0.08;
    params->stationary_accel_tolerance = 0.50;
    params->moving_gyro_threshold = 0.12;
    params->moving_accel_threshold = 0.55;
    params->impact_accel_threshold = 22.0;
    params->impact_gyro_threshold = 8.0;
    params->fall_freefall_threshold = 2.5;
    params->fall_min_duration_s = 0.10;
    params->tilt_angle_threshold_deg = 55.0;
    params->tilt_min_duration_s = 0.20;
    params->vibration_rms_threshold = 1.80;
    params->motion_window_samples = 24;
    params->event_hold_duration_s = 0.20;
    params->emit_moving_events = true;
  } else if (std::string(canonical) == "machine") {
    params->startup_duration_s = 1.50;
    params->startup_min_samples = 40;
    params->stationary_gyro_threshold = 0.12;
    params->stationary_accel_tolerance = 0.90;
    params->moving_gyro_threshold = 0.30;
    params->moving_accel_threshold = 1.25;
    params->impact_accel_threshold = 35.0;
    params->impact_gyro_threshold = 12.0;
    params->fall_freefall_threshold = 2.5;
    params->fall_min_duration_s = 0.08;
    params->tilt_angle_threshold_deg = 60.0;
    params->tilt_min_duration_s = 0.25;
    params->vibration_rms_threshold = 2.50;
    params->motion_window_samples = 32;
    params->event_hold_duration_s = 0.30;
    params->emit_moving_events = true;
  } else if (std::string(canonical) == "cargo") {
    params->startup_duration_s = 1.00;
    params->startup_min_samples = 20;
    params->stationary_gyro_threshold = 0.10;
    params->stationary_accel_tolerance = 0.55;
    params->moving_gyro_threshold = 0.20;
    params->moving_accel_threshold = 0.70;
    params->impact_accel_threshold = 15.0;
    params->impact_gyro_threshold = 6.0;
    params->fall_freefall_threshold = 3.0;
    params->fall_min_duration_s = 0.06;
    params->tilt_angle_threshold_deg = 40.0;
    params->tilt_min_duration_s = 0.15;
    params->vibration_rms_threshold = 1.00;
    params->motion_window_samples = 16;
    params->event_hold_duration_s = 0.35;
    params->emit_moving_events = false;
  } else if (std::string(canonical) == "drone") {
    params->startup_duration_s = 1.00;
    params->startup_min_samples = 50;
    params->stationary_gyro_threshold = 0.18;
    params->stationary_accel_tolerance = 1.00;
    params->moving_gyro_threshold = 0.35;
    params->moving_accel_threshold = 1.50;
    params->impact_accel_threshold = 30.0;
    params->impact_gyro_threshold = 14.0;
    params->fall_freefall_threshold = 2.0;
    params->fall_min_duration_s = 0.05;
    params->tilt_angle_threshold_deg = 65.0;
    params->tilt_min_duration_s = 0.12;
    params->vibration_rms_threshold = 3.00;
    params->motion_window_samples = 12;
    params->event_hold_duration_s = 0.15;
    params->emit_moving_events = true;
  }
  return true;
}

bool loadProfileFile(const std::string& path, ImuMotionHealthParams* params,
                     std::string* error) {
  if (params == nullptr) {
    if (error) *error = "profile destination is null";
    return false;
  }
  std::ifstream input(path);
  if (!input) {
    if (error) *error = "cannot open profile file: " + path;
    return false;
  }

  ImuMotionHealthParams parsed = *params;
  std::map<std::string, bool> seen;
  std::string line;
  std::size_t line_number = 0;
  bool saw_mapping = false;
  while (std::getline(input, line)) {
    ++line_number;
    if (line.find('\t') != std::string::npos) {
      if (error) *error = path + ":" + std::to_string(line_number) +
                           ": tabs are not supported in strict YAML subset";
      return false;
    }
    const std::size_t hash = line.find('#');
    if (hash != std::string::npos) line.resize(hash);
    if (trimCopy(line).empty()) continue;

    std::size_t indent = 0;
    while (indent < line.size() && line[indent] == ' ') ++indent;
    std::string content = trimCopy(line.substr(indent));
    if (!content.empty() && content.front() == '-') {
      if (error) *error = path + ":" + std::to_string(line_number) +
                           ": sequence values are not supported";
      return false;
    }
    const std::size_t colon = content.find(':');
    if (colon == std::string::npos) {
      if (error) *error = path + ":" + std::to_string(line_number) +
                           ": expected key: value";
      return false;
    }
    const std::string raw_key = trimCopy(content.substr(0, colon));
    const std::string raw_value = trimCopy(content.substr(colon + 1));
    if (raw_key.empty()) {
      if (error) *error = path + ":" + std::to_string(line_number) +
                           ": empty key";
      return false;
    }
    std::string key = lowerCopy(raw_key);
    std::replace(key.begin(), key.end(), '-', '_');
    if (raw_value.empty()) {
      // Permit exactly one mapping root, optionally indented children below
      // it.  Other empty values are ambiguous and therefore rejected.
      if (key == "imu_motion_health" || key == "params" || key == "profile") {
        if (key == "profile") {
          if (error) *error = path + ":" + std::to_string(line_number) +
                               ": profile requires a name";
          return false;
        }
        saw_mapping = true;
        continue;
      }
      if (error) *error = path + ":" + std::to_string(line_number) +
                           ": empty value for key '" + raw_key + "'";
      return false;
    }
    if (!saw_mapping && indent > 0) {
      if (error) *error = path + ":" + std::to_string(line_number) +
                           ": indented key without a mapping root";
      return false;
    }
    if (seen[key]) {
      if (error) *error = path + ":" + std::to_string(line_number) +
                           ": duplicate key '" + raw_key + "'";
      return false;
    }
    seen[key] = true;
    if (key == "schema") {
      const std::string schema = lowerCopy(unquoteYamlScalar(raw_value));
      if (schema != "imu_motion_health_profile_v1" &&
          schema != "imu_motion_health_profile") {
        if (error) *error = path + ":" + std::to_string(line_number) +
                             ": unsupported profile schema";
        return false;
      }
      continue;
    }
    if (key == "profile" || key == "preset") {
      if (!applyProfile(unquoteYamlScalar(raw_value), &parsed, error)) {
        if (error && error->find(':') == std::string::npos)
          *error = path + ":" + std::to_string(line_number) + ": " + *error;
        return false;
      }
      continue;
    }
    if (!assignProfileField(raw_key, raw_value, &parsed, error)) {
      if (error && error->find(':') == std::string::npos)
        *error = path + ":" + std::to_string(line_number) + ": " + *error;
      return false;
    }
  }
  if (input.bad()) {
    if (error) *error = "error reading profile file: " + path;
    return false;
  }
  if (!validateProfileParams(parsed, error)) return false;
  if (!seen.empty() || saw_mapping) {
    *params = parsed;
    return true;
  }
  if (error) *error = "profile file is empty: " + path;
  return false;
}

bool loadYamlProfile(const std::string& path, ImuMotionHealthParams* params,
                     std::string* error) {
  return loadProfileFile(path, params, error);
}

std::size_t eventIndex(ImuEventType type) {
  switch (type) {
    case ImuEventType::kImpact:
      return 0;
    case ImuEventType::kFall:
      return 1;
    case ImuEventType::kVibration:
      return 2;
    case ImuEventType::kMoving:
      return 3;
    case ImuEventType::kBiasJump:
      return 4;
  }
  return 0;
}

ImuEventType eventTypeAt(std::size_t index) {
  switch (index) {
    case 0:
      return ImuEventType::kImpact;
    case 1:
      return ImuEventType::kFall;
    case 2:
      return ImuEventType::kVibration;
    case 3:
      return ImuEventType::kMoving;
    case 4:
      return ImuEventType::kBiasJump;
    default:
      return ImuEventType::kImpact;
  }
}

void syncEventAliases(ImuMotionEvent* event) {
  if (event == nullptr) return;
  event->peak_accel = event->peak_accel_norm;
  event->peak_gyro = event->peak_gyro_norm;
  event->peak_vibration = event->peak_vibration_rms;
  event->confidence = event->peak_confidence;
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
  params_.event_queue_capacity =
      std::max<std::size_t>(1, params_.event_queue_capacity);
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
  for (ActiveEvent& event : active_events_) event = ActiveEvent();
  pending_events_.clear();
  next_event_id_ = 1;
  dropped_event_count_ = 0;
  event_impact_active_ = false;
  event_fall_active_ = false;
  event_vibration_active_ = false;
  event_moving_active_ = false;
  event_bias_jump_active_ = false;
  gyro_bias_jump_candidate_ = false;
  gyro_bias_jump_candidate_start_timestamp_ = 0.0;
  gyro_bias_jump_active_ = false;
  gyro_bias_jump_until_ = 0.0;
}

void ImuMotionHealth::reset() { clearRuntimeState(); }

void ImuMotionHealth::enqueueEvent(const ImuMotionEvent& event) {
  const std::size_t capacity = std::max<std::size_t>(1, params_.event_queue_capacity);
  while (pending_events_.size() >= capacity) {
    pending_events_.pop_front();
    ++dropped_event_count_;
  }
  pending_events_.push_back(event);
}

bool ImuMotionHealth::popEvent(ImuMotionEvent* event) {
  if (event == nullptr || pending_events_.empty()) return false;
  *event = pending_events_.front();
  pending_events_.pop_front();
  return true;
}

std::vector<ImuMotionEvent> ImuMotionHealth::drainEvents() {
  std::vector<ImuMotionEvent> result;
  result.reserve(pending_events_.size());
  while (!pending_events_.empty()) {
    result.push_back(pending_events_.front());
    pending_events_.pop_front();
  }
  return result;
}

std::vector<ImuMotionEvent> ImuMotionHealth::activeEvents() const {
  std::vector<ImuMotionEvent> result;
  result.reserve(active_events_.size());
  for (std::size_t index = 0; index < active_events_.size(); ++index) {
    const ActiveEvent& active = active_events_[index];
    if (!active.active) continue;
    ImuMotionEvent event;
    event.id = active.id;
    event.type = eventTypeAt(index);
    event.phase = ImuEventPhase::kUpdated;
    event.timestamp = active.current_timestamp;
    event.start_timestamp = active.start_timestamp;
    event.has_end_timestamp = false;
    event.duration_s = std::max(
        0.0, active.current_timestamp - active.start_timestamp);
    event.peak_accel_norm = active.peak_accel_norm;
    event.peak_gyro_norm = active.peak_gyro_norm;
    event.peak_vibration_rms = active.peak_vibration_rms;
    event.peak_confidence = active.peak_confidence;
    syncEventAliases(&event);
    result.push_back(event);
  }
  return result;
}

std::size_t ImuMotionHealth::activeEventCount() const {
  std::size_t count = 0;
  for (const ActiveEvent& active : active_events_) {
    if (active.active) ++count;
  }
  return count;
}

void ImuMotionHealth::updateBiasJump(double timestamp,
                                     const ImuSample& sample,
                                     bool had_data_error) {
  const Eigen::Vector3d residual = sample.gyro - gyro_bias_;
  state_.gyro_bias_delta_norm = residual.allFinite() ? residual.norm() : 0.0;

  // A jump is only meaningful while the accelerometer says the body is
  // quiet.  This gate prevents an intentional high-rate turn from being
  // mislabeled as a calibration fault, while still detecting a sensor offset
  // that appears while the device is parked.
  const bool accel_quiet =
      !had_data_error &&
      std::abs(state_.accel_norm - gravity_magnitude_) <=
          2.0 * params_.stationary_accel_tolerance &&
      state_.linear_accel_norm <=
          2.0 * params_.stationary_accel_tolerance &&
      state_.vibration_rms < params_.vibration_rms_threshold;
  const bool candidate =
      accel_quiet &&
      state_.gyro_bias_delta_norm >= params_.gyro_bias_jump_threshold;

  if (candidate) {
    if (!gyro_bias_jump_candidate_) {
      gyro_bias_jump_candidate_ = true;
      gyro_bias_jump_candidate_start_timestamp_ = timestamp;
    }
    state_.gyro_bias_jump_duration_s = std::max(
        0.0, timestamp - gyro_bias_jump_candidate_start_timestamp_);
    if (state_.gyro_bias_jump_duration_s >=
        params_.gyro_bias_jump_min_duration_s) {
      if (!gyro_bias_jump_active_) {
        ++counters_.gyro_bias_jump_detections;
      }
      gyro_bias_jump_active_ = true;
      gyro_bias_jump_until_ = std::max(
          gyro_bias_jump_until_, timestamp + params_.event_hold_duration_s);
    }
  } else {
    gyro_bias_jump_candidate_ = false;
    state_.gyro_bias_jump_duration_s = 0.0;
  }

  if (gyro_bias_jump_active_ && timestamp >= gyro_bias_jump_until_ &&
      !candidate) {
    gyro_bias_jump_active_ = false;
  }
  state_.gyro_bias_jump = gyro_bias_jump_active_;
  event_bias_jump_active_ = state_.gyro_bias_jump;
  if (state_.gyro_bias_jump) ++counters_.gyro_bias_jump_samples;
}

void ImuMotionHealth::finishEvent(std::size_t index, double timestamp) {
  if (index >= active_events_.size() || !active_events_[index].active) return;
  ActiveEvent& active = active_events_[index];
  const double end_timestamp =
      std::isfinite(timestamp) ? std::max(timestamp, active.start_timestamp)
                               : active.start_timestamp;
  ImuMotionEvent event;
  event.id = active.id;
  event.type = eventTypeAt(index);
  event.phase = ImuEventPhase::kEnded;
  event.timestamp = end_timestamp;
  event.start_timestamp = active.start_timestamp;
  event.end_timestamp = end_timestamp;
  event.has_end_timestamp = true;
  event.duration_s = std::max(0.0, end_timestamp - active.start_timestamp);
  event.peak_accel_norm = active.peak_accel_norm;
  event.peak_gyro_norm = active.peak_gyro_norm;
  event.peak_vibration_rms = active.peak_vibration_rms;
  event.peak_confidence = active.peak_confidence;
  syncEventAliases(&event);
  enqueueEvent(event);
  active = ActiveEvent();
}

void ImuMotionHealth::updateEvents(double timestamp, bool impact, bool fall,
                                   bool vibration, bool moving,
                                   bool bias_jump) {
  const bool active_now[5] = {impact, fall, vibration,
                              params_.emit_moving_events && moving, bias_jump};
  for (std::size_t index = 0; index < active_events_.size(); ++index) {
    ActiveEvent& active = active_events_[index];
    if (active_now[index]) {
      if (!active.active) {
        active.active = true;
        active.id = next_event_id_++;
        active.start_timestamp = timestamp;
        active.current_timestamp = timestamp;
        active.peak_accel_norm = state_.accel_norm;
        active.peak_gyro_norm = state_.gyro_norm;
        active.peak_vibration_rms = state_.vibration_rms;
        active.peak_confidence = state_.confidence;

        ImuMotionEvent event;
        event.id = active.id;
        event.type = eventTypeAt(index);
        event.phase = ImuEventPhase::kStarted;
        event.timestamp = timestamp;
        event.start_timestamp = timestamp;
        event.has_end_timestamp = false;
        event.duration_s = 0.0;
        event.peak_accel_norm = active.peak_accel_norm;
        event.peak_gyro_norm = active.peak_gyro_norm;
        event.peak_vibration_rms = active.peak_vibration_rms;
        event.peak_confidence = active.peak_confidence;
        syncEventAliases(&event);
        enqueueEvent(event);
      } else {
        active.current_timestamp = timestamp;
        active.peak_accel_norm =
            std::max(active.peak_accel_norm, state_.accel_norm);
        active.peak_gyro_norm =
            std::max(active.peak_gyro_norm, state_.gyro_norm);
        active.peak_vibration_rms =
            std::max(active.peak_vibration_rms, state_.vibration_rms);
        active.peak_confidence =
            std::max(active.peak_confidence, state_.confidence);
      }
    } else if (active.active) {
      finishEvent(index, timestamp);
    }
  }
  event_impact_active_ = impact;
  event_fall_active_ = fall;
  event_vibration_active_ = vibration;
  event_moving_active_ = moving;
  event_bias_jump_active_ = bias_jump;
}

void ImuMotionHealth::flushEvents(double timestamp) {
  double close_timestamp = timestamp;
  if (!std::isfinite(close_timestamp) || close_timestamp == 0.0) {
    close_timestamp = have_timestamp_ ? last_timestamp_ : state_.timestamp;
  }
  for (std::size_t index = 0; index < active_events_.size(); ++index)
    finishEvent(index, close_timestamp);
  event_impact_active_ = false;
  event_fall_active_ = false;
  event_vibration_active_ = false;
  event_moving_active_ = false;
  event_bias_jump_active_ = false;
}

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

  // Keep the independent detector outputs for the event manager.  The
  // legacy motion_state below intentionally remains a single prioritized
  // enum, so callers that only consume the old API see exactly the same
  // values even when multiple event types overlap.
  event_impact_active_ = impact;
  event_fall_active_ = fall_now;
  event_vibration_active_ = vibration;
  event_moving_active_ = moving_now;

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
  if (state_.gyro_bias_jump) confidence -= 0.20;
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
  if (state_.gyro_bias_jump)
    messages.emplace_back("gyro bias jump detected");

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
  state_.gyro_bias_jump = false;
  state_.gyro_bias_jump_duration_s = 0.0;
  state_.gyro_bias_delta_norm = 0.0;
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
  updateBiasJump(sample.timestamp, sample, had_data_error);

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
  if (!startup_quality_ok_ || had_data_error || state_.gyro_bias_jump ||
      (have_anomaly_timestamp_ && sample.timestamp < anomaly_until_)) {
    state_.health_state = HealthState::kDegraded;
  }
  state_.timestamp = sample.timestamp;
  state_.dt = dt;
  updateStateAliases();
  updateConfidence(had_data_error);
  makeDiagnostic(had_data_error);
  updateEvents(sample.timestamp, event_impact_active_, event_fall_active_,
               event_vibration_active_, event_moving_active_,
               event_bias_jump_active_);
  return state_;
}

}  // namespace imu_motion_health
}  // namespace localization_zoo
