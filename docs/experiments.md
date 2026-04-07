# Experiment Results

_Generated at 2026-04-08T13:02:25+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Overview

| Problem | Status | Current Default | Best ATE [m] | Best FPS | Aggregate |
|---------|--------|-----------------|--------------|----------|-----------|
| A-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 6.105 | 5.8 | `experiments/results/aloam_kitti_raw_0009_full_matrix.json` |
| BALM2 on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 3.338 | 12.7 | `experiments/results/balm2_kitti_raw_0009_full_matrix.json` |
| BALM2 on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.366 | 13.7 | `experiments/results/balm2_kitti_raw_0009_matrix.json` |
| BALM2 on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 9.533 | 11.4 | `experiments/results/balm2_kitti_raw_0061_full_matrix.json` |
| BALM2 on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 1.883 | 13.2 | `experiments/results/balm2_kitti_raw_0061_matrix.json` |
| BALM2 on MCD KTH day-06 sequence | `ready` | `fast` | 6.184 | 13.4 | `experiments/results/balm2_mcd_kth_day_06_matrix.json` |
| BALM2 on MCD NTU day-02 sequence | `ready` | `fast` | 0.062 | 12.7 | `experiments/results/balm2_mcd_ntu_day_02_matrix.json` |
| BALM2 on MCD TUHH night-09 sequence | `ready` | `fast` | 1.270 | 14.6 | `experiments/results/balm2_mcd_tuhh_night_09_matrix.json` |
| CT-ICP throughput and drift trade-off on the public ROS1 HDL-400 window with synthesized per-point time | `ready` | `fast_window` | 1.254 | 68.5 | `experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json` |
| CT-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `fast_window` | 0.556 | 2.4 | `experiments/results/ct_icp_hdl_400_reference_b_matrix.json` |
| CT-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `fast_window` | 1.251 | 72.9 | `experiments/results/ct_icp_hdl_400_reference_matrix.json` |
| CT-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `balanced_window` | 6.820 | 3.1 | `experiments/results/ct_icp_istanbul_window_b_matrix.json` |
| CT-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `balanced_window` | 7.539 | 2.8 | `experiments/results/ct_icp_istanbul_window_c_matrix.json` |
| CT-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `balanced_window` | 4.673 | 44.9 | `experiments/results/ct_icp_kitti_raw_0009_full_matrix.json` |
| CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `balanced_window` | 1.659 | 54.8 | `experiments/results/ct_icp_kitti_raw_0009_matrix.json` |
| CT-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `balanced_window` | 1.659 | 71.4 | `experiments/results/ct_icp_kitti_raw_0009_nogt_matrix.json` |
| CT-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_window` | 6.972 | 37.6 | `experiments/results/ct_icp_kitti_raw_0061_full_matrix.json` |
| CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_window` | 1.475 | 56.9 | `experiments/results/ct_icp_kitti_raw_0061_matrix.json` |
| CT-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_window` | 6.115 | 59.8 | `experiments/results/ct_icp_mcd_kth_day_06_matrix.json` |
| CT-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_window` | 0.325 | 71.7 | `experiments/results/ct_icp_mcd_ntu_day_02_matrix.json` |
| CT-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_window` | 1.652 | 71.6 | `experiments/results/ct_icp_mcd_tuhh_night_09_matrix.json` |
| CT-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `fast_window` | 75.075 | 2.7 | `experiments/results/ct_icp_profile_matrix.json` |
| CT-LIO trade-off on the public ROS1 HDL-400 window with synthesized per-point time | `ready` | `seed_only_fast` | 0.479 | 25.8 | `experiments/results/ct_lio_hdl_400_public_ros1_synthtime_matrix.json` |
| CT-LIO GT-backed public benchmark readiness on HDL-400 ROS2 data | `blocked` | `-` | - | - | `experiments/results/ct_lio_public_readiness_matrix.json` |
| CT-LIO reference-trajectory trade-off on the public HDL-400 120-frame window | `ready` | `seed_only_fast` | 0.488 | 26.0 | `experiments/results/ct_lio_reference_profile_matrix.json` |
| DLIO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.026 | 7.3 | `experiments/results/dlio_kitti_raw_0009_full_matrix.json` |
| DLO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.026 | 7.3 | `experiments/results/dlo_kitti_raw_0009_full_matrix.json` |
| FAST-LIO2 on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.199 | 13.5 | `experiments/results/fast_lio2_kitti_raw_0009_full_matrix.json` |
| FAST-LIO2 on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.328 | 12.3 | `experiments/results/fast_lio2_kitti_raw_0009_matrix.json` |
| FAST-LIO2 on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 5.066 | 13.5 | `experiments/results/fast_lio2_kitti_raw_0061_full_matrix.json` |
| FAST-LIO2 on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.634 | 13.6 | `experiments/results/fast_lio2_kitti_raw_0061_matrix.json` |
| FAST-LIO2 on MCD KTH day-06 sequence | `ready` | `fast` | 6.072 | 12.4 | `experiments/results/fast_lio2_mcd_kth_day_06_matrix.json` |
| FAST-LIO2 on MCD NTU day-02 sequence | `ready` | `fast` | 0.025 | 23.7 | `experiments/results/fast_lio2_mcd_ntu_day_02_matrix.json` |
| FAST-LIO2 on MCD TUHH night-09 sequence | `ready` | `fast` | 1.333 | 14.2 | `experiments/results/fast_lio2_mcd_tuhh_night_09_matrix.json` |
| FAST-LIO-SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.289 | 11.3 | `experiments/results/fast_lio_slam_kitti_raw_0009_full_matrix.json` |
| FAST-LIO-SLAM on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.382 | 8.8 | `experiments/results/fast_lio_slam_kitti_raw_0009_matrix.json` |
| FAST-LIO-SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 4.945 | 9.1 | `experiments/results/fast_lio_slam_kitti_raw_0061_full_matrix.json` |
| FAST-LIO-SLAM on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.660 | 11.2 | `experiments/results/fast_lio_slam_kitti_raw_0061_matrix.json` |
| FAST-LIO-SLAM on MCD KTH day-06 sequence | `ready` | `fast` | 6.075 | 9.4 | `experiments/results/fast_lio_slam_mcd_kth_day_06_matrix.json` |
| FAST-LIO-SLAM on MCD NTU day-02 sequence | `ready` | `fast` | 0.025 | 20.4 | `experiments/results/fast_lio_slam_mcd_ntu_day_02_matrix.json` |
| FAST-LIO-SLAM on MCD TUHH night-09 sequence | `ready` | `fast` | 1.330 | 12.8 | `experiments/results/fast_lio_slam_mcd_tuhh_night_09_matrix.json` |
| F-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.452 | 28.6 | `experiments/results/floam_kitti_raw_0009_full_matrix.json` |
| GICP throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_recent_map` | 0.135 | 1.7 | `experiments/results/gicp_hdl_400_reference_b_matrix.json` |
| GICP throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 0.101 | 23.3 | `experiments/results/gicp_hdl_400_reference_matrix.json` |
| GICP throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 1.166 | 5.7 | `experiments/results/gicp_istanbul_window_b_matrix.json` |
| GICP throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 0.982 | 4.3 | `experiments/results/gicp_istanbul_window_c_matrix.json` |
| GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 1.170 | 23.0 | `experiments/results/gicp_kitti_raw_0009_full_matrix.json` |
| GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_recent_map` | 1.177 | 25.8 | `experiments/results/gicp_kitti_raw_0009_matrix.json` |
| GICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `dense_recent_map` | 1.510 | 32.0 | `experiments/results/gicp_kitti_raw_0009_nogt_matrix.json` |
| GICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_recent_map` | 1.055 | 22.8 | `experiments/results/gicp_kitti_raw_0061_full_matrix.json` |
| GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_recent_map` | 0.781 | 25.7 | `experiments/results/gicp_kitti_raw_0061_matrix.json` |
| GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 0.630 | 24.7 | `experiments/results/gicp_mcd_kth_day_06_matrix.json` |
| GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.017 | 28.7 | `experiments/results/gicp_mcd_ntu_day_02_matrix.json` |
| GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 0.317 | 31.2 | `experiments/results/gicp_mcd_tuhh_night_09_matrix.json` |
| GICP throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 0.994 | 6.3 | `experiments/results/gicp_profile_matrix.json` |
| HDL Graph SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `default` | 185.826 | 0.2 | `experiments/results/hdl_graph_slam_kitti_raw_0009_full_matrix.json` |
| HDL Graph SLAM on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `default` | 122.141 | 1.8 | `experiments/results/hdl_graph_slam_kitti_raw_0009_matrix.json` |
| HDL Graph SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential) | `skipped` | `-` | -1.000 | -1.0 | `experiments/results/hdl_graph_slam_kitti_raw_0061_full_matrix.json` |
| HDL Graph SLAM on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `default` | 118.996 | 1.5 | `experiments/results/hdl_graph_slam_kitti_raw_0061_matrix.json` |
| HDL Graph SLAM on MCD KTH day-06 sequence | `ready` | `default` | 5.113 | 4.3 | `experiments/results/hdl_graph_slam_mcd_kth_day_06_matrix.json` |
| HDL Graph SLAM on MCD NTU day-02 sequence | `ready` | `default` | 27.212 | 2.7 | `experiments/results/hdl_graph_slam_mcd_ntu_day_02_matrix.json` |
| HDL Graph SLAM on MCD TUHH night-09 sequence | `ready` | `default` | 10.818 | 3.2 | `experiments/results/hdl_graph_slam_mcd_tuhh_night_09_matrix.json` |
| ISC-LOAM on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 4.323 | 30.5 | `experiments/results/isc_loam_kitti_raw_0009_full_matrix.json` |
| ISC-LOAM on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.321 | 35.6 | `experiments/results/isc_loam_kitti_raw_0009_matrix.json` |
| ISC-LOAM on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 4.854 | 33.5 | `experiments/results/isc_loam_kitti_raw_0061_full_matrix.json` |
| ISC-LOAM on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `dense` | 0.494 | 43.5 | `experiments/results/isc_loam_kitti_raw_0061_matrix.json` |
| ISC-LOAM on MCD KTH day-06 sequence | `ready` | `fast` | 6.023 | 48.6 | `experiments/results/isc_loam_mcd_kth_day_06_matrix.json` |
| ISC-LOAM on MCD NTU day-02 sequence | `ready` | `fast` | 0.065 | 50.2 | `experiments/results/isc_loam_mcd_ntu_day_02_matrix.json` |
| ISC-LOAM on MCD TUHH night-09 sequence | `ready` | `fast` | 1.337 | 53.2 | `experiments/results/isc_loam_mcd_tuhh_night_09_matrix.json` |
| KISS-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `fast_recent_map` | 0.218 | 0.4 | `experiments/results/kiss_icp_hdl_400_reference_b_matrix.json` |
| KISS-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 1.281 | 11.3 | `experiments/results/kiss_icp_hdl_400_reference_matrix.json` |
| KISS-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `dense_local_map` | 143.921 | 3.6 | `experiments/results/kiss_icp_istanbul_window_b_matrix.json` |
| KISS-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 131.691 | 3.7 | `experiments/results/kiss_icp_istanbul_window_c_matrix.json` |
| KISS-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 5.383 | 21.9 | `experiments/results/kiss_icp_kitti_raw_0009_full_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_recent_map` | 2.412 | 24.4 | `experiments/results/kiss_icp_kitti_raw_0009_matrix.json` |
| KISS-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_recent_map` | 2.412 | 28.2 | `experiments/results/kiss_icp_kitti_raw_0009_nogt_matrix.json` |
| KISS-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_recent_map` | 4.343 | 11.2 | `experiments/results/kiss_icp_kitti_raw_0061_full_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_recent_map` | 0.679 | 28.3 | `experiments/results/kiss_icp_kitti_raw_0061_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 5.568 | 11.3 | `experiments/results/kiss_icp_mcd_kth_day_06_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `fast_recent_map` | 0.017 | 66.7 | `experiments/results/kiss_icp_mcd_ntu_day_02_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 1.104 | 24.1 | `experiments/results/kiss_icp_mcd_tuhh_night_09_matrix.json` |
| KISS-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 182.960 | 4.0 | `experiments/results/kiss_icp_profile_matrix.json` |
| LeGO-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 6.066 | 9.5 | `experiments/results/lego_loam_kitti_raw_0009_full_matrix.json` |
| LINS on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 183.380 | 123.3 | `experiments/results/lins_kitti_raw_0009_full_matrix.json` |
| LINS on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 119.861 | 120.7 | `experiments/results/lins_kitti_raw_0009_matrix.json` |
| LINS on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 291.877 | 104.7 | `experiments/results/lins_kitti_raw_0061_full_matrix.json` |
| LINS on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 82.383 | 115.2 | `experiments/results/lins_kitti_raw_0061_matrix.json` |
| LINS on MCD KTH day-06 sequence | `ready` | `fast` | 7.120 | 166.1 | `experiments/results/lins_mcd_kth_day_06_matrix.json` |
| LINS on MCD NTU day-02 sequence | `ready` | `dense` | 0.111 | 147.2 | `experiments/results/lins_mcd_ntu_day_02_matrix.json` |
| LINS on MCD TUHH night-09 sequence | `ready` | `fast` | 1.147 | 173.4 | `experiments/results/lins_mcd_tuhh_night_09_matrix.json` |
| LIO-SAM on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.259 | 20.2 | `experiments/results/lio_sam_kitti_raw_0009_full_matrix.json` |
| LIO-SAM on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 2.579 | 24.9 | `experiments/results/lio_sam_kitti_raw_0009_matrix.json` |
| LIO-SAM on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 5.236 | 21.2 | `experiments/results/lio_sam_kitti_raw_0061_full_matrix.json` |
| LIO-SAM on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.577 | 27.1 | `experiments/results/lio_sam_kitti_raw_0061_matrix.json` |
| LIO-SAM on MCD KTH day-06 sequence | `ready` | `fast` | 6.020 | 29.7 | `experiments/results/lio_sam_mcd_kth_day_06_matrix.json` |
| LIO-SAM on MCD NTU day-02 sequence | `ready` | `fast` | 0.045 | 31.7 | `experiments/results/lio_sam_mcd_ntu_day_02_matrix.json` |
| LIO-SAM on MCD TUHH night-09 sequence | `ready` | `fast` | 1.314 | 30.0 | `experiments/results/lio_sam_mcd_tuhh_night_09_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_icp_only_half_threads` | 0.168 | 6.1 | `experiments/results/litamin2_hdl_400_reference_b_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `paper_cov_half_threads` | 0.111 | 94.2 | `experiments/results/litamin2_hdl_400_reference_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_icp_only_half_threads` | 1.222 | 20.9 | `experiments/results/litamin2_istanbul_window_b_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `paper_icp_only_half_threads` | 0.741 | 21.2 | `experiments/results/litamin2_istanbul_window_c_matrix.json` |
| LiTAMIN2 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_icp_only_half_threads` | 1.145 | 48.8 | `experiments/results/litamin2_kitti_raw_0009_full_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `paper_icp_only_half_threads` | 1.053 | 43.7 | `experiments/results/litamin2_kitti_raw_0009_matrix.json` |
| LiTAMIN2 trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `paper_cov_half_threads` | 122.275 | 87.3 | `experiments/results/litamin2_kitti_raw_0009_nogt_matrix.json` |
| LiTAMIN2 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_icp_only_half_threads` | 0.944 | 58.1 | `experiments/results/litamin2_kitti_raw_0061_full_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_cov_half_threads` | 0.511 | 68.6 | `experiments/results/litamin2_kitti_raw_0061_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_cov_half_threads` | 0.401 | 64.2 | `experiments/results/litamin2_mcd_kth_day_06_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `paper_icp_only_half_threads` | 0.045 | 105.6 | `experiments/results/litamin2_mcd_ntu_day_02_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_cov_half_threads` | 0.194 | 106.5 | `experiments/results/litamin2_mcd_tuhh_night_09_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_icp_only_half_threads` | 1.213 | 23.5 | `experiments/results/litamin2_profile_matrix.json` |
| LOAM Livox on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 124.828 | 33.1 | `experiments/results/loam_livox_kitti_raw_0009_full_matrix.json` |
| LOAM Livox on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 92.735 | 43.8 | `experiments/results/loam_livox_kitti_raw_0009_matrix.json` |
| LOAM Livox on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 200.533 | 36.6 | `experiments/results/loam_livox_kitti_raw_0061_full_matrix.json` |
| LOAM Livox on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 45.818 | 50.0 | `experiments/results/loam_livox_kitti_raw_0061_matrix.json` |
| LOAM Livox on MCD KTH day-06 sequence | `ready` | `fast` | 4.095 | 66.3 | `experiments/results/loam_livox_mcd_kth_day_06_matrix.json` |
| LOAM Livox on MCD NTU day-02 sequence | `ready` | `fast` | 0.057 | 69.0 | `experiments/results/loam_livox_mcd_ntu_day_02_matrix.json` |
| LOAM Livox on MCD TUHH night-09 sequence | `ready` | `fast` | 1.192 | 68.8 | `experiments/results/loam_livox_mcd_tuhh_night_09_matrix.json` |
| MULLS trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 4.610 | 3.3 | `experiments/results/mulls_kitti_raw_0009_full_matrix.json` |
| NDT throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_coarse_map` | 0.065 | 0.9 | `experiments/results/ndt_hdl_400_reference_b_matrix.json` |
| NDT throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_coarse_map` | 0.034 | 32.2 | `experiments/results/ndt_hdl_400_reference_matrix.json` |
| NDT throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.007 | 2.1 | `experiments/results/ndt_istanbul_window_b_matrix.json` |
| NDT throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.005 | 1.9 | `experiments/results/ndt_istanbul_window_c_matrix.json` |
| NDT trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_coarse_map` | 0.255 | 30.9 | `experiments/results/ndt_kitti_raw_0009_full_matrix.json` |
| NDT throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_coarse_map` | 0.279 | 36.1 | `experiments/results/ndt_kitti_raw_0009_matrix.json` |
| NDT trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_coarse_map` | 121.733 | 25.0 | `experiments/results/ndt_kitti_raw_0009_nogt_matrix.json` |
| NDT trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_coarse_map` | 0.247 | 23.8 | `experiments/results/ndt_kitti_raw_0061_full_matrix.json` |
| NDT throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_coarse_map` | 0.319 | 41.2 | `experiments/results/ndt_kitti_raw_0061_matrix.json` |
| NDT throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_coarse_map` | 0.136 | 31.2 | `experiments/results/ndt_mcd_kth_day_06_matrix.json` |
| NDT throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `balanced_local_map` | 0.013 | 44.9 | `experiments/results/ndt_mcd_ntu_day_02_matrix.json` |
| NDT throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_coarse_map` | 0.063 | 40.8 | `experiments/results/ndt_mcd_tuhh_night_09_matrix.json` |
| NDT throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.070 | 2.0 | `experiments/results/ndt_profile_matrix.json` |
| Point-LIO on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 183.384 | 113.1 | `experiments/results/point_lio_kitti_raw_0009_full_matrix.json` |
| Point-LIO on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 119.890 | 117.4 | `experiments/results/point_lio_kitti_raw_0009_matrix.json` |
| Point-LIO on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 292.011 | 89.5 | `experiments/results/point_lio_kitti_raw_0061_full_matrix.json` |
| Point-LIO on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 82.450 | 92.8 | `experiments/results/point_lio_kitti_raw_0061_matrix.json` |
| Point-LIO on MCD KTH day-06 sequence | `ready` | `fast` | 7.113 | 112.7 | `experiments/results/point_lio_mcd_kth_day_06_matrix.json` |
| Point-LIO on MCD NTU day-02 sequence | `ready` | `fast` | 0.083 | 77.3 | `experiments/results/point_lio_mcd_ntu_day_02_matrix.json` |
| Point-LIO on MCD TUHH night-09 sequence | `ready` | `fast` | 1.116 | 88.7 | `experiments/results/point_lio_mcd_tuhh_night_09_matrix.json` |
| Small-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 0.437 | 92.4 | `experiments/results/small_gicp_kitti_raw_0009_full_matrix.json` |
| Small-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 0.806 | 107.9 | `experiments/results/small_gicp_mcd_kth_day_06_matrix.json` |
| Small-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.031 | 113.8 | `experiments/results/small_gicp_mcd_ntu_day_02_matrix.json` |
| Small-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 0.250 | 107.2 | `experiments/results/small_gicp_mcd_tuhh_night_09_matrix.json` |
| SuMa on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `dense` | 5.487 | 85.5 | `experiments/results/suma_kitti_raw_0009_full_matrix.json` |
| SuMa on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `default` | 2.784 | 62.9 | `experiments/results/suma_kitti_raw_0009_matrix.json` |
| SuMa on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 15.381 | 110.9 | `experiments/results/suma_kitti_raw_0061_full_matrix.json` |
| SuMa on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `dense` | 1.496 | 111.2 | `experiments/results/suma_kitti_raw_0061_matrix.json` |
| SuMa on MCD KTH day-06 sequence | `ready` | `fast` | 6.064 | 150.2 | `experiments/results/suma_mcd_kth_day_06_matrix.json` |
| SuMa on MCD NTU day-02 sequence | `ready` | `dense` | 0.036 | 124.1 | `experiments/results/suma_mcd_ntu_day_02_matrix.json` |
| SuMa on MCD TUHH night-09 sequence | `ready` | `default` | 1.317 | 178.1 | `experiments/results/suma_mcd_tuhh_night_09_matrix.json` |
| VGICP SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 3.948 | 22.2 | `experiments/results/vgicp_slam_kitti_raw_0009_full_matrix.json` |
| VGICP SLAM on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast` | 1.772 | 23.7 | `experiments/results/vgicp_slam_kitti_raw_0009_matrix.json` |
| VGICP SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 5.230 | 22.8 | `experiments/results/vgicp_slam_kitti_raw_0061_full_matrix.json` |
| VGICP SLAM on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.799 | 31.0 | `experiments/results/vgicp_slam_kitti_raw_0061_matrix.json` |
| VGICP SLAM on MCD KTH day-06 sequence | `ready` | `fast` | 6.092 | 20.8 | `experiments/results/vgicp_slam_mcd_kth_day_06_matrix.json` |
| VGICP SLAM on MCD NTU day-02 sequence | `ready` | `fast` | 0.016 | 40.3 | `experiments/results/vgicp_slam_mcd_ntu_day_02_matrix.json` |
| VGICP SLAM on MCD TUHH night-09 sequence | `ready` | `fast` | 1.319 | 21.8 | `experiments/results/vgicp_slam_mcd_tuhh_night_09_matrix.json` |
| Voxel-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `dense_recent_map` | 0.640 | 110.1 | `experiments/results/voxel_gicp_kitti_raw_0009_full_matrix.json` |
| Voxel-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `dense_recent_map` | 0.926 | 124.2 | `experiments/results/voxel_gicp_mcd_kth_day_06_matrix.json` |
| Voxel-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.121 | 117.2 | `experiments/results/voxel_gicp_mcd_ntu_day_02_matrix.json` |
| Voxel-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `dense_recent_map` | 0.286 | 116.4 | `experiments/results/voxel_gicp_mcd_tuhh_night_09_matrix.json` |
| X-ICP on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `dense` | 0.130 | 92.9 | `experiments/results/xicp_kitti_raw_0009_full_matrix.json` |
| X-ICP on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `dense` | 0.139 | 102.8 | `experiments/results/xicp_kitti_raw_0009_matrix.json` |
| X-ICP on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast` | 0.128 | 104.6 | `experiments/results/xicp_kitti_raw_0061_full_matrix.json` |
| X-ICP on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast` | 0.098 | 102.0 | `experiments/results/xicp_kitti_raw_0061_matrix.json` |
| X-ICP on MCD KTH day-06 sequence | `ready` | `fast` | 0.305 | 84.8 | `experiments/results/xicp_mcd_kth_day_06_matrix.json` |
| X-ICP on MCD NTU day-02 sequence | `ready` | `dense` | 0.095 | 81.8 | `experiments/results/xicp_mcd_ntu_day_02_matrix.json` |
| X-ICP on MCD TUHH night-09 sequence | `ready` | `dense` | 0.081 | 100.4 | `experiments/results/xicp_mcd_tuhh_night_09_matrix.json` |

## A-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `aloam_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which A-LOAM profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `aloam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/aloam_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| KITTI default | balanced | 6.463 | 2.6 | 70.0 | 4.65 | 4.75 | Keep as reference variant |
| Fast | throughput-oriented | 6.105 | 5.8 | 100.0 | 4.30 | 4.50 | Adopt as current default |
| Dense | accuracy-oriented | 6.187 | 2.7 | 72.3 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 5.8 FPS.
3. `fast` is the most accurate observed variant at 6.105 m ATE.

### Variant Notes

#### `kitti_default`

- Intent: Baseline settings for KITTI (n_scans=64).
- CLI args: `--aloam-kitti-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods aloam --summary-json experiments/results/runs/aloam_kitti_raw_0009_full_matrix/kitti_default/summary.json --aloam-kitti-profile`
- Summary: `experiments/results/runs/aloam_kitti_raw_0009_full_matrix/kitti_default/summary.json`
- Log: `experiments/results/runs/aloam_kitti_raw_0009_full_matrix/kitti_default/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Odometry+mapping pipeline (no GT seed). KITTI assumes n_scans=64.

#### `fast`

- Intent: Reduce feature density and solver work for speed.
- CLI args: `--aloam-kitti-profile --aloam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods aloam --summary-json experiments/results/runs/aloam_kitti_raw_0009_full_matrix/fast/summary.json --aloam-kitti-profile --aloam-fast-profile`
- Summary: `experiments/results/runs/aloam_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/aloam_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Odometry+mapping pipeline (no GT seed). KITTI assumes n_scans=64.

#### `dense`

- Intent: Increase feature density and solver iterations for accuracy.
- CLI args: `--aloam-kitti-profile --aloam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods aloam --summary-json experiments/results/runs/aloam_kitti_raw_0009_full_matrix/dense/summary.json --aloam-kitti-profile --aloam-dense-profile`
- Summary: `experiments/results/runs/aloam_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/aloam_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Odometry+mapping pipeline (no GT seed). KITTI assumes n_scans=64.


## BALM2 on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `balm2_kitti_raw_0009_full`
- **Question**: How does BALM2 perform on the KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `balm2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/balm2_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 4.576 | 1.6 | 42.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 3.338 | 12.7 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 5.975 | 0.7 | 30.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 12.7 FPS.
3. `fast` is the most accurate observed variant at 3.338 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_kitti_raw_0009_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/balm2_kitti_raw_0009_full_matrix/default/summary.json`
- Log: `experiments/results/runs/balm2_kitti_raw_0009_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce feature density and bundle-adjustment effort for a lighter mapping window.
- CLI args: `--balm2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_kitti_raw_0009_full_matrix/fast/summary.json --balm2-fast-profile`
- Summary: `experiments/results/runs/balm2_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/balm2_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase feature density and keep a slightly larger optimization window for a more exhaustive mapping solve without the previous runtime blow-up.
- CLI args: `--balm2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_kitti_raw_0009_full_matrix/dense/summary.json --balm2-dense-profile`
- Summary: `experiments/results/runs/balm2_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/balm2_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).


