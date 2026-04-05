# Paper Assets

_Generated at 2026-04-06T06:33:08+00:00 by `evaluation/scripts/export_paper_assets.py`._

This page is the paper-facing cut of the experiment state.
It keeps only comparable ready-problem outputs and highlights default variants first.

## Files

- Pareto plot: [`ready_defaults_pareto.png`](assets/paper/ready_defaults_pareto.png)
- Variant fronts: [`variant_fronts_by_selector.png`](assets/paper/variant_fronts_by_selector.png)
- Core methods plot: [`manuscript_core_methods.png`](assets/paper/manuscript_core_methods.png)
- CSV export: [`ready_defaults.csv`](assets/paper/ready_defaults.csv)
- Manuscript core CSV: [`manuscript_core_defaults.csv`](assets/paper/manuscript_core_defaults.csv)
- Caption snippets: [`paper_captions.md`](paper_captions.md)

## Ready Defaults

| Method family | Dataset | Contract | Default variant | ATE [m] | FPS | Aggregate |
|---------------|---------|----------|-----------------|---------|-----|-----------|
| ct_icp | `autoware_istanbul_open_108` | gt-backed | `fast_window` | 79.761 | 2.7 | `experiments/results/ct_icp_profile_matrix.json` |
| ct_icp | `autoware_istanbul_open_108_b` | gt-backed | `balanced_window` | 6.820 | 1.6 | `experiments/results/ct_icp_istanbul_window_b_matrix.json` |
| ct_icp | `autoware_istanbul_open_108_c` | gt-backed | `balanced_window` | 7.539 | 1.3 | `experiments/results/ct_icp_istanbul_window_c_matrix.json` |
| ct_icp | `hdl_400_open_ct_lio_120` | reference-based | `fast_window` | 1.513 | 2.4 | `experiments/results/ct_icp_hdl_400_reference_matrix.json` |
| ct_icp | `hdl_400_open_ct_lio_120_b` | reference-based | `fast_window` | 1.211 | 2.4 | `experiments/results/ct_icp_hdl_400_reference_b_matrix.json` |
| ct_icp | `mcd_kth_day_06_108` | reference-based | `fast_window` | 6.525 | 59.8 | `experiments/results/ct_icp_mcd_kth_day_06_matrix.json` |
| ct_icp | `mcd_ntu_day_02_108` | reference-based | `dense_window` | 0.325 | 22.0 | `experiments/results/ct_icp_mcd_ntu_day_02_matrix.json` |
| ct_icp | `mcd_tuhh_night_09_108` | reference-based | `fast_window` | 3.553 | 71.6 | `experiments/results/ct_icp_mcd_tuhh_night_09_matrix.json` |
| ct_lio | `hdl_400_open_ct_lio_120` | reference-based | `seed_only_fast` | 0.412 | 0.4 | `experiments/results/ct_lio_reference_profile_matrix.json` |
| gicp | `autoware_istanbul_open_108` | gt-backed | `fast_recent_map` | 1.074 | 6.3 | `experiments/results/gicp_profile_matrix.json` |
| gicp | `autoware_istanbul_open_108_b` | gt-backed | `fast_recent_map` | 1.166 | 5.7 | `experiments/results/gicp_istanbul_window_b_matrix.json` |
| gicp | `autoware_istanbul_open_108_c` | gt-backed | `fast_recent_map` | 0.982 | 4.3 | `experiments/results/gicp_istanbul_window_c_matrix.json` |
| gicp | `hdl_400_open_ct_lio_120` | reference-based | `fast_recent_map` | 0.193 | 1.7 | `experiments/results/gicp_hdl_400_reference_matrix.json` |
| gicp | `hdl_400_open_ct_lio_120_b` | reference-based | `fast_recent_map` | 0.284 | 1.7 | `experiments/results/gicp_hdl_400_reference_b_matrix.json` |
| gicp | `mcd_kth_day_06_108` | reference-based | `fast_recent_map` | 0.630 | 24.7 | `experiments/results/gicp_mcd_kth_day_06_matrix.json` |
| gicp | `mcd_ntu_day_02_108` | reference-based | `dense_recent_map` | 0.017 | 13.0 | `experiments/results/gicp_mcd_ntu_day_02_matrix.json` |
| gicp | `mcd_tuhh_night_09_108` | reference-based | `fast_recent_map` | 0.317 | 31.2 | `experiments/results/gicp_mcd_tuhh_night_09_matrix.json` |
| kiss_icp | `autoware_istanbul_open_108` | gt-backed | `fast_recent_map` | 182.960 | 4.0 | `experiments/results/kiss_icp_profile_matrix.json` |
| kiss_icp | `autoware_istanbul_open_108_b` | gt-backed | `dense_local_map` | 144.086 | 3.6 | `experiments/results/kiss_icp_istanbul_window_b_matrix.json` |
| kiss_icp | `autoware_istanbul_open_108_c` | gt-backed | `fast_recent_map` | 131.692 | 3.7 | `experiments/results/kiss_icp_istanbul_window_c_matrix.json` |
| kiss_icp | `hdl_400_open_ct_lio_120` | reference-based | `fast_recent_map` | 1.646 | 0.5 | `experiments/results/kiss_icp_hdl_400_reference_matrix.json` |
| kiss_icp | `hdl_400_open_ct_lio_120_b` | reference-based | `fast_recent_map` | 0.218 | 0.4 | `experiments/results/kiss_icp_hdl_400_reference_b_matrix.json` |
| kiss_icp | `mcd_kth_day_06_108` | reference-based | `fast_recent_map` | 5.568 | 11.3 | `experiments/results/kiss_icp_mcd_kth_day_06_matrix.json` |
| kiss_icp | `mcd_ntu_day_02_108` | reference-based | `fast_recent_map` | 0.026 | 66.7 | `experiments/results/kiss_icp_mcd_ntu_day_02_matrix.json` |
| kiss_icp | `mcd_tuhh_night_09_108` | reference-based | `fast_recent_map` | 1.303 | 24.1 | `experiments/results/kiss_icp_mcd_tuhh_night_09_matrix.json` |
| litamin2 | `autoware_istanbul_open_108` | gt-backed | `fast_icp_only_half_threads` | 1.213 | 23.5 | `experiments/results/litamin2_profile_matrix.json` |
| litamin2 | `autoware_istanbul_open_108_b` | gt-backed | `fast_icp_only_half_threads` | 1.222 | 20.9 | `experiments/results/litamin2_istanbul_window_b_matrix.json` |
| litamin2 | `autoware_istanbul_open_108_c` | gt-backed | `paper_icp_only_half_threads` | 0.741 | 17.2 | `experiments/results/litamin2_istanbul_window_c_matrix.json` |
| litamin2 | `hdl_400_open_ct_lio_120` | reference-based | `paper_icp_only_half_threads` | 0.121 | 6.4 | `experiments/results/litamin2_hdl_400_reference_matrix.json` |
| litamin2 | `hdl_400_open_ct_lio_120_b` | reference-based | `fast_icp_only_half_threads` | 0.168 | 5.2 | `experiments/results/litamin2_hdl_400_reference_b_matrix.json` |
| litamin2 | `mcd_kth_day_06_108` | reference-based | `fast_cov_half_threads` | 0.401 | 64.2 | `experiments/results/litamin2_mcd_kth_day_06_matrix.json` |
| litamin2 | `mcd_ntu_day_02_108` | reference-based | `paper_icp_only_half_threads` | 0.045 | 105.6 | `experiments/results/litamin2_mcd_ntu_day_02_matrix.json` |
| litamin2 | `mcd_tuhh_night_09_108` | reference-based | `fast_cov_half_threads` | 0.194 | 106.5 | `experiments/results/litamin2_mcd_tuhh_night_09_matrix.json` |
| ndt | `autoware_istanbul_open_108` | gt-backed | `fast_coarse_map` | 0.070 | 2.0 | `experiments/results/ndt_profile_matrix.json` |
| ndt | `autoware_istanbul_open_108_b` | gt-backed | `fast_coarse_map` | 0.007 | 2.1 | `experiments/results/ndt_istanbul_window_b_matrix.json` |
| ndt | `autoware_istanbul_open_108_c` | gt-backed | `fast_coarse_map` | 0.005 | 1.9 | `experiments/results/ndt_istanbul_window_c_matrix.json` |
| ndt | `hdl_400_open_ct_lio_120` | reference-based | `fast_coarse_map` | 0.051 | 0.8 | `experiments/results/ndt_hdl_400_reference_matrix.json` |
| ndt | `hdl_400_open_ct_lio_120_b` | reference-based | `fast_coarse_map` | 0.065 | 0.9 | `experiments/results/ndt_hdl_400_reference_b_matrix.json` |
| ndt | `mcd_kth_day_06_108` | reference-based | `fast_coarse_map` | 0.208 | 31.2 | `experiments/results/ndt_mcd_kth_day_06_matrix.json` |
| ndt | `mcd_ntu_day_02_108` | reference-based | `balanced_local_map` | 0.014 | 32.7 | `experiments/results/ndt_mcd_ntu_day_02_matrix.json` |
| ndt | `mcd_tuhh_night_09_108` | reference-based | `fast_coarse_map` | 0.070 | 40.8 | `experiments/results/ndt_mcd_tuhh_night_09_matrix.json` |

## Notes

- `reference-based` means the trajectory CSV is a shared reference export rather than an independently curated GT file.
- `gt-backed` means the trajectory CSV is treated as the benchmark reference for that sequence.
- Blocked problems are intentionally excluded from the Pareto views.
