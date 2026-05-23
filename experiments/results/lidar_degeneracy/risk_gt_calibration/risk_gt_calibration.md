# LiDAR Degeneracy Risk GT Calibration

Policy: `lidar_degeneracy_triage_v2` (`evaluation/config/lidar_degeneracy_triage_policy.json`)

## GT Availability

| Sequence | GT CSV | Exists | Pose-like bag topics |
| --- | --- | --- | --- |
| `fog_200` | n/a | no | none |
| `tunnel_geom_2700_200` | n/a | no | none |

## GT Metrics

No GT-backed rows were produced. The downloaded NTNU LiDAR degeneracy bags expose LiDAR packets, IMU, trigger topics, and radar PointCloud2, but no pose/odom/tf topic in the current topic audits.

Pass `--gt-csv sequence=path/to/gt.csv` to compute window ATE and step RMSE when an external reference becomes available.

## Proxy Calibration

Stress-window rows only.

| Risk bucket | Windows | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `cross_method_suspicious` | 4 | 7.854 | 4.692 | 7.157 | 2.604 | 4.009 |
| `local_risk` | 7 | 5.684 | 4.433 | 11.714 | 2.231 | 5.832 |
| `ok` | 9 | 1.344 | 0.921 | 1.256 | 0.473 | 0.681 |

## Policy Decisions

Stress-window rows only.

| Decision | Rows | Methods | Mean accepted | Mean converged | Mean path/healthy | Mean path/all |
| --- | ---: | --- | ---: | ---: | ---: | ---: |
| `pass` | 8 | `geometry_icp`, `kiss_keyframe` | 1.000 | 1.000 | 0.964 | 0.495 |
| `watch` | 8 | `ct_icp`, `intensity_bev`, `kiss_keyframe` | 0.948 | 0.422 | 3.951 | 1.989 |
| `investigate` | 4 | `intensity_bev`, `kiss_keyframe` | 1.000 | 1.000 | 4.692 | 2.604 |
| `fail` | 0 | n/a | n/a | n/a | n/a | n/a |

## Reason Drilldown

Stress-window rows only. Rows with multiple active signals appear under each reason.

| Reason | Policy | Rows | Methods | Risk buckets | Mean accepted | Mean converged | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |
| --- | --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `overlap_tail` | `watch` | 1 | `intensity_bev` | `local_risk` | 0.931 | 0.931 | 11.516 | 11.714 | 11.714 | 5.832 | 5.832 |
| `motion_margin_dominant` | `watch` | 2 | `intensity_bev` | `local_risk` | 0.966 | 0.966 | 9.538 | 8.596 | 11.714 | 4.230 | 5.832 |
| `cross_method_suspicious` | `investigate` | 4 | `intensity_bev`, `kiss_keyframe` | `cross_method_suspicious` | 1.000 | 1.000 | 7.854 | 4.692 | 7.157 | 2.604 | 4.009 |
| `path_disagrees_with_healthy_median` | `investigate` | 4 | `intensity_bev`, `kiss_keyframe` | `cross_method_suspicious` | 1.000 | 1.000 | 7.854 | 4.692 | 7.157 | 2.604 | 4.009 |
| `path_disagrees_with_all_method_median` | `watch` | 4 | `intensity_bev`, `kiss_keyframe` | `cross_method_suspicious`, `ok` | 1.000 | 1.000 | 5.653 | 3.513 | 7.157 | 2.001 | 4.009 |
| `low_convergence` | `watch` | 5 | `ct_icp` | `local_risk` | 0.931 | 0.090 | 4.142 | 2.768 | 3.207 | 1.431 | 1.525 |
| `partial_acceptance` | `watch` | 1 | `ct_icp` | `local_risk` | 0.655 | 0.000 | 2.723 | 1.935 | 1.935 | 1.319 | 1.319 |
| `ok_no_risk` | `pass` | 8 | `geometry_icp`, `kiss_keyframe` | `ok` | 1.000 | 1.000 | 1.378 | 0.964 | 1.256 | 0.495 | 0.681 |

## Policy Detail

Stress-window rows only.

