# LiDAR Degeneracy Risk GT Calibration

Policy: `lidar_degeneracy_triage_v4` (`evaluation/config/lidar_degeneracy_triage_policy.json`)

## GT Availability

| Sequence | GT CSV | Exists | Pose-like bag topics |
| --- | --- | --- | --- |
| `tunnel_full` | n/a | no | none |
| `fog_full` | n/a | no | none |

## GT Metrics

No GT-backed rows were produced. The downloaded NTNU LiDAR degeneracy bags expose LiDAR packets, IMU, trigger topics, and radar PointCloud2, but no pose/odom/tf topic in the current topic audits.

Pass `--gt-csv sequence=path/to/gt.csv` to compute window ATE and step RMSE when an external reference becomes available.

## Proxy Calibration

Stress-window rows only.

| Risk bucket | Windows | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `local_risk` | 4 | 62.149 | 0.304 | 0.860 | 0.427 | 1.215 |
| `ok` | 12 | 314.803 | 1.021 | 2.218 | 1.217 | 2.346 |

## Policy Decisions

Stress-window rows only.

| Decision | Rows | Methods | Mean accepted | Mean converged | Mean path/healthy | Mean path/all |
| --- | ---: | --- | ---: | ---: | ---: | ---: |
| `pass` | 8 | `degen_sense_imu`, `degen_sense_no_imu`, `kiss_keyframe`, `litamin2`, `relead_no_imu`, `xicp` | 0.999 | 0.950 | 0.860 | 1.044 |
| `watch` | 1 | `ct_icp` | 1.000 | 0.119 | 1.274 | 1.347 |
| `investigate` | 3 | `degen_sense_no_imu`, `kiss_keyframe`, `relead_no_imu` | 0.997 | 0.981 | 1.367 | 1.635 |
| `fail` | 4 | `ct_icp`, `relead_imu`, `xicp` | 0.175 | 0.132 | 0.304 | 0.427 |

## Reason Drilldown

Stress-window rows only. Rows with multiple active signals appear under each reason.

| Reason | Policy | Rows | Methods | Risk buckets | Mean accepted | Mean converged | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |
| --- | --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `ok_no_risk` | `pass` | 8 | `degen_sense_imu`, `degen_sense_no_imu`, `kiss_keyframe`, `litamin2`, `relead_no_imu`, `xicp` | `ok` | 0.999 | 0.950 | 255.867 | 0.860 | 1.227 | 1.044 | 1.491 |
| `cross_method_suspicious` | `investigate` | 6 | `ct_icp`, `degen_sense_no_imu`, `kiss_keyframe`, `relead_imu`, `relead_no_imu` | `local_risk`, `ok` | 0.536 | 0.499 | 221.808 | 0.743 | 2.218 | 0.900 | 2.346 |
| `path_disagrees_with_all_method_median` | `watch` | 6 | `ct_icp`, `degen_sense_no_imu`, `kiss_keyframe`, `relead_imu`, `relead_no_imu` | `local_risk`, `ok` | 0.536 | 0.499 | 221.808 | 0.743 | 2.218 | 0.900 | 2.346 |
| `low_convergence` | `watch` | 5 | `ct_icp`, `relead_imu`, `xicp` | `local_risk`, `ok` | 0.340 | 0.129 | 144.662 | 0.498 | 1.274 | 0.611 | 1.347 |
| `low_acceptance` | `fail` | 4 | `ct_icp`, `relead_imu`, `xicp` | `local_risk` | 0.175 | 0.132 | 62.149 | 0.304 | 0.860 | 0.427 | 1.215 |

## Policy Detail

Stress-window rows only.

| Sequence | Window | Method | Decision | Risk bucket | Watch action | Guard decision | Accepted | Converged | Path/healthy | Path/all | Reasons |
| --- | --- | --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| `fog_full` | `full` 0-1728 | `ct_icp` | `fail` | `local_risk` | `n/a` | `n/a` | 0.211 | 0.051 | 0.302 | 0.426 | `cross_method_suspicious`, `low_acceptance`, `low_convergence`, `path_disagrees_with_all_method_median` |
| `fog_full` | `full` 0-1728 | `relead_imu` | `fail` | `local_risk` | `n/a` | `n/a` | 0.011 | 0.000 | 0.032 | 0.045 | `cross_method_suspicious`, `low_acceptance`, `low_convergence`, `path_disagrees_with_all_method_median` |
| `fog_full` | `full` 0-1728 | `xicp` | `fail` | `local_risk` | `n/a` | `n/a` | 0.473 | 0.476 | 0.860 | 1.215 | `low_acceptance`, `low_convergence` |
| `tunnel_full` | `full` 0-3240 | `relead_imu` | `fail` | `local_risk` | `n/a` | `n/a` | 0.003 | 0.000 | 0.020 | 0.021 | `cross_method_suspicious`, `low_acceptance`, `low_convergence`, `path_disagrees_with_all_method_median` |
| `fog_full` | `full` 0-1728 | `degen_sense_no_imu` | `investigate` | `ok` | `n/a` | `n/a` | 0.990 | 1.000 | 1.599 | 2.258 | `cross_method_suspicious`, `path_disagrees_with_all_method_median` |
| `tunnel_full` | `full` 0-3240 | `kiss_keyframe` | `investigate` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 0.285 | 0.301 | `cross_method_suspicious`, `path_disagrees_with_all_method_median` |
| `tunnel_full` | `full` 0-3240 | `relead_no_imu` | `investigate` | `ok` | `n/a` | `n/a` | 1.000 | 0.944 | 2.218 | 2.346 | `cross_method_suspicious`, `path_disagrees_with_all_method_median` |
| `tunnel_full` | `full` 0-3240 | `ct_icp` | `watch` | `ok` | `n/a` | `n/a` | 1.000 | 0.119 | 1.274 | 1.347 | `low_convergence` |
| `fog_full` | `full` 0-1728 | `degen_sense_imu` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 1.056 | 1.491 | `ok_no_risk` |
| `fog_full` | `full` 0-1728 | `kiss_keyframe` | `pass` | `ok` | `n/a` | `n/a` | 0.993 | 0.993 | 0.418 | 0.590 | `ok_no_risk` |
| `fog_full` | `full` 0-1728 | `litamin2` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 0.556 | 0.785 | `ok_no_risk` |
| `fog_full` | `full` 0-1728 | `relead_no_imu` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 0.819 | 1.000 | 1.412 | `ok_no_risk` |
| `tunnel_full` | `full` 0-3240 | `degen_sense_imu` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 1.000 | 1.058 | `ok_no_risk` |
| `tunnel_full` | `full` 0-3240 | `degen_sense_no_imu` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 1.227 | 1.298 | `ok_no_risk` |
| `tunnel_full` | `full` 0-3240 | `litamin2` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 0.733 | 0.775 | `ok_no_risk` |
| `tunnel_full` | `full` 0-3240 | `xicp` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 0.791 | 0.891 | 0.942 | `ok_no_risk` |

## Readout

- GT calibration is currently blocked by missing public pose/odom/tf for the local NTNU LiDAR degeneracy extraction.
- The comparison remains GT-free for now: local risk, diagnostic watch, and cross-method risk should be treated as triage signals, not error labels.
- The script is ready to rerun with external GT via `--gt-csv fog_200=... --gt-csv tunnel_geom_2700_200=...`.