## BALM2 on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `balm2_kitti_raw_0009`
- **Question**: How does BALM2 perform on the KITTI Raw drive 0009 (200 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `balm2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/balm2_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 3.419 | 1.7 | 41.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 2.366 | 13.7 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 3.608 | 0.6 | 34.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 13.7 FPS.
3. `fast` is the most accurate observed variant at 2.366 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_kitti_raw_0009_matrix/default/summary.json`
- Summary: `experiments/results/runs/balm2_kitti_raw_0009_matrix/default/summary.json`
- Log: `experiments/results/runs/balm2_kitti_raw_0009_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce feature density and bundle-adjustment effort for a lighter mapping window.
- CLI args: `--balm2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_kitti_raw_0009_matrix/fast/summary.json --balm2-fast-profile`
- Summary: `experiments/results/runs/balm2_kitti_raw_0009_matrix/fast/summary.json`
- Log: `experiments/results/runs/balm2_kitti_raw_0009_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase feature density and keep a slightly larger optimization window for a more exhaustive mapping solve without the previous runtime blow-up.
- CLI args: `--balm2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_kitti_raw_0009_matrix/dense/summary.json --balm2-dense-profile`
- Summary: `experiments/results/runs/balm2_kitti_raw_0009_matrix/dense/summary.json`
- Log: `experiments/results/runs/balm2_kitti_raw_0009_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).


## BALM2 on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `balm2_kitti_raw_0061_full`
- **Question**: How does BALM2 perform on the KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `balm2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/balm2_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 10.035 | 1.6 | 54.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 15.574 | 11.4 | 80.6 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 9.533 | 0.8 | 53.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 11.4 FPS.
3. `dense` is the most accurate observed variant at 9.533 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_kitti_raw_0061_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/balm2_kitti_raw_0061_full_matrix/default/summary.json`
- Log: `experiments/results/runs/balm2_kitti_raw_0061_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce feature density and bundle-adjustment effort for a lighter mapping window.
- CLI args: `--balm2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_kitti_raw_0061_full_matrix/fast/summary.json --balm2-fast-profile`
- Summary: `experiments/results/runs/balm2_kitti_raw_0061_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/balm2_kitti_raw_0061_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase feature density and keep a slightly larger optimization window for a more exhaustive mapping solve without the previous runtime blow-up.
- CLI args: `--balm2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_kitti_raw_0061_full_matrix/dense/summary.json --balm2-dense-profile`
- Summary: `experiments/results/runs/balm2_kitti_raw_0061_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/balm2_kitti_raw_0061_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).


## BALM2 on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `balm2_kitti_raw_0061`
- **Question**: How does BALM2 perform on the KITTI Raw drive 0061 (200 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `balm2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/balm2_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 2.721 | 1.8 | 41.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 2.926 | 13.2 | 82.2 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 1.883 | 0.7 | 52.8 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 13.2 FPS.
3. `dense` is the most accurate observed variant at 1.883 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_kitti_raw_0061_matrix/default/summary.json`
- Summary: `experiments/results/runs/balm2_kitti_raw_0061_matrix/default/summary.json`
- Log: `experiments/results/runs/balm2_kitti_raw_0061_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce feature density and bundle-adjustment effort for a lighter mapping window.
- CLI args: `--balm2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_kitti_raw_0061_matrix/fast/summary.json --balm2-fast-profile`
- Summary: `experiments/results/runs/balm2_kitti_raw_0061_matrix/fast/summary.json`
- Log: `experiments/results/runs/balm2_kitti_raw_0061_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase feature density and keep a slightly larger optimization window for a more exhaustive mapping solve without the previous runtime blow-up.
- CLI args: `--balm2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_kitti_raw_0061_matrix/dense/summary.json --balm2-dense-profile`
- Summary: `experiments/results/runs/balm2_kitti_raw_0061_matrix/dense/summary.json`
- Log: `experiments/results/runs/balm2_kitti_raw_0061_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).


## BALM2 on MCD KTH day-06 sequence

- **Problem ID**: `balm2_mcd_kth_day_06`
- **Question**: How does BALM2 perform on the MCD KTH day-06 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `balm2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/balm2_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 6.193 | 2.0 | 57.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 6.227 | 13.4 | 99.7 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 6.184 | 0.7 | 52.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 13.4 FPS.
3. `dense` is the most accurate observed variant at 6.184 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_mcd_kth_day_06_matrix/default/summary.json`
- Summary: `experiments/results/runs/balm2_mcd_kth_day_06_matrix/default/summary.json`
- Log: `experiments/results/runs/balm2_mcd_kth_day_06_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce feature density and bundle-adjustment effort for a lighter mapping window.
- CLI args: `--balm2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_mcd_kth_day_06_matrix/fast/summary.json --balm2-fast-profile`
- Summary: `experiments/results/runs/balm2_mcd_kth_day_06_matrix/fast/summary.json`
- Log: `experiments/results/runs/balm2_mcd_kth_day_06_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase feature density and keep a slightly larger optimization window for a more exhaustive mapping solve without the previous runtime blow-up.
- CLI args: `--balm2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_mcd_kth_day_06_matrix/dense/summary.json --balm2-dense-profile`
- Summary: `experiments/results/runs/balm2_mcd_kth_day_06_matrix/dense/summary.json`
- Log: `experiments/results/runs/balm2_mcd_kth_day_06_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).


## BALM2 on MCD NTU day-02 sequence

- **Problem ID**: `balm2_mcd_ntu_day_02`
- **Question**: How does BALM2 perform on the MCD NTU day-02 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `balm2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/balm2_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 0.062 | 1.6 | 56.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.172 | 12.7 | 68.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 0.069 | 0.6 | 47.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 12.7 FPS.
3. `default` is the most accurate observed variant at 0.062 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_mcd_ntu_day_02_matrix/default/summary.json`
- Summary: `experiments/results/runs/balm2_mcd_ntu_day_02_matrix/default/summary.json`
- Log: `experiments/results/runs/balm2_mcd_ntu_day_02_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce feature density and bundle-adjustment effort for a lighter mapping window.
- CLI args: `--balm2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_mcd_ntu_day_02_matrix/fast/summary.json --balm2-fast-profile`
- Summary: `experiments/results/runs/balm2_mcd_ntu_day_02_matrix/fast/summary.json`
- Log: `experiments/results/runs/balm2_mcd_ntu_day_02_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase feature density and keep a slightly larger optimization window for a more exhaustive mapping solve without the previous runtime blow-up.
- CLI args: `--balm2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_mcd_ntu_day_02_matrix/dense/summary.json --balm2-dense-profile`
- Summary: `experiments/results/runs/balm2_mcd_ntu_day_02_matrix/dense/summary.json`
- Log: `experiments/results/runs/balm2_mcd_ntu_day_02_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).


## BALM2 on MCD TUHH night-09 sequence

- **Problem ID**: `balm2_mcd_tuhh_night_09`
- **Question**: How does BALM2 perform on the MCD TUHH night-09 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `balm2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/balm2_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 1.270 | 2.0 | 57.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 1.698 | 14.6 | 87.4 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 1.315 | 0.7 | 50.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 14.6 FPS.
3. `default` is the most accurate observed variant at 1.270 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_mcd_tuhh_night_09_matrix/default/summary.json`
- Summary: `experiments/results/runs/balm2_mcd_tuhh_night_09_matrix/default/summary.json`
- Log: `experiments/results/runs/balm2_mcd_tuhh_night_09_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce feature density and bundle-adjustment effort for a lighter mapping window.
- CLI args: `--balm2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_mcd_tuhh_night_09_matrix/fast/summary.json --balm2-fast-profile`
- Summary: `experiments/results/runs/balm2_mcd_tuhh_night_09_matrix/fast/summary.json`
- Log: `experiments/results/runs/balm2_mcd_tuhh_night_09_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase feature density and keep a slightly larger optimization window for a more exhaustive mapping solve without the previous runtime blow-up.
- CLI args: `--balm2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods balm2 --summary-json experiments/results/runs/balm2_mcd_tuhh_night_09_matrix/dense/summary.json --balm2-dense-profile`
- Summary: `experiments/results/runs/balm2_mcd_tuhh_night_09_matrix/dense/summary.json`
- Log: `experiments/results/runs/balm2_mcd_tuhh_night_09_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).


## CT-ICP throughput and drift trade-off on the public ROS1 HDL-400 window with synthesized per-point time

- **Problem ID**: `ct_icp_profile_tradeoff_hdl_400_public_ros1_synthtime`
- **Question**: Which CT-ICP odometry profile should stay as the current default when the public ROS1 HDL-400 bag is reconstructed with scan-order synthetic point timestamps?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 1.604 | 40.3 | 68.5 | 5.00 | 5.00 | Keep as active challenger |
| Fast window | throughput-oriented | 2.465 | 68.5 | 75.4 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 1.254 | 16.6 | 62.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 68.5 FPS.
3. `dense_window` is the most accurate observed variant at 1.254 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index experiments/reference_data/hdl_400_public_reference.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_public_ros1_synthtime_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_hdl_400_public_ros1_synthtime_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_public_ros1_synthtime_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index experiments/reference_data/hdl_400_public_reference.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_public_ros1_synthtime_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_hdl_400_public_ros1_synthtime_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_public_ros1_synthtime_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index experiments/reference_data/hdl_400_public_reference.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_public_ros1_synthtime_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_hdl_400_public_ros1_synthtime_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_public_ros1_synthtime_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and drift trade-off on the second public HDL-400 reference window

- **Problem ID**: `ct_icp_profile_tradeoff_hdl_400_reference_b`
- **Question**: Which CT-ICP odometry profile should stay as the current default on a deeper HDL-400 public reference set while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120_b`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference_b.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_hdl_400_reference_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 0.873 | 1.2 | 57.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 1.211 | 2.4 | 73.0 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 0.556 | 0.5 | 60.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 2.4 FPS.
3. `dense_window` is the most accurate observed variant at 0.556 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_reference_b_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and drift trade-off on the public HDL-400 reference window

- **Problem ID**: `ct_icp_profile_tradeoff_hdl_400_reference`
- **Question**: Which CT-ICP odometry profile should stay as the current default on a different public dataset family while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_hdl_400_reference_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 1.668 | 22.6 | 53.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 2.582 | 72.9 | 74.2 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 1.251 | 19.5 | 63.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 72.9 FPS.
3. `dense_window` is the most accurate observed variant at 1.251 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_reference_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_hdl_400_reference_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_reference_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_reference_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_hdl_400_reference_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_reference_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_hdl_400_reference_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_hdl_400_reference_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_hdl_400_reference_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence

- **Problem ID**: `ct_icp_profile_tradeoff_istanbul_window_b`
- **Question**: Which CT-ICP odometry profile should stay as the current default on a second public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_b`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_b_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_istanbul_window_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 6.820 | 1.6 | 75.3 | 5.00 | 5.00 | Adopt as current default |
| Fast window | throughput-oriented | 35.035 | 3.1 | 59.7 | 4.65 | 4.75 | Keep as reference variant |
| Dense window | drift-oriented | 22.322 | 0.6 | 25.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `balanced_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 3.1 FPS.
3. `balanced_window` is the most accurate observed variant at 6.820 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_istanbul_window_b_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_istanbul_window_b_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_istanbul_window_b_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_istanbul_window_b_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_istanbul_window_b_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_istanbul_window_b_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_istanbul_window_b_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_istanbul_window_b_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_istanbul_window_b_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence

- **Problem ID**: `ct_icp_profile_tradeoff_istanbul_window_c`
- **Question**: Which CT-ICP odometry profile should stay as the current default on a third public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_c`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_c_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_istanbul_window_c_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 7.539 | 1.3 | 73.4 | 5.00 | 5.00 | Adopt as current default |
| Fast window | throughput-oriented | 42.692 | 2.8 | 58.8 | 4.65 | 4.75 | Keep as reference variant |
| Dense window | drift-oriented | 15.285 | 0.5 | 33.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `balanced_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 2.8 FPS.
3. `balanced_window` is the most accurate observed variant at 7.539 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_istanbul_window_c_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_istanbul_window_c_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_istanbul_window_c_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_istanbul_window_c_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_istanbul_window_c_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_istanbul_window_c_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_istanbul_window_c_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_istanbul_window_c_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_istanbul_window_c_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `ct_icp_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which CT-ICP profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 4.673 | 40.6 | 95.2 | 5.00 | 5.00 | Adopt as current default |
| Fast window | throughput-oriented | 5.853 | 44.9 | 89.9 | 4.65 | 4.75 | Keep as active challenger |
| Dense window | drift-oriented | 5.658 | 17.4 | 60.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `balanced_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 44.9 FPS.
3. `balanced_window` is the most accurate observed variant at 4.673 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Fast.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Dense.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_full_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `ct_icp_profile_tradeoff_kitti_raw_0009`
- **Question**: Which CT-ICP profile should stay as the current default on KITTI Raw drive 0009?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 1.659 | 34.4 | 81.4 | 5.00 | 5.00 | Adopt as current default |
| Fast window | throughput-oriented | 2.728 | 54.8 | 80.4 | 4.65 | 4.75 | Keep as active challenger |
| Dense window | drift-oriented | 2.653 | 17.6 | 47.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `balanced_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 54.8 FPS.
3. `balanced_window` is the most accurate observed variant at 1.659 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- **Problem ID**: `ct_icp_profile_tradeoff_kitti_raw_0009_nogt`
- **Question**: Which CT-ICP profile on KITTI Raw drive 0009 (200 frames, no GT seed)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_raw_0009_nogt_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 1.659 | 44.5 | 81.2 | 4.65 | 4.75 | Adopt as current default |
| Fast window | throughput-oriented | 2.728 | 71.4 | 80.4 | 4.30 | 4.50 | Keep as active challenger |
| Dense window | drift-oriented | 2.653 | 26.8 | 50.0 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `balanced_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 71.4 FPS.
3. `balanced_window` is the most accurate observed variant at 1.659 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Default.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/balanced_window/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/balanced_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `fast_window`

- Intent: Fast.
- CLI args: `--ct-icp-fast-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/fast_window/summary.json --ct-icp-fast-profile --no-gt-seed`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/fast_window/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Dense.
- CLI args: `--ct-icp-dense-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/dense_window/summary.json --ct-icp-dense-profile --no-gt-seed`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0009_nogt_matrix/dense_window/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `ct_icp_profile_tradeoff_kitti_raw_0061_full`
- **Question**: Which CT-ICP profile on KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 20.357 | 21.3 | 45.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 6.972 | 37.6 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 8.894 | 13.3 | 56.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 37.6 FPS.
3. `fast_window` is the most accurate observed variant at 6.972 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Fast.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Dense.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0061_full_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `ct_icp_profile_tradeoff_kitti_raw_0061`
- **Question**: Which CT-ICP profile should stay as the current default on KITTI Raw drive 0061?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 5.029 | 33.2 | 43.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 1.475 | 56.9 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 1.961 | 18.7 | 54.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 56.9 FPS.
3. `fast_window` is the most accurate observed variant at 1.475 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0061_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0061_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0061_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0061_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0061_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0061_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_kitti_raw_0061_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_kitti_raw_0061_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_kitti_raw_0061_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `ct_icp_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which CT-ICP profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 6.295 | 34.6 | 77.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 6.525 | 59.8 | 96.9 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 6.115 | 15.3 | 62.8 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 59.8 FPS.
3. `dense_window` is the most accurate observed variant at 6.115 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_kth_day_06_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `ct_icp_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which CT-ICP profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 0.926 | 39.2 | 44.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 2.498 | 71.7 | 56.5 | 4.65 | 4.75 | Keep as reference variant |
| Dense window | drift-oriented | 0.325 | 22.0 | 65.3 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 71.7 FPS.
3. `dense_window` is the most accurate observed variant at 0.325 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_ntu_day_02_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `ct_icp_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which CT-ICP profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 2.387 | 39.2 | 62.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 3.553 | 71.6 | 73.3 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 1.652 | 19.4 | 63.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 71.6 FPS.
3. `dense_window` is the most accurate observed variant at 1.652 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_mcd_tuhh_night_09_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-ICP throughput and drift trade-off on the repository-stored Istanbul sequence

- **Problem ID**: `ct_icp_profile_tradeoff`
- **Question**: Which CT-ICP odometry profile should stay as the current default while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_icp_profile_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced window | balanced | 75.075 | 1.4 | 74.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast window | throughput-oriented | 79.761 | 2.7 | 97.1 | 4.65 | 4.75 | Adopt as current default |
| Dense window | drift-oriented | 76.350 | 0.5 | 58.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_window` is the current default for this problem.
2. `fast_window` is the fastest observed variant at 2.7 FPS.
3. `balanced_window` is the most accurate observed variant at 75.075 m ATE.

### Variant Notes

#### `balanced_window`

- Intent: Keep the current repository default as the continuous-time baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_profile_matrix/balanced_window/summary.json`
- Summary: `experiments/results/runs/ct_icp_profile_matrix/balanced_window/summary.json`
- Log: `experiments/results/runs/ct_icp_profile_matrix/balanced_window/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_window`

- Intent: Shrink the working set and point budget to improve FPS.
- CLI args: `--ct-icp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_profile_matrix/fast_window/summary.json --ct-icp-fast-profile`
- Summary: `experiments/results/runs/ct_icp_profile_matrix/fast_window/summary.json`
- Log: `experiments/results/runs/ct_icp_profile_matrix/fast_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_window`

- Intent: Increase point density and window richness to preserve more structure.
- CLI args: `--ct-icp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods ct_icp --summary-json experiments/results/runs/ct_icp_profile_matrix/dense_window/summary.json --ct-icp-dense-profile`
- Summary: `experiments/results/runs/ct_icp_profile_matrix/dense_window/summary.json`
- Log: `experiments/results/runs/ct_icp_profile_matrix/dense_window/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## CT-LIO trade-off on the public ROS1 HDL-400 window with synthesized per-point time

- **Problem ID**: `ct_lio_hdl_400_public_ros1_synthtime_tradeoff`
- **Question**: Which CT-LIO profile should stay as the current default when the public ROS1 HDL-400 bag is reconstructed with scan-order synthetic point timestamps?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_lio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_lio_hdl_400_public_ros1_synthtime_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| IMU preintegration default | balanced | 2.203 | 8.4 | 27.1 | 4.05 | 4.35 | Keep as reference variant |
| Seed-only fast | throughput-oriented | 0.479 | 19.6 | 88.0 | 3.80 | 4.20 | Adopt as current default |
| History smoother dense | accuracy-oriented | 2.614 | 25.8 | 59.2 | 3.45 | 3.95 | Keep as reference variant |

### Observations

1. `seed_only_fast` is the current default for this problem.
2. `history_smoother_dense` is the fastest observed variant at 25.8 FPS.
3. `seed_only_fast` is the most accurate observed variant at 0.479 m ATE.

### Variant Notes

#### `imu_preintegration_default`

- Intent: Baseline CT-LIO profile with IMU bias estimation and a short fixed-lag prior.
- CLI args: `--ct-lio-estimate-bias --ct-lio-fixed-lag-window 4`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index experiments/reference_data/hdl_400_public_reference.csv --methods ct_lio --summary-json experiments/results/runs/ct_lio_hdl_400_public_ros1_synthtime_matrix/imu_preintegration_default/summary.json --ct-lio-estimate-bias --ct-lio-fixed-lag-window 4`
- Summary: `experiments/results/runs/ct_lio_hdl_400_public_ros1_synthtime_matrix/imu_preintegration_default/summary.json`
- Log: `experiments/results/runs/ct_lio_hdl_400_public_ros1_synthtime_matrix/imu_preintegration_default/run.log`
- Readability proxy: 4.05 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.35 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: No extra method note.

#### `seed_only_fast`

- Intent: Keep the IMU seed but avoid the heavier lag smoother path.
- CLI args: `--ct-lio-fixed-lag-window 2 --ct-lio-fixed-lag-outer-iterations 1`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index experiments/reference_data/hdl_400_public_reference.csv --methods ct_lio --summary-json experiments/results/runs/ct_lio_hdl_400_public_ros1_synthtime_matrix/seed_only_fast/summary.json --ct-lio-fixed-lag-window 2 --ct-lio-fixed-lag-outer-iterations 1`
- Summary: `experiments/results/runs/ct_lio_hdl_400_public_ros1_synthtime_matrix/seed_only_fast/summary.json`
- Log: `experiments/results/runs/ct_lio_hdl_400_public_ros1_synthtime_matrix/seed_only_fast/run.log`
- Readability proxy: 3.80 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.20 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: No extra method note.

#### `history_smoother_dense`

- Intent: Use a slightly longer lag window and one extra relinearization pass for stronger motion regularization without the unbounded smoother cost.
- CLI args: `--ct-lio-estimate-bias --ct-lio-fixed-lag-window 5 --ct-lio-fixed-lag-outer-iterations 2`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index experiments/reference_data/hdl_400_public_reference.csv --methods ct_lio --summary-json experiments/results/runs/ct_lio_hdl_400_public_ros1_synthtime_matrix/history_smoother_dense/summary.json --ct-lio-estimate-bias --ct-lio-fixed-lag-window 5 --ct-lio-fixed-lag-outer-iterations 2`
- Summary: `experiments/results/runs/ct_lio_hdl_400_public_ros1_synthtime_matrix/history_smoother_dense/summary.json`
- Log: `experiments/results/runs/ct_lio_hdl_400_public_ros1_synthtime_matrix/history_smoother_dense/run.log`
- Readability proxy: 3.45 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.95 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: No extra method note.


## CT-LIO GT-backed public benchmark readiness on HDL-400 ROS2 data

- **Problem ID**: `ct_lio_public_gt_readiness`
- **Question**: What still blocks a GT-backed CT-LIO public benchmark contract after the reference-based HDL-400 comparison is available?
- **Status**: `blocked`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_60`
- **Reference CSV**: `MISSING: repository-usable open GT CSV for HDL-400 is not prepared yet`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_lio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_lio_public_readiness_matrix.json`
- **Blocker**: A reference-based CT-LIO comparison now exists separately, but this repository still does not ship an independently curated GT CSV aligned to the extracted HDL-400 LiDAR+IMU sequence.
- **Next step**: No action planned. CT-LIO GT-backed is excluded from the main study scope. If an independent GT source becomes available for HDL-400, this can be revisited.

### Planned Variants

| Variant | Style | Intent | Args |
|---------|-------|--------|------|
| IMU preintegration default | balanced | Baseline CT-LIO profile with IMU bias estimation and a short fixed-lag prior. | `--ct-lio-estimate-bias --ct-lio-fixed-lag-window 4` |
| Seed-only fast | throughput-oriented | Keep the IMU seed but avoid the heavier lag smoother path. | `--ct-lio-fixed-lag-window 2 --ct-lio-fixed-lag-outer-iterations 1` |
| History smoother dense | accuracy-oriented | Use a slightly longer lag window and one extra relinearization pass once a GT-backed public benchmark contract is ready. | `--ct-lio-estimate-bias --ct-lio-fixed-lag-window 5 --ct-lio-fixed-lag-outer-iterations 2` |

## CT-LIO reference-trajectory trade-off on the public HDL-400 120-frame window

- **Problem ID**: `ct_lio_reference_tradeoff`
- **Question**: Which CT-LIO profile should stay as the current default when compared against a shared public reference trajectory export?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ct_lio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ct_lio_reference_profile_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| IMU preintegration default | balanced | 3.134 | 26.0 | 57.8 | 4.05 | 4.35 | Keep as reference variant |
| Seed-only fast | throughput-oriented | 0.488 | 17.5 | 83.5 | 3.80 | 4.20 | Adopt as current default |
| History smoother dense | accuracy-oriented | 1.609 | 25.4 | 63.9 | 3.45 | 3.95 | Keep as reference variant |

### Observations

1. `seed_only_fast` is the current default for this problem.
2. `imu_preintegration_default` is the fastest observed variant at 26.0 FPS.
3. `seed_only_fast` is the most accurate observed variant at 0.488 m ATE.

### Variant Notes

#### `imu_preintegration_default`

- Intent: Baseline CT-LIO profile with IMU bias estimation and a short fixed-lag prior.
- CLI args: `--ct-lio-estimate-bias --ct-lio-fixed-lag-window 4`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ct_lio --summary-json experiments/results/runs/ct_lio_reference_profile_matrix/imu_preintegration_default/summary.json --ct-lio-estimate-bias --ct-lio-fixed-lag-window 4`
- Summary: `experiments/results/runs/ct_lio_reference_profile_matrix/imu_preintegration_default/summary.json`
- Log: `experiments/results/runs/ct_lio_reference_profile_matrix/imu_preintegration_default/run.log`
- Readability proxy: 4.05 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.35 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: No extra method note.

#### `seed_only_fast`

- Intent: Keep the IMU seed but avoid the heavier lag smoother path.
- CLI args: `--ct-lio-fixed-lag-window 2 --ct-lio-fixed-lag-outer-iterations 1`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ct_lio --summary-json experiments/results/runs/ct_lio_reference_profile_matrix/seed_only_fast/summary.json --ct-lio-fixed-lag-window 2 --ct-lio-fixed-lag-outer-iterations 1`
- Summary: `experiments/results/runs/ct_lio_reference_profile_matrix/seed_only_fast/summary.json`
- Log: `experiments/results/runs/ct_lio_reference_profile_matrix/seed_only_fast/run.log`
- Readability proxy: 3.80 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 4.20 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: No extra method note.

#### `history_smoother_dense`

