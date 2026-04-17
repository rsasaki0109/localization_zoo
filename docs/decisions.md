# Decisions

_Generated at 2026-04-18T09:39:41+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Rules

1. Keep at least three concrete variants alive for each active problem.
2. Promote a variant only when it is comparable on the same binary, dataset, and metrics.
3. Store the benchmark contract in the stable core and keep the profile search in `experiments/`.
4. Do not delete weaker variants unless they stop being informative.

## A-LOAM throughput and accuracy trade-off on the second public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/aloam_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## A-LOAM throughput and accuracy trade-off on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/aloam_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## A-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/aloam_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## A-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/aloam_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## A-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/aloam_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## A-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/aloam_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## A-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/aloam_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## A-LOAM throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/aloam_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## A-LOAM throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `dense`.
- Active challengers: `fast`.
- Reference variants: `kitti_default`.
- Aggregate result: `experiments/results/aloam_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## A-LOAM throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/aloam_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## BALM2 on the second public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/balm2_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## BALM2 on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/balm2_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## BALM2 on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/balm2_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## BALM2 on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/balm2_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## BALM2 on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/balm2_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## BALM2 on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/balm2_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## BALM2 on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/balm2_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## BALM2 on MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/balm2_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## BALM2 on MCD NTU day-02 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/balm2_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## BALM2 on MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/balm2_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CLINS trade-off on the public ROS1 HDL-400 window with synthesized per-point time

- Current default: `dense`.
- Reference variants: `default`, `fast`.
- Aggregate result: `experiments/results/clins_hdl_400_public_ros1_synthtime_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## CT-ICP performance-priority trade-off on the public ROS1 HDL-400 window with synthesized per-point time

- Current default: `dense_window`.
- Reference variants: `balanced_window`, `fast_window`.
- Aggregate result: `experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## CT-ICP throughput and drift trade-off on the second public HDL-400 reference window

- Current default: `fast_window`.
- Reference variants: `balanced_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP throughput and drift trade-off on the public HDL-400 reference window

- Current default: `fast_window`.
- Reference variants: `balanced_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence

- Current default: `balanced_window`.
- Reference variants: `fast_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_istanbul_window_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence

- Current default: `balanced_window`.
- Reference variants: `fast_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_istanbul_window_c_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `balanced_window`.
- Active challengers: `fast_window`.
- Reference variants: `dense_window`.
- Aggregate result: `experiments/results/ct_icp_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_window | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `balanced_window`.
- Active challengers: `fast_window`.
- Reference variants: `dense_window`.
- Aggregate result: `experiments/results/ct_icp_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_window | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `balanced_window`.
- Active challengers: `fast_window`.
- Reference variants: `dense_window`.
- Aggregate result: `experiments/results/ct_icp_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_window | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast_window`.
- Reference variants: `balanced_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast_window`.
- Reference variants: `balanced_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `fast_window`.
- Reference variants: `balanced_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `dense_window`.
- Reference variants: `balanced_window`, `fast_window`.
- Aggregate result: `experiments/results/ct_icp_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## CT-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `fast_window`.
- Reference variants: `balanced_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP throughput and drift trade-off on the repository-stored Istanbul sequence

- Current default: `fast_window`.
- Reference variants: `balanced_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_profile_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-LIO trade-off on the public ROS1 HDL-400 window with synthesized per-point time

