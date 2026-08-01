# LiDAR Odometry SOTA Track Log

## 2026-07-30: bootstrap

### Environment

- External dataset root: `E:\datasets\loc_zoo`
- Capacity at bootstrap: 1863.02 GiB total, 74.14 GiB free
- WSL: Ubuntu 22.04
- Persistent build source: `/root/localization_zoo-build-src`
- Persistent Release build: `/root/localization_zoo-build-release`
- Large datasets and benchmark JSON remain on the external SSD.

The build source is a 7 MB filtered synchronization of CMake files and C/C++
sources/headers. This avoids compiling through the Windows NTFS mount while
keeping datasets outside the WSL filesystem.

### Dataset readiness

The initial `python evaluation/scripts/verify_lidar_sota_data.py` run found 2/10
suite entries ready:

- NTNU fog full: 1729 frames, robustness-only
- corrected KITTI Raw 0009 full: 443 frames, diagnostic

At that point KITTI Odometry 00/02/05/07/08, MulRan ParkingLot, NCLT
2013-01-10 full, and NTNU tunnel full still needed to be restored or prepared
under the external data root. KITTI Raw remains diagnostic because its exported
timestamps are 1.0 index units apart rather than native seconds.

### NCLT held-out preparation

The official NCLT 2013-01-10 Velodyne, sensor, and ground-truth files were
downloaded directly to the external SSD and verified:

| File | SHA-256 |
|---|---|
| `2013-01-10_vel.tar.gz` | `92118ba5dc8e197eb0dfd817a006b1acc20ff1efb2fa53be02f61d40d6438ccb` |
| `2013-01-10_sen.tar.gz` | `3ec1a5ac27ee716e6e5da0cd4e8fee96241d6a21ac63d14a844c40d2410d5ffb` |
| `groundtruth_2013-01-10.csv` | `4301c74fecc9578353a2f373f50320de97542962240fc314389bf6fdd9063b63` |

`prepare_nclt_inputs.py` produced 5105 PCD frames, 5105 frame timestamps, and
48122 IMU rows under the external data root. The generated reference CSV is
byte-equivalent by SHA-256 to the repository copy:
`e5d7ae51f4f1992823276dd352d6e7c208e9f386da944a3016a2ab5c45d9dc50`.

After preparation, suite readiness is 3/10 and SSD free space is 60.69 GiB.
NCLT remains held out: no candidate or baseline performance was run during
preparation.

### MulRan validation preparation

The official MulRan ParkingLot sample folder was downloaded to the external
SSD. The LiDAR archive and reference trajectory checksums are:

| File | SHA-256 |
|---|---|
| `Ouster.zip` | `b28cd1adbe46848ead7aea01ebf9dcb2ea267c10aa594306d5e890a626d847d2` |
| `global_pose.csv` | `c87a1ef1400b728b9a0d9269c3ca6f452ea369439496c63ae3b0ce6500d5a4aa` |

The current official sample contains 1176 Ouster scans, not the older
1177-frame assumption. `prepare_mulran_inputs.py` produced 1176 PCD frames and
1176 aligned GT poses. The generated reference CSV is byte-equivalent by
SHA-256 to the repository copy:
`cef5ad5ce793d48b69cce20d45e7ee0f78d721f60207aef93b7a48fa2fb2b358`.
The machine-readable suite expectation was corrected to 1176. This is a
validation dataset; preparation did not run candidate or baseline performance.

### GT-leak-free diagnostic baseline

The first 50 frames of corrected KITTI Raw 0009 were run with one first-pose
anchor and `--no-gt-seed`. Results are stored outside Git under
`E:\datasets\loc_zoo\results\sota_bootstrap`.

| Method | Translational RPE [%/100 m] | ATE [m] | FPS |
|---|---:|---:|---:|
| Small-GICP | **2.512** | **0.653** | 10.8 |
| KISS-ICP | 2.616 | 0.664 | 9.6 |
| LeGO-LOAM | 2.771 | 1.016 | 1.8 |
| A-LOAM | 2.939 | 1.831 | 1.5 |
| CT-ICP fast | 3.090 | 1.078 | **11.2** |

These numbers validate the execution path only. They are not SOTA evidence
because this is a short diagnostic window with non-native timestamp units.

### Current external baseline shortlist

- MAD-ICP: uncertainty-aware, open-source, real-time ICP odometry.
- MOLA-LO: adaptive open-source odometry with broad cross-sensor evaluation.
- SiMpLE: minimal-configuration open-source LiDAR odometry.
- Kurda et al. 2025: multiple independent scan-to-scan constraints over a
  moving pose-graph window, evaluated on KITTI and MulRan. No official code was
  located during the initial survey.

The suite registry records the official repository or paper URL for each.

### First implementation hypothesis

Build an **uncertainty-aware multi-horizon LiDAR odometry** candidate:

1. Use the existing KISS-ICP/Small-GICP-style voxelized registration as the
   real-time front end.
2. Register the newest scan independently against several recent key scans,
   rather than only a drifted aggregate map.
3. Convert each registration Hessian/covariance into an information-weighted
   relative-pose constraint.
4. Optimize a small fixed-lag SE(3) window without recomputing correspondences.
5. Reject or downweight constraints along degenerate eigen-directions and feed
   trajectory-shape diagnostics into the failure policy.
6. Marginalize old poses and keep one dataset-independent configuration.

This direction targets the observed gap directly: pairwise ICP is the strongest
available short-window baseline, while the latest NTNU work shows that
pair-level success alone cannot detect accumulated trajectory-shape failure.

### Next gate

Before parameter work, restore at least the development and validation roles:
KITTI 00/05/07 and MulRan ParkingLot. Then freeze Small-GICP, KISS-ICP,
MAD-ICP, and MOLA-LO baseline outputs on those sequences. Held-out 02/08/NCLT
must remain untouched until the candidate configuration is frozen.

## 2026-07-31: capacity-efficient KITTI bootstrap

`pcd_dogfooding` now discovers numeric `.bin` frames alongside the established
`NNNNNNNN/cloud.pcd` layout. KITTI's native records are loaded as
`x,y,z,intensity` float32 values and pass through the same filtering and
voxelization used by every existing method. This avoids roughly doubling each
sequence on the capacity-constrained external SSD.

`fetch_kitti_odometry_sequences.py` selectively reads the official
`data_odometry_velodyne.zip` with HTTP Range requests. Each requested sequence
is streamed once, split using ZIP local headers, checked against the official
per-entry CRC, and atomically promoted from `.part` to `.bin`. Index timestamps
are written to preserve the repository reference CSV's exact frame-ID
association contract.

KITTI Odometry 07 was prepared and verified:

- 1101/1101 native `.bin` frames
- 2,137,363,408 bytes of LiDAR data
- idempotent rerun: `extracted=0, reused=1101`
- strict GT association source: `frame_timestamps.csv`

A three-frame native-KITTI smoke run completed with KISS-ICP,
`--no-gt-seed`, and strict exact-frame association. It achieved 9.9 FPS; this
short run validates the loader and is not a performance result. Suite readiness
is now 5/10, and both validation datasets (KITTI 07 and MulRan ParkingLot) are
ready. Selective acquisition of development sequences 00 and 05 was started
after this check; their readiness must be verified only after all expected
frames are present.

### KITTI 07 frozen validation baselines

The native-bin path was then used for full-sequence, exact-frame,
GT-seed-free baselines. All rows used one first-pose anchor:

