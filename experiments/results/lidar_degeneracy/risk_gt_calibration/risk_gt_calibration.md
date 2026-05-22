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

## Reason Drilldown

Stress-window rows only. Rows with multiple active signals appear under each reason.

| Reason | Rows | Methods | Risk buckets | Mean accepted | Mean converged | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |
| --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `path_disagrees_with_healthy_median` | 2 | `intensity_bev` | `cross_method_suspicious` | 1.000 | 1.000 | 11.258 | 6.223 | 7.157 | 3.054 | 3.402 |
| `motion_margin_dominant` | 2 | `intensity_bev` | `local_risk` | 0.966 | 0.966 | 9.538 | 5.478 | 5.478 | 5.101 | 7.575 |
| `cross_method_suspicious` | 3 | `intensity_bev` | `cross_method_suspicious` | 1.000 | 1.000 | 10.266 | 4.482 | 7.157 | 4.063 | 6.081 |
| `path_disagrees_with_all_method_median` | 3 | `intensity_bev`, `kiss_keyframe` | `cross_method_suspicious`, `ok` | 1.000 | 1.000 | 7.331 | 2.910 | 7.157 | 3.259 | 6.081 |
| `low_convergence` | 7 | `ct_icp`, `kiss_keyframe` | `local_risk` | 0.665 | 0.064 | 2.959 | 1.894 | 3.207 | 1.178 | 2.000 |
| `no_healthy_peer` | 1 | `intensity_bev` | `cross_method_suspicious` | 1.000 | 1.000 | 8.281 | 1.000 | 1.000 | 6.081 | 6.081 |
| `ok_no_risk` | 5 | `geometry_icp`, `kiss_keyframe` | `ok` | 1.000 | 1.000 | 1.531 | 0.943 | 1.147 | 0.457 | 0.550 |
| `overlap_tail` | 1 | `intensity_bev` | `local_risk` | 0.931 | 0.931 | 11.516 | n/a | n/a | 7.575 | 7.575 |
| `low_acceptance` | 4 | `geometry_icp`, `kiss_keyframe` | `local_risk` | 0.034 | 0.000 | 0.081 | 0.000 | 0.000 | 0.054 | 0.214 |
| `all_pairs_failed` | 3 | `geometry_icp`, `kiss_keyframe` | `local_risk` | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 |

## Readout

- GT calibration is currently blocked by missing public pose/odom/tf for the local NTNU LiDAR degeneracy extraction.
- The comparison remains GT-free for now: local risk and cross-method risk should be treated as triage signals, not error labels.
- Reason drilldown separates local failure signals from cross-method disagreement so the strongest triage signals can be checked first when GT arrives.
- The script is ready to rerun with external GT via `--gt-csv fog_200=... --gt-csv tunnel_geom_2700_200=...`.
