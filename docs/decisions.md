# Decisions

_Generated at 2026-05-19T19:58:14+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Rules

1. Keep at least three concrete variants alive for each active problem.
2. Promote a variant only when it is comparable on the same binary, dataset, and metrics.
3. Store the benchmark contract in the stable core and keep the profile search in `experiments/`.
4. Do not delete weaker variants unless they stop being informative.

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
- Active challengers: `balanced_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_window | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

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

## CT-ICP seq 00 full: fine corr_dist grid (5/6/7/8 m²)

- Current default: `corr_5`.
- Active challengers: `corr_8_reference`, `corr_6`, `corr_7`.
- Aggregate result: `experiments/results/ct_icp_kitti_corr_fine_grid_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| corr_8_reference | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| corr_5 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| corr_6 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| corr_7 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP seq 02 full: map=50 retrofit on bare baseline winner

- Current default: `baseline_map_20`.
- Reference variants: `baseline_map_50`.
- Aggregate result: `experiments/results/ct_icp_kitti_map50_retrofit_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| baseline_map_20 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| baseline_map_50 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

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

- Current default: `fast_window`.
- Active challengers: `balanced_window`.
- Reference variants: `dense_window`.
- Aggregate result: `experiments/results/ct_icp_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
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

## CT-ICP seq 00 full: corr_dist on bare baseline (no ms_chol, no c2f)

- Current default: `bare_corr_8`.
- Active challengers: `bare_corr_4`.
- Reference variants: `bare_corr_default`, `bare_corr_5`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_00_corr_bare_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| bare_corr_default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| bare_corr_8 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| bare_corr_5 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| bare_corr_4 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP cauchy_mult sweep on KITTI seq 00 full (4542 frames)

- Current default: `cauchy_2_0_reference`.
- Active challengers: `cauchy_2_5`, `cauchy_3_0`.
- Reference variants: `cauchy_1_5`, `cauchy_4_0`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_00_full_cauchy_sweep_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| cauchy_2_0_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| cauchy_1_5 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| cauchy_2_5 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| cauchy_3_0 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| cauchy_4_0 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 00 full: coarse_iterations sweep on map=50 winner

- Current default: `iter_2`.
- Active challengers: `iter_3_reference`, `iter_1`, `iter_4`, `iter_6`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_00_full_coarse_iter_sweep_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| iter_3_reference | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| iter_1 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| iter_2 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| iter_4 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| iter_6 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP seq 00 full: max_correspondence_distance sweep on iter=2 winner

- Current default: `corr_8`.
- Active challengers: `corr_default_reference`, `corr_16`.
- Reference variants: `corr_4`, `corr_2`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_00_full_corr_dist_sweep_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| corr_default_reference | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| corr_16 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| corr_8 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| corr_4 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| corr_2 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP fine-phase Cauchy σ sweep on KITTI seq 00 full (cluster A simplified)

- Current default: `fine_sigma_0_375`.
- Active challengers: `fine_sigma_default`, `fine_sigma_0_75`, `fine_sigma_1_0`.
- Reference variants: `fine_sigma_0_25`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_00_full_fine_cauchy_sweep_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fine_sigma_default | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fine_sigma_0_25 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fine_sigma_0_375 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fine_sigma_0_75 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fine_sigma_1_0 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP map_size sweep on KITTI seq 00 full (4542 frames)

- Current default: `map_30`.
- Active challengers: `map_20_reference`, `map_50`.
- Reference variants: `map_15`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_00_full_map_size_sweep_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| map_20_reference | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| map_15 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| map_30 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| map_50 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP throughput and accuracy trade-off on the full KITTI Odometry sequence 00

- Current default: `dense_window`.
- Active challengers: `balanced_window`.
- Reference variants: `fast_window`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_00_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| dense_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## CT-ICP seq 00 full: leave-one-out ablation on full recipe