| Method / profile | RPE trans. [%] | RPE rot. [deg/m] | ATE [m] | FPS |
|---|---:|---:|---:|---:|
| MAD-ICP 0.0.10, official 16 keyframes / 16 cores | **0.571** | 0.00653 | **1.275** | 3.88 end-to-end |
| KISS-ICP default | 0.610 | **0.00617** | 1.799 | 6.67 |
| KISS-ICP fast | 0.671 | 0.00776 | 2.525 | 9.94 |
| CT-ICP fast | 2.295 | 0.03652 | 10.459 | **11.41** |
| Small-GICP default | 4.332 | 0.01255 | 10.851 | 9.62 |

MAD-ICP received only the LiDAR directory and its official `kitti`/`default`
configurations. Its camera/base-frame output was converted back to the sensor
frame using the same official `lidar_to_base` conjugation before metric-only GT
evaluation. It tracked all 1101 frames. The Python 0.0.10 source distribution
required `scikit-build-core==0.9.10` because its build metadata still uses
`cmake.verbose`, and `click==8.1.8` to remain compatible with its pinned Typer
CLI. These are packaging compatibility pins, not algorithm changes.

The validation result sharpens the implementation target: preserve or improve
the KISS default trajectory (within 0.039 percentage points of MAD-ICP), reduce
runtime by roughly one third to exceed 10 FPS, and use sparse independent
multi-horizon constraints to improve rather than merely densify the local map.
No held-out performance was inspected.

### Development data completion

The selective native-bin fetch subsequently completed and verified all three
allowed KITTI sequences:

| Sequence | Role | Frames | Native LiDAR bytes |
|---|---|---:|---:|
| 00 | development | 4541 | 8.22 GiB |
| 05 | development | 2761 | 5.14 GiB |
| 07 | validation | 1101 | 1.99 GiB |

Idempotent reruns for 00 and 05 reused all 4541 and 2761 frames respectively.
The official 586 KiB calibration archive was also stored on the external SSD;
only `calib.txt` and `times.txt` were copied into the three allowed sequence
directories. No KITTI pose file is present in the dataset tree. Suite readiness
is 7/10. KITTI 02/08 and NTNU tunnel remain deliberately absent.

### KISS front-end optimization and rejected hypotheses

The KISS correspondence search is now parallelized per point with OpenMP while
retaining the original serial normal-equation accumulation order. With four
threads, full KITTI 07 output and metrics remained identical:

- fast profile: 9.94 to 11.39 FPS;
- default profile: 6.67 to 7.04 FPS.

An opt-in policy that replaces points in full voxels was rejected after
development RPE worsened from 1.011% to 1.707%. It remains disabled by default.
A 500-frame KITTI 00 front-end sweep likewise left the fast profile on the
accuracy/speed Pareto frontier; none of the map resolution, point budget, or
source-voxel variants beat its 1.01% RPE at 10.88 FPS.

The first sparse multi-horizon implementation adds independent 5-frame submap
constraints at 10-frame keyframes, Hessian conditioning and consensus gates,
and fixed-lag correction interpolation. It is present as
`kiss_multi_horizon`, but correction gain defaults to zero. On KITTI 00
development, gain 0.05 was essentially neutral (1.013% versus 1.011% RPE);
larger gains worsened RPE despite a small ATE reduction. Pairwise KISS and GICP
constraints also failed the development/validation transfer check. These
negative results rule out blindly adding sparse edges; the next candidate must
improve uncertainty modeling or optimize the window jointly.

### Reproducible external baselines

`run_mad_icp_baseline.py` runs the official MAD-ICP process before opening the
reference CSV, then records commands, dependency versions, hashes, wall time,
algorithm time, and common-protocol metrics. Full development runs use the
same official 16-keyframe/16-core configuration as the frozen KITTI 07 row.

MOLA-LO 2.2.1 and its 3.0.0 academic-dataset parser were installed from the ROS
2 Humble build farm. `run_mola_baseline.py` similarly exposes only the KITTI
scan/calibration root to the child process, reads GT after process exit, accepts
the official GICP and simple-state-estimator YAML files explicitly, and records
their hashes. TUM parsing and the GT-isolation command boundary are unit tested.
MOLA execution is serialized after MAD-ICP so baseline timing is not distorted
by CPU contention.

The official MAD-ICP run on full KITTI 00 completed without tracking loss:

| Method / sequence | Frames | RPE trans. [%] | RPE rot. [deg/m] | ATE [m] | Algorithm FPS | End-to-end FPS |
|---|---:|---:|---:|---:|---:|---:|
| MAD-ICP official16 / KITTI 00 | 4541 | 0.852 | 0.00640 | 9.631 | 6.06 | 4.17 |

Wall time was 1088.44 s. The manifest contains 4541 per-frame odometry timing
samples and hashes the estimate and reference. A GT-absent MOLA GICP smoke test
then tracked 50/50 frames; its log explicitly reports that the conventional
`dataset/poses/00.txt` file was not found. A 200-frame development run produced
1.121% translational RPE, 0.01743 deg/m rotational RPE, and 1.981 m ATE. Its
short-run end-to-end rate was 4.36 FPS because 8 s of full-sequence file
enumeration is included, while MOLA's internal `onLidar` timing averaged about
89 ms/frame. This validates the adapter, but does not yet make MOLA the
strongest development baseline.

The same official MAD-ICP configuration was then run on KITTI 05:

| Method / sequence | Frames | RPE trans. [%] | RPE rot. [deg/m] | ATE [m] | Algorithm FPS | End-to-end FPS |
|---|---:|---:|---:|---:|---:|---:|
| MAD-ICP official16 / KITTI 05 | 2761 | 0.468 | 0.00529 | 4.774 | 6.22 | 4.29 |

### Model-deviation threshold and KITTI elevation calibration

The local KISS front end previously estimated its adaptive scale from recent
vehicle translation. An opt-in implementation now follows the upstream KISS
model-deviation definition instead: the deviation between the constant-velocity
prediction and registered pose combines translational displacement with
rotation-induced displacement at maximum range. The cumulative RMS scale and
correspondence multiplier are independently exposed. The legacy policy remains
the default.

Development sweeps selected a compact front end with a 0.8 m source voxel,
2000 source points, 1.2 m map voxels, six points per voxel, 15 ICP iterations,
a 45 m local radius, model-deviation thresholding, and a 2.0 correspondence
multiplier. Before sensor correction this produced 0.841% RPE on KITTI 00 and
0.623% on KITTI 05. Error decomposition on 05 showed better forward and lateral
components than MAD-ICP but a systematic negative vertical component.

The voxel sizes are explicit CLI overrides. An archival manifest briefly
listed the later fast-profile defaults (0.75/1.25 m), but every evaluated run
log records 0.8/1.2 m; the manifests were corrected without changing results
or historical source hashes.

KITTI's known +0.205 degree Velodyne elevation correction was therefore added
as an explicit, default-off preprocessing option. It preserves each point's
range and does not use trajectory GT. A paired 500-frame KITTI 05 ablation
changed translational RPE from 0.701% to 0.385% and ATE from 1.283 m to
1.265 m. The correction value was not tuned.

The unchanged configuration then completed all three allowed KITTI
development/validation sequences:

| Candidate / sequence | Frames | RPE trans. [%] | RPE rot. [deg/m] | ATE [m] | Algorithm FPS | I/O-included FPS |
|---|---:|---:|---:|---:|---:|---:|
| modeldev-p6-elevation / KITTI 00 | 4541 | 0.785 | 0.00693 | 15.472 | 108.57 | 8.32 |
| modeldev-p6-elevation / KITTI 05 | 2761 | 0.414 | 0.00424 | 7.881 | 101.64 | 7.65 |
| modeldev-p6-elevation / KITTI 07 | 1101 | 0.501 | 0.00598 | 2.177 | 119.24 | 7.97 |

Its geometric-mean translational RPE is 0.546%, versus 0.611% for official
MAD-ICP on the same three sequences, a 10.6% relative improvement. It beats
MAD-ICP's translational RPE on every row and is faster by the algorithm timers.
MAD-ICP retains lower ATE on every full sequence, so the candidate is not
uniformly better.

The candidate is recorded in
`evaluation/data/lidar_odometry_candidate_kiss_modeldev_p6_elevation.json`,
but is not frozen and no held-out sequence has been evaluated. Timing now
separates `registerFrame()` algorithm time from external-SSD file I/O. With
four OpenMP threads, algorithm FPS clears the 10 FPS gate on all three
sequences; I/O-included FPS is reported separately and is limited by scan
loading. The next permitted work is unchanged-configuration MulRan validation
before any held-out run. That validation exposed an evaluator-frame error,
described below; no held-out result was opened.

### MulRan evaluation-frame correction

The first MulRan evaluation incorrectly treated `global_pose.csv` as a LiDAR
pose. MulRan stores a base/rear-wheel pose, so comparing it directly with a
LiDAR odometry trajectory leaves the fixed base-to-Ouster lever arm and
rotation on only one side of every relative motion. The resulting 72--77 m ATE
and roughly 103% RPE values were evaluator artifacts, not estimator failures.
All repository MulRan claims produced with that old reference are invalid.

The GT converter now composes the published base-to-Ouster calibration before
writing `lidar_pose.*`; the corrected reference SHA-256 is
`9495e7cdeb4a95b58f823cb321440aaff99f3a0504f83afa4549e0f6ca8c1143`.
Pose-I/O and synthetic conversion tests cover the transform. Re-evaluating the
already-finished MAD-ICP estimate did not rerun odometry or expose GT to it.

| Method / corrected MulRan ParkingLot | Frames | RPE trans. [%] | RPE rot. [deg/m] | ATE [m] | Algorithm FPS | End-to-end FPS |
|---|---:|---:|---:|---:|---:|---:|
| modeldev-p6, no deskew | 1175 | 1.621 | 0.02733 | 4.335 | 77.87 | 25.80 |
| modeldev-p6, official-format deskew | 1175 | 1.458 | 0.02546 | 4.262 | 60.86 | 18.83 |
| MAD-ICP 0.0.10 official16 | 1175 | 1.439 | 0.02567 | 4.331 | 3.17 | n/a |

