# Paper Tracks

_Generated at 2026-10-10T21:33:33+00:00 by `evaluation/scripts/generate_publication_docs.py`._

This repository should not be pitched as "many implementations exist here".
The paper target has to be a claim about what this experiment-driven process reveals.

Current coverage: `425` ready, `1` blocked, `14` skipped problems.

## Current State

| Problem | Status | Default | Best ATE [m] | Best FPS | Dataset |
|---------|--------|---------|--------------|----------|---------|
| A-LOAM cluster discovery on KITTI Odom seq 07 full (1101 frames) | `ready` | `fast` | 2.544 | 1.7 | `dogfooding_results/kitti_seq_07_full` |
| A-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 3.199 | 4.2 | `dogfooding_results/kitti_raw_0009_200` |
| A-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 3.470 | 3.1 | `dogfooding_results/kitti_raw_0009_200` |
| A-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.459 | 6.0 | `dogfooding_results/kitti_raw_0061_200` |
| A-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 3.654 | 6.0 | `dogfooding_results/kitti_raw_0061_full` |
| A-LOAM throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast` | 6.077 | 6.7 | `dogfooding_results/mcd_kth_day_06_108` |
| A-LOAM throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense` | 0.035 | 5.8 | `dogfooding_results/mcd_ntu_day_02_108` |
| A-LOAM throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast` | 1.336 | 6.5 | `dogfooding_results/mcd_tuhh_night_09_108` |
| A-LOAM throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast` | 0.146 | 13.8 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| A-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 6.105 | 5.8 | `dogfooding_results/kitti_raw_0009_full` |
| A-LOAM transfer check on KITTI Odom seq 00 full (4541 frames) | `ready` | `fast` | 12.044 | 3.0 | `dogfooding_results/kitti_seq_00_full` |
| A-LOAM transfer check on KITTI Odom seq 02 full (4661 frames) | `ready` | `fast` | 50.114 | 3.2 | `dogfooding_results/kitti_seq_02_full` |
| A-LOAM transfer check on KITTI Odom seq 05 full (2761 frames) | `ready` | `fast` | 4.840 | 3.0 | `dogfooding_results/kitti_seq_05_full` |
| A-LOAM transfer check on KITTI Odom seq 08 full (4071 frames) | `ready` | `fast` | 18.110 | 2.9 | `dogfooding_results/kitti_seq_08_full` |
| BALM2 on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.405 | 6.3 | `dogfooding_results/kitti_raw_0009_200` |
| BALM2 on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.405 | 4.4 | `dogfooding_results/kitti_raw_0009_200` |
| BALM2 on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 3.338 | 12.7 | `dogfooding_results/kitti_raw_0009_full` |
| BALM2 on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 1.883 | 13.2 | `dogfooding_results/kitti_raw_0061_200` |
| BALM2 on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 9.533 | 11.4 | `dogfooding_results/kitti_raw_0061_full` |
| BALM2 on MCD KTH day-06 sequence | `ready` | `fast` | 6.184 | 13.4 | `dogfooding_results/mcd_kth_day_06_108` |
| BALM2 on MCD NTU day-02 sequence | `ready` | `fast` | 0.062 | 12.7 | `dogfooding_results/mcd_ntu_day_02_108` |
| BALM2 on MCD TUHH night-09 sequence | `ready` | `fast` | 1.270 | 14.6 | `dogfooding_results/mcd_tuhh_night_09_108` |
| BALM2 on the public HDL-400 reference window | `ready` | `fast` | 0.476 | 9.0 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| CLINS trade-off on the public ROS1 HDL-400 window with synthesized per-point time | `ready` | `dense` | 1.473 | 100.3 | `dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index` |
| CT-ICP cauchy_mult sweep on KITTI seq 00 full (4542 frames) | `ready` | `cauchy_4_0` | 18.226 | 10.6 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP cauchy_mult sweep on KITTI seq 08 full (4071 frames) | `ready` | `cauchy_2_5` | 31.958 | 11.9 | `dogfooding_results/kitti_seq_08_full` |
| CT-ICP cluster A + GT seed on KITTI Odometry seq 00 full | `ready` | `cluster_a_seeded` | 5.855 | 6.9 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP cluster A + GT seed on KITTI Odometry seq 08 full | `ready` | `cluster_a_seeded` | 6.813 | 6.8 | `dogfooding_results/kitti_seq_08_full` |
| CT-ICP cluster A simplified recipe transfer to MulRan parkinglot full | `ready` | `cluster_a_with_seed` | 9.186 | 14.6 | `dogfooding_results/mulran_parkinglot_full` |
| CT-ICP cluster A vs D + GT seed on KITTI Odometry seq 07 full (corrected cluster D) | `ready` | `cluster_d_full_seeded` | 1.603 | 12.5 | `dogfooding_results/kitti_seq_07_full` |
| CT-ICP cluster A/D + GT seed on MCD KTH day_06 (108 frames) | `ready` | `dense_seeded_reference` | 2.401 | 28.3 | `dogfooding_results/mcd_kth_day_06_108` |
| CT-ICP cluster A/D + GT seed on MCD NTU day_02 (108 frames) | `ready` | `dense_seeded_reference` | 0.425 | 28.4 | `dogfooding_results/mcd_ntu_day_02_108` |
| CT-ICP cluster A/D + GT seed on MCD TUHH night_09 (108 frames) | `ready` | `dense_seeded_reference` | 0.832 | 27.4 | `dogfooding_results/mcd_tuhh_night_09_108` |
| CT-ICP cluster A/D bake-off on KITTI Raw 0009 200-frame short window | `ready` | `cluster_d_ms_chol` | 2.579 | 7.2 | `dogfooding_results/kitti_raw_0009_200` |
| CT-ICP cluster A/D bake-off on KITTI Raw 0009 full (447 frames) | `ready` | `balanced_reference` | 4.105 | 7.4 | `dogfooding_results/kitti_raw_0009_full` |
| CT-ICP cluster A/D bake-off on KITTI Raw 0061 200-frame short window | `ready` | `fast_reference` | 0.944 | 59.4 | `dogfooding_results/kitti_raw_0061_200` |
| CT-ICP cluster A/D bake-off on KITTI Raw 0061 full (707 frames) | `ready` | `fast_window_reference` | 4.501 | 57.8 | `dogfooding_results/kitti_raw_0061_full` |
| CT-ICP cluster A/D transfer to MulRan parkinglot 120-frame short window | `ready` | `cluster_a_with_seed` | 2.547 | 27.0 | `dogfooding_results/mulran_parkinglot_120` |
| CT-ICP coarse_search_radius sweep on KITTI seq 00 full (cluster A) | `ready` | `radius_1` | 14.817 | 10.2 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP fine-phase Cauchy σ sweep on KITTI seq 00 full (cluster A simplified) | `ready` | `fine_sigma_0_25` | 12.351 | 11.4 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP KITTI Odom seq 00 full: elevation correction | `ready` | `arch_tuned_all_combined` | 93.156 | 16.6 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP KITTI Odom seq 07 full: elevation correction | `ready` | `velocity_reg_01` | 2.231 | 18.5 | `dogfooding_results/kitti_seq_07_full` |
| CT-ICP map_size sweep on KITTI seq 00 full (4542 frames) | `ready` | `map_20_reference` | 18.370 | 10.4 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP performance-priority trade-off on the public ROS1 HDL-400 window with synthesized per-point time | `ready` | `dense_window` | 1.254 | 65.9 | `dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index` |
| CT-ICP recipe sensitivity test on MCD NTU day_02 (108 frames) | `ready` | `dense_reference` | 0.325 | 23.9 | `dogfooding_results/mcd_ntu_day_02_108` |
| CT-ICP recipe sensitivity test on MCD TUHH night_09 (108 frames) | `ready` | `dense_reference` | 1.534 | 22.8 | `dogfooding_results/mcd_tuhh_night_09_108` |
| CT-ICP recipe transfer test on MCD KTH day_06 (108 frames) | `ready` | `dense_reference` | 6.107 | 18.0 | `dogfooding_results/mcd_kth_day_06_108` |
| CT-ICP seq 00 full: coarse_iterations sweep on map=50 winner | `ready` | `iter_1` | 13.766 | 9.8 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP seq 00 full: constant-velocity regularization (small weight) | `ready` | `c2f_reference` | 14.099 | 8.7 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP seq 00 full: corr_dist on bare baseline (no ms_chol, no c2f) | `ready` | `bare_corr_8` | 14.403 | 13.8 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP seq 00 full: corr_dist sweep on simplified map=50+c2f winner | `ready` | `plus_corr_5` | 12.931 | 11.4 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP seq 00 full: fine corr_dist grid (5/6/7/8 m²) | `ready` | `corr_5` | 12.931 | 9.8 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP seq 00 full: leave-one-out ablation on full recipe | `ready` | `minus_ms_chol` | 12.931 | 11.5 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP seq 00 full: map=50 + single partner knob isolation | `ready` | `map_50_plus_corr_5` | 14.894 | 12.5 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP seq 00 full: map_size on BARE baseline (recipe context dep?) | `ready` | `bare_map_50` | 15.753 | 13.7 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP seq 00 full: max_correspondence_distance sweep on iter=2 winner | `ready` | `corr_8` | 16.687 | 10.3 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP seq 02 full: c2f without ms_chol (probe whether ms_chol regression interacts with c2f) | `ready` | `baseline_reference` | 66.553 | 10.7 | `dogfooding_results/kitti_seq_02_full` |
| CT-ICP seq 02 full: corr_dist + ms_chol combinations | `ready` | `ms_chol_corr_5` | 65.305 | 10.8 | `dogfooding_results/kitti_seq_02_full` |
| CT-ICP seq 02 full: corr_dist sweep on bare baseline | `ready` | `default_reference` | 68.972 | 12.5 | `dogfooding_results/kitti_seq_02_full` |
| CT-ICP seq 02 full: fine corr_dist grid (5/6/7/8 m²) | `ready` | `corr_5` | 65.305 | 10.4 | `dogfooding_results/kitti_seq_02_full` |
| CT-ICP seq 02 full: map=50 retrofit on bare baseline winner | `ready` | `baseline_map_20` | 68.972 | 8.4 | `dogfooding_results/kitti_seq_02_full` |
| CT-ICP seq 02 full: map_size sweep on corr=8 winner | `ready` | `map_15` | 69.536 | 11.7 | `dogfooding_results/kitti_seq_02_full` |
| CT-ICP seq 02 full: simplified map=50+c2f probe | `ready` | `current_winner` | 92.116 | 13.0 | `dogfooding_results/kitti_seq_02_full` |
| CT-ICP seq 02 full: small map_size sweep (5/10/15/20) | `ready` | `map_15` | 68.972 | 5.7 | `dogfooding_results/kitti_seq_02_full` |
| CT-ICP seq 05 full: c2f σ×2 vs corr=4 on map=50 base | `ready` | `map_50_c2f_plus_corr_4` | 9.305 | 9.1 | `dogfooding_results/kitti_seq_05_full` |
| CT-ICP seq 05 full: constant-velocity regularization sweep | `ready` | `velocity_reg_005` | 11.158 | 5.1 | `dogfooding_results/kitti_seq_05_full` |
| CT-ICP seq 05 full: corr_dist sweep on arch_tuned winner | `ready` | `corr_4_reference` | 9.485 | 12.8 | `dogfooding_results/kitti_seq_05_full` |
| CT-ICP seq 05 full: map=50 retrofit on arch_tuned winner | `ready` | `arch_tuned_map_50` | 8.844 | 12.0 | `dogfooding_results/kitti_seq_05_full` |
| CT-ICP seq 05 full: map_size on BARE baseline | `ready` | `bare_map_30` | 11.707 | 7.0 | `dogfooding_results/kitti_seq_05_full` |
| CT-ICP seq 05 full: simplified recipes from bare + map=50 | `ready` | `arch_tuned_map_50` | 8.844 | 12.0 | `dogfooding_results/kitti_seq_05_full` |
| CT-ICP seq 07 full: constant-velocity regularization sweep | `ready` | `velocity_reg_01` | 2.490 | 6.0 | `dogfooding_results/kitti_seq_07_full` |
| CT-ICP seq 07 full: corr_dist=8 m² retrofit on ms_chol winner | `ready` | `corr_8` | 2.049 | 14.3 | `dogfooding_results/kitti_seq_07_full` |
| CT-ICP seq 07 full: map=50 retrofit on ms_chol winner | `ready` | `ms_chol_map_50` | 1.472 | 14.2 | `dogfooding_results/kitti_seq_07_full` |
| CT-ICP seq 07 full: ms_chol + simplified pattern combo | `ready` | `ms_chol_plus_simplified_a` | 2.010 | 10.6 | `dogfooding_results/kitti_seq_07_full` |
| CT-ICP seq 07 full: simplified map=50+c2f probe | `ready` | `simplified_seq_00_pattern` | 2.010 | 14.1 | `dogfooding_results/kitti_seq_07_full` |
| CT-ICP seq 08 full: c2f without ms_chol — knob reduction probe | `ready` | `cholesky_c2f_no_ms` | 37.043 | 12.2 | `dogfooding_results/kitti_seq_08_full` |
| CT-ICP seq 08 full: coarse_iterations sweep on c2f_only winner | `ready` | `iter_6` | 30.418 | 10.4 | `dogfooding_results/kitti_seq_08_full` |
| CT-ICP seq 08 full: constant-velocity regularization (small weight) | `ready` | `velocity_reg_001` | 38.901 | 9.4 | `dogfooding_results/kitti_seq_08_full` |
| CT-ICP seq 08 full: corr_dist on simplified map=50+c2f base | `ready` | `default_reference` | 28.427 | 11.3 | `dogfooding_results/kitti_seq_08_full` |
| CT-ICP seq 08 full: corr_dist=8 m² retrofit on c2f_only winner | `ready` | `corr_8` | 33.860 | 10.2 | `dogfooding_results/kitti_seq_08_full` |
| CT-ICP seq 08 full: map=50 retrofit on c2f_only winner | `ready` | `c2f_only_map_20` | 30.497 | 8.6 | `dogfooding_results/kitti_seq_08_full` |
| CT-ICP seq 08 full: simplified map=50+c2f probe | `ready` | `simplified_seq_00_pattern` | 28.427 | 8.5 | `dogfooding_results/kitti_seq_08_full` |
| CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_window` | 2.579 | 16.9 | `dogfooding_results/kitti_raw_0009_200` |
| CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_window` | 1.475 | 56.9 | `dogfooding_results/kitti_raw_0061_200` |
| CT-ICP throughput and accuracy trade-off on the full KITTI Odometry sequence 00 | `ready` | `balanced_window` | 19.413 | 46.4 | `dogfooding_results/kitti_seq_00_full` |
| CT-ICP throughput and accuracy trade-off on the full KITTI Odometry sequence 07 | `ready` | `dense_window` | 2.842 | 52.2 | `dogfooding_results/kitti_seq_07_full` |
| CT-ICP throughput and accuracy trade-off on the KITTI Odometry sequence 00 | `ready` | `fast_window` | 1.851 | 74.9 | `dogfooding_results/kitti_seq_00_108` |
| CT-ICP throughput and accuracy trade-off on the KITTI Odometry sequence 07 | `ready` | `fast_window` | 0.390 | 77.3 | `dogfooding_results/kitti_seq_07_108` |
| CT-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_window` | 6.115 | 57.2 | `dogfooding_results/mcd_kth_day_06_108` |
| CT-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_window` | 0.325 | 60.0 | `dogfooding_results/mcd_ntu_day_02_108` |
| CT-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_window` | 1.652 | 51.4 | `dogfooding_results/mcd_tuhh_night_09_108` |
| CT-ICP throughput and drift trade-off on MulRan ParkingLot (120-frame window) | `ready` | `fast_window` | 15.873 | 74.7 | `dogfooding_results/mulran_parkinglot_120` |
| CT-ICP throughput and drift trade-off on MulRan ParkingLot (full sequence) | `ready` | `fast_window` | 74.578 | 59.7 | `dogfooding_results/mulran_parkinglot_full` |
| CT-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `fast_window` | 1.251 | 54.9 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| CT-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `fast_window` | 75.075 | 2.7 | `dogfooding_results/autoware_istanbul_open_108` |
| CT-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `fast_window` | 0.556 | 2.4 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| CT-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `balanced_window` | 6.820 | 3.1 | `dogfooding_results/autoware_istanbul_open_108_b` |
| CT-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `balanced_window` | 7.539 | 2.8 | `dogfooding_results/autoware_istanbul_open_108_c` |
| CT-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `balanced_window` | 1.659 | 71.4 | `dogfooding_results/kitti_raw_0009_200` |
| CT-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_window` | 3.871 | 12.7 | `dogfooding_results/kitti_raw_0009_full` |
| CT-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_window` | 6.972 | 37.6 | `dogfooding_results/kitti_raw_0061_full` |
| CT-LIO GT-backed public benchmark readiness on HDL-400 ROS2 data | `blocked` | `-` | - | - | `dogfooding_results/hdl_400_open_ct_lio_60` |
| CT-LIO reference-trajectory trade-off on the public HDL-400 120-frame window | `ready` | `seed_only_fast` | 0.488 | 26.0 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| CT-LIO trade-off on the public ROS1 HDL-400 window with synthesized per-point time | `ready` | `seed_only_fast` | 0.479 | 25.8 | `dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index` |
| DLIO throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.362 | 7.0 | `dogfooding_results/kitti_raw_0009_200` |
| DLIO throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.362 | 5.3 | `dogfooding_results/kitti_raw_0009_200` |
| DLIO throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.798 | 8.5 | `dogfooding_results/kitti_raw_0061_200` |
| DLIO throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 7.349 | 10.7 | `dogfooding_results/kitti_raw_0061_full` |
| DLIO throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast` | 6.070 | 10.4 | `dogfooding_results/mcd_kth_day_06_108` |
| DLIO throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `kitti_default` | 0.016 | 16.6 | `dogfooding_results/mcd_ntu_day_02_108` |
| DLIO throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast` | 1.340 | 13.0 | `dogfooding_results/mcd_tuhh_night_09_108` |
| DLIO throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast` | 0.239 | 12.1 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| DLIO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.026 | 7.3 | `dogfooding_results/kitti_raw_0009_full` |
| DLO throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.362 | 7.1 | `dogfooding_results/kitti_raw_0009_200` |
| DLO throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.362 | 5.9 | `dogfooding_results/kitti_raw_0009_200` |
| DLO throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.798 | 8.5 | `dogfooding_results/kitti_raw_0061_200` |
| DLO throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 7.349 | 10.6 | `dogfooding_results/kitti_raw_0061_full` |
| DLO throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast` | 6.070 | 10.7 | `dogfooding_results/mcd_kth_day_06_108` |
| DLO throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `kitti_default` | 0.016 | 16.6 | `dogfooding_results/mcd_ntu_day_02_108` |
| DLO throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast` | 1.340 | 13.9 | `dogfooding_results/mcd_tuhh_night_09_108` |
| DLO throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast` | 0.101 | 15.0 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| DLO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.026 | 7.3 | `dogfooding_results/kitti_raw_0009_full` |
| F-LOAM cluster discovery on KITTI Odom seq 07 full (1102 frames) | `ready` | `fast` | 2.972 | 8.7 | `dogfooding_results/kitti_seq_07_full` |
| F-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.883 | 28.0 | `dogfooding_results/kitti_raw_0009_200` |
| F-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.883 | 24.6 | `dogfooding_results/kitti_raw_0009_200` |
| F-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.587 | 25.9 | `dogfooding_results/kitti_raw_0061_200` |
| F-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 3.822 | 30.7 | `dogfooding_results/kitti_raw_0061_full` |
| F-LOAM throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast` | 6.005 | 31.1 | `dogfooding_results/mcd_kth_day_06_108` |
| F-LOAM throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `fast` | 0.111 | 27.0 | `dogfooding_results/mcd_ntu_day_02_108` |
| F-LOAM throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast` | 1.345 | 27.6 | `dogfooding_results/mcd_tuhh_night_09_108` |
| F-LOAM throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast` | 0.193 | 64.2 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| F-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.452 | 28.6 | `dogfooding_results/kitti_raw_0009_full` |
| F-LOAM transfer check on KITTI Odom seq 00 full (4541 frames) | `ready` | `kitti_default` | 9.834 | 6.5 | `dogfooding_results/kitti_seq_00_full` |
| F-LOAM transfer check on KITTI Odom seq 02 full (4661 frames) | `ready` | `kitti_default` | 52.068 | 5.9 | `dogfooding_results/kitti_seq_02_full` |
| F-LOAM transfer check on KITTI Odom seq 05 full (2761 frames) | `ready` | `kitti_default` | 6.035 | 2.0 | `dogfooding_results/kitti_seq_05_full` |
| F-LOAM transfer check on KITTI Odom seq 08 full (4071 frames) | `ready` | `kitti_default` | 16.516 | 6.0 | `dogfooding_results/kitti_seq_08_full` |
| FAST-LIO-SLAM on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.382 | 11.3 | `dogfooding_results/kitti_raw_0009_200` |
| FAST-LIO-SLAM on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.382 | 8.8 | `dogfooding_results/kitti_raw_0009_200` |
| FAST-LIO-SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.289 | 11.3 | `dogfooding_results/kitti_raw_0009_full` |
| FAST-LIO-SLAM on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.660 | 11.2 | `dogfooding_results/kitti_raw_0061_200` |
| FAST-LIO-SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 4.945 | 9.1 | `dogfooding_results/kitti_raw_0061_full` |
| FAST-LIO-SLAM on MCD KTH day-06 sequence | `ready` | `fast` | 6.075 | 9.4 | `dogfooding_results/mcd_kth_day_06_108` |
| FAST-LIO-SLAM on MCD NTU day-02 sequence | `ready` | `fast` | 0.025 | 20.4 | `dogfooding_results/mcd_ntu_day_02_108` |
| FAST-LIO-SLAM on MCD TUHH night-09 sequence | `ready` | `fast` | 1.330 | 12.8 | `dogfooding_results/mcd_tuhh_night_09_108` |
| FAST-LIO-SLAM on the public HDL-400 reference window | `ready` | `fast` | 0.109 | 8.4 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| FAST-LIO2 on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.328 | 12.7 | `dogfooding_results/kitti_raw_0009_200` |
| FAST-LIO2 on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.328 | 12.3 | `dogfooding_results/kitti_raw_0009_200` |
| FAST-LIO2 on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.199 | 13.5 | `dogfooding_results/kitti_raw_0009_full` |
| FAST-LIO2 on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.634 | 13.6 | `dogfooding_results/kitti_raw_0061_200` |
| FAST-LIO2 on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 5.066 | 13.5 | `dogfooding_results/kitti_raw_0061_full` |
| FAST-LIO2 on MCD KTH day-06 sequence | `ready` | `fast` | 6.072 | 12.4 | `dogfooding_results/mcd_kth_day_06_108` |
| FAST-LIO2 on MCD NTU day-02 sequence | `ready` | `fast` | 0.025 | 23.7 | `dogfooding_results/mcd_ntu_day_02_108` |
| FAST-LIO2 on MCD TUHH night-09 sequence | `ready` | `fast` | 1.333 | 14.2 | `dogfooding_results/mcd_tuhh_night_09_108` |
| FAST-LIO2 on the public HDL-400 reference window | `ready` | `fast` | 0.104 | 9.1 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| FAST-LIVO2 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 31.298 | 8.3 | `dogfooding_results/kitti_raw_0009_200` |
| FAST-LIVO2 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 27.581 | 7.2 | `dogfooding_results/kitti_raw_0061_200` |
| FAST-LIVO2 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 49.209 | 7.5 | `dogfooding_results/kitti_raw_0009_full` |
| FAST-LIVO2 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 96.345 | 14.0 | `dogfooding_results/kitti_raw_0061_full` |
| GenZ-ICP profile trade-off on the NCLT 2013-01-10 window | `ready` | `default` | 3.513 | 11.0 | `dogfooding_results/nclt_2013_01_10_120` |
| GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_recent_map` | 1.151 | 11.8 | `dogfooding_results/kitti_raw_0009_200` |
| GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_recent_map` | 0.781 | 25.7 | `dogfooding_results/kitti_raw_0061_200` |
| GICP throughput and accuracy trade-off on MulRan ParkingLot (120-frame window) | `ready` | `fast_recent_map` | 0.644 | 27.7 | `dogfooding_results/mulran_parkinglot_120` |
| GICP throughput and accuracy trade-off on MulRan ParkingLot (full sequence) | `ready` | `fast_recent_map` | 0.944 | 30.3 | `dogfooding_results/mulran_parkinglot_full` |
| GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 0.630 | 24.7 | `dogfooding_results/mcd_kth_day_06_108` |
| GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.017 | 28.7 | `dogfooding_results/mcd_ntu_day_02_108` |
| GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 0.317 | 31.2 | `dogfooding_results/mcd_tuhh_night_09_108` |
| GICP throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 0.101 | 23.3 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| GICP throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 0.994 | 6.3 | `dogfooding_results/autoware_istanbul_open_108` |
| GICP throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_recent_map` | 0.135 | 1.7 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| GICP throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 1.166 | 5.7 | `dogfooding_results/autoware_istanbul_open_108_b` |
| GICP throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 0.982 | 4.3 | `dogfooding_results/autoware_istanbul_open_108_c` |
| GICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_recent_map` | 1.446 | 8.6 | `dogfooding_results/kitti_raw_0009_200` |
| GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 1.129 | 8.4 | `dogfooding_results/kitti_raw_0009_full` |
| GICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_recent_map` | 1.055 | 22.8 | `dogfooding_results/kitti_raw_0061_full` |
| HDL Graph SLAM on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.878 | 15.4 | `dogfooding_results/kitti_raw_0009_200` |
| HDL Graph SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `default` | 185.826 | 0.2 | `dogfooding_results/kitti_raw_0009_full` |
| HDL Graph SLAM on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 5.178 | 19.2 | `dogfooding_results/kitti_raw_0061_200` |
| HDL Graph SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0061_full` |
| HDL Graph SLAM on MCD KTH day-06 sequence | `ready` | `fast` | 5.057 | 13.9 | `dogfooding_results/mcd_kth_day_06_108` |
| HDL Graph SLAM on MCD NTU day-02 sequence | `ready` | `dense` | 0.180 | 21.9 | `dogfooding_results/mcd_ntu_day_02_108` |
| HDL Graph SLAM on MCD TUHH night-09 sequence | `ready` | `dense` | 1.373 | 14.5 | `dogfooding_results/mcd_tuhh_night_09_108` |
| HDL-Graph-SLAM on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `default` | 122.141 | 1.9 | `dogfooding_results/kitti_raw_0009_200` |
| HDL-Graph-SLAM on the public HDL-400 reference window | `ready` | `fast` | 13.820 | 6.7 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| IMU-DR (pure strapdown INS) aiding ablation on KITTI Raw drive 2011_09_26_0009 (200-frame window, OXTS) | `ready` | `zupt_kitti_0009` | 201.185 | 1988293.4 | `dogfooding_results/kitti_raw_0009_200` |
| IMU-DR (pure strapdown INS) aiding ablation on KITTI Raw drive 2011_09_26_0009 full sequence (443 frames, OXTS) | `ready` | `nhc_zupt_kitti_0009_full` | 1067.301 | 2692976.4 | `dogfooding_results/kitti_raw_0009_full` |
| IMU-DR (pure strapdown INS) aiding ablation on the NCLT 2013-01-10 120-frame window | `ready` | `zupt` | 2.887 | 1525979.8 | `dogfooding_results/nclt_2013_01_10_120` |
| IMU-DR (pure strapdown INS) aiding ablation on the NCLT 2013-01-10 full session (5105 frames) | `ready` | `nhc_zupt_full` | 9605.455 | 1354057.2 | `/media/sasaki/aiueo/loc_zoo/dogfooding_results/nclt_2013_01_10_full` |
| ISC-LOAM on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.321 | 37.6 | `dogfooding_results/kitti_raw_0009_200` |
| ISC-LOAM on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.321 | 35.6 | `dogfooding_results/kitti_raw_0009_200` |
| ISC-LOAM on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 4.323 | 30.5 | `dogfooding_results/kitti_raw_0009_full` |
| ISC-LOAM on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `dense` | 0.494 | 43.5 | `dogfooding_results/kitti_raw_0061_200` |
| ISC-LOAM on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 4.854 | 33.5 | `dogfooding_results/kitti_raw_0061_full` |
| ISC-LOAM on MCD KTH day-06 sequence | `ready` | `fast` | 6.023 | 48.6 | `dogfooding_results/mcd_kth_day_06_108` |
| ISC-LOAM on MCD NTU day-02 sequence | `ready` | `fast` | 0.065 | 50.2 | `dogfooding_results/mcd_ntu_day_02_108` |
| ISC-LOAM on MCD TUHH night-09 sequence | `ready` | `fast` | 1.337 | 53.2 | `dogfooding_results/mcd_tuhh_night_09_108` |
| ISC-LOAM on the public HDL-400 reference window | `ready` | `fast` | 0.161 | 37.0 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| KISS-ICP cluster discovery on KITTI Odom seq 00 full (4542 frames) | `ready` | `dense_profile` | 12.323 | 1.7 | `dogfooding_results/kitti_seq_00_full` |
| KISS-ICP cluster discovery on KITTI Odom seq 02 full (4661 frames) | `ready` | `balanced_reference` | 56.234 | 3.4 | `dogfooding_results/kitti_seq_02_full` |
| KISS-ICP cluster discovery on KITTI Odom seq 05 full (2761 frames) | `ready` | `dense_profile` | 4.556 | 2.1 | `dogfooding_results/kitti_seq_05_full` |
| KISS-ICP cluster discovery on KITTI Odom seq 07 full (1102 frames) | `ready` | `balanced_reference` | 2.238 | 3.4 | `dogfooding_results/kitti_seq_07_full` |
| KISS-ICP cluster discovery on KITTI Odom seq 08 full (4071 frames) | `ready` | `fast_profile` | 17.322 | 2.5 | `dogfooding_results/kitti_seq_08_full` |
| KISS-ICP KITTI Odom seq 00 full: elevation correction and upstream configuration | `ready` | `upstream_profile_elevation` | 9.037 | 32.5 | `dogfooding_results/kitti_seq_00_full` |
| KISS-ICP KITTI Odom seq 07 full: elevation correction and upstream configuration | `ready` | `balanced_reference` | 1.390 | 28.3 | `dogfooding_results/kitti_seq_07_full` |
| KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_recent_map` | 2.360 | 18.7 | `dogfooding_results/kitti_raw_0009_200` |
| KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_recent_map` | 0.679 | 28.3 | `dogfooding_results/kitti_raw_0061_200` |
| KISS-ICP throughput and accuracy trade-off on MulRan ParkingLot (120-frame window) | `ready` | `fast_recent_map` | 15.641 | 27.3 | `dogfooding_results/mulran_parkinglot_120` |
| KISS-ICP throughput and accuracy trade-off on MulRan ParkingLot (full sequence) | `ready` | `fast_recent_map` | 73.621 | 26.9 | `dogfooding_results/mulran_parkinglot_full` |
| KISS-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 5.568 | 11.3 | `dogfooding_results/mcd_kth_day_06_108` |
| KISS-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `fast_recent_map` | 0.017 | 66.7 | `dogfooding_results/mcd_ntu_day_02_108` |
| KISS-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 1.104 | 24.1 | `dogfooding_results/mcd_tuhh_night_09_108` |
| KISS-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 1.281 | 11.3 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| KISS-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 182.960 | 4.0 | `dogfooding_results/autoware_istanbul_open_108` |
| KISS-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `fast_recent_map` | 0.218 | 0.4 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| KISS-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `dense_local_map` | 143.921 | 3.6 | `dogfooding_results/autoware_istanbul_open_108_b` |
| KISS-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 131.691 | 3.7 | `dogfooding_results/autoware_istanbul_open_108_c` |
| KISS-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_recent_map` | 2.360 | 18.2 | `dogfooding_results/kitti_raw_0009_200` |
| KISS-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 3.721 | 10.8 | `dogfooding_results/kitti_raw_0009_full` |
| KISS-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_recent_map` | 4.343 | 11.2 | `dogfooding_results/kitti_raw_0061_full` |
| L-LO on KITTI Odom seq 00 full (4541 frames) | `ready` | `seq07_tuned` | 15.664 | 4.4 | `dogfooding_results/kitti_seq_00_full` |
| L-LO on KITTI Odom seq 02 full (4661 frames) | `ready` | `default` | 58.272 | 3.6 | `dogfooding_results/kitti_seq_02_full` |
| L-LO on KITTI Odom seq 05 full (2761 frames) | `ready` | `default` | 8.785 | 5.3 | `dogfooding_results/kitti_seq_05_full` |
| L-LO on KITTI Odom seq 07 full (1101 frames) | `ready` | `default` | 2.045 | 5.1 | `dogfooding_results/kitti_seq_07_full` |
| L-LO on KITTI Odom seq 07 full: combinations of the best round-1 settings | `ready` | `cell_0p35_gtol_0p15_range_80_min15` | 2.561 | 7.9 | `dogfooding_results/kitti_seq_07_full` |
| L-LO on KITTI Odom seq 07 full: landmark extraction sweep | `ready` | `sor_std_2` | 1.998 | 6.8 | `dogfooding_results/kitti_seq_07_full` |
| L-LO on KITTI Odom seq 07 full: landmark extraction, finer clustering cells | `ready` | `cluster_cell_0p15` | 1.686 | 17.9 | `dogfooding_results/kitti_seq_07_full` |
| L-LO on KITTI Odom seq 07 full: pre-processing sweep | `ready` | `voxel_0p4` | 2.013 | 22.9 | `dogfooding_results/kitti_seq_07_full` |
| L-LO on KITTI Odom seq 07 full: registration sweep | `ready` | `rounds_600_tol_1e5` | 2.045 | 5.9 | `dogfooding_results/kitti_seq_07_full` |
| L-LO on KITTI Odom seq 07 full: vertical pose sweep | `ready` | `pitch_band_10_40` | 1.957 | 5.2 | `dogfooding_results/kitti_seq_07_full` |
| L-LO on KITTI Odom seq 08 full (4071 frames) | `ready` | `default` | 32.662 | 4.0 | `dogfooding_results/kitti_seq_08_full` |
| LeGO-LOAM cluster discovery on KITTI Odom seq 07 full (1102 frames) | `ready` | `fast` | 2.590 | 2.8 | `dogfooding_results/kitti_seq_07_full` |
| LeGO-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.865 | 9.9 | `dogfooding_results/kitti_raw_0009_200` |
| LeGO-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.865 | 8.9 | `dogfooding_results/kitti_raw_0009_200` |
| LeGO-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.458 | 9.7 | `dogfooding_results/kitti_raw_0061_200` |
| LeGO-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 5.248 | 11.1 | `dogfooding_results/kitti_raw_0061_full` |
| LeGO-LOAM throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast` | 6.072 | 9.9 | `dogfooding_results/mcd_kth_day_06_108` |
| LeGO-LOAM throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `fast` | 0.036 | 8.4 | `dogfooding_results/mcd_ntu_day_02_108` |
| LeGO-LOAM throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast` | 1.344 | 10.1 | `dogfooding_results/mcd_tuhh_night_09_108` |
| LeGO-LOAM throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast` | 0.147 | 21.8 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| LeGO-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 6.066 | 9.5 | `dogfooding_results/kitti_raw_0009_full` |
| LeGO-LOAM transfer check on KITTI Odom seq 00 full (4541 frames) | `ready` | `kitti_default` | 12.833 | 1.4 | `dogfooding_results/kitti_seq_00_full` |
| LeGO-LOAM transfer check on KITTI Odom seq 02 full (4661 frames) | `ready` | `kitti_default` | 41.756 | 1.0 | `dogfooding_results/kitti_seq_02_full` |
| LeGO-LOAM transfer check on KITTI Odom seq 05 full (2761 frames) | `ready` | `kitti_default` | 6.449 | 0.8 | `dogfooding_results/kitti_seq_05_full` |
| LeGO-LOAM transfer check on KITTI Odom seq 08 full (4071 frames) | `ready` | `kitti_default` | 17.896 | 0.9 | `dogfooding_results/kitti_seq_08_full` |
| LF-GICP KITTI Odom seq 07 full: elevation correction ablation | `ready` | `paper_default` | 0.646 | 8.8 | `dogfooding_results/kitti_seq_07_full` |
| LF-GICP on KITTI Odom seq 00 full (4541 frames) | `ready` | `no_mitigation` | 7.848 | 3.8 | `dogfooding_results/kitti_seq_00_full` |
| LF-GICP on KITTI Odom seq 02 full (4661 frames) | `ready` | `no_mitigation` | 27.186 | 3.6 | `dogfooding_results/kitti_seq_02_full` |
| LF-GICP on KITTI Odom seq 05 full (2761 frames) | `ready` | `no_mitigation` | 5.556 | 5.4 | `dogfooding_results/kitti_seq_05_full` |
| LF-GICP on KITTI Odom seq 07 full (1101 frames) | `ready` | `paper_default` | 0.646 | 4.3 | `dogfooding_results/kitti_seq_07_full` |
| LF-GICP on KITTI Odom seq 08 full (4071 frames) | `ready` | `no_mitigation` | 16.280 | 2.9 | `dogfooding_results/kitti_seq_08_full` |
| LINS on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 119.861 | 105.0 | `dogfooding_results/kitti_raw_0009_200` |
| LINS on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 119.861 | 120.7 | `dogfooding_results/kitti_raw_0009_200` |
| LINS on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 183.380 | 123.3 | `dogfooding_results/kitti_raw_0009_full` |
| LINS on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 82.383 | 115.2 | `dogfooding_results/kitti_raw_0061_200` |
| LINS on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 291.877 | 104.7 | `dogfooding_results/kitti_raw_0061_full` |
| LINS on MCD KTH day-06 sequence | `ready` | `fast` | 7.120 | 166.1 | `dogfooding_results/mcd_kth_day_06_108` |
| LINS on MCD NTU day-02 sequence | `ready` | `dense` | 0.111 | 147.2 | `dogfooding_results/mcd_ntu_day_02_108` |
| LINS on MCD TUHH night-09 sequence | `ready` | `fast` | 1.147 | 173.4 | `dogfooding_results/mcd_tuhh_night_09_108` |
| LINS on the public HDL-400 reference window | `ready` | `fast` | 29.745 | 71.9 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| LIO-SAM on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.579 | 23.1 | `dogfooding_results/kitti_raw_0009_200` |
| LIO-SAM on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.579 | 24.9 | `dogfooding_results/kitti_raw_0009_200` |
| LIO-SAM on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.259 | 20.2 | `dogfooding_results/kitti_raw_0009_full` |
| LIO-SAM on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.577 | 27.1 | `dogfooding_results/kitti_raw_0061_200` |
| LIO-SAM on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 5.236 | 21.2 | `dogfooding_results/kitti_raw_0061_full` |
| LIO-SAM on MCD KTH day-06 sequence | `ready` | `fast` | 6.020 | 29.7 | `dogfooding_results/mcd_kth_day_06_108` |
| LIO-SAM on MCD NTU day-02 sequence | `ready` | `fast` | 0.045 | 31.7 | `dogfooding_results/mcd_ntu_day_02_108` |
| LIO-SAM on MCD TUHH night-09 sequence | `ready` | `fast` | 1.314 | 30.0 | `dogfooding_results/mcd_tuhh_night_09_108` |
| LIO-SAM on the public HDL-400 reference window | `ready` | `fast` | 0.202 | 18.1 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| LiTAMIN2 cluster T1 (voxel=0.5 + iter=12 + seed) on KITTI seq 07 full | `ready` | `fast_seeded_reference` | 0.647 | 94.7 | `dogfooding_results/kitti_seq_07_full` |
| LiTAMIN2 cluster T1 on KITTI Raw 0009 200 (200 frames) | `ready` | `cluster_t1_seeded` | 0.644 | 30.0 | `dogfooding_results/kitti_raw_0009_200` |
| LiTAMIN2 cluster T1 on KITTI Raw 0009 full (447 frames) | `ready` | `cluster_t1_seeded` | 0.676 | 20.6 | `dogfooding_results/kitti_raw_0009_full` |
| LiTAMIN2 cluster T1 on KITTI Raw 0061 200 (200 frames) | `ready` | `cluster_t1_seeded` | 0.511 | 48.1 | `dogfooding_results/kitti_raw_0061_200` |
| LiTAMIN2 cluster T1 on KITTI Raw 0061 full (707 frames) | `ready` | `cluster_t1_seeded` | 0.600 | 61.4 | `dogfooding_results/kitti_raw_0061_full` |
| LiTAMIN2 cluster T1 on KITTI seq 02 full (4661 frames, CT-ICP's worst seq) | `ready` | `cluster_t1_seeded` | 0.728 | 68.3 | `dogfooding_results/kitti_seq_02_full` |
| LiTAMIN2 cluster T1 on KITTI seq 05 full (2761 frames, mid-length) | `ready` | `fast_seeded_reference` | 0.751 | 17.2 | `dogfooding_results/kitti_seq_05_full` |
| LiTAMIN2 cluster T1 on KITTI seq 08 full (long urban, seed-flip territory) | `ready` | `fast_seeded_reference` | 0.696 | 104.9 | `dogfooding_results/kitti_seq_08_full` |
| LiTAMIN2 cluster T1 on MCD KTH day_06 (108 frames) | `ready` | `cluster_t1_seeded` | 0.192 | 33.0 | `dogfooding_results/mcd_kth_day_06_108` |
| LiTAMIN2 cluster T1 on MCD NTU day_02 (108 frames) | `ready` | `cluster_t1_seeded` | 0.021 | 43.4 | `dogfooding_results/mcd_ntu_day_02_108` |
| LiTAMIN2 cluster T1 on MCD TUHH night_09 (108 frames) | `ready` | `cluster_t1_seeded` | 0.132 | 39.5 | `dogfooding_results/mcd_tuhh_night_09_108` |
| LiTAMIN2 cluster T1 on MulRan parkinglot 120-frame | `ready` | `cluster_t1_seeded` | 0.212 | 39.0 | `dogfooding_results/mulran_parkinglot_120` |
| LiTAMIN2 cluster T1 on MulRan parkinglot full (CT-ICP cluster A territory) | `ready` | `cluster_t1_seeded` | 0.303 | 34.4 | `dogfooding_results/mulran_parkinglot_full` |
| LiTAMIN2 correspondence sweep on KITTI Odometry 02 (full) | `ready` | `cov_floor_1e_4` | 44.005 | 91.8 | `dogfooding_results/kitti_seq_02_full` |
| LiTAMIN2 correspondence sweep on KITTI Odometry 05 (full) | `ready` | `cov_floor_1e_4` | 6.069 | 93.4 | `dogfooding_results/kitti_seq_05_full` |
| LiTAMIN2 correspondence sweep on KITTI Odometry 07 (full) | `ready` | `cov_floor_1e_4` | 1.964 | 106.9 | `dogfooding_results/kitti_seq_07_full` |
| LiTAMIN2 correspondence sweep on KITTI Odometry 08 (full) | `ready` | `cov_floor_1e_4` | 18.327 | 94.2 | `dogfooding_results/kitti_seq_08_full` |
| LiTAMIN2 KITTI Odom seq 07 full: elevation correction ablation | `ready` | `coarse_to_fine_3_2_1_elevation` | 2.086 | 47.5 | `dogfooding_results/kitti_seq_07_full` |
| LiTAMIN2 paper-comparable on KITTI Odometry 00 (full) | `ready` | `fast_cov_no_gt_seed` | 110.484 | 98.9 | `dogfooding_results/kitti_seq_00_full` |
| LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_icp_only_half_threads` | 1.067 | 31.7 | `dogfooding_results/kitti_raw_0009_200` |
| LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_cov_half_threads` | 0.511 | 68.6 | `dogfooding_results/kitti_raw_0061_200` |
| LiTAMIN2 throughput and accuracy trade-off on MulRan ParkingLot (120-frame window) | `ready` | `fast_cov_half_threads` | 0.498 | 121.0 | `dogfooding_results/mulran_parkinglot_120` |
| LiTAMIN2 throughput and accuracy trade-off on MulRan ParkingLot (full sequence) | `ready` | `fast_icp_only_half_threads` | 0.711 | 118.6 | `dogfooding_results/mulran_parkinglot_full` |
| LiTAMIN2 throughput and accuracy trade-off on the full KITTI Odometry sequence 00 | `ready` | `fast_icp_only_half_threads` | 0.998 | 108.9 | `dogfooding_results/kitti_seq_00_full` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_icp_only_half_threads` | 0.401 | 113.0 | `dogfooding_results/mcd_kth_day_06_108` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `paper_icp_only_half_threads` | 0.045 | 81.2 | `dogfooding_results/mcd_ntu_day_02_108` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_icp_only_half_threads` | 0.194 | 121.2 | `dogfooding_results/mcd_tuhh_night_09_108` |
| LiTAMIN2 throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_cov_half_threads` | 0.111 | 80.7 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| LiTAMIN2 throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_icp_only_half_threads` | 1.213 | 23.5 | `dogfooding_results/autoware_istanbul_open_108` |
| LiTAMIN2 throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_icp_only_half_threads` | 0.168 | 6.1 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| LiTAMIN2 throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_icp_only_half_threads` | 1.222 | 20.9 | `dogfooding_results/autoware_istanbul_open_108_b` |
| LiTAMIN2 throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `paper_icp_only_half_threads` | 0.741 | 21.2 | `dogfooding_results/autoware_istanbul_open_108_c` |
| LiTAMIN2 trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_icp_only_half_threads` | 6.290 | 30.5 | `dogfooding_results/kitti_raw_0009_200` |
| LiTAMIN2 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_cov_half_threads` | 1.064 | 19.5 | `dogfooding_results/kitti_raw_0009_full` |
| LiTAMIN2 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_icp_only_half_threads` | 0.944 | 58.1 | `dogfooding_results/kitti_raw_0061_full` |
| LiTAMIN2 tuned (voxel 1.0 + iter 12) on KITTI Odometry 00 (full) | `ready` | `tuned_voxel1_iter12` | 13.464 | 60.9 | `dogfooding_results/kitti_seq_00_full` |
| LiTAMIN2 tuned knobs (voxel=1.0, iter=12) on KITTI seq 00 full with GT seed | `ready` | `tuned_voxel2_iter12_seeded` | 0.731 | 86.8 | `dogfooding_results/kitti_seq_00_full` |
| LiTAMIN2 voxel resolution on the NCLT 2012-12-01 5000-frame session (cross-session T1 check) | `ready` | `voxel_0_5_t1` | 0.519 | 6.9 | `dogfooding_results/nclt_2012_12_01_5000` |
| LiTAMIN2 voxel resolution on the NCLT 2013-01-10 600-frame window | `ready` | `default_voxel_2_0` | 0.358 | 14.7 | `dogfooding_results/nclt_2013_01_10_600` |
| LOAM Livox on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 92.735 | 43.8 | `dogfooding_results/kitti_raw_0009_200` |
| LOAM Livox on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 124.828 | 33.1 | `dogfooding_results/kitti_raw_0009_full` |
| LOAM Livox on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 45.818 | 50.0 | `dogfooding_results/kitti_raw_0061_200` |
| LOAM Livox on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 200.533 | 36.6 | `dogfooding_results/kitti_raw_0061_full` |
| LOAM Livox on MCD KTH day-06 sequence | `ready` | `fast` | 4.095 | 66.3 | `dogfooding_results/mcd_kth_day_06_108` |
| LOAM Livox on MCD NTU day-02 sequence | `ready` | `fast` | 0.057 | 69.0 | `dogfooding_results/mcd_ntu_day_02_108` |
| LOAM Livox on MCD TUHH night-09 sequence | `ready` | `fast` | 1.192 | 68.8 | `dogfooding_results/mcd_tuhh_night_09_108` |
| LOAM-Livox on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 92.735 | 40.2 | `dogfooding_results/kitti_raw_0009_200` |
| LOAM-Livox on the public HDL-400 reference window | `ready` | `default` | 0.079 | 52.0 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| LVI-SAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0009_200` |
| LVI-SAM throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0061_200` |
| LVI-SAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0009_full` |
| LVI-SAM trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0061_full` |
| MULLS cluster discovery on KITTI Odom seq 07 full (1102 frames) | `ready` | `fast` | 8.288 | 4.1 | `dogfooding_results/kitti_seq_07_full` |
| MULLS throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.695 | 1.2 | `dogfooding_results/kitti_raw_0009_200` |
| MULLS throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0009_200` |
| MULLS throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.425 | 3.3 | `dogfooding_results/kitti_raw_0061_200` |
| MULLS throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 10.173 | 3.3 | `dogfooding_results/kitti_raw_0061_full` |
| MULLS throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast` | 6.207 | 4.1 | `dogfooding_results/mcd_kth_day_06_108` |
| MULLS throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `kitti_default` | 0.097 | 2.3 | `dogfooding_results/mcd_ntu_day_02_108` |
| MULLS throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast` | 1.206 | 3.8 | `dogfooding_results/mcd_tuhh_night_09_108` |
| MULLS throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast` | 0.874 | 9.3 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| MULLS trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 4.610 | 3.3 | `dogfooding_results/kitti_raw_0009_full` |
| MULLS transfer check on KITTI Odom seq 00 full (4541 frames) | `ready` | `fast` | 48.686 | 1.8 | `dogfooding_results/kitti_seq_00_full` |
| MULLS transfer check on KITTI Odom seq 02 full (4661 frames) | `ready` | `fast` | 254.460 | 1.7 | `dogfooding_results/kitti_seq_02_full` |
| MULLS transfer check on KITTI Odom seq 05 full (2761 frames) | `ready` | `fast` | 19.617 | 1.7 | `dogfooding_results/kitti_seq_05_full` |
| MULLS transfer check on KITTI Odom seq 08 full (4071 frames) | `ready` | `fast` | 80.765 | 1.7 | `dogfooding_results/kitti_seq_08_full` |
| NDT cluster discovery on KITTI Odom seq 07 full (1102 frames) | `ready` | `fast_profile` | 0.076 | 43.6 | `dogfooding_results/kitti_seq_07_full` |
| NDT T1 confirmation on KITTI Odom seq 02 full (4661 frames) | `ready` | `t1_transfer_r05_i12` | 0.059 | 0.2 | `dogfooding_results/kitti_seq_02_full` |
| NDT T1 confirmation on KITTI Odom seq 05 full (2761 frames) | `ready` | `t1_transfer_r05_i12` | 0.059 | 0.2 | `dogfooding_results/kitti_seq_05_full` |
| NDT T1 confirmation on KITTI Odom seq 08 full (4071 frames) | `ready` | `t1_transfer_r05_i12` | 0.076 | 0.3 | `dogfooding_results/kitti_seq_08_full` |
| NDT T1 transfer on KITTI Odom seq 00 full (4542 frames) | `ready` | `t1_transfer_r03_i12` | 0.023 | 37.5 | `dogfooding_results/kitti_seq_00_full` |
| NDT throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_coarse_map` | 0.284 | 20.4 | `dogfooding_results/kitti_raw_0009_200` |
| NDT throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_coarse_map` | 0.319 | 41.2 | `dogfooding_results/kitti_raw_0061_200` |
| NDT throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_coarse_map` | 0.136 | 31.2 | `dogfooding_results/mcd_kth_day_06_108` |
| NDT throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `balanced_local_map` | 0.013 | 44.9 | `dogfooding_results/mcd_ntu_day_02_108` |
| NDT throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_coarse_map` | 0.063 | 40.8 | `dogfooding_results/mcd_tuhh_night_09_108` |
| NDT throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_coarse_map` | 0.034 | 32.2 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| NDT throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.070 | 2.0 | `dogfooding_results/autoware_istanbul_open_108` |
| NDT throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_coarse_map` | 0.065 | 0.9 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| NDT throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.007 | 2.1 | `dogfooding_results/autoware_istanbul_open_108_b` |
| NDT throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.005 | 1.9 | `dogfooding_results/autoware_istanbul_open_108_c` |
| NDT trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_coarse_map` | 93.576 | 14.3 | `dogfooding_results/kitti_raw_0009_200` |
| NDT trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_coarse_map` | 0.238 | 14.1 | `dogfooding_results/kitti_raw_0009_full` |
| NDT trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_coarse_map` | 0.247 | 23.8 | `dogfooding_results/kitti_raw_0061_full` |
| OKVIS throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 31.400 | 524.0 | `dogfooding_results/kitti_raw_0009_200` |
| OKVIS throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 27.889 | 894.7 | `dogfooding_results/kitti_raw_0061_200` |
| OKVIS trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 108.629 | 1146.6 | `dogfooding_results/kitti_raw_0009_full` |
| OKVIS trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 201.724 | 1717.1 | `dogfooding_results/kitti_raw_0061_full` |
| ORB-SLAM3 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0009_200` |
| ORB-SLAM3 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0061_200` |
| ORB-SLAM3 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0009_full` |
| ORB-SLAM3 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0061_full` |
| Point-LIO on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 119.890 | 95.6 | `dogfooding_results/kitti_raw_0009_200` |
| Point-LIO on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 119.890 | 117.4 | `dogfooding_results/kitti_raw_0009_200` |
| Point-LIO on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 183.384 | 113.1 | `dogfooding_results/kitti_raw_0009_full` |
| Point-LIO on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 82.450 | 92.8 | `dogfooding_results/kitti_raw_0061_200` |
| Point-LIO on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 292.011 | 89.5 | `dogfooding_results/kitti_raw_0061_full` |
| Point-LIO on MCD KTH day-06 sequence | `ready` | `fast` | 7.113 | 112.7 | `dogfooding_results/mcd_kth_day_06_108` |
| Point-LIO on MCD NTU day-02 sequence | `ready` | `fast` | 0.083 | 77.3 | `dogfooding_results/mcd_ntu_day_02_108` |
| Point-LIO on MCD TUHH night-09 sequence | `ready` | `fast` | 1.116 | 88.7 | `dogfooding_results/mcd_tuhh_night_09_108` |
| Point-LIO on the public HDL-400 reference window | `ready` | `fast` | 17.929 | 69.9 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| R2LIVE throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0009_200` |
| R2LIVE throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0061_200` |
| R2LIVE trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0009_full` |
| R2LIVE trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `skipped` | `-` | - | - | `dogfooding_results/kitti_raw_0061_full` |
| RKO-LIO gyro-bias feedback gain on the NCLT 2013-01-10 window | `ready` | `no_bias_feedback` | 0.141 | 8.1 | `dogfooding_results/nclt_2013_01_10_120` |
| Small-GICP cluster discovery on KITTI Odom seq 00 full (4542 frames) | `ready` | `fast_profile` | 0.890 | 106.4 | `dogfooding_results/kitti_seq_00_full` |
| Small-GICP cluster discovery on KITTI Odom seq 02 full (4661 frames) | `ready` | `fast_profile` | 0.909 | 96.5 | `dogfooding_results/kitti_seq_02_full` |
| Small-GICP cluster discovery on KITTI Odom seq 05 full (2761 frames) | `ready` | `fast_profile` | 0.984 | 104.5 | `dogfooding_results/kitti_seq_05_full` |
| Small-GICP cluster discovery on KITTI Odom seq 07 full (1102 frames) | `ready` | `fast_profile` | 0.682 | 107.7 | `dogfooding_results/kitti_seq_07_full` |
| Small-GICP cluster discovery on KITTI Odom seq 08 full (4071 frames) | `ready` | `fast_profile` | 0.858 | 74.6 | `dogfooding_results/kitti_seq_08_full` |
| Small-GICP no-seed robustness on KITTI Odom seq 00 full (4542 frames) | `ready` | `fast_seeded_reference` | 0.890 | 63.3 | `dogfooding_results/kitti_seq_00_full` |
| Small-GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `balanced_local_map` | 2.071 | 22.0 | `dogfooding_results/kitti_raw_0009_200` |
| Small-GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_recent_map` | 0.473 | 23.7 | `dogfooding_results/kitti_raw_0009_200` |
| Small-GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_recent_map` | 0.639 | 78.1 | `dogfooding_results/kitti_raw_0061_200` |
| Small-GICP throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_recent_map` | 0.959 | 82.2 | `dogfooding_results/kitti_raw_0061_full` |
| Small-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 0.806 | 107.9 | `dogfooding_results/mcd_kth_day_06_108` |
| Small-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.031 | 113.8 | `dogfooding_results/mcd_ntu_day_02_108` |
| Small-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 0.250 | 107.2 | `dogfooding_results/mcd_tuhh_night_09_108` |
| Small-GICP throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 0.109 | 110.3 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| Small-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 0.462 | 17.3 | `dogfooding_results/kitti_raw_0009_full` |
| small_gicp seed-gate tightening on the NCLT 2013-01-10 full 5105-frame trajectory | `ready` | `gate_0_5` | 0.348 | 11.8 | `dogfooding_results/nclt_2013_01_10_full` |
| SuMa KITTI Odom seq 07 full: elevation correction ablation | `ready` | `dense_profile_elevation` | 3.694 | 35.6 | `dogfooding_results/kitti_seq_07_full` |
| SuMa on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `dense` | 2.245 | 21.0 | `dogfooding_results/kitti_raw_0009_200` |
| SuMa on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `default` | 2.245 | 19.6 | `dogfooding_results/kitti_raw_0009_200` |
| SuMa on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `dense` | 4.073 | 15.9 | `dogfooding_results/kitti_raw_0009_full` |
| SuMa on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `dense` | 1.496 | 111.2 | `dogfooding_results/kitti_raw_0061_200` |
| SuMa on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 15.381 | 110.9 | `dogfooding_results/kitti_raw_0061_full` |
| SuMa on MCD KTH day-06 sequence | `ready` | `fast` | 6.064 | 150.2 | `dogfooding_results/mcd_kth_day_06_108` |
| SuMa on MCD NTU day-02 sequence | `ready` | `dense` | 0.036 | 124.1 | `dogfooding_results/mcd_ntu_day_02_108` |
| SuMa on MCD TUHH night-09 sequence | `ready` | `default` | 1.317 | 178.1 | `dogfooding_results/mcd_tuhh_night_09_108` |
| SuMa on the public HDL-400 reference window | `ready` | `default` | 0.183 | 168.4 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| SuMa transfer check on KITTI Odom seq 00 full (4541 frames) | `ready` | `dense_profile` | 18.961 | 33.8 | `dogfooding_results/kitti_seq_00_full` |
| SuMa transfer check on KITTI Odom seq 02 full (4661 frames) | `ready` | `dense_profile` | 51.911 | 38.0 | `dogfooding_results/kitti_seq_02_full` |
| SuMa transfer check on KITTI Odom seq 05 full (2761 frames) | `ready` | `default` | 9.511 | 40.3 | `dogfooding_results/kitti_seq_05_full` |
| SuMa transfer check on KITTI Odom seq 08 full (4071 frames) | `ready` | `dense_profile` | 19.290 | 39.6 | `dogfooding_results/kitti_seq_08_full` |
| VGICP SLAM on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 1.674 | 6.2 | `dogfooding_results/kitti_raw_0009_200` |
| VGICP SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 2.252 | 8.4 | `dogfooding_results/kitti_raw_0009_full` |
| VGICP SLAM on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.799 | 31.0 | `dogfooding_results/kitti_raw_0061_200` |
| VGICP SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 5.230 | 22.8 | `dogfooding_results/kitti_raw_0061_full` |
| VGICP SLAM on MCD KTH day-06 sequence | `ready` | `fast` | 6.092 | 20.8 | `dogfooding_results/mcd_kth_day_06_108` |
| VGICP SLAM on MCD NTU day-02 sequence | `ready` | `fast` | 0.016 | 40.3 | `dogfooding_results/mcd_ntu_day_02_108` |
| VGICP SLAM on MCD TUHH night-09 sequence | `ready` | `fast` | 1.319 | 21.8 | `dogfooding_results/mcd_tuhh_night_09_108` |
| VGICP-SLAM on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `default` | 1.674 | 5.1 | `dogfooding_results/kitti_raw_0009_200` |
| VGICP-SLAM on the public HDL-400 reference window | `ready` | `fast` | 0.145 | 16.8 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| VINS-Fusion throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 31.407 | 83.5 | `dogfooding_results/kitti_raw_0009_200` |
| VINS-Fusion throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 27.811 | 255.5 | `dogfooding_results/kitti_raw_0061_200` |
| VINS-Fusion trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 50.238 | 93.7 | `dogfooding_results/kitti_raw_0009_full` |
| VINS-Fusion trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 97.611 | 236.7 | `dogfooding_results/kitti_raw_0061_full` |
| Voxel-GICP cluster discovery on KITTI Odom seq 00 full (4542 frames) | `ready` | `dense_profile` | 1.047 | 97.9 | `dogfooding_results/kitti_seq_00_full` |
| Voxel-GICP cluster discovery on KITTI Odom seq 02 full (4661 frames) | `ready` | `dense_profile` | 0.944 | 84.8 | `dogfooding_results/kitti_seq_02_full` |
| Voxel-GICP cluster discovery on KITTI Odom seq 05 full (2761 frames) | `ready` | `dense_profile` | 1.031 | 111.1 | `dogfooding_results/kitti_seq_05_full` |
| Voxel-GICP cluster discovery on KITTI Odom seq 07 full (1102 frames) | `ready` | `dense_profile` | 0.950 | 101.4 | `dogfooding_results/kitti_seq_07_full` |
| Voxel-GICP cluster discovery on KITTI Odom seq 08 full (4071 frames) | `ready` | `dense_profile` | 0.955 | 91.1 | `dogfooding_results/kitti_seq_08_full` |
| Voxel-GICP no-seed robustness on KITTI Odom seq 00 full (4542 frames) | `ready` | `dense_seeded_reference` | 1.047 | 95.6 | `dogfooding_results/kitti_seq_00_full` |
| Voxel-GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_recent_map` | 18.235 | 24.1 | `dogfooding_results/kitti_raw_0009_200` |
| Voxel-GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `dense_recent_map` | 0.644 | 23.3 | `dogfooding_results/kitti_raw_0009_200` |
| Voxel-GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `dense_recent_map` | 0.741 | 73.7 | `dogfooding_results/kitti_raw_0061_200` |
| Voxel-GICP throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `dense_recent_map` | 0.917 | 75.4 | `dogfooding_results/kitti_raw_0061_full` |
| Voxel-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `dense_recent_map` | 0.926 | 124.2 | `dogfooding_results/mcd_kth_day_06_108` |
| Voxel-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.121 | 117.2 | `dogfooding_results/mcd_ntu_day_02_108` |
| Voxel-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `dense_recent_map` | 0.286 | 116.4 | `dogfooding_results/mcd_tuhh_night_09_108` |
| Voxel-GICP throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `dense_recent_map` | 0.268 | 141.1 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| Voxel-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `dense_recent_map` | 0.614 | 27.5 | `dogfooding_results/kitti_raw_0009_full` |
| X-ICP on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `default` | 22.317 | 24.6 | `dogfooding_results/kitti_raw_0009_200` |
| X-ICP on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `default` | 0.140 | 27.5 | `dogfooding_results/kitti_raw_0009_200` |
| X-ICP on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `dense` | 0.129 | 18.1 | `dogfooding_results/kitti_raw_0009_full` |
| X-ICP on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.098 | 102.0 | `dogfooding_results/kitti_raw_0061_200` |
| X-ICP on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 0.128 | 104.6 | `dogfooding_results/kitti_raw_0061_full` |
| X-ICP on MCD KTH day-06 sequence | `ready` | `fast` | 0.305 | 84.8 | `dogfooding_results/mcd_kth_day_06_108` |
| X-ICP on MCD NTU day-02 sequence | `ready` | `dense` | 0.095 | 81.8 | `dogfooding_results/mcd_ntu_day_02_108` |
| X-ICP on MCD TUHH night-09 sequence | `ready` | `dense` | 0.081 | 100.4 | `dogfooding_results/mcd_tuhh_night_09_108` |
| X-ICP on the public HDL-400 reference window | `ready` | `dense` | 0.168 | 123.9 | `dogfooding_results/hdl_400_open_ct_lio_120` |