- Intent: Use a slightly longer lag window and one extra relinearization pass for stronger motion regularization without the unbounded smoother cost.
- CLI args: `--ct-lio-estimate-bias --ct-lio-fixed-lag-window 5 --ct-lio-fixed-lag-outer-iterations 2`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ct_lio --summary-json experiments/results/runs/ct_lio_reference_profile_matrix/history_smoother_dense/summary.json --ct-lio-estimate-bias --ct-lio-fixed-lag-window 5 --ct-lio-fixed-lag-outer-iterations 2`
- Summary: `experiments/results/runs/ct_lio_reference_profile_matrix/history_smoother_dense/summary.json`
- Log: `experiments/results/runs/ct_lio_reference_profile_matrix/history_smoother_dense/run.log`
- Readability proxy: 3.45 / 5.00. Adds extra tuning knobs and therefore more command complexity.
- Extensibility proxy: 3.95 / 5.00. Still stable-interface compatible, but with a larger parameter surface.
- Method note: No extra method note.


## DLIO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `dlio_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which DLIO profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `dlio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/dlio_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| KITTI default | balanced GICP+IMU fusion | 5.543 | 2.8 | 64.7 | 4.65 | 4.75 | Keep as reference variant |
| Fast | throughput-oriented | 5.026 | 7.3 | 100.0 | 4.30 | 4.50 | Adopt as current default |
| Dense | accuracy-oriented | 5.864 | 1.2 | 50.9 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 7.3 FPS.
3. `fast` is the most accurate observed variant at 5.026 m ATE.

### Variant Notes

#### `kitti_default`

- Intent: Baseline DLIO-style GICP keyframe odometry with default IMU/LiDAR fusion weights.
- CLI args: `--dlio-kitti-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods dlio --summary-json experiments/results/runs/dlio_kitti_raw_0009_full_matrix/kitti_default/summary.json --dlio-kitti-profile`
- Summary: `experiments/results/runs/dlio_kitti_raw_0009_full_matrix/kitti_default/summary.json`
- Log: `experiments/results/runs/dlio_kitti_raw_0009_full_matrix/kitti_default/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Direct LiDAR-inertial odometry: GICP scan-to-map plus IMU preintegration prior between scans when imu.csv and per-frame timestamps align; otherwise LiDAR motion prior only (no GT seed).

#### `fast`

- Intent: Coarser downsampling, fewer GICP iterations, lighter IMU fusion influence.
- CLI args: `--dlio-kitti-profile --dlio-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods dlio --summary-json experiments/results/runs/dlio_kitti_raw_0009_full_matrix/fast/summary.json --dlio-kitti-profile --dlio-fast-profile`
- Summary: `experiments/results/runs/dlio_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/dlio_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Direct LiDAR-inertial odometry: GICP scan-to-map plus IMU preintegration prior between scans when imu.csv and per-frame timestamps align; otherwise LiDAR motion prior only (no GT seed).

#### `dense`

- Intent: Finer voxels, deeper GICP, stronger IMU term when imu.csv is present.
- CLI args: `--dlio-kitti-profile --dlio-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods dlio --summary-json experiments/results/runs/dlio_kitti_raw_0009_full_matrix/dense/summary.json --dlio-kitti-profile --dlio-dense-profile`
- Summary: `experiments/results/runs/dlio_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/dlio_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Direct LiDAR-inertial odometry: GICP scan-to-map plus IMU preintegration prior between scans when imu.csv and per-frame timestamps align; otherwise LiDAR motion prior only (no GT seed).


## DLO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `dlo_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which DLO profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `dlo`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/dlo_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| KITTI default | balanced | 5.543 | 2.6 | 63.1 | 4.65 | 4.75 | Keep as reference variant |
| Fast | throughput-oriented | 5.026 | 7.3 | 100.0 | 4.30 | 4.50 | Adopt as current default |
| Dense | accuracy-oriented | 5.864 | 1.2 | 50.7 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 7.3 FPS.
3. `fast` is the most accurate observed variant at 5.026 m ATE.

### Variant Notes

#### `kitti_default`

- Intent: Baseline DLO GICP keyframe odometry for KITTI-scale LiDAR.
- CLI args: `--dlo-kitti-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods dlo --summary-json experiments/results/runs/dlo_kitti_raw_0009_full_matrix/kitti_default/summary.json --dlo-kitti-profile`
- Summary: `experiments/results/runs/dlo_kitti_raw_0009_full_matrix/kitti_default/summary.json`
- Log: `experiments/results/runs/dlo_kitti_raw_0009_full_matrix/kitti_default/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Keyframe scan-to-map odometry with GICP alignment (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarser downsampling and fewer GICP iterations.
- CLI args: `--dlo-kitti-profile --dlo-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods dlo --summary-json experiments/results/runs/dlo_kitti_raw_0009_full_matrix/fast/summary.json --dlo-kitti-profile --dlo-fast-profile`
- Summary: `experiments/results/runs/dlo_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/dlo_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Keyframe scan-to-map odometry with GICP alignment (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Finer voxels, more map keyframes, and deeper GICP.
- CLI args: `--dlo-kitti-profile --dlo-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods dlo --summary-json experiments/results/runs/dlo_kitti_raw_0009_full_matrix/dense/summary.json --dlo-kitti-profile --dlo-dense-profile`
- Summary: `experiments/results/runs/dlo_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/dlo_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Keyframe scan-to-map odometry with GICP alignment (no GT seed; anchor matches first GT pose).


## FAST-LIO2 on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `fast_lio2_kitti_raw_0009_full`
- **Question**: How does FAST-LIO2 perform on the KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio2_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 5.468 | 4.4 | 63.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 5.199 | 13.5 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 5.759 | 3.0 | 56.2 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 13.5 FPS.
3. `fast` is the most accurate observed variant at 5.199 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_kitti_raw_0009_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio2_kitti_raw_0009_full_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio2_kitti_raw_0009_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce keypoint density and map window for faster LiDAR-inertial updates.
- CLI args: `--fast-lio2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_kitti_raw_0009_full_matrix/fast/summary.json --fast-lio2-fast-profile`
- Summary: `experiments/results/runs/fast_lio2_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio2_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase keypoint density and map window for a more exhaustive LiDAR-inertial solve.
- CLI args: `--fast-lio2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_kitti_raw_0009_full_matrix/dense/summary.json --fast-lio2-dense-profile`
- Summary: `experiments/results/runs/fast_lio2_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio2_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).


## FAST-LIO2 on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `fast_lio2_kitti_raw_0009`
- **Question**: How does FAST-LIO2 perform on the KITTI Raw drive 0009 (200 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio2_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 2.483 | 5.2 | 67.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 2.328 | 12.3 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 2.656 | 2.7 | 54.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 12.3 FPS.
3. `fast` is the most accurate observed variant at 2.328 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_kitti_raw_0009_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio2_kitti_raw_0009_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio2_kitti_raw_0009_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce keypoint density and map window for faster LiDAR-inertial updates.
- CLI args: `--fast-lio2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_kitti_raw_0009_matrix/fast/summary.json --fast-lio2-fast-profile`
- Summary: `experiments/results/runs/fast_lio2_kitti_raw_0009_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio2_kitti_raw_0009_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase keypoint density and map window for a more exhaustive LiDAR-inertial solve.
- CLI args: `--fast-lio2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_kitti_raw_0009_matrix/dense/summary.json --fast-lio2-dense-profile`
- Summary: `experiments/results/runs/fast_lio2_kitti_raw_0009_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio2_kitti_raw_0009_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).


## FAST-LIO2 on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `fast_lio2_kitti_raw_0061_full`
- **Question**: How does FAST-LIO2 perform on the KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio2_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 5.351 | 4.3 | 63.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 5.066 | 13.5 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 5.881 | 2.3 | 51.6 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 13.5 FPS.
3. `fast` is the most accurate observed variant at 5.066 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_kitti_raw_0061_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio2_kitti_raw_0061_full_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio2_kitti_raw_0061_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce keypoint density and map window for faster LiDAR-inertial updates.
- CLI args: `--fast-lio2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_kitti_raw_0061_full_matrix/fast/summary.json --fast-lio2-fast-profile`
- Summary: `experiments/results/runs/fast_lio2_kitti_raw_0061_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio2_kitti_raw_0061_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase keypoint density and map window for a more exhaustive LiDAR-inertial solve.
- CLI args: `--fast-lio2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_kitti_raw_0061_full_matrix/dense/summary.json --fast-lio2-dense-profile`
- Summary: `experiments/results/runs/fast_lio2_kitti_raw_0061_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio2_kitti_raw_0061_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).


## FAST-LIO2 on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `fast_lio2_kitti_raw_0061`
- **Question**: How does FAST-LIO2 perform on the KITTI Raw drive 0061 (200 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio2_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 0.729 | 4.2 | 58.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.634 | 13.6 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 0.739 | 2.9 | 53.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 13.6 FPS.
3. `fast` is the most accurate observed variant at 0.634 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_kitti_raw_0061_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio2_kitti_raw_0061_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio2_kitti_raw_0061_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce keypoint density and map window for faster LiDAR-inertial updates.
- CLI args: `--fast-lio2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_kitti_raw_0061_matrix/fast/summary.json --fast-lio2-fast-profile`
- Summary: `experiments/results/runs/fast_lio2_kitti_raw_0061_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio2_kitti_raw_0061_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase keypoint density and map window for a more exhaustive LiDAR-inertial solve.
- CLI args: `--fast-lio2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_kitti_raw_0061_matrix/dense/summary.json --fast-lio2-dense-profile`
- Summary: `experiments/results/runs/fast_lio2_kitti_raw_0061_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio2_kitti_raw_0061_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).


## FAST-LIO2 on MCD KTH day-06 sequence

- **Problem ID**: `fast_lio2_mcd_kth_day_06`
- **Question**: How does FAST-LIO2 perform on the MCD KTH day-06 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio2_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 6.086 | 3.7 | 64.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 6.072 | 12.4 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 6.087 | 2.5 | 60.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 12.4 FPS.
3. `fast` is the most accurate observed variant at 6.072 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_mcd_kth_day_06_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio2_mcd_kth_day_06_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio2_mcd_kth_day_06_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce keypoint density and map window for faster LiDAR-inertial updates.
- CLI args: `--fast-lio2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_mcd_kth_day_06_matrix/fast/summary.json --fast-lio2-fast-profile`
- Summary: `experiments/results/runs/fast_lio2_mcd_kth_day_06_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio2_mcd_kth_day_06_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase keypoint density and map window for a more exhaustive LiDAR-inertial solve.
- CLI args: `--fast-lio2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_mcd_kth_day_06_matrix/dense/summary.json --fast-lio2-dense-profile`
- Summary: `experiments/results/runs/fast_lio2_mcd_kth_day_06_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio2_mcd_kth_day_06_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).


## FAST-LIO2 on MCD NTU day-02 sequence

- **Problem ID**: `fast_lio2_mcd_ntu_day_02`
- **Question**: How does FAST-LIO2 perform on the MCD NTU day-02 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio2_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 0.025 | 10.3 | 71.7 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.025 | 23.7 | 99.9 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 0.027 | 7.8 | 62.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 23.7 FPS.
3. `default` is the most accurate observed variant at 0.025 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_mcd_ntu_day_02_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio2_mcd_ntu_day_02_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio2_mcd_ntu_day_02_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce keypoint density and map window for faster LiDAR-inertial updates.
- CLI args: `--fast-lio2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_mcd_ntu_day_02_matrix/fast/summary.json --fast-lio2-fast-profile`
- Summary: `experiments/results/runs/fast_lio2_mcd_ntu_day_02_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio2_mcd_ntu_day_02_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase keypoint density and map window for a more exhaustive LiDAR-inertial solve.
- CLI args: `--fast-lio2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_mcd_ntu_day_02_matrix/dense/summary.json --fast-lio2-dense-profile`
- Summary: `experiments/results/runs/fast_lio2_mcd_ntu_day_02_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio2_mcd_ntu_day_02_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).


## FAST-LIO2 on MCD TUHH night-09 sequence

- **Problem ID**: `fast_lio2_mcd_tuhh_night_09`
- **Question**: How does FAST-LIO2 perform on the MCD TUHH night-09 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio2_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 1.334 | 5.4 | 69.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 1.339 | 14.2 | 99.8 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 1.333 | 3.3 | 61.6 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 14.2 FPS.
3. `dense` is the most accurate observed variant at 1.333 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_mcd_tuhh_night_09_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio2_mcd_tuhh_night_09_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio2_mcd_tuhh_night_09_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce keypoint density and map window for faster LiDAR-inertial updates.
- CLI args: `--fast-lio2-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_mcd_tuhh_night_09_matrix/fast/summary.json --fast-lio2-fast-profile`
- Summary: `experiments/results/runs/fast_lio2_mcd_tuhh_night_09_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio2_mcd_tuhh_night_09_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase keypoint density and map window for a more exhaustive LiDAR-inertial solve.
- CLI args: `--fast-lio2-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods fast_lio2 --summary-json experiments/results/runs/fast_lio2_mcd_tuhh_night_09_matrix/dense/summary.json --fast-lio2-dense-profile`
- Summary: `experiments/results/runs/fast_lio2_mcd_tuhh_night_09_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio2_mcd_tuhh_night_09_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial odometry with iterated Kalman filter and voxel map (no GT seed; anchor matches first GT pose).


## FAST-LIO-SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `fast_lio_slam_kitti_raw_0009_full`
- **Question**: How does FAST-LIO-SLAM perform on the KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio_slam_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 5.468 | 3.4 | 63.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 5.289 | 11.3 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 5.701 | 2.6 | 58.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 11.3 FPS.
3. `fast` is the most accurate observed variant at 5.289 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_kitti_raw_0009_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio_slam_kitti_raw_0009_full_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_kitti_raw_0009_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, shrink the front-end map window, and disable loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_kitti_raw_0009_full_matrix/fast/summary.json --fast-lio-slam-fast-profile`
- Summary: `experiments/results/runs/fast_lio_slam_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density, keep a larger front-end map, and tighten loop-closure cadence; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_kitti_raw_0009_full_matrix/dense/summary.json --fast-lio-slam-dense-profile`
- Summary: `experiments/results/runs/fast_lio_slam_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).


## FAST-LIO-SLAM on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `fast_lio_slam_kitti_raw_0009`
- **Question**: How does FAST-LIO-SLAM perform on the KITTI Raw drive 0009 (200 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio_slam_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 2.483 | 3.9 | 70.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 2.382 | 8.8 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 2.621 | 2.7 | 61.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 8.8 FPS.
3. `fast` is the most accurate observed variant at 2.382 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_kitti_raw_0009_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio_slam_kitti_raw_0009_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_kitti_raw_0009_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, shrink the front-end map window, and disable loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_kitti_raw_0009_matrix/fast/summary.json --fast-lio-slam-fast-profile`
- Summary: `experiments/results/runs/fast_lio_slam_kitti_raw_0009_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_kitti_raw_0009_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density, keep a larger front-end map, and tighten loop-closure cadence; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_kitti_raw_0009_matrix/dense/summary.json --fast-lio-slam-dense-profile`
- Summary: `experiments/results/runs/fast_lio_slam_kitti_raw_0009_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_kitti_raw_0009_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).


## FAST-LIO-SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `fast_lio_slam_kitti_raw_0061_full`
- **Question**: How does FAST-LIO-SLAM perform on the KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio_slam_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 5.351 | 2.5 | 59.7 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 4.945 | 9.1 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 5.756 | 0.8 | 47.6 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 9.1 FPS.
3. `fast` is the most accurate observed variant at 4.945 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_kitti_raw_0061_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio_slam_kitti_raw_0061_full_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_kitti_raw_0061_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, shrink the front-end map window, and disable loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_kitti_raw_0061_full_matrix/fast/summary.json --fast-lio-slam-fast-profile`
- Summary: `experiments/results/runs/fast_lio_slam_kitti_raw_0061_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_kitti_raw_0061_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density, keep a larger front-end map, and tighten loop-closure cadence; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_kitti_raw_0061_full_matrix/dense/summary.json --fast-lio-slam-dense-profile`
- Summary: `experiments/results/runs/fast_lio_slam_kitti_raw_0061_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_kitti_raw_0061_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).


## FAST-LIO-SLAM on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `fast_lio_slam_kitti_raw_0061`
- **Question**: How does FAST-LIO-SLAM perform on the KITTI Raw drive 0061 (200 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio_slam_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 0.729 | 3.7 | 61.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.660 | 11.2 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 0.736 | 2.4 | 55.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 11.2 FPS.
3. `fast` is the most accurate observed variant at 0.660 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_kitti_raw_0061_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio_slam_kitti_raw_0061_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_kitti_raw_0061_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, shrink the front-end map window, and disable loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_kitti_raw_0061_matrix/fast/summary.json --fast-lio-slam-fast-profile`
- Summary: `experiments/results/runs/fast_lio_slam_kitti_raw_0061_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_kitti_raw_0061_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density, keep a larger front-end map, and tighten loop-closure cadence; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_kitti_raw_0061_matrix/dense/summary.json --fast-lio-slam-dense-profile`
- Summary: `experiments/results/runs/fast_lio_slam_kitti_raw_0061_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_kitti_raw_0061_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).


## FAST-LIO-SLAM on MCD KTH day-06 sequence

- **Problem ID**: `fast_lio_slam_mcd_kth_day_06`
- **Question**: How does FAST-LIO-SLAM perform on the MCD KTH day-06 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio_slam_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 6.086 | 3.6 | 69.1 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 6.075 | 9.4 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 6.091 | 1.8 | 59.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 9.4 FPS.
3. `fast` is the most accurate observed variant at 6.075 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_mcd_kth_day_06_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio_slam_mcd_kth_day_06_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_mcd_kth_day_06_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, shrink the front-end map window, and disable loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_mcd_kth_day_06_matrix/fast/summary.json --fast-lio-slam-fast-profile`
- Summary: `experiments/results/runs/fast_lio_slam_mcd_kth_day_06_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_mcd_kth_day_06_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density, keep a larger front-end map, and tighten loop-closure cadence; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_mcd_kth_day_06_matrix/dense/summary.json --fast-lio-slam-dense-profile`
- Summary: `experiments/results/runs/fast_lio_slam_mcd_kth_day_06_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_mcd_kth_day_06_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).


## FAST-LIO-SLAM on MCD NTU day-02 sequence

- **Problem ID**: `fast_lio_slam_mcd_ntu_day_02`
- **Question**: How does FAST-LIO-SLAM perform on the MCD NTU day-02 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio_slam_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 0.025 | 9.0 | 72.1 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.028 | 20.4 | 94.7 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 0.025 | 6.0 | 63.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 20.4 FPS.
3. `default` is the most accurate observed variant at 0.025 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_mcd_ntu_day_02_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio_slam_mcd_ntu_day_02_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_mcd_ntu_day_02_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, shrink the front-end map window, and disable loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_mcd_ntu_day_02_matrix/fast/summary.json --fast-lio-slam-fast-profile`
- Summary: `experiments/results/runs/fast_lio_slam_mcd_ntu_day_02_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_mcd_ntu_day_02_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density, keep a larger front-end map, and tighten loop-closure cadence; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_mcd_ntu_day_02_matrix/dense/summary.json --fast-lio-slam-dense-profile`
- Summary: `experiments/results/runs/fast_lio_slam_mcd_ntu_day_02_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_mcd_ntu_day_02_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).


## FAST-LIO-SLAM on MCD TUHH night-09 sequence

- **Problem ID**: `fast_lio_slam_mcd_tuhh_night_09`
- **Question**: How does FAST-LIO-SLAM perform on the MCD TUHH night-09 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `fast_lio_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/fast_lio_slam_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 1.334 | 4.9 | 69.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 1.332 | 12.8 | 99.9 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 1.330 | 3.1 | 62.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 12.8 FPS.
3. `dense` is the most accurate observed variant at 1.330 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_mcd_tuhh_night_09_matrix/default/summary.json`
- Summary: `experiments/results/runs/fast_lio_slam_mcd_tuhh_night_09_matrix/default/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_mcd_tuhh_night_09_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, shrink the front-end map window, and disable loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_mcd_tuhh_night_09_matrix/fast/summary.json --fast-lio-slam-fast-profile`
- Summary: `experiments/results/runs/fast_lio_slam_mcd_tuhh_night_09_matrix/fast/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_mcd_tuhh_night_09_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density, keep a larger front-end map, and tighten loop-closure cadence; remains LiDAR-only when imu.csv is absent.
- CLI args: `--fast-lio-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods fast_lio_slam --summary-json experiments/results/runs/fast_lio_slam_mcd_tuhh_night_09_matrix/dense/summary.json --fast-lio-slam-dense-profile`
- Summary: `experiments/results/runs/fast_lio_slam_mcd_tuhh_night_09_matrix/dense/summary.json`
- Log: `experiments/results/runs/fast_lio_slam_mcd_tuhh_night_09_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization (no GT seed; anchor matches first GT pose).


## F-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `floam_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which F-LOAM profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `floam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/floam_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| KITTI default | balanced | 6.564 | 11.9 | 62.4 | 4.65 | 4.75 | Keep as reference variant |
| Fast | throughput-oriented | 5.452 | 28.6 | 100.0 | 4.30 | 4.50 | Adopt as current default |
| Dense | accuracy-oriented | 6.094 | 2.8 | 49.6 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 28.6 FPS.
3. `fast` is the most accurate observed variant at 5.452 m ATE.

### Variant Notes

#### `kitti_default`

- Intent: Baseline F-LOAM settings for KITTI (n_scans=64).
- CLI args: `--floam-kitti-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods floam --summary-json experiments/results/runs/floam_kitti_raw_0009_full_matrix/kitti_default/summary.json --floam-kitti-profile`
- Summary: `experiments/results/runs/floam_kitti_raw_0009_full_matrix/kitti_default/summary.json`
- Log: `experiments/results/runs/floam_kitti_raw_0009_full_matrix/kitti_default/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Fast feature-based odometry+mapping with thinned input and throttled mapping updates (no GT seed).

#### `fast`

- Intent: Thin input and throttle mapping for speed.
- CLI args: `--floam-kitti-profile --floam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods floam --summary-json experiments/results/runs/floam_kitti_raw_0009_full_matrix/fast/summary.json --floam-kitti-profile --floam-fast-profile`
- Summary: `experiments/results/runs/floam_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/floam_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Fast feature-based odometry+mapping with thinned input and throttled mapping updates (no GT seed).

#### `dense`

- Intent: Process denser input and update mapping every frame.
- CLI args: `--floam-kitti-profile --floam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods floam --summary-json experiments/results/runs/floam_kitti_raw_0009_full_matrix/dense/summary.json --floam-kitti-profile --floam-dense-profile`
- Summary: `experiments/results/runs/floam_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/floam_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Fast feature-based odometry+mapping with thinned input and throttled mapping updates (no GT seed).


## GICP throughput and accuracy trade-off on the second public HDL-400 reference window

- **Problem ID**: `gicp_profile_tradeoff_hdl_400_reference_b`
- **Question**: Which GICP dogfooding profile should stay as the current default on a deeper HDL-400 public reference set while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120_b`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference_b.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_hdl_400_reference_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.207 | 0.8 | 56.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.284 | 1.7 | 73.8 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 0.135 | 0.5 | 63.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 1.7 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.135 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods gicp --summary-json experiments/results/runs/gicp_hdl_400_reference_b_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_hdl_400_reference_b_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_hdl_400_reference_b_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods gicp --summary-json experiments/results/runs/gicp_hdl_400_reference_b_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_hdl_400_reference_b_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_hdl_400_reference_b_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods gicp --summary-json experiments/results/runs/gicp_hdl_400_reference_b_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_hdl_400_reference_b_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_hdl_400_reference_b_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the public HDL-400 reference window

- **Problem ID**: `gicp_profile_tradeoff_hdl_400_reference`
- **Question**: Which GICP dogfooding profile should stay as the current default on a different public dataset family while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_hdl_400_reference_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.146 | 16.0 | 69.0 | 5.00 | 5.00 | Keep as active challenger |
| Fast recent map | throughput-oriented | 0.215 | 23.3 | 73.6 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 0.101 | 9.3 | 69.9 | 4.65 | 4.75 | Keep as active challenger |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 23.3 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.101 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods gicp --summary-json experiments/results/runs/gicp_hdl_400_reference_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_hdl_400_reference_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_hdl_400_reference_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods gicp --summary-json experiments/results/runs/gicp_hdl_400_reference_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_hdl_400_reference_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_hdl_400_reference_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods gicp --summary-json experiments/results/runs/gicp_hdl_400_reference_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_hdl_400_reference_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_hdl_400_reference_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the second repository-stored Istanbul sequence

- **Problem ID**: `gicp_profile_tradeoff_istanbul_window_b`
- **Question**: Which GICP dogfooding profile should stay as the current default on a second public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_b`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_b_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_istanbul_window_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.233 | 3.1 | 74.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.166 | 5.7 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.228 | 1.6 | 61.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 5.7 FPS.
3. `fast_recent_map` is the most accurate observed variant at 1.166 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_istanbul_window_b_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_istanbul_window_b_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_istanbul_window_b_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_istanbul_window_b_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_istanbul_window_b_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_istanbul_window_b_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_istanbul_window_b_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_istanbul_window_b_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_istanbul_window_b_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the third repository-stored Istanbul sequence

- **Problem ID**: `gicp_profile_tradeoff_istanbul_window_c`
- **Question**: Which GICP dogfooding profile should stay as the current default on a third public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_c`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_c_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_istanbul_window_c_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.254 | 2.3 | 66.1 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.982 | 4.3 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.295 | 1.3 | 53.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 4.3 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.982 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_istanbul_window_c_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_istanbul_window_c_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_istanbul_window_c_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_istanbul_window_c_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_istanbul_window_c_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_istanbul_window_c_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_istanbul_window_c_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_istanbul_window_c_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_istanbul_window_c_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `gicp_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which GICP profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.378 | 12.5 | 69.7 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.170 | 23.0 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.261 | 7.6 | 62.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 23.0 FPS.
3. `fast_recent_map` is the most accurate observed variant at 1.170 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_full_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_full_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_full_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_full_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `gicp_profile_tradeoff_kitti_raw_0009`
- **Question**: Which GICP profile should stay as the current default on KITTI Raw drive 0009?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.455 | 14.1 | 67.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.177 | 25.8 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.358 | 7.6 | 58.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 25.8 FPS.
3. `fast_recent_map` is the most accurate observed variant at 1.177 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- **Problem ID**: `gicp_profile_tradeoff_kitti_raw_0009_nogt`
- **Question**: Which GICP profile on KITTI Raw drive 0009 (200 frames, no GT seed)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_kitti_raw_0009_nogt_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 2.206 | 16.3 | 59.7 | 4.65 | 4.75 | Keep as reference variant |
| Fast recent map | throughput-oriented | 14.057 | 32.0 | 55.4 | 4.30 | 4.50 | Keep as reference variant |
| Dense recent map | accuracy-oriented | 1.510 | 10.9 | 67.1 | 4.30 | 4.50 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 32.0 FPS.
3. `dense_recent_map` is the most accurate observed variant at 1.510 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/balanced_local_map/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/balanced_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--gicp-fast-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/fast_recent_map/summary.json --gicp-fast-profile --no-gt-seed`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/fast_recent_map/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--gicp-dense-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/dense_recent_map/summary.json --gicp-dense-profile --no-gt-seed`
- Summary: `experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0009_nogt_matrix/dense_recent_map/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).


## GICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `gicp_profile_tradeoff_kitti_raw_0061_full`
- **Question**: Which GICP profile on KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.173 | 13.8 | 75.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.081 | 22.8 | 98.8 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.055 | 7.3 | 66.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 22.8 FPS.
3. `dense_recent_map` is the most accurate observed variant at 1.055 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0061_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_kitti_raw_0061_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0061_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0061_full_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0061_full_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0061_full_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0061_full_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0061_full_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0061_full_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `gicp_profile_tradeoff_kitti_raw_0061`
- **Question**: Which GICP profile should stay as the current default on KITTI Raw drive 0061?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.846 | 14.5 | 74.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.959 | 25.7 | 90.7 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 0.781 | 9.2 | 68.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 25.7 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.781 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0061_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_kitti_raw_0061_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0061_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0061_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0061_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0061_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_kitti_raw_0061_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_kitti_raw_0061_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_kitti_raw_0061_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `gicp_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which GICP profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.026 | 12.9 | 56.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.630 | 24.7 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.175 | 7.2 | 41.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 24.7 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.630 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_kth_day_06_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_kth_day_06_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_mcd_kth_day_06_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_kth_day_06_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_kth_day_06_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_mcd_kth_day_06_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_kth_day_06_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `gicp_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which GICP profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.058 | 20.3 | 50.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.256 | 28.7 | 53.3 | 4.65 | 4.75 | Keep as reference variant |
| Dense recent map | accuracy-oriented | 0.017 | 13.0 | 72.7 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 28.7 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.017 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_ntu_day_02_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_ntu_day_02_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_ntu_day_02_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_mcd_ntu_day_02_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_ntu_day_02_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `gicp_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which GICP profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.490 | 17.2 | 59.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.317 | 31.2 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 0.549 | 9.2 | 43.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 31.2 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.317 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_mcd_tuhh_night_09_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## GICP throughput and accuracy trade-off on the repository-stored Istanbul sequence

- **Problem ID**: `gicp_profile_tradeoff`
- **Question**: Which GICP dogfooding profile should stay as the current default while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/gicp_profile_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.994 | 3.4 | 77.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.074 | 6.3 | 96.3 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.009 | 1.9 | 64.2 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 6.3 FPS.
3. `balanced_local_map` is the most accurate observed variant at 0.994 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_profile_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/gicp_profile_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/gicp_profile_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Reduce scan density and map pressure to increase FPS.
- CLI args: `--gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_profile_matrix/fast_recent_map/summary.json --gicp-fast-profile`
- Summary: `experiments/results/runs/gicp_profile_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_profile_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Increase scan density, map budget, and correspondence richness.
- CLI args: `--gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods gicp --summary-json experiments/results/runs/gicp_profile_matrix/dense_recent_map/summary.json --gicp-dense-profile`
- Summary: `experiments/results/runs/gicp_profile_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/gicp_profile_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## HDL Graph SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `hdl_graph_slam_kitti_raw_0009_full`
- **Question**: How does HDL Graph SLAM perform on the KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `hdl_graph_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/hdl_graph_slam_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 185.826 | 0.2 | 100.0 | 5.00 | 5.00 | Adopt as current default |

### Observations

1. `default` is the current default for this problem.
2. `default` is the fastest observed variant at 0.2 FPS.
3. `default` is the most accurate observed variant at 185.826 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods hdl_graph_slam --summary-json experiments/results/runs/hdl_graph_slam_kitti_raw_0009_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/hdl_graph_slam_kitti_raw_0009_full_matrix/default/summary.json`
- Log: `experiments/results/runs/hdl_graph_slam_kitti_raw_0009_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: NDT odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## HDL Graph SLAM on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `hdl_graph_slam_kitti_raw_0009`
- **Question**: How does HDL Graph SLAM perform on the KITTI Raw drive 0009 (200 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `hdl_graph_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/hdl_graph_slam_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 122.141 | 1.8 | 100.0 | 5.00 | 5.00 | Adopt as current default |

### Observations

1. `default` is the current default for this problem.
2. `default` is the fastest observed variant at 1.8 FPS.
3. `default` is the most accurate observed variant at 122.141 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods hdl_graph_slam --summary-json experiments/results/runs/hdl_graph_slam_kitti_raw_0009_matrix/default/summary.json`
- Summary: `experiments/results/runs/hdl_graph_slam_kitti_raw_0009_matrix/default/summary.json`
- Log: `experiments/results/runs/hdl_graph_slam_kitti_raw_0009_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: NDT odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## HDL Graph SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `hdl_graph_slam_kitti_raw_0061_full`
- **Question**: How does HDL Graph SLAM perform on the KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `skipped`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `hdl_graph_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/hdl_graph_slam_kitti_raw_0061_full_matrix.json`
- **Blocker**: Skipped: computation exceeds 1 hour on KITTI Raw 0061 full (703 frames)
- **Next step**: Run a lighter slice/profile, or keep this problem out of the ready set until a real result is available.

### Attempted Variants

| Variant | Style | Status | Note | Summary | Log |
|---------|-------|--------|------|---------|-----|
| Default | balanced | `SKIPPED` | Skipped: computation exceeds 1 hour on KITTI Raw 0061 full (703 frames) | `experiments/results/runs/hdl_graph_slam_kitti_raw_0061_full_matrix/default/summary.json` | `experiments/results/runs/hdl_graph_slam_kitti_raw_0061_full_matrix/default/run.log` |

## HDL Graph SLAM on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `hdl_graph_slam_kitti_raw_0061`
- **Question**: How does HDL Graph SLAM perform on the KITTI Raw drive 0061 (200 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `hdl_graph_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/hdl_graph_slam_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 118.996 | 1.5 | 100.0 | 5.00 | 5.00 | Adopt as current default |

### Observations

1. `default` is the current default for this problem.
2. `default` is the fastest observed variant at 1.5 FPS.
3. `default` is the most accurate observed variant at 118.996 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods hdl_graph_slam --summary-json experiments/results/runs/hdl_graph_slam_kitti_raw_0061_matrix/default/summary.json`
- Summary: `experiments/results/runs/hdl_graph_slam_kitti_raw_0061_matrix/default/summary.json`
- Log: `experiments/results/runs/hdl_graph_slam_kitti_raw_0061_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: NDT odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## HDL Graph SLAM on MCD KTH day-06 sequence

- **Problem ID**: `hdl_graph_slam_mcd_kth_day_06`
- **Question**: How does HDL Graph SLAM perform on the MCD KTH day-06 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `hdl_graph_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/hdl_graph_slam_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 5.113 | 4.3 | 100.0 | 5.00 | 5.00 | Adopt as current default |

### Observations

1. `default` is the current default for this problem.
2. `default` is the fastest observed variant at 4.3 FPS.
3. `default` is the most accurate observed variant at 5.113 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods hdl_graph_slam --summary-json experiments/results/runs/hdl_graph_slam_mcd_kth_day_06_matrix/default/summary.json`
- Summary: `experiments/results/runs/hdl_graph_slam_mcd_kth_day_06_matrix/default/summary.json`
- Log: `experiments/results/runs/hdl_graph_slam_mcd_kth_day_06_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: NDT odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## HDL Graph SLAM on MCD NTU day-02 sequence

- **Problem ID**: `hdl_graph_slam_mcd_ntu_day_02`
- **Question**: How does HDL Graph SLAM perform on the MCD NTU day-02 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `hdl_graph_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/hdl_graph_slam_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 27.212 | 2.7 | 100.0 | 5.00 | 5.00 | Adopt as current default |

### Observations

1. `default` is the current default for this problem.
2. `default` is the fastest observed variant at 2.7 FPS.
3. `default` is the most accurate observed variant at 27.212 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods hdl_graph_slam --summary-json experiments/results/runs/hdl_graph_slam_mcd_ntu_day_02_matrix/default/summary.json`
- Summary: `experiments/results/runs/hdl_graph_slam_mcd_ntu_day_02_matrix/default/summary.json`
- Log: `experiments/results/runs/hdl_graph_slam_mcd_ntu_day_02_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: NDT odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## HDL Graph SLAM on MCD TUHH night-09 sequence

- **Problem ID**: `hdl_graph_slam_mcd_tuhh_night_09`
- **Question**: How does HDL Graph SLAM perform on the MCD TUHH night-09 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `hdl_graph_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/hdl_graph_slam_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 10.818 | 3.2 | 100.0 | 5.00 | 5.00 | Adopt as current default |

### Observations

1. `default` is the current default for this problem.
2. `default` is the fastest observed variant at 3.2 FPS.
3. `default` is the most accurate observed variant at 10.818 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods hdl_graph_slam --summary-json experiments/results/runs/hdl_graph_slam_mcd_tuhh_night_09_matrix/default/summary.json`
- Summary: `experiments/results/runs/hdl_graph_slam_mcd_tuhh_night_09_matrix/default/summary.json`
- Log: `experiments/results/runs/hdl_graph_slam_mcd_tuhh_night_09_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: NDT odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## ISC-LOAM on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `isc_loam_kitti_raw_0009_full`
- **Question**: How does ISC-LOAM perform on the KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `isc_loam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/isc_loam_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 5.267 | 9.2 | 56.1 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 4.323 | 30.5 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 5.290 | 16.6 | 68.2 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 30.5 FPS.
3. `fast` is the most accurate observed variant at 4.323 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_kitti_raw_0009_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/isc_loam_kitti_raw_0009_full_matrix/default/summary.json`
- Log: `experiments/results/runs/isc_loam_kitti_raw_0009_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, keyframe rate, and loop-closure frequency for faster scan-context SLAM updates.
- CLI args: `--isc-loam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_kitti_raw_0009_full_matrix/fast/summary.json --isc-loam-fast-profile`
- Summary: `experiments/results/runs/isc_loam_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/isc_loam_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density and revisit keyframes more aggressively with tighter loop-closure cadence.
- CLI args: `--isc-loam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_kitti_raw_0009_full_matrix/dense/summary.json --isc-loam-dense-profile`
- Summary: `experiments/results/runs/isc_loam_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/isc_loam_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).


## ISC-LOAM on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `isc_loam_kitti_raw_0009`
- **Question**: How does ISC-LOAM perform on the KITTI Raw drive 0009 (200 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `isc_loam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/isc_loam_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 2.778 | 9.4 | 54.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 2.321 | 35.6 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 2.740 | 21.7 | 72.8 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 35.6 FPS.
3. `fast` is the most accurate observed variant at 2.321 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_kitti_raw_0009_matrix/default/summary.json`
- Summary: `experiments/results/runs/isc_loam_kitti_raw_0009_matrix/default/summary.json`
- Log: `experiments/results/runs/isc_loam_kitti_raw_0009_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, keyframe rate, and loop-closure frequency for faster scan-context SLAM updates.
- CLI args: `--isc-loam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_kitti_raw_0009_matrix/fast/summary.json --isc-loam-fast-profile`
- Summary: `experiments/results/runs/isc_loam_kitti_raw_0009_matrix/fast/summary.json`
- Log: `experiments/results/runs/isc_loam_kitti_raw_0009_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density and revisit keyframes more aggressively with tighter loop-closure cadence.
- CLI args: `--isc-loam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_kitti_raw_0009_matrix/dense/summary.json --isc-loam-dense-profile`
- Summary: `experiments/results/runs/isc_loam_kitti_raw_0009_matrix/dense/summary.json`
- Log: `experiments/results/runs/isc_loam_kitti_raw_0009_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).


## ISC-LOAM on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `isc_loam_kitti_raw_0061_full`
- **Question**: How does ISC-LOAM perform on the KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `isc_loam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/isc_loam_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 4.854 | 6.6 | 59.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 5.439 | 33.5 | 94.6 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 5.800 | 14.9 | 64.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 33.5 FPS.
3. `default` is the most accurate observed variant at 4.854 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_kitti_raw_0061_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/isc_loam_kitti_raw_0061_full_matrix/default/summary.json`
- Log: `experiments/results/runs/isc_loam_kitti_raw_0061_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, keyframe rate, and loop-closure frequency for faster scan-context SLAM updates.
- CLI args: `--isc-loam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_kitti_raw_0061_full_matrix/fast/summary.json --isc-loam-fast-profile`
- Summary: `experiments/results/runs/isc_loam_kitti_raw_0061_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/isc_loam_kitti_raw_0061_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density and revisit keyframes more aggressively with tighter loop-closure cadence.
- CLI args: `--isc-loam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_kitti_raw_0061_full_matrix/dense/summary.json --isc-loam-dense-profile`
- Summary: `experiments/results/runs/isc_loam_kitti_raw_0061_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/isc_loam_kitti_raw_0061_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).


## ISC-LOAM on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `isc_loam_kitti_raw_0061`
- **Question**: How does ISC-LOAM perform on the KITTI Raw drive 0061 (200 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `isc_loam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/isc_loam_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 0.986 | 11.6 | 38.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 1.002 | 43.5 | 74.6 | 4.65 | 4.75 | Keep as active challenger |
| Dense | accuracy-oriented | 0.494 | 23.7 | 77.2 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense` is the current default for this problem.
2. `fast` is the fastest observed variant at 43.5 FPS.
3. `dense` is the most accurate observed variant at 0.494 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_kitti_raw_0061_matrix/default/summary.json`
- Summary: `experiments/results/runs/isc_loam_kitti_raw_0061_matrix/default/summary.json`
- Log: `experiments/results/runs/isc_loam_kitti_raw_0061_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, keyframe rate, and loop-closure frequency for faster scan-context SLAM updates.
- CLI args: `--isc-loam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_kitti_raw_0061_matrix/fast/summary.json --isc-loam-fast-profile`
- Summary: `experiments/results/runs/isc_loam_kitti_raw_0061_matrix/fast/summary.json`
- Log: `experiments/results/runs/isc_loam_kitti_raw_0061_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density and revisit keyframes more aggressively with tighter loop-closure cadence.
- CLI args: `--isc-loam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_kitti_raw_0061_matrix/dense/summary.json --isc-loam-dense-profile`
- Summary: `experiments/results/runs/isc_loam_kitti_raw_0061_matrix/dense/summary.json`
- Log: `experiments/results/runs/isc_loam_kitti_raw_0061_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).


## ISC-LOAM on MCD KTH day-06 sequence

- **Problem ID**: `isc_loam_mcd_kth_day_06`
- **Question**: How does ISC-LOAM perform on the MCD KTH day-06 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `isc_loam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/isc_loam_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 6.023 | 11.0 | 61.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 6.094 | 48.6 | 99.4 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 6.055 | 26.6 | 77.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 48.6 FPS.
3. `default` is the most accurate observed variant at 6.023 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_mcd_kth_day_06_matrix/default/summary.json`
- Summary: `experiments/results/runs/isc_loam_mcd_kth_day_06_matrix/default/summary.json`
- Log: `experiments/results/runs/isc_loam_mcd_kth_day_06_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, keyframe rate, and loop-closure frequency for faster scan-context SLAM updates.
- CLI args: `--isc-loam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_mcd_kth_day_06_matrix/fast/summary.json --isc-loam-fast-profile`
- Summary: `experiments/results/runs/isc_loam_mcd_kth_day_06_matrix/fast/summary.json`
- Log: `experiments/results/runs/isc_loam_mcd_kth_day_06_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density and revisit keyframes more aggressively with tighter loop-closure cadence.
- CLI args: `--isc-loam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_mcd_kth_day_06_matrix/dense/summary.json --isc-loam-dense-profile`
- Summary: `experiments/results/runs/isc_loam_mcd_kth_day_06_matrix/dense/summary.json`
- Log: `experiments/results/runs/isc_loam_mcd_kth_day_06_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).