The candidate's no-deskew estimate is already valid after the reference fix.
The remaining gap motivated a GT-independent deskew implementation matching
the [upstream KISS-ICP MulRan loader](https://github.com/PRBonn/kiss-icp):
raw Ouster scans contain 64 beams by 1024 azimuth columns, so
`floor(raw_point_index / 64) / 1024` supplies normalized per-point time.
Constant-velocity SE(3) interpolation uses only the previous registered pose
delta and compensates every point to scan end before source voxelization.
The final empty scan falls back safely; all 1175 nonempty scans deskewed.

Deskew improves translational RPE by 10.0% and ATE by 1.7% over the corrected
no-deskew candidate. Against MAD-ICP, it has 1.36% higher translational RPE and
slightly lower rotation error, plus 1.60% lower ATE and 19.21x higher algorithm
FPS. It is therefore a cross-dataset Pareto-frontier candidate, not a uniform
accuracy winner.

One GT-independent map-update ablation replaced points in full voxels. It
improved the first 600 frames but regressed full-sequence RPE to 1.722%, so it
was rejected. The candidate remains unfrozen until default-off KITTI
non-regression and provenance audits pass. Held-out evaluation remains
prohibited until that freeze.

### Frozen held-out evaluation

After the MulRan provenance audit, focused tests, and an exact KITTI 05
500-frame non-regression check passed, the candidate was frozen. KITTI 02,
KITTI 08, and NCLT 2013-01-10 had not previously been evaluated with the
candidate. Each full sequence was then opened exactly once; no parameter or
implementation change followed.

| Frozen candidate / held-out | Frames | RPE trans. [%] | RPE rot. [deg/m] | ATE [m] | Algorithm FPS | I/O-included FPS |
|---|---:|---:|---:|---:|---:|---:|
| KITTI 02 | 4661 | 0.769 | 0.00542 | 37.232 | 52.10 | 4.70 |
| KITTI 08 | 4071 | 1.354 | 0.00550 | 17.135 | 83.56 | 6.76 |
| NCLT 2013-01-10 full | 5105 | 2.374 | 0.02928 | 6.836 | 59.21 | 4.92 |

Official MAD-ICP 0.0.10 official16 was then run once on KITTI 02/08 with its
odometry process finishing before GT was opened. The frozen default KISS
configuration supplied the NCLT comparator because the MAD runner does not
have a validated NCLT extrinsic conversion.

| Held-out comparator | Candidate RPE [%] | Baseline RPE [%] | Candidate ATE [m] | Baseline ATE [m] | Candidate / baseline algorithm FPS |
|---|---:|---:|---:|---:|---:|
| KITTI 02 / MAD-ICP | 0.769 | 0.806 | 37.232 | 20.572 | 52.10 / 6.34 |
| KITTI 08 / MAD-ICP | 1.354 | 1.411 | 17.135 | 15.671 | 83.56 / 3.85 |
| NCLT / default KISS | 2.374 | 15.134 | 6.836 | 60.629 | 59.21 / 5.20 |

The candidate wins translational RPE on all three rows. Its KITTI held-out
geometric-mean RPE is 1.020%, versus 1.066% for MAD-ICP, a 4.34% relative
improvement. Across the explicit per-sequence comparator set, geometric-mean
RPE is 1.352% versus 2.582%, a 47.64% improvement. This mixed comparator is
reported transparently and is not presented as one universal baseline.
MAD-ICP retains lower ATE on both held-out KITTI sequences.

The GT-backed promotion gate passes. Full promotion remains conditional on a
GT-free health run over the already-prepared NTNU fog sequence; NTNU tunnel is
still absent and should be restored only when external-SSD capacity permits.

### NTNU fog robustness failure

The frozen candidate was then run once on all 1729 prepared NTNU fog frames.
This dataset has no ranking-quality GT. The evaluator's required reference
argument was an explicitly synthetic identity trajectory used only to anchor
the first pose; all ATE/RPE values against it are invalid and excluded.
Health is computed solely from finite estimates and inter-frame motion.
Because the PCDs contain a real per-point `time` field, the frozen
timestamp-aware deskew policy was applied to all frames.

| GT-free health / NTNU fog full | Value |
|---|---:|
| Finite poses | 1729 / 1729 |
| Estimated path / net displacement | 14228.2 m / 19.3 m |
| Median / p95 / max translation step | 1.22 / 29.20 / 31.50 m |
| Translation steps above 2 m | 631 / 1728 |
| Median / p95 / max rotation step | 117.74 / 179.82 / 179.99 deg |
| Rotation steps above 20 deg | 1192 / 1728 |
| Algorithm / I/O-included FPS | 79.54 / 12.31 |

This is a catastrophic inter-frame-motion failure despite every pose remaining
finite. The candidate is therefore **not promoted**, notwithstanding its
GT-backed held-out RPE wins. No deskew-off rerun or post-held-out retuning was
performed. The next generation must be a new candidate with an online,
GT-free step-consistency guard and must use fresh GT-backed held-out sequences
for any later promotion claim.

### Motion-guard candidate frozen

A new candidate adds only an online, GT-free motion-consistency guard to the
failed frozen configuration. A proposed update above 2.0 m translation or
20 degrees rotation is rejected: the last trusted pose is held, the velocity
prediction is reset, and threshold and map updates are skipped. The guard made
zero rejections and reproduced the previous metrics exactly on KITTI 00/05/07
and corrected MulRan ParkingLot.

On the 1729-frame NTNU fog health run it rejected 49 proposed updates. All
poses remained finite, estimated path length fell from 14228.2 m to 383.8 m,
and the maximum accepted translation/rotation steps were 1.373 m and
19.994 degrees, with no health-threshold violations. This is robustness
evidence only; NTNU fog has no ranking-quality GT.

The implementation and configuration are frozen in
`evaluation/data/lidar_odometry_candidate_kiss_modeldev_guard_v1.json`.
Before downloading or inspecting their GT, KITTI Odometry 06 and 09 were
declared as the fresh held-out set. Each candidate sequence may be run once,
with no post-result tuning. Promotion requires finite trajectories, at least
10 algorithm FPS on both sequences, and candidate geometric-mean
translational RPE no worse than MAD-ICP official16 under the same evaluator.

### Motion-guard fresh held-out promotion

Both frozen candidate runs completed once with finite trajectories and zero
guard rejections. MAD-ICP 0.0.10 official16 was evaluated afterward with GT
opened only after its odometry process exited.

| Fresh held-out | Candidate RPE [%] | MAD-ICP RPE [%] | Candidate / MAD ATE [m] | Candidate / MAD algorithm FPS |
|---|---:|---:|---:|---:|
| KITTI 06 | **0.345** | 0.494 | 4.024 / **1.943** | **50.42** / 2.87 |
| KITTI 09 | **0.657** | 0.733 | 6.438 / **5.365** | **51.16** / 3.63 |

The candidate geometric-mean translational RPE is 0.476%, versus 0.602% for
MAD-ICP, a 20.88% relative improvement. It passes the primary accuracy and
runtime gates on both rows and is promoted. MAD-ICP retains lower unaligned
ATE on both sequences, so this is not a uniform metric win. No parameter or
implementation change followed the held-out results.

### Conservative Scan Context pose-graph experiment

A GT-free pose-graph backend was added around the promoted KISS front end.
Every tenth scan becomes a keyframe. Scan Context proposes loop candidates,
local five-keyframe submaps are registered by GICP, and only converged loops
with at least 80 correspondences and at most 1.0 m Euclidean RMSE are accepted.
The graph correction is applied with a fixed gain of 0.05. Ground truth is
never used for detection, registration, gating, or optimization.

The fixed configuration met its KITTI development guard:

| Dataset | Raw KISS ATE / RPE | Pose graph ATE / RPE | Accepted loops |
|---|---:|---:|---:|
| KITTI 05 | 7.808 m / 0.412% | 7.427 m / 0.415% | 4 |
| KITTI 07 | 2.376 m / 0.510% | 2.259 m / 0.515% | 1 |
| MulRan ParkingLot | 4.252 m / 1.464% | 4.252 m / 1.462% | 4 |

This is roughly a 4.9% ATE improvement on both KITTI sequences while holding
the RPE regression below 1%, and it is neutral on MulRan. A fresh, predeclared
KITTI 04 run was then prepared entirely on the external SSD from the official
271-frame release. No loop candidate was found, so the backend reproduced raw
KISS exactly: 1.235650 m ATE, 0.353133% translational RPE, and 0.003150 deg/m
rotational RPE. Its isolated algorithm rate was 12.0878 FPS.

Baselines were run only after the frozen candidate on KITTI 04:

| Method | ATE [m] | RPE trans. [%] | RPE rot. [deg/m] | Reported FPS |
|---|---:|---:|---:|---:|
| KISS pose graph gain 0.05 | **1.236** | **0.353** | 0.00315 | **12.09 algorithm** |
| Raw KISS front end | **1.236** | **0.353** | 0.00315 | 11.74 algorithm |
| MOLA-LO 2.2.1 GICP | 1.558 | 0.479 | **0.00248** | 3.15 end-to-end |
| MAD-ICP 0.0.10 official16 | 2.144 | 0.604 | 0.00378 | 3.24 algorithm |
| CT-ICP fast | 47.886 | 7.141 | 0.04927 | 6.73 end-to-end |

The same fixed candidate does not transfer cleanly to NCLT. One accepted loop
improved translational RPE from 2.374% to 2.300%, but unaligned ATE worsened
from 6.836 m to 7.887 m (15.4%). Algorithm speed remained above the gate at
14.66 FPS. The generation is therefore recorded as
`experimental_rejected_cross_dataset_ate_regression`, not promoted. Its
machine-readable record is
`evaluation/data/lidar_odometry_candidate_kiss_pose_graph_gain005_v1.json`.
Any next pose-graph generation must add a GT-free correction-consistency gate
and use a different fresh held-out sequence; KITTI 04 may not be reused for
promotion after further tuning.

### Two-cluster correction gate and KITTI 10 held-out result

The next frozen generation requires two independent accepted loop clusters
before publishing any pose-graph correction. A single cluster may still be
optimized internally, but the externally visible trajectory stays identical
to its raw odometry input. This removes the additional NCLT regression caused
by the one-loop correction: the current rebuilt front end remains at 7.884 m
ATE and 2.300% RPE rather than being changed by that loop. This is not a claim
that the historical promoted binary was reproduced; that older artifact had
6.836 m ATE and 2.374% RPE on NCLT.

KITTI Odometry 10 was declared before its scans or GT were acquired, then the
frozen candidate was run exactly once. It found no loop candidate, correctly
left correction disabled, produced finite poses, and passed the 10 FPS gate.
All baselines were run afterward with the same unaligned ATE and 100 m KITTI
segment metrics. Their odometry subprocesses had no access to GT; GT was
opened only for evaluation after each process exited.

| KITTI 10 full (1201 frames) | ATE [m] | RPE trans. [%] | RPE rot. [deg/m] | Reported FPS |
|---|---:|---:|---:|---:|
| Official KISS-ICP 1.3.0 default | **2.881** | **0.686** | **0.00360** | **33.37 algorithm** |
| MAD-ICP 0.0.10 official16 | 5.911 | 0.797 | 0.00568 | 5.91 algorithm |
| Frozen cluster2 candidate | 6.380 | 0.812 | 0.00636 | 28.03 algorithm |
| MOLA-LO 2.2.1 GICP | 11.430 | 0.801 | 0.00549 | 4.37 end-to-end |
| CT-ICP fast | 38.198 | 3.702 | 0.07033 | 9.59 end-to-end |

The cluster gate passes its safety and runtime claims, but the generation is
**not promoted**. Official KISS-ICP reduces ATE by 54.85% and translational RPE
by 15.46% relative to the candidate on this fresh sequence; MAD-ICP is also
slightly better on both accuracy measures. The strongest next direction is
therefore to use the official KISS trajectory as the front end and evaluate
GT-free loop correction separately, rather than tuning the weaker rebuilt
front end against KITTI 10. The evaluator accepts an external KITTI-format
trajectory for this purpose; the already observed KITTI 10 result is
development evidence only for every subsequent generation.

### Dual-KISS causal rotation consensus

The official KISS result exposed a useful complementarity: its low-frequency
orientation is substantially better for ATE on several KITTI sequences, while
the promoted Zoo front end can retain lower local translational RPE. A causal
fusion therefore tracks only the left-multiplicative rotation disagreement
with gain 0.0002 per frame. It rotates the primary trajectory but never copies
the official trajectory's translation. Output frame i uses only inputs 0..i.

The gain was selected on KITTI 07 and locked before KITTI 05 transfer. The
primary trajectory is the two-cluster pose-graph output. KITTI 10 had already
been observed by the preceding generation, so its result below is transfer
evidence rather than fresh held-out evidence.

| Sequence | Primary ATE / RPE | Rotation-consensus ATE / RPE | Outcome |
|---|---:|---:|---|
| KITTI 07 development | 2.376 m / 0.5104% | **2.308 m / 0.5046%** | both improve |
| KITTI 05 locked-gain transfer | 7.427 m / **0.4148%** | **6.924 m** / 0.4222% | ATE -6.77%, RPE +1.79% |
| KITTI 10 previously observed | 6.380 m / 0.8120% | **4.808 m / 0.7991%** | both improve |
| MulRan ParkingLot transfer | **4.252 m** / 1.4617% | 4.254 m / **1.4598%** | accuracy-neutral |

Across KITTI 05/07, geometric-mean ATE improves by 4.83% while geometric-mean
translational RPE changes by +0.32%, inside the fixed 1% preservation guard.
This generation is frozen in
`evaluation/data/lidar_odometry_candidate_dual_kiss_rotation_consensus_v3.json`.
It is not promoted at this stage: concurrent dual-front-end runtime, NCLT
transfer, and a new predeclared KITTI 03 held-out run remain as gates. The
official PCD reference ran at only 4.11 algorithm FPS on full-resolution
MulRan, so cross-dataset runtime also needs optimization. Official KISS is now
runnable directly on Zoo PCD sequences without a GT argument, including
per-point timestamp deskew when the PCD provides a `time` field.

### Rotation-consensus v3 fresh held-out rejection

KITTI 03 was downloaded only after v3 and gain 0.0002 were frozen. Both front
ends ran concurrently against the 801 official scans with no GT present: the
Zoo component received a synthetic identity reference only to satisfy the
evaluator interface, and the official KISS subprocess received no reference
argument. Their outputs were fused and hashed before the official pose and
calibration archives were downloaded. GT SHA-256 is
`f1ab14759dad481b763374be07d20337785c34a6b68d21a81a2f886b681080ab`.

| KITTI 03 fresh held-out | ATE [m] | RPE trans. [%] | RPE rot. [deg/m] | Concurrent algorithm FPS |
|---|---:|---:|---:|---:|
| Official KISS-ICP 1.3.0 | **2.328** | **0.8433** | **0.00224** | 7.18 |
| Cluster2 primary | 6.210 | **0.8673** | 0.00500 | **19.42** |
| Dual-KISS rotation consensus v3 | **5.703** | 0.8841 | **0.00487** | bounded by 7.18 |

The candidate improves ATE over its primary by 8.17%, but translational RPE
regresses by 1.93%, exceeding the predeclared 1% guard. The official component
also falls below the 10 FPS concurrent-load gate. v3 is therefore **not
promoted**, and no post-held-out gain change is allowed. A next generation must
reduce the reference compute cost and add an online confidence gate rather
than tuning gain 0.0002 against KITTI 03.

### Deadbanded rotation consensus v4 frozen

KITTI 03 is now development data. The next generation adds a GT-free 1.25
degree absolute orientation-disagreement deadband: below it, the current
correction is held rather than chasing small differences between front ends.
The gain remains 0.0002. Official KISS registration is capped at four threads;
on KITTI 03 this reproduced the default accuracy exactly and increased its
isolated algorithm rate from the contention-affected 7.18 FPS to 15.04 FPS.

Across KITTI 03/05/07, v4 improves geometric-mean ATE by 2.83% with a 0.54%
geometric-mean RPE regression, inside the 1% development guard. It also changes
KITTI 10 from 6.380 m / 0.8120% to 5.051 m / 0.8112%. The configuration is
frozen in
`evaluation/data/lidar_odometry_candidate_dual_kiss_deadband_v4.json` before
acquiring its new fresh held-out sequence, KITTI 01. Concurrent four-thread
runtime and the KITTI 01 accuracy gate remain unproven; v4 is not promoted.

### Deadband v4 fresh held-out rejection and causal failover

KITTI 01 was acquired after v4 was frozen. The two four-thread front ends ran
concurrently without GT, and the fused estimate was written before the stored
GT was opened. Both components passed the algorithm-rate gate (47.75 FPS for
the primary and 14.73 FPS for official KISS), although concurrent end-to-end
throughput including duplicate scan I/O was 7.67 FPS.

| KITTI 01 fresh held-out | ATE [m] | RPE trans. [%] | RPE rot. [deg/m] |
|---|---:|---:|---:|
| Official KISS-ICP 1.3.0, 4 threads | **8.764** | **0.831** | **0.00139** |
| Cluster2 primary | 1049.502 | 56.540 | 0.02922 |
| Deadband consensus v4 | 1027.997 | 58.236 | 0.02895 |

The highway sequence triggers 59 primary motion-guard rejections and exposes a
catastrophic front-end failure. v4 improves ATE by only 2.05% and worsens RPE
by 3.00%, so it is **not promoted**. This failure is observable without GT:
the rejected update holds the primary pose while official KISS continues to
move.

A causal emergency failover was therefore added for the next generation. On
the first exact primary hold with at least 0.05 m reference motion, it publishes
the current fused pose, fixes a primary-to-reference alignment, and follows the
aligned official trajectory from the next frame onward. KITTI 01 triggers at
frame 91 and improves to 4.855 m ATE, 0.931% translational RPE, and 0.00160
deg/m rotational RPE. It does not trigger on KITTI 03/05/07/10. This is v5
development evidence, not a fresh held-out or promotion result.

The same frozen threshold was replayed on all 1175 usable MulRan ParkingLot
frames. No failover was triggered, and the result remained 4.252 m ATE,
1.462% translational RPE, and 0.02569 deg/m rotational RPE. Together with the
four non-triggering KITTI sequences, this clears the current false-trigger
development check. A different, unconsumed public sequence must still be
declared before v5 can provide fresh held-out evidence.

### Causal failover v5 fresh held-out rejection

After the false-trigger checks, v5 was frozen before downloading KITTI Raw
`2011_09_26_drive_0001_sync`. Only the 108 Velodyne scans were initially
extracted. The scan-only dataset contained no OXTS directory or KITTI pose
file. Both front ends and the causal fused estimate completed before OXTS and
calibration were extracted. The fused estimate was SHA-256 fixed as
`eb8dc1b794c1eb05d2d0d754b59394e28b0a58c57f14e136d49f393c1b93f3ad`.

GT was then generated in the initial Velodyne frame with the KITTI Mercator
projection and calibrated IMU-to-Velodyne transform. The 106.92 m trajectory
provides six 100 m RPE segments.

| KITTI Raw 0001 fresh held-out | ATE [m] | RPE trans. [%] | RPE rot. [deg/m] | Algorithm FPS |
|---|---:|---:|---:|---:|
| Cluster2 primary | **0.260** | **0.397** | 0.00346 | **23.88** |
| Official KISS-ICP 1.3.0, 4 threads | 1.208 | 0.500 | **0.00236** | 9.64 |
| Causal failover v5 | **0.260** | **0.397** | 0.00346 | dual-front-end gate fails |

No primary hold or orientation correction crossed the frozen thresholds, so
v5 exactly reproduced the primary. Tracking and RPE preservation pass, but
the predeclared ATE-improvement gate fails. The official reference also falls
below the 10 algorithm-FPS threshold. v5 is therefore **not promoted**, and
no post-held-out threshold tuning is allowed.

### Adaptive motion guard v6 development

The KITTI 01 failure is caused by the promoted guard's fixed 2 m translation
limit, not by an unobservable global drift. v6 returns to the promoted KISS
front end and adds a causal exception only after the fixed guard rejects. A
candidate step must remain below 4 m and 20 degrees, have at least three prior
trusted steps, and differ from the previous trusted SE(3) motion by at most
0.75 m and 5 degrees. The original rejection and map-update policy is otherwise
unchanged.

On the first 200 KITTI 01 frames, v6 accepts 105 high-speed steps through the
adaptive branch with zero rejections. It reaches 1.901 m ATE, 0.699%
translational RPE, 0.00475 deg/m rotational RPE, and 24.79 algorithm FPS.

The full 1101-frame run accepts 755 steps through the adaptive branch and has
no rejection. Relative to the fixed guard, ATE falls from 1049.502 to 50.764 m
and translational RPE from 56.540% to 0.633%; algorithm throughput is 15.54
FPS. On ordinary motion, the branch remains inactive: KITTI 07 and KITTI Raw
0001 are byte-identical to same-binary fixed controls, while KITTI 05 has zero
adaptive acceptances and zero rejections. Thus v6 clears its development
recovery and normal-motion non-regression gates. These are consumed sequences,
so they are not fresh promotion evidence.

### Causal incremental-rotation fusion v7 development

To reduce the remaining KITTI 01 accumulated heading drift, v7 preserves every
v6 local-frame translation and blends only the two front ends' relative
rotations on SO(3). It then re-integrates the increments online. Reference
translation is never consumed, and output pose `i` uses inputs only through
`i`. A 5 degree incremental-disagreement gate falls back to the primary.

Five blend values were explored only on consumed KITTI 01 development data.
Blend 0.05 was selected because it gives a useful ATE reduction while keeping
the primary translational-RPE regression below 1%, then locked for transfer.

| Locked v7 transfer | v6 ATE [m] | v7 ATE [m] | ATE change | v6 RPE trans. [%] | v7 RPE trans. [%] |
|---|---:|---:|---:|---:|---:|
| KITTI 01 (selection) | 50.764 | 47.977 | -5.49% | 0.6327 | 0.6361 |
| KITTI 05 | 7.808 | 7.531 | -3.54% | 0.4125 | 0.4114 |
| KITTI 07 | 2.376 | 2.257 | -5.02% | 0.5104 | 0.5066 |
| MulRan ParkingLot | 4.25195 | 4.25246 | +0.012% | 1.4636 | 1.4619 |

No disagreement gate fired in these runs. The fixed setting therefore improves
ATE and RPE on both untouched KITTI development transfers; MulRan is effectively
ATE-neutral and improves RPE slightly, remaining well inside the 1% transfer
guard.

The original uncapped official KISS MulRan reference ran at 4.11 algorithm
FPS. Choosing only by GT-free timing on the first 200 frames, an eight-thread
cap reached 27.16 FPS. The same cap on all 1175 usable scans reached 10.53
algorithm FPS and 9.97 end-to-end FPS. Parallel algorithm throughput therefore
passes the 10 FPS gate, but end-to-end throughput misses it by 0.29% and is too
close to call stable.

The unchanged v7 setting was then transferred to all 5105 NCLT 2013-01-10
frames. One increment crossed the 5 degree disagreement gate. ATE improves
from 7.8841 to 7.8716 m, translational RPE from 2.3003% to 2.2780%, and
rotational RPE from 0.02889 to 0.02819 deg/m. The primary and official
reference run at 51.45 and 14.04 algorithm FPS, respectively. The Python
official runner reaches only 4.32 FPS including its offline nested-PCD parser;
this I/O/conversion diagnostic is recorded separately from the established
algorithm-rate promotion gate.

With locked transfers passing on KITTI 05/07, MulRan, and NCLT, v7 is frozen.
Before any download or scan/GT inspection, KITTI Raw
`2011_09_26_drive_0005_sync` is declared as the sole fresh held-out sequence.
The candidate may run once after scan-only extraction. OXTS and calibration
must remain unavailable until both component trajectories and the fused output
are generated and hashed. Passing requires at least 1% ATE improvement over
v6, no more than 1% translational-RPE regression, finite poses, and at least
10 algorithm FPS for both front ends. v7 remains unpromoted pending that run.

### Incremental-rotation v7 fresh held-out rejection

The official 616 MiB KITTI Raw drive 0005 archive was downloaded to the
external SSD and SHA-256 fixed as
`eaa3635b6ad6a707a8bab53ca38567ada8c78eb743e96d558497872fa436c136`.
Only its 154 Velodyne scans were CRC-checked and extracted initially. The v6
primary, official KISS eight-thread reference, and frozen v7 fusion ran against
a synthetic identity file whose metrics are invalid by construction. Their
trajectories were fixed before OXTS or calibration extraction:

- v6 primary: `401c7d7a479ce70f91ba9aebec0850674b244e4d617b92e7f214df07d362f4ae`
- official KISS: `d1dadb3b0238ec2a8aef04f3a723f9971081103647d934d49b68849f270b265e`
- v7 fused: `d7f3dbd2f2a0deafbbf566a151b8f564e1068ca8d9905886f8d50cbe32726e3a`

The subsequently generated initial-Velodyne GT has SHA-256
`9fd29880807bd459011340b2ceb1f4eed5ffc2064ec7aa06730e02f5fef08fec`.
The route is only 69.38 m, so it cannot produce a 100 m RPE segment. v6 ATE is
1.52052 m, official KISS ATE is 1.42892 m, and v7 ATE is 1.51877 m. Both
front ends pass the algorithm-rate gate at 27.36 and 11.44 FPS, but v7 improves
ATE by only 0.115%, below the frozen 1% minimum. v7 is therefore **not
promoted**, and the blend may not be retuned against this held-out result.

### Speed/disagreement adaptive v8 development

Raw 0005 is development data only after rejecting v7. v8 explores a materially
new causal policy: retain the v7 blend of 0.05 at high speed or uncertain
increments, but allow stronger rotation fusion only for primary steps at most
1 m whose two front ends disagree by a very small angle. A cumulative causal
rate limiter bounds how often strong corrections can be published. Reference
translation remains unused.

NCLT's incremental disagreement is separable in aggregate (0.228 degree
median) from KITTI/Raw/MulRan (roughly 0.05--0.07 degree). A 0.1 degree strong
threshold without rate limiting produces promising ATE changes on Raw 0005
(-1.57%), KITTI 01 (-3.21%), and NCLT (-38.05%), but it regresses translational
RPE by 16.66% on KITTI 05 and 11.69% on KITTI 07. Cumulative strong-correction
fractions 0.05, 0.10, 0.15, and 0.20 were also evaluated. The conservative
0.05 case keeps KITTI 07 close and improves MulRan/NCLT, but KITTI 05 still
regresses 3.36% RPE and 8.79% ATE.

