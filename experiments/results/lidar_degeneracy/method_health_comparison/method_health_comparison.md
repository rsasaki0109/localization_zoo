# LiDAR Degeneracy Method Health Comparison

Policy: `lidar_degeneracy_triage_v4` (`evaluation/config/lidar_degeneracy_triage_policy.json`)

## Method Aggregate

| Sequence | Method | Windows | Mean accepted | Min accepted | Mean converged | CT gate | CT iter | Failed windows | Diagnostic watch | Local risk | Cross-method risk | Total risk | Pass | Watch | Investigate | Fail | Max used path m |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `fog_200` | `geometry_icp` | 3 | 1.000 | 1.000 | 1.000 | n/a | n/a | 0 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 1.407 |
| `fog_200` | `intensity_bev` | 3 | 1.000 | 1.000 | 1.000 | n/a | n/a | 0 | 0 | 2 | 0 | 2 | 1 | 2 | 0 | 0 | 7.250 |
| `fog_200` | `kiss_keyframe` | 3 | 1.000 | 1.000 | 1.000 | n/a | n/a | 0 | 0 | 0 | 0 | 0 | 2 | 1 | 0 | 0 | 0.743 |
| `fog_200` | `ct_icp` | 3 | 0.885 | 0.655 | 0.046 | 0.885 | 7.920 | 0 | 2 | 1 | 0 | 1 | 0 | 3 | 0 | 0 | 2.703 |
| `tunnel_geom_2700_200` | `geometry_icp` | 4 | 1.000 | 1.000 | 1.000 | n/a | n/a | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 1.867 |
| `tunnel_geom_2700_200` | `intensity_bev` | 4 | 1.000 | 1.000 | 1.000 | n/a | n/a | 0 | 0 | 3 | 0 | 3 | 1 | 3 | 0 | 0 | 7.975 |
| `tunnel_geom_2700_200` | `kiss_keyframe` | 4 | 1.000 | 1.000 | 1.000 | n/a | n/a | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 1.186 |
| `tunnel_geom_2700_200` | `ct_icp` | 4 | 1.000 | 1.000 | 0.147 | 1.000 | 7.793 | 0 | 4 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 5.676 |

## Failure Awareness

| Sequence | Method | Stress windows | Local flagged | Cross-method flagged | Residual unflagged | False alarms | Diagnostic watch |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `fog_200` | `geometry_icp` | 2 | 0 | 0 | 2 | 0 | 0 |
| `fog_200` | `intensity_bev` | 2 | 2 | 0 | 0 | 0 | 0 |
| `fog_200` | `kiss_keyframe` | 2 | 0 | 0 | 2 | 0 | 0 |
| `fog_200` | `ct_icp` | 2 | 1 | 0 | 0 | 0 | 2 |
| `tunnel_geom_2700_200` | `geometry_icp` | 3 | 0 | 0 | 3 | 0 | 0 |
| `tunnel_geom_2700_200` | `intensity_bev` | 3 | 3 | 0 | 0 | 0 | 0 |
| `tunnel_geom_2700_200` | `kiss_keyframe` | 3 | 0 | 0 | 3 | 0 | 0 |
| `tunnel_geom_2700_200` | `ct_icp` | 3 | 0 | 0 | 0 | 0 | 4 |

## Confidence Probes