| Sequence | Window | Method | Decision | Risk bucket | Accepted | Converged | Path/healthy | Path/all | Reasons |
| --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| `fog_200` | `degraded` 170-199 | `intensity_bev` | `investigate` | `cross_method_suspicious` | 1.000 | 1.000 | 5.884 | 4.009 | `cross_method_suspicious`, `path_disagrees_with_all_method_median`, `path_disagrees_with_healthy_median` |
| `fog_200` | `degraded` 170-199 | `kiss_keyframe` | `investigate` | `cross_method_suspicious` | 1.000 | 1.000 | 0.438 | 0.299 | `cross_method_suspicious`, `path_disagrees_with_all_method_median`, `path_disagrees_with_healthy_median` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `intensity_bev` | `investigate` | `cross_method_suspicious` | 1.000 | 1.000 | 5.290 | 2.705 | `cross_method_suspicious`, `path_disagrees_with_healthy_median` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `intensity_bev` | `investigate` | `cross_method_suspicious` | 1.000 | 1.000 | 7.157 | 3.402 | `cross_method_suspicious`, `path_disagrees_with_all_method_median`, `path_disagrees_with_healthy_median` |
| `fog_200` | `point_count_tail` 115-144 | `ct_icp` | `watch` | `local_risk` | 1.000 | 0.000 | 2.761 | 1.375 | `low_convergence` |
| `fog_200` | `point_count_tail` 115-144 | `intensity_bev` | `watch` | `local_risk` | 0.931 | 0.931 | 11.714 | 5.832 | `motion_margin_dominant`, `overlap_tail` |
| `fog_200` | `degraded` 170-199 | `ct_icp` | `watch` | `local_risk` | 0.655 | 0.000 | 1.935 | 1.319 | `low_convergence`, `partial_acceptance` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `ct_icp` | `watch` | `local_risk` | 1.000 | 0.103 | 2.911 | 1.489 | `low_convergence` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `kiss_keyframe` | `watch` | `ok` | 1.000 | 1.000 | 0.574 | 0.294 | `path_disagrees_with_all_method_median` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `ct_icp` | `watch` | `local_risk` | 1.000 | 0.138 | 3.207 | 1.525 | `low_convergence` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `ct_icp` | `watch` | `local_risk` | 1.000 | 0.207 | 3.025 | 1.450 | `low_convergence` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `intensity_bev` | `watch` | `local_risk` | 1.000 | 1.000 | 5.478 | 2.627 | `motion_margin_dominant` |
| `fog_200` | `point_count_tail` 115-144 | `geometry_icp` | `pass` | `ok` | 1.000 | 1.000 | 1.256 | 0.625 | `ok_no_risk` |
| `fog_200` | `point_count_tail` 115-144 | `kiss_keyframe` | `pass` | `ok` | 1.000 | 1.000 | 0.744 | 0.371 | `ok_no_risk` |
| `fog_200` | `degraded` 170-199 | `geometry_icp` | `pass` | `ok` | 1.000 | 1.000 | 1.000 | 0.681 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `geometry_icp` | `pass` | `ok` | 1.000 | n/a | 1.000 | 0.511 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_icp` | `pass` | `ok` | 1.000 | n/a | 1.000 | 0.475 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `kiss_keyframe` | `pass` | `ok` | 1.000 | 1.000 | 0.713 | 0.339 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `geometry_icp` | `pass` | `ok` | 1.000 | n/a | 0.853 | 0.409 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `kiss_keyframe` | `pass` | `ok` | 1.000 | 1.000 | 1.147 | 0.550 | `ok_no_risk` |

## Readout

- GT calibration is currently blocked by missing public pose/odom/tf for the local NTNU LiDAR degeneracy extraction.
- The comparison remains GT-free for now: local risk and cross-method risk should be treated as triage signals, not error labels.
- Policy decisions are pre-GT triage labels: `fail` for hard local failure, `investigate` for unresolved cross-method disagreement, `watch` for calibrated local confidence downgrades and medium risk, and `pass` for rows with no active risk reason.
- Reason drilldown separates local failure signals from cross-method disagreement so the strongest triage signals can be checked first when GT arrives.
- The script is ready to rerun with external GT via `--gt-csv fog_200=... --gt-csv tunnel_geom_2700_200=...`.
