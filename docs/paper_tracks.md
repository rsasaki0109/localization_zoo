# Paper Tracks

_Generated at 2026-04-13T12:41:47+00:00 by `evaluation/scripts/generate_publication_docs.py`._

This repository should not be pitched as "many implementations exist here".
The paper target has to be a claim about what this experiment-driven process reveals.

Current coverage: `248` ready problems and `2` blocked problems.

## Current State

| Problem | Status | Default | Best ATE [m] | Best FPS | Dataset |
|---------|--------|---------|--------------|----------|---------|
| A-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 3.185 | 4.8 | `dogfooding_results/kitti_raw_0009_200` |
| A-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 3.185 | 3.4 | `dogfooding_results/kitti_raw_0009_200` |
| A-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.459 | 6.0 | `dogfooding_results/kitti_raw_0061_200` |
| A-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 3.654 | 6.0 | `dogfooding_results/kitti_raw_0061_full` |
| A-LOAM throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast` | 6.077 | 6.7 | `dogfooding_results/mcd_kth_day_06_108` |
| A-LOAM throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense` | 0.035 | 5.8 | `dogfooding_results/mcd_ntu_day_02_108` |
| A-LOAM throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast` | 1.336 | 6.5 | `dogfooding_results/mcd_tuhh_night_09_108` |
| A-LOAM throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast` | 0.146 | 13.8 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| A-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 6.105 | 5.8 | `dogfooding_results/kitti_raw_0009_full` |
| BALM2 on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.366 | 12.6 | `dogfooding_results/kitti_raw_0009_200` |
| BALM2 on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.366 | 13.7 | `dogfooding_results/kitti_raw_0009_200` |
| BALM2 on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 3.338 | 12.7 | `dogfooding_results/kitti_raw_0009_full` |
| BALM2 on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 1.883 | 13.2 | `dogfooding_results/kitti_raw_0061_200` |
| BALM2 on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 9.533 | 11.4 | `dogfooding_results/kitti_raw_0061_full` |
| BALM2 on MCD KTH day-06 sequence | `ready` | `fast` | 6.184 | 13.4 | `dogfooding_results/mcd_kth_day_06_108` |
| BALM2 on MCD NTU day-02 sequence | `ready` | `fast` | 0.062 | 12.7 | `dogfooding_results/mcd_ntu_day_02_108` |
| BALM2 on MCD TUHH night-09 sequence | `ready` | `fast` | 1.270 | 14.6 | `dogfooding_results/mcd_tuhh_night_09_108` |
| BALM2 on the public HDL-400 reference window | `ready` | `fast` | 0.476 | 9.0 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| CLINS trade-off on the public ROS1 HDL-400 window with synthesized per-point time | `ready` | `dense` | 1.473 | 100.3 | `dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index` |
| CT-ICP performance-priority trade-off on the public ROS1 HDL-400 window with synthesized per-point time | `ready` | `dense_window` | 1.254 | 68.5 | `dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index` |
| CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `balanced_window` | 1.659 | 54.8 | `dogfooding_results/kitti_raw_0009_200` |
| CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_window` | 1.475 | 56.9 | `dogfooding_results/kitti_raw_0061_200` |
| CT-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_window` | 6.115 | 59.8 | `dogfooding_results/mcd_kth_day_06_108` |
| CT-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_window` | 0.325 | 71.7 | `dogfooding_results/mcd_ntu_day_02_108` |
| CT-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_window` | 1.652 | 71.6 | `dogfooding_results/mcd_tuhh_night_09_108` |
| CT-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `fast_window` | 1.251 | 72.9 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| CT-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `fast_window` | 75.075 | 2.7 | `dogfooding_results/autoware_istanbul_open_108` |
| CT-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `fast_window` | 0.556 | 2.4 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| CT-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `balanced_window` | 6.820 | 3.1 | `dogfooding_results/autoware_istanbul_open_108_b` |
| CT-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `balanced_window` | 7.539 | 2.8 | `dogfooding_results/autoware_istanbul_open_108_c` |
| CT-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `balanced_window` | 1.659 | 71.4 | `dogfooding_results/kitti_raw_0009_200` |
| CT-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `balanced_window` | 4.673 | 44.9 | `dogfooding_results/kitti_raw_0009_full` |
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
| F-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.883 | 28.0 | `dogfooding_results/kitti_raw_0009_200` |
| F-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.883 | 24.6 | `dogfooding_results/kitti_raw_0009_200` |
| F-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.587 | 25.9 | `dogfooding_results/kitti_raw_0061_200` |
| F-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 3.822 | 30.7 | `dogfooding_results/kitti_raw_0061_full` |
| F-LOAM throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast` | 6.005 | 31.1 | `dogfooding_results/mcd_kth_day_06_108` |
| F-LOAM throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `fast` | 0.111 | 27.0 | `dogfooding_results/mcd_ntu_day_02_108` |
| F-LOAM throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast` | 1.345 | 27.6 | `dogfooding_results/mcd_tuhh_night_09_108` |
| F-LOAM throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast` | 0.193 | 64.2 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| F-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.452 | 28.6 | `dogfooding_results/kitti_raw_0009_full` |
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
| GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_recent_map` | 1.177 | 25.8 | `dogfooding_results/kitti_raw_0009_200` |
| GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_recent_map` | 0.781 | 25.7 | `dogfooding_results/kitti_raw_0061_200` |
| GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 0.630 | 24.7 | `dogfooding_results/mcd_kth_day_06_108` |
| GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.017 | 28.7 | `dogfooding_results/mcd_ntu_day_02_108` |
| GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 0.317 | 31.2 | `dogfooding_results/mcd_tuhh_night_09_108` |
| GICP throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 0.101 | 23.3 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| GICP throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 0.994 | 6.3 | `dogfooding_results/autoware_istanbul_open_108` |
| GICP throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_recent_map` | 0.135 | 1.7 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| GICP throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 1.166 | 5.7 | `dogfooding_results/autoware_istanbul_open_108_b` |
| GICP throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 0.982 | 4.3 | `dogfooding_results/autoware_istanbul_open_108_c` |
| GICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `dense_recent_map` | 1.510 | 32.0 | `dogfooding_results/kitti_raw_0009_200` |
| GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 1.170 | 23.0 | `dogfooding_results/kitti_raw_0009_full` |
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
| ISC-LOAM on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.321 | 37.6 | `dogfooding_results/kitti_raw_0009_200` |
| ISC-LOAM on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.321 | 35.6 | `dogfooding_results/kitti_raw_0009_200` |
| ISC-LOAM on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 4.323 | 30.5 | `dogfooding_results/kitti_raw_0009_full` |
| ISC-LOAM on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `dense` | 0.494 | 43.5 | `dogfooding_results/kitti_raw_0061_200` |
| ISC-LOAM on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 4.854 | 33.5 | `dogfooding_results/kitti_raw_0061_full` |
| ISC-LOAM on MCD KTH day-06 sequence | `ready` | `fast` | 6.023 | 48.6 | `dogfooding_results/mcd_kth_day_06_108` |
| ISC-LOAM on MCD NTU day-02 sequence | `ready` | `fast` | 0.065 | 50.2 | `dogfooding_results/mcd_ntu_day_02_108` |
| ISC-LOAM on MCD TUHH night-09 sequence | `ready` | `fast` | 1.337 | 53.2 | `dogfooding_results/mcd_tuhh_night_09_108` |
| ISC-LOAM on the public HDL-400 reference window | `ready` | `fast` | 0.161 | 37.0 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_recent_map` | 2.412 | 24.4 | `dogfooding_results/kitti_raw_0009_200` |
| KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_recent_map` | 0.679 | 28.3 | `dogfooding_results/kitti_raw_0061_200` |
| KISS-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 5.568 | 11.3 | `dogfooding_results/mcd_kth_day_06_108` |
| KISS-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `fast_recent_map` | 0.017 | 66.7 | `dogfooding_results/mcd_ntu_day_02_108` |
| KISS-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 1.104 | 24.1 | `dogfooding_results/mcd_tuhh_night_09_108` |
| KISS-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 1.281 | 11.3 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| KISS-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 182.960 | 4.0 | `dogfooding_results/autoware_istanbul_open_108` |
| KISS-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `fast_recent_map` | 0.218 | 0.4 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| KISS-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `dense_local_map` | 143.921 | 3.6 | `dogfooding_results/autoware_istanbul_open_108_b` |
| KISS-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 131.691 | 3.7 | `dogfooding_results/autoware_istanbul_open_108_c` |
| KISS-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_recent_map` | 2.412 | 28.2 | `dogfooding_results/kitti_raw_0009_200` |
| KISS-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 5.383 | 21.9 | `dogfooding_results/kitti_raw_0009_full` |
| KISS-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_recent_map` | 4.343 | 11.2 | `dogfooding_results/kitti_raw_0061_full` |
| LeGO-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.865 | 9.9 | `dogfooding_results/kitti_raw_0009_200` |
| LeGO-LOAM throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.865 | 8.9 | `dogfooding_results/kitti_raw_0009_200` |
| LeGO-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.458 | 9.7 | `dogfooding_results/kitti_raw_0061_200` |
| LeGO-LOAM throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 5.248 | 11.1 | `dogfooding_results/kitti_raw_0061_full` |
| LeGO-LOAM throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast` | 6.072 | 9.9 | `dogfooding_results/mcd_kth_day_06_108` |
| LeGO-LOAM throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `fast` | 0.036 | 8.4 | `dogfooding_results/mcd_ntu_day_02_108` |
| LeGO-LOAM throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast` | 1.344 | 10.1 | `dogfooding_results/mcd_tuhh_night_09_108` |
| LeGO-LOAM throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast` | 0.147 | 21.8 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| LeGO-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 6.066 | 9.5 | `dogfooding_results/kitti_raw_0009_full` |
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
| LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `paper_icp_only_half_threads` | 1.053 | 43.7 | `dogfooding_results/kitti_raw_0009_200` |
| LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_cov_half_threads` | 0.511 | 68.6 | `dogfooding_results/kitti_raw_0061_200` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_cov_half_threads` | 0.401 | 64.2 | `dogfooding_results/mcd_kth_day_06_108` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `paper_icp_only_half_threads` | 0.045 | 105.6 | `dogfooding_results/mcd_ntu_day_02_108` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_cov_half_threads` | 0.194 | 106.5 | `dogfooding_results/mcd_tuhh_night_09_108` |
| LiTAMIN2 throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `paper_cov_half_threads` | 0.111 | 94.2 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| LiTAMIN2 throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_icp_only_half_threads` | 1.213 | 23.5 | `dogfooding_results/autoware_istanbul_open_108` |
| LiTAMIN2 throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_icp_only_half_threads` | 0.168 | 6.1 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| LiTAMIN2 throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_icp_only_half_threads` | 1.222 | 20.9 | `dogfooding_results/autoware_istanbul_open_108_b` |
| LiTAMIN2 throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `paper_icp_only_half_threads` | 0.741 | 21.2 | `dogfooding_results/autoware_istanbul_open_108_c` |
| LiTAMIN2 trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `paper_cov_half_threads` | 122.275 | 87.3 | `dogfooding_results/kitti_raw_0009_200` |
| LiTAMIN2 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_icp_only_half_threads` | 1.145 | 48.8 | `dogfooding_results/kitti_raw_0009_full` |
| LiTAMIN2 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_icp_only_half_threads` | 0.944 | 58.1 | `dogfooding_results/kitti_raw_0061_full` |
| LOAM Livox on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 92.735 | 43.8 | `dogfooding_results/kitti_raw_0009_200` |
| LOAM Livox on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 124.828 | 33.1 | `dogfooding_results/kitti_raw_0009_full` |
| LOAM Livox on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 45.818 | 50.0 | `dogfooding_results/kitti_raw_0061_200` |
| LOAM Livox on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 200.533 | 36.6 | `dogfooding_results/kitti_raw_0061_full` |
| LOAM Livox on MCD KTH day-06 sequence | `ready` | `fast` | 4.095 | 66.3 | `dogfooding_results/mcd_kth_day_06_108` |
| LOAM Livox on MCD NTU day-02 sequence | `ready` | `fast` | 0.057 | 69.0 | `dogfooding_results/mcd_ntu_day_02_108` |
| LOAM Livox on MCD TUHH night-09 sequence | `ready` | `fast` | 1.192 | 68.8 | `dogfooding_results/mcd_tuhh_night_09_108` |
| LOAM-Livox on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 92.735 | 40.2 | `dogfooding_results/kitti_raw_0009_200` |
| LOAM-Livox on the public HDL-400 reference window | `ready` | `default` | 0.079 | 52.0 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| MULLS throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 2.651 | 3.4 | `dogfooding_results/kitti_raw_0009_200` |
| MULLS throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.651 | 3.1 | `dogfooding_results/kitti_raw_0009_200` |
| MULLS throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.425 | 3.3 | `dogfooding_results/kitti_raw_0061_200` |
| MULLS throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 10.173 | 3.3 | `dogfooding_results/kitti_raw_0061_full` |
| MULLS throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast` | 6.207 | 4.1 | `dogfooding_results/mcd_kth_day_06_108` |
| MULLS throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `kitti_default` | 0.097 | 2.3 | `dogfooding_results/mcd_ntu_day_02_108` |
| MULLS throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast` | 1.206 | 3.8 | `dogfooding_results/mcd_tuhh_night_09_108` |
| MULLS throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast` | 0.874 | 9.3 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| MULLS trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 4.610 | 3.3 | `dogfooding_results/kitti_raw_0009_full` |
| NDT throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_coarse_map` | 0.279 | 36.1 | `dogfooding_results/kitti_raw_0009_200` |
| NDT throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_coarse_map` | 0.319 | 41.2 | `dogfooding_results/kitti_raw_0061_200` |
| NDT throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_coarse_map` | 0.136 | 31.2 | `dogfooding_results/mcd_kth_day_06_108` |
| NDT throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `balanced_local_map` | 0.013 | 44.9 | `dogfooding_results/mcd_ntu_day_02_108` |
| NDT throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_coarse_map` | 0.063 | 40.8 | `dogfooding_results/mcd_tuhh_night_09_108` |
| NDT throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_coarse_map` | 0.034 | 32.2 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| NDT throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.070 | 2.0 | `dogfooding_results/autoware_istanbul_open_108` |
| NDT throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_coarse_map` | 0.065 | 0.9 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| NDT throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.007 | 2.1 | `dogfooding_results/autoware_istanbul_open_108_b` |
| NDT throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.005 | 1.9 | `dogfooding_results/autoware_istanbul_open_108_c` |
| NDT trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_coarse_map` | 121.733 | 25.0 | `dogfooding_results/kitti_raw_0009_200` |
| NDT trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_coarse_map` | 0.255 | 30.9 | `dogfooding_results/kitti_raw_0009_full` |
| NDT trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_coarse_map` | 0.247 | 23.8 | `dogfooding_results/kitti_raw_0061_full` |
| Point-LIO on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 119.890 | 95.6 | `dogfooding_results/kitti_raw_0009_200` |
| Point-LIO on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 119.890 | 117.4 | `dogfooding_results/kitti_raw_0009_200` |
| Point-LIO on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 183.384 | 113.1 | `dogfooding_results/kitti_raw_0009_full` |
| Point-LIO on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 82.450 | 92.8 | `dogfooding_results/kitti_raw_0061_200` |
| Point-LIO on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 292.011 | 89.5 | `dogfooding_results/kitti_raw_0061_full` |
| Point-LIO on MCD KTH day-06 sequence | `ready` | `fast` | 7.113 | 112.7 | `dogfooding_results/mcd_kth_day_06_108` |
| Point-LIO on MCD NTU day-02 sequence | `ready` | `fast` | 0.083 | 77.3 | `dogfooding_results/mcd_ntu_day_02_108` |
| Point-LIO on MCD TUHH night-09 sequence | `ready` | `fast` | 1.116 | 88.7 | `dogfooding_results/mcd_tuhh_night_09_108` |
| Point-LIO on the public HDL-400 reference window | `ready` | `fast` | 17.929 | 69.9 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| Small-GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `balanced_local_map` | 1.624 | 76.3 | `dogfooding_results/kitti_raw_0009_200` |
| Small-GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_recent_map` | 0.471 | 83.3 | `dogfooding_results/kitti_raw_0009_200` |
| Small-GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_recent_map` | 0.639 | 78.1 | `dogfooding_results/kitti_raw_0061_200` |
| Small-GICP throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_recent_map` | 0.959 | 82.2 | `dogfooding_results/kitti_raw_0061_full` |
| Small-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 0.806 | 107.9 | `dogfooding_results/mcd_kth_day_06_108` |
| Small-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.031 | 113.8 | `dogfooding_results/mcd_ntu_day_02_108` |
| Small-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 0.250 | 107.2 | `dogfooding_results/mcd_tuhh_night_09_108` |
| Small-GICP throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 0.109 | 110.3 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| Small-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 0.437 | 92.4 | `dogfooding_results/kitti_raw_0009_full` |
| SuMa on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `default` | 2.784 | 109.4 | `dogfooding_results/kitti_raw_0009_200` |
| SuMa on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `default` | 2.784 | 62.9 | `dogfooding_results/kitti_raw_0009_200` |
| SuMa on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `dense` | 5.487 | 85.5 | `dogfooding_results/kitti_raw_0009_full` |
| SuMa on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `dense` | 1.496 | 111.2 | `dogfooding_results/kitti_raw_0061_200` |
| SuMa on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 15.381 | 110.9 | `dogfooding_results/kitti_raw_0061_full` |
| SuMa on MCD KTH day-06 sequence | `ready` | `fast` | 6.064 | 150.2 | `dogfooding_results/mcd_kth_day_06_108` |
| SuMa on MCD NTU day-02 sequence | `ready` | `dense` | 0.036 | 124.1 | `dogfooding_results/mcd_ntu_day_02_108` |
| SuMa on MCD TUHH night-09 sequence | `ready` | `default` | 1.317 | 178.1 | `dogfooding_results/mcd_tuhh_night_09_108` |
| SuMa on the public HDL-400 reference window | `ready` | `default` | 0.183 | 168.4 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| VGICP SLAM on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 1.772 | 23.7 | `dogfooding_results/kitti_raw_0009_200` |
| VGICP SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 3.948 | 22.2 | `dogfooding_results/kitti_raw_0009_full` |
| VGICP SLAM on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.799 | 31.0 | `dogfooding_results/kitti_raw_0061_200` |
| VGICP SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 5.230 | 22.8 | `dogfooding_results/kitti_raw_0061_full` |
| VGICP SLAM on MCD KTH day-06 sequence | `ready` | `fast` | 6.092 | 20.8 | `dogfooding_results/mcd_kth_day_06_108` |
| VGICP SLAM on MCD NTU day-02 sequence | `ready` | `fast` | 0.016 | 40.3 | `dogfooding_results/mcd_ntu_day_02_108` |
| VGICP SLAM on MCD TUHH night-09 sequence | `ready` | `fast` | 1.319 | 21.8 | `dogfooding_results/mcd_tuhh_night_09_108` |
| VGICP-SLAM on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 1.772 | 19.1 | `dogfooding_results/kitti_raw_0009_200` |
| VGICP-SLAM on the public HDL-400 reference window | `ready` | `fast` | 0.145 | 16.8 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| Voxel-GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `dense_recent_map` | 27.610 | 82.1 | `dogfooding_results/kitti_raw_0009_200` |
| Voxel-GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `dense_recent_map` | 0.670 | 93.9 | `dogfooding_results/kitti_raw_0009_200` |
| Voxel-GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `dense_recent_map` | 0.741 | 73.7 | `dogfooding_results/kitti_raw_0061_200` |
| Voxel-GICP throughput and accuracy trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `dense_recent_map` | 0.917 | 75.4 | `dogfooding_results/kitti_raw_0061_full` |
| Voxel-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `dense_recent_map` | 0.926 | 124.2 | `dogfooding_results/mcd_kth_day_06_108` |
| Voxel-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.121 | 117.2 | `dogfooding_results/mcd_ntu_day_02_108` |
| Voxel-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `dense_recent_map` | 0.286 | 116.4 | `dogfooding_results/mcd_tuhh_night_09_108` |
| Voxel-GICP throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `dense_recent_map` | 0.268 | 141.1 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| Voxel-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `dense_recent_map` | 0.640 | 110.1 | `dogfooding_results/kitti_raw_0009_full` |
| X-ICP on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast` | 108.748 | 81.2 | `dogfooding_results/kitti_raw_0009_200` |
| X-ICP on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `dense` | 0.139 | 102.8 | `dogfooding_results/kitti_raw_0009_200` |
| X-ICP on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `dense` | 0.130 | 92.9 | `dogfooding_results/kitti_raw_0009_full` |
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

- The stable benchmark contract now covers 248 ready problems under one CLI and one summary JSON interface.
- Each active problem keeps at least three concrete variants alive instead of collapsing immediately to a single abstraction.
- Current defaults already show non-trivial trade-offs, such as `LiTAMIN2=paper_icp_only_half_threads` at 43.7 FPS and `CT-LIO=seed_only_fast` at 0.488 m ATE on the public HDL-400 reference window.

### Gaps

- LiDAR-only methods now cover 13 repository-stored open sequences across 6 public dataset families, but external validity still rests on only 6 families.
- Reference-based and GT-backed results are now separated conceptually, but the GT-backed CT-LIO public benchmark is still blocked.
- There is no paper-ready comparison against originally reported results yet.
- Hardware-normalized reruns and confidence intervals are not exported yet.

### Next Experiments

- Add a third public dataset family, or expand the current two-family evidence with longer and less curated windows.
- Unblock or explicitly exclude GT-backed CT-LIO from the main study before writing.
- Generate a method-by-method table comparing repository defaults, challengers, and original-paper numbers.
- Export paper-ready Pareto figures from `experiments/results/*.json`.

## Track B: Artifact / Reproducibility

- **Readiness**: High (87/100)
- **Primary claim**: Localization research tooling is more reproducible when the benchmark contract is stable and variant search remains explicit, comparable, and discardable.
- **Decision**: Parallel target. This is the fastest submission path if the goal is near-term publication evidence.

### Strongest Evidence

- The repository already separates stable core (`pcd_dogfooding --summary-json`) from discardable experimental manifests.
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