## Recommendation Order

1. Empirical study as the primary paper target.
2. Artifact / reproducibility submission in parallel.
3. Focused method paper only after a stronger multi-dataset delta exists.

## Track A: Empirical Study

- **Readiness**: Medium (55/100)
- **Primary claim**: Variant-first localization benchmarking reveals Pareto fronts that are hidden when repositories force one canonical implementation too early.
- **Decision**: Primary target. This is the most defensible full-paper narrative for the current repository.

### Strongest Evidence

- The stable benchmark contract now covers 425 ready problems under stable dogfooding binaries and one shared summary JSON interface.
- Each active problem keeps at least three concrete variants alive instead of collapsing immediately to a single abstraction.
- Current defaults already show non-trivial trade-offs, such as `LiTAMIN2=fast_seeded_reference` at 94.7 FPS and `CT-LIO=seed_only_fast` at 0.488 m ATE on the public HDL-400 reference window.

### Gaps

- LiDAR-only methods now cover 27 repository-stored open sequences across 6 public dataset families, but external validity still rests on only 6 families.
- Reference-based and GT-backed results are separated, and the GT-backed CT-LIO public benchmark is explicitly scoped out of the main study until independent GT appears.
- There is no paper-ready comparison against originally reported results yet.
- Hardware-normalized reruns and confidence intervals are not exported yet.