The final mechanism clamps the published orientation to at most 0.1 degree
from the current primary orientation. The selected fully causal configuration
therefore uses a 0.1 degree strong-candidate threshold, a 5% cumulative strong
rate, a 0.1 degree cumulative correction cap, and the original 5 degree hard
gate. It produces:

| Frozen v8 development | ATE change vs v6 | RPE trans. change vs v6 |
|---|---:|---:|
| KITTI 01 | -3.075% | -0.097% |
| KITTI 05 | -2.227% | -0.112% |
| KITTI 07 | -0.437% | +0.095% |
| KITTI Raw 0005 | -0.266% | no 100 m segment |
| MulRan ParkingLot | -0.301% | -1.361% |
| NCLT 2013-01-10 | +0.025% | +0.153% |

All measured RPE rows remain inside the 1% guard. Five of six ATE rows improve;
the 0.025% NCLT change is treated as numerical-scale nonregression. v8 is
frozen on 2026-08-01. Before downloading or inspecting it, KITTI Raw
`2011_09_26_drive_0019_sync` is declared as the sole fresh held-out sequence.
The scan-only trajectory and hashes must again be fixed before OXTS or
calibration extraction, and no post-result tuning is permitted.

### Direction-consistent rotation v10 fresh held-out promotion

The official drive 0023 synchronized archive is 1,999,129,806 bytes with
SHA-256 `a9436a3da5199821712b0341dbabc9b88d3f6fa62bde38935734577c9f3a9d97`.
All 479 Velodyne members passed CRC checks, and only 474 scans were initially
extracted. OXTS and calibration remained absent while component and fused
trajectories were generated and fixed as:

- v6 primary: `e0811beba946d978edbb00de3c6b67484ed8d104f3d01efc95f1d8ba54143144`
- load-adaptive official reference: `f9fabff7103a091f6bc192b9e3fb93ee25b5185598361d22e23d011aa8c1e2ef`
- frozen v10: `b34e725fe4e4edcda3b278ce179b37c5f4ac6979193e9835293c0e8ee5be3633`

The subsequently generated 474-pose GT has SHA-256
`297a74076038044a5d9985bc75a8775da5e7361577da615641e5d998e1d296a5`.
The route is 414.48 m and supplies 366 100 m RPE segments. v10 improves ATE
from 5.60560 to 5.49352 m (-2.00%) and rotational RPE from 0.006913 to
0.006823 deg/m (-1.30%). Translational RPE changes from 1.62009% to 1.62245%
(+0.146%), safely within the frozen 1% guard. Primary and reference algorithm
rates are 26.24 and 23.45 FPS; their end-to-end rates also exceed 10 FPS at
11.29 and 13.79. Tracking is complete and no guard rejection occurs.

All frozen accuracy, runtime, tracking, causality, and GT-isolation gates pass
without post-result tuning. v10 is therefore **promoted** as the suite's
default candidate. Public baseline completion on drive 0023 and validation on
an additional normal-environment dataset family remain follow-up evidence,
not conditions retroactively added to this completed promotion decision.

