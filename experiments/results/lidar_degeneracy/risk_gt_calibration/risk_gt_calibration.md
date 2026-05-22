# LiDAR Degeneracy Risk GT Calibration

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
| `cross_method_suspicious` | 3 | 10.266 | 4.482 | 7.157 | 4.063 | 6.081 |
| `local_risk` | 11 | 3.647 | 2.136 | 5.478 | 1.697 | 7.575 |
| `ok` | 6 | 1.454 | 0.881 | 1.147 | 0.430 | 0.550 |

## Policy Decisions

Stress-window rows only.

| Decision | Rows | Methods | Mean accepted | Mean converged | Mean path/healthy | Mean path/all |
| --- | ---: | --- | ---: | ---: | ---: | ---: |
| `pass` | 5 | `geometry_icp`, `kiss_keyframe` | 1.000 | 1.000 | 0.943 | 0.457 |
| `watch` | 6 | `ct_icp`, `kiss_keyframe` | 0.943 | 0.241 | 2.009 | 1.424 |
| `investigate` | 5 | `intensity_bev` | 0.986 | 0.986 | 4.731 | 4.478 |
| `fail` | 4 | `geometry_icp`, `kiss_keyframe` | 0.034 | 0.000 | 0.000 | 0.054 |

## Reason Drilldown

Stress-window rows only. Rows with multiple active signals appear under each reason.

| Reason | Policy | Rows | Methods | Risk buckets | Mean accepted | Mean converged | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |
| --- | --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `path_disagrees_with_healthy_median` | `investigate` | 2 | `intensity_bev` | `cross_method_suspicious` | 1.000 | 1.000 | 11.258 | 6.223 | 7.157 | 3.054 | 3.402 |
| `motion_margin_dominant` | `investigate` | 2 | `intensity_bev` | `local_risk` | 0.966 | 0.966 | 9.538 | 5.478 | 5.478 | 5.101 | 7.575 |
| `cross_method_suspicious` | `investigate` | 3 | `intensity_bev` | `cross_method_suspicious` | 1.000 | 1.000 | 10.266 | 4.482 | 7.157 | 4.063 | 6.081 |
| `path_disagrees_with_all_method_median` | `watch` | 3 | `intensity_bev`, `kiss_keyframe` | `cross_method_suspicious`, `ok` | 1.000 | 1.000 | 7.331 | 2.910 | 7.157 | 3.259 | 6.081 |
| `low_convergence` | `watch` | 7 | `ct_icp`, `kiss_keyframe` | `local_risk` | 0.665 | 0.064 | 2.959 | 1.894 | 3.207 | 1.178 | 2.000 |
| `no_healthy_peer` | `watch` | 1 | `intensity_bev` | `cross_method_suspicious` | 1.000 | 1.000 | 8.281 | 1.000 | 1.000 | 6.081 | 6.081 |
| `ok_no_risk` | `pass` | 5 | `geometry_icp`, `kiss_keyframe` | `ok` | 1.000 | 1.000 | 1.531 | 0.943 | 1.147 | 0.457 | 0.550 |
| `partial_acceptance` | `watch` | 1 | `ct_icp` | `local_risk` | 0.655 | 0.000 | 2.723 | 0.329 | 0.329 | 2.000 | 2.000 |
| `overlap_tail` | `investigate` | 1 | `intensity_bev` | `local_risk` | 0.931 | 0.931 | 11.516 | n/a | n/a | 7.575 | 7.575 |
| `low_acceptance` | `fail` | 4 | `geometry_icp`, `kiss_keyframe` | `local_risk` | 0.034 | 0.000 | 0.081 | 0.000 | 0.000 | 0.054 | 0.214 |
| `all_pairs_failed` | `fail` | 3 | `geometry_icp`, `kiss_keyframe` | `local_risk` | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 |

## Policy Detail

Stress-window rows only.

