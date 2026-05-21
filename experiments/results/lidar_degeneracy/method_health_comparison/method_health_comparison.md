# LiDAR Degeneracy Method Health Comparison

## Method Aggregate

| Sequence | Method | Windows | Mean accepted | Min accepted | Mean converged | Failed windows | Local risk | Cross-method risk | Total risk | Max used path m |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `fog_200` | `geometry_icp` | 3 | 0.264 | 0.000 | 0.264 | 2 | 2 | 0 | 2 | 1.276 |
| `fog_200` | `intensity_bev` | 3 | 0.977 | 0.931 | 0.977 | 0 | 1 | 1 | 2 | 11.516 |
| `fog_200` | `kiss_keyframe` | 3 | 0.000 | 0.000 | 0.000 | 3 | 3 | 0 | 3 | 0.000 |
| `fog_200` | `ct_icp` | 3 | 0.885 | 0.655 | 0.046 | 0 | 3 | 0 | 3 | 2.703 |
| `tunnel_geom_2700_200` | `geometry_icp` | 4 | 1.000 | 1.000 | 1.000 | 0 | 0 | 0 | 0 | 1.867 |
| `tunnel_geom_2700_200` | `intensity_bev` | 4 | 1.000 | 1.000 | 1.000 | 0 | 1 | 2 | 3 | 12.642 |
| `tunnel_geom_2700_200` | `kiss_keyframe` | 4 | 1.000 | 1.000 | 1.000 | 0 | 0 | 0 | 0 | 1.186 |
| `tunnel_geom_2700_200` | `ct_icp` | 4 | 1.000 | 1.000 | 0.147 | 0 | 4 | 0 | 4 | 5.676 |

## Failure Awareness

| Sequence | Method | Stress windows | Local flagged | Cross-method flagged | Residual unflagged | False alarms |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| `fog_200` | `geometry_icp` | 2 | 2 | 0 | 0 | 1 |
| `fog_200` | `intensity_bev` | 2 | 1 | 1 | 0 | 0 |
| `fog_200` | `kiss_keyframe` | 2 | 2 | 0 | 0 | 1 |
| `fog_200` | `ct_icp` | 2 | 2 | 0 | 0 | 1 |
| `tunnel_geom_2700_200` | `geometry_icp` | 3 | 0 | 0 | 3 | 0 |
| `tunnel_geom_2700_200` | `intensity_bev` | 3 | 1 | 2 | 0 | 0 |
| `tunnel_geom_2700_200` | `kiss_keyframe` | 3 | 0 | 0 | 3 | 0 |
| `tunnel_geom_2700_200` | `ct_icp` | 3 | 3 | 0 | 0 | 1 |

## Confidence Probes