### Next Experiments

- Add another public dataset family, or expand the current 6-family evidence with longer and less curated windows.
- Keep GT-backed CT-LIO out of the main evidence tables and revisit only if independent HDL-400 GT becomes available.
- Generate a method-by-method table comparing repository defaults, challengers, and original-paper numbers.
- Export paper-ready Pareto figures from `experiments/results/*.json`.

## Track B: Artifact / Reproducibility

- **Readiness**: High (87/100)
- **Primary claim**: Localization research tooling is more reproducible when the benchmark contract is stable and variant search remains explicit, comparable, and discardable.
- **Decision**: Parallel target. This is the fastest submission path if the goal is near-term publication evidence.

### Strongest Evidence

- The repository already separates stable cores (`pcd_dogfooding --summary-json`, `multimodal_dogfooding --summary-json`) from discardable experimental manifests.
- Comparison state is externalized into generated docs instead of being trapped in code comments or ad-hoc notebooks.
- The runner now supports `--reuse-existing`, which makes expensive comparisons reproducible without rerunning every variant.
- Experiment-facing and publication-facing docs can now be refreshed in one command via `python3 evaluation/scripts/refresh_study_docs.py`.
- Paper-ready tables and Pareto figures are now exported from aggregate JSON via `python3 evaluation/scripts/export_paper_assets.py`.