| Sequence | Window | Method | Decision | Risk bucket | Accepted | Converged | Path/healthy | Path/all | Reasons |
| --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| `fog_200` | `point_count_tail` 115-144 | `geometry_icp` | `fail` | `local_risk` | 0.138 | n/a | n/a | 0.214 | `low_acceptance` |
| `fog_200` | `point_count_tail` 115-144 | `kiss_keyframe` | `fail` | `local_risk` | 0.000 | 0.000 | n/a | 0.000 | `all_pairs_failed`, `low_acceptance`, `low_convergence` |
| `fog_200` | `degraded` 170-199 | `geometry_icp` | `fail` | `local_risk` | 0.000 | n/a | 0.000 | 0.000 | `all_pairs_failed`, `low_acceptance` |
| `fog_200` | `degraded` 170-199 | `kiss_keyframe` | `fail` | `local_risk` | 0.000 | 0.000 | 0.000 | 0.000 | `all_pairs_failed`, `low_acceptance`, `low_convergence` |
| `fog_200` | `point_count_tail` 115-144 | `intensity_bev` | `investigate` | `local_risk` | 0.931 | 0.931 | n/a | 7.575 | `motion_margin_dominant`, `overlap_tail` |
| `fog_200` | `degraded` 170-199 | `intensity_bev` | `investigate` | `cross_method_suspicious` | 1.000 | 1.000 | 1.000 | 6.081 | `cross_method_suspicious`, `no_healthy_peer`, `path_disagrees_with_all_method_median` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `intensity_bev` | `investigate` | `cross_method_suspicious` | 1.000 | 1.000 | 5.290 | 2.705 | `cross_method_suspicious`, `path_disagrees_with_healthy_median` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `intensity_bev` | `investigate` | `cross_method_suspicious` | 1.000 | 1.000 | 7.157 | 3.402 | `cross_method_suspicious`, `path_disagrees_with_all_method_median`, `path_disagrees_with_healthy_median` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `intensity_bev` | `investigate` | `local_risk` | 1.000 | 1.000 | 5.478 | 2.627 | `motion_margin_dominant` |
| `fog_200` | `point_count_tail` 115-144 | `ct_icp` | `watch` | `local_risk` | 1.000 | 0.000 | n/a | 1.786 | `low_convergence` |
| `fog_200` | `degraded` 170-199 | `ct_icp` | `watch` | `local_risk` | 0.655 | 0.000 | 0.329 | 2.000 | `low_convergence`, `partial_acceptance` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `ct_icp` | `watch` | `local_risk` | 1.000 | 0.103 | 2.911 | 1.489 | `low_convergence` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `kiss_keyframe` | `watch` | `ok` | 1.000 | 1.000 | 0.574 | 0.294 | `path_disagrees_with_all_method_median` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `ct_icp` | `watch` | `local_risk` | 1.000 | 0.138 | 3.207 | 1.525 | `low_convergence` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `ct_icp` | `watch` | `local_risk` | 1.000 | 0.207 | 3.025 | 1.450 | `low_convergence` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `geometry_icp` | `pass` | `ok` | 1.000 | n/a | 1.000 | 0.511 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_icp` | `pass` | `ok` | 1.000 | n/a | 1.000 | 0.475 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `kiss_keyframe` | `pass` | `ok` | 1.000 | 1.000 | 0.713 | 0.339 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `geometry_icp` | `pass` | `ok` | 1.000 | n/a | 0.853 | 0.409 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `kiss_keyframe` | `pass` | `ok` | 1.000 | 1.000 | 1.147 | 0.550 | `ok_no_risk` |

## Readout

- GT calibration is currently blocked by missing public pose/odom/tf for the local NTNU LiDAR degeneracy extraction.
- The comparison remains GT-free for now: local risk and cross-method risk should be treated as triage signals, not error labels.
- Policy decisions are pre-GT triage labels: `fail` for hard local failure, `investigate` for strong false-confidence signals, `watch` for medium risk, and `pass` for rows with no active risk reason.
- Reason drilldown separates local failure signals from cross-method disagreement so the strongest triage signals can be checked first when GT arrives.
- The script is ready to rerun with external GT via `--gt-csv fog_200=... --gt-csv tunnel_geom_2700_200=...`.
