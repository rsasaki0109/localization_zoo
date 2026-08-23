# IMU fault matrix, evaluator, and replay dashboard

This directory contains deterministic, LiDAR-free evaluation tooling for the
`imu_motion_health` C++ core and CLI.  It uses only the Python standard
library.  The only optional dependency is `matplotlib` when a PNG plot is
requested.

## Generate reproducible fixtures

```sh
python papers/imu_motion_health/evaluation/generate_fault_matrix.py \
  --output-dir build/imu_faults
```

The output includes a CSV and a truth JSON for each scenario, plus
`manifest.json`:

| Scenario | Injected condition | CLI evidence expected |
| --- | --- | --- |
| `nominal` | quiet gravity-aligned stream | no integrity flags; stationary |
| `impact` | 40 m/s² specific-force spike | `impact` state |
| `freefall_fall` | 110 ms free-fall and landing spike | `fall` state |
| `vibration` | 15 Hz, 2.5 m/s² lateral vibration | `vibration` state |
| `timestamp_gap` | 400 ms monotonic gap | `gap`, `timestamp_gaps` |
| `timestamp_backward` | one 40 ms backward timestamp | `nonmonotonic`, counter |
| `nonfinite` | one NaN gyro component | `nonfinite`, counter |
| `gyro_saturation` | 40 rad/s gyro sample | `gyro_saturated`, counter |
| `accel_saturation` | 160 m/s² accelerometer sample | `accel_saturated`, counter |
| `row_dropout` | 35 removed rows (360 ms gap) | `gap`, `timestamp_gaps` |
| `gyro_bias_jump` | 0.8 rad/s stationary gyro step | `gyro_bias_jump`, degraded health, counter and lifecycle event |

The generator has no random source.  `--seed` is recorded in the manifest so
that future stochastic fixtures can extend the same contract without making
current fixtures non-reproducible.  CSV values are SI units and match the CLI
header `timestamp,gx,gy,gz,ax,ay,az`; `nan` is intentionally present only in
the nonfinite fixture.

## Replay and score

For a one-command smoke demo (including HTML for `impact`), use
`demo/run_fault_matrix_demo.py`:

```sh
python papers/imu_motion_health/demo/run_fault_matrix_demo.py \
  --output-dir build/imu_demo \
  --cli build/imu_motion_health/imu_motion_health_cli
```

```sh
python papers/imu_motion_health/evaluation/evaluate_fault_matrix.py \
  --manifest build/imu_faults/manifest.json \
  --cli build/imu_motion_health/imu_motion_health_cli \
  --artifacts-dir build/imu_faults/runs \
  --output build/imu_faults/report.json
```

On Windows, point `--cli` at `Release/imu_motion_health_cli.exe`.  The
evaluator writes one JSONL trace and summary per scenario, then checks truth
observations against per-snapshot flags, summary counters, and motion states.
It returns `0` only when every scenario passes.  Existing artifacts can be
scored without replaying by using `--runs-dir` instead of `--cli`; accepted
layouts are `<scenario>.jsonl` plus `<scenario>.summary.json`, or one
directory per scenario.

The evaluator asks the current CLI for `<scenario>.events.jsonl` and scores
the started/ended lifecycle records alongside snapshots and counters. Existing
event artifacts can instead be supplied with `--events-dir`.

## Self-contained HTML replay

```sh
python papers/imu_motion_health/evaluation/render_dashboard.py \
  --jsonl build/imu_faults/runs/impact.jsonl \
  --summary build/imu_faults/runs/impact.summary.json \
  --truth build/imu_faults/impact.truth.json \
  --output build/imu_faults/impact.html
```

Open the resulting file directly.  It embeds the snapshot series, cards for
motion/health/confidence/relative motion, slider and play/pause replay
controls, SVG metric plots, injected truth intervals, events, and counters.
No HTTP server, LiDAR input, CDN, or JavaScript package is required.  Add
`--png-output build/imu_faults/impact.png` when matplotlib is available for a
small confidence/acceleration companion plot; HTML generation never depends
on matplotlib.

## Verification and hardware acceptance

The Python tests are compatible with both unittest and pytest:

```sh
python papers/imu_motion_health/test/test_imu_evaluation.py -v
pytest papers/imu_motion_health/test/test_imu_evaluation.py
```

