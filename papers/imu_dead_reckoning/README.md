# IMU Dead Reckoning

Pure strapdown IMU-only dead reckoning. This is a **compact baseline, not a
paper reimplementation** — the unaided IMU-only lower-bound reference for the
LIO family (compare OdoNet / NHC-Net / NN-ZUPT, which all add a learned aid on
top of the same strapdown core). Claim tier **T3 smoke / concept** per
[`docs/paper_ready_reproducibility.md`](../../docs/paper_ready_reproducibility.md):
"This is a compact baseline or concept port."

## Core idea

1. **Static-window init** (first 2.0 s by default) — gyro bias estimated from
   the mean static gyro reading; initial roll/pitch aligned from the mean
   static accel vector; gravity magnitude taken from the static accel norm
   (falls back to 9.80665 if disabled).
2. **Midpoint quaternion/rotation integration** of bias-corrected gyro to
   propagate attitude (forward Euler available as an ablation).
3. **Double integration of specific force** — world-frame acceleration is
   `R * a - g_w`, integrated once for velocity and once for position. No
   NHC, no ZUPT, no LiDAR, no learned aiding unless explicitly enabled.

Everything beyond the strapdown core is an opt-in ablation knob, off by
default:

| Flag | Effect |
|---|---|
| `--imu-dr-zupt` | Enable zero-velocity updates (zeroes velocity when a windowed gyro/accel-norm gate detects "stationary"). |
| `--imu-dr-euler` | Forward Euler instead of the default midpoint gyro integration. |
| `--imu-dr-no-gyro-bias` | Skip static-window gyro-bias estimation (bias stays zero). |
| `--imu-dr-static-init-sec <s>` | Override the static-init window length (default 2.0 s). |
| `--imu-dr-zupt-gyro-threshold <rad/s>` | ZUPT gyro-norm gate (default 0.05). |
| `--imu-dr-zupt-accel-tolerance <m/s^2>` | ZUPT `\|\|a\| - g\|` gate (default 0.8). |

## Tests

`test_imu_dead_reckoning` (5 cases, `ctest -R imu_dead_reckoning`):
`StaticWithKnownBiasHasNoDrift`, `ConstantYawRateNoTranslation`,
`ConstantWorldAccelerationMatchesAnalytic`,
`ZuptResetsVelocityAndLimitsTailDrift`, `IntegrateTrajectoryFrameSampling`.

## Reproduce (requires `imu.csv`)

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/nclt_2013_01_10_120 \
  experiments/reference_data/nclt_2013_01_10_120_gt.csv \
  --methods imu_dead_reckoning
