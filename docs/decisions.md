# Decisions

_Generated at 2026-04-06T06:33:08+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Rules

1. Keep at least three concrete variants alive for each active problem.
2. Promote a variant only when it is comparable on the same binary, dataset, and metrics.
3. Store the benchmark contract in the stable core and keep the profile search in `experiments/`.
4. Do not delete weaker variants unless they stop being informative.

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
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

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

- Current default: `paper_icp_only_half_threads`.
- Active challengers: `paper_cov_half_threads`.
- Reference variants: `fast_cov_half_threads`, `fast_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_cov_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| paper_icp_only_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

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
- Reference variants: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/ndt_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
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
