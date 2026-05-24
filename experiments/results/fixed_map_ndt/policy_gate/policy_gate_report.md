# LiDAR Degeneracy Policy Gate

Policy: `lidar_degeneracy_triage_v4`

## Reports

| Report | Total | Pass | Watch | Investigate | Fail |
| --- | ---: | ---: | ---: | ---: | ---: |
| `method_health_comparison` | 28 | 15 | 13 | 0 | 0 |
| `risk_gt_calibration` | 28 | 15 | 13 | 0 | 0 |
| `fixed_map_ndt_failure_audit` | 51 | 12 | 16 | 8 | 15 |

## Violations

- fixed_map_ndt_failure_audit: fail rows 15 > 0
- fixed_map_ndt_failure_audit: investigate rows 8 > 0

## Top Offenders

| Report | Sequence | Window | Method | Decision | Reasons |
| --- | --- | --- | --- | --- | --- |
| `fixed_map_ndt_failure_audit` | `kitti_seq_02` | `seed_ct_icp` | `fixed_map_ndt:ct_icp` | `fail` | accepted_bad_localization |
| `fixed_map_ndt_failure_audit` | `kitti_seq_02` | `seed_scan_context_stride_10` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization |
| `fixed_map_ndt_failure_audit` | `kitti_seq_02` | `seed_scan_context_stride_10_topk_10` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization |
| `fixed_map_ndt_failure_audit` | `kitti_seq_02` | `seed_scan_context_stride_10_topk_10_unfiltered` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `fixed_map_ndt_failure_audit` | `kitti_seq_02` | `seed_scan_context_stride_10_topk_5` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization |
| `fixed_map_ndt_failure_audit` | `kitti_seq_02` | `seed_scan_context_stride_10_topk_5_unfiltered` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `fixed_map_ndt_failure_audit` | `kitti_seq_02` | `seed_scan_context_stride_5_topk_10_unfiltered` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `fixed_map_ndt_failure_audit` | `kitti_seq_02` | `seed_scan_context_stride_5_topk_5_unfiltered` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `fixed_map_ndt_failure_audit` | `kitti_seq_05` | `seed_scan_context_stride_10_topk_10_unfiltered` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `fixed_map_ndt_failure_audit` | `kitti_seq_05` | `seed_scan_context_stride_10_topk_5_unfiltered` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `fixed_map_ndt_failure_audit` | `kitti_seq_05` | `seed_scan_context_stride_5_topk_10_unfiltered` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `fixed_map_ndt_failure_audit` | `kitti_seq_05` | `seed_scan_context_stride_5_topk_5_unfiltered` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `fixed_map_ndt_failure_audit` | `kitti_seq_08` | `seed_scan_context_stride_10_topk_10_unfiltered` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `fixed_map_ndt_failure_audit` | `kitti_seq_08` | `seed_scan_context_stride_5_topk_10_unfiltered` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `fixed_map_ndt_failure_audit` | `kitti_seq_08` | `seed_scan_context_stride_5_topk_5_unfiltered` | `fixed_map_ndt:scan_context` | `fail` | accepted_bad_localization, unfiltered_ndt_score_trap |
| `fixed_map_ndt_failure_audit` | `kitti_seq_02` | `seed_velocity` | `fixed_map_ndt:velocity` | `investigate` | rejected_bad_seed_detectable |
| `fixed_map_ndt_failure_audit` | `kitti_seq_05` | `seed_velocity` | `fixed_map_ndt:velocity` | `investigate` | rejected_bad_seed_detectable |
| `fixed_map_ndt_failure_audit` | `kitti_seq_08` | `seed_ct_icp` | `fixed_map_ndt:ct_icp` | `investigate` | rejected_bad_seed_detectable |
| `fixed_map_ndt_failure_audit` | `kitti_seq_08` | `seed_scan_context_stride_10` | `fixed_map_ndt:scan_context` | `investigate` | bad_localization |
| `fixed_map_ndt_failure_audit` | `kitti_seq_08` | `seed_scan_context_stride_10_topk_10` | `fixed_map_ndt:scan_context` | `investigate` | bad_localization |
| `fixed_map_ndt_failure_audit` | `kitti_seq_08` | `seed_scan_context_stride_10_topk_5` | `fixed_map_ndt:scan_context` | `investigate` | bad_localization |
| `fixed_map_ndt_failure_audit` | `kitti_seq_08` | `seed_scan_context_stride_10_topk_5_unfiltered` | `fixed_map_ndt:scan_context` | `investigate` | bad_localization |
| `fixed_map_ndt_failure_audit` | `kitti_seq_08` | `seed_velocity` | `fixed_map_ndt:velocity` | `investigate` | rejected_bad_seed_detectable |