```

Add `--imu-dr-zupt` / `--imu-dr-euler` / `--imu-dr-no-gyro-bias` to run an
ablation variant. Full manifest:
[`experiments/imu_dead_reckoning_nclt_2013_01_10_matrix.json`](../../experiments/imu_dead_reckoning_nclt_2013_01_10_matrix.json),
aggregate result:
[`experiments/results/imu_dead_reckoning_nclt_2013_01_10_matrix.json`](../../experiments/results/imu_dead_reckoning_nclt_2013_01_10_matrix.json).

## Result (NCLT 2013-01-10, 120-frame window, ms25 IMU ~47 Hz, 11.5 m / ~24 s)

| Variant | ATE (m) | RPE (%/100m) | Notes |
|---|---|---|---|
| Default (pure DR, midpoint, no ZUPT) | 9.071 | 170.617 | Repository default; zupt_frames=0. |
| `--imu-dr-zupt` | 2.887 | 30.607 | zupt_frames=481/1051 IMU samples gated stationary — this short window has the vehicle at rest for a large fraction of the time, so the ATE/RPE win is **not representative of continuous motion** and should not be overclaimed. |
| `--imu-dr-euler` | 10.280 | 198.899 | Forward Euler vs default midpoint; integration scheme is a second-order effect next to ZUPT/gyro-bias. |
| `--imu-dr-no-gyro-bias` | 24.676 | 482.028 | Largest degradation of any ablation tested — static-window gyro-bias estimation is the single most load-bearing component of this baseline. |

Family context on the same window (IMU-only dead-reckoning methods, for
scale — none of these are LiDAR/LIO and should not be read against the KITTI
point-cloud leaderboard):

| Method | ATE (m) | RPE (%/100m) | Notes |
|---|---|---|---|
| OdoNet | 138.872 | 1842.230 | KITTI-Raw-OXTS-trained CNN weights, out-of-domain on NCLT's ms25 IMU. |
| NHC-Net | 4.295 | 102.938 | |
| NN-ZUPT | 3.799 | 96.164 | |

Note: the experiment-matrix runner's automatic heuristic labeled `zupt`
"Adopt as current default" in
[`experiments/results/imu_dead_reckoning_nclt_2013_01_10_matrix.json`](../../experiments/results/imu_dead_reckoning_nclt_2013_01_10_matrix.json)
purely on the shared ATE/RPE score. The method's actual repository default
remains pure dead reckoning (ZUPT off) — see the stationarity caveat above
for why the `zupt` number on this particular window should not be read as a
general recommendation.

## Result (NCLT 2013-01-10, full session, 5105 frames, ~17 min / 1021.7 s, 1138.8 m)

Full-session export (~6.3 GB of ASCII PCDs; only needed for frame-count/GT
bookkeeping since IMU-DR does not read point clouds) lives on removable media
(`/media/sasaki/aiueo`, see [Limitations](#limitations--scope-notes)) because
the local disk had ~18 GB free at evaluation time.

| Variant | ATE (m) | RPE (%/100m) | Notes |
|---|---|---|---|
| Default (pure DR, midpoint, no ZUPT) | 288700.449 | 67435.005 | zupt_frames=0; ~250x the 1138.8 m trajectory length in RMSE -- uncorrected gyro bias/heading error compounds for the full ~17 min instead of the window's ~24 s. |
| `--imu-dr-zupt` | 14531.743 | 2859.304 | -94.97% ATE / -95.76% RPE vs. default. zupt_frames=3984/48122 IMU samples (~8.3%) gated stationary -- much lower than the 120-frame window's ~46%, so this session is mostly continuous motion, yet ZUPT still removes ~95% of the drift by repeatedly re-zeroing velocity error before it integrates into position error. |
| `--imu-dr-euler` | 291892.627 | 68484.744 | +1.11% ATE / +1.56% RPE vs. default. Integration scheme stays a small second-order effect even over ~17 minutes, same conclusion as the 120-frame window. |
| `--imu-dr-no-gyro-bias` | 672302.751 | 139392.868 | +132.9% ATE / +106.7% RPE vs. default -- again the largest degradation of the three ablations, confirming static-init gyro-bias estimation as the single most load-bearing aid at any timescale tested. |

All four runs returned finite, physically explicable numbers (no NaNs, no
`1e6`+ km positions) -- the hundreds-of-km ATE is the expected honest failure
mode of unaided IMU-only dead reckoning compounding over ~17 minutes with no
external aiding, not a bug. Full-session manifest:
[`experiments/imu_dead_reckoning_nclt_2013_01_10_full_matrix.json`](../../experiments/imu_dead_reckoning_nclt_2013_01_10_full_matrix.json),
aggregate:
[`experiments/results/imu_dead_reckoning_nclt_2013_01_10_full_matrix.json`](../../experiments/results/imu_dead_reckoning_nclt_2013_01_10_full_matrix.json).

The 120-frame window's ZUPT caveat above should now be read alongside this
full-session number: the short window's ~46% (481/1051) stationary fraction
was flagged as unrepresentative, and the full session confirms that -- true
stationary fraction is only ~8.3% (3984/48122) -- but ZUPT is *still* the
dominant single aid at both scales, just for a different reason (bounding
velocity-error growth between infrequent resets rather than reflecting mostly
parked time).

## Limitations / scope notes

- Full-session export lives on removable media (`/media/sasaki/aiueo`, an
  external SSD) because the ~6.3 GB ASCII PCD export did not fit on this
  machine's root filesystem (~18 GB free). If that drive is not mounted, the
  full-session manifest's `pcd_dir` will not resolve; re-export with:
  `python3 evaluation/scripts/prepare_nclt_inputs.py --velodyne-dir data/nclt_2013_01_10/2013-01-10/velodyne_sync --ground-truth data/nclt_2013_01_10/groundtruth_2013-01-10.csv --ms25 data/nclt_2013_01_10/2013-01-10/ms25.csv --date 2013-01-10 --max-frames -1 --output-root <ssd path>/dogfooding_results --reference-data-dir experiments/reference_data`
  (stem becomes `nclt_2013_01_10_full`; the regenerated GT CSV was verified
  byte-identical, md5 `ff32d5666754fc1fb95333a3835752f4`, to the one already
  committed at `experiments/reference_data/nclt_2013_01_10_full_gt.csv`).
- No KITTI evaluation: KITTI Odometry trees have no IMU, so the method
  **skips** there (like OdoNet / NHC-Net / NN-ZUPT). KITTI Raw OXTS has
  `imu.csv` via `evaluation/scripts/kitti_oxts_imu_for_dogfooding.py` but was
  not run as part of this pass.
- FPS numbers reported by the harness for this method are not meaningful
  next to point-cloud registration methods — there is no per-frame
  scan-matching cost, only IMU integration between frame timestamps.
- This is a lower-bound reference, not a tuned filter: no EKF, no
  loosely/tightly-coupled fusion, no learned aiding. Compare against
  OdoNet/NHC-Net/NN-ZUPT (same strapdown core plus a trained CNN aid) for
  how much a cheap learned aid buys over this baseline.
