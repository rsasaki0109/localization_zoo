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
| `--imu-dr-nhc` | Enable non-holonomic constraints (zero lateral/vertical body velocity; forward-only). |
| `--imu-dr-nhc-gain <gain>` | Soft NHC pull strength (default 0 = hard projection). |
| `--imu-dr-forward-axis <0\|1\|2>` | Body forward axis index (default 0 = x, KITTI Velodyne). |
| `--imu-dr-accel-bias` | Estimate accelerometer bias from the static-init window. |

## Tests

`test_imu_dead_reckoning` (7 cases, `ctest -R imu_dead_reckoning`):
`StaticWithKnownBiasHasNoDrift`, `ConstantYawRateNoTranslation`,
`ConstantWorldAccelerationMatchesAnalytic`,
`ZuptResetsVelocityAndLimitsTailDrift`, `IntegrateTrajectoryFrameSampling`,
`NhcSuppressesLateralVelocity`, `AccelBiasEstimationRemovesStaticBias`.

## Reproduce (requires `imu.csv`)

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/nclt_2013_01_10_120 \
  experiments/reference_data/nclt_2013_01_10_120_gt.csv \
  --methods imu_dead_reckoning
```

Add `--imu-dr-zupt` / `--imu-dr-euler` / `--imu-dr-no-gyro-bias` /
`--imu-dr-nhc` / `--imu-dr-accel-bias` to run an ablation variant. Full manifest:
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
| `--imu-dr-nhc` | 9.156 | 156.297 | Hard NHC alone: ~same ATE as default, mild RPE improvement (-8%). Lateral velocity stripping is a second-order effect on this short window. |
| `--imu-dr-nhc --imu-dr-zupt` | 3.717 | 18.521 | NHC+ZUPT stacks on this window: slightly worse ATE than ZUPT alone (2.887 m) but much better RPE (18.5% vs 30.6%). |
| `--imu-dr-accel-bias` | 9.075 | 170.704 | +0.04%/+0.05% — negligible. Estimating accel bias against fixed standard gravity finds almost no residual here: NCLT's ms25 static accel norm is very close to 9.80665, so there is little bias to remove. |

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
| `--imu-dr-nhc` | 46003.188 | 16267.197 | -84.1% ATE / -75.9% RPE vs. default. NHC alone is the second-most effective single aid on the full session after ZUPT, by suppressing lateral velocity drift that double integration would otherwise accumulate. |
| `--imu-dr-nhc --imu-dr-zupt` | 9605.455 | 1901.379 | -96.7% ATE / -97.2% RPE vs. default; **beats ZUPT alone** (14531.743 m / 2859.304%) on both metrics — the two classical vehicle aids stack on continuous-motion data. |
| `--imu-dr-accel-bias` | 288751.377 | 67446.881 | +0.02%/+0.02% — negligible, same explanation as the 120-frame window (ms25 static norm ≈ standard gravity, little accel bias to remove). |

All seven runs returned finite, physically explicable numbers (no NaNs, no
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

On this full session the runner's automatic heuristic now labels `nhc_zupt`
"current default" (it beats ZUPT alone on both metrics); as with the other
windows this is a score-based label only -- the repository default remains
pure dead reckoning with all aids off.

## Result (KITTI Raw drive 2011_09_26_0009, OXTS)

Second dataset, using the established KITTI Raw fixture lineage
(`evaluation/scripts/kitti_raw_to_benchmark.py` +
`evaluation/scripts/kitti_oxts_imu_for_dogfooding.py`). Drive + calib
downloaded from the public, unauthenticated
`s3.eu-central-1.amazonaws.com/avg-kitti` mirror (`2011_09_26_drive_0009_sync.zip`,
1.79 GB; `2011_09_26_calib.zip`, 4 KB) to
`/media/sasaki/aiueo/loc_zoo/kitti_raw_download` (external SSD; local disk had
~16 GB free). GT for both windows was independently regenerated from the raw
drive and verified byte-identical (md5) to the already-committed
`experiments/reference_data/kitti_raw_0009_200_gt.csv` /
`kitti_raw_0009_full_gt.csv`, so no GT changes were needed.

**OXTS rate finding**: the sync package's `oxts/data/*.txt` is one packet per
Velodyne frame (447 OXTS samples vs. 443 retained Velodyne frames over
~46.2 s), i.e. **~9.7 Hz effective**, not the 100 Hz raw OXTS rate (that only
exists in the separate, unsynced `_extract` package). `--write-imu-csv` writes
one midpoint IMU row per LiDAR interval, so this is sparse for a strapdown DR
baseline -- worse than NCLT's ms25 ~47 Hz. A 100 Hz variant from the
`_extract` package's `oxts/timestamps.txt` was considered but **not built**:
`frame_timestamps.csv`'s `timestamp` column is the Velodyne file's positional
index (0, 1, 2, ...), not real time, and `pcd_dogfooding`'s GT association for
this fixture directly compares `imu.csv` stamps against those same integer
indices -- so `imu.csv` cannot be expressed in real seconds without breaking
GT association or the sample-selection walk in `integrateImuTrajectory`.
Rescaling would require changing `frame_timestamps.csv`/GT semantics shared by
every other method's `kitti_raw_0009*` leaderboard entry, well outside this
task's scope. Documented here rather than attempted.

**Deeper unit/timescale caveat (more important than the rate alone)**: because
`imu.csv` stamps live on that same synthetic per-frame-index timeline (rows
spaced exactly 1.0 index-unit apart: `0.5, 1.5, 2.5, ...`), the harness reports
`Frame gap [s]: min=median=mean=max=1.000` and IMU-DR's literal double
integration (`dt = imu.stamp[i] - imu.stamp[i-1]`, clamped to
`max_dt=0.5 s`) runs with an effective step of **0.5 s** per ~0.103 s of real
elapsed time (~4.8x too large per step) -- a structural property of this
fixture (shared by every `imu.csv`-consuming method here, tolerable for
LIO-style methods that use IMU as a soft prior, but not innocuous for a
literal-physics DR baseline). The 2.0 s `static_init_duration_s` window is
similarly compressed to only **~3 IMU samples (~0.3 s real time)** instead of
a genuine 2 s average.

**Static-init validity finding**: KITTI drive 0009 is a moving car, and the
static-init assumption is violated in the most direct way possible -- OXTS
forward speed at frame 0 is **~10.7 m/s** (already cruising), and over the
200-frame window the vehicle's speed **never drops below 1.339 m/s**. Despite
this, the pipeline's static-init warning (gated on gyro std vs. a 0.05 rad/s
threshold) **did not fire** on either window: the check only looks at
rotational noise, and this particular cruise segment happened to have low yaw
rate, so a real translational-motion violation of "static" produced no
warning. This is a genuine blind spot in the built-in stationarity check for
wheeled-vehicle data, not a bug in this evaluation.

**ZUPT false positives**: on the 200-frame window, ZUPT fires on 174/199
(~87%) of IMU samples even though ground-truth OXTS speed **never falls below
1.339 m/s** anywhere in that window (i.e. essentially 100% of ZUPT triggers
are false positives on low-yaw-rate cruising, not real stops). On the full
443-frame sequence, ZUPT fires on 344/442 (~78%) of samples vs. an actual
measured stationary fraction of only ~9.7% (43/443, concentrated near the end
where the vehicle slows for what looks like an intersection) -- roughly 8 out
of 9 triggers are false positives there too. The gyro/accel-norm gate cannot
distinguish "stationary" from "cruising straight at near-constant speed."
Despite this, ZUPT remains the single most effective aid at both window
sizes (see tables below), by bounding velocity-error growth via frequent
resets rather than by correctly detecting rest.

**Gyro-bias reversal (new, dataset-specific finding)**: unlike both NCLT
windows (where disabling static-init gyro-bias estimation was by far the
*worst* ablation, +133%/+172% ATE), on KITTI Raw 0009 disabling it is an
*improvement* on both windows (-87% ATE at 200 frames, -87% ATE at 443
frames). Honest explanation: the ~3-sample (~0.3 s real-time) static-init
window here is taken while the vehicle is already moving at ~10.7 m/s, so the
"bias" it estimates is contaminated by real angular rate rather than pure
sensor bias -- applying it as a correction is actively harmful. This
reproduces consistently across both window sizes, so it is a real dataset
effect, not an aggregation artifact of one window.

| Variant (200-frame window, ~19.9 s, 186.97 m) | ATE (m) | RPE (%/100m) | Notes |
|---|---|---|---|
| Default (pure DR, midpoint, no ZUPT) | 9769.887 | 7580.498 | zupt_frames=0; ~52x trajectory length. |
| `--imu-dr-zupt` | 214.400 | 172.926 | -97.81%/-97.72%; zupt_frames=174/199 (~87%) but OXTS speed never < 1.339 m/s here -- essentially all false positives. |
| `--imu-dr-euler` | 9930.599 | 7725.910 | +1.64%/+1.92%; still a small second-order effect. |
| `--imu-dr-no-gyro-bias` | 1235.339 | 1024.241 | -87.36%/-86.49% -- **improvement**, opposite ordering from NCLT; see gyro-bias reversal finding above. |
| `--imu-dr-nhc` | 7887.422 | 6393.419 | -19.3%/-15.7% vs. default; NHC helps even on this sparse ~9.7 Hz OXTS fixture. |
| `--imu-dr-nhc --imu-dr-zupt` | 205.442 | 172.342 | Comparable to ZUPT alone (214.400 m / 172.926%); NHC does not materially change the ZUPT-dominated outcome on this window. |
| `--imu-dr-accel-bias` | 9626.068 | 7468.717 | -1.47%/-1.47% — a small but real improvement here, unlike NCLT: the OXTS-derived accel has a measurable static residual against standard gravity that this ablation removes. |

| Variant (full 443-frame sequence, ~44.2 s, 332.42 m) | ATE (m) | RPE (%/100m) | Notes |
|---|---|---|---|
| Default (pure DR, midpoint, no ZUPT) | 91821.017 | 51751.724 | zupt_frames=0; ~276x trajectory length. |
| `--imu-dr-zupt` | 5958.011 | 4510.478 | -93.51%/-91.28%; zupt_frames=344/442 (~78%) vs. ~9.7% actual stationary fraction. |
| `--imu-dr-euler` | 92462.565 | 52137.410 | +0.70%/+0.75%; still second-order. |
| `--imu-dr-no-gyro-bias` | 11794.635 | 6491.912 | -87.15%/-87.46% -- confirms the 200-frame window's reversal, not an artifact. |
| `--imu-dr-nhc` | 31967.399 | 18546.068 | -65.2%/-64.2% vs. default. |
| `--imu-dr-nhc --imu-dr-zupt` | 1063.760 | 827.371 | -98.8%/-98.4% vs. default; **beats ZUPT alone** (5958.011 m / 4510.478%) — same stacking effect as the NCLT full session. |
| `--imu-dr-accel-bias` | 90422.524 | 50966.134 | -1.52%/-1.52% — confirms the 200-frame window's small improvement, not an artifact. |

Manifests:
[`experiments/imu_dead_reckoning_kitti_raw_0009_matrix.json`](../../experiments/imu_dead_reckoning_kitti_raw_0009_matrix.json),
[`experiments/imu_dead_reckoning_kitti_raw_0009_full_matrix.json`](../../experiments/imu_dead_reckoning_kitti_raw_0009_full_matrix.json);
aggregates:
[`experiments/results/imu_dead_reckoning_kitti_raw_0009_matrix.json`](../../experiments/results/imu_dead_reckoning_kitti_raw_0009_matrix.json),
[`experiments/results/imu_dead_reckoning_kitti_raw_0009_full_matrix.json`](../../experiments/results/imu_dead_reckoning_kitti_raw_0009_full_matrix.json).
As with NCLT, the experiment-matrix runner's automatic heuristic labels an
aided variant "current default" in both aggregates purely on the shared
ATE/RPE score (`zupt_kitti_0009` on the 200-frame window, `nhc_zupt` on the
full sequence); the method's actual repository default remains ZUPT-off pure
dead reckoning -- see the ZUPT false-positive finding above for why this
particular number should not be read as a general recommendation.

Family context on the same windows (IMU-only DR methods; not on the KITTI
point-cloud leaderboard). Note: OdoNet/NHC-Net/NN-ZUPT were all trained on
KITTI Raw OXTS data (`papers/{odonet,nhc_net,nn_zupt}/scripts/build_kitti_*_dataset.py`,
default `--val-drive 2011_09_26_drive_0056_sync`) -- this is the first
in-domain KITTI Raw OXTS evaluation for these methods (previous results were
cross-sensor transfer to HDL-400/NCLT). Whether drive 0009 itself was part of
the original training corpus is **not recorded** (only the held-out val drive
is documented); some train/eval overlap for these three methods cannot be
ruled out, unlike IMU-DR which is never trained:

| Method (200-frame window) | ATE (m) | RPE (%/100m) | Notes |
|---|---|---|---|
| OdoNet | 855.668 | 883.201 | cnn_frames=150 zupt_frames=49; KITTI-OXTS-trained weights, in-domain sensor. |
| NHC-Net | 121.881 | 99.144 | vmsc_frames=150 zupt_frames=0. |
| NN-ZUPT | 122.547 | 99.982 | nn_frames=150 zupt_frames=0. |

| Method (full 443-frame sequence) | ATE (m) | RPE (%/100m) | Notes |
|---|---|---|---|
| OdoNet | 1723.136 | 982.819 | cnn_frames=393 zupt_frames=49. |
| NHC-Net | 180.504 | 88.481 | vmsc_frames=393 zupt_frames=0. |
| NN-ZUPT | 186.050 | 93.636 | nn_frames=393 zupt_frames=0. |

All three learned-aid methods beat pure IMU-DR by 1-2 orders of magnitude
here (in-domain sensor, unlike the NCLT cross-sensor transfer where OdoNet
was a honest negative) -- consistent with the family's design intent, and a
useful sanity check that the learned aids are not simply broken on this
drive.

## Limitations / scope notes

- Full-session export lives on removable media (`/media/sasaki/aiueo`, an
  external SSD) because the ~6.3 GB ASCII PCD export did not fit on this
  machine's root filesystem (~18 GB free). If that drive is not mounted, the
  full-session manifest's `pcd_dir` will not resolve; re-export with:
  `python3 evaluation/scripts/prepare_nclt_inputs.py --velodyne-dir data/nclt_2013_01_10/2013-01-10/velodyne_sync --ground-truth data/nclt_2013_01_10/groundtruth_2013-01-10.csv --ms25 data/nclt_2013_01_10/2013-01-10/ms25.csv --date 2013-01-10 --max-frames -1 --output-root <ssd path>/dogfooding_results --reference-data-dir experiments/reference_data`
  (stem becomes `nclt_2013_01_10_full`; the regenerated GT CSV was verified
  byte-identical, md5 `ff32d5666754fc1fb95333a3835752f4`, to the one already
  committed at `experiments/reference_data/nclt_2013_01_10_full_gt.csv`).
- KITTI **Odometry** trees have no IMU, so the method **skips** there (like
  OdoNet / NHC-Net / NN-ZUPT). KITTI **Raw** OXTS is now evaluated (see
  above) via `evaluation/scripts/kitti_oxts_imu_for_dogfooding.py`.
- KITTI Raw drive 0009's downloaded sync package is missing Velodyne frames
  177-180 (native indices) in the middle of the sequence while OXTS has no
  such gap (447 contiguous OXTS samples vs. 443 retained Velodyne files).
  Discovered while preparing this evaluation. Because
  `kitti_raw_to_benchmark.py` indexes both point clouds and OXTS poses by
  *position* in their respective (gap-compacted for Velodyne, gap-free for
  OXTS) file lists rather than by native frame number, every exported frame
  from position 177 onward pairs its GT pose with a point cloud captured ~4
  native frames (~0.4 s) later than the OXTS sample says — a pre-existing
  GT-to-point-cloud skew affecting the scan-matching entries of the whole
  `kitti_raw_0009*` fixture family (dozens of methods), **not** something
  this pass introduced. It does **not** affect IMU-DR/OdoNet/NHC-Net/NN-ZUPT:
  their GT and `imu.csv` are both purely OXTS-native and positionally
  consistent with each other regardless of the Velodyne gap. Fixing the
  shared exporter is out of scope for this pass (would touch every existing
  `kitti_raw_0009` leaderboard entry); flagged here for whoever picks it up.
- FPS numbers reported by the harness for this method are not meaningful
  next to point-cloud registration methods — there is no per-frame
  scan-matching cost, only IMU integration between frame timestamps.
- This is a lower-bound reference, not a tuned filter: no EKF, no
  loosely/tightly-coupled fusion, no learned aiding. Compare against
  OdoNet/NHC-Net/NN-ZUPT (same strapdown core plus a trained CNN aid) for
  how much a cheap learned aid buys over this baseline.
