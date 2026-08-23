#include "imu_motion_health/imu_motion_health.h"

#include <Eigen/Core>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <set>
#include <string>
#include <vector>

namespace {

using localization_zoo::imu_motion_health::healthStateName;
using localization_zoo::imu_motion_health::motionStateName;
using localization_zoo::imu_motion_health::ImuMotionHealth;
using localization_zoo::imu_motion_health::ImuMotionHealthParams;
using localization_zoo::imu_motion_health::ImuMotionHealthState;
using localization_zoo::imu_motion_health::ImuSample;
using localization_zoo::imu_motion_health::ImuMotionEvent;

constexpr int kExitOk = 0;
constexpr int kExitUsage = 2;
constexpr int kExitIo = 3;
constexpr int kExitInput = 4;

struct Options {
  std::string input_path;
  std::string jsonl_output;
  std::string events_output;
  std::string summary_output;
  std::string profile_name;
  std::string profile_file;
  bool skip_invalid = false;
  bool help = false;
  ImuMotionHealthParams params;
  // CLI values are collected first; profile and file layers are applied only
  // after parsing, which makes precedence independent of argument order.
  std::set<std::string> explicit_params;
};

struct EventStats {
  std::size_t started = 0;
  std::size_t ended = 0;
  std::map<std::string, std::size_t> ended_by_type;
};

std::string trim(const std::string& value) {
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

std::string lower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) {
                   return static_cast<char>(std::tolower(c));
                 });
  return value;
}

std::vector<std::string> splitCsv(const std::string& line) {
  std::vector<std::string> fields;
  std::size_t begin = 0;
  while (true) {
    const std::size_t comma = line.find(',', begin);
    if (comma == std::string::npos) {
      fields.emplace_back(line.substr(begin));
      break;
    }
    fields.emplace_back(line.substr(begin, comma - begin));
    begin = comma + 1;
  }
  return fields;
}

bool isHeader(const std::vector<std::string>& fields) {
  static const char* const kNames[] = {"timestamp", "gx", "gy", "gz",
                                        "ax", "ay", "az"};
  if (fields.size() != 7) return false;
  for (std::size_t i = 0; i < fields.size(); ++i) {
    if (lower(trim(fields[i])) != kNames[i]) return false;
  }
  return true;
}

bool parseNumber(const std::string& token, double* value) {
  const std::string text = trim(token);
  if (text.empty()) return false;
  std::size_t consumed = 0;
  try {
    *value = std::stod(text, &consumed);
  } catch (const std::exception&) {
    return false;
  }
  return consumed == text.size();
}