On the same drive 0023 scans and GT, official MAD-ICP 0.0.10 official16 reaches
4.792 m ATE, 1.577% translational RPE, and 5.46 algorithm FPS. It is somewhat
more accurate than v10 but fails the 10 FPS gate. CT-ICP fast with no GT seed
reaches 24.041 m ATE, 3.221% RPE, and 12.76 FPS; v10 wins both accuracy metrics
while remaining real-time. MOLA-LO remains represented by the existing
GT-isolated KITTI Odometry 04 run (1.558 m ATE, 0.479% RPE, 3.15 end-to-end
FPS). It is not forced through a non-official Raw-to-Odometry calibration
adapter merely to add a drive 0023 row.

For a strict all-method same-input comparison, v10 was rerun on the exact 271
KITTI Odometry 04 PCD frames used by the existing MOLA/MAD/CT baselines. v10
obtains 1.145 m ATE and 0.354% translational RPE, ahead of v6 (1.267/0.355),
MOLA-LO (1.558/0.479), MAD-ICP (2.144/0.604), load-adaptive official KISS
(2.972/0.658), and CT-ICP fast (47.886/7.141). MOLA retains the best rotation
RPE at 0.00248 deg/m versus v10's 0.00284. Summing both v10 component algorithm
times conservatively still gives 10.67 FPS, so the accuracy win is not bought
by missing the established algorithm-rate gate.

The aggregate machine-readable comparison is
`evaluation/data/lidar_odometry_v10_aggregate.json`. Claims remain explicitly
Pareto and cross-dataset: official KISS is the accuracy winner on Raw 0023,
while v10 is the candidate that preserves real-time operation and avoids the
official method's Raw 0019 and NCLT transfer failures.

### Speed/disagreement adaptive v8 fresh held-out rejection

The official 1.95 GiB KITTI Raw drive 0019 archive was downloaded directly to
the external SSD and fixed as SHA-256
`77264aeb557441bb969fb928060173c940998b40c9814bef068a943e31fb551c`.
All 486 Velodyne archive members passed CRC checks; the synchronized sequence
contains 481 scans. The v6 primary, official KISS-ICP 1.3.0 eight-thread
reference, and frozen v8 fusion were run against scan-only data and a synthetic
identity reference. Before OXTS or calibration was extracted, their trajectory
hashes were fixed as:

- v6 primary: `48b8ea33af9add14a8c09d797be47c19a0d64322bc44b490b2cf7959952916cb`
- official KISS-ICP: `c8a5ef1390c9f5403a07d8b50b3aabe488bab2c247cfd33e61de96f8fa87d3f5`
- frozen v8: `8ca6846732046304af4d325690c1b643cf5f54753b15beb189f394709aa7e43`

Only then was initial-Velodyne GT generated, with SHA-256
`275090aafaabecb72d35b633af99d805b1a192363c71db6677c89e177f2fdf10`.
The route is 405.68 m and supplies 248 independent 100 m RPE segments. v8
improves ATE from 2.11891 to 2.06279 m (-2.65%), translational RPE from
0.97870% to 0.97292% (-0.59%), and rotational RPE from 0.004045 to 0.003997
deg/m. Thus the frozen accuracy gate passes.

The v6 primary reaches 27.59 algorithm FPS, but the frozen official KISS-ICP
reference reaches only 8.97 algorithm FPS (5.71 end-to-end FPS). Because the
predeclared gate requires both front ends to reach 10 algorithm FPS, v8 is
**not promoted** despite its accuracy win. Drive 0019 is now consumed and may
be used only for development of a materially new runtime-efficient reference
architecture; the v8 thresholds may not be retuned against this result.

Frozen external baselines were also evaluated over the same 481 frames. The
official MAD-ICP 0.0.10 configuration (`kitti`, default parameters, 16 cores,
16 keyframes) receives no GT until its process exits; it obtains 10.979 m ATE,
1.328% translational RPE, and 2.91 algorithm FPS. The repository's frozen
CT-ICP fast profile runs with `--no-gt-seed` against the synthetic identity
reference and is evaluated externally afterward; it obtains 25.541 m ATE,
5.728% translational RPE, and 5.88 algorithm FPS. v8 therefore beats both on
accuracy and rate on this drive, although that does not override its own
predeclared dual-front-end runtime failure.

