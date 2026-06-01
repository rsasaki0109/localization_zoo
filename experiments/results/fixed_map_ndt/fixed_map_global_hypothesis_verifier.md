# Fixed-Map Global Hypothesis Verifier

Audit source: `experiments/results/fixed_map_ndt/fixed_map_ndt_failure_audit.json`

## Thresholds

| Feature | Gate |
| --- | ---: |
| Scan Context mean distance | <= 0.120 |
| Scan Context hit rate | >= 0.300 |
| NDT candidate evals per frame | <= 1.000 |
| NDT accepted rate | >= 0.500 |

## Aggregate

| Rows | ScanContext | Accept for refinement | Pose publishable | Unsafe accepted | Score traps blocked | Near-basin retained |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 51 | 33 | 9 | 0 | 0 | 11/11 | 9/9 |

Decisions: accept_for_refinement:9, hold_for_sequence_prior:9, lab_only:15, out_of_scope:6, reject_score_trap:12

Outcomes: lab_only:15, near_basin_retained:9, out_of_scope:6, threshold_miss_held:3, unsafe_blocked:18

## Verifier Rows

| Sequence | Variant | GT label | Decision | Mean distance | Hit rate | Evals/frame | Accepted rate | ATE [m] | RPE [%] | Next gate |
| --- | --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| `02` | `seed_scan_context_stride_5` | `near_basin` | `accept_for_refinement` | 0.076 | 0.383 | n/a | 0.916 | 2.579 | 3.817 | `map_consistency_or_sequence_verifier` |
| `02` | `seed_scan_context_stride_5_topk_10` | `near_basin` | `accept_for_refinement` | 0.076 | 0.383 | 0.383 | 0.916 | 2.627 | 3.817 | `map_consistency_or_sequence_verifier` |
| `02` | `seed_scan_context_stride_5_topk_5` | `near_basin` | `accept_for_refinement` | 0.076 | 0.383 | 0.383 | 0.916 | 2.627 | 3.817 | `map_consistency_or_sequence_verifier` |
| `05` | `seed_scan_context_stride_5` | `near_basin` | `accept_for_refinement` | 0.096 | 0.542 | n/a | 0.963 | 1.730 | 3.926 | `map_consistency_or_sequence_verifier` |
| `05` | `seed_scan_context_stride_5_topk_10` | `near_basin` | `accept_for_refinement` | 0.096 | 0.542 | 0.542 | 0.963 | 1.730 | 3.926 | `map_consistency_or_sequence_verifier` |
| `05` | `seed_scan_context_stride_5_topk_5` | `near_basin` | `accept_for_refinement` | 0.096 | 0.542 | 0.542 | 0.963 | 1.730 | 3.926 | `map_consistency_or_sequence_verifier` |
| `08` | `seed_scan_context_stride_5` | `near_basin` | `accept_for_refinement` | 0.069 | 0.346 | n/a | 0.551 | 1.483 | 8.799 | `map_consistency_or_sequence_verifier` |
| `08` | `seed_scan_context_stride_5_topk_10` | `near_basin` | `accept_for_refinement` | 0.069 | 0.346 | 0.364 | 0.551 | 1.503 | 8.874 | `map_consistency_or_sequence_verifier` |
| `08` | `seed_scan_context_stride_5_topk_5` | `near_basin` | `accept_for_refinement` | 0.069 | 0.346 | 0.364 | 0.551 | 1.503 | 8.874 | `map_consistency_or_sequence_verifier` |
| `02` | `gt_x0p25` | `basin_probe` | `lab_only` | n/a | n/a | n/a | 0.991 | 0.163 | 0.231 | `non_oracle_runtime_initializer` |
| `02` | `gt_x0p5` | `basin_probe` | `lab_only` | n/a | n/a | n/a | 1.000 | 0.488 | 0.692 | `non_oracle_runtime_initializer` |
| `02` | `gt_yaw20` | `basin_probe` | `lab_only` | n/a | n/a | n/a | 1.000 | 0.049 | 26.908 | `non_oracle_runtime_initializer` |
| `02` | `seed_gt` | `oracle_seed` | `lab_only` | n/a | n/a | n/a | 0.981 | 0.003 | 0.012 | `non_oracle_runtime_initializer` |
| `02` | `seed_scan_context` | `dense_db_smoke` | `lab_only` | 0.000 | 1.000 | n/a | 0.981 | 0.003 | 0.012 | `non_oracle_runtime_initializer` |
| `05` | `gt_x0p25` | `basin_probe` | `lab_only` | n/a | n/a | n/a | 1.000 | 0.158 | 0.393 | `non_oracle_runtime_initializer` |
| `05` | `gt_x0p5` | `basin_probe` | `lab_only` | n/a | n/a | n/a | 1.000 | 0.483 | 0.115 | `non_oracle_runtime_initializer` |
| `05` | `gt_yaw20` | `basin_probe` | `lab_only` | n/a | n/a | n/a | 0.981 | 0.037 | 34.129 | `non_oracle_runtime_initializer` |
| `05` | `seed_gt` | `oracle_seed` | `lab_only` | n/a | n/a | n/a | 0.991 | 0.003 | 0.011 | `non_oracle_runtime_initializer` |
| `05` | `seed_scan_context` | `dense_db_smoke` | `lab_only` | -0.000 | 1.000 | n/a | 0.991 | 0.003 | 0.011 | `non_oracle_runtime_initializer` |
| `08` | `gt_x0p25` | `basin_probe` | `lab_only` | n/a | n/a | n/a | 0.729 | 0.231 | 0.957 | `non_oracle_runtime_initializer` |
| `08` | `gt_x0p5` | `basin_probe` | `lab_only` | n/a | n/a | n/a | 0.729 | 0.495 | 2.078 | `non_oracle_runtime_initializer` |
| `08` | `gt_yaw20` | `basin_probe` | `lab_only` | n/a | n/a | n/a | 0.579 | 0.145 | 28.307 | `non_oracle_runtime_initializer` |
| `08` | `seed_gt` | `oracle_seed` | `lab_only` | n/a | n/a | n/a | 0.925 | 0.021 | 0.057 | `non_oracle_runtime_initializer` |
| `08` | `seed_scan_context` | `dense_db_smoke` | `lab_only` | 0.000 | 1.000 | n/a | 0.925 | 0.021 | 0.057 | `non_oracle_runtime_initializer` |
| `02` | `seed_ct_icp` | `out_of_scope` | `out_of_scope` | n/a | n/a | n/a | 0.916 | 8.356 | 2.313 | `seed_specific_verifier` |
| `02` | `seed_velocity` | `out_of_scope` | `out_of_scope` | n/a | n/a | n/a | 0.000 | 67.959 | 83.426 | `seed_specific_verifier` |
| `05` | `seed_ct_icp` | `out_of_scope` | `out_of_scope` | n/a | n/a | n/a | 0.972 | 0.568 | 1.321 | `seed_specific_verifier` |
| `05` | `seed_velocity` | `out_of_scope` | `out_of_scope` | n/a | n/a | n/a | 0.000 | 55.976 | 99.995 | `seed_specific_verifier` |
| `08` | `seed_ct_icp` | `out_of_scope` | `out_of_scope` | n/a | n/a | n/a | 0.009 | 11.632 | 32.101 | `seed_specific_verifier` |
| `08` | `seed_velocity` | `out_of_scope` | `out_of_scope` | n/a | n/a | n/a | 0.000 | 22.021 | 88.922 | `seed_specific_verifier` |
| `02` | `seed_scan_context_stride_10` | `unsafe_wrong_pose` | `hold_for_sequence_prior` | 0.083 | 0.196 | n/a | 0.916 | 5.714 | 7.933 | `sequential_place_prior` |
| `02` | `seed_scan_context_stride_10_topk_10` | `unsafe_wrong_pose` | `hold_for_sequence_prior` | 0.083 | 0.196 | 0.196 | 0.916 | 5.714 | 7.933 | `sequential_place_prior` |
| `02` | `seed_scan_context_stride_10_topk_5` | `unsafe_wrong_pose` | `hold_for_sequence_prior` | 0.083 | 0.196 | 0.196 | 0.916 | 5.714 | 7.933 | `sequential_place_prior` |
| `05` | `seed_scan_context_stride_10` | `threshold_miss` | `hold_for_sequence_prior` | 0.092 | 0.243 | n/a | 0.916 | 4.353 | 6.568 | `sequential_place_prior` |
| `05` | `seed_scan_context_stride_10_topk_10` | `threshold_miss` | `hold_for_sequence_prior` | 0.092 | 0.243 | 0.243 | 0.916 | 4.353 | 6.568 | `sequential_place_prior` |
| `05` | `seed_scan_context_stride_10_topk_5` | `threshold_miss` | `hold_for_sequence_prior` | 0.092 | 0.243 | 0.243 | 0.916 | 4.353 | 6.568 | `sequential_place_prior` |
| `08` | `seed_scan_context_stride_10` | `unsafe_localization_gap` | `hold_for_sequence_prior` | 0.075 | 0.178 | n/a | 0.280 | 3.199 | 18.309 | `sequential_place_prior` |
| `08` | `seed_scan_context_stride_10_topk_10` | `unsafe_localization_gap` | `hold_for_sequence_prior` | 0.075 | 0.178 | 0.178 | 0.215 | 3.939 | 22.226 | `sequential_place_prior` |
| `08` | `seed_scan_context_stride_10_topk_5` | `unsafe_localization_gap` | `hold_for_sequence_prior` | 0.075 | 0.178 | 0.178 | 0.215 | 3.939 | 22.226 | `sequential_place_prior` |
| `02` | `seed_scan_context_stride_10_topk_10_unfiltered` | `unsafe_score_trap` | `reject_score_trap` | 0.266 | 1.000 | 10.000 | 1.000 | 53.370 | 93.676 | `filtered_retrieval_or_geometric_verifier` |
| `02` | `seed_scan_context_stride_10_topk_5_unfiltered` | `unsafe_score_trap` | `reject_score_trap` | 0.266 | 1.000 | 5.000 | 1.000 | 46.358 | 85.906 | `filtered_retrieval_or_geometric_verifier` |
| `02` | `seed_scan_context_stride_5_topk_10_unfiltered` | `unsafe_score_trap` | `reject_score_trap` | 0.179 | 1.000 | 10.000 | 1.000 | 46.457 | 90.006 | `filtered_retrieval_or_geometric_verifier` |
| `02` | `seed_scan_context_stride_5_topk_5_unfiltered` | `unsafe_score_trap` | `reject_score_trap` | 0.179 | 1.000 | 5.000 | 1.000 | 33.491 | 61.876 | `filtered_retrieval_or_geometric_verifier` |
| `05` | `seed_scan_context_stride_10_topk_10_unfiltered` | `unsafe_score_trap` | `reject_score_trap` | 0.256 | 1.000 | 10.000 | 1.000 | 53.798 | 136.835 | `filtered_retrieval_or_geometric_verifier` |
| `05` | `seed_scan_context_stride_10_topk_5_unfiltered` | `unsafe_score_trap` | `reject_score_trap` | 0.256 | 1.000 | 5.000 | 1.000 | 23.095 | 96.320 | `filtered_retrieval_or_geometric_verifier` |
| `05` | `seed_scan_context_stride_5_topk_10_unfiltered` | `unsafe_score_trap` | `reject_score_trap` | 0.158 | 1.000 | 10.000 | 1.000 | 38.674 | 92.938 | `filtered_retrieval_or_geometric_verifier` |
| `05` | `seed_scan_context_stride_5_topk_5_unfiltered` | `unsafe_score_trap` | `reject_score_trap` | 0.158 | 1.000 | 5.000 | 1.000 | 17.019 | 45.503 | `filtered_retrieval_or_geometric_verifier` |
| `08` | `seed_scan_context_stride_10_topk_10_unfiltered` | `unsafe_score_trap` | `reject_score_trap` | 0.259 | 1.000 | 10.000 | 0.533 | 13.407 | 72.870 | `filtered_retrieval_or_geometric_verifier` |
| `08` | `seed_scan_context_stride_10_topk_5_unfiltered` | `unsafe_localization_gap` | `reject_score_trap` | 0.259 | 1.000 | 5.000 | 0.421 | 9.153 | 56.964 | `filtered_retrieval_or_geometric_verifier` |
| `08` | `seed_scan_context_stride_5_topk_10_unfiltered` | `unsafe_score_trap` | `reject_score_trap` | 0.189 | 1.000 | 10.000 | 0.935 | 8.543 | 30.906 | `filtered_retrieval_or_geometric_verifier` |
| `08` | `seed_scan_context_stride_5_topk_5_unfiltered` | `unsafe_score_trap` | `reject_score_trap` | 0.189 | 1.000 | 5.000 | 0.869 | 3.902 | 15.027 | `filtered_retrieval_or_geometric_verifier` |

## Readout

- This verifier only decides whether a Scan Context global hypothesis stream may enter local NDT refinement.
- It never allows direct pose publish; publish still requires map consistency and runtime health gates.
- Unfiltered top-K NDT score selection is blocked before refinement because the audit shows it can select high-acceptance wrong poses.
- Filtered stride-5 retrieval is retained for refinement, but it is still not a product localization output by itself.
