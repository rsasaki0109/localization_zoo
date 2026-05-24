# Fixed-Map NDT Publish Guard

Audit source: `experiments/results/fixed_map_ndt/fixed_map_ndt_failure_audit.json`

## Aggregate

| Rows | Publishable | Decisions | Blocked seed sources |
| ---: | ---: | --- | --- |
| 51 | 0 | block_publish:15, hold_for_verifier:16, lab_only:15, profile_before_publish:1, return_unknown:4 | ct_icp:2, scan_context:14, velocity:3 |

Required gates: additional_evidence:12, calibrated_global_hypothesis_verifier:11, map_consistency_or_sequence_verifier:4, non_oracle_runtime_initializer:15, runtime_budget:1, second_pose_verifier:4, unknown_state_output:4

## Guard Rows

| Sequence | Variant | Seed | Audit | Publish decision | Safety state | ATE [m] | RPE [%] | FPS | Accepted | Required gate |
| --- | --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| `05` | `seed_ct_icp` | `ct_icp` | `watch` | `profile_before_publish` | `runtime_tail_risk` | 0.568 | 1.321 | 8.755 | 104/107 | `runtime_budget` |
| `02` | `gt_x0p25` | `gt_perturbed` | `pass` | `lab_only` | `basin_probe` | 0.163 | 0.231 | 13.558 | 106/107 | `non_oracle_runtime_initializer` |
| `02` | `gt_x0p5` | `gt_perturbed` | `pass` | `lab_only` | `basin_probe` | 0.488 | 0.692 | 12.731 | 107/107 | `non_oracle_runtime_initializer` |
| `02` | `gt_yaw20` | `gt_perturbed` | `watch` | `lab_only` | `basin_probe` | 0.049 | 26.908 | 13.364 | 107/107 | `non_oracle_runtime_initializer` |
| `02` | `seed_gt` | `gt` | `pass` | `lab_only` | `oracle_seed` | 0.003 | 0.012 | 32.471 | 105/107 | `non_oracle_runtime_initializer` |
| `02` | `seed_scan_context` | `scan_context` | `pass` | `lab_only` | `dense_db_smoke` | 0.003 | 0.012 | 35.400 | 105/107 | `non_oracle_runtime_initializer` |
| `05` | `gt_x0p25` | `gt_perturbed` | `pass` | `lab_only` | `basin_probe` | 0.158 | 0.393 | 14.376 | 107/107 | `non_oracle_runtime_initializer` |
| `05` | `gt_x0p5` | `gt_perturbed` | `pass` | `lab_only` | `basin_probe` | 0.483 | 0.115 | 13.380 | 107/107 | `non_oracle_runtime_initializer` |
| `05` | `gt_yaw20` | `gt_perturbed` | `watch` | `lab_only` | `basin_probe` | 0.037 | 34.129 | 13.016 | 105/107 | `non_oracle_runtime_initializer` |
| `05` | `seed_gt` | `gt` | `pass` | `lab_only` | `oracle_seed` | 0.003 | 0.011 | 35.154 | 106/107 | `non_oracle_runtime_initializer` |
| `05` | `seed_scan_context` | `scan_context` | `pass` | `lab_only` | `dense_db_smoke` | 0.003 | 0.011 | 32.803 | 106/107 | `non_oracle_runtime_initializer` |
| `08` | `gt_x0p25` | `gt_perturbed` | `pass` | `lab_only` | `basin_probe` | 0.231 | 0.957 | 16.809 | 78/107 | `non_oracle_runtime_initializer` |
| `08` | `gt_x0p5` | `gt_perturbed` | `pass` | `lab_only` | `basin_probe` | 0.495 | 2.078 | 20.537 | 78/107 | `non_oracle_runtime_initializer` |
| `08` | `gt_yaw20` | `gt_perturbed` | `watch` | `lab_only` | `basin_probe` | 0.145 | 28.307 | 24.973 | 62/107 | `non_oracle_runtime_initializer` |
| `08` | `seed_gt` | `gt` | `pass` | `lab_only` | `oracle_seed` | 0.021 | 0.057 | 45.962 | 99/107 | `non_oracle_runtime_initializer` |
| `08` | `seed_scan_context` | `scan_context` | `pass` | `lab_only` | `dense_db_smoke` | 0.021 | 0.057 | 46.043 | 99/107 | `non_oracle_runtime_initializer` |
| `02` | `seed_scan_context_stride_5` | `scan_context` | `watch` | `hold_for_verifier` | `needs_verifier` | 2.579 | 3.817 | 18.286 | 98/107 | `additional_evidence` |
| `02` | `seed_scan_context_stride_5_topk_10` | `scan_context` | `watch` | `hold_for_verifier` | `needs_verifier` | 2.627 | 3.817 | 18.615 | 98/107 | `additional_evidence` |
| `02` | `seed_scan_context_stride_5_topk_5` | `scan_context` | `watch` | `hold_for_verifier` | `needs_verifier` | 2.627 | 3.817 | 18.161 | 98/107 | `additional_evidence` |
| `05` | `seed_scan_context_stride_10` | `scan_context` | `watch` | `hold_for_verifier` | `needs_verifier` | 4.353 | 6.568 | 20.264 | 98/107 | `additional_evidence` |
| `05` | `seed_scan_context_stride_10_topk_10` | `scan_context` | `watch` | `hold_for_verifier` | `needs_verifier` | 4.353 | 6.568 | 20.562 | 98/107 | `additional_evidence` |
| `05` | `seed_scan_context_stride_10_topk_5` | `scan_context` | `watch` | `hold_for_verifier` | `needs_verifier` | 4.353 | 6.568 | 20.296 | 98/107 | `additional_evidence` |
| `05` | `seed_scan_context_stride_5` | `scan_context` | `watch` | `hold_for_verifier` | `needs_verifier` | 1.730 | 3.926 | 20.084 | 103/107 | `additional_evidence` |
| `05` | `seed_scan_context_stride_5_topk_10` | `scan_context` | `watch` | `hold_for_verifier` | `needs_verifier` | 1.730 | 3.926 | 20.192 | 103/107 | `additional_evidence` |
| `05` | `seed_scan_context_stride_5_topk_5` | `scan_context` | `watch` | `hold_for_verifier` | `needs_verifier` | 1.730 | 3.926 | 20.672 | 103/107 | `additional_evidence` |
| `08` | `seed_scan_context_stride_10` | `scan_context` | `investigate` | `hold_for_verifier` | `map_localization_gap` | 3.199 | 18.309 | 84.704 | 30/107 | `map_consistency_or_sequence_verifier` |
| `08` | `seed_scan_context_stride_10_topk_10` | `scan_context` | `investigate` | `hold_for_verifier` | `map_localization_gap` | 3.939 | 22.226 | 90.909 | 23/107 | `map_consistency_or_sequence_verifier` |
| `08` | `seed_scan_context_stride_10_topk_5` | `scan_context` | `investigate` | `hold_for_verifier` | `map_localization_gap` | 3.939 | 22.226 | 93.490 | 23/107 | `map_consistency_or_sequence_verifier` |
| `08` | `seed_scan_context_stride_10_topk_5_unfiltered` | `scan_context` | `investigate` | `hold_for_verifier` | `map_localization_gap` | 9.153 | 56.964 | 51.926 | 45/107 | `map_consistency_or_sequence_verifier` |
| `08` | `seed_scan_context_stride_5` | `scan_context` | `watch` | `hold_for_verifier` | `needs_verifier` | 1.483 | 8.799 | 61.357 | 59/107 | `additional_evidence` |
| `08` | `seed_scan_context_stride_5_topk_10` | `scan_context` | `watch` | `hold_for_verifier` | `needs_verifier` | 1.503 | 8.874 | 60.584 | 59/107 | `additional_evidence` |
| `08` | `seed_scan_context_stride_5_topk_5` | `scan_context` | `watch` | `hold_for_verifier` | `needs_verifier` | 1.503 | 8.874 | 53.283 | 59/107 | `additional_evidence` |
| `02` | `seed_velocity` | `velocity` | `investigate` | `return_unknown` | `bad_seed_detected` | 67.959 | 83.426 | 11.318 | 0/107 | `unknown_state_output` |
| `05` | `seed_velocity` | `velocity` | `investigate` | `return_unknown` | `bad_seed_detected` | 55.976 | 99.995 | 12.233 | 0/107 | `unknown_state_output` |
| `08` | `seed_ct_icp` | `ct_icp` | `investigate` | `return_unknown` | `bad_seed_detected` | 11.632 | 32.101 | 21.691 | 1/107 | `unknown_state_output` |
| `08` | `seed_velocity` | `velocity` | `investigate` | `return_unknown` | `bad_seed_detected` | 22.021 | 88.922 | 13.994 | 0/107 | `unknown_state_output` |
| `02` | `seed_ct_icp` | `ct_icp` | `fail` | `block_publish` | `accepted_wrong_pose` | 8.356 | 2.313 | 11.809 | 98/107 | `second_pose_verifier` |
| `02` | `seed_scan_context_stride_10` | `scan_context` | `fail` | `block_publish` | `accepted_wrong_pose` | 5.714 | 7.933 | 23.325 | 98/107 | `second_pose_verifier` |
| `02` | `seed_scan_context_stride_10_topk_10` | `scan_context` | `fail` | `block_publish` | `accepted_wrong_pose` | 5.714 | 7.933 | 23.114 | 98/107 | `second_pose_verifier` |
| `02` | `seed_scan_context_stride_10_topk_10_unfiltered` | `scan_context` | `fail` | `block_publish` | `global_candidate_score_trap` | 53.370 | 93.676 | 2.737 | 107/107 | `calibrated_global_hypothesis_verifier` |
| `02` | `seed_scan_context_stride_10_topk_5` | `scan_context` | `fail` | `block_publish` | `accepted_wrong_pose` | 5.714 | 7.933 | 23.287 | 98/107 | `second_pose_verifier` |
| `02` | `seed_scan_context_stride_10_topk_5_unfiltered` | `scan_context` | `fail` | `block_publish` | `global_candidate_score_trap` | 46.358 | 85.906 | 5.364 | 107/107 | `calibrated_global_hypothesis_verifier` |
| `02` | `seed_scan_context_stride_5_topk_10_unfiltered` | `scan_context` | `fail` | `block_publish` | `global_candidate_score_trap` | 46.457 | 90.006 | 2.164 | 107/107 | `calibrated_global_hypothesis_verifier` |
| `02` | `seed_scan_context_stride_5_topk_5_unfiltered` | `scan_context` | `fail` | `block_publish` | `global_candidate_score_trap` | 33.491 | 61.876 | 3.997 | 107/107 | `calibrated_global_hypothesis_verifier` |
| `05` | `seed_scan_context_stride_10_topk_10_unfiltered` | `scan_context` | `fail` | `block_publish` | `global_candidate_score_trap` | 53.798 | 136.835 | 2.531 | 107/107 | `calibrated_global_hypothesis_verifier` |
| `05` | `seed_scan_context_stride_10_topk_5_unfiltered` | `scan_context` | `fail` | `block_publish` | `global_candidate_score_trap` | 23.095 | 96.320 | 4.916 | 107/107 | `calibrated_global_hypothesis_verifier` |
| `05` | `seed_scan_context_stride_5_topk_10_unfiltered` | `scan_context` | `fail` | `block_publish` | `global_candidate_score_trap` | 38.674 | 92.938 | 2.147 | 107/107 | `calibrated_global_hypothesis_verifier` |
| `05` | `seed_scan_context_stride_5_topk_5_unfiltered` | `scan_context` | `fail` | `block_publish` | `global_candidate_score_trap` | 17.019 | 45.503 | 4.141 | 107/107 | `calibrated_global_hypothesis_verifier` |
| `08` | `seed_scan_context_stride_10_topk_10_unfiltered` | `scan_context` | `fail` | `block_publish` | `global_candidate_score_trap` | 13.407 | 72.870 | 36.888 | 57/107 | `calibrated_global_hypothesis_verifier` |
| `08` | `seed_scan_context_stride_5_topk_10_unfiltered` | `scan_context` | `fail` | `block_publish` | `global_candidate_score_trap` | 8.543 | 30.906 | 18.524 | 100/107 | `calibrated_global_hypothesis_verifier` |
| `08` | `seed_scan_context_stride_5_topk_5_unfiltered` | `scan_context` | `fail` | `block_publish` | `global_candidate_score_trap` | 3.902 | 15.027 | 25.870 | 93/107 | `calibrated_global_hypothesis_verifier` |

## Readout

- This guard is generated from GT-backed audit results. It is a product-contract candidate, not a runtime estimator by itself.
- `allow_pose_publish=false` means the row should publish `unknown`, retry relocalization, or stay in a lab-only benchmark path.
- The current fixed-map NDT slice has zero publishable non-oracle rows, so the next implementation target is a verifier for global hypotheses, not a new leaderboard table.