bool parseSize(const std::string& text, std::size_t* value) {
  const std::string token = trim(text);
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

bool parseDoubleOption(const std::string& option, const std::string& text,
                       double* destination, bool allow_zero,
                       std::string* error) {
  double value = 0.0;
  if (!parseNumber(text, &value) || !std::isfinite(value) ||
      (allow_zero ? value < 0.0 : value <= 0.0)) {
    *error = option + " expects a finite " +
             (allow_zero ? "non-negative" : "positive") + " number";
    return false;
  }
  *destination = value;
  return true;
}

bool splitOption(const std::string& argument, std::string* key,
                 std::string* inline_value, bool* has_inline_value) {
  if (argument.empty() || argument[0] != '-') return false;
  const std::size_t equals = argument.find('=');
  if (equals == std::string::npos) {
    *key = argument;
    *inline_value = std::string();
    *has_inline_value = false;
  } else {
    *key = argument.substr(0, equals);
    *inline_value = argument.substr(equals + 1);
    *has_inline_value = true;
  }
  return true;
}

void printUsage(std::ostream& stream) {
  stream <<
      "Usage: imu_motion_health_cli --input imu.csv [options]\n\n"
      "CSV format: timestamp,gx,gy,gz,ax,ay,az (seconds, rad/s, m/s^2).\n"
      "The header is optional; blank lines and lines beginning with # are\n"
      "ignored.  JSONL snapshots and the final summary are independent\n"
      "outputs.  Use '-' for stdout.\n\n"
      "Output options:\n"
      "  --jsonl-output PATH       Write one JSON snapshot per input row\n"
      "  --emit-jsonl              Same as --jsonl-output -\n"
      "  --events-output PATH      Write started/ended event JSONL records\n"
      "  --emit-events             Same as --events-output -\n"
      "  --summary-output PATH     Write the final JSON summary (default stdout)\n"
      "  --skip-invalid            Continue after malformed CSV rows (exit 4)\n\n"
      "Profiles/configuration:\n"
      "  --profile NAME             default/wearable/vehicle/machine/cargo/drone\n"
      "  --profile-file PATH        Strict dependency-free YAML scalar mapping\n"
      "  Precedence: default -> profile -> profile-file -> explicit CLI values\n\n"
      "Startup/data thresholds:\n"
      "  --startup-duration S      Static startup window (default 1.0)\n"
      "  --startup-min-samples N   Minimum startup samples (default 20)\n"
      "  --gravity MPS2             Expected gravity magnitude\n"
      "  --max-gap S               Timestamp gap threshold\n"
      "  --max-integration-dt S    Largest interval integrated\n"
      "  --gyro-saturation RADPS   Gyro full-scale threshold\n"
      "  --accel-saturation MPS2   Accelerometer full-scale threshold\n\n"
      "Motion thresholds:\n"
      "  --stationary-gyro RADPS   Stationary gyro RMS threshold\n"
      "  --stationary-accel MPS2   Stationary accel tolerance\n"
      "  --moving-gyro RADPS       Moving gyro threshold\n"
      "  --moving-accel MPS2       Moving linear-accel threshold\n"
      "  --impact-accel MPS2       Impact acceleration threshold\n"
      "  --impact-gyro RADPS       Impact gyro threshold\n"
      "  --fall-freefall MPS2      Free-fall acceleration threshold\n"
      "  --fall-min-duration S     Minimum free-fall duration\n"
      "  --tilt-angle DEG          Quiet-pose tilt threshold\n"
      "  --tilt-min-duration S     Tilt duration before fall event\n"
      "  --event-hold S            Event latch duration\n"
      "  --vibration-rms MPS2      Vibration RMS threshold\n"
      "  --motion-window N         Rolling motion window size\n\n"
      "Calibration/behavior:\n"
      "  --stationary-bias-gain G  Online gyro bias gain [0,1]\n"
      "  --leveling-gain G         Stationary gravity leveling gain [0,1]\n"
      "  --gyro-bias-jump RADPS    Stationary gyro bias-jump threshold\n"
      "  --bias-jump-min-duration S  Confirmation duration\n"
      "  --no-gyro-bias            Disable gyro bias estimation\n"
      "  --estimate-accel-bias     Estimate startup accelerometer bias\n"
      "  --no-zero-velocity        Do not clamp velocity while stationary\n"
      "  --event-queue-capacity N  Bounded pending event FIFO capacity\n"
      "  --emit-moving-events      Also track moving start/end events\n"
      "  --no-moving-events        Disable moving start/end events\n"
      "  -h, --help                Show this help\n";
}

void applyExplicitParams(const ImuMotionHealthParams& source,
                         const std::set<std::string>& keys,
                         ImuMotionHealthParams* destination) {
  const auto has = [&](const char* key) { return keys.count(key) != 0; };
  if (has("startup_duration_s"))
    destination->startup_duration_s = source.startup_duration_s;
  if (has("startup_min_samples"))
    destination->startup_min_samples = source.startup_min_samples;
  if (has("gravity_magnitude"))
    destination->gravity_magnitude = source.gravity_magnitude;
  if (has("max_gap_s")) destination->max_gap_s = source.max_gap_s;
  if (has("max_integration_dt_s"))
    destination->max_integration_dt_s = source.max_integration_dt_s;
  if (has("gyro_saturation_rad_s"))
    destination->gyro_saturation_rad_s = source.gyro_saturation_rad_s;
  if (has("accel_saturation_mps2"))
    destination->accel_saturation_mps2 = source.accel_saturation_mps2;
  if (has("stationary_gyro_threshold"))
    destination->stationary_gyro_threshold = source.stationary_gyro_threshold;
  if (has("stationary_accel_tolerance"))
    destination->stationary_accel_tolerance = source.stationary_accel_tolerance;
  if (has("moving_gyro_threshold"))
    destination->moving_gyro_threshold = source.moving_gyro_threshold;
  if (has("moving_accel_threshold"))
    destination->moving_accel_threshold = source.moving_accel_threshold;
  if (has("impact_accel_threshold"))
    destination->impact_accel_threshold = source.impact_accel_threshold;
  if (has("impact_gyro_threshold"))
    destination->impact_gyro_threshold = source.impact_gyro_threshold;
  if (has("impact_requires_accel_and_gyro"))
    destination->impact_requires_accel_and_gyro =
        source.impact_requires_accel_and_gyro;
  if (has("fall_freefall_threshold"))
    destination->fall_freefall_threshold = source.fall_freefall_threshold;
  if (has("fall_min_duration_s"))
    destination->fall_min_duration_s = source.fall_min_duration_s;
  if (has("event_hold_duration_s"))
    destination->event_hold_duration_s = source.event_hold_duration_s;
  if (has("tilt_angle_threshold_deg"))
    destination->tilt_angle_threshold_deg = source.tilt_angle_threshold_deg;
  if (has("tilt_min_duration_s"))
    destination->tilt_min_duration_s = source.tilt_min_duration_s;
  if (has("vibration_rms_threshold"))
    destination->vibration_rms_threshold = source.vibration_rms_threshold;
  if (has("motion_window_samples"))
    destination->motion_window_samples = source.motion_window_samples;
  if (has("stationary_bias_gain"))
    destination->stationary_bias_gain = source.stationary_bias_gain;
  if (has("leveling_gain")) destination->leveling_gain = source.leveling_gain;
  if (has("gyro_bias_jump_threshold"))
    destination->gyro_bias_jump_threshold = source.gyro_bias_jump_threshold;
  if (has("gyro_bias_jump_min_duration_s"))
    destination->gyro_bias_jump_min_duration_s =
        source.gyro_bias_jump_min_duration_s;
  if (has("estimate_gyro_bias"))
    destination->estimate_gyro_bias = source.estimate_gyro_bias;
  if (has("estimate_accel_bias"))
    destination->estimate_accel_bias = source.estimate_accel_bias;
  if (has("zero_velocity_when_stationary"))
    destination->zero_velocity_when_stationary =
        source.zero_velocity_when_stationary;
  if (has("event_queue_capacity"))
    destination->event_queue_capacity = source.event_queue_capacity;
  if (has("emit_moving_events"))
    destination->emit_moving_events = source.emit_moving_events;
}

bool parseArgs(int argc, char** argv, Options* options, std::string* error) {
  auto markExplicit = [&](const char* key) {
    options->explicit_params.insert(key);
  };
  for (int index = 1; index < argc; ++index) {
    const std::string argument(argv[index]);
    if (argument == "-h" || argument == "--help") {
      options->help = true;
      continue;
    }
    std::string key;
    std::string inline_value;
    bool has_inline_value = false;
    if (!splitOption(argument, &key, &inline_value, &has_inline_value)) {
      if (options->input_path.empty()) {
        options->input_path = argument;
        continue;
      }
      *error = "unexpected positional argument: " + argument;
      return false;
    }

    const bool is_flag =
        key == "--skip-invalid" || key == "--emit-jsonl" ||
        key == "--emit-events" ||
        key == "--emit-moving-events" || key == "--no-moving-events" ||
        key == "--no-gyro-bias" || key == "--estimate-accel-bias" ||
        key == "--no-zero-velocity";
    if (is_flag) {
      if (has_inline_value) {
        *error = key + " does not take a value";
        return false;
      }
      if (key == "--skip-invalid") options->skip_invalid = true;
      if (key == "--emit-jsonl") {
        if (!options->jsonl_output.empty()) {
          *error = "--emit-jsonl conflicts with --jsonl-output";
          return false;
        }
        options->jsonl_output = "-";
      }
      if (key == "--emit-events") {
        if (!options->events_output.empty()) {
          *error = "--emit-events conflicts with --events-output";
          return false;
        }
        options->events_output = "-";
      }
      if (key == "--no-gyro-bias") {
        options->params.estimate_gyro_bias = false;
        markExplicit("estimate_gyro_bias");
      }
      if (key == "--estimate-accel-bias")
        options->params.estimate_accel_bias = true;
      if (key == "--estimate-accel-bias") markExplicit("estimate_accel_bias");
      if (key == "--no-zero-velocity") {
        options->params.zero_velocity_when_stationary = false;
        markExplicit("zero_velocity_when_stationary");
      }
      if (key == "--emit-moving-events") {
        options->params.emit_moving_events = true;
        markExplicit("emit_moving_events");
      }
      if (key == "--no-moving-events") {
        options->params.emit_moving_events = false;
        markExplicit("emit_moving_events");
      }
      continue;
    }

    std::string value;
    auto takeValue = [&](const char* name) -> bool {
      if (has_inline_value) {
        value = inline_value;
        return true;
      }
      if (index + 1 >= argc) {
        *error = std::string(name) + " expects a value";
        return false;
      }
      value = argv[++index];
      return true;
    };

    if (key == "--input" || key == "-i") {
      if (!takeValue(key.c_str())) return false;
      if (value.empty()) {
        *error = key + " must not be empty";
        return false;
      }
      options->input_path = value;
    } else if (key == "--jsonl-output") {
      if (!takeValue(key.c_str())) return false;
      if (value.empty()) {
        *error = key + " must not be empty";
        return false;
      }
      if (!options->jsonl_output.empty()) {
        *error = "JSONL output was specified more than once";
        return false;
      }
      options->jsonl_output = value;
    } else if (key == "--events-output") {
      if (!takeValue(key.c_str())) return false;
      if (value.empty()) {
        *error = key + " must not be empty";
        return false;
      }
      if (!options->events_output.empty()) {
        *error = "events output was specified more than once";
        return false;
      }
      options->events_output = value;
    } else if (key == "--summary-output") {
      if (!takeValue(key.c_str())) return false;
      if (value.empty()) {
        *error = key + " must not be empty";
        return false;
      }
      options->summary_output = value;
    } else if (key == "--profile" || key == "--preset") {
      if (!takeValue(key.c_str())) return false;
      if (value.empty()) {
        *error = key + " must not be empty";
        return false;
      }
      options->profile_name = value;
    } else if (key == "--profile-file" || key == "--config" ||
               key == "--config-file") {
      if (!takeValue(key.c_str())) return false;
      if (value.empty()) {
        *error = key + " must not be empty";
        return false;
      }
      options->profile_file = value;
    } else if (key == "--startup-duration" || key == "--startup-duration-s") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value, &options->params.startup_duration_s,
                             true, error))
        return false;
      markExplicit("startup_duration_s");
    } else if (key == "--startup-min-samples") {
      if (!takeValue(key.c_str()) ||
          !parseSize(value, &options->params.startup_min_samples) ||
          options->params.startup_min_samples == 0) {
        *error = key + " expects a positive integer";
        return false;
      }
      markExplicit("startup_min_samples");
    } else if (key == "--gravity" || key == "--gravity-magnitude") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value, &options->params.gravity_magnitude,
                             false, error))
        return false;
      markExplicit("gravity_magnitude");
    } else if (key == "--max-gap" || key == "--max-gap-s") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value, &options->params.max_gap_s, false,
                             error))
        return false;
      markExplicit("max_gap_s");
    } else if (key == "--max-integration-dt" ||
               key == "--max-integration-dt-s") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value,
                             &options->params.max_integration_dt_s, false,
                             error))
        return false;
      markExplicit("max_integration_dt_s");
    } else if (key == "--gyro-saturation" ||
               key == "--gyro-saturation-rad-s") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value,
                             &options->params.gyro_saturation_rad_s, false,
                             error))
        return false;
      markExplicit("gyro_saturation_rad_s");
    } else if (key == "--accel-saturation" ||
               key == "--accel-saturation-mps2") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value,
                             &options->params.accel_saturation_mps2, false,
                             error))
        return false;
      markExplicit("accel_saturation_mps2");
    } else if (key == "--stationary-gyro") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value,
                             &options->params.stationary_gyro_threshold, false,
                             error))
        return false;
      markExplicit("stationary_gyro_threshold");
    } else if (key == "--stationary-accel") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value,
                             &options->params.stationary_accel_tolerance, false,
                             error))
        return false;
      markExplicit("stationary_accel_tolerance");
    } else if (key == "--moving-gyro") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value,
                             &options->params.moving_gyro_threshold, false,
                             error))
        return false;
      markExplicit("moving_gyro_threshold");
    } else if (key == "--moving-accel") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value,
                             &options->params.moving_accel_threshold, false,
                             error))
        return false;
      markExplicit("moving_accel_threshold");
    } else if (key == "--impact-accel") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value,
                             &options->params.impact_accel_threshold, false,
                             error))
        return false;
      markExplicit("impact_accel_threshold");
    } else if (key == "--impact-gyro") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value,
                             &options->params.impact_gyro_threshold, false,
                             error))
        return false;
      markExplicit("impact_gyro_threshold");
    } else if (key == "--fall-freefall") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value,
                             &options->params.fall_freefall_threshold, false,
                             error))
        return false;
      markExplicit("fall_freefall_threshold");
  } else if (key == "--fall-min-duration") {
    if (!takeValue(key.c_str()) ||
        !parseDoubleOption(key, value, &options->params.fall_min_duration_s,
                           true, error))
      return false;
    markExplicit("fall_min_duration_s");
  } else if (key == "--tilt-angle" ||
             key == "--tilt-angle-threshold-deg") {
    if (!takeValue(key.c_str()) ||
        !parseDoubleOption(key, value,
                           &options->params.tilt_angle_threshold_deg, false,
                           error) ||
        options->params.tilt_angle_threshold_deg >= 180.0) {
      *error = key + " expects a number in (0,180) degrees";
      return false;
    }
    markExplicit("tilt_angle_threshold_deg");
  } else if (key == "--tilt-min-duration") {
    if (!takeValue(key.c_str()) ||
        !parseDoubleOption(key, value, &options->params.tilt_min_duration_s,
                           true, error))
        return false;
      markExplicit("tilt_min_duration_s");
  } else if (key == "--event-hold") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value, &options->params.event_hold_duration_s,
                             true, error))
        return false;
      markExplicit("event_hold_duration_s");
    } else if (key == "--vibration-rms") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value, &options->params.vibration_rms_threshold,
                             false, error))
        return false;
      markExplicit("vibration_rms_threshold");
    } else if (key == "--motion-window") {
      if (!takeValue(key.c_str()) ||
          !parseSize(value, &options->params.motion_window_samples) ||
          options->params.motion_window_samples == 0) {
        *error = key + " expects a positive integer";
        return false;
      }
      markExplicit("motion_window_samples");
    } else if (key == "--event-queue-capacity") {
      if (!takeValue(key.c_str()) ||
          !parseSize(value, &options->params.event_queue_capacity) ||
          options->params.event_queue_capacity == 0) {
        *error = key + " expects a positive integer";
        return false;
      }
      markExplicit("event_queue_capacity");
    } else if (key == "--stationary-bias-gain") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value, &options->params.stationary_bias_gain,
                             true, error) ||
          options->params.stationary_bias_gain > 1.0) {
        *error = key + " expects a number in [0,1]";
        return false;
      }
      markExplicit("stationary_bias_gain");
    } else if (key == "--leveling-gain") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value, &options->params.leveling_gain, true,
                             error) ||
          options->params.leveling_gain > 1.0) {
        *error = key + " expects a number in [0,1]";
        return false;
      }
      markExplicit("leveling_gain");
    } else if (key == "--gyro-bias-jump" ||
               key == "--gyro-bias-jump-threshold" ||
               key == "--bias-jump-threshold") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(key, value,
                             &options->params.gyro_bias_jump_threshold, false,
                             error))
        return false;
      markExplicit("gyro_bias_jump_threshold");
    } else if (key == "--bias-jump-min-duration" ||
               key == "--gyro-bias-jump-min-duration") {
      if (!takeValue(key.c_str()) ||
          !parseDoubleOption(
              key, value, &options->params.gyro_bias_jump_min_duration_s,
              true, error))
        return false;
      markExplicit("gyro_bias_jump_min_duration_s");
    } else {
      *error = "unknown option: " + key;
      return false;
    }
  }

  // Resolve configuration layers only after all arguments have been scanned.
  // This guarantees that `--impact-accel 8 --profile cargo` and the reverse
  // ordering have identical semantics: explicit CLI always wins.
  const ImuMotionHealthParams cli_values = options->params;
  options->params = ImuMotionHealthParams();
  if (!options->profile_name.empty() &&
      !localization_zoo::imu_motion_health::applyProfile(
          options->profile_name, &options->params, error)) {
    return false;
  }
  if (!options->profile_file.empty() &&
      !localization_zoo::imu_motion_health::loadProfileFile(
          options->profile_file, &options->params, error)) {
    return false;
  }
  applyExplicitParams(cli_values, options->explicit_params, &options->params);

  if (!options->help && options->input_path.empty()) {
    *error = "--input PATH is required";
    return false;
  }
  if (!options->jsonl_output.empty() &&
      !options->summary_output.empty() &&
      options->jsonl_output != "-" && options->summary_output != "-" &&
      options->jsonl_output == options->summary_output) {
    *error = "--jsonl-output and --summary-output must be different files";
    return false;
  }
  if (!options->events_output.empty() &&
      !options->summary_output.empty() && options->events_output != "-" &&
      options->summary_output != "-" &&
      options->events_output == options->summary_output) {
    *error = "--events-output and --summary-output must be different files";
    return false;
  }
  if (!options->events_output.empty() && !options->jsonl_output.empty() &&
      options->events_output != "-" && options->jsonl_output != "-" &&
      options->events_output == options->jsonl_output) {
    *error = "--events-output and --jsonl-output must be different files";
    return false;
  }
  return true;
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