## ISC-LOAM on MCD NTU day-02 sequence

- **Problem ID**: `isc_loam_mcd_ntu_day_02`
- **Question**: How does ISC-LOAM perform on the MCD NTU day-02 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `isc_loam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/isc_loam_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 0.096 | 9.9 | 43.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.065 | 50.2 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 0.109 | 25.4 | 55.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 50.2 FPS.
3. `fast` is the most accurate observed variant at 0.065 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_mcd_ntu_day_02_matrix/default/summary.json`
- Summary: `experiments/results/runs/isc_loam_mcd_ntu_day_02_matrix/default/summary.json`
- Log: `experiments/results/runs/isc_loam_mcd_ntu_day_02_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, keyframe rate, and loop-closure frequency for faster scan-context SLAM updates.
- CLI args: `--isc-loam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_mcd_ntu_day_02_matrix/fast/summary.json --isc-loam-fast-profile`
- Summary: `experiments/results/runs/isc_loam_mcd_ntu_day_02_matrix/fast/summary.json`
- Log: `experiments/results/runs/isc_loam_mcd_ntu_day_02_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density and revisit keyframes more aggressively with tighter loop-closure cadence.
- CLI args: `--isc-loam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_mcd_ntu_day_02_matrix/dense/summary.json --isc-loam-dense-profile`
- Summary: `experiments/results/runs/isc_loam_mcd_ntu_day_02_matrix/dense/summary.json`
- Log: `experiments/results/runs/isc_loam_mcd_ntu_day_02_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).


## ISC-LOAM on MCD TUHH night-09 sequence

- **Problem ID**: `isc_loam_mcd_tuhh_night_09`
- **Question**: How does ISC-LOAM perform on the MCD TUHH night-09 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `isc_loam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/isc_loam_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 1.337 | 11.7 | 61.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 1.357 | 53.2 | 99.2 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 1.388 | 28.1 | 74.6 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 53.2 FPS.
3. `default` is the most accurate observed variant at 1.337 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_mcd_tuhh_night_09_matrix/default/summary.json`
- Summary: `experiments/results/runs/isc_loam_mcd_tuhh_night_09_matrix/default/summary.json`
- Log: `experiments/results/runs/isc_loam_mcd_tuhh_night_09_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density, keyframe rate, and loop-closure frequency for faster scan-context SLAM updates.
- CLI args: `--isc-loam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_mcd_tuhh_night_09_matrix/fast/summary.json --isc-loam-fast-profile`
- Summary: `experiments/results/runs/isc_loam_mcd_tuhh_night_09_matrix/fast/summary.json`
- Log: `experiments/results/runs/isc_loam_mcd_tuhh_night_09_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density and revisit keyframes more aggressively with tighter loop-closure cadence.
- CLI args: `--isc-loam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods isc_loam --summary-json experiments/results/runs/isc_loam_mcd_tuhh_night_09_matrix/dense/summary.json --isc-loam-dense-profile`
- Summary: `experiments/results/runs/isc_loam_mcd_tuhh_night_09_matrix/dense/summary.json`
- Log: `experiments/results/runs/isc_loam_mcd_tuhh_night_09_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Intensity scan context + LOAM with loop closure (no GT seed; anchor matches first GT pose).


## KISS-ICP throughput and drift trade-off on the second public HDL-400 reference window

- **Problem ID**: `kiss_icp_profile_tradeoff_hdl_400_reference_b`
- **Question**: Which KISS-ICP odometry profile should stay as the current default on a deeper HDL-400 public reference set while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120_b`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference_b.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_hdl_400_reference_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.498 | 0.2 | 43.7 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.218 | 0.4 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 0.447 | 0.1 | 39.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 0.4 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.218 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_hdl_400_reference_b_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and drift trade-off on the public HDL-400 reference window

- **Problem ID**: `kiss_icp_profile_tradeoff_hdl_400_reference`
- **Question**: Which KISS-ICP odometry profile should stay as the current default on a different public dataset family while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_hdl_400_reference_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 3.084 | 3.5 | 36.1 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.281 | 11.3 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 2.823 | 1.7 | 30.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 11.3 FPS.
3. `fast_recent_map` is the most accurate observed variant at 1.281 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_hdl_400_reference_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_hdl_400_reference_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_hdl_400_reference_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_hdl_400_reference_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_hdl_400_reference_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_hdl_400_reference_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_hdl_400_reference_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_hdl_400_reference_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_hdl_400_reference_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence

- **Problem ID**: `kiss_icp_profile_tradeoff_istanbul_window_b`
- **Question**: Which KISS-ICP odometry profile should stay as the current default on a second public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_b`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_b_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_istanbul_window_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 143.956 | 3.4 | 97.8 | 5.00 | 5.00 | Keep as active challenger |
| Fast recent map | throughput-oriented | 143.921 | 3.6 | 99.8 | 4.65 | 4.75 | Keep as active challenger |
| Dense local map | drift-oriented | 144.086 | 3.6 | 99.9 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_local_map` is the current default for this problem.
2. `dense_local_map` is the fastest observed variant at 3.6 FPS.
3. `fast_recent_map` is the most accurate observed variant at 143.921 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_istanbul_window_b_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_istanbul_window_b_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_istanbul_window_b_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_istanbul_window_b_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_istanbul_window_b_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_istanbul_window_b_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_istanbul_window_b_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_istanbul_window_b_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_istanbul_window_b_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence

- **Problem ID**: `kiss_icp_profile_tradeoff_istanbul_window_c`
- **Question**: Which KISS-ICP odometry profile should stay as the current default on a third public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_c`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_c_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_istanbul_window_c_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 131.691 | 3.4 | 95.2 | 5.00 | 5.00 | Keep as active challenger |
| Fast recent map | throughput-oriented | 131.692 | 3.7 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 131.752 | 3.3 | 93.4 | 4.65 | 4.75 | Keep as active challenger |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 3.7 FPS.
3. `balanced_local_map` is the most accurate observed variant at 131.691 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_istanbul_window_c_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_istanbul_window_c_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_istanbul_window_c_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_istanbul_window_c_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_istanbul_window_c_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_istanbul_window_c_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_istanbul_window_c_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_istanbul_window_c_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_istanbul_window_c_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `kiss_icp_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which KISS-ICP profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 5.383 | 9.5 | 71.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 5.839 | 21.9 | 96.1 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 5.456 | 7.0 | 65.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 21.9 FPS.
3. `balanced_local_map` is the most accurate observed variant at 5.383 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Dense.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_full_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `kiss_icp_profile_tradeoff_kitti_raw_0009`
- **Question**: Which KISS-ICP profile should stay as the current default on KITTI Raw drive 0009?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 2.412 | 12.5 | 75.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 2.642 | 24.4 | 95.7 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 2.480 | 7.3 | 63.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 24.4 FPS.
3. `balanced_local_map` is the most accurate observed variant at 2.412 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- **Problem ID**: `kiss_icp_profile_tradeoff_kitti_raw_0009_nogt`
- **Question**: Which KISS-ICP profile on KITTI Raw drive 0009 (200 frames, no GT seed)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_kitti_raw_0009_nogt_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 2.412 | 15.1 | 76.8 | 4.65 | 4.75 | Keep as reference variant |
| Fast recent map | throughput-oriented | 2.642 | 28.2 | 95.7 | 4.30 | 4.50 | Adopt as current default |
| Dense local map | drift-oriented | 2.480 | 11.7 | 69.4 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 28.2 FPS.
3. `balanced_local_map` is the most accurate observed variant at 2.412 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/balanced_local_map/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/balanced_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--kiss-fast-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/fast_recent_map/summary.json --kiss-fast-profile --no-gt-seed`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/fast_recent_map/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Dense.
- CLI args: `--kiss-dense-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/dense_local_map/summary.json --kiss-dense-profile --no-gt-seed`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0009_nogt_matrix/dense_local_map/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `kiss_icp_profile_tradeoff_kitti_raw_0061_full`
- **Question**: Which KISS-ICP profile on KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 4.343 | 9.7 | 93.4 | 5.00 | 5.00 | Keep as active challenger |
| Fast recent map | throughput-oriented | 4.623 | 11.2 | 97.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 4.706 | 3.0 | 59.6 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 11.2 FPS.
3. `balanced_local_map` is the most accurate observed variant at 4.343 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Dense.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0061_full_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `kiss_icp_profile_tradeoff_kitti_raw_0061`
- **Question**: Which KISS-ICP profile should stay as the current default on KITTI Raw drive 0061?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.738 | 15.4 | 73.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.679 | 28.3 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 0.773 | 10.0 | 61.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 28.3 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.679 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_kitti_raw_0061_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `kiss_icp_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which KISS-ICP profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 6.095 | 4.2 | 64.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 5.568 | 11.3 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 6.109 | 1.9 | 53.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 11.3 FPS.
3. `fast_recent_map` is the most accurate observed variant at 5.568 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_kth_day_06_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `kiss_icp_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which KISS-ICP profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.024 | 42.2 | 66.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.026 | 66.7 | 83.2 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 0.017 | 34.0 | 75.5 | 4.65 | 4.75 | Keep as active challenger |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 66.7 FPS.
3. `dense_local_map` is the most accurate observed variant at 0.017 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_ntu_day_02_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `kiss_icp_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which KISS-ICP profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.104 | 9.4 | 69.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.303 | 24.1 | 92.4 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 1.139 | 6.3 | 61.6 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 24.1 FPS.
3. `balanced_local_map` is the most accurate observed variant at 1.104 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_mcd_tuhh_night_09_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## KISS-ICP throughput and drift trade-off on the repository-stored Istanbul sequence

- **Problem ID**: `kiss_icp_profile_tradeoff`
- **Question**: Which KISS-ICP odometry profile should stay as the current default while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `kiss_icp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/kiss_icp_profile_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 183.178 | 3.7 | 95.5 | 5.00 | 5.00 | Keep as active challenger |
| Fast recent map | throughput-oriented | 182.960 | 4.0 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | drift-oriented | 183.226 | 3.4 | 91.7 | 4.65 | 4.75 | Keep as active challenger |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 4.0 FPS.
3. `fast_recent_map` is the most accurate observed variant at 182.960 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the odometry baseline.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_profile_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/kiss_icp_profile_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_profile_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: No extra method note.

#### `fast_recent_map`

- Intent: Reduce source density and map pressure to improve FPS.
- CLI args: `--kiss-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_profile_matrix/fast_recent_map/summary.json --kiss-fast-profile`
- Summary: `experiments/results/runs/kiss_icp_profile_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_profile_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.

#### `dense_local_map`

- Intent: Increase source density and local-map richness to fight odometry drift.
- CLI args: `--kiss-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods kiss_icp --summary-json experiments/results/runs/kiss_icp_profile_matrix/dense_local_map/summary.json --kiss-dense-profile`
- Summary: `experiments/results/runs/kiss_icp_profile_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/kiss_icp_profile_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: No extra method note.


## LeGO-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `lego_loam_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which LeGO-LOAM profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lego_loam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lego_loam_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| KITTI default | balanced | 6.066 | 2.9 | 65.5 | 4.65 | 4.75 | Keep as reference variant |
| Fast | throughput-oriented | 6.498 | 9.5 | 96.7 | 4.30 | 4.50 | Adopt as current default |
| Dense | accuracy-oriented | 7.249 | 3.5 | 60.2 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 9.5 FPS.
3. `kitti_default` is the most accurate observed variant at 6.066 m ATE.

### Variant Notes

#### `kitti_default`