- Current default: `seed_only_fast`.
- Reference variants: `imu_preintegration_default`, `history_smoother_dense`.
- Aggregate result: `experiments/results/ct_lio_hdl_400_public_ros1_synthtime_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| imu_preintegration_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| seed_only_fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| history_smoother_dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-LIO GT-backed public benchmark readiness on HDL-400 ROS2 data

- Current default: `-`.
- Status: `blocked`.
- Blocker: A reference-based CT-LIO comparison now exists separately, but this repository still does not ship an independently curated GT CSV aligned to the extracted HDL-400 LiDAR+IMU sequence.
- Next step: No action planned. CT-LIO GT-backed is excluded from the main study scope. If an independent GT source becomes available for HDL-400, this can be revisited.
- Aggregate result: `experiments/results/ct_lio_public_readiness_matrix.json`

## CT-LIO reference-trajectory trade-off on the public HDL-400 120-frame window

- Current default: `seed_only_fast`.
- Reference variants: `imu_preintegration_default`, `history_smoother_dense`.
- Aggregate result: `experiments/results/ct_lio_reference_profile_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| imu_preintegration_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| seed_only_fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| history_smoother_dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLIO throughput and accuracy trade-off on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlio_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLIO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlio_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLIO throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlio_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLIO throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlio_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLIO throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlio_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLIO throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlio_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLIO throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlio_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLIO throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `kitti_default`.
- Active challengers: `fast`.
- Reference variants: `dense`.
- Aggregate result: `experiments/results/dlio_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLIO throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlio_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLO throughput and accuracy trade-off on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlo_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlo_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLO throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlo_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLO throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlo_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLO throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlo_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLO throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlo_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLO throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlo_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLO throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `kitti_default`.
- Active challengers: `fast`.
- Reference variants: `dense`.
- Aggregate result: `experiments/results/dlo_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## DLO throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/dlo_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO2 on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio2_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO2 on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio2_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO2 on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio2_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO2 on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio2_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO2 on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio2_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO2 on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio2_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO2 on MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio2_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO2 on MCD NTU day-02 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio2_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO2 on MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio2_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO-SLAM on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio_slam_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO-SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio_slam_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO-SLAM on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio_slam_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO-SLAM on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio_slam_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO-SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio_slam_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO-SLAM on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio_slam_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO-SLAM on MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio_slam_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO-SLAM on MCD NTU day-02 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio_slam_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIO-SLAM on MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_lio_slam_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## F-LOAM throughput and accuracy trade-off on the second public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/floam_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## F-LOAM throughput and accuracy trade-off on the public HDL-400 reference window

- Current default: `fast`.
- Active challengers: `kitti_default`.
- Reference variants: `dense`.
- Aggregate result: `experiments/results/floam_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## F-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/floam_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## F-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/floam_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## F-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/floam_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## F-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/floam_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## F-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/floam_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## F-LOAM throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/floam_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## F-LOAM throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/floam_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## F-LOAM throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/floam_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## GICP throughput and accuracy trade-off on the second public HDL-400 reference window

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## GICP throughput and accuracy trade-off on the public HDL-400 reference window

- Current default: `fast_recent_map`.
- Active challengers: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## GICP throughput and accuracy trade-off on the second repository-stored Istanbul sequence

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_istanbul_window_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## GICP throughput and accuracy trade-off on the third repository-stored Istanbul sequence

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_istanbul_window_c_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## GICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `dense_recent_map`.
- Reference variants: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/gicp_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## GICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `dense_recent_map`.
- Reference variants: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/gicp_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## GICP throughput and accuracy trade-off on the repository-stored Istanbul sequence

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_profile_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## HDL-Graph-SLAM on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/hdl_graph_slam_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## HDL Graph SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `default`.
- Aggregate result: `experiments/results/hdl_graph_slam_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## HDL Graph SLAM on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/hdl_graph_slam_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## HDL-Graph-SLAM on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `default`.
- Aggregate result: `experiments/results/hdl_graph_slam_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## HDL Graph SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Skipped: computation exceeds 1 hour on KITTI Raw 0061 full (703 frames)
- Next step: Run a lighter slice/profile, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/hdl_graph_slam_kitti_raw_0061_full_matrix.json`

## HDL Graph SLAM on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/hdl_graph_slam_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## HDL Graph SLAM on MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/hdl_graph_slam_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## HDL Graph SLAM on MCD NTU day-02 sequence

