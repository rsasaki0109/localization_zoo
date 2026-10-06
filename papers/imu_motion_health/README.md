# IMU Motion & Health SDK

`imu_motion_health` is a LiDAR-free, dependency-light 6-axis IMU front end.
It accepts timestamped gyroscope and accelerometer samples and provides:

- startup stationarity/quality evidence, gyro bias, gravity magnitude and
  initial roll/pitch alignment;
- timestamp, non-finite, saturation and sampling-gap diagnostics;
- stationary, moving, impact, fall/free-fall and vibration classifications;
- persistent stationary gyro-bias jump diagnosis with a configurable
  confirmation duration;
- a confidence score and a ready/degraded/invalid health state;
- orientation plus short-term relative velocity and position.

Impact, fall, vibration (and optionally moving) detections are also exposed as
independent lifecycle events.  This means overlapping conditions are retained
even though the backwards-compatible `motion_state` field remains a single
prioritized enum.

The reusable C++ core (`imu_motion_health`) has no ROS or point-cloud
dependency.  `imu_motion_health_cli` is an offline CSV replay tool built on
the same core, so a recorded stream and a live driver exercise identical
logic.

## Build

From the repository root, the module is included by the top-level CMake file.
For a quick standalone build (Eigen3 and GTest must be discoverable):

```sh
cmake -S papers/imu_motion_health -B build/imu_motion_health
cmake --build build/imu_motion_health --config Release
ctest --test-dir build/imu_motion_health -C Release --output-on-failure
```

The executable is named `imu_motion_health_cli` (on Windows, use the
configuration subdirectory if the generator creates one).

## CSV replay

The input has seven comma-separated columns, in SI units. Three optional
magnetometer columns (any unit) may follow:

```text
timestamp,gx,gy,gz,ax,ay,az[,mx,my,mz]
```

`timestamp` is seconds on a monotonic sensor clock, gyro is rad/s, and accel
is m/s² specific force.  A header is optional.  Blank lines and lines whose
first non-space character is `#` are ignored.  The parser is deliberately
strict about column count and numeric tokens; use `--skip-invalid` to continue
after malformed rows.  `nan` and `inf` are numeric tokens and are passed to the
core so they become explicit non-finite health diagnostics rather than being
silently discarded.

The default command writes one final summary JSON object to stdout:

```sh
build/imu_motion_health/imu_motion_health_cli \
  --input recording/imu.csv \
  --summary-output recording/imu.summary.json
```

To save one JSON snapshot per input row as JSONL:

```sh
build/imu_motion_health/imu_motion_health_cli \
  --input recording/imu.csv \
  --jsonl-output recording/imu.jsonl \
  --summary-output recording/imu.summary.json
```

`--emit-jsonl` is shorthand for `--jsonl-output -`.  In that mode snapshots
are printed one per line and the final summary is the last JSON line.  Use
`--summary-output path` when stdout should contain only JSONL snapshots, or
`--jsonl-output path` when stdout should contain only the summary.

To save event lifecycle records, use `--events-output path` or
`--emit-events` (stdout).  Every event has a stable `id` shared by its
`started` and `ended` records.  The CLI closes held events at end-of-file, so a
replay produces deterministic pairs:

```sh
build/imu_motion_health/imu_motion_health_cli \
  --input recording/imu.csv \
  --events-output recording/imu.events.jsonl \
  --summary-output recording/imu.summary.json
```

Useful configuration flags (all distances/angles are in the units shown) are:

