# Paper Assets

_Generated at 2026-04-16T22:57:03+00:00 by `evaluation/scripts/export_paper_assets.py`._

This page is the paper-facing cut of the experiment state.
It keeps only comparable ready-problem outputs and highlights default variants first.

## Files

- Pareto plot: [`ready_defaults_pareto.png`](assets/paper/ready_defaults_pareto.png)
- Variant fronts: [`variant_fronts_by_selector.png`](assets/paper/variant_fronts_by_selector.png)
- Core methods plot: [`manuscript_core_methods.png`](assets/paper/manuscript_core_methods.png)
- CSV export: [`ready_defaults.csv`](assets/paper/ready_defaults.csv)
- Manuscript core CSV: [`manuscript_core_defaults.csv`](assets/paper/manuscript_core_defaults.csv)
- Default matrix (Table 3): [`default_variant_matrix.csv`](assets/paper/default_variant_matrix.csv), [`default_variant_matrix_long.csv`](assets/paper/default_variant_matrix_long.csv)
- Default instability figure: [`default_variant_instability.png`](assets/paper/default_variant_instability.png)
- Caption snippets: [`paper_captions.md`](paper_captions.md)

## Ready Defaults

| Method family | Dataset | Contract | Default variant | ATE [m] | FPS | Aggregate |
|---------------|---------|----------|-----------------|---------|-----|-----------|
| aloam | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 0.193 | 13.8 | `experiments/results/aloam_hdl_400_reference_matrix.json` |
| aloam | `hdl_400_open_ct_lio_120_b` | reference-based | `fast` | 0.166 | 19.0 | `experiments/results/aloam_hdl_400_reference_b_matrix.json` |
| aloam | `kitti_raw_0009_200` | reference-based | `fast` | 3.433 | 3.4 | `experiments/results/aloam_kitti_raw_0009_matrix.json` |
| aloam | `kitti_raw_0009_200` | reference-based | `fast` | 3.433 | 4.8 | `experiments/results/aloam_kitti_raw_0009_nogt_matrix.json` |
| aloam | `kitti_raw_0009_full` | reference-based | `fast` | 6.105 | 5.8 | `experiments/results/aloam_kitti_raw_0009_full_matrix.json` |
| aloam | `kitti_raw_0061_200` | reference-based | `fast` | 0.527 | 6.0 | `experiments/results/aloam_kitti_raw_0061_matrix.json` |
| aloam | `kitti_raw_0061_full` | reference-based | `fast` | 3.654 | 6.0 | `experiments/results/aloam_kitti_raw_0061_full_matrix.json` |
| aloam | `mcd_kth_day_06_108` | reference-based | `fast` | 6.100 | 6.7 | `experiments/results/aloam_mcd_kth_day_06_matrix.json` |
| aloam | `mcd_ntu_day_02_108` | reference-based | `dense` | 0.035 | 3.0 | `experiments/results/aloam_mcd_ntu_day_02_matrix.json` |
| aloam | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.374 | 6.5 | `experiments/results/aloam_mcd_tuhh_night_09_matrix.json` |
| balm2 | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 0.827 | 9.0 | `experiments/results/balm2_hdl_400_reference_matrix.json` |
| balm2 | `kitti_raw_0009_200` | reference-based | `fast` | 2.366 | 13.7 | `experiments/results/balm2_kitti_raw_0009_matrix.json` |
| balm2 | `kitti_raw_0009_200` | reference-based | `fast` | 2.366 | 12.6 | `experiments/results/balm2_kitti_raw_0009_nogt_matrix.json` |
| balm2 | `kitti_raw_0009_full` | reference-based | `fast` | 3.338 | 12.7 | `experiments/results/balm2_kitti_raw_0009_full_matrix.json` |
| balm2 | `kitti_raw_0061_200` | reference-based | `fast` | 2.926 | 13.2 | `experiments/results/balm2_kitti_raw_0061_matrix.json` |
| balm2 | `kitti_raw_0061_full` | reference-based | `fast` | 15.574 | 11.4 | `experiments/results/balm2_kitti_raw_0061_full_matrix.json` |
| balm2 | `mcd_kth_day_06_108` | reference-based | `fast` | 6.227 | 13.4 | `experiments/results/balm2_mcd_kth_day_06_matrix.json` |
| balm2 | `mcd_ntu_day_02_108` | reference-based | `fast` | 0.172 | 12.7 | `experiments/results/balm2_mcd_ntu_day_02_matrix.json` |
| balm2 | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.698 | 14.6 | `experiments/results/balm2_mcd_tuhh_night_09_matrix.json` |
| clins | `hdl_400_ros1_open_ct_lio_120_time_index` | reference-based | `dense` | 1.473 | 12.2 | `experiments/results/clins_hdl_400_public_ros1_synthtime_matrix.json` |
| ct_icp | `autoware_istanbul_open_108` | gt-backed | `fast_window` | 79.761 | 2.7 | `experiments/results/ct_icp_profile_matrix.json` |
| ct_icp | `autoware_istanbul_open_108_b` | gt-backed | `balanced_window` | 6.820 | 1.6 | `experiments/results/ct_icp_istanbul_window_b_matrix.json` |
| ct_icp | `autoware_istanbul_open_108_c` | gt-backed | `balanced_window` | 7.539 | 1.3 | `experiments/results/ct_icp_istanbul_window_c_matrix.json` |
| ct_icp | `hdl_400_open_ct_lio_120` | reference-based | `fast_window` | 2.582 | 72.9 | `experiments/results/ct_icp_hdl_400_reference_matrix.json` |
| ct_icp | `hdl_400_open_ct_lio_120_b` | reference-based | `fast_window` | 1.211 | 2.4 | `experiments/results/ct_icp_hdl_400_reference_b_matrix.json` |
| ct_icp | `hdl_400_ros1_open_ct_lio_120_time_index` | reference-based | `dense_window` | 1.254 | 16.6 | `experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json` |
| ct_icp | `kitti_raw_0009_200` | reference-based | `balanced_window` | 1.659 | 34.4 | `experiments/results/ct_icp_kitti_raw_0009_matrix.json` |
| ct_icp | `kitti_raw_0009_200` | reference-based | `balanced_window` | 1.659 | 44.5 | `experiments/results/ct_icp_kitti_raw_0009_nogt_matrix.json` |
| ct_icp | `kitti_raw_0009_full` | reference-based | `balanced_window` | 4.673 | 40.6 | `experiments/results/ct_icp_kitti_raw_0009_full_matrix.json` |
| ct_icp | `kitti_raw_0061_200` | reference-based | `fast_window` | 1.475 | 56.9 | `experiments/results/ct_icp_kitti_raw_0061_matrix.json` |
| ct_icp | `kitti_raw_0061_full` | reference-based | `fast_window` | 6.972 | 37.6 | `experiments/results/ct_icp_kitti_raw_0061_full_matrix.json` |
| ct_icp | `mcd_kth_day_06_108` | reference-based | `fast_window` | 6.525 | 59.8 | `experiments/results/ct_icp_mcd_kth_day_06_matrix.json` |
| ct_icp | `mcd_ntu_day_02_108` | reference-based | `dense_window` | 0.325 | 22.0 | `experiments/results/ct_icp_mcd_ntu_day_02_matrix.json` |
| ct_icp | `mcd_tuhh_night_09_108` | reference-based | `fast_window` | 3.553 | 71.6 | `experiments/results/ct_icp_mcd_tuhh_night_09_matrix.json` |
| ct_lio | `hdl_400_open_ct_lio_120` | reference-based | `seed_only_fast` | 0.488 | 17.5 | `experiments/results/ct_lio_reference_profile_matrix.json` |
| ct_lio | `hdl_400_ros1_open_ct_lio_120_time_index` | reference-based | `seed_only_fast` | 0.479 | 19.6 | `experiments/results/ct_lio_hdl_400_public_ros1_synthtime_matrix.json` |
| dlio | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 0.239 | 12.1 | `experiments/results/dlio_hdl_400_reference_matrix.json` |
| dlio | `kitti_raw_0009_200` | reference-based | `fast` | 2.362 | 5.3 | `experiments/results/dlio_kitti_raw_0009_matrix.json` |
| dlio | `kitti_raw_0009_200` | reference-based | `fast` | 2.362 | 7.0 | `experiments/results/dlio_kitti_raw_0009_nogt_matrix.json` |
| dlio | `kitti_raw_0009_full` | reference-based | `fast` | 5.026 | 7.3 | `experiments/results/dlio_kitti_raw_0009_full_matrix.json` |
| dlio | `kitti_raw_0061_200` | reference-based | `fast` | 0.882 | 8.5 | `experiments/results/dlio_kitti_raw_0061_matrix.json` |
| dlio | `kitti_raw_0061_full` | reference-based | `fast` | 7.370 | 10.7 | `experiments/results/dlio_kitti_raw_0061_full_matrix.json` |
| dlio | `mcd_kth_day_06_108` | reference-based | `fast` | 6.081 | 10.4 | `experiments/results/dlio_mcd_kth_day_06_matrix.json` |
| dlio | `mcd_ntu_day_02_108` | reference-based | `kitti_default` | 0.016 | 10.3 | `experiments/results/dlio_mcd_ntu_day_02_matrix.json` |
| dlio | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.344 | 13.0 | `experiments/results/dlio_mcd_tuhh_night_09_matrix.json` |
| dlo | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 0.118 | 15.0 | `experiments/results/dlo_hdl_400_reference_matrix.json` |
| dlo | `kitti_raw_0009_200` | reference-based | `fast` | 2.362 | 5.9 | `experiments/results/dlo_kitti_raw_0009_matrix.json` |
| dlo | `kitti_raw_0009_200` | reference-based | `fast` | 2.362 | 7.1 | `experiments/results/dlo_kitti_raw_0009_nogt_matrix.json` |
| dlo | `kitti_raw_0009_full` | reference-based | `fast` | 5.026 | 7.3 | `experiments/results/dlo_kitti_raw_0009_full_matrix.json` |
| dlo | `kitti_raw_0061_200` | reference-based | `fast` | 0.882 | 8.5 | `experiments/results/dlo_kitti_raw_0061_matrix.json` |
| dlo | `kitti_raw_0061_full` | reference-based | `fast` | 7.370 | 10.6 | `experiments/results/dlo_kitti_raw_0061_full_matrix.json` |
| dlo | `mcd_kth_day_06_108` | reference-based | `fast` | 6.081 | 10.7 | `experiments/results/dlo_mcd_kth_day_06_matrix.json` |
| dlo | `mcd_ntu_day_02_108` | reference-based | `kitti_default` | 0.016 | 10.2 | `experiments/results/dlo_mcd_ntu_day_02_matrix.json` |
| dlo | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.344 | 13.9 | `experiments/results/dlo_mcd_tuhh_night_09_matrix.json` |
| fast_lio2 | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 0.114 | 9.1 | `experiments/results/fast_lio2_hdl_400_reference_matrix.json` |
| fast_lio2 | `kitti_raw_0009_200` | reference-based | `fast` | 2.328 | 12.3 | `experiments/results/fast_lio2_kitti_raw_0009_matrix.json` |
| fast_lio2 | `kitti_raw_0009_200` | reference-based | `fast` | 2.328 | 12.7 | `experiments/results/fast_lio2_kitti_raw_0009_nogt_matrix.json` |
| fast_lio2 | `kitti_raw_0009_full` | reference-based | `fast` | 5.199 | 13.5 | `experiments/results/fast_lio2_kitti_raw_0009_full_matrix.json` |
| fast_lio2 | `kitti_raw_0061_200` | reference-based | `fast` | 0.634 | 13.6 | `experiments/results/fast_lio2_kitti_raw_0061_matrix.json` |
| fast_lio2 | `kitti_raw_0061_full` | reference-based | `fast` | 5.066 | 13.5 | `experiments/results/fast_lio2_kitti_raw_0061_full_matrix.json` |
| fast_lio2 | `mcd_kth_day_06_108` | reference-based | `fast` | 6.072 | 12.4 | `experiments/results/fast_lio2_mcd_kth_day_06_matrix.json` |
| fast_lio2 | `mcd_ntu_day_02_108` | reference-based | `fast` | 0.025 | 23.7 | `experiments/results/fast_lio2_mcd_ntu_day_02_matrix.json` |
| fast_lio2 | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.339 | 14.2 | `experiments/results/fast_lio2_mcd_tuhh_night_09_matrix.json` |
| fast_lio_slam | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 0.128 | 8.4 | `experiments/results/fast_lio_slam_hdl_400_reference_matrix.json` |
| fast_lio_slam | `kitti_raw_0009_200` | reference-based | `fast` | 2.382 | 8.8 | `experiments/results/fast_lio_slam_kitti_raw_0009_matrix.json` |
| fast_lio_slam | `kitti_raw_0009_200` | reference-based | `fast` | 2.382 | 11.3 | `experiments/results/fast_lio_slam_kitti_raw_0009_nogt_matrix.json` |
| fast_lio_slam | `kitti_raw_0009_full` | reference-based | `fast` | 5.289 | 11.3 | `experiments/results/fast_lio_slam_kitti_raw_0009_full_matrix.json` |
| fast_lio_slam | `kitti_raw_0061_200` | reference-based | `fast` | 0.660 | 11.2 | `experiments/results/fast_lio_slam_kitti_raw_0061_matrix.json` |
| fast_lio_slam | `kitti_raw_0061_full` | reference-based | `fast` | 4.945 | 9.1 | `experiments/results/fast_lio_slam_kitti_raw_0061_full_matrix.json` |
| fast_lio_slam | `mcd_kth_day_06_108` | reference-based | `fast` | 6.075 | 9.4 | `experiments/results/fast_lio_slam_mcd_kth_day_06_matrix.json` |
| fast_lio_slam | `mcd_ntu_day_02_108` | reference-based | `fast` | 0.028 | 20.4 | `experiments/results/fast_lio_slam_mcd_ntu_day_02_matrix.json` |
| fast_lio_slam | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.332 | 12.8 | `experiments/results/fast_lio_slam_mcd_tuhh_night_09_matrix.json` |
| floam | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 0.411 | 64.2 | `experiments/results/floam_hdl_400_reference_matrix.json` |
| floam | `kitti_raw_0009_200` | reference-based | `fast` | 3.486 | 24.6 | `experiments/results/floam_kitti_raw_0009_matrix.json` |
| floam | `kitti_raw_0009_200` | reference-based | `fast` | 3.486 | 28.0 | `experiments/results/floam_kitti_raw_0009_nogt_matrix.json` |
| floam | `kitti_raw_0009_full` | reference-based | `fast` | 5.452 | 28.6 | `experiments/results/floam_kitti_raw_0009_full_matrix.json` |
| floam | `kitti_raw_0061_200` | reference-based | `fast` | 0.756 | 25.9 | `experiments/results/floam_kitti_raw_0061_matrix.json` |
| floam | `kitti_raw_0061_full` | reference-based | `fast` | 3.822 | 30.7 | `experiments/results/floam_kitti_raw_0061_full_matrix.json` |
| floam | `mcd_kth_day_06_108` | reference-based | `fast` | 6.005 | 31.1 | `experiments/results/floam_mcd_kth_day_06_matrix.json` |
| floam | `mcd_ntu_day_02_108` | reference-based | `fast` | 0.152 | 27.0 | `experiments/results/floam_mcd_ntu_day_02_matrix.json` |
| floam | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.345 | 27.6 | `experiments/results/floam_mcd_tuhh_night_09_matrix.json` |
| gicp | `autoware_istanbul_open_108` | gt-backed | `fast_recent_map` | 1.074 | 6.3 | `experiments/results/gicp_profile_matrix.json` |
| gicp | `autoware_istanbul_open_108_b` | gt-backed | `fast_recent_map` | 1.166 | 5.7 | `experiments/results/gicp_istanbul_window_b_matrix.json` |
| gicp | `autoware_istanbul_open_108_c` | gt-backed | `fast_recent_map` | 0.982 | 4.3 | `experiments/results/gicp_istanbul_window_c_matrix.json` |
| gicp | `hdl_400_open_ct_lio_120` | reference-based | `fast_recent_map` | 0.215 | 23.3 | `experiments/results/gicp_hdl_400_reference_matrix.json` |
| gicp | `hdl_400_open_ct_lio_120_b` | reference-based | `fast_recent_map` | 0.284 | 1.7 | `experiments/results/gicp_hdl_400_reference_b_matrix.json` |
| gicp | `kitti_raw_0009_200` | reference-based | `fast_recent_map` | 1.177 | 25.8 | `experiments/results/gicp_kitti_raw_0009_matrix.json` |
| gicp | `kitti_raw_0009_200` | reference-based | `dense_recent_map` | 1.510 | 10.9 | `experiments/results/gicp_kitti_raw_0009_nogt_matrix.json` |
| gicp | `kitti_raw_0009_full` | reference-based | `fast_recent_map` | 1.170 | 23.0 | `experiments/results/gicp_kitti_raw_0009_full_matrix.json` |
| gicp | `kitti_raw_0061_200` | reference-based | `fast_recent_map` | 0.959 | 25.7 | `experiments/results/gicp_kitti_raw_0061_matrix.json` |
| gicp | `kitti_raw_0061_full` | reference-based | `fast_recent_map` | 1.081 | 22.8 | `experiments/results/gicp_kitti_raw_0061_full_matrix.json` |
| gicp | `mcd_kth_day_06_108` | reference-based | `fast_recent_map` | 0.630 | 24.7 | `experiments/results/gicp_mcd_kth_day_06_matrix.json` |
| gicp | `mcd_ntu_day_02_108` | reference-based | `dense_recent_map` | 0.017 | 13.0 | `experiments/results/gicp_mcd_ntu_day_02_matrix.json` |
| gicp | `mcd_tuhh_night_09_108` | reference-based | `fast_recent_map` | 0.317 | 31.2 | `experiments/results/gicp_mcd_tuhh_night_09_matrix.json` |
| hdl_graph_slam | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 15.656 | 6.7 | `experiments/results/hdl_graph_slam_hdl_400_reference_matrix.json` |
| hdl_graph_slam | `kitti_raw_0009_200` | reference-based | `fast` | 2.878 | 15.4 | `experiments/results/hdl_graph_slam_kitti_raw_0009_matrix.json` |
| hdl_graph_slam | `kitti_raw_0009_200` | reference-based | `default` | 122.141 | 1.9 | `experiments/results/hdl_graph_slam_kitti_raw_0009_nogt_matrix.json` |
| hdl_graph_slam | `kitti_raw_0009_full` | reference-based | `default` | 185.826 | 0.2 | `experiments/results/hdl_graph_slam_kitti_raw_0009_full_matrix.json` |
| hdl_graph_slam | `kitti_raw_0061_200` | reference-based | `fast` | 6.421 | 19.2 | `experiments/results/hdl_graph_slam_kitti_raw_0061_matrix.json` |
| hdl_graph_slam | `mcd_kth_day_06_108` | reference-based | `fast` | 9.241 | 13.9 | `experiments/results/hdl_graph_slam_mcd_kth_day_06_matrix.json` |
| hdl_graph_slam | `mcd_ntu_day_02_108` | reference-based | `dense` | 0.180 | 6.5 | `experiments/results/hdl_graph_slam_mcd_ntu_day_02_matrix.json` |
| hdl_graph_slam | `mcd_tuhh_night_09_108` | reference-based | `dense` | 1.373 | 4.2 | `experiments/results/hdl_graph_slam_mcd_tuhh_night_09_matrix.json` |
| isc_loam | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 0.162 | 37.0 | `experiments/results/isc_loam_hdl_400_reference_matrix.json` |
| isc_loam | `kitti_raw_0009_200` | reference-based | `fast` | 2.321 | 35.6 | `experiments/results/isc_loam_kitti_raw_0009_matrix.json` |
| isc_loam | `kitti_raw_0009_200` | reference-based | `fast` | 2.321 | 37.6 | `experiments/results/isc_loam_kitti_raw_0009_nogt_matrix.json` |
| isc_loam | `kitti_raw_0009_full` | reference-based | `fast` | 4.323 | 30.5 | `experiments/results/isc_loam_kitti_raw_0009_full_matrix.json` |
| isc_loam | `kitti_raw_0061_200` | reference-based | `dense` | 0.494 | 23.7 | `experiments/results/isc_loam_kitti_raw_0061_matrix.json` |
| isc_loam | `kitti_raw_0061_full` | reference-based | `fast` | 5.439 | 33.5 | `experiments/results/isc_loam_kitti_raw_0061_full_matrix.json` |
| isc_loam | `mcd_kth_day_06_108` | reference-based | `fast` | 6.094 | 48.6 | `experiments/results/isc_loam_mcd_kth_day_06_matrix.json` |
| isc_loam | `mcd_ntu_day_02_108` | reference-based | `fast` | 0.065 | 50.2 | `experiments/results/isc_loam_mcd_ntu_day_02_matrix.json` |
| isc_loam | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.357 | 53.2 | `experiments/results/isc_loam_mcd_tuhh_night_09_matrix.json` |
| kiss_icp | `autoware_istanbul_open_108` | gt-backed | `fast_recent_map` | 182.960 | 4.0 | `experiments/results/kiss_icp_profile_matrix.json` |
| kiss_icp | `autoware_istanbul_open_108_b` | gt-backed | `dense_local_map` | 144.086 | 3.6 | `experiments/results/kiss_icp_istanbul_window_b_matrix.json` |
| kiss_icp | `autoware_istanbul_open_108_c` | gt-backed | `fast_recent_map` | 131.692 | 3.7 | `experiments/results/kiss_icp_istanbul_window_c_matrix.json` |
| kiss_icp | `hdl_400_open_ct_lio_120` | reference-based | `fast_recent_map` | 1.281 | 11.3 | `experiments/results/kiss_icp_hdl_400_reference_matrix.json` |
| kiss_icp | `hdl_400_open_ct_lio_120_b` | reference-based | `fast_recent_map` | 0.218 | 0.4 | `experiments/results/kiss_icp_hdl_400_reference_b_matrix.json` |
| kiss_icp | `kitti_raw_0009_200` | reference-based | `fast_recent_map` | 2.642 | 24.4 | `experiments/results/kiss_icp_kitti_raw_0009_matrix.json` |
| kiss_icp | `kitti_raw_0009_200` | reference-based | `fast_recent_map` | 2.642 | 28.2 | `experiments/results/kiss_icp_kitti_raw_0009_nogt_matrix.json` |
| kiss_icp | `kitti_raw_0009_full` | reference-based | `fast_recent_map` | 5.839 | 21.9 | `experiments/results/kiss_icp_kitti_raw_0009_full_matrix.json` |
| kiss_icp | `kitti_raw_0061_200` | reference-based | `fast_recent_map` | 0.679 | 28.3 | `experiments/results/kiss_icp_kitti_raw_0061_matrix.json` |
| kiss_icp | `kitti_raw_0061_full` | reference-based | `fast_recent_map` | 4.623 | 11.2 | `experiments/results/kiss_icp_kitti_raw_0061_full_matrix.json` |
| kiss_icp | `mcd_kth_day_06_108` | reference-based | `fast_recent_map` | 5.568 | 11.3 | `experiments/results/kiss_icp_mcd_kth_day_06_matrix.json` |
| kiss_icp | `mcd_ntu_day_02_108` | reference-based | `fast_recent_map` | 0.026 | 66.7 | `experiments/results/kiss_icp_mcd_ntu_day_02_matrix.json` |
| kiss_icp | `mcd_tuhh_night_09_108` | reference-based | `fast_recent_map` | 1.303 | 24.1 | `experiments/results/kiss_icp_mcd_tuhh_night_09_matrix.json` |
| lego_loam | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 0.226 | 21.8 | `experiments/results/lego_loam_hdl_400_reference_matrix.json` |
| lego_loam | `kitti_raw_0009_200` | reference-based | `fast` | 3.216 | 8.9 | `experiments/results/lego_loam_kitti_raw_0009_matrix.json` |
| lego_loam | `kitti_raw_0009_200` | reference-based | `fast` | 3.216 | 9.9 | `experiments/results/lego_loam_kitti_raw_0009_nogt_matrix.json` |
| lego_loam | `kitti_raw_0009_full` | reference-based | `fast` | 6.498 | 9.5 | `experiments/results/lego_loam_kitti_raw_0009_full_matrix.json` |
| lego_loam | `kitti_raw_0061_200` | reference-based | `fast` | 0.481 | 9.7 | `experiments/results/lego_loam_kitti_raw_0061_matrix.json` |
| lego_loam | `kitti_raw_0061_full` | reference-based | `fast` | 5.248 | 11.1 | `experiments/results/lego_loam_kitti_raw_0061_full_matrix.json` |
| lego_loam | `mcd_kth_day_06_108` | reference-based | `fast` | 6.099 | 9.9 | `experiments/results/lego_loam_mcd_kth_day_06_matrix.json` |
| lego_loam | `mcd_ntu_day_02_108` | reference-based | `fast` | 0.079 | 8.4 | `experiments/results/lego_loam_mcd_ntu_day_02_matrix.json` |
| lego_loam | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.401 | 10.1 | `experiments/results/lego_loam_mcd_tuhh_night_09_matrix.json` |
| lins | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 29.745 | 71.9 | `experiments/results/lins_hdl_400_reference_matrix.json` |
| lins | `kitti_raw_0009_200` | reference-based | `fast` | 120.032 | 120.7 | `experiments/results/lins_kitti_raw_0009_matrix.json` |
| lins | `kitti_raw_0009_200` | reference-based | `fast` | 120.032 | 105.0 | `experiments/results/lins_kitti_raw_0009_nogt_matrix.json` |
| lins | `kitti_raw_0009_full` | reference-based | `fast` | 183.686 | 123.3 | `experiments/results/lins_kitti_raw_0009_full_matrix.json` |
| lins | `kitti_raw_0061_200` | reference-based | `fast` | 82.393 | 115.2 | `experiments/results/lins_kitti_raw_0061_matrix.json` |
| lins | `kitti_raw_0061_full` | reference-based | `fast` | 291.877 | 104.7 | `experiments/results/lins_kitti_raw_0061_full_matrix.json` |
| lins | `mcd_kth_day_06_108` | reference-based | `fast` | 7.120 | 166.1 | `experiments/results/lins_mcd_kth_day_06_matrix.json` |
| lins | `mcd_ntu_day_02_108` | reference-based | `dense` | 0.111 | 34.9 | `experiments/results/lins_mcd_ntu_day_02_matrix.json` |
| lins | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.147 | 173.4 | `experiments/results/lins_mcd_tuhh_night_09_matrix.json` |
| lio_sam | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 0.202 | 18.1 | `experiments/results/lio_sam_hdl_400_reference_matrix.json` |
| lio_sam | `kitti_raw_0009_200` | reference-based | `fast` | 2.649 | 24.9 | `experiments/results/lio_sam_kitti_raw_0009_matrix.json` |
| lio_sam | `kitti_raw_0009_200` | reference-based | `fast` | 2.649 | 23.1 | `experiments/results/lio_sam_kitti_raw_0009_nogt_matrix.json` |
| lio_sam | `kitti_raw_0009_full` | reference-based | `fast` | 5.296 | 20.2 | `experiments/results/lio_sam_kitti_raw_0009_full_matrix.json` |
| lio_sam | `kitti_raw_0061_200` | reference-based | `fast` | 0.704 | 27.1 | `experiments/results/lio_sam_kitti_raw_0061_matrix.json` |
| lio_sam | `kitti_raw_0061_full` | reference-based | `fast` | 6.067 | 21.2 | `experiments/results/lio_sam_kitti_raw_0061_full_matrix.json` |
| lio_sam | `mcd_kth_day_06_108` | reference-based | `fast` | 6.074 | 29.7 | `experiments/results/lio_sam_mcd_kth_day_06_matrix.json` |
| lio_sam | `mcd_ntu_day_02_108` | reference-based | `fast` | 0.073 | 31.7 | `experiments/results/lio_sam_mcd_ntu_day_02_matrix.json` |
| lio_sam | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.314 | 30.0 | `experiments/results/lio_sam_mcd_tuhh_night_09_matrix.json` |
| litamin2 | `autoware_istanbul_open_108` | gt-backed | `fast_icp_only_half_threads` | 1.213 | 23.5 | `experiments/results/litamin2_profile_matrix.json` |
| litamin2 | `autoware_istanbul_open_108_b` | gt-backed | `fast_icp_only_half_threads` | 1.222 | 20.9 | `experiments/results/litamin2_istanbul_window_b_matrix.json` |
| litamin2 | `autoware_istanbul_open_108_c` | gt-backed | `paper_icp_only_half_threads` | 0.741 | 17.2 | `experiments/results/litamin2_istanbul_window_c_matrix.json` |
| litamin2 | `hdl_400_open_ct_lio_120` | reference-based | `paper_cov_half_threads` | 0.129 | 94.2 | `experiments/results/litamin2_hdl_400_reference_matrix.json` |
| litamin2 | `hdl_400_open_ct_lio_120_b` | reference-based | `fast_icp_only_half_threads` | 0.168 | 5.2 | `experiments/results/litamin2_hdl_400_reference_b_matrix.json` |
| litamin2 | `kitti_raw_0009_200` | reference-based | `paper_icp_only_half_threads` | 1.397 | 43.7 | `experiments/results/litamin2_kitti_raw_0009_matrix.json` |
| litamin2 | `kitti_raw_0009_200` | reference-based | `paper_cov_half_threads` | 122.275 | 87.3 | `experiments/results/litamin2_kitti_raw_0009_nogt_matrix.json` |
| litamin2 | `kitti_raw_0009_full` | reference-based | `fast_icp_only_half_threads` | 1.145 | 48.8 | `experiments/results/litamin2_kitti_raw_0009_full_matrix.json` |
| litamin2 | `kitti_raw_0061_200` | reference-based | `fast_cov_half_threads` | 0.511 | 67.7 | `experiments/results/litamin2_kitti_raw_0061_matrix.json` |
| litamin2 | `kitti_raw_0061_full` | reference-based | `fast_icp_only_half_threads` | 0.944 | 58.1 | `experiments/results/litamin2_kitti_raw_0061_full_matrix.json` |
| litamin2 | `mcd_kth_day_06_108` | reference-based | `fast_cov_half_threads` | 0.401 | 64.2 | `experiments/results/litamin2_mcd_kth_day_06_matrix.json` |
| litamin2 | `mcd_ntu_day_02_108` | reference-based | `paper_icp_only_half_threads` | 0.045 | 105.6 | `experiments/results/litamin2_mcd_ntu_day_02_matrix.json` |
| litamin2 | `mcd_tuhh_night_09_108` | reference-based | `fast_cov_half_threads` | 0.194 | 106.5 | `experiments/results/litamin2_mcd_tuhh_night_09_matrix.json` |
| loam_livox | `hdl_400_open_ct_lio_120` | reference-based | `default` | 0.091 | 17.8 | `experiments/results/loam_livox_hdl_400_reference_matrix.json` |
| loam_livox | `kitti_raw_0009_200` | reference-based | `fast` | 98.811 | 43.8 | `experiments/results/loam_livox_kitti_raw_0009_matrix.json` |
| loam_livox | `kitti_raw_0009_200` | reference-based | `fast` | 98.811 | 40.2 | `experiments/results/loam_livox_kitti_raw_0009_nogt_matrix.json` |
| loam_livox | `kitti_raw_0009_full` | reference-based | `fast` | 136.097 | 33.1 | `experiments/results/loam_livox_kitti_raw_0009_full_matrix.json` |
| loam_livox | `kitti_raw_0061_200` | reference-based | `fast` | 78.138 | 50.0 | `experiments/results/loam_livox_kitti_raw_0061_matrix.json` |
| loam_livox | `kitti_raw_0061_full` | reference-based | `fast` | 277.148 | 36.6 | `experiments/results/loam_livox_kitti_raw_0061_full_matrix.json` |
| loam_livox | `mcd_kth_day_06_108` | reference-based | `fast` | 4.095 | 66.3 | `experiments/results/loam_livox_mcd_kth_day_06_matrix.json` |
| loam_livox | `mcd_ntu_day_02_108` | reference-based | `fast` | 0.137 | 69.0 | `experiments/results/loam_livox_mcd_ntu_day_02_matrix.json` |
| loam_livox | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.192 | 68.8 | `experiments/results/loam_livox_mcd_tuhh_night_09_matrix.json` |
| mulls | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 0.876 | 9.3 | `experiments/results/mulls_hdl_400_reference_matrix.json` |
| mulls | `kitti_raw_0009_200` | reference-based | `fast` | 2.651 | 3.1 | `experiments/results/mulls_kitti_raw_0009_matrix.json` |
| mulls | `kitti_raw_0009_200` | reference-based | `fast` | 2.651 | 3.4 | `experiments/results/mulls_kitti_raw_0009_nogt_matrix.json` |
| mulls | `kitti_raw_0009_full` | reference-based | `fast` | 4.610 | 3.3 | `experiments/results/mulls_kitti_raw_0009_full_matrix.json` |
| mulls | `kitti_raw_0061_200` | reference-based | `fast` | 0.490 | 3.3 | `experiments/results/mulls_kitti_raw_0061_matrix.json` |
| mulls | `kitti_raw_0061_full` | reference-based | `fast` | 11.390 | 3.3 | `experiments/results/mulls_kitti_raw_0061_full_matrix.json` |
| mulls | `mcd_kth_day_06_108` | reference-based | `fast` | 6.297 | 4.1 | `experiments/results/mulls_mcd_kth_day_06_matrix.json` |
| mulls | `mcd_ntu_day_02_108` | reference-based | `kitti_default` | 0.097 | 1.2 | `experiments/results/mulls_mcd_ntu_day_02_matrix.json` |
| mulls | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.206 | 3.8 | `experiments/results/mulls_mcd_tuhh_night_09_matrix.json` |
| ndt | `autoware_istanbul_open_108` | gt-backed | `fast_coarse_map` | 0.070 | 2.0 | `experiments/results/ndt_profile_matrix.json` |
| ndt | `autoware_istanbul_open_108_b` | gt-backed | `fast_coarse_map` | 0.007 | 2.1 | `experiments/results/ndt_istanbul_window_b_matrix.json` |
| ndt | `autoware_istanbul_open_108_c` | gt-backed | `fast_coarse_map` | 0.005 | 1.9 | `experiments/results/ndt_istanbul_window_c_matrix.json` |
| ndt | `hdl_400_open_ct_lio_120` | reference-based | `fast_coarse_map` | 0.052 | 32.2 | `experiments/results/ndt_hdl_400_reference_matrix.json` |
| ndt | `hdl_400_open_ct_lio_120_b` | reference-based | `fast_coarse_map` | 0.065 | 0.9 | `experiments/results/ndt_hdl_400_reference_b_matrix.json` |
| ndt | `kitti_raw_0009_200` | reference-based | `fast_coarse_map` | 0.374 | 36.1 | `experiments/results/ndt_kitti_raw_0009_matrix.json` |
| ndt | `kitti_raw_0009_200` | reference-based | `fast_coarse_map` | 122.547 | 25.0 | `experiments/results/ndt_kitti_raw_0009_nogt_matrix.json` |
| ndt | `kitti_raw_0009_full` | reference-based | `fast_coarse_map` | 0.307 | 30.9 | `experiments/results/ndt_kitti_raw_0009_full_matrix.json` |
| ndt | `kitti_raw_0061_200` | reference-based | `fast_coarse_map` | 0.319 | 41.2 | `experiments/results/ndt_kitti_raw_0061_matrix.json` |
| ndt | `kitti_raw_0061_full` | reference-based | `fast_coarse_map` | 0.247 | 23.8 | `experiments/results/ndt_kitti_raw_0061_full_matrix.json` |
| ndt | `mcd_kth_day_06_108` | reference-based | `fast_coarse_map` | 0.208 | 31.2 | `experiments/results/ndt_mcd_kth_day_06_matrix.json` |
| ndt | `mcd_ntu_day_02_108` | reference-based | `balanced_local_map` | 0.014 | 32.7 | `experiments/results/ndt_mcd_ntu_day_02_matrix.json` |
| ndt | `mcd_tuhh_night_09_108` | reference-based | `fast_coarse_map` | 0.070 | 40.8 | `experiments/results/ndt_mcd_tuhh_night_09_matrix.json` |
| point_lio | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 165.820 | 69.9 | `experiments/results/point_lio_hdl_400_reference_matrix.json` |
| point_lio | `kitti_raw_0009_200` | reference-based | `fast` | 119.890 | 117.4 | `experiments/results/point_lio_kitti_raw_0009_matrix.json` |
| point_lio | `kitti_raw_0009_200` | reference-based | `fast` | 119.890 | 95.6 | `experiments/results/point_lio_kitti_raw_0009_nogt_matrix.json` |
| point_lio | `kitti_raw_0009_full` | reference-based | `fast` | 183.384 | 113.1 | `experiments/results/point_lio_kitti_raw_0009_full_matrix.json` |
| point_lio | `kitti_raw_0061_200` | reference-based | `fast` | 82.450 | 92.8 | `experiments/results/point_lio_kitti_raw_0061_matrix.json` |
| point_lio | `kitti_raw_0061_full` | reference-based | `fast` | 292.011 | 89.5 | `experiments/results/point_lio_kitti_raw_0061_full_matrix.json` |
| point_lio | `mcd_kth_day_06_108` | reference-based | `fast` | 7.325 | 112.7 | `experiments/results/point_lio_mcd_kth_day_06_matrix.json` |
| point_lio | `mcd_ntu_day_02_108` | reference-based | `fast` | 0.083 | 77.3 | `experiments/results/point_lio_mcd_ntu_day_02_matrix.json` |
| point_lio | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.158 | 88.7 | `experiments/results/point_lio_mcd_tuhh_night_09_matrix.json` |
| small_gicp | `hdl_400_open_ct_lio_120` | reference-based | `fast_recent_map` | 0.251 | 110.3 | `experiments/results/small_gicp_hdl_400_reference_matrix.json` |
| small_gicp | `kitti_raw_0009_200` | reference-based | `fast_recent_map` | 0.471 | 83.3 | `experiments/results/small_gicp_kitti_raw_0009_matrix.json` |
| small_gicp | `kitti_raw_0009_200` | reference-based | `balanced_local_map` | 1.624 | 49.6 | `experiments/results/small_gicp_kitti_raw_0009_nogt_matrix.json` |
| small_gicp | `kitti_raw_0009_full` | reference-based | `fast_recent_map` | 0.437 | 92.4 | `experiments/results/small_gicp_kitti_raw_0009_full_matrix.json` |
| small_gicp | `kitti_raw_0061_200` | reference-based | `fast_recent_map` | 0.639 | 78.1 | `experiments/results/small_gicp_kitti_raw_0061_matrix.json` |
| small_gicp | `kitti_raw_0061_full` | reference-based | `fast_recent_map` | 0.959 | 82.2 | `experiments/results/small_gicp_kitti_raw_0061_full_matrix.json` |
| small_gicp | `mcd_kth_day_06_108` | reference-based | `fast_recent_map` | 0.806 | 107.9 | `experiments/results/small_gicp_mcd_kth_day_06_matrix.json` |
| small_gicp | `mcd_ntu_day_02_108` | reference-based | `dense_recent_map` | 0.031 | 56.8 | `experiments/results/small_gicp_mcd_ntu_day_02_matrix.json` |
| small_gicp | `mcd_tuhh_night_09_108` | reference-based | `fast_recent_map` | 0.466 | 107.2 | `experiments/results/small_gicp_mcd_tuhh_night_09_matrix.json` |
| suma | `hdl_400_open_ct_lio_120` | reference-based | `default` | 0.248 | 74.6 | `experiments/results/suma_hdl_400_reference_matrix.json` |
| suma | `kitti_raw_0009_200` | reference-based | `default` | 3.291 | 39.7 | `experiments/results/suma_kitti_raw_0009_matrix.json` |
| suma | `kitti_raw_0009_200` | reference-based | `default` | 3.291 | 82.3 | `experiments/results/suma_kitti_raw_0009_nogt_matrix.json` |
| suma | `kitti_raw_0009_full` | reference-based | `dense` | 5.487 | 25.0 | `experiments/results/suma_kitti_raw_0009_full_matrix.json` |
| suma | `kitti_raw_0061_200` | reference-based | `dense` | 1.496 | 33.5 | `experiments/results/suma_kitti_raw_0061_matrix.json` |
| suma | `kitti_raw_0061_full` | reference-based | `fast` | 32.429 | 110.9 | `experiments/results/suma_kitti_raw_0061_full_matrix.json` |
| suma | `mcd_kth_day_06_108` | reference-based | `fast` | 7.419 | 150.2 | `experiments/results/suma_mcd_kth_day_06_matrix.json` |
| suma | `mcd_ntu_day_02_108` | reference-based | `dense` | 0.036 | 33.9 | `experiments/results/suma_mcd_ntu_day_02_matrix.json` |
| suma | `mcd_tuhh_night_09_108` | reference-based | `default` | 1.414 | 59.1 | `experiments/results/suma_mcd_tuhh_night_09_matrix.json` |
| vgicp_slam | `hdl_400_open_ct_lio_120` | reference-based | `fast` | 0.145 | 16.8 | `experiments/results/vgicp_slam_hdl_400_reference_matrix.json` |
| vgicp_slam | `kitti_raw_0009_200` | reference-based | `fast` | 2.085 | 23.7 | `experiments/results/vgicp_slam_kitti_raw_0009_matrix.json` |
| vgicp_slam | `kitti_raw_0009_200` | reference-based | `fast` | 2.085 | 19.1 | `experiments/results/vgicp_slam_kitti_raw_0009_nogt_matrix.json` |
| vgicp_slam | `kitti_raw_0009_full` | reference-based | `fast` | 4.544 | 22.2 | `experiments/results/vgicp_slam_kitti_raw_0009_full_matrix.json` |
| vgicp_slam | `kitti_raw_0061_200` | reference-based | `fast` | 0.903 | 31.0 | `experiments/results/vgicp_slam_kitti_raw_0061_matrix.json` |
| vgicp_slam | `kitti_raw_0061_full` | reference-based | `fast` | 5.230 | 22.8 | `experiments/results/vgicp_slam_kitti_raw_0061_full_matrix.json` |
| vgicp_slam | `mcd_kth_day_06_108` | reference-based | `fast` | 6.096 | 20.8 | `experiments/results/vgicp_slam_mcd_kth_day_06_matrix.json` |
| vgicp_slam | `mcd_ntu_day_02_108` | reference-based | `fast` | 0.026 | 40.3 | `experiments/results/vgicp_slam_mcd_ntu_day_02_matrix.json` |
| vgicp_slam | `mcd_tuhh_night_09_108` | reference-based | `fast` | 1.322 | 21.8 | `experiments/results/vgicp_slam_mcd_tuhh_night_09_matrix.json` |
| voxel_gicp | `hdl_400_open_ct_lio_120` | reference-based | `dense_recent_map` | 0.268 | 141.1 | `experiments/results/voxel_gicp_hdl_400_reference_matrix.json` |
| voxel_gicp | `kitti_raw_0009_200` | reference-based | `dense_recent_map` | 0.670 | 93.9 | `experiments/results/voxel_gicp_kitti_raw_0009_matrix.json` |
| voxel_gicp | `kitti_raw_0009_200` | reference-based | `dense_recent_map` | 52.522 | 82.1 | `experiments/results/voxel_gicp_kitti_raw_0009_nogt_matrix.json` |
| voxel_gicp | `kitti_raw_0009_full` | reference-based | `dense_recent_map` | 0.640 | 110.1 | `experiments/results/voxel_gicp_kitti_raw_0009_full_matrix.json` |
| voxel_gicp | `kitti_raw_0061_200` | reference-based | `dense_recent_map` | 1.041 | 73.7 | `experiments/results/voxel_gicp_kitti_raw_0061_matrix.json` |
| voxel_gicp | `kitti_raw_0061_full` | reference-based | `dense_recent_map` | 1.062 | 75.4 | `experiments/results/voxel_gicp_kitti_raw_0061_full_matrix.json` |
| voxel_gicp | `mcd_kth_day_06_108` | reference-based | `dense_recent_map` | 0.981 | 124.2 | `experiments/results/voxel_gicp_mcd_kth_day_06_matrix.json` |
| voxel_gicp | `mcd_ntu_day_02_108` | reference-based | `dense_recent_map` | 0.121 | 117.2 | `experiments/results/voxel_gicp_mcd_ntu_day_02_matrix.json` |
| voxel_gicp | `mcd_tuhh_night_09_108` | reference-based | `dense_recent_map` | 0.478 | 116.4 | `experiments/results/voxel_gicp_mcd_tuhh_night_09_matrix.json` |
| xicp | `hdl_400_open_ct_lio_120` | reference-based | `dense` | 0.168 | 71.8 | `experiments/results/xicp_hdl_400_reference_matrix.json` |
| xicp | `kitti_raw_0009_200` | reference-based | `dense` | 0.139 | 58.5 | `experiments/results/xicp_kitti_raw_0009_matrix.json` |
| xicp | `kitti_raw_0009_200` | reference-based | `fast` | 121.338 | 81.2 | `experiments/results/xicp_kitti_raw_0009_nogt_matrix.json` |
| xicp | `kitti_raw_0009_full` | reference-based | `dense` | 0.130 | 59.9 | `experiments/results/xicp_kitti_raw_0009_full_matrix.json` |
| xicp | `kitti_raw_0061_200` | reference-based | `fast` | 0.171 | 102.0 | `experiments/results/xicp_kitti_raw_0061_matrix.json` |
| xicp | `kitti_raw_0061_full` | reference-based | `fast` | 0.202 | 104.6 | `experiments/results/xicp_kitti_raw_0061_full_matrix.json` |
| xicp | `mcd_kth_day_06_108` | reference-based | `fast` | 0.401 | 84.8 | `experiments/results/xicp_mcd_kth_day_06_matrix.json` |
| xicp | `mcd_ntu_day_02_108` | reference-based | `dense` | 0.095 | 69.0 | `experiments/results/xicp_mcd_ntu_day_02_matrix.json` |
| xicp | `mcd_tuhh_night_09_108` | reference-based | `dense` | 0.081 | 72.0 | `experiments/results/xicp_mcd_tuhh_night_09_matrix.json` |

## Notes

- `reference-based` means the trajectory CSV is a shared reference export rather than an independently curated GT file.
- `gt-backed` means the trajectory CSV is treated as the benchmark reference for that sequence.
- Blocked problems are intentionally excluded from the Pareto views.
