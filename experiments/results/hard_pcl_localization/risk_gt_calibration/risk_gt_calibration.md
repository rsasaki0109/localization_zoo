# LiDAR Degeneracy Risk GT Calibration

Policy: `lidar_degeneracy_triage_v4` (`evaluation\config\lidar_degeneracy_triage_policy.json`)

## GT Availability

| Sequence | GT CSV | Exists | Pose-like bag topics |
| --- | --- | --- | --- |
| `indoor_easy_01` | experiments\reference_data\hard_pcl_indoor_easy_01_gt.csv | yes | none |
| `indoor_hard_01` | experiments\reference_data\hard_pcl_indoor_hard_01_gt.csv | yes | none |

## GT Metrics

| Sequence | Window | Method | Risk bucket | ATE m | Mean err m | Max err m | Step RMSE m | GT path m |
| --- | --- | --- | --- | ---: | ---: | ---: | ---: | ---: |
| `indoor_easy_01` | `full` 0-2026 | `ct_icp` | `diagnostic_watch` | 18.767 | 14.557 | 37.939 | 0.265 | 76.314 |
| `indoor_easy_01` | `full` 0-2026 | `degen_sense_imu` | `ok` | 13.201 | 10.164 | 29.961 | 0.541 | 76.314 |
| `indoor_easy_01` | `full` 0-2026 | `degen_sense_no_imu` | `diagnostic_watch` | 11.759 | 9.139 | 26.561 | 2.714 | 76.314 |
| `indoor_easy_01` | `full` 0-2026 | `kiss_keyframe` | `diagnostic_watch` | 8.763 | 7.763 | 15.775 | 0.056 | 76.314 |
| `indoor_easy_01` | `full` 0-2026 | `litamin2` | `local_risk` | 10.815 | 8.502 | 21.676 | 0.348 | 76.314 |
| `indoor_easy_01` | `full` 0-2026 | `xicp` | `local_risk` | 10.037 | 8.409 | 22.111 | 0.314 | 76.314 |
| `indoor_hard_01` | `full` 0-2378 | `ct_icp` | `diagnostic_watch` | 16.795 | 15.071 | 32.564 | 0.325 | 115.491 |
| `indoor_hard_01` | `full` 0-2378 | `degen_sense_imu` | `local_risk` | 617.008 | 428.009 | 1657.475 | 8.317 | 115.491 |
| `indoor_hard_01` | `full` 0-2378 | `degen_sense_no_imu` | `local_risk` | 11.729 | 10.057 | 25.218 | 3.654 | 115.491 |
| `indoor_hard_01` | `full` 0-2378 | `kiss_keyframe` | `local_risk` | 25.307 | 21.246 | 45.308 | 0.080 | 115.491 |
| `indoor_hard_01` | `full` 0-2378 | `litamin2` | `local_risk` | 17.875 | 13.852 | 37.601 | 0.586 | 115.491 |
| `indoor_hard_01` | `full` 0-2378 | `xicp` | `local_risk` | 11.733 | 10.037 | 23.477 | 0.339 | 115.491 |

## Proxy Calibration

Stress-window rows only.

| Risk bucket | Windows | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `diagnostic_watch` | 1 | 468.384 | n/a | n/a | 0.794 | 0.794 |
| `local_risk` | 5 | 4613.905 | n/a | n/a | 7.818 | 25.788 |

## Policy Decisions

Stress-window rows only.

| Decision | Rows | Methods | Mean accepted | Mean converged | Mean path/healthy | Mean path/all |
| --- | ---: | --- | ---: | ---: | ---: | ---: |
| `pass` | 0 | n/a | n/a | n/a | n/a | n/a |
| `watch` | 1 | `ct_icp` | 0.664 | 0.228 | n/a | 0.794 |
| `investigate` | 0 | n/a | n/a | n/a | n/a | n/a |
| `fail` | 5 | `degen_sense_imu`, `degen_sense_no_imu`, `kiss_keyframe`, `litamin2`, `xicp` | 0.317 | 0.716 | n/a | 7.818 |