| Sequence | Window | Method | Accepted | Score min | Overlap min | Motion-margin rate | Decision reasons | Probe |
| --- | --- | --- | ---: | ---: | ---: | ---: | --- | --- |
| `fog_200` | `point_count_tail` 115-144 | `geometry_icp` | 1.000 | 0.726 | 65.0 | 0.000 | accepted:29 | needs_gt_or_cross_method_check |
| `fog_200` | `point_count_tail` 115-144 | `kiss_keyframe` | 1.000 | 0.758 | 207.0 | 0.000 | accepted:29 | needs_gt_or_cross_method_check |
| `fog_200` | `degraded` 170-199 | `geometry_icp` | 1.000 | 0.750 | 53.0 | 0.000 | accepted:29 | needs_gt_or_cross_method_check |
| `fog_200` | `degraded` 170-199 | `kiss_keyframe` | 1.000 | 0.767 | 149.0 | 0.000 | accepted:29 | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `geometry_icp` | 1.000 | 0.694 | 495.0 | n/a | n/a | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `kiss_keyframe` | 1.000 | 0.718 | 2554.0 | n/a | n/a | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_icp` | 1.000 | 0.694 | 512.0 | n/a | n/a | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `kiss_keyframe` | 1.000 | 0.718 | 2628.0 | n/a | n/a | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `geometry_icp` | 1.000 | 0.697 | 607.0 | n/a | n/a | needs_gt_or_cross_method_check |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `kiss_keyframe` | 1.000 | 0.707 | 2957.0 | n/a | n/a | needs_gt_or_cross_method_check |

## Cross-Method Consistency

| Sequence | Window | Method | Path m | Net m | Yaw deg | Healthy peers | Healthy median path m | Path/healthy | All median path m | Path/all | Cross risk | Probe |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |
| `fog_200` | `point_count_tail` 115-144 | `geometry_icp` | 1.234 | 0.431 | 6.259 | 1 | 0.983 | 1.256 | 0.983 | 1.256 | no | none |
| `fog_200` | `point_count_tail` 115-144 | `kiss_keyframe` | 0.732 | 0.382 | 1.892 | 1 | 0.983 | 0.744 | 0.983 | 0.744 | no | none |
| `fog_200` | `degraded` 170-199 | `geometry_icp` | 1.407 | 0.681 | 2.851 | 1 | 1.012 | 1.391 | 2.065 | 0.681 | no | none |
| `fog_200` | `degraded` 170-199 | `kiss_keyframe` | 0.617 | 0.280 | 0.668 | 1 | 1.012 | 0.609 | 2.065 | 0.299 | no | path_disagrees_with_all_method_median |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `geometry_icp` | 1.867 | 0.123 | 2.392 | 1 | 1.469 | 1.270 | 1.469 | 1.270 | no | none |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `kiss_keyframe` | 1.072 | 0.398 | 0.072 | 1 | 1.469 | 0.730 | 1.469 | 0.730 | no | none |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_icp` | 1.766 | 0.596 | 2.385 | 1 | 1.513 | 1.168 | 1.513 | 1.168 | no | none |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `kiss_keyframe` | 1.259 | 0.098 | 0.664 | 1 | 1.513 | 0.832 | 1.513 | 0.832 | no | none |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `geometry_icp` | 1.178 | 0.311 | 1.798 | 1 | 1.380 | 0.853 | 1.380 | 0.853 | no | none |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `kiss_keyframe` | 1.582 | 1.005 | 1.184 | 1 | 1.380 | 1.147 | 1.380 | 1.147 | no | none |

## Diagnostic Watch Rows

| Sequence | Window | Expected | Method | Accepted | Converged | CT gate | CT iter | Path/healthy | Path/all | Clear? | Action | Blockers | Policy reasons |
| --- | --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- | --- | --- |
| `fog_200` | `baseline` 0-29 | `nominal` | `ct_icp` | 1.000 | 0.138 | 1.000 | 7.759 | 0.947 | 0.973 | no | `optimizer_retry` | iterations_pinned | ct_icp_internal_convergence_low |
| `fog_200` | `point_count_tail` 115-144 | `obscurant_stress` | `ct_icp` | 1.000 | 0.000 | 1.000 | 8.000 | 2.761 | 2.761 | no | `fallback_required` | iterations_pinned, path_vs_healthy_high | ct_icp_internal_convergence_low |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `nominal` | `ct_icp` | 1.000 | 0.138 | 1.000 | 8.000 | 3.411 | 1.547 | no | `fallback_required` | iterations_pinned, path_vs_healthy_high | ct_icp_internal_convergence_low |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `point_count_stress` | `ct_icp` | 1.000 | 0.103 | 1.000 | 7.793 | 3.699 | 3.699 | no | `fallback_required` | iterations_pinned, path_vs_healthy_high, path_vs_all_high | ct_icp_internal_convergence_low |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_stress` | `ct_icp` | 1.000 | 0.138 | 1.000 | 7.724 | 3.745 | 3.745 | no | `fallback_required` | iterations_pinned, path_vs_healthy_high, path_vs_all_high | ct_icp_internal_convergence_low |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `degradation_stress` | `ct_icp` | 1.000 | 0.207 | 1.000 | 7.655 | 3.025 | 3.025 | no | `fallback_required` | iterations_pinned, path_vs_healthy_high, path_vs_all_high | ct_icp_internal_convergence_low |

## CT-ICP Production Guard

Guard counts: `fallback_to_prior`:5, `reject_or_retry`:1, `retry_optimizer`:1

| Sequence | Window | Expected | State | Risk state | Accepted | Watch action | Guard decision | Uses refined? | Guard reasons |
| --- | --- | --- | --- | --- | ---: | --- | --- | --- | --- |
| `fog_200` | `baseline` 0-29 | `nominal` | `diagnostic_watch` | `diagnostic_watch` | 1.000 | `optimizer_retry` | `retry_optimizer` | no | iterations_pinned |
| `fog_200` | `point_count_tail` 115-144 | `obscurant_stress` | `diagnostic_watch` | `diagnostic_watch` | 1.000 | `fallback_required` | `fallback_to_prior` | no | path_reference_disagreement |
| `fog_200` | `degraded` 170-199 | `obscurant_stress` | `suspicious` | `suspicious` | 0.655 | `n/a` | `reject_or_retry` | no | partial_acceptance |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `nominal` | `diagnostic_watch` | `diagnostic_watch` | 1.000 | `fallback_required` | `fallback_to_prior` | no | path_reference_disagreement |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `point_count_stress` | `diagnostic_watch` | `diagnostic_watch` | 1.000 | `fallback_required` | `fallback_to_prior` | no | path_reference_disagreement |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_stress` | `diagnostic_watch` | `diagnostic_watch` | 1.000 | `fallback_required` | `fallback_to_prior` | no | path_reference_disagreement |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `degradation_stress` | `diagnostic_watch` | `diagnostic_watch` | 1.000 | `fallback_required` | `fallback_to_prior` | no | path_reference_disagreement |