- Current default: `full_recipe_reference`.
- Active challengers: `minus_c2f`, `minus_map_50`, `minus_corr_5`.
- Reference variants: `minus_ms_chol`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_00_leave_one_out_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| full_recipe_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| minus_ms_chol | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| minus_c2f | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| minus_map_50 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| minus_corr_5 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP seq 00 full: map=50 + single partner knob isolation

- Current default: `map_50_plus_c2f`.
- Active challengers: `map_50_plus_ms_chol`.
- Reference variants: `bare_map_50_reference`, `map_50_plus_corr_5`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_00_map50_partner_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| bare_map_50_reference | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| map_50_plus_ms_chol | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| map_50_plus_c2f | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| map_50_plus_corr_5 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 00 full: map_size on BARE baseline (recipe context dep?)

- Current default: `bare_map_30`.
- Active challengers: `bare_map_20`.
- Reference variants: `bare_map_50`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_00_map_bare_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| bare_map_20 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| bare_map_30 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| bare_map_50 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP throughput and accuracy trade-off on the KITTI Odometry sequence 00

- Current default: `fast_window`.
- Reference variants: `balanced_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_00_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 00 full: corr_dist sweep on simplified map=50+c2f winner

- Current default: `default_reference`.
- Active challengers: `plus_corr_8`.
- Reference variants: `plus_corr_5`, `plus_corr_16`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_00_simplified_corr_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| plus_corr_5 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| plus_corr_8 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| plus_corr_16 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 02 full: corr_dist sweep on bare baseline

- Current default: `corr_8`.
- Reference variants: `default_reference`, `corr_4`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_02_corr_dist_retrofit_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default_reference | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| corr_8 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| corr_4 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 02 full: fine corr_dist grid (5/6/7/8 m²)

- Current default: `corr_8_reference`.
- Reference variants: `corr_5`, `corr_6`, `corr_7`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_02_corr_fine_grid_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| corr_8_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| corr_5 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| corr_6 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| corr_7 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 02 full: corr_dist + ms_chol combinations

- Current default: `bare_corr_8_reference`.
- Reference variants: `ms_chol_corr_8`, `ms_chol_corr_5`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_02_corr_ms_chol_combo_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| bare_corr_8_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| ms_chol_corr_8 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| ms_chol_corr_5 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 02 full: c2f without ms_chol (probe whether ms_chol regression interacts with c2f)

- Current default: `baseline_reference`.
- Reference variants: `c2f_only_sigma`, `c2f_only_full`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_02_full_c2f_without_ms_chol_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| baseline_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| c2f_only_sigma | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| c2f_only_full | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 02 full: small map_size sweep (5/10/15/20)

- Current default: `map_15`.
- Active challengers: `map_20_reference`, `map_10`.
- Reference variants: `map_5`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_02_full_small_map_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| map_20_reference | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| map_15 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| map_10 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| map_5 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 02 full: map_size sweep on corr=8 winner

- Current default: `map_20_reference`.
- Active challengers: `map_15`.
- Reference variants: `map_30`, `map_50`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_02_map_with_corr_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| map_20_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| map_15 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| map_30 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| map_50 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 05 full: simplified recipes from bare + map=50

- Current default: `bare_map_50_corr_4`.
- Active challengers: `bare_map_50_reference`, `bare_map_50_iter_8`, `bare_map_50_voxel_05`.
- Reference variants: `arch_tuned_map_50`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_05_combo_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| bare_map_50_reference | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| bare_map_50_corr_4 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| bare_map_50_iter_8 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| bare_map_50_voxel_05 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| arch_tuned_map_50 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 05 full: corr_dist sweep on arch_tuned winner

- Current default: `corr_4_reference`.
- Active challengers: `corr_8`, `corr_16`, `corr_default`.
- Reference variants: `corr_2`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_05_corr_dist_sweep_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| corr_4_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| corr_2 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| corr_8 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| corr_16 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| corr_default | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP seq 05 full: map=50 retrofit on arch_tuned winner

- Current default: `arch_tuned_map_30`.
- Active challengers: `arch_tuned_map_50`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_05_map50_retrofit_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| arch_tuned_map_30 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| arch_tuned_map_50 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP seq 05 full: map_size on BARE baseline

- Current default: `bare_map_50`.
- Active challengers: `bare_map_20`, `bare_map_30`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_05_map_bare_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| bare_map_20 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| bare_map_30 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| bare_map_50 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## CT-ICP seq 07 full: ms_chol + simplified pattern combo

- Current default: `ms_chol_reference`.
- Active challengers: `ms_chol_plus_simplified_a`.
- Reference variants: `ms_chol_plus_c2f`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_07_combo_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| ms_chol_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| ms_chol_plus_c2f | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| ms_chol_plus_simplified_a | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP seq 07 full: corr_dist=8 m² retrofit on ms_chol winner

- Current default: `default_reference`.
- Reference variants: `corr_8`, `corr_4`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_07_corr_dist_retrofit_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| corr_8 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| corr_4 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP throughput and accuracy trade-off on the full KITTI Odometry sequence 07

- Current default: `fast_window`.
- Active challengers: `dense_window`.
- Reference variants: `balanced_window`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_07_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_window | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP seq 07 full: map=50 retrofit on ms_chol winner

- Current default: `ms_chol_map_20`.
- Reference variants: `ms_chol_map_50`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_07_map50_retrofit_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| ms_chol_map_20 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| ms_chol_map_50 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP throughput and accuracy trade-off on the KITTI Odometry sequence 07

- Current default: `fast_window`.
- Active challengers: `dense_window`.
- Reference variants: `balanced_window`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_07_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_window | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP seq 08 full: c2f without ms_chol — knob reduction probe

- Current default: `c2f_only_reference`.
- Active challengers: `bare_c2f`, `ms_chol_c2f_no_cholesky`, `cholesky_c2f_no_ms`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_08_c2f_alone_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| c2f_only_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| bare_c2f | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| ms_chol_c2f_no_cholesky | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| cholesky_c2f_no_ms | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP seq 08 full: corr_dist=8 m² retrofit on c2f_only winner

- Current default: `default_reference`.
- Active challengers: `corr_8`.
- Reference variants: `corr_4`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_08_corr_dist_retrofit_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| corr_8 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| corr_4 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP cauchy_mult sweep on KITTI seq 08 full (4071 frames)

- Current default: `cauchy_3_0`.
- Active challengers: `cauchy_2_5`, `cauchy_4_0`.
- Reference variants: `cauchy_2_0_reference`, `cauchy_1_5`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_08_full_cauchy_sweep_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| cauchy_2_0_reference | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| cauchy_1_5 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| cauchy_2_5 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| cauchy_3_0 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| cauchy_4_0 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP seq 08 full: coarse_iterations sweep on c2f_only winner

- Current default: `iter_6`.
- Active challengers: `iter_3_reference`, `iter_1`, `iter_2`.
- Reference variants: `iter_4`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_08_full_coarse_iter_sweep_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| iter_3_reference | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| iter_1 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| iter_2 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| iter_4 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| iter_6 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## CT-ICP seq 08 full: map=50 retrofit on c2f_only winner

- Current default: `c2f_only_map_20`.
- Active challengers: `c2f_only_map_50`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_08_map50_retrofit_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| c2f_only_map_20 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| c2f_only_map_50 | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## CT-ICP seq 08 full: corr_dist on simplified map=50+c2f base

- Current default: `default_reference`.
- Reference variants: `plus_corr_4`, `plus_corr_8`.
- Aggregate result: `experiments/results/ct_icp_kitti_seq_08_simplified_corr_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default_reference | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| plus_corr_4 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| plus_corr_8 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 02 full: simplified map=50+c2f probe

