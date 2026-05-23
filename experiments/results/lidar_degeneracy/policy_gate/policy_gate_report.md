# LiDAR Degeneracy Policy Gate

Policy: `lidar_degeneracy_triage_v1`

## Reports

| Report | Total | Pass | Watch | Investigate | Fail |
| --- | ---: | ---: | ---: | ---: | ---: |
| `method_health_comparison` | 28 | 10 | 11 | 5 | 2 |
| `risk_gt_calibration` | 28 | 10 | 11 | 5 | 2 |

## Violations

- method_health_comparison: fail rows 2 > 0
- method_health_comparison: investigate rows 5 > 0
- risk_gt_calibration: fail rows 2 > 0
- risk_gt_calibration: investigate rows 5 > 0

## Top Offenders

| Report | Sequence | Window | Method | Decision | Reasons |
| --- | --- | --- | --- | --- | --- |
| `method_health_comparison` | `fog_200` | `degraded` | `geometry_icp` | `fail` | all_pairs_failed, low_acceptance |
| `method_health_comparison` | `fog_200` | `point_count_tail` | `geometry_icp` | `fail` | low_acceptance |
| `risk_gt_calibration` | `fog_200` | `degraded` | `geometry_icp` | `fail` | all_pairs_failed, low_acceptance |
| `risk_gt_calibration` | `fog_200` | `point_count_tail` | `geometry_icp` | `fail` | low_acceptance |
| `method_health_comparison` | `fog_200` | `degraded` | `kiss_keyframe` | `investigate` | cross_method_suspicious, path_disagrees_with_healthy_median |
| `method_health_comparison` | `fog_200` | `point_count_tail` | `intensity_bev` | `investigate` | motion_margin_dominant, overlap_tail |
| `method_health_comparison` | `tunnel_geom_2700_200` | `degraded` | `intensity_bev` | `investigate` | motion_margin_dominant |
| `method_health_comparison` | `tunnel_geom_2700_200` | `geometry_degeneracy` | `intensity_bev` | `investigate` | cross_method_suspicious, path_disagrees_with_all_method_median, path_disagrees_with_healthy_median |
| `method_health_comparison` | `tunnel_geom_2700_200` | `point_count_tail` | `intensity_bev` | `investigate` | cross_method_suspicious, path_disagrees_with_healthy_median |
| `risk_gt_calibration` | `fog_200` | `degraded` | `kiss_keyframe` | `investigate` | cross_method_suspicious, path_disagrees_with_healthy_median |
| `risk_gt_calibration` | `fog_200` | `point_count_tail` | `intensity_bev` | `investigate` | motion_margin_dominant, overlap_tail |
| `risk_gt_calibration` | `tunnel_geom_2700_200` | `degraded` | `intensity_bev` | `investigate` | motion_margin_dominant |
| `risk_gt_calibration` | `tunnel_geom_2700_200` | `geometry_degeneracy` | `intensity_bev` | `investigate` | cross_method_suspicious, path_disagrees_with_all_method_median, path_disagrees_with_healthy_median |
| `risk_gt_calibration` | `tunnel_geom_2700_200` | `point_count_tail` | `intensity_bev` | `investigate` | cross_method_suspicious, path_disagrees_with_healthy_median |
