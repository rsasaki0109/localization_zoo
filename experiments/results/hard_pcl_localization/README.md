# Hard Point Cloud Localization: first GT-backed slice

Full-trajectory comparison of Koide `indoor_easy_01` (2,027 frames) and
`indoor_hard_01` (2,379 frames), using unchanged runner defaults. Both bags
have exact one-to-one PointCloud2 header-stamp association with their TUM GT.

## Easy vs hard

| Method | Easy ATE-XY (m) | Hard ATE-XY (m) | Easy RPE-XY (m/f) | Hard RPE-XY (m/f) | Easy accepted | Hard accepted |
|---|---:|---:|---:|---:|---:|---:|
| KISS keyframe | **8.763** | 25.307 | **0.076** | **0.124** | 1,168 / 2,026 | 864 / 2,378 |
| LiTAMIN2 | 10.815 | 17.875 | 0.358 | 0.611 | 322 / 2,026 | 927 / 2,378 |
| CT-ICP | 18.767 | 16.795 | 0.284 | 0.350 | 1,462 / 2,026 | 1,579 / 2,378 |
| X-ICP | 10.037 | **11.733** | 0.320 | 0.353 | 232 / 2,026 | 471 / 2,378 |
| DegenSense + IMU | 13.201 | 617.008 | 0.567 | 8.319 | 1,932 / 2,026 | 865 / 2,378 |
| DegenSense, no IMU | 11.759 | 11.729 | 2.736 | 3.679 | 1,065 / 2,026 | 643 / 2,378 |

The hard sequence clearly separates the baseline methods: KISS ATE nearly
triples, LiTAMIN2 worsens by 65%, and DegenSense's current IMU blend diverges.
CT-ICP's ATE happens to improve, but its hard estimated path is 468.38 m for a
115.49 m GT path, so this is not a true robustness improvement. X-ICP has the
lowest hard ATE but accepts only 19.8% of pairs; it mostly prevents publication
rather than maintaining continuous odometry.

## Why ATE alone is unsafe here

Both trajectories return close to their start:

| Sequence | GT planar path (m) | GT net XY (m) |
|---|---:|---:|
| indoor_easy_01 | 76.31 | 0.37 |
| indoor_hard_01 | 115.49 | 1.05 |

This makes average position error vulnerable to cancellation and frozen-pose
effects. The strongest example is hard DegenSense without IMU: ATE is only
11.73 m, but the estimate travels 6,587.52 m and has 100.18 deg/frame yaw RPE.
With IMU it travels 15,220.34 m and ends 339.72 m from its start. Report ATE
together with RPE, accepted-pair rate, path length, and net displacement.

## Hard-sequence details

| Method | RPE-yaw (deg/f) | Estimated path (m) | Net XY (m) | Degenerate pairs |
|---|---:|---:|---:|---:|
| KISS keyframe | 8.571 | 239.99 | 15.86 | n/a |
| LiTAMIN2 | 18.004 | 712.02 | 12.76 | n/a |
| CT-ICP | 16.089 | 468.38 | 10.68 | n/a |
| X-ICP | **7.897** | 309.65 | 3.02 | 510 |
| DegenSense + IMU | 14.350 | 15,220.34 | 339.72 | 206 |
| DegenSense, no IMU | 100.176 | 6,587.52 | 4.71 | 546 |

Machine-readable traces are under each sequence's `full/` directory. The
evaluation uses the initial full SE(3) GT pose to express motion in the initial
LiDAR frame, then computes planar ATE and adjacent-frame RPE from the runners'
`x/y/yaw` trajectories.

## BIEVR-LIO note

The repository's simplified from-paper BIEVR front-end diverges even on easy
(3D ATE 4.97 km default, 3.29 km dense; bump constraints active on roughly
0.1% of points). This does not test the newly published upstream system, which
adds map-informed fine sampling and an IMU backend. See the setup document and
the easy-sequence README for the upstream integration gap and negative-result
provenance.

## Fixed-map kidnap result

The provided `map_indoor_hard.ply` was also evaluated on both indoor kidnap
sequences. CT-ICP-seeded fixed-map NDT exposes the central false-lock result:
the embedded runtime guard emits 2,183 pose/hold outputs, and more than 99% are
over the 1 m GT safety envelope. The sparse-trace-aware verifier blocks both
sequences. A GT-informed replay becomes error-free only with pose holding
disabled, at the cost of returning unknown/blocking on nearly the entire
sequence. See `fixed_map_ndt/README.md`.