| Option | Default | Meaning |
| --- | ---: | --- |
| `--startup-duration S` | 1.0 s | Initial static calibration window |
| `--startup-min-samples N` | 20 | Minimum samples in that window |
| `--gravity MPS2` | 9.80665 | Expected gravity magnitude |
| `--max-gap S` | 0.25 s | Gap diagnostic threshold |
| `--max-integration-dt S` | 0.10 s | Largest interval integrated |
| `--gyro-saturation RADPS` | 34.0 | Gyro full-scale threshold |
| `--accel-saturation MPS2` | 156.9 | Accelerometer full-scale threshold (16 g) |
| `--stationary-gyro RADPS` | 0.06 | Rolling gyro RMS gate |
| `--stationary-accel MPS2` | 0.45 | Stationary gravity/linear-accel tolerance |
| `--moving-gyro RADPS` | 0.16 | Moving gyro gate |
| `--moving-accel MPS2` | 0.65 | Moving linear-accel gate |
| `--impact-accel MPS2` | 25.0 | Impact acceleration gate |
| `--impact-gyro RADPS` | 8.0 | Impact angular-rate gate |
| profile `impact_requires_accel_and_gyro` | false | Require both impact gates instead of the legacy OR rule |
| `--fall-freefall MPS2` | 2.5 | Free-fall norm gate |
| `--fall-min-duration S` | 0.08 s | Free-fall duration before fall event |
| `--tilt-angle DEG` | 55.0° | Quiet-pose tilt threshold |
| `--tilt-min-duration S` | 0.15 s | Tilt duration before fall event |
| `--event-hold S` | 0.20 s | Impact/fall/vibration latch time |
| `--vibration-rms MPS2` | 1.0 | Rolling vibration RMS gate |
| `--motion-window N` | 20 | Number of samples in motion window |
| `--event-queue-capacity N` | 256 | Bounded core event FIFO capacity |
| `--gyro-bias-jump RADPS` | 0.25 | Stationary gyro residual jump gate |
| `--bias-jump-min-duration S` | 0.05 s | Duration before bias-jump confirmation |

## Profiles and YAML configuration

The CLI includes dependency-free built-ins for `default`, `wearable`,
`vehicle`, `machine`, `cargo`, and `drone`.  Select one with `--profile NAME`,
or use one of the checked-in examples under
`papers/imu_motion_health/config/`:

```sh
imu_motion_health_cli --input recording/imu.csv \
  --profile wearable --events-output recording/imu.events.jsonl
```

`--profile-file PATH` accepts a strict scalar YAML subset: comments and blank
lines are allowed, mappings may optionally be under `imu_motion_health:` or
`params:`, and values are finite numbers, booleans, or a `profile:`/`preset:`
name.  Lists, tabs, duplicate keys, unknown keys, and malformed values are
rejected as usage errors.  The deterministic precedence is:

```text
default -> --profile preset -> --profile-file YAML -> explicit CLI options
```

The precedence is independent of the order in which arguments appear.  A
profile changes detector policy while preserving board-specific sensor limits
unless the YAML or CLI overrides them.

Calibration behavior can be changed with `--no-gyro-bias`,
`--estimate-accel-bias`, `--stationary-bias-gain G`, `--leveling-gain G`, and
`--no-zero-velocity`.  Run `imu_motion_health_cli --help` for the complete
list, including inline `--option=value` spelling.

## JSON schema

Each JSONL snapshot is a serialized `ImuMotionHealthState`.  Important fields
include `timestamp`, `dt`, `sample_rate_hz`, `sample_accepted`, `integrated`,
`nonfinite`, `nonmonotonic`, `gap`, `gyro_saturated`, `accel_saturated`,
`startup_*`, `motion_state`, `health_state`, `confidence`, `gyro_bias`,
`accel_bias`, `gravity_world`, `orientation_wxyz`, `velocity`, `position`,
`relative_velocity`, `relative_position`, `tilt_angle_rad`, `tilt_angle_deg`,
`tilted`, `magnetometer_used`, `magnetic_disturbance`, `gyro_bias_jump`,
`gyro_bias_delta_norm`,
`gyro_bias_jump_duration_s`, and `diagnostic`.  Vectors are `[x,y,z]`; quaternions are
`[w,x,y,z]`.  Invalid floating-point values are encoded as JSON `null`, never
as non-standard `NaN`/`Infinity` tokens.

The final object has `schema: "imu_motion_health_summary_v1"`, stream counts,
first/last timestamp and duration, final motion/health state, counters, and a
nested `final_state`.  It also includes `events_started`, `events_ended`,
`events_dropped`, and `event_counts` (completed events by type, including
`bias_jump` and `fall_confirmed`).  A shortened
example is:

```json
{
  "schema": "imu_motion_health_summary_v1",
  "rows_read": 120,
  "parse_errors": 0,
  "duration_s": 1.19,
  "motion_state": "stationary",
  "health_state": "ready",
  "confidence": 0.92,
  "event_counts": {"impact": 1, "fall": 0, "vibration": 0, "moving": 0, "bias_jump": 0, "fall_confirmed": 0},
  "counters": {
    "samples": 120,
    "accepted_samples": 120,
    "timestamp_gaps": 0,
    "gyro_saturated_samples": 0,
    "accel_saturated_samples": 0
  },
  "final_state": {
    "timestamp": 1.19,
    "motion_state": "stationary",
    "health_state": "ready",
    "orientation_wxyz": [1, 0, 0, 0],
    "relative_position": [0, 0, 0]
  }
}
```

The example is abbreviated for readability; actual output includes every
counter and every state field.

Event JSON uses `schema: "imu_motion_health_event_v1"` and contains `id`,
`type`, `phase`, `timestamp`, `start_timestamp`, nullable `end_timestamp`,
`duration_s`, `peak_accel_norm`, `peak_gyro_norm`, `peak_vibration_rms`, and
`peak_confidence`.  `bias_jump` is emitted when a quiet, gravity-consistent
segment has a gyro residual above the configured threshold for the minimum
duration; it degrades health and appears in the same lifecycle stream.
`fall_confirmed` comes only from the opt-in posture confirmation (below). It is
a one-shot record: `started` and `ended` share one ID and timestamp, and the
duration is 0.  The
core queue is bounded by
`ImuMotionHealthParams::event_queue_capacity`; when it fills, the oldest
pending record is dropped and `droppedEventCount()`/`events_dropped` records the
loss.  Applications can call `popEvent`, `drainEvents`, `pendingEventCount`,
and `flushEvents` without depending on ROS or a YAML library.  Ongoing
lifecycle state is available on demand through `activeEvents()` and
`activeEventCount()`; those records use phase `updated`, retain the same ID and
start timestamp, and report current duration/peak metrics.  They are not added
to the pending FIFO, so polling active state does not flood event output.

## Deterministic demo

The standard-library-only demo generator creates a fixed sequence containing
static startup, linear motion, an impact, free-fall, and a final static phase.
It does not download data or use LiDAR:

```sh
python papers/imu_motion_health/demo/run_demo.py \
  --output /tmp/imu_motion_health_demo.csv
```

To generate and replay in one command (PowerShell uses the same argument
spelling):

```sh
python papers/imu_motion_health/demo/run_demo.py \
  --output /tmp/imu_motion_health_demo.csv \
  --cli build/imu_motion_health/imu_motion_health_cli
```

This writes `.jsonl` and `.summary.json` next to the generated CSV.  The
sequence is deterministic, making it suitable for a smoke test in CI or a
first integration with a sensor driver.

## Exit codes

The CLI returns 0 after successfully replaying at least one syntactically
valid row, even when the sensor health state is degraded or invalid; those
conditions are data to inspect in JSON.  It returns 2 for usage/configuration
errors, 3 for input/output file errors, and 4 for malformed CSV rows or an
empty input.  With `--skip-invalid`, malformed rows are reported in the
summary and replay continues, but exit code 4 remains so batch pipelines do
not silently accept a partial stream.

## Semantics and limitations

- The first startup window is assumed to be stationary.  A moving or noisy
  startup transitions to streaming mode but does not promote its mean to a
  trusted bias.
- The orientation comes from a 6D VQF attitude by default (`vqf_attitude`).
  Accelerometer gravity corrects roll and pitch, with a 3 s time constant
  (`vqf_attitude_tau_acc_s`).  VQF starts from the static startup window.
  Six-axis IMU data cannot observe absolute yaw, so yaw drifts with any
  remaining gyro bias.  `vqf_attitude: false` restores gyro integration with
  stationary-only leveling.
- With the VQF attitude, `tilt_angle` is the larger of the orientation's tilt
  and the tilt of the measured gravity in the body frame.  The latter catches
  a quiet post-impact pose before the 3 s correction has moved the
  orientation.
- Velocity and position are short-term relative dead reckoning.  They drift
  without wheel, GNSS, vision, magnetometer, LiDAR, or another aid; this SDK
  intentionally has no LiDAR input path.