### Gaps

- Container or pinned environment instructions are still missing.
- The public Pages site does not yet expose the experiment docs alongside the benchmark snapshot.
- Dataset bootstrap is still manual, so full artifact replay is not yet one-command from a clean machine.

### Next Experiments

- Ship a pinned environment definition or container recipe.
- Publish `docs/paper_tracks.md` and `docs/paper_roadmap.md` through Pages so the artifact narrative is visible externally.
- Add dataset bootstrap helpers so a clean machine can reproduce the study without manual path setup.

## Track C: Focused Method Paper

- **Readiness**: Low (20/100)
- **Primary claim**: One concrete localization method variant materially improves the accuracy / throughput frontier beyond the current repository baselines.
- **Decision**: Hold. Keep collecting evidence, but do not make this the main paper narrative yet.

### Strongest Evidence

- CT-LIO now has a public reference-based trade-off problem with three bounded variants under one interface.
- LiTAMIN2 already has a measurable throughput-oriented variant family and a paper-profile family under the same evaluator.
- The repository can now keep weaker and stronger method variants alive without deleting the evidence trail.

### Gaps

- No single method currently shows a strong enough multi-dataset improvement claim for a focused algorithm paper.
- Current CT-LIO evidence is reference-based and slow; it is not yet enough for a strong method contribution.
- LiTAMIN2 speedups are real inside this repository, but they are not yet benchmarked broadly enough to support a standalone claim.

### Next Experiments

- Pick one method only after it shows repeatable improvement on at least two open sequences.
- Add ablations that isolate exactly one new algorithmic idea instead of a profile bundle.
- Treat the focused-method path as a by-product of Track A, not the current main plan.