| Sequence | Window | Method | Accepted | Score min | Overlap min | Motion-margin rate | Decision reasons | Probe |
| --- | --- | --- | ---: | ---: | ---: | ---: | --- | --- |
| `fog_200` | `degraded` 170-199 | `intensity_bev` | 1.000 | 0.751 | 3565.0 | 0.138 | best_score:25, motion_margin:4 | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `geometry_icp` | 1.000 | 0.694 | 495.0 | n/a | n/a | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `intensity_bev` | 1.000 | 0.496 | 4338.0 | 0.483 | best_score:15, motion_margin:14 | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `kiss_keyframe` | 1.000 | 0.718 | 2554.0 | n/a | n/a | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_icp` | 1.000 | 0.694 | 512.0 | n/a | n/a | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `intensity_bev` | 1.000 | 0.496 | 4338.0 | 0.207 | best_score:23, motion_margin:6 | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `kiss_keyframe` | 1.000 | 0.718 | 2628.0 | n/a | n/a | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `geometry_icp` | 1.000 | 0.697 | 607.0 | n/a | n/a | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `kiss_keyframe` | 1.000 | 0.707 | 2957.0 | n/a | n/a | needs_gt_or_cross_method_check |

## Cross-Method Consistency

| Sequence | Window | Method | Path m | Net m | Yaw deg | Healthy peers | Healthy median path m | Path/healthy | All median path m | Path/all | Cross risk | Probe |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |
| `fog_200` | `degraded` 170-199 | `intensity_bev` | 8.281 | 6.127 | 46.500 | 0 | 8.281 | 1.000 | 1.362 | 6.081 | yes | no_healthy_peer, path_disagrees_with_all_method_median |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `geometry_icp` | 1.867 | 0.123 | 2.392 | 2 | 1.867 | 1.000 | 3.651 | 0.511 | no | none |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `intensity_bev` | 9.875 | 9.213 | 7.500 | 2 | 1.867 | 5.290 | 3.651 | 2.705 | yes | path_disagrees_with_healthy_median |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `kiss_keyframe` | 1.072 | 0.398 | 0.072 | 2 | 1.867 | 0.574 | 3.651 | 0.294 | no | path_disagrees_with_all_method_median |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_icp` | 1.766 | 0.596 | 2.385 | 2 | 1.766 | 1.000 | 3.716 | 0.475 | no | none |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `intensity_bev` | 12.642 | 11.841 | 25.500 | 2 | 1.766 | 7.157 | 3.716 | 3.402 | yes | path_disagrees_with_healthy_median, path_disagrees_with_all_method_median |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `kiss_keyframe` | 1.259 | 0.098 | 0.664 | 2 | 1.766 | 0.713 | 3.716 | 0.339 | no | none |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `geometry_icp` | 1.178 | 0.311 | 1.798 | 1 | 1.380 | 0.853 | 2.878 | 0.409 | no | none |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `kiss_keyframe` | 1.582 | 1.005 | 1.184 | 1 | 1.380 | 1.147 | 2.878 | 0.550 | no | none |

## Window Detail