void writeJsonNumber(std::ostream& stream, double value) {
  if (std::isfinite(value)) {
    stream << value;
  } else {
    stream << "null";
  }
}

void writeNullableTimestamp(std::ostream& stream, bool present, double value) {
  if (present) {
    writeJsonNumber(stream, value);
  } else {
    stream << "null";
  }
}

void writeCounters(std::ostream& stream,
                   const localization_zoo::imu_motion_health::
                       ImuMotionHealthCounters& counters) {
  stream << "{\"samples\":" << counters.samples
         << ",\"accepted_samples\":" << counters.accepted_samples
         << ",\"rejected_samples\":" << counters.rejected_samples
         << ",\"nonfinite_samples\":" << counters.nonfinite_samples
         << ",\"nonmonotonic_timestamps\":"
         << counters.nonmonotonic_timestamps
         << ",\"timestamp_gaps\":" << counters.timestamp_gaps
         << ",\"gyro_saturated_samples\":"
         << counters.gyro_saturated_samples
         << ",\"accel_saturated_samples\":"
         << counters.accel_saturated_samples << ",\"stationary_samples\":"
         << counters.stationary_samples << ",\"moving_samples\":"
         << counters.moving_samples << ",\"impact_samples\":"
         << counters.impact_samples << ",\"fall_samples\":"
         << counters.fall_samples << ",\"vibration_samples\":"
         << counters.vibration_samples
         << ",\"gyro_bias_jump_samples\":"
         << counters.gyro_bias_jump_samples
         << ",\"gyro_bias_jump_detections\":"
         << counters.gyro_bias_jump_detections << '}';
}