- Saturated and timestamp-gap intervals are never integrated.  Non-monotonic
  samples are rejected.  The per-sample flags are edge-triggered; historical
  totals are in `counters`.
- Motion/event thresholds are deterministic gates, not a learned classifier.
  Tune them to the sensor's full-scale range, sample rate, mounting, and
  application before using the state as a safety interlock.

## Fault matrix and replay dashboard

The standard-library evaluation tools generate deterministic nominal, impact,
free-fall, vibration, timestamp, nonfinite, saturation, row-dropout, and
gyro-bias-jump fixtures.  They replay through the same CLI, score snapshot
flags/counters/states/events, and render a self-contained HTML dashboard:

```sh
python papers/imu_motion_health/demo/run_fault_matrix_demo.py \
  --output-dir build/imu_faults \
  --cli build/imu_motion_health/imu_motion_health_cli
```

For the full matrix/evaluator options, existing-artifact mode, event JSONL
contract, dashboard controls, and physical-IMU acceptance procedure, see
[evaluation/README.md](evaluation/README.md).  When Python 3 is available,
`test_imu_motion_health_evaluation` is also registered with CTest alongside
the core and CLI tests. `test_imu_motion_health_ros_wiring` validates the ROS
target, package dependencies, launch file, parameter YAML, topics, profiles,
and event connection even when ROS itself is not installed.

## ROS 2 real-time node

`localization_zoo_ros` includes a dedicated LiDAR-free
`imu_motion_health_node`. It subscribes to `sensor_msgs/Imu` and publishes
short-term `nav_msgs/Odometry`, optional TF, health snapshot JSON, and event
lifecycle JSON. A launch file and ROS parameter YAML are provided; see the
[ROS 2 wrapper guide](../../ros2/localization_zoo_ros/README.md) for topics,
profiles, timestamp policy, and commands.

## Physical calibration and validation pipeline

`evaluation/calibration_validation.py` provides a standard-library-only path
from a driver CSV stream to an archived calibration and acceptance report. It
supports the normal seven CLI columns plus optional `temperature_c`, fits a
three-axis stationary gyro temperature model and accelerometer offset, emits a
corrected seven-column replay, scores stationary/walking/vehicle sessions,
runs extended CLI replay, exports ROS parameters, and renders self-contained
HTML. No LiDAR data or dependency is accepted anywhere in this workflow.

Generate a complete deterministic example with:

```sh
python papers/imu_motion_health/demo/run_calibration_validation_demo.py \
  --output-dir build/imu_calibration_demo \
  --cli build/imu_motion_health/imu_motion_health_cli
```