- Current default: `current_winner`.
- Reference variants: `simplified_seq_00_pattern`, `simplified_plus_corr_8`.
- Aggregate result: `experiments/results/ct_icp_kitti_simplified_cross_seq_02_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| current_winner | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| simplified_seq_00_pattern | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| simplified_plus_corr_8 | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 05 full: c2f σ×2 vs corr=4 on map=50 base

- Current default: `map_50_c2f_plus_corr_4`.
- Active challengers: `current_winner`.
- Reference variants: `map_50_c2f`.
- Aggregate result: `experiments/results/ct_icp_kitti_simplified_cross_seq_05_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| current_winner | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| map_50_c2f | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| map_50_c2f_plus_corr_4 | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

## CT-ICP seq 07 full: simplified map=50+c2f probe

- Current default: `current_winner`.
- Reference variants: `simplified_seq_00_pattern`.
- Aggregate result: `experiments/results/ct_icp_kitti_simplified_cross_seq_07_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| current_winner | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| simplified_seq_00_pattern | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP seq 08 full: simplified map=50+c2f probe

- Current default: `simplified_seq_00_pattern`.
- Active challengers: `current_winner`.
- Aggregate result: `experiments/results/ct_icp_kitti_simplified_cross_seq_08_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| current_winner | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| simplified_seq_00_pattern | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |

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