void drainAndWriteEvents(ImuMotionHealth* pipeline, std::ostream* output,
                         EventStats* stats) {
  if (pipeline == nullptr || stats == nullptr) return;
  for (const ImuMotionEvent& event : pipeline->drainEvents()) {
    if (event.phase == localization_zoo::imu_motion_health::ImuEventPhase::
                       kStarted) {
      ++stats->started;
    } else {
      ++stats->ended;
      ++stats->ended_by_type[
          localization_zoo::imu_motion_health::eventTypeName(event.type)];
    }
    if (output != nullptr) *output << localization_zoo::imu_motion_health::toJson(event) << '\n';
  }
}

std::string makeSummary(const Options& options, const ImuMotionHealth& pipeline,
                         std::size_t rows_read, std::size_t parse_errors,
                         bool header_seen, bool have_first_timestamp,
                         double first_timestamp, bool have_last_timestamp,
                         double last_timestamp, const EventStats& event_stats) {
  const ImuMotionHealthState& state = pipeline.snapshot();
  std::ostringstream stream;
  stream << std::setprecision(12) << '{';
  stream << "\"schema\":\"imu_motion_health_summary_v1\"";
  stream << ",\"input\":\"" << jsonEscape(options.input_path) << '"';
  stream << ",\"header_seen\":" << (header_seen ? "true" : "false");
  stream << ",\"rows_read\":" << rows_read;
  stream << ",\"parse_errors\":" << parse_errors;
  stream << ",\"first_timestamp\":";
  writeNullableTimestamp(stream, have_first_timestamp, first_timestamp);
  stream << ",\"last_timestamp\":";
  writeNullableTimestamp(stream, have_last_timestamp, last_timestamp);
  stream << ",\"duration_s\":";
  if (have_first_timestamp && have_last_timestamp) {
    writeJsonNumber(stream, last_timestamp - first_timestamp);
  } else {
    stream << "null";
  }
  stream << ",\"motion_state\":\"" << motionStateName(state.motion_state)
         << "\"";
  stream << ",\"health_state\":\"" << healthStateName(state.health_state)
         << "\"";
  stream << ",\"startup_complete\":"
         << (state.startup_complete ? "true" : "false");
  stream << ",\"startup_quality_ok\":"
         << (state.startup_quality_ok ? "true" : "false");
  stream << ",\"confidence\":";
  writeJsonNumber(stream, state.confidence);
  stream << ",\"counters\":";
  writeCounters(stream, pipeline.counters());
  stream << ",\"events_started\":" << event_stats.started;
  stream << ",\"events_ended\":" << event_stats.ended;
  stream << ",\"events_dropped\":" << pipeline.droppedEventCount();
  stream << ",\"event_counts\":{";
  const char* const event_names[] = {"impact", "fall", "vibration", "moving",
                                     "bias_jump"};
  for (std::size_t i = 0; i < 5; ++i) {
    if (i != 0) stream << ',';
    const auto found = event_stats.ended_by_type.find(event_names[i]);
    stream << '\"' << event_names[i] << "\":"
           << (found == event_stats.ended_by_type.end() ? 0 : found->second);
  }
  stream << '}';
  stream << ",\"final_state\":" << pipeline.toJson();
  stream << '}';
  return stream.str();
}