### Load-adaptive official reference v9 freeze

Drive 0019 becomes development-only after v8 rejection. Increasing the
official reference cap from 8 to 16 threads is counterproductive (8.58 FPS),
while reducing it to 4 reaches 13.98 FPS. The four-thread reference has the
same ATE/RPE as eight threads, and the resulting fused trajectory is
byte-identical to v8. A universal four-thread cap does not transfer: on MulRan
ParkingLot it reaches only 4.18 FPS versus 10.53 FPS at eight threads.

v9 therefore retains the frozen v8 accuracy mechanism and selects reference
parallelism using only the first PCD schema. Scans with a timestamp field or
at most 50,000 points use eight threads; larger untimestamped scans use four.
This maps consumed KITTI Raw 0019 (126,085 points) to four, MulRan (40,013
timestamped points) to eight, and NCLT (24,309 points) to eight. It uses no GT,
dataset name, or future frame. The selected full-sequence reference rates are
13.98, 10.53, and 14.04 algorithm FPS respectively.

The policy and unchanged fusion are frozen on 2026-08-01. Before any download
or inspection, KITTI Raw `2011_09_26_drive_0022_sync` is declared as the sole
fresh held-out sequence. Its scan-only component and fused trajectories must
be generated and hashed before OXTS or calibration extraction. No threshold,
point-count boundary, or thread cap may be retuned after seeing that result.

### Load-adaptive reference v9 fresh held-out rejection

The official drive 0022 synchronized archive is 3,409,516,068 bytes with
SHA-256 `ec227729d0bfba90a61ee0567d614ce19d1a3038501297a85355d9b4519b2f3c`.
All 805 Velodyne members passed CRC checks, and only 800 scans were initially
extracted. OXTS and calibration were absent while the three trajectories were
generated and fixed as:

- v6 primary: `f9adc5d215ed4eb4b2381f5a22d5a3aec00b4a24062e2521ee6d3e12ada32fff`
- adaptive official reference: `d472feec4aa68e2d226b5d1a5808e51554f32c9c35ddfa589358520c511e4ce3`
- frozen v9: `64ad27afdeefe08ef956292f50b930860dbdcdb96e19eb5b06325e31aaa8cdd4`

The subsequently generated 800-pose GT has SHA-256
`ea8a65d4e26de6d5da2170b328a1d2e006a5b4c91c59e03f1528aab6f6e81fcf`.
The route is 516.06 m and supplies 601 100 m RPE segments. The adaptive
reference correctly selects four threads. Primary and reference rates are
22.81 and 21.28 algorithm FPS, so the runtime gate passes. v9 improves
translational RPE from 1.6241% to 1.5993% (-1.53%) and rotational RPE from
0.009318 to 0.008768 deg/m (-5.90%). However, ATE regresses from 4.0122 to
4.1493 m (+3.42%), violating the predeclared non-regression gate. v9 is
therefore **not promoted** and may not be retuned on this held-out result.

### Direction-consistent rotation v10 freeze

Drive 0022 becomes consumed development data after v9 rejection. Analysis of
the prior correction showed that its signed yaw agrees with the independent
primary/reference trailing travel-direction error much less often on the v9
failure than on KITTI 01/05 and Raw 0019. v10 uses that observable as a causal
gate. Over a trailing 10-frame window with at least 2 m travel, it publishes a
rotation correction only when the correction yaw has the sign needed to turn
the primary XY displacement toward the reference displacement. Reference
translation is used only for this decision and is never copied into output.
On rejection, output orientation returns to current primary orientation while
position continues to integrate primary relative translation.

A bounded consumed-data grid tested windows 10/20/50 and minimum direction
errors 0.05/0.1 degree. The selected 10-frame, 0.05-degree configuration is
the only chosen result; no further thresholds were explored. Relative to v6,
its ATE changes are -2.03% KITTI 01, -1.71% KITTI 05, -0.38% KITTI 07,
-0.018% MulRan, -0.22% NCLT, -0.59% Raw 0019, and -0.11% Raw 0022. All seven
ATE rows improve. Translational RPE changes range from -1.59% to +0.242%, so
every row remains inside the 1% preservation guard.

v10 retains the v9 load-adaptive official-reference runtime policy and is
frozen on 2026-08-01. Before download or inspection, KITTI Raw
`2011_09_26_drive_0023_sync` is declared as the sole fresh held-out sequence.
Component and fused trajectories must be generated and hashed before OXTS or
calibration extraction, and no post-result tuning is permitted.

### Direction-consistent rotation v10 fresh promotion

The declared drive 0023 archive contains 474 scans. The v6 primary, adaptive
official KISS reference, and frozen v10 trajectories were generated and
SHA-256 hashed while OXTS and calibration remained absent. Only afterward was
the GT extracted for external scoring. v10 improves ATE from 5.6056 m to
5.4935 m (-2.00%). Translational RPE changes from 1.6201% to 1.6225%
(+0.146%, within the 1% guard), while rotational RPE improves by 1.30%.
Primary and reference rates are 26.24 and 23.45 algorithm FPS; even their
conservative sequential combined rate is 12.38 FPS. All frozen accuracy,
runtime, tracking, and no-retuning gates pass, so v10 is promoted.

On the exact same 271 KITTI 04 PCD frames and external GT, v10 records the
best ATE (1.1448 m) and translational RPE (0.3541%) among v6, MOLA-LO 2.2.1,
MAD-ICP 0.0.10, official KISS-ICP 1.3.0, and CT-ICP fast. MOLA-LO retains the
best rotational RPE. Official KISS remains more accurate than v10 on fresh
Raw 0023, so the supported claim is cross-dataset/Pareto strength, not
universal per-sequence dominance.

The pre-Boreas promotion audit found 19 of 20 registered public datasets ready on the
external SSD, including every development, validation, and held-out role.
Only the low-priority NTNU tunnel robustness set is absent; fog and all normal
environment promotion data are present. The relevant repository audit passes
60 tests with five environment-dependent skips.

### External normal-environment transfer: Boreas

After v10 promotion, a fifth public dataset family was fixed before download:
the first 600 timestamp-ordered LiDAR scans from Boreas
`boreas-2021-06-03-16-00`, a summer urban drive. The selected scans total
3,084,291,600 bytes and 128,512,150 points. Only `lidar/*.bin` was downloaded
initially. The v6 primary, official KISS 1.3.0 reference, and frozen v10
trajectory hashes were fixed before `applanix/lidar_poses.csv` was obtained.
No thresholds or runtime policy were changed after scoring.

The 390.48 m window provides 478 100 m RPE segments. Relative to v6, v10
improves ATE from 0.66610 m to 0.66339 m (-0.407%) and translational RPE from
0.96701% to 0.96604% (-0.100%); rotational RPE changes by +0.012%. Official
KISS is the accuracy winner at 0.39070 m ATE and 0.95205% RPE. The primary
remains fast at 35.88 algorithm FPS, but official KISS reaches only 3.30 FPS on
the 208k-point scans, limiting the conservative dual rate to 3.02 FPS. Thus
accuracy transfer passes without tuning, while previously unseen dense-sensor
runtime is an explicit limitation rather than a promotion claim.

After registering Boreas, the data verifier reports 20 of 21 datasets ready;
the sole missing entry remains the low-priority NTNU tunnel robustness set.