## Window Detail

| Sequence | Window | Expected | Frames | Obscurant | Method | Accepted | Converged | CT gate | CT iter | Score | Overlap | Used path m | Max step m | State | Risk state | Policy | Failure awareness | Risk awareness | Keyframes | Flags | Risk flags | Policy reasons |
| --- | --- | --- | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `fog_200` | `baseline` 0-29 | `nominal` | 30 | 0.006 | `geometry_icp` | 1.000 | 1.000 | n/a | n/a | 0.777 | 82.0 | 1.363 | 0.099 | `ok` | `ok` | `pass` | `nominal_ok` | `nominal_ok` | n/a | ok | ok | ok_no_risk |
| `fog_200` | `baseline` 0-29 | `nominal` | 30 | 0.006 | `intensity_bev` | 1.000 | 1.000 | n/a | n/a | 0.606 | 5852.6 | 7.250 | 0.250 | `ok` | `ok` | `pass` | `nominal_ok` | `nominal_ok` | n/a | ok | ok | ok_no_risk |
| `fog_200` | `baseline` 0-29 | `nominal` | 30 | 0.006 | `kiss_keyframe` | 1.000 | 1.000 | n/a | n/a | 0.845 | 309.0 | 0.369 | 0.024 | `ok` | `ok` | `pass` | `nominal_ok` | `nominal_ok` | 0/2 | ok | ok | ok_no_risk |
| `fog_200` | `baseline` 0-29 | `nominal` | 30 | 0.006 | `ct_icp` | 1.000 | 0.138 | 1.000 | 7.759 | n/a | n/a | 1.283 | 0.126 | `diagnostic_watch` | `diagnostic_watch` | `watch` | `diagnostic_watch` | `diagnostic_watch` | n/a | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low |
| `fog_200` | `point_count_tail` 115-144 | `obscurant_stress` | 30 | 0.521 | `geometry_icp` | 1.000 | 1.000 | n/a | n/a | 0.791 | 73.4 | 1.234 | 0.091 | `ok` | `ok` | `pass` | `stress_unflagged` | `stress_unflagged` | n/a | ok | ok | ok_no_risk |
| `fog_200` | `point_count_tail` 115-144 | `obscurant_stress` | 30 | 0.521 | `intensity_bev` | 1.000 | 1.000 | n/a | n/a | 0.740 | 5971.3 | 0.000 | 0.000 | `suspicious` | `suspicious` | `watch` | `stress_flagged` | `stress_flagged` | n/a | low_used_path, low_motion_margin_dominant | low_used_path, low_motion_margin_dominant | low_motion_margin_dominant, low_used_path |
| `fog_200` | `point_count_tail` 115-144 | `obscurant_stress` | 30 | 0.521 | `kiss_keyframe` | 1.000 | 1.000 | n/a | n/a | 0.802 | 239.4 | 0.743 | 0.083 | `ok` | `ok` | `pass` | `stress_unflagged` | `stress_unflagged` | 0/2 | ok | ok | ok_no_risk |
| `fog_200` | `point_count_tail` 115-144 | `obscurant_stress` | 30 | 0.521 | `ct_icp` | 1.000 | 0.000 | 1.000 | 8.000 | n/a | n/a | 2.703 | 0.323 | `diagnostic_watch` | `diagnostic_watch` | `watch` | `diagnostic_watch` | `diagnostic_watch` | n/a | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low |
| `fog_200` | `degraded` 170-199 | `obscurant_stress` | 30 | 0.911 | `geometry_icp` | 1.000 | 1.000 | n/a | n/a | 0.813 | 59.3 | 1.407 | 0.122 | `ok` | `ok` | `pass` | `stress_unflagged` | `stress_unflagged` | n/a | ok | ok | ok_no_risk |
| `fog_200` | `degraded` 170-199 | `obscurant_stress` | 30 | 0.911 | `intensity_bev` | 1.000 | 1.000 | n/a | n/a | 0.797 | 5695.7 | 4.177 | 0.559 | `suspicious` | `suspicious` | `watch` | `stress_flagged` | `stress_flagged` | n/a | low_motion_margin_dominant | low_motion_margin_dominant | low_motion_margin_dominant |
| `fog_200` | `degraded` 170-199 | `obscurant_stress` | 30 | 0.911 | `kiss_keyframe` | 1.000 | 1.000 | n/a | n/a | 0.797 | 163.3 | 0.620 | 0.050 | `ok` | `ok` | `watch` | `stress_unflagged` | `stress_unflagged` | 0/2 | ok | ok | path_disagrees_with_all_method_median |
| `fog_200` | `degraded` 170-199 | `obscurant_stress` | 30 | 0.911 | `ct_icp` | 0.655 | 0.000 | 0.655 | 8.000 | n/a | n/a | 2.655 | 0.229 | `suspicious` | `suspicious` | `watch` | `stress_flagged` | `stress_flagged` | n/a | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low, partial_acceptance |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `nominal` | 30 | 0.002 | `geometry_icp` | 1.000 | n/a | n/a | n/a | 0.704 | 472.6 | 1.519 | 0.194 | `ok` | `ok` | `pass` | `nominal_ok` | `nominal_ok` | n/a | ok | ok | ok_no_risk |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `nominal` | 30 | 0.002 | `intensity_bev` | 1.000 | 1.000 | n/a | n/a | 0.535 | 5522.3 | 7.975 | 0.354 | `ok` | `ok` | `pass` | `nominal_ok` | `nominal_ok` | n/a | ok | ok | ok_no_risk |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `nominal` | 30 | 0.002 | `kiss_keyframe` | 1.000 | 1.000 | n/a | n/a | 0.747 | 2255.8 | 1.186 | 0.100 | `ok` | `ok` | `pass` | `nominal_ok` | `nominal_ok` | 2/2 | ok | ok | ok_no_risk |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `nominal` | 30 | 0.002 | `ct_icp` | 1.000 | 0.138 | 1.000 | 8.000 | n/a | n/a | 5.189 | 0.842 | `diagnostic_watch` | `diagnostic_watch` | `watch` | `diagnostic_watch` | `diagnostic_watch` | n/a | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `point_count_stress` | 30 | 0.035 | `geometry_icp` | 1.000 | n/a | n/a | n/a | 0.706 | 525.0 | 1.867 | 0.213 | `ok` | `ok` | `pass` | `stress_unflagged` | `stress_unflagged` | n/a | ok | ok | ok_no_risk |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `point_count_stress` | 30 | 0.035 | `intensity_bev` | 1.000 | 1.000 | n/a | n/a | 0.538 | 5675.9 | 0.000 | 0.000 | `suspicious` | `suspicious` | `watch` | `stress_flagged` | `stress_flagged` | n/a | low_used_path, low_motion_margin_dominant | low_used_path, low_motion_margin_dominant | low_motion_margin_dominant, low_used_path |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `point_count_stress` | 30 | 0.035 | `kiss_keyframe` | 1.000 | 1.000 | n/a | n/a | 0.753 | 2643.2 | 0.988 | 0.102 | `ok` | `ok` | `pass` | `stress_unflagged` | `stress_unflagged` | 2/2 | ok | ok | ok_no_risk |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `point_count_stress` | 30 | 0.035 | `ct_icp` | 1.000 | 0.103 | 1.000 | 7.793 | n/a | n/a | 5.412 | 0.433 | `diagnostic_watch` | `diagnostic_watch` | `watch` | `diagnostic_watch` | `diagnostic_watch` | n/a | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_stress` | 30 | 0.045 | `geometry_icp` | 1.000 | n/a | n/a | n/a | 0.709 | 536.1 | 1.766 | 0.213 | `ok` | `ok` | `pass` | `stress_unflagged` | `stress_unflagged` | n/a | ok | ok | ok_no_risk |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_stress` | 30 | 0.045 | `intensity_bev` | 1.000 | 1.000 | n/a | n/a | 0.515 | 5641.0 | 0.750 | 0.250 | `suspicious` | `suspicious` | `watch` | `stress_flagged` | `stress_flagged` | n/a | low_motion_margin_dominant | low_motion_margin_dominant | low_motion_margin_dominant |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_stress` | 30 | 0.045 | `kiss_keyframe` | 1.000 | 1.000 | n/a | n/a | 0.755 | 2675.4 | 0.920 | 0.102 | `ok` | `ok` | `pass` | `stress_unflagged` | `stress_unflagged` | 2/2 | ok | ok | ok_no_risk |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_stress` | 30 | 0.045 | `ct_icp` | 1.000 | 0.138 | 1.000 | 7.724 | n/a | n/a | 5.676 | 0.529 | `diagnostic_watch` | `diagnostic_watch` | `watch` | `diagnostic_watch` | `diagnostic_watch` | n/a | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `degradation_stress` | 30 | 0.227 | `geometry_icp` | 1.000 | n/a | n/a | n/a | 0.714 | 656.2 | 1.178 | 0.153 | `ok` | `ok` | `pass` | `stress_unflagged` | `stress_unflagged` | n/a | ok | ok | ok_no_risk |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `degradation_stress` | 30 | 0.227 | `intensity_bev` | 1.000 | 1.000 | n/a | n/a | 0.654 | 5632.7 | 0.000 | 0.000 | `suspicious` | `suspicious` | `watch` | `stress_flagged` | `stress_flagged` | n/a | low_used_path, low_motion_margin_dominant | low_used_path, low_motion_margin_dominant | low_motion_margin_dominant, low_used_path |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `degradation_stress` | 30 | 0.227 | `kiss_keyframe` | 1.000 | 1.000 | n/a | n/a | 0.745 | 3184.1 | 1.088 | 0.100 | `ok` | `ok` | `pass` | `stress_unflagged` | `stress_unflagged` | 2/2 | ok | ok | ok_no_risk |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `degradation_stress` | 30 | 0.227 | `ct_icp` | 1.000 | 0.207 | 1.000 | 7.655 | n/a | n/a | 4.161 | 0.345 | `diagnostic_watch` | `diagnostic_watch` | `watch` | `diagnostic_watch` | `diagnostic_watch` | n/a | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low | ct_icp_internal_convergence_low |