void reportCsvError(std::ostream& stream, std::size_t line_number,
                    const std::string& message) {
  stream << "imu_motion_health_cli: line " << line_number << ": " << message
         << '\n';
}

}  // namespace

int main(int argc, char** argv) {
  Options options;
  std::string argument_error;
  if (!parseArgs(argc, argv, &options, &argument_error)) {
    std::cerr << "imu_motion_health_cli: " << argument_error << "\n\n";
    printUsage(std::cerr);
    return kExitUsage;
  }
  if (options.help) {
    printUsage(std::cout);
    return kExitOk;
  }

  std::ifstream input_file;
  std::istream* input = &std::cin;
  if (options.input_path != "-") {
    input_file.open(options.input_path);
    if (!input_file) {
      std::cerr << "imu_motion_health_cli: cannot open input '"
                << options.input_path << "'\n";
      return kExitIo;
    }
    input = &input_file;
  }

  std::ofstream jsonl_file;
  std::ostream* jsonl = nullptr;
  if (!options.jsonl_output.empty()) {
    if (options.jsonl_output == "-") {
      jsonl = &std::cout;
    } else {
      jsonl_file.open(options.jsonl_output);
      if (!jsonl_file) {
        std::cerr << "imu_motion_health_cli: cannot open JSONL output '"
                  << options.jsonl_output << "'\n";
        return kExitIo;
      }
      jsonl = &jsonl_file;
    }
  }

  std::ofstream events_file;
  std::ostream* events = nullptr;
  if (!options.events_output.empty()) {
    if (options.events_output == "-") {
      events = &std::cout;
    } else {
      events_file.open(options.events_output);
      if (!events_file) {
        std::cerr << "imu_motion_health_cli: cannot open events output '"
                  << options.events_output << "'\n";
        return kExitIo;
      }
      events = &events_file;
    }
  }

  std::ofstream summary_file;
  std::ostream* summary = &std::cout;
  if (!options.summary_output.empty() && options.summary_output != "-") {
    summary_file.open(options.summary_output);
    if (!summary_file) {
      std::cerr << "imu_motion_health_cli: cannot open summary output '"
                << options.summary_output << "'\n";
      return kExitIo;
    }
    summary = &summary_file;
  }

  ImuMotionHealth pipeline(options.params);
  std::string line;
  std::size_t line_number = 0;
  std::size_t rows_read = 0;
  std::size_t parse_errors = 0;
  bool header_seen = false;
  bool saw_non_comment_line = false;
  bool have_first_timestamp = false;
  bool have_last_timestamp = false;
  double first_timestamp = 0.0;
  double last_timestamp = 0.0;
  EventStats event_stats;

  while (std::getline(*input, line)) {
    ++line_number;
    if (line_number == 1 && line.size() >= 3 &&
        static_cast<unsigned char>(line[0]) == 0xEF &&
        static_cast<unsigned char>(line[1]) == 0xBB &&
        static_cast<unsigned char>(line[2]) == 0xBF) {
      line.erase(0, 3);
    }
    const std::string stripped = trim(line);
    if (stripped.empty() || stripped[0] == '#') continue;

    const std::vector<std::string> fields = splitCsv(stripped);
    if (!saw_non_comment_line && isHeader(fields)) {
      header_seen = true;
      saw_non_comment_line = true;
      continue;
    }
    saw_non_comment_line = true;

    if (fields.size() != 7) {
      ++parse_errors;
      reportCsvError(std::cerr, line_number,
                     "expected 7 fields (timestamp,gx,gy,gz,ax,ay,az), got " +
                         std::to_string(fields.size()));
      if (!options.skip_invalid) return kExitInput;
      continue;
    }

    double values[7] = {};
    bool parsed = true;
    for (std::size_t i = 0; i < 7; ++i) {
      if (!parseNumber(fields[i], &values[i])) {
        parsed = false;
        reportCsvError(std::cerr, line_number,
                       "field " + std::to_string(i + 1) +
                           " is not a number: '" + trim(fields[i]) + "'");
        break;
      }
    }
    if (!parsed) {
      ++parse_errors;
      if (!options.skip_invalid) return kExitInput;
      continue;
    }

    ImuSample sample;
    sample.timestamp = values[0];
    sample.gyro = Eigen::Vector3d(values[1], values[2], values[3]);
    sample.accel = Eigen::Vector3d(values[4], values[5], values[6]);
    const ImuMotionHealthState state = pipeline.process(sample);
    ++rows_read;
    if (std::isfinite(sample.timestamp)) {
      if (!have_first_timestamp) {
        have_first_timestamp = true;
        first_timestamp = sample.timestamp;
      }
      have_last_timestamp = true;
      last_timestamp = sample.timestamp;
    }
    if (jsonl != nullptr) {
      *jsonl << localization_zoo::imu_motion_health::toJson(state) << '\n';
    }
    drainAndWriteEvents(&pipeline, events, &event_stats);
  }

  if (input->bad()) {
    std::cerr << "imu_motion_health_cli: error while reading input\n";
    return kExitIo;
  }
  if (rows_read == 0) {
    std::cerr << "imu_motion_health_cli: input contains no data rows\n";
    return kExitInput;
  }

  // Close any held impact/fall/vibration/moving lifecycle records at EOF so
  // every started event has a deterministic ended counterpart in a replay.
  pipeline.flushEvents(have_last_timestamp ? last_timestamp : 0.0);
  drainAndWriteEvents(&pipeline, events, &event_stats);

  const std::string summary_json =
      makeSummary(options, pipeline, rows_read, parse_errors, header_seen,
                  have_first_timestamp, first_timestamp, have_last_timestamp,
                  last_timestamp, event_stats);
  *summary << summary_json << '\n';
  if (jsonl != nullptr) jsonl->flush();
  if (events != nullptr) events->flush();
  summary->flush();
  if (!(*summary) || (jsonl != nullptr && !(*jsonl)) ||
      (events != nullptr && !(*events))) {
    std::cerr << "imu_motion_health_cli: error while writing output\n";
    return kExitIo;
  }
  return parse_errors == 0 ? kExitOk : kExitInput;
}