The matrix verifies parser/core behavior and deterministic threshold gates;
it is not a substitute for sensor acceptance.  For a physical IMU, record
the same seven-column CSV with the device stationary for at least the startup
window, then repeat each controlled action (tap/impact, free-fall in a safe
fixture, sinusoidal vibration, timestamp/packet fault injection, and a bias
step if the driver supports it).  Replay with the exact sensor full-scale
limits and profile thresholds, archive CSV/truth/JSONL/summary/report/HTML,
and require:

1. stationary startup is `ready` with trusted bias and finite gravity;
2. every injected integrity fault appears in both per-sample diagnostics and
   the corresponding summary counter;
3. impact/fall/vibration detection latency and false alarms meet the
   application profile's acceptance limits;
4. a clean final stationary segment returns to `stationary`/`ready`; and
5. relative position/velocity are treated as short-term values and are not
   accepted as global localization without an aiding sensor.

## Calibration and hardware validation

The calibration pipeline accepts driver output with this header (temperature
is optional but required to identify temperature drift):

```text
timestamp,gx,gy,gz,ax,ay,az,temperature_c
```

Archive a driver stream from stdin together with traceable metadata:

```sh
sensor_driver --csv | python papers/imu_motion_health/evaluation/calibration_validation.py record \
  --input - --output recordings/static_cold.csv \
  --metadata recordings/static_cold.metadata.json \
  --device-id imu-001 --activity stationary
```

Record level stationary sessions near the lowest and highest expected device
temperature, at least 60 seconds each. Fit a calibration artifact and convert
it to ROS parameters:

```sh
python papers/imu_motion_health/evaluation/calibration_validation.py calibrate \
  --input recordings/static_cold.csv --input recordings/static_hot.csv \
  --output recordings/calibration.json
python papers/imu_motion_health/evaluation/calibration_validation.py export-ros \
  --calibration recordings/calibration.json \
  --output recordings/imu_calibration.yaml
```

Create `manifest.json` with stationary, walking, and vehicle sessions:

```json
{"schema":"imu_validation_manifest_v1","sessions":[
  {"name":"static","activity":"stationary","path":"static_check.csv"},
  {"name":"walking","activity":"walking","path":"walking.csv"},
  {"name":"vehicle","activity":"vehicle","path":"vehicle.csv"}
]}
```

Then run the same C++ CLI used in production and render the evidence bundle:

```sh
python papers/imu_motion_health/evaluation/calibration_validation.py validate \
  --manifest recordings/manifest.json --calibration recordings/calibration.json \
  --cli build/imu_motion_health/imu_motion_health_cli \
  --artifacts-dir recordings/runs --output recordings/validation.json
python papers/imu_motion_health/evaluation/calibration_validation.py report \
  --validation recordings/validation.json --calibration recordings/calibration.json \
  --output recordings/report.html
python papers/imu_motion_health/evaluation/calibration_validation.py endurance \
  --input recordings/static_check.csv --calibration recordings/calibration.json \
  --cli build/imu_motion_health/imu_motion_health_cli \
  --duration-s 86400 --output recordings/endurance.json
```

Default gates require monotonic timestamps, at least 50 Hz, no interval over
250 ms, and corrected stationary gyro RMS at most 0.03 rad/s. The JSON output
contains every individual check, so deployments can tighten these limits in
CI. A two-temperature calibration should span the deployed temperature range;
a zero span is valid only as a constant-bias calibration and is explicitly
reported in `quality.temperature_span_c`.

For ROS 2, export the artifact and launch the calibration node immediately
before Motion & Health. `sensor_msgs/Temperature` is optional at runtime; if
it is absent, the reference-temperature bias is still applied:

```sh
ros2 launch localization_zoo_ros imu_calibrated_motion_health.launch.py \
  calibration_params_file:=recordings/imu_calibration.yaml
python papers/imu_motion_health/evaluation/ros2_endurance.py \
  --duration-s 86400 --output recordings/ros2_endurance.json -- \
  ros2 launch localization_zoo_ros imu_calibrated_motion_health.launch.py \
  calibration_params_file:=recordings/imu_calibration.yaml
```

The watchdog fails when the ROS launch exits before the required duration and
archives output tails for diagnosis. Run rosbag playback or the physical
driver concurrently, and archive its bag beside the generated JSON/HTML.