See [the calibration and hardware guide](evaluation/README.md#calibration-and-hardware-validation)
for recording commands, acceptance gates, ROS 2 temperature compensation,
and long-duration tests.

## Public wearable benchmark

The reproducible [public dataset benchmark](evaluation/public_datasets.md)
fetches three CC BY 4.0 sources, converts them to the CLI schema, enforces
subject-disjoint tuning/test splits, tunes `wearable-public-v1`, replays the
real CLI, and emits JSON plus self-contained HTML. Public-data experiments
also introduced the opt-in `impact_requires_accel_and_gyro` gate; it defaults
to false, so all existing profiles retain their original OR behavior.

### Attitude accuracy on BROAD

[`evaluation/broad_attitude.py`](evaluation/broad_attitude.py) replays the 39
BROAD trials through the CLI. BROAD has motion-capture ground truth, CC BY 4.0;
see [`papers/attitude_estimation`](../attitude_estimation/). The script scores
`orientation_wxyz` by inclination RMSE over the movement phases. Heading is
unobservable without a magnetometer, so it is not scored.

| Configuration | Mean | Median | Max |
|---|---:|---:|---:|
| **`default` (VQF attitude)** | **0.70°** | **0.60°** | **1.8°** |
| `default` with magnetometer columns: total error incl. heading | 2.30° | 1.76° | 7.7° |
| `wearable` | 0.70° | 0.60° | 1.8° |
| `vqf_attitude: false` (integrating) | 9.6° | 7.0° | 24.0° |
| `vqf_attitude: false`, per-sample bias update (before 2026-10-05) | 26.8° | 25.7° | 97.5° |
| `vqf_attitude: false`, `stationary_bias_gain: 0` | 3.4° | 1.8° | 24.1° |

**Heading.** With magnetometer samples (`mx,my,mz` CSV columns, or
`ImuSample::has_mag`) the orientation becomes VQF's 9D estimate in ENU, with
VQF's magnetic disturbance rejection. Its total error, heading included, is
2.30°, equal to standalone VQF. `magnetometer_used` and
`magnetic_disturbance` report the state. `use_magnetometer: false` ignores the
magnetometer.

**Calibrate the magnetometer first.** `mag_offset_*` and `mag_matrix_*`
apply a hard/soft-iron calibration (`m_cal = W (m - b)`) before VQF. Fit them
from a recording that rotates the sensor through many orientations:

```sh
build/papers/attitude_estimation/magnetometer_calibration_cli rotation.csv \
  --columns 7,8,9 --profile-yaml mag.yaml
```

Then pass `mag.yaml` as `--profile-file`. On BROAD with a known distortion,
the uncalibrated heading error is 70° and the calibrated one 2.3°
(`papers/attitude_estimation`).

**The VQF attitude** is a 6D VQF that estimates its own gyro bias with rest
detection. It starts from the static startup window: VQF's bias is set to the
trusted startup bias, and its initial averaging is ended with the window
mean. Two parts of VQF were built for this use and are not in the paper:
`setBiasEstimate` and `endInitialAveraging`. Its BROAD result equals
standalone VQF.

**Defaults that existing tests check.** Becoming the default required two
changes:

- **Tilt from body-frame gravity.** VQF leans towards gravity, so the tilt is
  also measured from the body-frame gravity direction. The quiet-pose tilt
  fall still reads 60°.
- **Integration check moved to the integrating mode.** The exact
  dead-reckoning check now runs with `vqf_attitude: false`. A VQF variant
  allows the ~0.01° lean that 0.2 s of sustained 1 m/s² acceleration causes.

The fault matrix (11/11), the CLI tests, the demo's event counts, and the
public wearable benchmark are unchanged.

**The stationary bias update of the integrating mode.** The stationary gate
uses the bias-corrected gyro. Once slow real rotation has been learned as
bias, the gate stays open and keeps learning it. Two safeguards are on by
default:

- `stationary_bias_reference_rate_hz: 100`: the gain is defined per 100 Hz
  sample and scaled with the sample interval.
- `stationary_bias_min_duration_s: 1.5`: learn only after 1.5 s of
  stationarity.

They were chosen on the odd-numbered BROAD trials only, and the held-out even
trials improved from 24.5° to 9.9°
([`analyze_stationary_bias.py`](evaluation/analyze_stationary_bias.py),
[`stationary_bias_selection.json`](evaluation/stationary_bias_selection.json)).
The SDK's `gyro_bias` output and the motion gates still use this update in
both modes.

### Posture confirmation (opt-in)

`posture_confirmation: true` adds a VQF attitude estimate (6D,
[`papers/attitude_estimation`](../attitude_estimation/)) and emits
`fall_confirmed` once an impact has been followed by a lasting posture change:

- **VQF rule.** The body's up direction must turn by at least
  `posture_threshold_deg` (default 50) from its mean over [t-2.0, t-1.0] s and
  hold for `posture_dwell_s` (0.1), within [t + `posture_start_s` (0), t+1.5]
  s. `posture_tau_acc_s` (3) is VQF's accelerometer time constant.
- **Fallback.** Otherwise, at t+1.5 s, the mean accelerometer direction over
  [t+0.5, t+1.5] s must differ by 50° from the [t-2.0, t-1.0] s mean.

The confirmer sees every finite, time-ordered sample. It sets VQF's fixed rate
from the median interval of the first 16 samples.

The option is off in every built-in profile and on in `wearable-public-v1`.
There it lowers the CGU-BES median detection latency from 2.95 s to 2.13 s at
unchanged sensitivity (96.7%) and false-positive rate (0%). The selection
protocol and test-once evaluation are in
[`evaluation/public_datasets.md`](evaluation/public_datasets.md).