- Intent: Ground-aware LeGO-LOAM baseline for HDL-64E (n_scans=64, sensor height ~KITTI).
- CLI args: `--lego-loam-kitti-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods lego_loam --summary-json experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/kitti_default/summary.json --lego-loam-kitti-profile`
- Summary: `experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/kitti_default/summary.json`
- Log: `experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/kitti_default/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Ground-aware feature odometry+mapping (no GT seed). n_scans=64 matches Velodyne HDL-64E layout.

#### `fast`

- Intent: Coarser surf sampling and lighter odometry/mapping Ceres budgets.
- CLI args: `--lego-loam-kitti-profile --lego-loam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods lego_loam --summary-json experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/fast/summary.json --lego-loam-kitti-profile --lego-loam-fast-profile`
- Summary: `experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Ground-aware feature odometry+mapping (no GT seed). n_scans=64 matches Velodyne HDL-64E layout.

#### `dense`

- Intent: Finer features and deeper optimization passes.
- CLI args: `--lego-loam-kitti-profile --lego-loam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods lego_loam --summary-json experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/dense/summary.json --lego-loam-kitti-profile --lego-loam-dense-profile`
- Summary: `experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/lego_loam_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Ground-aware feature odometry+mapping (no GT seed). n_scans=64 matches Velodyne HDL-64E layout.


## LINS on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `lins_kitti_raw_0009_full`
- **Question**: How does LINS perform on the KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lins`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lins_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 183.795 | 40.4 | 66.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 183.686 | 123.3 | 99.9 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 183.380 | 32.9 | 63.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 123.3 FPS.
3. `dense` is the most accurate observed variant at 183.380 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods lins --summary-json experiments/results/runs/lins_kitti_raw_0009_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/lins_kitti_raw_0009_full_matrix/default/summary.json`
- Log: `experiments/results/runs/lins_kitti_raw_0009_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen the voxel maps and cap correspondences to favor throughput.
- CLI args: `--lins-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods lins --summary-json experiments/results/runs/lins_kitti_raw_0009_full_matrix/fast/summary.json --lins-fast-profile`
- Summary: `experiments/results/runs/lins_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/lins_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer voxel maps and more correspondences for a denser iterated Kalman update.
- CLI args: `--lins-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods lins --summary-json experiments/results/runs/lins_kitti_raw_0009_full_matrix/dense/summary.json --lins-dense-profile`
- Summary: `experiments/results/runs/lins_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/lins_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## LINS on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `lins_kitti_raw_0009`
- **Question**: How does LINS perform on the KITTI Raw drive 0009 (200 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lins`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lins_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 120.280 | 31.5 | 62.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 120.032 | 120.7 | 99.9 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 119.861 | 28.5 | 61.8 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 120.7 FPS.
3. `dense` is the most accurate observed variant at 119.861 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods lins --summary-json experiments/results/runs/lins_kitti_raw_0009_matrix/default/summary.json`
- Summary: `experiments/results/runs/lins_kitti_raw_0009_matrix/default/summary.json`
- Log: `experiments/results/runs/lins_kitti_raw_0009_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen the voxel maps and cap correspondences to favor throughput.
- CLI args: `--lins-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods lins --summary-json experiments/results/runs/lins_kitti_raw_0009_matrix/fast/summary.json --lins-fast-profile`
- Summary: `experiments/results/runs/lins_kitti_raw_0009_matrix/fast/summary.json`
- Log: `experiments/results/runs/lins_kitti_raw_0009_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer voxel maps and more correspondences for a denser iterated Kalman update.
- CLI args: `--lins-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods lins --summary-json experiments/results/runs/lins_kitti_raw_0009_matrix/dense/summary.json --lins-dense-profile`
- Summary: `experiments/results/runs/lins_kitti_raw_0009_matrix/dense/summary.json`
- Log: `experiments/results/runs/lins_kitti_raw_0009_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## LINS on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `lins_kitti_raw_0061_full`
- **Question**: How does LINS perform on the KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lins`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lins_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 291.978 | 17.8 | 58.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 291.877 | 104.7 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 292.404 | 23.4 | 61.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 104.7 FPS.
3. `fast` is the most accurate observed variant at 291.877 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods lins --summary-json experiments/results/runs/lins_kitti_raw_0061_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/lins_kitti_raw_0061_full_matrix/default/summary.json`
- Log: `experiments/results/runs/lins_kitti_raw_0061_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen the voxel maps and cap correspondences to favor throughput.
- CLI args: `--lins-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods lins --summary-json experiments/results/runs/lins_kitti_raw_0061_full_matrix/fast/summary.json --lins-fast-profile`
- Summary: `experiments/results/runs/lins_kitti_raw_0061_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/lins_kitti_raw_0061_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer voxel maps and more correspondences for a denser iterated Kalman update.
- CLI args: `--lins-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods lins --summary-json experiments/results/runs/lins_kitti_raw_0061_full_matrix/dense/summary.json --lins-dense-profile`
- Summary: `experiments/results/runs/lins_kitti_raw_0061_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/lins_kitti_raw_0061_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## LINS on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `lins_kitti_raw_0061`
- **Question**: How does LINS perform on the KITTI Raw drive 0061 (200 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lins`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lins_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 82.383 | 14.7 | 56.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 82.393 | 115.2 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 82.877 | 32.2 | 63.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 115.2 FPS.
3. `default` is the most accurate observed variant at 82.383 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods lins --summary-json experiments/results/runs/lins_kitti_raw_0061_matrix/default/summary.json`
- Summary: `experiments/results/runs/lins_kitti_raw_0061_matrix/default/summary.json`
- Log: `experiments/results/runs/lins_kitti_raw_0061_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen the voxel maps and cap correspondences to favor throughput.
- CLI args: `--lins-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods lins --summary-json experiments/results/runs/lins_kitti_raw_0061_matrix/fast/summary.json --lins-fast-profile`
- Summary: `experiments/results/runs/lins_kitti_raw_0061_matrix/fast/summary.json`
- Log: `experiments/results/runs/lins_kitti_raw_0061_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer voxel maps and more correspondences for a denser iterated Kalman update.
- CLI args: `--lins-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods lins --summary-json experiments/results/runs/lins_kitti_raw_0061_matrix/dense/summary.json --lins-dense-profile`
- Summary: `experiments/results/runs/lins_kitti_raw_0061_matrix/dense/summary.json`
- Log: `experiments/results/runs/lins_kitti_raw_0061_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## LINS on MCD KTH day-06 sequence

- **Problem ID**: `lins_mcd_kth_day_06`
- **Question**: How does LINS perform on the MCD KTH day-06 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lins`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lins_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 7.393 | 65.0 | 67.7 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 7.120 | 166.1 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 7.207 | 49.8 | 64.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 166.1 FPS.
3. `fast` is the most accurate observed variant at 7.120 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods lins --summary-json experiments/results/runs/lins_mcd_kth_day_06_matrix/default/summary.json`
- Summary: `experiments/results/runs/lins_mcd_kth_day_06_matrix/default/summary.json`
- Log: `experiments/results/runs/lins_mcd_kth_day_06_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen the voxel maps and cap correspondences to favor throughput.
- CLI args: `--lins-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods lins --summary-json experiments/results/runs/lins_mcd_kth_day_06_matrix/fast/summary.json --lins-fast-profile`
- Summary: `experiments/results/runs/lins_mcd_kth_day_06_matrix/fast/summary.json`
- Log: `experiments/results/runs/lins_mcd_kth_day_06_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer voxel maps and more correspondences for a denser iterated Kalman update.
- CLI args: `--lins-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods lins --summary-json experiments/results/runs/lins_mcd_kth_day_06_matrix/dense/summary.json --lins-dense-profile`
- Summary: `experiments/results/runs/lins_mcd_kth_day_06_matrix/dense/summary.json`
- Log: `experiments/results/runs/lins_mcd_kth_day_06_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## LINS on MCD NTU day-02 sequence

- **Problem ID**: `lins_mcd_ntu_day_02`
- **Question**: How does LINS perform on the MCD NTU day-02 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lins`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lins_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 0.269 | 42.5 | 35.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.479 | 147.2 | 61.6 | 4.65 | 4.75 | Keep as active challenger |
| Dense | accuracy-oriented | 0.111 | 34.9 | 61.9 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense` is the current default for this problem.
2. `fast` is the fastest observed variant at 147.2 FPS.
3. `dense` is the most accurate observed variant at 0.111 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods lins --summary-json experiments/results/runs/lins_mcd_ntu_day_02_matrix/default/summary.json`
- Summary: `experiments/results/runs/lins_mcd_ntu_day_02_matrix/default/summary.json`
- Log: `experiments/results/runs/lins_mcd_ntu_day_02_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen the voxel maps and cap correspondences to favor throughput.
- CLI args: `--lins-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods lins --summary-json experiments/results/runs/lins_mcd_ntu_day_02_matrix/fast/summary.json --lins-fast-profile`
- Summary: `experiments/results/runs/lins_mcd_ntu_day_02_matrix/fast/summary.json`
- Log: `experiments/results/runs/lins_mcd_ntu_day_02_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer voxel maps and more correspondences for a denser iterated Kalman update.
- CLI args: `--lins-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods lins --summary-json experiments/results/runs/lins_mcd_ntu_day_02_matrix/dense/summary.json --lins-dense-profile`
- Summary: `experiments/results/runs/lins_mcd_ntu_day_02_matrix/dense/summary.json`
- Log: `experiments/results/runs/lins_mcd_ntu_day_02_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## LINS on MCD TUHH night-09 sequence

- **Problem ID**: `lins_mcd_tuhh_night_09`
- **Question**: How does LINS perform on the MCD TUHH night-09 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lins`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lins_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 1.269 | 73.0 | 66.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 1.147 | 173.4 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 1.150 | 52.3 | 64.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 173.4 FPS.
3. `fast` is the most accurate observed variant at 1.147 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods lins --summary-json experiments/results/runs/lins_mcd_tuhh_night_09_matrix/default/summary.json`
- Summary: `experiments/results/runs/lins_mcd_tuhh_night_09_matrix/default/summary.json`
- Log: `experiments/results/runs/lins_mcd_tuhh_night_09_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen the voxel maps and cap correspondences to favor throughput.
- CLI args: `--lins-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods lins --summary-json experiments/results/runs/lins_mcd_tuhh_night_09_matrix/fast/summary.json --lins-fast-profile`
- Summary: `experiments/results/runs/lins_mcd_tuhh_night_09_matrix/fast/summary.json`
- Log: `experiments/results/runs/lins_mcd_tuhh_night_09_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer voxel maps and more correspondences for a denser iterated Kalman update.
- CLI args: `--lins-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods lins --summary-json experiments/results/runs/lins_mcd_tuhh_night_09_matrix/dense/summary.json --lins-dense-profile`
- Summary: `experiments/results/runs/lins_mcd_tuhh_night_09_matrix/dense/summary.json`
- Log: `experiments/results/runs/lins_mcd_tuhh_night_09_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LiDAR-inertial navigation with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## LIO-SAM on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `lio_sam_kitti_raw_0009_full`
- **Question**: How does LIO-SAM perform on the KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lio_sam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lio_sam_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 5.259 | 6.5 | 66.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 5.296 | 20.2 | 99.7 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 5.832 | 10.1 | 70.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 20.2 FPS.
3. `default` is the most accurate observed variant at 5.259 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_kitti_raw_0009_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/lio_sam_kitti_raw_0009_full_matrix/default/summary.json`
- Log: `experiments/results/runs/lio_sam_kitti_raw_0009_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density and keyframe rate while disabling loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_kitti_raw_0009_full_matrix/fast/summary.json --lio-sam-fast-profile`
- Summary: `experiments/results/runs/lio_sam_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/lio_sam_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density while keeping the full keyframe cadence and loop closure enabled; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_kitti_raw_0009_full_matrix/dense/summary.json --lio-sam-dense-profile`
- Summary: `experiments/results/runs/lio_sam_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/lio_sam_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).


## LIO-SAM on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `lio_sam_kitti_raw_0009`
- **Question**: How does LIO-SAM perform on the KITTI Raw drive 0009 (200 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lio_sam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lio_sam_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 2.579 | 9.3 | 68.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 2.649 | 24.9 | 98.7 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 2.935 | 13.4 | 70.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 24.9 FPS.
3. `default` is the most accurate observed variant at 2.579 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_kitti_raw_0009_matrix/default/summary.json`
- Summary: `experiments/results/runs/lio_sam_kitti_raw_0009_matrix/default/summary.json`
- Log: `experiments/results/runs/lio_sam_kitti_raw_0009_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density and keyframe rate while disabling loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_kitti_raw_0009_matrix/fast/summary.json --lio-sam-fast-profile`
- Summary: `experiments/results/runs/lio_sam_kitti_raw_0009_matrix/fast/summary.json`
- Log: `experiments/results/runs/lio_sam_kitti_raw_0009_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density while keeping the full keyframe cadence and loop closure enabled; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_kitti_raw_0009_matrix/dense/summary.json --lio-sam-dense-profile`
- Summary: `experiments/results/runs/lio_sam_kitti_raw_0009_matrix/dense/summary.json`
- Log: `experiments/results/runs/lio_sam_kitti_raw_0009_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).


## LIO-SAM on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `lio_sam_kitti_raw_0061_full`
- **Question**: How does LIO-SAM perform on the KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lio_sam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lio_sam_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 5.236 | 5.8 | 63.7 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 6.067 | 21.2 | 93.2 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 5.756 | 10.8 | 71.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 21.2 FPS.
3. `default` is the most accurate observed variant at 5.236 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_kitti_raw_0061_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/lio_sam_kitti_raw_0061_full_matrix/default/summary.json`
- Log: `experiments/results/runs/lio_sam_kitti_raw_0061_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density and keyframe rate while disabling loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_kitti_raw_0061_full_matrix/fast/summary.json --lio-sam-fast-profile`
- Summary: `experiments/results/runs/lio_sam_kitti_raw_0061_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/lio_sam_kitti_raw_0061_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density while keeping the full keyframe cadence and loop closure enabled; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_kitti_raw_0061_full_matrix/dense/summary.json --lio-sam-dense-profile`
- Summary: `experiments/results/runs/lio_sam_kitti_raw_0061_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/lio_sam_kitti_raw_0061_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).


## LIO-SAM on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `lio_sam_kitti_raw_0061`
- **Question**: How does LIO-SAM perform on the KITTI Raw drive 0061 (200 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lio_sam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lio_sam_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 0.577 | 10.3 | 68.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.704 | 27.1 | 91.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 0.840 | 17.2 | 66.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 27.1 FPS.
3. `default` is the most accurate observed variant at 0.577 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_kitti_raw_0061_matrix/default/summary.json`
- Summary: `experiments/results/runs/lio_sam_kitti_raw_0061_matrix/default/summary.json`
- Log: `experiments/results/runs/lio_sam_kitti_raw_0061_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density and keyframe rate while disabling loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_kitti_raw_0061_matrix/fast/summary.json --lio-sam-fast-profile`
- Summary: `experiments/results/runs/lio_sam_kitti_raw_0061_matrix/fast/summary.json`
- Log: `experiments/results/runs/lio_sam_kitti_raw_0061_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density while keeping the full keyframe cadence and loop closure enabled; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_kitti_raw_0061_matrix/dense/summary.json --lio-sam-dense-profile`
- Summary: `experiments/results/runs/lio_sam_kitti_raw_0061_matrix/dense/summary.json`
- Log: `experiments/results/runs/lio_sam_kitti_raw_0061_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).


## LIO-SAM on MCD KTH day-06 sequence

- **Problem ID**: `lio_sam_mcd_kth_day_06`
- **Question**: How does LIO-SAM perform on the MCD KTH day-06 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lio_sam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lio_sam_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 6.020 | 8.1 | 63.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 6.074 | 29.7 | 99.5 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 6.060 | 11.7 | 69.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 29.7 FPS.
3. `default` is the most accurate observed variant at 6.020 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_mcd_kth_day_06_matrix/default/summary.json`
- Summary: `experiments/results/runs/lio_sam_mcd_kth_day_06_matrix/default/summary.json`
- Log: `experiments/results/runs/lio_sam_mcd_kth_day_06_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density and keyframe rate while disabling loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_mcd_kth_day_06_matrix/fast/summary.json --lio-sam-fast-profile`
- Summary: `experiments/results/runs/lio_sam_mcd_kth_day_06_matrix/fast/summary.json`
- Log: `experiments/results/runs/lio_sam_mcd_kth_day_06_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density while keeping the full keyframe cadence and loop closure enabled; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_mcd_kth_day_06_matrix/dense/summary.json --lio-sam-dense-profile`
- Summary: `experiments/results/runs/lio_sam_mcd_kth_day_06_matrix/dense/summary.json`
- Log: `experiments/results/runs/lio_sam_mcd_kth_day_06_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).


## LIO-SAM on MCD NTU day-02 sequence

- **Problem ID**: `lio_sam_mcd_ntu_day_02`
- **Question**: How does LIO-SAM perform on the MCD NTU day-02 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lio_sam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lio_sam_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 0.067 | 8.4 | 46.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.073 | 31.7 | 80.9 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 0.045 | 12.2 | 69.2 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 31.7 FPS.
3. `dense` is the most accurate observed variant at 0.045 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_mcd_ntu_day_02_matrix/default/summary.json`
- Summary: `experiments/results/runs/lio_sam_mcd_ntu_day_02_matrix/default/summary.json`
- Log: `experiments/results/runs/lio_sam_mcd_ntu_day_02_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density and keyframe rate while disabling loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_mcd_ntu_day_02_matrix/fast/summary.json --lio-sam-fast-profile`
- Summary: `experiments/results/runs/lio_sam_mcd_ntu_day_02_matrix/fast/summary.json`
- Log: `experiments/results/runs/lio_sam_mcd_ntu_day_02_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density while keeping the full keyframe cadence and loop closure enabled; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_mcd_ntu_day_02_matrix/dense/summary.json --lio-sam-dense-profile`
- Summary: `experiments/results/runs/lio_sam_mcd_ntu_day_02_matrix/dense/summary.json`
- Log: `experiments/results/runs/lio_sam_mcd_ntu_day_02_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).


## LIO-SAM on MCD TUHH night-09 sequence

- **Problem ID**: `lio_sam_mcd_tuhh_night_09`
- **Question**: How does LIO-SAM perform on the MCD TUHH night-09 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `lio_sam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/lio_sam_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 1.331 | 9.3 | 64.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 1.314 | 30.0 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 1.347 | 10.4 | 66.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 30.0 FPS.
3. `fast` is the most accurate observed variant at 1.314 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_mcd_tuhh_night_09_matrix/default/summary.json`
- Summary: `experiments/results/runs/lio_sam_mcd_tuhh_night_09_matrix/default/summary.json`
- Log: `experiments/results/runs/lio_sam_mcd_tuhh_night_09_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Reduce input density and keyframe rate while disabling loop closure; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_mcd_tuhh_night_09_matrix/fast/summary.json --lio-sam-fast-profile`
- Summary: `experiments/results/runs/lio_sam_mcd_tuhh_night_09_matrix/fast/summary.json`
- Log: `experiments/results/runs/lio_sam_mcd_tuhh_night_09_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Increase input density while keeping the full keyframe cadence and loop closure enabled; remains LiDAR-only when imu.csv is absent.
- CLI args: `--lio-sam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods lio_sam --summary-json experiments/results/runs/lio_sam_mcd_tuhh_night_09_matrix/dense/summary.json --lio-sam-dense-profile`
- Summary: `experiments/results/runs/lio_sam_mcd_tuhh_night_09_matrix/dense/summary.json`
- Log: `experiments/results/runs/lio_sam_mcd_tuhh_night_09_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure (no GT seed; anchor matches first GT pose).


## LiTAMIN2 throughput and accuracy trade-off on the second public HDL-400 reference window

- **Problem ID**: `litamin2_profile_tradeoff_hdl_400_reference_b`
- **Question**: Which LiTAMIN2 profile should stay as the current default on a deeper HDL-400 public reference set without collapsing the exploration space to one implementation?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120_b`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference_b.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_hdl_400_reference_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.168 | 4.9 | 89.9 | 5.00 | 5.00 | Keep as active challenger |
| Fast local-map + ICP-only | single-term simplification | 0.168 | 5.2 | 92.8 | 4.65 | 4.75 | Adopt as current default |
| Paper-like 3m + covariance | paper-oriented | 0.274 | 5.8 | 78.3 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 0.274 | 6.1 | 80.8 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_icp_only_half_threads` is the current default for this problem.
2. `paper_icp_only_half_threads` is the fastest observed variant at 6.1 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 0.168 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_b_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_b_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_b_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_b_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_b_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the public HDL-400 reference window

- **Problem ID**: `litamin2_profile_tradeoff_hdl_400_reference`
- **Question**: Which LiTAMIN2 profile should stay as the current default on a different public dataset family without collapsing the exploration space to one implementation?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_hdl_400_reference_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.111 | 61.6 | 82.7 | 5.00 | 5.00 | Keep as reference variant |
| Fast local-map + ICP-only | single-term simplification | 0.111 | 58.6 | 81.1 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + covariance | paper-oriented | 0.129 | 94.2 | 93.1 | 4.65 | 4.75 | Adopt as current default |
| Paper-like 3m + ICP-only | paper-oriented simplification | 0.129 | 89.9 | 90.8 | 4.30 | 4.50 | Keep as active challenger |

### Observations

1. `paper_cov_half_threads` is the current default for this problem.
2. `paper_cov_half_threads` is the fastest observed variant at 94.2 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 0.111 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_hdl_400_reference_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_hdl_400_reference_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_hdl_400_reference_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the second repository-stored Istanbul sequence

- **Problem ID**: `litamin2_profile_tradeoff_istanbul_window_b`
- **Question**: Which LiTAMIN2 profile should stay as the current default on a second public Istanbul window without collapsing the exploration space to one implementation?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_b`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_b_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_istanbul_window_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 1.222 | 18.6 | 94.3 | 5.00 | 5.00 | Keep as active challenger |
| Fast local-map + ICP-only | single-term simplification | 1.222 | 20.9 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Paper-like 3m + covariance | paper-oriented | 1.332 | 15.0 | 81.6 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 1.332 | 17.0 | 86.5 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_icp_only_half_threads` is the current default for this problem.
2. `fast_icp_only_half_threads` is the fastest observed variant at 20.9 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 1.222 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_b_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_istanbul_window_b_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_b_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_b_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_istanbul_window_b_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_b_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_b_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_istanbul_window_b_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_b_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_b_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_istanbul_window_b_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_b_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the third repository-stored Istanbul sequence

- **Problem ID**: `litamin2_profile_tradeoff_istanbul_window_c`
- **Question**: Which LiTAMIN2 profile should stay as the current default on a third public Istanbul window without collapsing the exploration space to one implementation?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_c`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_c_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_istanbul_window_c_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.930 | 18.0 | 82.4 | 5.00 | 5.00 | Keep as active challenger |
| Fast local-map + ICP-only | single-term simplification | 0.930 | 21.2 | 89.9 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + covariance | paper-oriented | 0.741 | 14.8 | 85.0 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + ICP-only | paper-oriented simplification | 0.741 | 17.2 | 90.8 | 4.30 | 4.50 | Adopt as current default |

### Observations

1. `paper_icp_only_half_threads` is the current default for this problem.
2. `fast_icp_only_half_threads` is the fastest observed variant at 21.2 FPS.
3. `paper_cov_half_threads` is the most accurate observed variant at 0.741 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_c_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_istanbul_window_c_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_c_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_c_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_istanbul_window_c_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_c_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_c_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_istanbul_window_c_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_c_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_istanbul_window_c_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_istanbul_window_c_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_istanbul_window_c_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `litamin2_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which LiTAMIN2 profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 1.145 | 37.9 | 88.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast local-map + ICP-only | single-term simplification | 1.145 | 48.8 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Paper-like 3m + covariance | paper-oriented | 1.438 | 43.3 | 84.2 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 1.438 | 28.1 | 68.6 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_icp_only_half_threads` is the current default for this problem.
2. `fast_icp_only_half_threads` is the fastest observed variant at 48.8 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 1.145 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove covariance-shape term.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Paper 3m profile.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Paper 3m + ICP-only.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_full_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `litamin2_profile_tradeoff_kitti_raw_0009`
- **Question**: Which LiTAMIN2 profile should stay as the current default on KITTI Raw drive 0009?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 1.053 | 27.1 | 81.0 | 5.00 | 5.00 | Keep as active challenger |
| Fast local-map + ICP-only | single-term simplification | 1.053 | 30.0 | 84.3 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + covariance | paper-oriented | 1.397 | 33.8 | 76.4 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 1.397 | 43.7 | 87.7 | 4.30 | 4.50 | Adopt as current default |

### Observations

1. `paper_icp_only_half_threads` is the current default for this problem.
2. `paper_icp_only_half_threads` is the fastest observed variant at 43.7 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 1.053 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- **Problem ID**: `litamin2_profile_tradeoff_kitti_raw_0009_nogt`
- **Question**: Which LiTAMIN2 profile on KITTI Raw drive 0009 (200 frames, no GT seed)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_kitti_raw_0009_nogt_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 122.280 | 63.6 | 86.4 | 4.65 | 4.75 | Keep as reference variant |
| Fast local-map + ICP-only | single-term simplification | 122.280 | 81.7 | 96.8 | 4.30 | 4.50 | Keep as active challenger |
| Paper-like 3m + covariance | paper-oriented | 122.275 | 87.3 | 100.0 | 4.30 | 4.50 | Adopt as current default |
| Paper-like 3m + ICP-only | paper-oriented simplification | 122.275 | 82.7 | 97.3 | 3.95 | 4.25 | Keep as active challenger |

### Observations

1. `paper_cov_half_threads` is the current default for this problem.
2. `paper_cov_half_threads` is the fastest observed variant at 87.3 FPS.
3. `paper_cov_half_threads` is the most accurate observed variant at 122.275 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/fast_cov_half_threads/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

#### `fast_icp_only_half_threads`

- Intent: Remove covariance-shape term.
- CLI args: `--litamin2-icp-only --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed). Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Paper 3m profile.
- CLI args: `--litamin2-paper-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

#### `paper_icp_only_half_threads`

- Intent: Paper 3m + ICP-only.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only --no-gt-seed`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0009_nogt_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 3.95 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.25 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed). Covariance-shape term disabled.


## LiTAMIN2 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `litamin2_profile_tradeoff_kitti_raw_0061_full`
- **Question**: Which LiTAMIN2 profile on KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.944 | 53.5 | 96.0 | 5.00 | 5.00 | Keep as active challenger |
| Fast local-map + ICP-only | single-term simplification | 0.944 | 58.1 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Paper-like 3m + covariance | paper-oriented | 1.325 | 36.3 | 66.8 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 1.325 | 43.2 | 72.8 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_icp_only_half_threads` is the current default for this problem.
2. `fast_icp_only_half_threads` is the fastest observed variant at 58.1 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 0.944 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove covariance-shape term.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Paper 3m profile.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Paper 3m + ICP-only.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_full_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `litamin2_profile_tradeoff_kitti_raw_0061`
- **Question**: Which LiTAMIN2 profile should stay as the current default on KITTI Raw drive 0061?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.511 | 67.7 | 99.4 | 5.00 | 5.00 | Adopt as current default |
| Fast local-map + ICP-only | single-term simplification | 0.511 | 64.9 | 97.3 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + covariance | paper-oriented | 1.130 | 67.2 | 71.6 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 1.130 | 68.6 | 72.6 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_cov_half_threads` is the current default for this problem.
2. `paper_icp_only_half_threads` is the fastest observed variant at 68.6 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 0.511 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_kitti_raw_0061_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_kitti_raw_0061_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `litamin2_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which LiTAMIN2 profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.401 | 64.2 | 100.0 | 5.00 | 5.00 | Adopt as current default |
| Fast local-map + ICP-only | single-term simplification | 0.401 | 53.6 | 91.8 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + covariance | paper-oriented | 0.953 | 45.2 | 56.3 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 0.953 | 54.5 | 63.5 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_cov_half_threads` is the current default for this problem.
2. `fast_cov_half_threads` is the fastest observed variant at 64.2 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 0.401 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_kth_day_06_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_kth_day_06_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_kth_day_06_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_kth_day_06_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_kth_day_06_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `litamin2_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which LiTAMIN2 profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.075 | 99.5 | 77.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast local-map + ICP-only | single-term simplification | 0.075 | 100.6 | 77.9 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + covariance | paper-oriented | 0.045 | 102.8 | 98.7 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + ICP-only | paper-oriented simplification | 0.045 | 105.6 | 100.0 | 4.30 | 4.50 | Adopt as current default |

### Observations

1. `paper_icp_only_half_threads` is the current default for this problem.
2. `paper_icp_only_half_threads` is the fastest observed variant at 105.6 FPS.
3. `paper_cov_half_threads` is the most accurate observed variant at 0.045 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_ntu_day_02_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `litamin2_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which LiTAMIN2 profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 0.194 | 106.5 | 100.0 | 5.00 | 5.00 | Adopt as current default |
| Fast local-map + ICP-only | single-term simplification | 0.194 | 105.8 | 99.7 | 4.65 | 4.75 | Keep as active challenger |
| Paper-like 3m + covariance | paper-oriented | 0.291 | 97.9 | 79.2 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 0.291 | 106.4 | 83.2 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_cov_half_threads` is the current default for this problem.
2. `fast_cov_half_threads` is the fastest observed variant at 106.5 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 0.194 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_mcd_tuhh_night_09_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LiTAMIN2 throughput and accuracy trade-off on the repository-stored Istanbul sequence

- **Problem ID**: `litamin2_profile_tradeoff`
- **Question**: Which LiTAMIN2 profile should stay as the current default without collapsing the exploration space to one implementation?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `litamin2`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/litamin2_profile_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Fast local-map + covariance | throughput-oriented | 1.213 | 21.5 | 95.6 | 5.00 | 5.00 | Keep as active challenger |
| Fast local-map + ICP-only | single-term simplification | 1.213 | 23.5 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Paper-like 3m + covariance | paper-oriented | 1.264 | 15.5 | 81.0 | 4.65 | 4.75 | Keep as reference variant |
| Paper-like 3m + ICP-only | paper-oriented simplification | 1.264 | 18.0 | 86.3 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_icp_only_half_threads` is the current default for this problem.
2. `fast_icp_only_half_threads` is the fastest observed variant at 23.5 FPS.
3. `fast_cov_half_threads` is the most accurate observed variant at 1.213 m ATE.

### Variant Notes

#### `fast_cov_half_threads`

- Intent: Keep the current repository default with recent/local-map pruning and covariance-aware cost.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_profile_matrix/fast_cov_half_threads/summary.json`
- Summary: `experiments/results/runs/litamin2_profile_matrix/fast_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_profile_matrix/fast_cov_half_threads/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_icp_only_half_threads`

- Intent: Remove the covariance-shape term while keeping the same fast local-map profile.
- CLI args: `--litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_profile_matrix/fast_icp_only_half_threads/summary.json --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_profile_matrix/fast_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_profile_matrix/fast_icp_only_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.

#### `paper_cov_half_threads`

- Intent: Move closer to the paper's 3 m profile while preserving the same benchmark interface.
- CLI args: `--litamin2-paper-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_profile_matrix/paper_cov_half_threads/summary.json --litamin2-paper-profile`
- Summary: `experiments/results/runs/litamin2_profile_matrix/paper_cov_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_profile_matrix/paper_cov_half_threads/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `paper_icp_only_half_threads`

- Intent: Use the paper-like 3 m profile, but isolate the first cost term by disabling covariance cost.
- CLI args: `--litamin2-paper-profile --litamin2-icp-only`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods litamin2 --summary-json experiments/results/runs/litamin2_profile_matrix/paper_icp_only_half_threads/summary.json --litamin2-paper-profile --litamin2-icp-only`
- Summary: `experiments/results/runs/litamin2_profile_matrix/paper_icp_only_half_threads/summary.json`
- Log: `experiments/results/runs/litamin2_profile_matrix/paper_icp_only_half_threads/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool. Covariance-shape term disabled.


## LOAM Livox on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `loam_livox_kitti_raw_0009_full`
- **Question**: How does LOAM Livox perform on the KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `loam_livox`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/loam_livox_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 124.828 | 6.7 | 60.1 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 136.097 | 33.1 | 95.9 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 130.624 | 8.6 | 60.8 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 33.1 FPS.
3. `default` is the most accurate observed variant at 124.828 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_kitti_raw_0009_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/loam_livox_kitti_raw_0009_full_matrix/default/summary.json`
- Log: `experiments/results/runs/loam_livox_kitti_raw_0009_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser map grids and fewer outer refinement passes for faster Livox-style LOAM updates.
- CLI args: `--loam-livox-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_kitti_raw_0009_full_matrix/fast/summary.json --loam-livox-fast-profile`
- Summary: `experiments/results/runs/loam_livox_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/loam_livox_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer map grids and extra outer refinement passes for a more exhaustive LOAM solve.
- CLI args: `--loam-livox-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_kitti_raw_0009_full_matrix/dense/summary.json --loam-livox-dense-profile`
- Summary: `experiments/results/runs/loam_livox_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/loam_livox_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).


## LOAM Livox on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `loam_livox_kitti_raw_0009`
- **Question**: How does LOAM Livox perform on the KITTI Raw drive 0009 (200 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `loam_livox`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/loam_livox_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 93.193 | 6.7 | 57.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 98.811 | 43.8 | 96.9 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 92.735 | 11.5 | 63.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 43.8 FPS.
3. `dense` is the most accurate observed variant at 92.735 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_kitti_raw_0009_matrix/default/summary.json`
- Summary: `experiments/results/runs/loam_livox_kitti_raw_0009_matrix/default/summary.json`
- Log: `experiments/results/runs/loam_livox_kitti_raw_0009_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser map grids and fewer outer refinement passes for faster Livox-style LOAM updates.
- CLI args: `--loam-livox-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_kitti_raw_0009_matrix/fast/summary.json --loam-livox-fast-profile`
- Summary: `experiments/results/runs/loam_livox_kitti_raw_0009_matrix/fast/summary.json`
- Log: `experiments/results/runs/loam_livox_kitti_raw_0009_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer map grids and extra outer refinement passes for a more exhaustive LOAM solve.
- CLI args: `--loam-livox-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_kitti_raw_0009_matrix/dense/summary.json --loam-livox-dense-profile`
- Summary: `experiments/results/runs/loam_livox_kitti_raw_0009_matrix/dense/summary.json`
- Log: `experiments/results/runs/loam_livox_kitti_raw_0009_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).


## LOAM Livox on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `loam_livox_kitti_raw_0061_full`
- **Question**: How does LOAM Livox perform on the KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `loam_livox`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/loam_livox_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 200.533 | 8.4 | 61.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 277.148 | 36.6 | 86.2 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 201.053 | 9.8 | 63.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 36.6 FPS.
3. `default` is the most accurate observed variant at 200.533 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_kitti_raw_0061_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/loam_livox_kitti_raw_0061_full_matrix/default/summary.json`
- Log: `experiments/results/runs/loam_livox_kitti_raw_0061_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser map grids and fewer outer refinement passes for faster Livox-style LOAM updates.
- CLI args: `--loam-livox-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_kitti_raw_0061_full_matrix/fast/summary.json --loam-livox-fast-profile`
- Summary: `experiments/results/runs/loam_livox_kitti_raw_0061_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/loam_livox_kitti_raw_0061_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer map grids and extra outer refinement passes for a more exhaustive LOAM solve.
- CLI args: `--loam-livox-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_kitti_raw_0061_full_matrix/dense/summary.json --loam-livox-dense-profile`
- Summary: `experiments/results/runs/loam_livox_kitti_raw_0061_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/loam_livox_kitti_raw_0061_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).