- Current default: `dense`.
- Reference variants: `default`, `fast`.
- Aggregate result: `experiments/results/hdl_graph_slam_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## HDL Graph SLAM on MCD TUHH night-09 sequence

- Current default: `dense`.
- Reference variants: `default`, `fast`.
- Aggregate result: `experiments/results/hdl_graph_slam_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## ISC-LOAM on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/isc_loam_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## ISC-LOAM on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/isc_loam_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## ISC-LOAM on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/isc_loam_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## ISC-LOAM on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/isc_loam_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## ISC-LOAM on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/isc_loam_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## ISC-LOAM on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `dense`.
- Active challengers: `fast`.
- Reference variants: `default`.
- Aggregate result: `experiments/results/isc_loam_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## ISC-LOAM on MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/isc_loam_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## ISC-LOAM on MCD NTU day-02 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/isc_loam_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## ISC-LOAM on MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/isc_loam_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## KISS-ICP throughput and drift trade-off on the second public HDL-400 reference window

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/kiss_icp_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## KISS-ICP throughput and drift trade-off on the public HDL-400 reference window

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/kiss_icp_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## KISS-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence

- Current default: `dense_local_map`.
- Active challengers: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/kiss_icp_istanbul_window_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_recent_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense_local_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## KISS-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence

- Current default: `fast_recent_map`.
- Active challengers: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/kiss_icp_istanbul_window_c_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## KISS-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/kiss_icp_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/kiss_icp_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## KISS-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/kiss_icp_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## KISS-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast_recent_map`.
- Active challengers: `balanced_local_map`.
- Reference variants: `dense_local_map`.
- Aggregate result: `experiments/results/kiss_icp_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/kiss_icp_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## KISS-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/kiss_icp_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## KISS-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `fast_recent_map`.
- Active challengers: `dense_local_map`.
- Reference variants: `balanced_local_map`.
- Aggregate result: `experiments/results/kiss_icp_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## KISS-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/kiss_icp_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## KISS-ICP throughput and drift trade-off on the repository-stored Istanbul sequence

- Current default: `fast_recent_map`.
- Active challengers: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/kiss_icp_profile_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## LeGO-LOAM throughput and accuracy trade-off on the second public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/lego_loam_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LeGO-LOAM throughput and accuracy trade-off on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/lego_loam_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LeGO-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/lego_loam_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LeGO-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/lego_loam_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LeGO-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/lego_loam_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LeGO-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/lego_loam_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LeGO-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/lego_loam_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LeGO-LOAM throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/lego_loam_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LeGO-LOAM throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `fast`.
- Active challengers: `dense`.
- Reference variants: `kitti_default`.
- Aggregate result: `experiments/results/lego_loam_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## LeGO-LOAM throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/lego_loam_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LINS on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lins_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LINS on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lins_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LINS on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lins_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LINS on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lins_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LINS on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lins_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LINS on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lins_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LINS on MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lins_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LINS on MCD NTU day-02 sequence

- Current default: `dense`.
- Active challengers: `fast`.
- Reference variants: `default`.
- Aggregate result: `experiments/results/lins_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## LINS on MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lins_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LIO-SAM on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lio_sam_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LIO-SAM on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lio_sam_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LIO-SAM on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lio_sam_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LIO-SAM on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lio_sam_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LIO-SAM on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lio_sam_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LIO-SAM on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lio_sam_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LIO-SAM on MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lio_sam_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LIO-SAM on MCD NTU day-02 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lio_sam_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LIO-SAM on MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/lio_sam_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LiTAMIN2 throughput and accuracy trade-off on the second public HDL-400 reference window

- Current default: `fast_icp_only_half_threads`.
- Active challengers: `fast_cov_half_threads`.
- Reference variants: `paper_cov_half_threads`, `paper_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_icp_only_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LiTAMIN2 throughput and accuracy trade-off on the public HDL-400 reference window

- Current default: `paper_cov_half_threads`.
- Active challengers: `paper_icp_only_half_threads`.
- Reference variants: `fast_cov_half_threads`, `fast_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_cov_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| paper_icp_only_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## LiTAMIN2 throughput and accuracy trade-off on the second repository-stored Istanbul sequence