| Sequence | Window | Expected | Frames | Obscurant | Method | Accepted | Converged | Score | Overlap | Used path m | Max step m | State | Risk state | Failure awareness | Risk awareness | Keyframes | Flags | Risk flags |
| --- | --- | --- | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- | --- | --- | --- | --- | --- |
| `fog_200` | `baseline` 0-29 | `nominal` | 30 | 0.006 | `geometry_icp` | 0.655 | n/a | 0.775 | 65.9 | 1.276 | 0.095 | `suspicious` | `suspicious` | `false_alarm` | `false_alarm` | n/a | ok | ok |
| `fog_200` | `baseline` 0-29 | `nominal` | 30 | 0.006 | `intensity_bev` | 1.000 | 1.000 | 0.606 | 5852.6 | 7.250 | 0.250 | `ok` | `ok` | `nominal_ok` | `nominal_ok` | n/a | ok | ok |
| `fog_200` | `baseline` 0-29 | `nominal` | 30 | 0.006 | `kiss_keyframe` | 0.000 | 0.000 | 0.844 | 309.0 | 0.000 | 0.000 | `failed` | `failed` | `false_alarm` | `false_alarm` | 0/2 | all_pairs_failed, low_acceptance, low_convergence | all_pairs_failed, low_acceptance, low_convergence |
| `fog_200` | `baseline` 0-29 | `nominal` | 30 | 0.006 | `ct_icp` | 1.000 | 0.138 | n/a | n/a | 1.283 | 0.126 | `degraded` | `degraded` | `false_alarm` | `false_alarm` | n/a | low_convergence | low_convergence |
| `fog_200` | `point_count_tail` 115-144 | `obscurant_stress` | 30 | 0.521 | `geometry_icp` | 0.138 | n/a | 0.790 | 11.3 | 0.326 | 0.057 | `failed` | `failed` | `stress_flagged` | `stress_flagged` | n/a | low_acceptance | low_acceptance |
| `fog_200` | `point_count_tail` 115-144 | `obscurant_stress` | 30 | 0.521 | `intensity_bev` | 0.931 | 0.931 | 0.743 | 5159.7 | 11.516 | 1.521 | `suspicious` | `suspicious` | `stress_flagged` | `stress_flagged` | n/a | motion_margin_dominant, overlap_tail | motion_margin_dominant, overlap_tail |
| `fog_200` | `point_count_tail` 115-144 | `obscurant_stress` | 30 | 0.521 | `kiss_keyframe` | 0.000 | 0.000 | 0.795 | 239.4 | 0.000 | 0.000 | `failed` | `failed` | `stress_flagged` | `stress_flagged` | 0/2 | all_pairs_failed, low_acceptance, low_convergence | all_pairs_failed, low_acceptance, low_convergence |
| `fog_200` | `point_count_tail` 115-144 | `obscurant_stress` | 30 | 0.521 | `ct_icp` | 1.000 | 0.000 | n/a | n/a | 2.703 | 0.323 | `degraded` | `degraded` | `stress_flagged` | `stress_flagged` | n/a | low_convergence | low_convergence |
| `fog_200` | `degraded` 170-199 | `obscurant_stress` | 30 | 0.911 | `geometry_icp` | 0.000 | n/a | n/a | 0.0 | 0.000 | 0.000 | `failed` | `failed` | `stress_flagged` | `stress_flagged` | n/a | all_pairs_failed, low_acceptance | all_pairs_failed, low_acceptance |
| `fog_200` | `degraded` 170-199 | `obscurant_stress` | 30 | 0.911 | `intensity_bev` | 1.000 | 1.000 | 0.801 | 5542.0 | 8.281 | 0.559 | `ok` | `cross_method_suspicious` | `stress_unflagged` | `stress_flagged` | n/a | ok | cross_method_suspicious |
| `fog_200` | `degraded` 170-199 | `obscurant_stress` | 30 | 0.911 | `kiss_keyframe` | 0.000 | 0.000 | 0.794 | 163.3 | 0.000 | 0.000 | `failed` | `failed` | `stress_flagged` | `stress_flagged` | 0/2 | all_pairs_failed, low_acceptance, low_convergence | all_pairs_failed, low_acceptance, low_convergence |
| `fog_200` | `degraded` 170-199 | `obscurant_stress` | 30 | 0.911 | `ct_icp` | 0.655 | 0.000 | n/a | n/a | 2.655 | 0.229 | `suspicious` | `suspicious` | `stress_flagged` | `stress_flagged` | n/a | low_convergence | low_convergence |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `nominal` | 30 | 0.002 | `geometry_icp` | 1.000 | n/a | 0.704 | 472.6 | 1.519 | 0.194 | `ok` | `ok` | `nominal_ok` | `nominal_ok` | n/a | ok | ok |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `nominal` | 30 | 0.002 | `intensity_bev` | 1.000 | 1.000 | 0.535 | 5522.3 | 7.975 | 0.354 | `ok` | `ok` | `nominal_ok` | `nominal_ok` | n/a | ok | ok |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `nominal` | 30 | 0.002 | `kiss_keyframe` | 1.000 | 1.000 | 0.747 | 2255.8 | 1.186 | 0.100 | `ok` | `ok` | `nominal_ok` | `nominal_ok` | 2/2 | ok | ok |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `nominal` | 30 | 0.002 | `ct_icp` | 1.000 | 0.138 | n/a | n/a | 5.189 | 0.842 | `degraded` | `degraded` | `false_alarm` | `false_alarm` | n/a | low_convergence | low_convergence |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `point_count_stress` | 30 | 0.035 | `geometry_icp` | 1.000 | n/a | 0.706 | 525.0 | 1.867 | 0.213 | `ok` | `ok` | `stress_unflagged` | `stress_unflagged` | n/a | ok | ok |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `point_count_stress` | 30 | 0.035 | `intensity_bev` | 1.000 | 1.000 | 0.540 | 5374.6 | 9.875 | 0.707 | `ok` | `cross_method_suspicious` | `stress_unflagged` | `stress_flagged` | n/a | ok | cross_method_suspicious |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `point_count_stress` | 30 | 0.035 | `kiss_keyframe` | 1.000 | 1.000 | 0.753 | 2643.2 | 0.988 | 0.102 | `ok` | `ok` | `stress_unflagged` | `stress_unflagged` | 2/2 | ok | ok |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `point_count_stress` | 30 | 0.035 | `ct_icp` | 1.000 | 0.103 | n/a | n/a | 5.412 | 0.433 | `degraded` | `degraded` | `stress_flagged` | `stress_flagged` | n/a | low_convergence | low_convergence |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_stress` | 30 | 0.045 | `geometry_icp` | 1.000 | n/a | 0.709 | 536.1 | 1.766 | 0.213 | `ok` | `ok` | `stress_unflagged` | `stress_unflagged` | n/a | ok | ok |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_stress` | 30 | 0.045 | `intensity_bev` | 1.000 | 1.000 | 0.521 | 5114.1 | 12.642 | 0.707 | `ok` | `cross_method_suspicious` | `stress_unflagged` | `stress_flagged` | n/a | ok | cross_method_suspicious |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_stress` | 30 | 0.045 | `kiss_keyframe` | 1.000 | 1.000 | 0.755 | 2675.4 | 0.920 | 0.102 | `ok` | `ok` | `stress_unflagged` | `stress_unflagged` | 2/2 | ok | ok |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_stress` | 30 | 0.045 | `ct_icp` | 1.000 | 0.138 | n/a | n/a | 5.676 | 0.529 | `degraded` | `degraded` | `stress_flagged` | `stress_flagged` | n/a | low_convergence | low_convergence |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `degradation_stress` | 30 | 0.227 | `geometry_icp` | 1.000 | n/a | 0.714 | 656.2 | 1.178 | 0.153 | `ok` | `ok` | `stress_unflagged` | `stress_unflagged` | n/a | ok | ok |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `degradation_stress` | 30 | 0.227 | `intensity_bev` | 1.000 | 1.000 | 0.648 | 5534.3 | 7.561 | 0.354 | `suspicious` | `suspicious` | `stress_flagged` | `stress_flagged` | n/a | motion_margin_dominant | motion_margin_dominant |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `degradation_stress` | 30 | 0.227 | `kiss_keyframe` | 1.000 | 1.000 | 0.745 | 3184.1 | 1.088 | 0.100 | `ok` | `ok` | `stress_unflagged` | `stress_unflagged` | 2/2 | ok | ok |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `degradation_stress` | 30 | 0.227 | `ct_icp` | 1.000 | 0.207 | n/a | n/a | 4.161 | 0.345 | `degraded` | `degraded` | `stress_flagged` | `stress_flagged` | n/a | low_convergence | low_convergence |

## Readout

- `fog_200`: intensity BEV keeps 93.1-100% acceptance on selected windows after zero-motion score-margin preference, including the strongest fog slice.
- `fog_200`: KISS keyframe rejects every selected window, geometry ICP collapses on the strongest fog window, and CT-ICP keeps baseline/tail healthy but drops on strongest fog.
- Failure-awareness columns are heuristic because this dataset layer has no GT: `stress_unflagged` means a stress window stayed externally healthy, not necessarily that the estimate is wrong.
- Intensity BEV false-confidence gates now promote dominant motion-margin decisions and sharp overlap tails to suspicious health flags on non-baseline selected windows.
- Confidence probes expose stress-unflagged windows that need a GT or cross-method check, especially when motion-margin decisions dominate or overlap has a sharp tail.
- Cross-method consistency now contributes to total risk when a stress-unflagged trajectory disagrees with healthy-peer or all-method path medians.
- `tunnel_geom_2700_200`: the short-window checks stay accepted, so this slice is not yet a local-odometry failure case.
- CT-ICP convergence is reported separately from acceptance because this repo's CT-ICP dogfooding path uses gate-accepted refinements even when the internal stopping bit is low.