## LOAM Livox on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `loam_livox_kitti_raw_0061`
- **Question**: How does LOAM Livox perform on the KITTI Raw drive 0061 (200 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `loam_livox`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/loam_livox_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 51.613 | 9.5 | 53.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 78.138 | 50.0 | 79.3 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 45.818 | 12.9 | 62.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 50.0 FPS.
3. `dense` is the most accurate observed variant at 45.818 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_kitti_raw_0061_matrix/default/summary.json`
- Summary: `experiments/results/runs/loam_livox_kitti_raw_0061_matrix/default/summary.json`
- Log: `experiments/results/runs/loam_livox_kitti_raw_0061_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser map grids and fewer outer refinement passes for faster Livox-style LOAM updates.
- CLI args: `--loam-livox-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_kitti_raw_0061_matrix/fast/summary.json --loam-livox-fast-profile`
- Summary: `experiments/results/runs/loam_livox_kitti_raw_0061_matrix/fast/summary.json`
- Log: `experiments/results/runs/loam_livox_kitti_raw_0061_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer map grids and extra outer refinement passes for a more exhaustive LOAM solve.
- CLI args: `--loam-livox-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_kitti_raw_0061_matrix/dense/summary.json --loam-livox-dense-profile`
- Summary: `experiments/results/runs/loam_livox_kitti_raw_0061_matrix/dense/summary.json`
- Log: `experiments/results/runs/loam_livox_kitti_raw_0061_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).


## LOAM Livox on MCD KTH day-06 sequence

- **Problem ID**: `loam_livox_mcd_kth_day_06`
- **Question**: How does LOAM Livox perform on the MCD KTH day-06 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `loam_livox`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/loam_livox_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 5.972 | 6.7 | 39.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 4.095 | 66.3 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 5.913 | 12.5 | 44.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 66.3 FPS.
3. `fast` is the most accurate observed variant at 4.095 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_mcd_kth_day_06_matrix/default/summary.json`
- Summary: `experiments/results/runs/loam_livox_mcd_kth_day_06_matrix/default/summary.json`
- Log: `experiments/results/runs/loam_livox_mcd_kth_day_06_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser map grids and fewer outer refinement passes for faster Livox-style LOAM updates.
- CLI args: `--loam-livox-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_mcd_kth_day_06_matrix/fast/summary.json --loam-livox-fast-profile`
- Summary: `experiments/results/runs/loam_livox_mcd_kth_day_06_matrix/fast/summary.json`
- Log: `experiments/results/runs/loam_livox_mcd_kth_day_06_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer map grids and extra outer refinement passes for a more exhaustive LOAM solve.
- CLI args: `--loam-livox-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_mcd_kth_day_06_matrix/dense/summary.json --loam-livox-dense-profile`
- Summary: `experiments/results/runs/loam_livox_mcd_kth_day_06_matrix/dense/summary.json`
- Log: `experiments/results/runs/loam_livox_mcd_kth_day_06_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).


## LOAM Livox on MCD NTU day-02 sequence

- **Problem ID**: `loam_livox_mcd_ntu_day_02`
- **Question**: How does LOAM Livox perform on the MCD NTU day-02 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `loam_livox`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/loam_livox_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 0.057 | 7.2 | 55.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.137 | 69.0 | 70.9 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 0.062 | 12.9 | 55.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 69.0 FPS.
3. `default` is the most accurate observed variant at 0.057 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_mcd_ntu_day_02_matrix/default/summary.json`
- Summary: `experiments/results/runs/loam_livox_mcd_ntu_day_02_matrix/default/summary.json`
- Log: `experiments/results/runs/loam_livox_mcd_ntu_day_02_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser map grids and fewer outer refinement passes for faster Livox-style LOAM updates.
- CLI args: `--loam-livox-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_mcd_ntu_day_02_matrix/fast/summary.json --loam-livox-fast-profile`
- Summary: `experiments/results/runs/loam_livox_mcd_ntu_day_02_matrix/fast/summary.json`
- Log: `experiments/results/runs/loam_livox_mcd_ntu_day_02_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer map grids and extra outer refinement passes for a more exhaustive LOAM solve.
- CLI args: `--loam-livox-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_mcd_ntu_day_02_matrix/dense/summary.json --loam-livox-dense-profile`
- Summary: `experiments/results/runs/loam_livox_mcd_ntu_day_02_matrix/dense/summary.json`
- Log: `experiments/results/runs/loam_livox_mcd_ntu_day_02_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).


## LOAM Livox on MCD TUHH night-09 sequence

- **Problem ID**: `loam_livox_mcd_tuhh_night_09`
- **Question**: How does LOAM Livox perform on the MCD TUHH night-09 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `loam_livox`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/loam_livox_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 1.311 | 8.2 | 51.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 1.192 | 68.8 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 1.309 | 12.6 | 54.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 68.8 FPS.
3. `fast` is the most accurate observed variant at 1.192 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_mcd_tuhh_night_09_matrix/default/summary.json`
- Summary: `experiments/results/runs/loam_livox_mcd_tuhh_night_09_matrix/default/summary.json`
- Log: `experiments/results/runs/loam_livox_mcd_tuhh_night_09_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser map grids and fewer outer refinement passes for faster Livox-style LOAM updates.
- CLI args: `--loam-livox-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_mcd_tuhh_night_09_matrix/fast/summary.json --loam-livox-fast-profile`
- Summary: `experiments/results/runs/loam_livox_mcd_tuhh_night_09_matrix/fast/summary.json`
- Log: `experiments/results/runs/loam_livox_mcd_tuhh_night_09_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer map grids and extra outer refinement passes for a more exhaustive LOAM solve.
- CLI args: `--loam-livox-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods loam_livox --summary-json experiments/results/runs/loam_livox_mcd_tuhh_night_09_matrix/dense/summary.json --loam-livox-dense-profile`
- Summary: `experiments/results/runs/loam_livox_mcd_tuhh_night_09_matrix/dense/summary.json`
- Log: `experiments/results/runs/loam_livox_mcd_tuhh_night_09_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).


## MULLS trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `mulls_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which MULLS profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `mulls`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/mulls_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| KITTI default | balanced | 4.933 | 1.2 | 64.5 | 4.65 | 4.75 | Keep as reference variant |
| Fast | throughput-oriented | 4.610 | 3.3 | 100.0 | 4.30 | 4.50 | Adopt as current default |
| Dense | accuracy-oriented | 5.004 | 1.0 | 60.9 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 3.3 FPS.
3. `fast` is the most accurate observed variant at 4.610 m ATE.

### Variant Notes

#### `kitti_default`

- Intent: A-LOAM-style features + multi-metric scan-to-map (line/plane/point) baseline for HDL-64E.
- CLI args: `--mulls-kitti-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods mulls --summary-json experiments/results/runs/mulls_kitti_raw_0009_full_matrix/kitti_default/summary.json --mulls-kitti-profile`
- Summary: `experiments/results/runs/mulls_kitti_raw_0009_full_matrix/kitti_default/summary.json`
- Log: `experiments/results/runs/mulls_kitti_raw_0009_full_matrix/kitti_default/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Multi-metric scan-to-map (line/plane/point Ceres) after A-LOAM-style feature odometry (no GT seed).

#### `fast`

- Intent: Coarser voxels, lighter Ceres, shorter submap window, sparser point metric.
- CLI args: `--mulls-kitti-profile --mulls-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods mulls --summary-json experiments/results/runs/mulls_kitti_raw_0009_full_matrix/fast/summary.json --mulls-kitti-profile --mulls-fast-profile`
- Summary: `experiments/results/runs/mulls_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/mulls_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Multi-metric scan-to-map (line/plane/point Ceres) after A-LOAM-style feature odometry (no GT seed).

#### `dense`

- Intent: Finer map voxels, more knn, deeper Ceres, longer history.
- CLI args: `--mulls-kitti-profile --mulls-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods mulls --summary-json experiments/results/runs/mulls_kitti_raw_0009_full_matrix/dense/summary.json --mulls-kitti-profile --mulls-dense-profile`
- Summary: `experiments/results/runs/mulls_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/mulls_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Multi-metric scan-to-map (line/plane/point Ceres) after A-LOAM-style feature odometry (no GT seed).


## NDT throughput and accuracy trade-off on the second public HDL-400 reference window

- **Problem ID**: `ndt_profile_tradeoff_hdl_400_reference_b`
- **Question**: Which NDT dogfooding profile should stay as the current default on a deeper HDL-400 public reference set while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120_b`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference_b.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_hdl_400_reference_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.090 | 0.5 | 63.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.065 | 0.9 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.087 | 0.2 | 51.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 0.9 FPS.
3. `fast_coarse_map` is the most accurate observed variant at 0.065 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods ndt --summary-json experiments/results/runs/ndt_hdl_400_reference_b_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_hdl_400_reference_b_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_hdl_400_reference_b_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods ndt --summary-json experiments/results/runs/ndt_hdl_400_reference_b_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_hdl_400_reference_b_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_hdl_400_reference_b_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120_b experiments/reference_data/hdl_400_public_reference_b.csv --methods ndt --summary-json experiments/results/runs/ndt_hdl_400_reference_b_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_hdl_400_reference_b_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_hdl_400_reference_b_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the public HDL-400 reference window

- **Problem ID**: `ndt_profile_tradeoff_hdl_400_reference`
- **Question**: Which NDT dogfooding profile should stay as the current default on a different public dataset family while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **Reference CSV**: `experiments/reference_data/hdl_400_public_reference.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_hdl_400_reference_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.034 | 20.9 | 82.4 | 5.00 | 5.00 | Keep as active challenger |
| Fast coarse map | throughput-oriented | 0.052 | 32.2 | 82.9 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.034 | 10.2 | 65.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 32.2 FPS.
3. `balanced_local_map` is the most accurate observed variant at 0.034 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ndt --summary-json experiments/results/runs/ndt_hdl_400_reference_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_hdl_400_reference_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_hdl_400_reference_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ndt --summary-json experiments/results/runs/ndt_hdl_400_reference_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_hdl_400_reference_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_hdl_400_reference_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 experiments/reference_data/hdl_400_public_reference.csv --methods ndt --summary-json experiments/results/runs/ndt_hdl_400_reference_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_hdl_400_reference_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_hdl_400_reference_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the second repository-stored Istanbul sequence

- **Problem ID**: `ndt_profile_tradeoff_istanbul_window_b`
- **Question**: Which NDT dogfooding profile should stay as the current default on a second public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_b`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_b_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_istanbul_window_b_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.062 | 1.5 | 41.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.007 | 2.1 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.067 | 1.0 | 30.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 2.1 FPS.
3. `fast_coarse_map` is the most accurate observed variant at 0.007 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_istanbul_window_b_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_istanbul_window_b_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_istanbul_window_b_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_istanbul_window_b_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_istanbul_window_b_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_istanbul_window_b_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_b dogfooding_results/autoware_istanbul_open_108_b_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_istanbul_window_b_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_istanbul_window_b_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_istanbul_window_b_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the third repository-stored Istanbul sequence

- **Problem ID**: `ndt_profile_tradeoff_istanbul_window_c`
- **Question**: Which NDT dogfooding profile should stay as the current default on a third public Istanbul window while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108_c`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_c_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_istanbul_window_c_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.035 | 1.4 | 44.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.005 | 1.9 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.105 | 1.0 | 28.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 1.9 FPS.
3. `fast_coarse_map` is the most accurate observed variant at 0.005 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_istanbul_window_c_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_istanbul_window_c_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_istanbul_window_c_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_istanbul_window_c_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_istanbul_window_c_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_istanbul_window_c_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108_c dogfooding_results/autoware_istanbul_open_108_c_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_istanbul_window_c_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_istanbul_window_c_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_istanbul_window_c_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `ndt_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which NDT profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.255 | 21.1 | 84.1 | 5.00 | 5.00 | Keep as active challenger |
| Fast coarse map | throughput-oriented | 0.307 | 30.9 | 91.5 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.291 | 14.5 | 67.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 30.9 FPS.
3. `balanced_local_map` is the most accurate observed variant at 0.255 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Fast.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_full_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_full_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_full_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Dense.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_full_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_full_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_full_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `ndt_profile_tradeoff_kitti_raw_0009`
- **Question**: Which NDT profile should stay as the current default on KITTI Raw drive 0009?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.279 | 22.9 | 81.7 | 5.00 | 5.00 | Keep as active challenger |
| Fast coarse map | throughput-oriented | 0.374 | 36.1 | 87.3 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.358 | 13.5 | 57.6 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 36.1 FPS.
3. `balanced_local_map` is the most accurate observed variant at 0.279 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT trade-off on KITTI Raw drive 0009 (200 frames, no GT seed)

- **Problem ID**: `ndt_profile_tradeoff_kitti_raw_0009_nogt`
- **Question**: Which NDT profile on KITTI Raw drive 0009 (200 frames, no GT seed)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_kitti_raw_0009_nogt_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 122.662 | 22.8 | 95.2 | 4.65 | 4.75 | Keep as active challenger |
| Fast coarse map | throughput-oriented | 122.547 | 25.0 | 99.7 | 4.30 | 4.50 | Adopt as current default |
| Dense local map | accuracy-oriented | 121.733 | 12.2 | 74.4 | 4.30 | 4.50 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 25.0 FPS.
3. `dense_local_map` is the most accurate observed variant at 121.733 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/balanced_local_map/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/balanced_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

#### `fast_coarse_map`

- Intent: Fast.
- CLI args: `--ndt-fast-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/fast_coarse_map/summary.json --ndt-fast-profile --no-gt-seed`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

#### `dense_local_map`

- Intent: Dense.
- CLI args: `--ndt-dense-profile --no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/dense_local_map/summary.json --ndt-dense-profile --no-gt-seed`
- Summary: `experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0009_nogt_matrix/dense_local_map/run.log`
- Readability proxy: 4.30 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.50 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).


## NDT trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `ndt_profile_tradeoff_kitti_raw_0061_full`
- **Question**: Which NDT profile on KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.259 | 15.3 | 79.7 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.247 | 23.8 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.276 | 10.1 | 66.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 23.8 FPS.
3. `fast_coarse_map` is the most accurate observed variant at 0.247 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0061_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_kitti_raw_0061_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0061_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Fast.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0061_full_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0061_full_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0061_full_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Dense.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0061_full_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0061_full_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0061_full_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `ndt_profile_tradeoff_kitti_raw_0061`
- **Question**: Which NDT profile should stay as the current default on KITTI Raw drive 0061?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.325 | 28.4 | 83.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.319 | 41.2 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.361 | 15.5 | 63.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 41.2 FPS.
3. `fast_coarse_map` is the most accurate observed variant at 0.319 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0061_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_kitti_raw_0061_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0061_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0061_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0061_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0061_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_kitti_raw_0061_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_kitti_raw_0061_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_kitti_raw_0061_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `ndt_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which NDT profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.166 | 18.3 | 70.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.208 | 31.2 | 82.7 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.136 | 10.9 | 67.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 31.2 FPS.
3. `dense_local_map` is the most accurate observed variant at 0.136 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_kth_day_06_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_kth_day_06_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_mcd_kth_day_06_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_kth_day_06_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_kth_day_06_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_mcd_kth_day_06_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_kth_day_06_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `ndt_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which NDT profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.014 | 32.7 | 80.8 | 5.00 | 5.00 | Adopt as current default |
| Fast coarse map | throughput-oriented | 0.024 | 44.9 | 76.0 | 4.65 | 4.75 | Keep as active challenger |
| Dense local map | accuracy-oriented | 0.013 | 22.0 | 74.5 | 4.65 | 4.75 | Keep as active challenger |

### Observations

1. `balanced_local_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 44.9 FPS.
3. `dense_local_map` is the most accurate observed variant at 0.013 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_ntu_day_02_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_ntu_day_02_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_mcd_ntu_day_02_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_ntu_day_02_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_ntu_day_02_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_mcd_ntu_day_02_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_ntu_day_02_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `ndt_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which NDT profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.075 | 26.0 | 74.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.070 | 40.8 | 94.7 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.063 | 14.2 | 67.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 40.8 FPS.
3. `dense_local_map` is the most accurate observed variant at 0.063 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_mcd_tuhh_night_09_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## NDT throughput and accuracy trade-off on the repository-stored Istanbul sequence

- **Problem ID**: `ndt_profile_tradeoff`
- **Question**: Which NDT dogfooding profile should stay as the current default while keeping faster and denser alternatives comparable?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/autoware_istanbul_open_108`
- **Reference CSV**: `dogfooding_results/autoware_istanbul_open_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `ndt`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/ndt_profile_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.109 | 1.4 | 67.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast coarse map | throughput-oriented | 0.070 | 2.0 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense local map | accuracy-oriented | 0.122 | 1.0 | 54.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_coarse_map` is the current default for this problem.
2. `fast_coarse_map` is the fastest observed variant at 2.0 FPS.
3. `fast_coarse_map` is the most accurate observed variant at 0.070 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Keep the current repository default as the baseline trade-off.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_profile_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/ndt_profile_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/ndt_profile_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_coarse_map`

- Intent: Use a coarser map and fewer points to increase FPS.
- CLI args: `--ndt-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_profile_matrix/fast_coarse_map/summary.json --ndt-fast-profile`
- Summary: `experiments/results/runs/ndt_profile_matrix/fast_coarse_map/summary.json`
- Log: `experiments/results/runs/ndt_profile_matrix/fast_coarse_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_local_map`