- Current default: `fast_icp_only_half_threads`.
- Active challengers: `fast_cov_half_threads`.
- Reference variants: `paper_cov_half_threads`, `paper_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_istanbul_window_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_icp_only_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LiTAMIN2 throughput and accuracy trade-off on the third repository-stored Istanbul sequence

- Current default: `paper_icp_only_half_threads`.
- Active challengers: `fast_cov_half_threads`, `fast_icp_only_half_threads`, `paper_cov_half_threads`.
- Aggregate result: `experiments/results/litamin2_istanbul_window_c_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_icp_only_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| paper_cov_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| paper_icp_only_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## LiTAMIN2 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast_icp_only_half_threads`.
- Reference variants: `fast_cov_half_threads`, `paper_cov_half_threads`, `paper_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_icp_only_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `paper_icp_only_half_threads`.
- Active challengers: `fast_cov_half_threads`, `fast_icp_only_half_threads`.
- Reference variants: `paper_cov_half_threads`.
- Aggregate result: `experiments/results/litamin2_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_icp_only_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## LiTAMIN2 trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `paper_cov_half_threads`.
- Active challengers: `fast_icp_only_half_threads`, `paper_icp_only_half_threads`.
- Reference variants: `fast_cov_half_threads`.
- Aggregate result: `experiments/results/litamin2_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_icp_only_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| paper_cov_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| paper_icp_only_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## LiTAMIN2 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast_icp_only_half_threads`.
- Active challengers: `fast_cov_half_threads`.
- Reference variants: `paper_cov_half_threads`, `paper_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_icp_only_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast_cov_half_threads`.
- Active challengers: `fast_icp_only_half_threads`.
- Reference variants: `paper_cov_half_threads`, `paper_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_icp_only_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LiTAMIN2 throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `fast_cov_half_threads`.
- Active challengers: `fast_icp_only_half_threads`.
- Reference variants: `paper_cov_half_threads`, `paper_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_icp_only_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LiTAMIN2 throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `paper_icp_only_half_threads`.
- Active challengers: `paper_cov_half_threads`.
- Reference variants: `fast_cov_half_threads`, `fast_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_cov_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| paper_icp_only_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## LiTAMIN2 throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `fast_cov_half_threads`.
- Active challengers: `fast_icp_only_half_threads`.
- Reference variants: `paper_cov_half_threads`, `paper_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_icp_only_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LiTAMIN2 throughput and accuracy trade-off on the repository-stored Istanbul sequence

- Current default: `fast_icp_only_half_threads`.
- Active challengers: `fast_cov_half_threads`.
- Reference variants: `paper_cov_half_threads`, `paper_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_profile_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_icp_only_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LOAM-Livox on the public HDL-400 reference window

- Current default: `default`.
- Active challengers: `dense`.
- Reference variants: `fast`.
- Aggregate result: `experiments/results/loam_livox_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## LOAM Livox on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/loam_livox_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LOAM Livox on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/loam_livox_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LOAM-Livox on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/loam_livox_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LOAM Livox on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/loam_livox_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LOAM Livox on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/loam_livox_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LOAM Livox on MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/loam_livox_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LOAM Livox on MCD NTU day-02 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/loam_livox_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LOAM Livox on MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/loam_livox_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## MULLS throughput and accuracy trade-off on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/mulls_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## MULLS trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/mulls_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## MULLS throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/mulls_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## MULLS throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/mulls_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## MULLS throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/mulls_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## MULLS throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/mulls_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## MULLS throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/mulls_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## MULLS throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `kitti_default`.
- Active challengers: `fast`.
- Reference variants: `dense`.
- Aggregate result: `experiments/results/mulls_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## MULLS throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `kitti_default`, `dense`.
- Aggregate result: `experiments/results/mulls_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| kitti_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## NDT throughput and accuracy trade-off on the second public HDL-400 reference window

- Current default: `fast_coarse_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/ndt_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_coarse_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## NDT throughput and accuracy trade-off on the public HDL-400 reference window

- Current default: `fast_coarse_map`.
- Active challengers: `balanced_local_map`.
- Reference variants: `dense_local_map`.
- Aggregate result: `experiments/results/ndt_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_coarse_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## NDT throughput and accuracy trade-off on the second repository-stored Istanbul sequence