## CT-ICP throughput and drift trade-off on MulRan ParkingLot (120-frame window)

- Current default: `fast_window`.
- Reference variants: `balanced_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_mulran_parkinglot_120_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_window | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_window | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## CT-ICP throughput and drift trade-off on MulRan ParkingLot (full sequence)

- Current default: `fast_window`.
- Reference variants: `balanced_window`, `dense_window`.
- Aggregate result: `experiments/results/ct_icp_mulran_parkinglot_full_matrix.json`

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

## FAST-LIVO2 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_livo2_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIVO2 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_livo2_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIVO2 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_livo2_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## FAST-LIVO2 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/fast_livo2_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
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

## GICP throughput and accuracy trade-off on MulRan ParkingLot (120-frame window)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_mulran_parkinglot_120_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## GICP throughput and accuracy trade-off on MulRan ParkingLot (full sequence)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/gicp_mulran_parkinglot_full_matrix.json`

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

## KISS-ICP throughput and accuracy trade-off on MulRan ParkingLot (120-frame window)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/kiss_icp_mulran_parkinglot_120_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## KISS-ICP throughput and accuracy trade-off on MulRan ParkingLot (full sequence)

- Current default: `fast_recent_map`.
- Reference variants: `balanced_local_map`, `dense_recent_map`.
- Aggregate result: `experiments/results/kiss_icp_mulran_parkinglot_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_recent_map | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## KISS-ICP throughput and drift trade-off on the repository-stored Istanbul sequence

- Current default: `fast_recent_map`.
- Active challengers: `balanced_local_map`, `dense_local_map`.
- Aggregate result: `experiments/results/kiss_icp_profile_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| balanced_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_recent_map | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense_local_map | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

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

- Current default: `fast_cov_half_threads`.
- Active challengers: `fast_icp_only_half_threads`, `paper_icp_only_half_threads`.
- Reference variants: `paper_cov_half_threads`.
- Aggregate result: `experiments/results/litamin2_hdl_400_reference_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_icp_only_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
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

- Current default: `fast_cov_half_threads`.
- Active challengers: `fast_icp_only_half_threads`.
- Reference variants: `paper_cov_half_threads`, `paper_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_icp_only_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

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

- Current default: `fast_icp_only_half_threads`.
- Active challengers: `fast_cov_half_threads`.
- Reference variants: `paper_cov_half_threads`, `paper_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_mcd_kth_day_06_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |
| fast_icp_only_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
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

- Current default: `fast_icp_only_half_threads`.
- Reference variants: `fast_cov_half_threads`, `paper_cov_half_threads`, `paper_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_mcd_tuhh_night_09_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_icp_only_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## LiTAMIN2 throughput and accuracy trade-off on MulRan ParkingLot (120-frame window)