- Intent: Use denser scans and a larger map budget for a stronger refinement path.
- CLI args: `--ndt-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/autoware_istanbul_open_108 dogfooding_results/autoware_istanbul_open_108_gt.csv --methods ndt --summary-json experiments/results/runs/ndt_profile_matrix/dense_local_map/summary.json --ndt-dense-profile`
- Summary: `experiments/results/runs/ndt_profile_matrix/dense_local_map/summary.json`
- Log: `experiments/results/runs/ndt_profile_matrix/dense_local_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Point-LIO on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `point_lio_kitti_raw_0009_full`
- **Question**: How does Point-LIO perform on the KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `point_lio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/point_lio_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 183.631 | 4.8 | 52.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 183.384 | 113.1 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 183.725 | 37.8 | 66.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 113.1 FPS.
3. `fast` is the most accurate observed variant at 183.384 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_kitti_raw_0009_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/point_lio_kitti_raw_0009_full_matrix/default/summary.json`
- Log: `experiments/results/runs/point_lio_kitti_raw_0009_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser registration and a smaller local map to speed up point-level Kalman updates; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_kitti_raw_0009_full_matrix/fast/summary.json --point-lio-fast-profile`
- Summary: `experiments/results/runs/point_lio_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/point_lio_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer registration and a larger local map with more correspondences; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_kitti_raw_0009_full_matrix/dense/summary.json --point-lio-dense-profile`
- Summary: `experiments/results/runs/point_lio_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/point_lio_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## Point-LIO on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `point_lio_kitti_raw_0009`
- **Question**: How does Point-LIO perform on the KITTI Raw drive 0009 (200 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `point_lio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/point_lio_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 120.082 | 5.4 | 52.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 119.890 | 117.4 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 120.217 | 33.0 | 63.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 117.4 FPS.
3. `fast` is the most accurate observed variant at 119.890 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_kitti_raw_0009_matrix/default/summary.json`
- Summary: `experiments/results/runs/point_lio_kitti_raw_0009_matrix/default/summary.json`
- Log: `experiments/results/runs/point_lio_kitti_raw_0009_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser registration and a smaller local map to speed up point-level Kalman updates; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_kitti_raw_0009_matrix/fast/summary.json --point-lio-fast-profile`
- Summary: `experiments/results/runs/point_lio_kitti_raw_0009_matrix/fast/summary.json`
- Log: `experiments/results/runs/point_lio_kitti_raw_0009_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer registration and a larger local map with more correspondences; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_kitti_raw_0009_matrix/dense/summary.json --point-lio-dense-profile`
- Summary: `experiments/results/runs/point_lio_kitti_raw_0009_matrix/dense/summary.json`
- Log: `experiments/results/runs/point_lio_kitti_raw_0009_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## Point-LIO on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `point_lio_kitti_raw_0061_full`
- **Question**: How does Point-LIO perform on the KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `point_lio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/point_lio_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 292.255 | 2.3 | 51.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 292.011 | 89.5 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 292.444 | 11.4 | 56.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 89.5 FPS.
3. `fast` is the most accurate observed variant at 292.011 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_kitti_raw_0061_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/point_lio_kitti_raw_0061_full_matrix/default/summary.json`
- Log: `experiments/results/runs/point_lio_kitti_raw_0061_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser registration and a smaller local map to speed up point-level Kalman updates; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_kitti_raw_0061_full_matrix/fast/summary.json --point-lio-fast-profile`
- Summary: `experiments/results/runs/point_lio_kitti_raw_0061_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/point_lio_kitti_raw_0061_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer registration and a larger local map with more correspondences; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_kitti_raw_0061_full_matrix/dense/summary.json --point-lio-dense-profile`
- Summary: `experiments/results/runs/point_lio_kitti_raw_0061_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/point_lio_kitti_raw_0061_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## Point-LIO on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `point_lio_kitti_raw_0061`
- **Question**: How does Point-LIO perform on the KITTI Raw drive 0061 (200 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `point_lio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/point_lio_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 82.702 | 2.1 | 51.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 82.450 | 92.8 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 82.858 | 9.1 | 54.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 92.8 FPS.
3. `fast` is the most accurate observed variant at 82.450 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_kitti_raw_0061_matrix/default/summary.json`
- Summary: `experiments/results/runs/point_lio_kitti_raw_0061_matrix/default/summary.json`
- Log: `experiments/results/runs/point_lio_kitti_raw_0061_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser registration and a smaller local map to speed up point-level Kalman updates; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_kitti_raw_0061_matrix/fast/summary.json --point-lio-fast-profile`
- Summary: `experiments/results/runs/point_lio_kitti_raw_0061_matrix/fast/summary.json`
- Log: `experiments/results/runs/point_lio_kitti_raw_0061_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer registration and a larger local map with more correspondences; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_kitti_raw_0061_matrix/dense/summary.json --point-lio-dense-profile`
- Summary: `experiments/results/runs/point_lio_kitti_raw_0061_matrix/dense/summary.json`
- Log: `experiments/results/runs/point_lio_kitti_raw_0061_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## Point-LIO on MCD KTH day-06 sequence

- **Problem ID**: `point_lio_mcd_kth_day_06`
- **Question**: How does Point-LIO perform on the MCD KTH day-06 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `point_lio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/point_lio_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 7.113 | 1.2 | 50.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 7.325 | 112.7 | 98.5 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 7.298 | 14.7 | 55.2 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 112.7 FPS.
3. `default` is the most accurate observed variant at 7.113 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_mcd_kth_day_06_matrix/default/summary.json`
- Summary: `experiments/results/runs/point_lio_mcd_kth_day_06_matrix/default/summary.json`
- Log: `experiments/results/runs/point_lio_mcd_kth_day_06_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser registration and a smaller local map to speed up point-level Kalman updates; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_mcd_kth_day_06_matrix/fast/summary.json --point-lio-fast-profile`
- Summary: `experiments/results/runs/point_lio_mcd_kth_day_06_matrix/fast/summary.json`
- Log: `experiments/results/runs/point_lio_mcd_kth_day_06_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer registration and a larger local map with more correspondences; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_mcd_kth_day_06_matrix/dense/summary.json --point-lio-dense-profile`
- Summary: `experiments/results/runs/point_lio_mcd_kth_day_06_matrix/dense/summary.json`
- Log: `experiments/results/runs/point_lio_mcd_kth_day_06_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## Point-LIO on MCD NTU day-02 sequence

- **Problem ID**: `point_lio_mcd_ntu_day_02`
- **Question**: How does Point-LIO perform on the MCD NTU day-02 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `point_lio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/point_lio_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 0.234 | 1.4 | 18.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.083 | 77.3 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 0.096 | 3.3 | 45.4 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 77.3 FPS.
3. `fast` is the most accurate observed variant at 0.083 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_mcd_ntu_day_02_matrix/default/summary.json`
- Summary: `experiments/results/runs/point_lio_mcd_ntu_day_02_matrix/default/summary.json`
- Log: `experiments/results/runs/point_lio_mcd_ntu_day_02_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser registration and a smaller local map to speed up point-level Kalman updates; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_mcd_ntu_day_02_matrix/fast/summary.json --point-lio-fast-profile`
- Summary: `experiments/results/runs/point_lio_mcd_ntu_day_02_matrix/fast/summary.json`
- Log: `experiments/results/runs/point_lio_mcd_ntu_day_02_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer registration and a larger local map with more correspondences; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_mcd_ntu_day_02_matrix/dense/summary.json --point-lio-dense-profile`
- Summary: `experiments/results/runs/point_lio_mcd_ntu_day_02_matrix/dense/summary.json`
- Log: `experiments/results/runs/point_lio_mcd_ntu_day_02_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## Point-LIO on MCD TUHH night-09 sequence

- **Problem ID**: `point_lio_mcd_tuhh_night_09`
- **Question**: How does Point-LIO perform on the MCD TUHH night-09 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `point_lio`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/point_lio_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (no IMU) | balanced | 1.116 | 0.8 | 50.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 1.158 | 88.7 | 98.2 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 1.128 | 2.8 | 51.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 88.7 FPS.
3. `default` is the most accurate observed variant at 1.116 m ATE.

### Variant Notes

#### `default`

- Intent: LiDAR-only mode; no IMU data available in KITTI Raw fixtures.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_mcd_tuhh_night_09_matrix/default/summary.json`
- Summary: `experiments/results/runs/point_lio_mcd_tuhh_night_09_matrix/default/summary.json`
- Log: `experiments/results/runs/point_lio_mcd_tuhh_night_09_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser registration and a smaller local map to speed up point-level Kalman updates; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_mcd_tuhh_night_09_matrix/fast/summary.json --point-lio-fast-profile`
- Summary: `experiments/results/runs/point_lio_mcd_tuhh_night_09_matrix/fast/summary.json`
- Log: `experiments/results/runs/point_lio_mcd_tuhh_night_09_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer registration and a larger local map with more correspondences; remains LiDAR-only when imu.csv is absent.
- CLI args: `--point-lio-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods point_lio --summary-json experiments/results/runs/point_lio_mcd_tuhh_night_09_matrix/dense/summary.json --point-lio-dense-profile`
- Summary: `experiments/results/runs/point_lio_mcd_tuhh_night_09_matrix/dense/summary.json`
- Log: `experiments/results/runs/point_lio_mcd_tuhh_night_09_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Point-level LiDAR-inertial odometry with error-state iterated Kalman filter (no GT seed; anchor matches first GT pose).


## Small-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `small_gicp_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which Small-GICP profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `small_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/small_gicp_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.105 | 51.8 | 47.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.437 | 92.4 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 1.090 | 41.0 | 42.2 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 92.4 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.437 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--small-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json --small-gicp-fast-profile`
- Summary: `experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--small-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/dense_recent_map/summary.json --small-gicp-dense-profile`
- Summary: `experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_kitti_raw_0009_full_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Small-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `small_gicp_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which Small-GICP profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `small_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/small_gicp_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.070 | 57.7 | 64.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.806 | 107.9 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 0.898 | 37.5 | 62.2 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 107.9 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.806 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--small-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/fast_recent_map/summary.json --small-gicp-fast-profile`
- Summary: `experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--small-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/dense_recent_map/summary.json --small-gicp-dense-profile`
- Summary: `experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_kth_day_06_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Small-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `small_gicp_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which Small-GICP profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `small_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/small_gicp_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.101 | 69.6 | 45.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.111 | 113.8 | 63.8 | 4.65 | 4.75 | Keep as reference variant |
| Dense recent map | accuracy-oriented | 0.031 | 56.8 | 74.9 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 113.8 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.031 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--small-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json --small-gicp-fast-profile`
- Summary: `experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--small-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/dense_recent_map/summary.json --small-gicp-dense-profile`
- Summary: `experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_ntu_day_02_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Small-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `small_gicp_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which Small-GICP profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `small_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/small_gicp_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.325 | 54.1 | 63.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.466 | 107.2 | 76.8 | 4.65 | 4.75 | Adopt as current default |
| Dense recent map | accuracy-oriented | 0.250 | 40.2 | 68.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast_recent_map` is the current default for this problem.
2. `fast_recent_map` is the fastest observed variant at 107.2 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.250 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Default.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Fast.
- CLI args: `--small-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json --small-gicp-fast-profile`
- Summary: `experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Dense.
- CLI args: `--small-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods small_gicp --summary-json experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/dense_recent_map/summary.json --small-gicp-dense-profile`
- Summary: `experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/small_gicp_mcd_tuhh_night_09_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## SuMa on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `suma_kitti_raw_0009_full`
- **Question**: How does SuMa perform on the KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/suma_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 14.181 | 35.0 | 39.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 116.600 | 85.5 | 52.4 | 4.65 | 4.75 | Keep as reference variant |
| Dense | accuracy-oriented | 5.487 | 25.0 | 64.6 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense` is the current default for this problem.
2. `fast` is the fastest observed variant at 85.5 FPS.
3. `dense` is the most accurate observed variant at 5.487 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods suma --summary-json experiments/results/runs/suma_kitti_raw_0009_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/suma_kitti_raw_0009_full_matrix/default/summary.json`
- Log: `experiments/results/runs/suma_kitti_raw_0009_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser surfels and a shorter history window for faster dense mapping.
- CLI args: `--suma-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods suma --summary-json experiments/results/runs/suma_kitti_raw_0009_full_matrix/fast/summary.json --suma-fast-profile`
- Summary: `experiments/results/runs/suma_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/suma_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer surfels and a longer history window for a denser dense-mapping solve.
- CLI args: `--suma-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods suma --summary-json experiments/results/runs/suma_kitti_raw_0009_full_matrix/dense/summary.json --suma-dense-profile`
- Summary: `experiments/results/runs/suma_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/suma_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).


## SuMa on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `suma_kitti_raw_0009`
- **Question**: How does SuMa perform on the KITTI Raw drive 0009 (200 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/suma_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 3.291 | 39.7 | 73.8 | 5.00 | 5.00 | Adopt as current default |
| Fast | throughput-oriented | 63.525 | 62.9 | 52.2 | 4.65 | 4.75 | Keep as reference variant |
| Dense | accuracy-oriented | 2.784 | 28.1 | 72.3 | 4.65 | 4.75 | Keep as active challenger |

### Observations

1. `default` is the current default for this problem.
2. `fast` is the fastest observed variant at 62.9 FPS.
3. `dense` is the most accurate observed variant at 2.784 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods suma --summary-json experiments/results/runs/suma_kitti_raw_0009_matrix/default/summary.json`
- Summary: `experiments/results/runs/suma_kitti_raw_0009_matrix/default/summary.json`
- Log: `experiments/results/runs/suma_kitti_raw_0009_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser surfels and a shorter history window for faster dense mapping.
- CLI args: `--suma-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods suma --summary-json experiments/results/runs/suma_kitti_raw_0009_matrix/fast/summary.json --suma-fast-profile`
- Summary: `experiments/results/runs/suma_kitti_raw_0009_matrix/fast/summary.json`
- Log: `experiments/results/runs/suma_kitti_raw_0009_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer surfels and a longer history window for a denser dense-mapping solve.
- CLI args: `--suma-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods suma --summary-json experiments/results/runs/suma_kitti_raw_0009_matrix/dense/summary.json --suma-dense-profile`
- Summary: `experiments/results/runs/suma_kitti_raw_0009_matrix/dense/summary.json`
- Log: `experiments/results/runs/suma_kitti_raw_0009_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).


## SuMa on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `suma_kitti_raw_0061_full`
- **Question**: How does SuMa perform on the KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/suma_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 19.448 | 24.4 | 50.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 32.429 | 110.9 | 73.7 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 15.381 | 28.9 | 63.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 110.9 FPS.
3. `dense` is the most accurate observed variant at 15.381 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods suma --summary-json experiments/results/runs/suma_kitti_raw_0061_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/suma_kitti_raw_0061_full_matrix/default/summary.json`
- Log: `experiments/results/runs/suma_kitti_raw_0061_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser surfels and a shorter history window for faster dense mapping.
- CLI args: `--suma-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods suma --summary-json experiments/results/runs/suma_kitti_raw_0061_full_matrix/fast/summary.json --suma-fast-profile`
- Summary: `experiments/results/runs/suma_kitti_raw_0061_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/suma_kitti_raw_0061_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer surfels and a longer history window for a denser dense-mapping solve.
- CLI args: `--suma-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods suma --summary-json experiments/results/runs/suma_kitti_raw_0061_full_matrix/dense/summary.json --suma-dense-profile`
- Summary: `experiments/results/runs/suma_kitti_raw_0061_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/suma_kitti_raw_0061_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).


## SuMa on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `suma_kitti_raw_0061`
- **Question**: How does SuMa perform on the KITTI Raw drive 0061 (200 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/suma_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 3.877 | 31.6 | 33.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 16.071 | 111.2 | 54.6 | 4.65 | 4.75 | Keep as reference variant |
| Dense | accuracy-oriented | 1.496 | 33.5 | 65.1 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense` is the current default for this problem.
2. `fast` is the fastest observed variant at 111.2 FPS.
3. `dense` is the most accurate observed variant at 1.496 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods suma --summary-json experiments/results/runs/suma_kitti_raw_0061_matrix/default/summary.json`
- Summary: `experiments/results/runs/suma_kitti_raw_0061_matrix/default/summary.json`
- Log: `experiments/results/runs/suma_kitti_raw_0061_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser surfels and a shorter history window for faster dense mapping.
- CLI args: `--suma-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods suma --summary-json experiments/results/runs/suma_kitti_raw_0061_matrix/fast/summary.json --suma-fast-profile`
- Summary: `experiments/results/runs/suma_kitti_raw_0061_matrix/fast/summary.json`
- Log: `experiments/results/runs/suma_kitti_raw_0061_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer surfels and a longer history window for a denser dense-mapping solve.
- CLI args: `--suma-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods suma --summary-json experiments/results/runs/suma_kitti_raw_0061_matrix/dense/summary.json --suma-dense-profile`
- Summary: `experiments/results/runs/suma_kitti_raw_0061_matrix/dense/summary.json`
- Log: `experiments/results/runs/suma_kitti_raw_0061_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).


## SuMa on MCD KTH day-06 sequence

- **Problem ID**: `suma_mcd_kth_day_06`
- **Question**: How does SuMa perform on the MCD KTH day-06 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/suma_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 6.064 | 22.7 | 57.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 7.419 | 150.2 | 90.9 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 6.064 | 26.6 | 58.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 150.2 FPS.
3. `dense` is the most accurate observed variant at 6.064 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods suma --summary-json experiments/results/runs/suma_mcd_kth_day_06_matrix/default/summary.json`
- Summary: `experiments/results/runs/suma_mcd_kth_day_06_matrix/default/summary.json`
- Log: `experiments/results/runs/suma_mcd_kth_day_06_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser surfels and a shorter history window for faster dense mapping.
- CLI args: `--suma-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods suma --summary-json experiments/results/runs/suma_mcd_kth_day_06_matrix/fast/summary.json --suma-fast-profile`
- Summary: `experiments/results/runs/suma_mcd_kth_day_06_matrix/fast/summary.json`
- Log: `experiments/results/runs/suma_mcd_kth_day_06_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer surfels and a longer history window for a denser dense-mapping solve.
- CLI args: `--suma-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods suma --summary-json experiments/results/runs/suma_mcd_kth_day_06_matrix/dense/summary.json --suma-dense-profile`
- Summary: `experiments/results/runs/suma_mcd_kth_day_06_matrix/dense/summary.json`
- Log: `experiments/results/runs/suma_mcd_kth_day_06_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).


## SuMa on MCD NTU day-02 sequence