- Current default: `fast_coarse_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/ndt_istanbul_window_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_coarse_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## NDT throughput and accuracy trade-off on the third repository-stored Istanbul sequence

- Current default: `fast_coarse_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/ndt_istanbul_window_c_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_coarse_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## NDT trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast_coarse_map`.
- Active challengers: `balanced_local_map`.
- Reference variants: `dense_local_map`.
- Aggregate result: `experiments/results/ndt_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_coarse_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## NDT throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast_coarse_map`.
- Active challengers: `balanced_local_map`.
- Reference variants: `dense_local_map`.
- Aggregate result: `experiments/results/ndt_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_coarse_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## NDT trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast_coarse_map`.
- Active challengers: `balanced_local_map`.
- Reference variants: `dense_local_map`.
- Aggregate result: `experiments/results/ndt_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_coarse_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## NDT trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast_coarse_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/ndt_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_coarse_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## NDT throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast_coarse_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/ndt_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_coarse_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## NDT throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `fast_coarse_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/ndt_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_coarse_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## NDT throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `balanced_local_map`.
- Active challengers: `fast_coarse_map`, `dense_local_map`.
- Aggregate result: `experiments/results/ndt_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_coarse_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## NDT throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `fast_coarse_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/ndt_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_coarse_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## NDT throughput and accuracy trade-off on the repository-stored Istanbul sequence

- Current default: `fast_coarse_map`.
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/ndt_profile_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_coarse_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Point-LIO on the public HDL-400 reference window

- Current default: `fast`.
- Active challengers: `dense`.
- Reference variants: `default`.
- Aggregate result: `experiments/results/point_lio_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## Point-LIO on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/point_lio_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Point-LIO on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/point_lio_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Point-LIO on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/point_lio_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Point-LIO on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/point_lio_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Point-LIO on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/point_lio_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Point-LIO on MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/point_lio_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Point-LIO on MCD NTU day-02 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/point_lio_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Point-LIO on MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/point_lio_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Small-GICP throughput and accuracy trade-off on the second public HDL-400 reference window

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/small_gicp_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Small-GICP throughput and accuracy trade-off on the public HDL-400 reference window

- Current default: `fast_recent_map`.
- Active challengers: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/small_gicp_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## Small-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/small_gicp_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Small-GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/small_gicp_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Small-GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `balanced_local_map`.
- Reference variants: `fast_recent_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/small_gicp_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Small-GICP throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/small_gicp_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Small-GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/small_gicp_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Small-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/small_gicp_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Small-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `dense_recent_map`.
- Reference variants: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/small_gicp_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## Small-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/small_gicp_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## SuMa on the second public HDL-400 reference window

- Current default: `dense`.
- Active challengers: `fast`.
- Reference variants: `default`.
- Aggregate result: `experiments/results/suma_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## SuMa on the public HDL-400 reference window

- Current default: `default`.
- Active challengers: `fast`, `dense`.
- Aggregate result: `experiments/results/suma_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## SuMa on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `dense`.
- Reference variants: `default`, `fast`.
- Aggregate result: `experiments/results/suma_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## SuMa on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `default`.
- Active challengers: `dense`.
- Reference variants: `fast`.
- Aggregate result: `experiments/results/suma_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## SuMa on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `default`.
- Reference variants: `fast`, `dense`.
- Aggregate result: `experiments/results/suma_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## SuMa on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/suma_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## SuMa on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `dense`.
- Reference variants: `default`, `fast`.
- Aggregate result: `experiments/results/suma_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## SuMa on MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/suma_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## SuMa on MCD NTU day-02 sequence

