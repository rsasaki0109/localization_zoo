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

The input has seven comma-separated columns, in SI units:

```text
timestamp,gx,gy,gz,ax,ay,az
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
`tilted`, `gyro_bias_jump`, `gyro_bias_delta_norm`,
`gyro_bias_jump_duration_s`, and `diagnostic`.  Vectors are `[x,y,z]`; quaternions are
`[w,x,y,z]`.  Invalid floating-point values are encoded as JSON `null`, never
as non-standard `NaN`/`Infinity` tokens.

The final object has `schema: "imu_motion_health_summary_v1"`, stream counts,
first/last timestamp and duration, final motion/health state, counters, and a
nested `final_state`.  It also includes `events_started`, `events_ended`,
`events_dropped`, and `event_counts` (completed events by type, including
`bias_jump`).  A shortened
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
  "event_counts": {"impact": 1, "fall": 0, "vibration": 0, "moving": 0, "bias_jump": 0},
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
duration; it degrades health and appears in the same lifecycle stream.  The
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
- Accelerometer gravity provides roll/pitch leveling only.  Six-axis IMU data
  cannot observe absolute yaw, so yaw drifts with gyro bias.
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