## Readout

- `fog_200`: intensity BEV keeps 100% acceptance on selected windows, while the stress-only low-motion regularizer downgrades ambiguous stress trajectories instead of letting weak score gains accumulate into accepted drift.
- `fog_200`: geometry ICP and KISS keyframe now accept every selected short window after the fog/crop correspondence gates were relaxed; CT-ICP keeps baseline/tail accepted but drops on strongest fog.
- Failure-awareness columns are heuristic because this dataset layer has no GT: `stress_unflagged` means a stress window stayed externally healthy, not necessarily that the estimate is wrong.
- Intensity BEV false-confidence gates now promote dominant motion-margin decisions, dominant low-motion regularization, and sharp overlap tails to suspicious health flags on non-baseline selected windows.
- Confidence probes expose stress-unflagged windows that need a GT or cross-method check, especially when score-margin decisions dominate or overlap has a sharp tail.
- Cross-method consistency now contributes to total risk when a stress-unflagged trajectory disagrees with healthy-peer or all-method path medians.
- Policy decisions are GT-free triage labels: `fail` for hard local failure, `investigate` for unresolved cross-method disagreement, `watch` for calibrated local confidence downgrades and medium-risk rows, and `pass` for no active risk reason.
- `tunnel_geom_2700_200`: the short-window checks stay accepted, so this slice is not yet a local-odometry failure case.
- CT-ICP internal convergence watch is reported as `diagnostic_watch` when it is the only active signal; accepted refinements with no product-health downgrade no longer count as false alarms.
