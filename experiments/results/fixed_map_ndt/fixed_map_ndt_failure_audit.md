# Fixed-Map NDT Failure Audit

Input dir: `experiments/results/fixed_map_ndt`

## Gate

- Product candidate: ATE <= 1.0 m, RPE <= 5.0%, FPS >= 10.0
- Silent bad accept: accepted rate >= 0.5 and ATE > 5.0 m or RPE > 10.0%

## Aggregate

| Rows | Pass | Watch | Investigate | Fail | Silent bad accepts | Non-oracle product candidates |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 51 | 12 | 16 | 8 | 15 | 15 | 0 |

## Source Summary

| Seed source | Rows | Pass | Watch | Investigate | Fail | Best ATE [m] | Best RPE [%] |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `ct_icp` | 3 | 0 | 1 | 1 | 1 | 0.568 | 1.321 |
| `gt` | 3 | 3 | 0 | 0 | 0 | 0.003 | 0.011 |
| `gt_perturbed` | 9 | 6 | 3 | 0 | 0 | 0.037 | 0.115 |
| `scan_context` | 33 | 3 | 12 | 4 | 14 | 0.003 | 0.011 |
| `velocity` | 3 | 0 | 0 | 3 | 0 | 22.021 | 83.426 |

## Best Non-Oracle Per Sequence

| Sequence | Source | Variant | ATE [m] | RPE [%] | FPS | Accepted | Decision | Reasons |
| --- | --- | --- | ---: | ---: | ---: | ---: | --- | --- |
| `02` | `scan_context` | `seed_scan_context_stride_5` | 2.579 | 3.817 | 18.286 | 98/107 | `watch` | global_initializer_near_basin |
| `05` | `ct_icp` | `seed_ct_icp` | 0.568 | 1.321 | 8.755 | 104/107 | `watch` | runtime_below_budget |
| `08` | `scan_context` | `seed_scan_context_stride_5` | 1.483 | 8.799 | 61.357 | 59/107 | `watch` | global_initializer_near_basin |

## Risk Rows

| Sequence | Source | Variant | ATE [m] | RPE [%] | FPS | Accepted | Decision | Reasons |
| --- | --- | --- | ---: | ---: | ---: | ---: | --- | --- |
| `02` | `ct_icp` | `seed_ct_icp` | 8.356 | 2.313 | 11.809 | 98/107 | `fail` | accepted_bad_localization |
| `02` | `scan_context` | `seed_scan_context_stride_5_topk_10_unfiltered` | 46.457 | 90.006 | 2.164 | 107/107 | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `02` | `scan_context` | `seed_scan_context_stride_5_topk_5_unfiltered` | 33.491 | 61.876 | 3.997 | 107/107 | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `02` | `scan_context` | `seed_scan_context_stride_10` | 5.714 | 7.933 | 23.325 | 98/107 | `fail` | accepted_bad_localization |
| `02` | `scan_context` | `seed_scan_context_stride_10_topk_10` | 5.714 | 7.933 | 23.114 | 98/107 | `fail` | accepted_bad_localization |
| `02` | `scan_context` | `seed_scan_context_stride_10_topk_10_unfiltered` | 53.370 | 93.676 | 2.737 | 107/107 | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `02` | `scan_context` | `seed_scan_context_stride_10_topk_5` | 5.714 | 7.933 | 23.287 | 98/107 | `fail` | accepted_bad_localization |
| `02` | `scan_context` | `seed_scan_context_stride_10_topk_5_unfiltered` | 46.358 | 85.906 | 5.364 | 107/107 | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `02` | `velocity` | `seed_velocity` | 67.959 | 83.426 | 11.318 | 0/107 | `investigate` | rejected_bad_seed_detectable |
| `05` | `scan_context` | `seed_scan_context_stride_5_topk_10_unfiltered` | 38.674 | 92.938 | 2.147 | 107/107 | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `05` | `scan_context` | `seed_scan_context_stride_5_topk_5_unfiltered` | 17.019 | 45.503 | 4.141 | 107/107 | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `05` | `scan_context` | `seed_scan_context_stride_10_topk_10_unfiltered` | 53.798 | 136.835 | 2.531 | 107/107 | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `05` | `scan_context` | `seed_scan_context_stride_10_topk_5_unfiltered` | 23.095 | 96.320 | 4.916 | 107/107 | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `05` | `velocity` | `seed_velocity` | 55.976 | 99.995 | 12.233 | 0/107 | `investigate` | rejected_bad_seed_detectable |
| `08` | `ct_icp` | `seed_ct_icp` | 11.632 | 32.101 | 21.691 | 1/107 | `investigate` | rejected_bad_seed_detectable |
| `08` | `scan_context` | `seed_scan_context_stride_5_topk_10_unfiltered` | 8.543 | 30.906 | 18.524 | 100/107 | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `08` | `scan_context` | `seed_scan_context_stride_5_topk_5_unfiltered` | 3.902 | 15.027 | 25.870 | 93/107 | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `08` | `scan_context` | `seed_scan_context_stride_10` | 3.199 | 18.309 | 84.704 | 30/107 | `investigate` | bad_localization |
| `08` | `scan_context` | `seed_scan_context_stride_10_topk_10` | 3.939 | 22.226 | 90.909 | 23/107 | `investigate` | bad_localization |
| `08` | `scan_context` | `seed_scan_context_stride_10_topk_10_unfiltered` | 13.407 | 72.870 | 36.888 | 57/107 | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `08` | `scan_context` | `seed_scan_context_stride_10_topk_5` | 3.939 | 22.226 | 93.490 | 23/107 | `investigate` | bad_localization |
| `08` | `scan_context` | `seed_scan_context_stride_10_topk_5_unfiltered` | 9.153 | 56.964 | 51.926 | 45/107 | `investigate` | bad_localization |
| `08` | `velocity` | `seed_velocity` | 22.021 | 88.922 | 13.994 | 0/107 | `investigate` | rejected_bad_seed_detectable |

## Readout

- GT-seeded rows validate the fixed-map NDT localizer and map path; they are oracle smoke tests, not deployable initialization.
- Scan Context stride=1 rows use every reference frame in the retrieval DB, so they are dense-DB smoke checks and are excluded from non-oracle product-candidate counts.
- High acceptance with bad GT error is treated as the dangerous case because it can silently publish a wrong pose.
- Low acceptance with bad GT error is still bad localization, but it is easier to guard because the localizer is already rejecting most corrections.
- Sparse Scan Context rows that miss the product threshold remain useful as initializer-basin probes, while unfiltered top-K rows expose NDT-score-only selection risk.