- Current default: `fast_cov_half_threads`.
- Active challengers: `paper_icp_only_half_threads`.
- Reference variants: `fast_icp_only_half_threads`, `paper_cov_half_threads`.
- Aggregate result: `experiments/results/litamin2_mulran_parkinglot_120_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| fast_icp_only_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| paper_icp_only_half_threads | Keep as active challenger | Close enough to the current default to keep as a live alternative. |

## LiTAMIN2 throughput and accuracy trade-off on MulRan ParkingLot (full sequence)

- Current default: `fast_icp_only_half_threads`.
- Reference variants: `fast_cov_half_threads`, `paper_cov_half_threads`, `paper_icp_only_half_threads`.
- Aggregate result: `experiments/results/litamin2_mulran_parkinglot_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| fast_cov_half_threads | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast_icp_only_half_threads | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
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

## LVI-SAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Timed out after 300 seconds.
- Next step: Run a lighter slice/profile, raise the timeout budget, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/lvi_sam_kitti_raw_0009_full_matrix.json`

## LVI-SAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Timed out after 120 seconds.
- Next step: Run a lighter slice/profile, raise the timeout budget, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/lvi_sam_kitti_raw_0009_matrix.json`

## LVI-SAM trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Timed out after 300 seconds.
- Next step: Run a lighter slice/profile, raise the timeout budget, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/lvi_sam_kitti_raw_0061_full_matrix.json`

## LVI-SAM throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Timed out after 120 seconds.
- Next step: Run a lighter slice/profile, raise the timeout budget, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/lvi_sam_kitti_raw_0061_matrix.json`

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

## OKVIS trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/okvis_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## OKVIS throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/okvis_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## OKVIS trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/okvis_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## OKVIS throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/okvis_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## ORB-SLAM3 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Timed out after 300 seconds.
- Next step: Run a lighter slice/profile, raise the timeout budget, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/orb_slam3_kitti_raw_0009_full_matrix.json`

## ORB-SLAM3 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Timed out after 120 seconds.
- Next step: Run a lighter slice/profile, raise the timeout budget, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/orb_slam3_kitti_raw_0009_matrix.json`

## ORB-SLAM3 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Timed out after 300 seconds.
- Next step: Run a lighter slice/profile, raise the timeout budget, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/orb_slam3_kitti_raw_0061_full_matrix.json`

## ORB-SLAM3 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Timed out after 120 seconds.
- Next step: Run a lighter slice/profile, raise the timeout budget, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/orb_slam3_kitti_raw_0061_matrix.json`

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

## R2LIVE trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Timed out after 300 seconds.
- Next step: Run a lighter slice/profile, raise the timeout budget, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/r2live_kitti_raw_0009_full_matrix.json`

## R2LIVE throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Timed out after 120 seconds.
- Next step: Run a lighter slice/profile, raise the timeout budget, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/r2live_kitti_raw_0009_matrix.json`

## R2LIVE trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Timed out after 300 seconds.
- Next step: Run a lighter slice/profile, raise the timeout budget, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/r2live_kitti_raw_0061_full_matrix.json`

## R2LIVE throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `-`.
- Status: `skipped`.
- Blocker: Timed out after 120 seconds.
- Next step: Run a lighter slice/profile, raise the timeout budget, or keep this problem out of the ready set until a real result is available.
- Aggregate result: `experiments/results/r2live_kitti_raw_0061_matrix.json`

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

## VINS-Fusion trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/vins_fusion_kitti_raw_0009_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## VINS-Fusion throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/vins_fusion_kitti_raw_0009_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## VINS-Fusion trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/vins_fusion_kitti_raw_0061_full_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

## VINS-Fusion throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- Current default: `fast`.
- Reference variants: `default`, `dense`.
- Aggregate result: `experiments/results/vins_fusion_kitti_raw_0061_matrix.json`

| Variant | Decision | Why |
|---------|----------|-----|
| default | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |
| fast | Adopt as current default | Best combined benchmark score on the shared dataset and interface. |
| dense | Keep as reference variant | Useful for comparison, but not strong enough to replace the current default. |

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
