# Hard Point Cloud Localization Benchmark

Source dataset: [Koide, "A dataset to evaluate the robustness of point-cloud-based
localization algorithms in extremely severe situations"](https://zenodo.org/records/10122133)
(Zenodo 10.5281/zenodo.10122133, CC-BY 4.0). Eight sequences: indoor (Azure
Kinect, dense RGB-D-style clouds) `easy_01/02`, `hard_01`, `kidnap_01/02`, and
outdoor (Livox MID360, non-repetitive scan pattern) `hard_01/02`, `kidnap`. TUM
ground truth (`gt.zip`) and four provided environment `.ply` maps ship with the
dataset, which unblocks two things the NTNU LiDAR degeneracy bags could not:
GT-backed ATE/RPE for degeneracy-aware methods, and a fixed-map localization
false-lock / kidnap-recovery track against real maps instead of maps rebuilt
from the evaluated sequence's own GT.

Full campaign notes (extraction, clock-domain checks, per-sequence frame
counts, upstream integration detail): [`evaluation/scripts/SETUP_HARD_PCL_LOCALIZATION_BENCHMARK.md`](../../../evaluation/scripts/SETUP_HARD_PCL_LOCALIZATION_BENCHMARK.md).
Full result trees and machine-readable traces: [`experiments/results/hard_pcl_localization/`](../../../experiments/results/hard_pcl_localization/).

## Odometry track (GT-backed, six unchanged runner defaults)

Full-trajectory `indoor_easy_01` (2,027 frames) vs `indoor_hard_01` (2,379
frames), same runner configuration on both:

| Method | Easy ATE-XY (m) | Hard ATE-XY (m) | Easy RPE-XY (m/f) | Hard RPE-XY (m/f) | Easy accepted | Hard accepted |
|---|---:|---:|---:|---:|---:|---:|
| KISS keyframe | **8.763** | 25.307 | **0.076** | **0.124** | 1,168 / 2,026 | 864 / 2,378 |
| LiTAMIN2 | 10.815 | 17.875 | 0.358 | 0.611 | 322 / 2,026 | 927 / 2,378 |
| CT-ICP | 18.767 | 16.795 | 0.284 | 0.350 | 1,462 / 2,026 | 1,579 / 2,378 |
| X-ICP | 10.037 | **11.733** | 0.320 | 0.353 | 232 / 2,026 | 471 / 2,378 |
| DegenSense + IMU | 13.201 | 617.008 | 0.567 | 8.319 | 1,932 / 2,026 | 865 / 2,378 |
| DegenSense, no IMU | 11.759 | 11.729 | 2.736 | 3.679 | 1,065 / 2,026 | 643 / 2,378 |

The hard sequence separates the baselines: KISS ATE nearly triples, LiTAMIN2
worsens 65%, and DegenSense's IMU blend diverges. CT-ICP's ATE happens to
improve on hard, but its estimated path is 468.38 m against a 115.49 m GT path
— not a real robustness gain. X-ICP has the lowest hard ATE but accepts only
19.8% of pairs, so it is mostly blocking publication rather than tracking
continuously. Both GT trajectories return close to their start (net XY 0.37 m
and 1.05 m against 76.31 m / 115.49 m paths), so ATE alone is unsafe here —
hard DegenSense-no-IMU shows an 11.73 m ATE next to a 6,587.52 m estimated path
and 100.18 deg/frame yaw RPE.

### Expanded coverage — all eight sequences

`indoor_easy_02` and the three outdoor sequences now also have the full
six-method matrix:

| Sequence | Frames | Lowest ATE-XY (m) | Method | RPE-XY (m/f) |
|---|---:|---:|---|---:|
| indoor_easy_01 | 2,027 | 8.763 | KISS keyframe | 0.076 |
| indoor_easy_02 | 1,967 | 9.797 | KISS keyframe | 0.077 |
| indoor_hard_01 | 2,379 | 11.729 | DegenSense no IMU | 3.679 |
| outdoor_hard_01 | 5,147 | 128.088 | DegenSense + IMU | 0.293 |
| outdoor_hard_02 | 5,127 | 105.224 | LiTAMIN2 | 0.291 |
| outdoor_kidnap | 4,017 | 112.915 | LiTAMIN2 | 0.208 |

This is a lowest-ATE table, not a robustness ranking — the low indoor-hard
DegenSense-no-IMU ATE above is misleading given its RPE and path length. The
outdoor ATE values stay large even where adjacent-frame RPE looks modest: this
is sustained global drift, which short-window health gates cannot catch.

Outdoor DegenSense IMU sensitivity is not a uniform rescue:

| Sequence | IMU ATE / RPE-XY (m) | No-IMU ATE / RPE-XY (m) |
|---|---:|---:|
| outdoor_hard_01 | 128.088 / 0.293 | **530,394.694 / 560.386** |
| outdoor_hard_02 | 264.218 / 0.952 | 160.330 / 0.504 |
| outdoor_kidnap | 204.541 / 1.654 | 5,872.830 / 46.038 |

IMU blending prevents catastrophic no-IMU failure on `outdoor_hard_01` and
`outdoor_kidnap`, but is worse on `outdoor_hard_02`. No-IMU can also look
locally accepted for long stretches before a data-loss reconnection drives
enormous global error — recovery after data loss needs its own validation
state, not just short-window convergence checks.

## Upstream BIEVR-LIO (recovery baseline)

The repository's from-paper simplified BIEVR-LIO front-end diverges even on
easy (3D ATE 4.97 km default / 3.29 km dense; bump constraints active on
~0.1% of points). Evaluating the official [upstream BIEVR-LIO](https://github.com/ethz-asl/BIEVR-LIO)
core (commit `21121698f273d6fbfffca57546b940edb1de2ff0`) through a standalone
PCD/IMU adapter gives a very different, and much stronger, result on easy:

| Sequence | Associated poses | ATE-XY (m) | RPE-XY (m/f) | Estimated / GT path (m) |
|---|---:|---:|---:|---:|
| `indoor_easy_01` | 2,021 | **0.422** | **0.0033** | 76.26 / 77.29 |
| `indoor_hard_01` | 2,373 | 8,882.579 | 21.947 | 28,754.33 / 114.71 |

The easy result is substantially better than any of the six existing defaults
and validates the adapter/calibration. The hard run crosses 1 m error at
associated frame 943, 10 m at 1,124, and 100 m at 1,228, then diverges.
Upstream BIEVR-LIO's map-informed sampling plus inertial backend materially
improves nominal tracking, but it is a strong odometry baseline, not a
general relocalization or global-lock guarantee under this hard trajectory.

## Fixed-map localization / kidnap false-lock track

CT-ICP-seeded fixed-map NDT was evaluated against the dataset-provided `.ply`
maps on all three kidnap sequences (the explicit `--fixed-map-ndt-map-ply`
option, so the target map is not rebuilt from the evaluated sequence's own
GT):

| Sequence | Input frames | Trace frames | NDT accepted | CT prior ATE (m) | Fixed-map ATE (m) |
|---|---:|---:|---:|---:|---:|
| indoor_kidnap_01 | 2,154 | 1,773 | 549 / 1,772 | 9.546 | 8.839 |
| indoor_kidnap_02 | 1,609 | 1,303 | 59 / 1,302 | 15.100 | 16.966 |
| outdoor_kidnap | 4,017 | 2,907 | 171 / 2,906 | 107.730 | 103.823 |

The embedded GT-free runtime publish guard is unsafe on all three traces: it
emits 4,015 pose/hold outputs, 3,987-3,995 of which are wrong by replay (>1 m
error, up to 183.805 m). The post-hoc GT sequence verifier classifies all
three traces as `block_publish` (5,690/5,983 raw frames wrong, 2,069 unsafe
transitions). A GT-informed replay only becomes error-free with pose holding
disabled (`max_hold_frames=0`: 226 outputs, zero wrong, max 0.945 m), at the
cost of returning unknown/blocking on most frames. **Publish-guard finding:**
stable-refinement/hold runtime signals are not evidence of a correct global
lock; the default 3-frame-hold policy still leaks 9 wrong poses among 236
outputs (up to 3.843 m). See
[`experiments/results/hard_pcl_localization/fixed_map_ndt/README.md`](../../../experiments/results/hard_pcl_localization/fixed_map_ndt/README.md).

## Degeneracy triage GT calibration

The GT-free `lidar_degeneracy_triage_v4` policy was calibrated against real GT
error using this dataset (36 method/sequence rows across both easy indoor
sequences, indoor hard, and all three outdoor trajectories). Seven outdoor
rows receive the current runtime-proxy `pass` decision despite ATE from
105.224 m to 264.218 m — high accepted/converged rates and cross-method path
consistency can look locally coherent while remaining blind to accumulated
global drift. The policy's `pass` should be read as "no local alarm," not as
an accuracy certificate. Detail:
[`experiments/results/hard_pcl_localization/risk_gt_calibration/risk_gt_calibration.md`](../../../experiments/results/hard_pcl_localization/risk_gt_calibration/risk_gt_calibration.md).

## Reproduce

```bash
# Extraction (per sequence) and GT conversion
python3 evaluation/scripts/extract_ros2_lidar_imu.py \
  --bag data/hard_pcl_localization/indoor_easy_01 \
  --pointcloud-topic /points2/decompressed --imu-topic /imu \
  --output-dir dogfooding_results/hard_pcl_localization/indoor_easy_01
python3 evaluation/scripts/tum_to_gt_csv.py \
  --input data/hard_pcl_localization/gt/gt/traj_lidar_indoor_easy_01.txt \
  --output experiments/reference_data/hard_pcl_indoor_easy_01_gt.csv

# Six default configurations with GT-backed ATE/RPE attached
python3 evaluation/scripts/run_hard_pcl_odometry_benchmark.py \
  --pcd-dir dogfooding_results/hard_pcl_localization/indoor_easy_01 \
  --gt-csv experiments/reference_data/hard_pcl_indoor_easy_01_gt.csv \
  --output-dir experiments/results/hard_pcl_localization/indoor_easy_01/full \
  --jobs 6

# Fixed-map NDT against the provided PLY map
build/evaluation/pcd_dogfooding \
  dogfooding_results/hard_pcl_localization/indoor_kidnap_01 \
  experiments/reference_data/hard_pcl_indoor_kidnap_01_gt.csv 2154 \
  --methods fixed_map_ndt \
  --fixed-map-ndt-map-ply data/hard_pcl_localization/map_indoor_hard.ply \
  --fixed-map-ndt-seed-source ct_icp

# Upstream BIEVR-LIO recovery baseline
evaluation/scripts/setup_bievr_lio_upstream.sh
evaluation/scripts/run_bievr_lio_hard_pcl.sh indoor_easy_01
evaluation/scripts/run_bievr_lio_hard_pcl.sh indoor_hard_01
```

Setup and campaign detail: [`evaluation/scripts/SETUP_HARD_PCL_LOCALIZATION_BENCHMARK.md`](../../../evaluation/scripts/SETUP_HARD_PCL_LOCALIZATION_BENCHMARK.md).
Per-sequence result READMEs: [`indoor_easy_01`](../../../experiments/results/hard_pcl_localization/indoor_easy_01/README.md),
[`indoor_hard_01`](../../../experiments/results/hard_pcl_localization/indoor_hard_01/README.md),
[full results tree](../../../experiments/results/hard_pcl_localization/README.md).