- Current default: `dense`.
- Reference variants: `default`, `fast`.
- Aggregate result: `experiments/results/suma_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## SuMa on MCD TUHH night-09 sequence

- Current default: `default`.
- Active challengers: `dense`.
- Reference variants: `fast`.
- Aggregate result: `experiments/results/suma_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## VGICP-SLAM on the public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/vgicp_slam_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## VGICP SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/vgicp_slam_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## VGICP SLAM on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/vgicp_slam_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## VGICP-SLAM on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Active challengers: `default`.
- Reference variants: `dense`.
- Aggregate result: `experiments/results/vgicp_slam_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## VGICP SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/vgicp_slam_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## VGICP SLAM on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/vgicp_slam_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## VGICP SLAM on MCD KTH day-06 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/vgicp_slam_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## VGICP SLAM on MCD NTU day-02 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/vgicp_slam_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## VGICP SLAM on MCD TUHH night-09 sequence

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/vgicp_slam_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## Voxel-GICP throughput and accuracy trade-off on the second public HDL-400 reference window

- Current default: `dense_recent_map`.
- Reference variants: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/voxel_gicp_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## Voxel-GICP throughput and accuracy trade-off on the public HDL-400 reference window

- Current default: `dense_recent_map`.
- Reference variants: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/voxel_gicp_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## Voxel-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `dense_recent_map`.
- Reference variants: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/voxel_gicp_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## Voxel-GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `dense_recent_map`.
- Reference variants: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/voxel_gicp_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## Voxel-GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `dense_recent_map`.
- Reference variants: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/voxel_gicp_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## Voxel-GICP throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `dense_recent_map`.
- Reference variants: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/voxel_gicp_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## Voxel-GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `dense_recent_map`.
- Reference variants: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/voxel_gicp_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## Voxel-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- Current default: `dense_recent_map`.
- Reference variants: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/voxel_gicp_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## Voxel-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- Current default: `dense_recent_map`.
- Reference variants: `balanced_local_map`, `fast_recent_map`.
- Aggregate result: `experiments/results/voxel_gicp_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## Voxel-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- Current default: `dense_recent_map`.
- Active challengers: `fast_recent_map`.
- Reference variants: `balanced_local_map`.
- Aggregate result: `experiments/results/voxel_gicp_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## X-ICP on the second public HDL-400 reference window

- Current default: `fast`.
- Reference variants: `default`, `dense`, `no_gt_seed`.
- Aggregate result: `experiments/results/xicp_hdl_400_reference_b_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## X-ICP on the public HDL-400 reference window

- Current default: `dense`.
- Reference variants: `default`, `fast`, `no_gt_seed`.
- Aggregate result: `experiments/results/xicp_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## X-ICP on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `dense`.
- Reference variants: `default`, `fast`, `no_gt_seed`.
- Aggregate result: `experiments/results/xicp_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## X-ICP on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `dense`.
- Active challengers: `default`.
- Reference variants: `fast`, `no_gt_seed`.
- Aggregate result: `experiments/results/xicp_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## X-ICP on KITTI Raw drive 0009 (200 frames, no GT seed)

- Current default: `fast`.
- Active challengers: `default`, `no_gt_seed`.
- Reference variants: `dense`.
- Aggregate result: `experiments/results/xicp_kitti_raw_0009_nogt_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| no_gt_seed | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## X-ICP on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Active challengers: `default`, `dense`.
- Reference variants: `no_gt_seed`.
- Aggregate result: `experiments/results/xicp_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## X-ICP on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Active challengers: `dense`.
- Reference variants: `default`, `no_gt_seed`.
- Aggregate result: `experiments/results/xicp_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## X-ICP on MCD KTH day-06 sequence

- Current default: `fast`.
- Active challengers: `default`.
- Reference variants: `dense`, `no_gt_seed`.
- Aggregate result: `experiments/results/xicp_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## X-ICP on MCD NTU day-02 sequence

- Current default: `dense`.
- Reference variants: `default`, `fast`, `no_gt_seed`.
- Aggregate result: `experiments/results/xicp_mcd_ntu_day_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## X-ICP on MCD TUHH night-09 sequence

- Current default: `dense`.
- Active challengers: `fast`.
- Reference variants: `default`, `no_gt_seed`.
- Aggregate result: `experiments/results/xicp_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| dense | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| no_gt_seed | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
