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