- **Problem ID**: `suma_mcd_ntu_day_02`
- **Question**: How does SuMa perform on the MCD NTU day-02 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/suma_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 0.065 | 29.1 | 39.2 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 2.101 | 124.1 | 50.9 | 4.65 | 4.75 | Keep as reference variant |
| Dense | accuracy-oriented | 0.036 | 33.9 | 63.7 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense` is the current default for this problem.
2. `fast` is the fastest observed variant at 124.1 FPS.
3. `dense` is the most accurate observed variant at 0.036 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods suma --summary-json experiments/results/runs/suma_mcd_ntu_day_02_matrix/default/summary.json`
- Summary: `experiments/results/runs/suma_mcd_ntu_day_02_matrix/default/summary.json`
- Log: `experiments/results/runs/suma_mcd_ntu_day_02_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser surfels and a shorter history window for faster dense mapping.
- CLI args: `--suma-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods suma --summary-json experiments/results/runs/suma_mcd_ntu_day_02_matrix/fast/summary.json --suma-fast-profile`
- Summary: `experiments/results/runs/suma_mcd_ntu_day_02_matrix/fast/summary.json`
- Log: `experiments/results/runs/suma_mcd_ntu_day_02_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer surfels and a longer history window for a denser dense-mapping solve.
- CLI args: `--suma-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods suma --summary-json experiments/results/runs/suma_mcd_ntu_day_02_matrix/dense/summary.json --suma-dense-profile`
- Summary: `experiments/results/runs/suma_mcd_ntu_day_02_matrix/dense/summary.json`
- Log: `experiments/results/runs/suma_mcd_ntu_day_02_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).


## SuMa on MCD TUHH night-09 sequence

- **Problem ID**: `suma_mcd_tuhh_night_09`
- **Question**: How does SuMa perform on the MCD TUHH night-09 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `suma`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/suma_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 1.414 | 59.1 | 63.2 | 5.00 | 5.00 | Adopt as current default |
| Fast | throughput-oriented | 91.620 | 178.1 | 50.7 | 4.65 | 4.75 | Keep as reference variant |
| Dense | accuracy-oriented | 1.317 | 45.5 | 62.8 | 4.65 | 4.75 | Keep as active challenger |

### Observations

1. `default` is the current default for this problem.
2. `fast` is the fastest observed variant at 178.1 FPS.
3. `dense` is the most accurate observed variant at 1.317 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods suma --summary-json experiments/results/runs/suma_mcd_tuhh_night_09_matrix/default/summary.json`
- Summary: `experiments/results/runs/suma_mcd_tuhh_night_09_matrix/default/summary.json`
- Log: `experiments/results/runs/suma_mcd_tuhh_night_09_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Use coarser surfels and a shorter history window for faster dense mapping.
- CLI args: `--suma-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods suma --summary-json experiments/results/runs/suma_mcd_tuhh_night_09_matrix/fast/summary.json --suma-fast-profile`
- Summary: `experiments/results/runs/suma_mcd_tuhh_night_09_matrix/fast/summary.json`
- Log: `experiments/results/runs/suma_mcd_tuhh_night_09_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Use finer surfels and a longer history window for a denser dense-mapping solve.
- CLI args: `--suma-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods suma --summary-json experiments/results/runs/suma_mcd_tuhh_night_09_matrix/dense/summary.json --suma-dense-profile`
- Summary: `experiments/results/runs/suma_mcd_tuhh_night_09_matrix/dense/summary.json`
- Log: `experiments/results/runs/suma_mcd_tuhh_night_09_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Surfel-based mapping (no GT seed; anchor matches first GT pose).


## VGICP SLAM on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `vgicp_slam_kitti_raw_0009_full`
- **Question**: How does VGICP SLAM perform on the KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `vgicp_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/vgicp_slam_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 3.948 | 5.4 | 62.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 4.544 | 22.2 | 93.5 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 4.690 | 10.5 | 65.8 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 22.2 FPS.
3. `default` is the most accurate observed variant at 3.948 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_kitti_raw_0009_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/vgicp_slam_kitti_raw_0009_full_matrix/default/summary.json`
- Log: `experiments/results/runs/vgicp_slam_kitti_raw_0009_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen registration and submap density while reducing keyframe and loop-closure frequency for faster pose-graph updates.
- CLI args: `--vgicp-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_kitti_raw_0009_full_matrix/fast/summary.json --vgicp-slam-fast-profile`
- Summary: `experiments/results/runs/vgicp_slam_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/vgicp_slam_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Densify registration and submaps while revisiting keyframes and loop closure more aggressively.
- CLI args: `--vgicp-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_kitti_raw_0009_full_matrix/dense/summary.json --vgicp-slam-dense-profile`
- Summary: `experiments/results/runs/vgicp_slam_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/vgicp_slam_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## VGICP SLAM on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `vgicp_slam_kitti_raw_0009`
- **Question**: How does VGICP SLAM perform on the KITTI Raw drive 0009 (200 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `vgicp_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/vgicp_slam_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 1.772 | 7.7 | 66.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 2.085 | 23.7 | 92.5 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 2.104 | 10.4 | 63.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 23.7 FPS.
3. `default` is the most accurate observed variant at 1.772 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_kitti_raw_0009_matrix/default/summary.json`
- Summary: `experiments/results/runs/vgicp_slam_kitti_raw_0009_matrix/default/summary.json`
- Log: `experiments/results/runs/vgicp_slam_kitti_raw_0009_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen registration and submap density while reducing keyframe and loop-closure frequency for faster pose-graph updates.
- CLI args: `--vgicp-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_kitti_raw_0009_matrix/fast/summary.json --vgicp-slam-fast-profile`
- Summary: `experiments/results/runs/vgicp_slam_kitti_raw_0009_matrix/fast/summary.json`
- Log: `experiments/results/runs/vgicp_slam_kitti_raw_0009_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Densify registration and submaps while revisiting keyframes and loop closure more aggressively.
- CLI args: `--vgicp-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_kitti_raw_0009_matrix/dense/summary.json --vgicp-slam-dense-profile`
- Summary: `experiments/results/runs/vgicp_slam_kitti_raw_0009_matrix/dense/summary.json`
- Log: `experiments/results/runs/vgicp_slam_kitti_raw_0009_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## VGICP SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `vgicp_slam_kitti_raw_0061_full`
- **Question**: How does VGICP SLAM perform on the KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `vgicp_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/vgicp_slam_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 5.917 | 5.4 | 56.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 5.230 | 22.8 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 6.127 | 10.4 | 65.5 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 22.8 FPS.
3. `fast` is the most accurate observed variant at 5.230 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_kitti_raw_0061_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/vgicp_slam_kitti_raw_0061_full_matrix/default/summary.json`
- Log: `experiments/results/runs/vgicp_slam_kitti_raw_0061_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen registration and submap density while reducing keyframe and loop-closure frequency for faster pose-graph updates.
- CLI args: `--vgicp-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_kitti_raw_0061_full_matrix/fast/summary.json --vgicp-slam-fast-profile`
- Summary: `experiments/results/runs/vgicp_slam_kitti_raw_0061_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/vgicp_slam_kitti_raw_0061_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Densify registration and submaps while revisiting keyframes and loop closure more aggressively.
- CLI args: `--vgicp-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_kitti_raw_0061_full_matrix/dense/summary.json --vgicp-slam-dense-profile`
- Summary: `experiments/results/runs/vgicp_slam_kitti_raw_0061_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/vgicp_slam_kitti_raw_0061_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## VGICP SLAM on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `vgicp_slam_kitti_raw_0061`
- **Question**: How does VGICP SLAM perform on the KITTI Raw drive 0061 (200 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `vgicp_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/vgicp_slam_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 0.803 | 8.6 | 63.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.903 | 31.0 | 94.3 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 0.799 | 17.3 | 77.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 31.0 FPS.
3. `dense` is the most accurate observed variant at 0.799 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_kitti_raw_0061_matrix/default/summary.json`
- Summary: `experiments/results/runs/vgicp_slam_kitti_raw_0061_matrix/default/summary.json`
- Log: `experiments/results/runs/vgicp_slam_kitti_raw_0061_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen registration and submap density while reducing keyframe and loop-closure frequency for faster pose-graph updates.
- CLI args: `--vgicp-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_kitti_raw_0061_matrix/fast/summary.json --vgicp-slam-fast-profile`
- Summary: `experiments/results/runs/vgicp_slam_kitti_raw_0061_matrix/fast/summary.json`
- Log: `experiments/results/runs/vgicp_slam_kitti_raw_0061_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Densify registration and submaps while revisiting keyframes and loop closure more aggressively.
- CLI args: `--vgicp-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_kitti_raw_0061_matrix/dense/summary.json --vgicp-slam-dense-profile`
- Summary: `experiments/results/runs/vgicp_slam_kitti_raw_0061_matrix/dense/summary.json`
- Log: `experiments/results/runs/vgicp_slam_kitti_raw_0061_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## VGICP SLAM on MCD KTH day-06 sequence

- **Problem ID**: `vgicp_slam_mcd_kth_day_06`
- **Question**: How does VGICP SLAM perform on the MCD KTH day-06 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `vgicp_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/vgicp_slam_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 6.092 | 4.5 | 60.9 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 6.096 | 20.8 | 100.0 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 6.099 | 9.9 | 73.8 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 20.8 FPS.
3. `default` is the most accurate observed variant at 6.092 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_mcd_kth_day_06_matrix/default/summary.json`
- Summary: `experiments/results/runs/vgicp_slam_mcd_kth_day_06_matrix/default/summary.json`
- Log: `experiments/results/runs/vgicp_slam_mcd_kth_day_06_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen registration and submap density while reducing keyframe and loop-closure frequency for faster pose-graph updates.
- CLI args: `--vgicp-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_mcd_kth_day_06_matrix/fast/summary.json --vgicp-slam-fast-profile`
- Summary: `experiments/results/runs/vgicp_slam_mcd_kth_day_06_matrix/fast/summary.json`
- Log: `experiments/results/runs/vgicp_slam_mcd_kth_day_06_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Densify registration and submaps while revisiting keyframes and loop closure more aggressively.
- CLI args: `--vgicp-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_mcd_kth_day_06_matrix/dense/summary.json --vgicp-slam-dense-profile`
- Summary: `experiments/results/runs/vgicp_slam_mcd_kth_day_06_matrix/dense/summary.json`
- Log: `experiments/results/runs/vgicp_slam_mcd_kth_day_06_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## VGICP SLAM on MCD NTU day-02 sequence

- **Problem ID**: `vgicp_slam_mcd_ntu_day_02`
- **Question**: How does VGICP SLAM perform on the MCD NTU day-02 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `vgicp_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/vgicp_slam_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 0.020 | 9.4 | 52.4 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 0.026 | 40.3 | 80.5 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 0.016 | 12.3 | 65.2 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 40.3 FPS.
3. `dense` is the most accurate observed variant at 0.016 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_mcd_ntu_day_02_matrix/default/summary.json`
- Summary: `experiments/results/runs/vgicp_slam_mcd_ntu_day_02_matrix/default/summary.json`
- Log: `experiments/results/runs/vgicp_slam_mcd_ntu_day_02_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen registration and submap density while reducing keyframe and loop-closure frequency for faster pose-graph updates.
- CLI args: `--vgicp-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_mcd_ntu_day_02_matrix/fast/summary.json --vgicp-slam-fast-profile`
- Summary: `experiments/results/runs/vgicp_slam_mcd_ntu_day_02_matrix/fast/summary.json`
- Log: `experiments/results/runs/vgicp_slam_mcd_ntu_day_02_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Densify registration and submaps while revisiting keyframes and loop closure more aggressively.
- CLI args: `--vgicp-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_mcd_ntu_day_02_matrix/dense/summary.json --vgicp-slam-dense-profile`
- Summary: `experiments/results/runs/vgicp_slam_mcd_ntu_day_02_matrix/dense/summary.json`
- Log: `experiments/results/runs/vgicp_slam_mcd_ntu_day_02_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## VGICP SLAM on MCD TUHH night-09 sequence

- **Problem ID**: `vgicp_slam_mcd_tuhh_night_09`
- **Question**: How does VGICP SLAM perform on the MCD TUHH night-09 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `vgicp_slam`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/vgicp_slam_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default | balanced | 1.319 | 6.0 | 63.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast | throughput-oriented | 1.322 | 21.8 | 99.9 | 4.65 | 4.75 | Adopt as current default |
| Dense | accuracy-oriented | 1.325 | 10.2 | 73.1 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 21.8 FPS.
3. `default` is the most accurate observed variant at 1.319 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_mcd_tuhh_night_09_matrix/default/summary.json`
- Summary: `experiments/results/runs/vgicp_slam_mcd_tuhh_night_09_matrix/default/summary.json`
- Log: `experiments/results/runs/vgicp_slam_mcd_tuhh_night_09_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `fast`

- Intent: Coarsen registration and submap density while reducing keyframe and loop-closure frequency for faster pose-graph updates.
- CLI args: `--vgicp-slam-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_mcd_tuhh_night_09_matrix/fast/summary.json --vgicp-slam-fast-profile`
- Summary: `experiments/results/runs/vgicp_slam_mcd_tuhh_night_09_matrix/fast/summary.json`
- Log: `experiments/results/runs/vgicp_slam_mcd_tuhh_night_09_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).

#### `dense`

- Intent: Densify registration and submaps while revisiting keyframes and loop closure more aggressively.
- CLI args: `--vgicp-slam-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods vgicp_slam --summary-json experiments/results/runs/vgicp_slam_mcd_tuhh_night_09_matrix/dense/summary.json --vgicp-slam-dense-profile`
- Summary: `experiments/results/runs/vgicp_slam_mcd_tuhh_night_09_matrix/dense/summary.json`
- Log: `experiments/results/runs/vgicp_slam_mcd_tuhh_night_09_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure (no GT seed; anchor matches first GT pose).


## Voxel-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `voxel_gicp_profile_tradeoff_kitti_raw_0009_full`
- **Question**: Which Voxel-GICP profile on KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `voxel_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/voxel_gicp_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 1.102 | 18.8 | 37.6 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.253 | 52.7 | 49.5 | 4.65 | 4.75 | Keep as reference variant |
| Dense recent map | accuracy-oriented | 0.640 | 110.1 | 100.0 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `dense_recent_map` is the fastest observed variant at 110.1 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.640 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Coarser voxels and lighter iterations.
- CLI args: `--voxel-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json --voxel-gicp-fast-profile`
- Summary: `experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Finer voxels and deeper alignment.
- CLI args: `--voxel-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/dense_recent_map/summary.json --voxel-gicp-dense-profile`
- Summary: `experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_kitti_raw_0009_full_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Voxel-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence

- **Problem ID**: `voxel_gicp_profile_tradeoff_mcd_kth_day_06`
- **Question**: Which Voxel-GICP profile should stay as the current default on the MCD public dataset (KTH day-06)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `voxel_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/voxel_gicp_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.926 | 18.2 | 57.3 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 1.263 | 42.2 | 53.6 | 4.65 | 4.75 | Keep as reference variant |
| Dense recent map | accuracy-oriented | 0.981 | 124.2 | 97.2 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `dense_recent_map` is the fastest observed variant at 124.2 FPS.
3. `balanced_local_map` is the most accurate observed variant at 0.926 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Coarser voxels and lighter iterations.
- CLI args: `--voxel-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/fast_recent_map/summary.json --voxel-gicp-fast-profile`
- Summary: `experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Finer voxels and deeper alignment.
- CLI args: `--voxel-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/dense_recent_map/summary.json --voxel-gicp-dense-profile`
- Summary: `experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_kth_day_06_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Voxel-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence

- **Problem ID**: `voxel_gicp_profile_tradeoff_mcd_ntu_day_02`
- **Question**: Which Voxel-GICP profile should stay as the current default on the MCD public dataset (NTU day-02)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `voxel_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/voxel_gicp_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.598 | 44.4 | 29.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.518 | 64.9 | 39.3 | 4.65 | 4.75 | Keep as reference variant |
| Dense recent map | accuracy-oriented | 0.121 | 117.2 | 100.0 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `dense_recent_map` is the fastest observed variant at 117.2 FPS.
3. `dense_recent_map` is the most accurate observed variant at 0.121 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Coarser voxels and lighter iterations.
- CLI args: `--voxel-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json --voxel-gicp-fast-profile`
- Summary: `experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Finer voxels and deeper alignment.
- CLI args: `--voxel-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/dense_recent_map/summary.json --voxel-gicp-dense-profile`
- Summary: `experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_ntu_day_02_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## Voxel-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence

- **Problem ID**: `voxel_gicp_profile_tradeoff_mcd_tuhh_night_09`
- **Question**: Which Voxel-GICP profile should stay as the current default on the MCD public dataset (TUHH night-09)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `voxel_gicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/voxel_gicp_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Balanced local map | balanced | 0.381 | 26.2 | 48.8 | 5.00 | 5.00 | Keep as reference variant |
| Fast recent map | throughput-oriented | 0.286 | 57.5 | 74.7 | 4.65 | 4.75 | Keep as active challenger |
| Dense recent map | accuracy-oriented | 0.478 | 116.4 | 79.9 | 4.65 | 4.75 | Adopt as current default |

### Observations

1. `dense_recent_map` is the current default for this problem.
2. `dense_recent_map` is the fastest observed variant at 116.4 FPS.
3. `fast_recent_map` is the most accurate observed variant at 0.286 m ATE.

### Variant Notes

#### `balanced_local_map`

- Intent: Repository default hyperparameters.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Summary: `experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/balanced_local_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/balanced_local_map/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast_recent_map`

- Intent: Coarser voxels and lighter iterations.
- CLI args: `--voxel-gicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json --voxel-gicp-fast-profile`
- Summary: `experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/fast_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/fast_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense_recent_map`

- Intent: Finer voxels and deeper alignment.
- CLI args: `--voxel-gicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods voxel_gicp --summary-json experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/dense_recent_map/summary.json --voxel-gicp-dense-profile`
- Summary: `experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/dense_recent_map/summary.json`
- Log: `experiments/results/runs/voxel_gicp_mcd_tuhh_night_09_matrix/dense_recent_map/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.


## X-ICP on KITTI Raw drive 0009 full sequence (443 frames, urban)

- **Problem ID**: `xicp_kitti_raw_0009_full`
- **Question**: How does X-ICP perform on the KITTI Raw drive 0009 full sequence (443 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `xicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/xicp_kitti_raw_0009_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (GT seed) | balanced | 0.169 | 56.8 | 69.0 | 5.00 | 5.00 | Keep as reference variant |
| Fast (GT seed) | throughput-oriented | 0.501 | 92.9 | 62.9 | 4.65 | 4.75 | Keep as reference variant |
| Dense (GT seed) | accuracy-oriented | 0.130 | 59.9 | 82.2 | 4.65 | 4.75 | Adopt as current default |
| No GT seed | accuracy-oriented | 177.449 | 64.8 | 34.9 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `dense` is the current default for this problem.
2. `fast` is the fastest observed variant at 92.9 FPS.
3. `dense` is the most accurate observed variant at 0.130 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default with ground-truth seed initialization.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0009_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/xicp_kitti_raw_0009_full_matrix/default/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0009_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast`

- Intent: Coarser voxel map and smaller working set while keeping the GT-seeded initialization path.
- CLI args: `--xicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0009_full_matrix/fast/summary.json --xicp-fast-profile`
- Summary: `experiments/results/runs/xicp_kitti_raw_0009_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0009_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense`

- Intent: Finer voxel map and larger correspondence budget while keeping the GT-seeded initialization path.
- CLI args: `--xicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0009_full_matrix/dense/summary.json --xicp-dense-profile`
- Summary: `experiments/results/runs/xicp_kitti_raw_0009_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0009_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `no_gt_seed`

- Intent: Ablation without ground-truth seed to test convergence from identity.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_full experiments/reference_data/kitti_raw_0009_full_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0009_full_matrix/no_gt_seed/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/xicp_kitti_raw_0009_full_matrix/no_gt_seed/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0009_full_matrix/no_gt_seed/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).


## X-ICP on KITTI Raw drive 0009 (200 frames, urban)

- **Problem ID**: `xicp_kitti_raw_0009`
- **Question**: How does X-ICP perform on the KITTI Raw drive 0009 (200 frames, urban)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0009_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0009_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `xicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/xicp_kitti_raw_0009_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (GT seed) | balanced | 0.158 | 58.5 | 72.7 | 5.00 | 5.00 | Keep as active challenger |
| Fast (GT seed) | throughput-oriented | 0.546 | 102.8 | 62.8 | 4.65 | 4.75 | Keep as reference variant |
| Dense (GT seed) | accuracy-oriented | 0.139 | 58.5 | 78.4 | 4.65 | 4.75 | Adopt as current default |
| No GT seed | accuracy-oriented | 116.407 | 65.1 | 31.7 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `dense` is the current default for this problem.
2. `fast` is the fastest observed variant at 102.8 FPS.
3. `dense` is the most accurate observed variant at 0.139 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default with ground-truth seed initialization.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0009_matrix/default/summary.json`
- Summary: `experiments/results/runs/xicp_kitti_raw_0009_matrix/default/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0009_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast`

- Intent: Coarser voxel map and smaller working set while keeping the GT-seeded initialization path.
- CLI args: `--xicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0009_matrix/fast/summary.json --xicp-fast-profile`
- Summary: `experiments/results/runs/xicp_kitti_raw_0009_matrix/fast/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0009_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense`

- Intent: Finer voxel map and larger correspondence budget while keeping the GT-seeded initialization path.
- CLI args: `--xicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0009_matrix/dense/summary.json --xicp-dense-profile`
- Summary: `experiments/results/runs/xicp_kitti_raw_0009_matrix/dense/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0009_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `no_gt_seed`

- Intent: Ablation without ground-truth seed to test convergence from identity.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0009_200 experiments/reference_data/kitti_raw_0009_200_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0009_matrix/no_gt_seed/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/xicp_kitti_raw_0009_matrix/no_gt_seed/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0009_matrix/no_gt_seed/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).


## X-ICP on KITTI Raw drive 0061 full sequence (703 frames, residential)

- **Problem ID**: `xicp_kitti_raw_0061_full`
- **Question**: How does X-ICP perform on the KITTI Raw drive 0061 full sequence (703 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_full`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_full_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `xicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/xicp_kitti_raw_0061_full_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (GT seed) | balanced | 0.138 | 60.4 | 75.2 | 5.00 | 5.00 | Keep as active challenger |
| Fast (GT seed) | throughput-oriented | 0.202 | 104.6 | 81.7 | 4.65 | 4.75 | Adopt as current default |
| Dense (GT seed) | accuracy-oriented | 0.128 | 54.1 | 75.8 | 4.65 | 4.75 | Keep as active challenger |
| No GT seed | accuracy-oriented | 281.232 | 66.1 | 31.6 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 104.6 FPS.
3. `dense` is the most accurate observed variant at 0.128 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default with ground-truth seed initialization.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0061_full_matrix/default/summary.json`
- Summary: `experiments/results/runs/xicp_kitti_raw_0061_full_matrix/default/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0061_full_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast`

- Intent: Coarser voxel map and smaller working set while keeping the GT-seeded initialization path.
- CLI args: `--xicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0061_full_matrix/fast/summary.json --xicp-fast-profile`
- Summary: `experiments/results/runs/xicp_kitti_raw_0061_full_matrix/fast/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0061_full_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense`

- Intent: Finer voxel map and larger correspondence budget while keeping the GT-seeded initialization path.
- CLI args: `--xicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0061_full_matrix/dense/summary.json --xicp-dense-profile`
- Summary: `experiments/results/runs/xicp_kitti_raw_0061_full_matrix/dense/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0061_full_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `no_gt_seed`

- Intent: Ablation without ground-truth seed to test convergence from identity.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_full experiments/reference_data/kitti_raw_0061_full_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0061_full_matrix/no_gt_seed/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/xicp_kitti_raw_0061_full_matrix/no_gt_seed/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0061_full_matrix/no_gt_seed/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).


## X-ICP on KITTI Raw drive 0061 (200 frames, residential)

- **Problem ID**: `xicp_kitti_raw_0061`
- **Question**: How does X-ICP perform on the KITTI Raw drive 0061 (200 frames, residential)?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/kitti_raw_0061_200`
- **Reference CSV**: `experiments/reference_data/kitti_raw_0061_200_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `xicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/xicp_kitti_raw_0061_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (GT seed) | balanced | 0.141 | 60.8 | 64.5 | 5.00 | 5.00 | Keep as reference variant |
| Fast (GT seed) | throughput-oriented | 0.171 | 102.0 | 78.7 | 4.65 | 4.75 | Adopt as current default |
| Dense (GT seed) | accuracy-oriented | 0.098 | 57.6 | 78.2 | 4.65 | 4.75 | Keep as active challenger |
| No GT seed | accuracy-oriented | 78.836 | 61.6 | 30.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 102.0 FPS.
3. `dense` is the most accurate observed variant at 0.098 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default with ground-truth seed initialization.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0061_matrix/default/summary.json`
- Summary: `experiments/results/runs/xicp_kitti_raw_0061_matrix/default/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0061_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast`

- Intent: Coarser voxel map and smaller working set while keeping the GT-seeded initialization path.
- CLI args: `--xicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0061_matrix/fast/summary.json --xicp-fast-profile`
- Summary: `experiments/results/runs/xicp_kitti_raw_0061_matrix/fast/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0061_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense`

- Intent: Finer voxel map and larger correspondence budget while keeping the GT-seeded initialization path.
- CLI args: `--xicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0061_matrix/dense/summary.json --xicp-dense-profile`
- Summary: `experiments/results/runs/xicp_kitti_raw_0061_matrix/dense/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0061_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `no_gt_seed`

- Intent: Ablation without ground-truth seed to test convergence from identity.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/kitti_raw_0061_200 experiments/reference_data/kitti_raw_0061_200_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_kitti_raw_0061_matrix/no_gt_seed/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/xicp_kitti_raw_0061_matrix/no_gt_seed/summary.json`
- Log: `experiments/results/runs/xicp_kitti_raw_0061_matrix/no_gt_seed/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).


## X-ICP on MCD KTH day-06 sequence

- **Problem ID**: `xicp_mcd_kth_day_06`
- **Question**: How does X-ICP perform on the MCD KTH day-06 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_kth_day_06_108`
- **Reference CSV**: `experiments/reference_data/mcd_kth_day_06_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `xicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/xicp_mcd_kth_day_06_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (GT seed) | balanced | 0.305 | 63.5 | 87.4 | 5.00 | 5.00 | Keep as active challenger |
| Fast (GT seed) | throughput-oriented | 0.401 | 84.8 | 88.0 | 4.65 | 4.75 | Adopt as current default |
| Dense (GT seed) | accuracy-oriented | 0.600 | 65.2 | 63.8 | 4.65 | 4.75 | Keep as reference variant |
| No GT seed | accuracy-oriented | 8.838 | 75.2 | 46.0 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `fast` is the current default for this problem.
2. `fast` is the fastest observed variant at 84.8 FPS.
3. `default` is the most accurate observed variant at 0.305 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default with ground-truth seed initialization.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_mcd_kth_day_06_matrix/default/summary.json`
- Summary: `experiments/results/runs/xicp_mcd_kth_day_06_matrix/default/summary.json`
- Log: `experiments/results/runs/xicp_mcd_kth_day_06_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast`

- Intent: Coarser voxel map and smaller working set while keeping the GT-seeded initialization path.
- CLI args: `--xicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_mcd_kth_day_06_matrix/fast/summary.json --xicp-fast-profile`
- Summary: `experiments/results/runs/xicp_mcd_kth_day_06_matrix/fast/summary.json`
- Log: `experiments/results/runs/xicp_mcd_kth_day_06_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense`

- Intent: Finer voxel map and larger correspondence budget while keeping the GT-seeded initialization path.
- CLI args: `--xicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_mcd_kth_day_06_matrix/dense/summary.json --xicp-dense-profile`
- Summary: `experiments/results/runs/xicp_mcd_kth_day_06_matrix/dense/summary.json`
- Log: `experiments/results/runs/xicp_mcd_kth_day_06_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `no_gt_seed`

- Intent: Ablation without ground-truth seed to test convergence from identity.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_kth_day_06_108 experiments/reference_data/mcd_kth_day_06_108_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_mcd_kth_day_06_matrix/no_gt_seed/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/xicp_mcd_kth_day_06_matrix/no_gt_seed/summary.json`
- Log: `experiments/results/runs/xicp_mcd_kth_day_06_matrix/no_gt_seed/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).


## X-ICP on MCD NTU day-02 sequence

- **Problem ID**: `xicp_mcd_ntu_day_02`
- **Question**: How does X-ICP perform on the MCD NTU day-02 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_ntu_day_02_108`
- **Reference CSV**: `experiments/reference_data/mcd_ntu_day_02_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `xicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/xicp_mcd_ntu_day_02_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (GT seed) | balanced | 0.252 | 68.4 | 60.7 | 5.00 | 5.00 | Keep as reference variant |
| Fast (GT seed) | throughput-oriented | 0.277 | 81.8 | 67.2 | 4.65 | 4.75 | Keep as reference variant |
| Dense (GT seed) | accuracy-oriented | 0.095 | 69.0 | 92.2 | 4.65 | 4.75 | Adopt as current default |
| No GT seed | accuracy-oriented | 2.943 | 66.5 | 42.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `dense` is the current default for this problem.
2. `fast` is the fastest observed variant at 81.8 FPS.
3. `dense` is the most accurate observed variant at 0.095 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default with ground-truth seed initialization.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_mcd_ntu_day_02_matrix/default/summary.json`
- Summary: `experiments/results/runs/xicp_mcd_ntu_day_02_matrix/default/summary.json`
- Log: `experiments/results/runs/xicp_mcd_ntu_day_02_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast`

- Intent: Coarser voxel map and smaller working set while keeping the GT-seeded initialization path.
- CLI args: `--xicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_mcd_ntu_day_02_matrix/fast/summary.json --xicp-fast-profile`
- Summary: `experiments/results/runs/xicp_mcd_ntu_day_02_matrix/fast/summary.json`
- Log: `experiments/results/runs/xicp_mcd_ntu_day_02_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense`

- Intent: Finer voxel map and larger correspondence budget while keeping the GT-seeded initialization path.
- CLI args: `--xicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_mcd_ntu_day_02_matrix/dense/summary.json --xicp-dense-profile`
- Summary: `experiments/results/runs/xicp_mcd_ntu_day_02_matrix/dense/summary.json`
- Log: `experiments/results/runs/xicp_mcd_ntu_day_02_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `no_gt_seed`

- Intent: Ablation without ground-truth seed to test convergence from identity.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_ntu_day_02_108 experiments/reference_data/mcd_ntu_day_02_108_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_mcd_ntu_day_02_matrix/no_gt_seed/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/xicp_mcd_ntu_day_02_matrix/no_gt_seed/summary.json`
- Log: `experiments/results/runs/xicp_mcd_ntu_day_02_matrix/no_gt_seed/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).


## X-ICP on MCD TUHH night-09 sequence

- **Problem ID**: `xicp_mcd_tuhh_night_09`
- **Question**: How does X-ICP perform on the MCD TUHH night-09 sequence?
- **Status**: `ready`
- **Dataset PCD directory**: `dogfooding_results/mcd_tuhh_night_09_108`
- **Reference CSV**: `experiments/reference_data/mcd_tuhh_night_09_108_gt.csv`
- **Stable binary**: `build/evaluation/pcd_dogfooding`
- **Shared method selector**: `xicp`
- **Shared metrics**: ate_m, fps, readability_score, extensibility_score
- **Aggregate result**: `experiments/results/xicp_mcd_tuhh_night_09_matrix.json`

| Variant | Style | ATE [m] | FPS | Benchmark | Readability | Extensibility | Decision |
|---------|-------|---------|-----|-----------|-------------|---------------|----------|
| Default (GT seed) | balanced | 0.122 | 76.6 | 71.1 | 5.00 | 5.00 | Keep as reference variant |
| Fast (GT seed) | throughput-oriented | 0.130 | 100.4 | 80.9 | 4.65 | 4.75 | Keep as active challenger |
| Dense (GT seed) | accuracy-oriented | 0.081 | 72.0 | 85.8 | 4.65 | 4.75 | Adopt as current default |
| No GT seed | accuracy-oriented | 1.965 | 74.8 | 39.3 | 4.65 | 4.75 | Keep as reference variant |

### Observations

1. `dense` is the current default for this problem.
2. `fast` is the fastest observed variant at 100.4 FPS.
3. `dense` is the most accurate observed variant at 0.081 m ATE.

### Variant Notes

#### `default`

- Intent: Repository default with ground-truth seed initialization.
- CLI args: `(default flags only)`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_mcd_tuhh_night_09_matrix/default/summary.json`
- Summary: `experiments/results/runs/xicp_mcd_tuhh_night_09_matrix/default/summary.json`
- Log: `experiments/results/runs/xicp_mcd_tuhh_night_09_matrix/default/run.log`
- Readability proxy: 5.00 / 5.00. Uses the default CLI surface only.
- Extensibility proxy: 5.00 / 5.00. No extra profile knobs beyond the stable core contract.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `fast`

- Intent: Coarser voxel map and smaller working set while keeping the GT-seeded initialization path.
- CLI args: `--xicp-fast-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_mcd_tuhh_night_09_matrix/fast/summary.json --xicp-fast-profile`
- Summary: `experiments/results/runs/xicp_mcd_tuhh_night_09_matrix/fast/summary.json`
- Log: `experiments/results/runs/xicp_mcd_tuhh_night_09_matrix/fast/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `dense`

- Intent: Finer voxel map and larger correspondence budget while keeping the GT-seeded initialization path.
- CLI args: `--xicp-dense-profile`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_mcd_tuhh_night_09_matrix/dense/summary.json --xicp-dense-profile`
- Summary: `experiments/results/runs/xicp_mcd_tuhh_night_09_matrix/dense/summary.json`
- Log: `experiments/results/runs/xicp_mcd_tuhh_night_09_matrix/dense/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses GT-seeded scan-to-map initialization with weak-update fallback in this dogfooding tool.

#### `no_gt_seed`

- Intent: Ablation without ground-truth seed to test convergence from identity.
- CLI args: `--no-gt-seed`
- Command: `build/evaluation/pcd_dogfooding dogfooding_results/mcd_tuhh_night_09_108 experiments/reference_data/mcd_tuhh_night_09_108_gt.csv --methods xicp --summary-json experiments/results/runs/xicp_mcd_tuhh_night_09_matrix/no_gt_seed/summary.json --no-gt-seed`
- Summary: `experiments/results/runs/xicp_mcd_tuhh_night_09_matrix/no_gt_seed/summary.json`
- Log: `experiments/results/runs/xicp_mcd_tuhh_night_09_matrix/no_gt_seed/run.log`
- Readability proxy: 4.65 / 5.00. Adds only boolean toggles on top of the stable CLI.
- Extensibility proxy: 4.75 / 5.00. Still stays inside the stable CLI, but expands the toggle surface.
- Method note: Uses odometry-chain scan-to-map initialization (no GT seed).

