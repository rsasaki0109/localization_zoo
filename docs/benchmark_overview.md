# Benchmark overview

Results that the [README](../README.md) only links to. Compare numbers only
within the same dataset, window, initialization policy and runtime profile.

## Contents

- [From-paper reimplementations on KITTI](#from-paper-reimplementations-no-public-reference-code--kitti-full)
- [Does LiDAR intensity help? I-LOAM ablation](#does-lidar-intensity-actually-help--i-loam-ablation)
- [Published benchmark groups](#published-benchmark-groups)
- [Synthetic urban](#synthetic-urban-30-frames)
- [2D laser scan odometry](#2d-laser-scan-odometry)
- [Hard point cloud localization](#hard-point-cloud-localization)
- [Promoted LiDAR odometry: v10 re-score](#promoted-lidar-odometry-v10-re-score)
- [Paper pipelines on another lidar (NCLT)](nclt_paper_paths.md)

## From-paper reimplementations (no public reference code) — KITTI full

> Details and negative results: [Reproducibility Report](reproducibility_report.md).

KITTI full-sequence odometry with first-pose anchor and `--no-gt-seed`.
RPE is drift %/100 m; ATE is shown in parentheses. LIO rows use a no-IMU fallback.

> This is a shared-harness catalog, not an endorsement of paper fidelity.
> Manuscript evidence is limited to the [frozen bundle](benchmarks/paper_ready_bundle.json).

| Method | Seq 00 _(4541 fr)_ | Seq 07 _(1101 fr)_ | Paper |
|---|---:|---:|---|
| DegenSense | **0.811%** <sub>(12 m)</sub> | 0.558% <sub>(1 m)</sub> | arXiv:2412.07513 |
| D2-LIO | 0.814% <sub>(11 m)</sub> | 0.541% <sub>(1 m)</sub> | arXiv:2508.14355 |
| M-GCLO | 0.835% <sub>(19 m)</sub> | 0.671% <sub>(2 m)</sub> | ISPRS Ann. 2024 |
| KC-LO | 0.837% <sub>(13 m)</sub> | **0.510%** <sub>(1 m)</sub> | ECCV 2004 |
| LiDAR-IBA | 0.841% <sub>(11 m)</sub> | 0.633% <sub>(1 m)</sub> | arXiv:2602.06380 |
| LODESTAR | 0.848% <sub>(7 m)</sub> | 0.598% <sub>(1 m)</sub> | arXiv:2511.09142 |
| Terrain-RBF-LIO | 0.849% <sub>(8 m)</sub> | 0.587% <sub>(1 m)</sub> | arXiv:2509.26222 |
| DALI-SLAM | 0.849% <sub>(8 m)</sub> | 0.600% <sub>(1 m)</sub> | ISPRS JPRS 2025 |
| DAMM-LOAM | 0.851% <sub>(7 m)</sub> | 0.598% <sub>(1 m)</sub> | arXiv:2510.13287 |
| CUBE-LIO | 0.851% <sub>(9 m)</sub> | 0.608% <sub>(1 m)</sub> | ROBOMECH 2026 |
| Intensity-Flow | 0.856% <sub>(8 m)</sub> | 0.616% <sub>(1 m)</sub> | Measurement 2026 |
| Quadric-LO | 0.867% <sub>(15 m)</sub> | 0.598% <sub>(2 m)</sub> | arXiv:2304.14190 |
| Adaptive-ICP | 0.870% <sub>(11 m)</sub> | 0.569% <sub>(1 m)</sub> | arXiv:2509.22058 |
| MCC-LO | 0.892% <sub>(13 m)</sub> | 0.611% <sub>(2 m)</sub> | PLOS ONE 2018 |
| **I-LOAM** | **0.899%** <sub>(13 m)</sub> | **0.575%** <sub>(2 m)</sub> | UR 2020 |
| Mesh-LOAM | 0.901% <sub>(13 m)</sub> | 0.616% <sub>(1 m)</sub> | IEEE T-IV 2024 |
| NHC-LIO | 0.902% <sub>(18 m)</sub> | 0.608% <sub>(3 m)</sub> | IEEE Sens. J. 2023 |
| SVN-ICP | 0.912% <sub>(14 m)</sub> | 0.607% <sub>(3 m)</sub> | arXiv:2509.08069 |
| ICPSC-LO | 0.912% <sub>(19 m)</sub> | 0.660% <sub>(4 m)</sub> | JAG 2023 |
| VLOM | 0.914% <sub>(10 m)</sub> | 0.605% <sub>(3 m)</sub> | arXiv:2304.08978 |
| TrICP-LO | 0.931% <sub>(10 m)</sub> | 0.662% <sub>(2 m)</sub> | IVC 2005 |
| GMM-LO | 0.941% <sub>(14 m)</sub> | 0.657% <sub>(1 m)</sub> | arXiv:1807.02587 |
| MCGICP-LO | 0.940% <sub>(20 m)</sub> | 0.774% <sub>(5 m)</sub> | RAS 2017 |
| Student-T-LO | 0.952% <sub>(15 m)</sub> | 0.696% <sub>(2 m)</sub> | PMC11314997 2024 |
| Small-but-Mighty | 0.961% <sub>(15 m)</sub> | 0.897% <sub>(3 m)</sub> | Remote Sens. 2025 |
| GNC-LO | 0.986% <sub>(18 m)</sub> | 0.722% <sub>(2 m)</sub> | arXiv:1909.08605 |
| IMLS-SLAM | 1.000% <sub>(18 m)</sub> | 0.700% <sub>(3 m)</sub> | ICRA 2018 |
| CT-VoxelMap | 1.046% <sub>(21 m)</sub> | 0.800% <sub>(3 m)</sub> | arXiv:2604.03747 |
| OPL-LVIO | 1.050% <sub>(15 m)</sub> | 0.902% <sub>(4 m)</sub> | Remote Sens. 2022 |
| AD-VLO | 1.052% <sub>(13 m)</sub> | 0.939% <sub>(3 m)</sub> | ICRA 2019 |
| TC-MVLO | 1.054% <sub>(11 m)</sub> | 0.939% <sub>(3 m)</sub> | ISR 2022 |
| TC-LVGF | 1.055% <sub>(12 m)</sub> | 0.941% <sub>(4 m)</sub> | ROBIO 2023 |
| TC-VLO | 1.060% <sub>(12 m)</sub> | 0.925% <sub>(4 m)</sub> | IV 2019 |
| BIEVR-LIO | 1.063% <sub>(25 m)</sub> | 0.873% <sub>(4 m)</sub> | arXiv:2604.14421 |
| V-LOAM2015 | 1.066% <sub>(14 m)</sub> | 0.910% <sub>(4 m)</sub> | ICRA 2015 |
| Vibration-LIO | 1.082% <sub>(15 m)</sub> | 0.781% <sub>(3 m)</sub> | arXiv:2507.04311 |
| ID-LIO | 1.111% <sub>(15 m)</sub> | 0.999% <sub>(5 m)</sub> | Sensors 2023 |
| ELO | 1.124% <sub>(23 m)</sub> | 0.981% <sub>(4 m)</sub> | IEEE RA-L 2021 |
| UA-LIO | 1.132% <sub>(33 m)</sub> | 0.967% <sub>(3 m)</sub> | IEEE TIM 2025 |
| DiLO | 1.200% <sub>(39 m)</sub> | 1.533% <sub>(7 m)</sub> | ETRI J. 2021 |
| PCR-DAT | 1.239% <sub>(11 m)</sub> | 1.040% <sub>(4 m)</sub> | ISR 2024 |
| RF-LIO | 1.351% <sub>(23 m)</sub> | 1.272% <sub>(5 m)</sub> | IROS 2021 |
| Spectral-LO | 2.901% <sub>(67 m)</sub> | 3.939% <sub>(27 m)</sub> | arXiv:2005.02042 |
| _KISS-ICP (same profile, ref)_ | _0.872%_ <sub>(12 m)</sub> | _0.618%_ <sub>(2 m)</sub> | — |
| _CT-ICP (same profile, ref)_ | _2.577%_ <sub>(17 m)</sub> | _2.500%_ <sub>(4 m)</sub> | — |

Mechanism evidence and limitations are summarized below; raw artifacts are linked.
<!-- EVIDENCE:LIO-IMU-HDL400:START -->
On public HDL-400 open (120 frames, 24k IMU samples), IMU-gated paths activate for D2-LIO, DegenSense, ID-LIO, and RF-LIO; RPE deltas vs no-`imu.csv` fallback are small on this window (D2-LIO 1.58% vs 1.52%, DegenSense 1.72% vs 1.60%) ([`hdl_400_lio_imu_validation_summary.json`](benchmarks/lio_imu_public/hdl_400_lio_imu_validation_summary.json)). LiDAR-IBA IMU residuals are not wired in `pcd_dogfooding` yet.
<!-- EVIDENCE:LIO-IMU-HDL400:END -->
<!-- EVIDENCE:LIO-IMU-NCLT:START -->
Public NCLT 2013-01-10 (120 frames, MS25 IMU) confirms IMU-gated paths and shows DegenSense compensation lowering ATE from 0.24 m to 0.16 m (~45% worse without IMU); KISS-ICP sanity is poor on this window (86% RPE) so the row is mechanism evidence only ([`nclt_2013_01_10_120_lio_imu_validation_summary.json`](benchmarks/lio_imu_public/nclt_2013_01_10_120_lio_imu_validation_summary.json)).
<!-- EVIDENCE:LIO-IMU-NCLT:END -->
**M-GCLO:** disabling ground factors worsens ATE and rotational drift on KITTI
([ablation](benchmarks/kitti_full_new_methods/m_gclo_ground_factor_ablation.json))
and synthetic rolling ground ([stress test](benchmarks/nonflat_ground_stress/m_gclo_nonflat_ground_stress_summary.json)).
<!-- EVIDENCE:M-GCLO-SEQ08:START -->
Public hilly KITTI seq08 full shows the same anchoring pattern: ground off
leaves RPE ~1.35% unchanged but worsens ATE by about 149%
([`m_gclo_kitti_seq08_validation_summary.json`](benchmarks/kitti_seq08_public/m_gclo_kitti_seq08_validation_summary.json)).
<!-- EVIDENCE:M-GCLO-SEQ08:END -->
<!-- EVIDENCE:M-GCLO-MULRAN:START -->
Public MulRan ParkingLot full (1176 Ouster frames): no-gt-seed odometry diverges (~103% KISS-ICP RPE). GT-seeded M-GCLO stays stable (~2.49% RPE); ground off improves ATE from 2.38 m to 1.93 m (~19% delta) — oracle-init mechanism evidence, not blind odometry ([`m_gclo_mulran_parkinglot_validation_summary.json`](benchmarks/mulran_parkinglot_public/m_gclo_mulran_parkinglot_validation_summary.json)).
<!-- EVIDENCE:M-GCLO-MULRAN:END -->
**KC-LO:** best seq07 drift here, but only ~1.4–3.1 FPS ([ablation](benchmarks/kitti_full_new_methods/kc_lo_sigma_schedule_ablation.json)).
**LiDAR-IBA:** bundle adjustment slightly lowers ATE but worsens RPE and throughput ([ablation](benchmarks/kitti_full_new_methods/lidar_iba_ba_ablation.json)).
**Quadric-LO:** plane fallback is rare on highway data ([ablation](benchmarks/kitti_full_new_methods/quadric_lo_plane_fallback_ablation.json); [curved-object stress](benchmarks/quadric_curved_stress/quadric_curved_stress_summary.json)).
<!-- EVIDENCE:QUADRIC-SEQ02:START -->
Public residential KITTI seq02 full still uses >99% quadric correspondences, but
disabling plane fallback worsens RPE by about 55% and ATE by about 84% — the rare
fallback carries more weight on suburban/vegetation-rich driving than on highway
seq00/07
([`quadric_lo_kitti_seq02_validation_summary.json`](benchmarks/kitti_seq02_public/quadric_lo_kitti_seq02_validation_summary.json)).
<!-- EVIDENCE:QUADRIC-SEQ02:END -->
On geometry-rich, IMU-free KITTI, many robust and multimodal mechanisms become
secondary to the point-to-plane core. Dynamic-scene stress still favors RF-LIO
over ID-LIO ([artifact](benchmarks/dynamic_object_stress/rf_id_lio_dynamic_object_stress_summary.json)).
<!-- EVIDENCE:RF-ID-LIO-SEQ05:START -->
Public urban KITTI seq05 full keeps both paths active but RF-LIO default still
trails ID-LIO (0.961% vs 0.712% RPE) and KISS-ICP (0.617%)
([`rf_id_lio_kitti_seq05_validation_summary.json`](benchmarks/kitti_seq05_public/rf_id_lio_kitti_seq05_validation_summary.json)).
<!-- EVIDENCE:RF-ID-LIO-SEQ05:END -->
Full ablations and negative results: [benchmark artifacts](benchmarks/kitti_full_new_methods/).

## Does LiDAR intensity actually help? — I-LOAM ablation

[I-LOAM](../papers/i_loam/) intensity on/off results with mapping disabled:

| Sequence | Geometric baseline (intensity off) | I-LOAM (intensity on) | Δ drift |
|---|---:|---:|---:|
| Seq 00 | 3.186% <sub>(76.0 m)</sub> | **2.606%** <sub>(49.4 m)</sub> | **−18.2%** |
| Seq 07 | 3.806% <sub>(18.5 m)</sub> | **3.055%** <sub>(15.1 m)</sub> | **−19.7%** |

Reflectance cuts drift by 18–20% in this paired KITTI ablation. Raw artifacts:
[`seq00 on`](benchmarks/kitti_full_new_methods/seq00_i_loam_no_mapping.json),
[`seq00 off`](benchmarks/kitti_full_new_methods/seq00_i_loam_no_intensity.json),
[`seq07 on`](benchmarks/kitti_full_new_methods/seq07_i_loam_no_mapping.json),
[`seq07 off`](benchmarks/kitti_full_new_methods/seq07_i_loam_no_intensity.json),
and the paired summary
[`i_loam_intensity_ablation.json`](benchmarks/kitti_full_new_methods/i_loam_intensity_ablation.json).

## Published benchmark groups

Current published groups:

| Group | Role | Ranking policy | Source |
|---|---|---|---|
| KITTI Odometry seq00 full | Odometry full sequence | Ranked within seq00 by translational RPE [%/100 m] | [`docs/benchmarks/paper_ready_bundle.json`](benchmarks/paper_ready_bundle.json) |
| KITTI Odometry seq07 full | Odometry full sequence | Ranked within seq07 by translational RPE [%/100 m] | [`docs/benchmarks/paper_ready_bundle.json`](benchmarks/paper_ready_bundle.json) |
| KITTI Odometry seq07 108-frame smoke | Regression smoke | Unranked; exact frame-ID association check only | [`docs/benchmarks/latest/results.json`](benchmarks/latest/results.json) |
| Autoware Istanbul 108-frame snapshot | GT-seeded scan-to-map references | Reference-only; not ranked against odometry | [`docs/benchmarks/latest/results.json`](benchmarks/latest/results.json) |

Detailed rows and provenance are in the
[interactive explorer](https://rsasaki0109.github.io/localization_zoo/),
[interfaces](interfaces.md), and [methodology](dogfooding_methodology.md).
KITTI preparation example:

```bash
python3 evaluation/scripts/prepare_kitti_odometry_inputs.py \
  --kitti-root /path/to/data_odometry --sequence 00 --sequence 07 \
  --window-size 108 --include-full
```

## Synthetic Urban (30 frames)

```
Method         ATE [m]     FPS
─────────────────────────────────
CT-ICP         0.124       0.1   << best accuracy
A-LOAM         2.059       4.6   << balanced baseline
X-ICP          16.634      5.9
LiTAMIN2       31.155      2.6
```

> Reproduce with `./synthetic_benchmark`. No external dataset is required.

## 2D Laser Scan Odometry

Eight planar matchers (papers 43–50) plus **Karto-Matcher** (map-based CSM extension) in [`scan_dogfooding`](../evaluation/src/scan_dogfooding.cpp).
Full leaderboard, fixtures, and reproduction steps:
[**docs/benchmarks/scan2d/README.md**](benchmarks/scan2d/README.md).

| Method | Intel val | fr079 val | MIT val | Synth corridor |
|--------|---:|---:|---:|---:|
| | _73 fr / 378 m_ | _384 fr / 373 m_ | _33 fr / 267 m_ | _120 fr / 9.5 m_ |
| **RF2O** | **14.3%** | 15.4% | 27.6% | 1.3% |
| **Karto-Matcher** | 14.0% | 13.7% | 28.1% | 30.5% |
| **CSM** | 14.0% | 13.7% | 28.1% | 30.5% |
| **NDT-2D** | 14.9% | **14.4%** | 27.8% | **0.8%** |
| **IDC** | 15.3% | 27.7% | 29.5% | 42.6% |
| **MbICP** | 14.5% | 15.4% | 27.5% | **0.05%** |
| **PL-ICP** | 15.0% | 14.1% | 27.2% | **0.01%** |
| **Kinematic-ICP** | 18.4% | 18.9% | **23.4%** | 83.8% |
| **PSM** | 21.8% | **13.9%** | 27.9% | 11.6% |

```bash
cmake --build build --target scan_dogfooding
bash evaluation/scripts/run_scan2d_benchmark.sh
```

Setup: [`evaluation/scripts/SETUP_2D_SCAN_BENCHMARK.md`](../evaluation/scripts/SETUP_2D_SCAN_BENCHMARK.md).

## Hard Point Cloud Localization

[Hard Point Cloud Localization Dataset](https://zenodo.org/records/10122133)
results; full protocol and coverage are in the
[benchmark README](benchmarks/hard_pcl/README.md).

Full-trajectory `indoor_easy_01` vs `indoor_hard_01`, six unchanged runner defaults:

| Method | Easy ATE-XY (m) | Hard ATE-XY (m) | Easy RPE-XY (m/f) | Hard RPE-XY (m/f) |
|---|---:|---:|---:|---:|
| KISS keyframe | **8.763** | 25.307 | **0.076** | **0.124** |
| LiTAMIN2 | 10.815 | 17.875 | 0.358 | 0.611 |
| CT-ICP | 18.767 | 16.795 | 0.284 | 0.350 |
| X-ICP | 10.037 | **11.733** | 0.320 | 0.353 |
| DegenSense + IMU | 13.201 | 617.008 | 0.567 | 8.319 |
| DegenSense, no IMU | 11.759 | 11.729 | 2.736 | 3.679 |

CT-ICP's lower hard ATE is misleading: its estimated path is 468.38 m versus
115.49 m ground truth. Official [BIEVR-LIO](https://github.com/ethz-asl/BIEVR-LIO):

| Sequence | ATE-XY (m) | RPE-XY (m/f) | Estimated / GT path (m) |
|---|---:|---:|---:|
| `indoor_easy_01` | **0.422** | **0.0033** | 76.26 / 77.29 |
| `indoor_hard_01` | 8,882.579 | 21.947 | 28,754.33 / 114.71 |

BIEVR-LIO excels on easy but diverges on hard. Fixed-map NDT also false-locks
on all three kidnap sequences, showing that stable refinement alone does not
prove a correct global lock.

```bash
python3 evaluation/scripts/run_hard_pcl_odometry_benchmark.py \
  --pcd-dir dogfooding_results/hard_pcl_localization/indoor_easy_01 \
  --gt-csv experiments/reference_data/hard_pcl_indoor_easy_01_gt.csv \
  --output-dir experiments/results/hard_pcl_localization/indoor_easy_01/full --jobs 6
```

Setup: [`evaluation/scripts/SETUP_HARD_PCL_LOCALIZATION_BENCHMARK.md`](../evaluation/scripts/SETUP_HARD_PCL_LOCALIZATION_BENCHMARK.md).

## Promoted LiDAR odometry: v10 re-score

[protocol](lidar_odometry_sota_protocol.md). The earlier v10 KITTI 04,
KITTI Raw 0023, and Boreas 2021-06-03 comparison against MOLA-LO, MAD-ICP,
official KISS-ICP, and CT-ICP remains in the
[v10 aggregate](../evaluation/data/lidar_odometry_v10_aggregate.json) and
[SOTA log](lidar_odometry_sota_log.md).

Re-score the frozen v10 trajectories and verify all hashes from the external SSD:

```bash
export LOCALIZATION_ZOO_DATA_ROOT=/media/external/loc_zoo
python3 evaluation/scripts/reproduce_lidar_v10.py
```

On PowerShell, set `$env:LOCALIZATION_ZOO_DATA_ROOT` before running the command.