## Reason Drilldown

Stress-window rows only. Rows with multiple active signals appear under each reason.

| Reason | Policy | Rows | Methods | Risk buckets | Mean accepted | Mean converged | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |
| --- | --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `cross_method_suspicious` | `investigate` | 3 | `degen_sense_imu`, `degen_sense_no_imu`, `kiss_keyframe` | `local_risk` | 0.332 | 0.802 | 7349.285 | n/a | n/a | 12.452 | 25.788 |
| `path_disagrees_with_all_method_median` | `watch` | 3 | `degen_sense_imu`, `degen_sense_no_imu`, `kiss_keyframe` | `local_risk` | 0.332 | 0.802 | 7349.285 | n/a | n/a | 12.452 | 25.788 |
| `low_acceptance` | `fail` | 5 | `degen_sense_imu`, `degen_sense_no_imu`, `kiss_keyframe`, `litamin2`, `xicp` | `local_risk` | 0.317 | 0.716 | 4613.905 | n/a | n/a | 7.818 | 25.788 |
| `partial_acceptance` | `watch` | 1 | `ct_icp` | `diagnostic_watch` | 0.664 | 0.228 | 468.384 | n/a | n/a | 0.794 | 0.794 |
| `low_convergence` | `watch` | 3 | `ct_icp`, `kiss_keyframe`, `xicp` | `diagnostic_watch`, `local_risk` | 0.408 | 0.277 | 339.342 | n/a | n/a | 0.575 | 0.794 |

## Policy Detail

Stress-window rows only.

| Sequence | Window | Method | Decision | Risk bucket | Watch action | Guard decision | Accepted | Converged | Path/healthy | Path/all | Reasons |
| --- | --- | --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| `indoor_hard_01` | `full` 0-2378 | `degen_sense_imu` | `fail` | `local_risk` | `n/a` | `n/a` | 0.364 | 0.999 | n/a | 25.788 | `cross_method_suspicious`, `low_acceptance`, `path_disagrees_with_all_method_median` |
| `indoor_hard_01` | `full` 0-2378 | `degen_sense_no_imu` | `fail` | `local_risk` | `n/a` | `n/a` | 0.270 | 0.999 | n/a | 11.161 | `cross_method_suspicious`, `low_acceptance`, `path_disagrees_with_all_method_median` |
| `indoor_hard_01` | `full` 0-2378 | `kiss_keyframe` | `fail` | `local_risk` | `n/a` | `n/a` | 0.363 | 0.410 | n/a | 0.407 | `cross_method_suspicious`, `low_acceptance`, `low_convergence`, `path_disagrees_with_all_method_median` |
| `indoor_hard_01` | `full` 0-2378 | `litamin2` | `fail` | `local_risk` | `n/a` | `n/a` | 0.390 | 0.983 | n/a | 1.206 | `low_acceptance` |
| `indoor_hard_01` | `full` 0-2378 | `xicp` | `fail` | `local_risk` | `n/a` | `n/a` | 0.198 | 0.192 | n/a | 0.525 | `low_acceptance`, `low_convergence` |
| `indoor_hard_01` | `full` 0-2378 | `ct_icp` | `watch` | `diagnostic_watch` | `n/a` | `n/a` | 0.664 | 0.228 | n/a | 0.794 | `low_convergence`, `partial_acceptance` |

## Readout

- GT-backed rows were produced. Risk buckets and policy decisions remain runtime proxy labels; compare them directly with ATE/RPE rather than treating them as error labels.
- `fail` marks hard local failure, `investigate` unresolved cross-method disagreement, `watch` calibrated confidence downgrades, and `pass` no active proxy risk.
- Reason drilldown now shows which runtime signals coincide with measured trajectory error and which apparently healthy rows still have material GT error.
