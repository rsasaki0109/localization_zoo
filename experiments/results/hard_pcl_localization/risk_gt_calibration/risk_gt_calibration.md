# LiDAR Degeneracy Risk GT Calibration

Policy: `lidar_degeneracy_triage_v4` (`evaluation\config\lidar_degeneracy_triage_policy.json`)

## GT Availability

| Sequence | GT CSV | Exists | Pose-like bag topics |
| --- | --- | --- | --- |
| `indoor_easy_01` | experiments\reference_data\hard_pcl_indoor_easy_01_gt.csv | yes | none |
| `indoor_easy_02` | experiments\reference_data\hard_pcl_indoor_easy_02_gt.csv | yes | none |
| `indoor_hard_01` | experiments\reference_data\hard_pcl_indoor_hard_01_gt.csv | yes | none |
| `outdoor_hard_01` | experiments\reference_data\hard_pcl_outdoor_hard_01_gt.csv | yes | none |
| `outdoor_hard_02` | experiments\reference_data\hard_pcl_outdoor_hard_02_gt.csv | yes | none |
| `outdoor_kidnap` | experiments\reference_data\hard_pcl_outdoor_kidnap_gt.csv | yes | none |

## GT Metrics

| Sequence | Window | Method | Risk bucket | ATE m | Mean err m | Max err m | Step RMSE m | GT path m |
| --- | --- | --- | --- | ---: | ---: | ---: | ---: | ---: |
| `indoor_easy_01` | `full` 0-2026 | `ct_icp` | `diagnostic_watch` | 18.767 | 14.557 | 37.939 | 0.265 | 76.314 |
| `indoor_easy_01` | `full` 0-2026 | `degen_sense_imu` | `ok` | 13.201 | 10.164 | 29.961 | 0.541 | 76.314 |
| `indoor_easy_01` | `full` 0-2026 | `degen_sense_no_imu` | `diagnostic_watch` | 11.759 | 9.139 | 26.561 | 2.714 | 76.314 |
| `indoor_easy_01` | `full` 0-2026 | `kiss_keyframe` | `diagnostic_watch` | 8.763 | 7.763 | 15.775 | 0.056 | 76.314 |
| `indoor_easy_01` | `full` 0-2026 | `litamin2` | `local_risk` | 10.815 | 8.502 | 21.676 | 0.348 | 76.314 |
| `indoor_easy_01` | `full` 0-2026 | `xicp` | `local_risk` | 10.037 | 8.409 | 22.111 | 0.314 | 76.314 |
| `indoor_easy_02` | `full` 0-1966 | `ct_icp` | `diagnostic_watch` | 20.471 | 19.383 | 30.935 | 0.274 | 78.518 |
| `indoor_easy_02` | `full` 0-1966 | `degen_sense_imu` | `ok` | 12.790 | 9.775 | 26.734 | 0.519 | 78.518 |
| `indoor_easy_02` | `full` 0-1966 | `degen_sense_no_imu` | `local_risk` | 12.632 | 10.438 | 25.947 | 3.104 | 78.518 |
| `indoor_easy_02` | `full` 0-1966 | `kiss_keyframe` | `diagnostic_watch` | 9.797 | 7.849 | 26.649 | 0.056 | 78.518 |
| `indoor_easy_02` | `full` 0-1966 | `litamin2` | `local_risk` | 12.547 | 10.154 | 23.254 | 0.442 | 78.518 |
| `indoor_easy_02` | `full` 0-1966 | `xicp` | `local_risk` | 15.237 | 12.550 | 26.450 | 0.225 | 78.518 |
| `indoor_hard_01` | `full` 0-2378 | `ct_icp` | `diagnostic_watch` | 16.795 | 15.071 | 32.564 | 0.325 | 115.491 |
| `indoor_hard_01` | `full` 0-2378 | `degen_sense_imu` | `local_risk` | 617.008 | 428.009 | 1657.475 | 8.317 | 115.491 |
| `indoor_hard_01` | `full` 0-2378 | `degen_sense_no_imu` | `local_risk` | 11.729 | 10.057 | 25.218 | 3.654 | 115.491 |
| `indoor_hard_01` | `full` 0-2378 | `kiss_keyframe` | `local_risk` | 25.307 | 21.246 | 45.308 | 0.080 | 115.491 |
| `indoor_hard_01` | `full` 0-2378 | `litamin2` | `local_risk` | 17.875 | 13.852 | 37.601 | 0.586 | 115.491 |
| `indoor_hard_01` | `full` 0-2378 | `xicp` | `local_risk` | 11.733 | 10.037 | 23.477 | 0.339 | 115.491 |
| `outdoor_hard_01` | `full` 0-5146 | `ct_icp` | `ok` | 178.686 | 146.539 | 342.199 | 0.225 | 997.645 |
| `outdoor_hard_01` | `full` 0-5146 | `degen_sense_imu` | `ok` | 128.088 | 109.513 | 267.868 | 0.273 | 997.645 |
| `outdoor_hard_01` | `full` 0-5146 | `degen_sense_no_imu` | `diagnostic_watch` | 530394.694 | 268273.155 | 1574021.556 | 562.036 | 997.645 |
| `outdoor_hard_01` | `full` 0-5146 | `kiss_keyframe` | `diagnostic_watch` | 273.666 | 244.641 | 397.548 | 0.143 | 997.645 |
| `outdoor_hard_01` | `full` 0-5146 | `litamin2` | `ok` | 175.098 | 153.642 | 339.674 | 0.182 | 997.645 |
| `outdoor_hard_01` | `full` 0-5146 | `xicp` | `ok` | 157.230 | 140.882 | 237.156 | 0.236 | 997.645 |
| `outdoor_hard_02` | `full` 0-5126 | `ct_icp` | `ok` | 179.956 | 153.854 | 307.525 | 0.208 | 969.368 |
| `outdoor_hard_02` | `full` 0-5126 | `degen_sense_imu` | `ok` | 264.218 | 217.241 | 477.451 | 0.919 | 969.368 |
| `outdoor_hard_02` | `full` 0-5126 | `degen_sense_no_imu` | `ok` | 160.330 | 105.931 | 390.015 | 0.429 | 969.368 |
| `outdoor_hard_02` | `full` 0-5126 | `kiss_keyframe` | `diagnostic_watch` | 227.109 | 183.174 | 390.038 | 0.111 | 969.368 |
| `outdoor_hard_02` | `full` 0-5126 | `litamin2` | `ok` | 105.224 | 89.543 | 169.744 | 0.176 | 969.368 |
| `outdoor_hard_02` | `full` 0-5126 | `xicp` | `ok` | 140.411 | 119.594 | 286.376 | 0.133 | 969.368 |
| `outdoor_kidnap` | `full` 0-4016 | `ct_icp` | `diagnostic_watch` | 180.862 | 151.179 | 285.607 | 0.208 | 741.484 |
| `outdoor_kidnap` | `full` 0-4016 | `degen_sense_imu` | `diagnostic_watch` | 204.541 | 171.463 | 510.725 | 1.609 | 741.484 |
| `outdoor_kidnap` | `full` 0-4016 | `degen_sense_no_imu` | `local_risk` | 5872.830 | 3494.390 | 15280.395 | 45.865 | 741.484 |
| `outdoor_kidnap` | `full` 0-4016 | `kiss_keyframe` | `diagnostic_watch` | 236.508 | 210.353 | 355.358 | 0.178 | 741.484 |
| `outdoor_kidnap` | `full` 0-4016 | `litamin2` | `diagnostic_watch` | 112.915 | 96.493 | 248.995 | 0.198 | 741.484 |
| `outdoor_kidnap` | `full` 0-4016 | `xicp` | `diagnostic_watch` | 212.166 | 178.761 | 311.659 | 0.196 | 741.484 |

## Proxy Calibration

Stress-window rows only.

| Risk bucket | Windows | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `diagnostic_watch` | 9 | 183960.906 | 448.580 | 1344.483 | 150.340 | 1344.483 |
| `local_risk` | 6 | 26315.239 | n/a | n/a | 45.479 | 233.785 |
| `ok` | 9 | 1163.527 | 0.969 | 1.132 | 1.039 | 1.268 |

## Policy Decisions

Stress-window rows only.

| Decision | Rows | Methods | Mean accepted | Mean converged | Mean path/healthy | Mean path/all |
| --- | ---: | --- | ---: | ---: | ---: | ---: |
| `pass` | 7 | `degen_sense_imu`, `degen_sense_no_imu`, `litamin2`, `xicp` | 0.990 | 0.972 | 0.951 | 1.022 |
| `watch` | 9 | `ct_icp`, `kiss_keyframe`, `litamin2`, `xicp` | 0.789 | 0.483 | 0.831 | 0.882 |
| `investigate` | 2 | `degen_sense_imu`, `degen_sense_no_imu` | 0.670 | 0.860 | 1344.483 | 673.660 |
| `fail` | 6 | `degen_sense_imu`, `degen_sense_no_imu`, `kiss_keyframe`, `litamin2`, `xicp` | 0.273 | 0.718 | n/a | 45.479 |

## Reason Drilldown

Stress-window rows only. Rows with multiple active signals appear under each reason.

| Reason | Policy | Rows | Methods | Risk buckets | Mean accepted | Mean converged | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |
| --- | --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `cross_method_suspicious` | `investigate` | 6 | `degen_sense_imu`, `degen_sense_no_imu`, `kiss_keyframe` | `diagnostic_watch`, `local_risk` | 0.398 | 0.808 | 301411.583 | 1344.483 | 1344.483 | 269.743 | 1344.483 |
| `path_disagrees_with_all_method_median` | `watch` | 6 | `degen_sense_imu`, `degen_sense_no_imu`, `kiss_keyframe` | `diagnostic_watch`, `local_risk` | 0.398 | 0.808 | 301411.583 | 1344.483 | 1344.483 | 269.743 | 1344.483 |
| `partial_acceptance` | `watch` | 9 | `ct_icp`, `degen_sense_imu`, `degen_sense_no_imu`, `kiss_keyframe`, `litamin2`, `xicp` | `diagnostic_watch` | 0.721 | 0.673 | 183960.906 | 448.580 | 1344.483 | 150.340 | 1344.483 |
| `low_convergence` | `watch` | 6 | `ct_icp`, `kiss_keyframe`, `xicp` | `diagnostic_watch`, `local_risk`, `ok` | 0.651 | 0.141 | 690.100 | 1.034 | 1.067 | 0.838 | 1.135 |
| `ok_no_risk` | `pass` | 7 | `degen_sense_imu`, `degen_sense_no_imu`, `litamin2`, `xicp` | `ok` | 0.990 | 0.972 | 1140.601 | 0.951 | 1.132 | 1.022 | 1.268 |
| `low_acceptance` | `fail` | 6 | `degen_sense_imu`, `degen_sense_no_imu`, `kiss_keyframe`, `litamin2`, `xicp` | `local_risk` | 0.273 | 0.718 | 26315.239 | n/a | n/a | 45.479 | 233.785 |

## Policy Detail

Stress-window rows only.

| Sequence | Window | Method | Decision | Risk bucket | Watch action | Guard decision | Accepted | Converged | Path/healthy | Path/all | Reasons |
| --- | --- | --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| `indoor_hard_01` | `full` 0-2378 | `degen_sense_imu` | `fail` | `local_risk` | `n/a` | `n/a` | 0.364 | 0.999 | n/a | 25.788 | `cross_method_suspicious`, `low_acceptance`, `path_disagrees_with_all_method_median` |
| `indoor_hard_01` | `full` 0-2378 | `degen_sense_no_imu` | `fail` | `local_risk` | `n/a` | `n/a` | 0.270 | 0.999 | n/a | 11.161 | `cross_method_suspicious`, `low_acceptance`, `path_disagrees_with_all_method_median` |
| `indoor_hard_01` | `full` 0-2378 | `kiss_keyframe` | `fail` | `local_risk` | `n/a` | `n/a` | 0.363 | 0.410 | n/a | 0.407 | `cross_method_suspicious`, `low_acceptance`, `low_convergence`, `path_disagrees_with_all_method_median` |
| `indoor_hard_01` | `full` 0-2378 | `litamin2` | `fail` | `local_risk` | `n/a` | `n/a` | 0.390 | 0.983 | n/a | 1.206 | `low_acceptance` |
| `indoor_hard_01` | `full` 0-2378 | `xicp` | `fail` | `local_risk` | `n/a` | `n/a` | 0.198 | 0.192 | n/a | 0.525 | `low_acceptance`, `low_convergence` |
| `outdoor_kidnap` | `full` 0-4016 | `degen_sense_no_imu` | `fail` | `local_risk` | `n/a` | `n/a` | 0.050 | 0.724 | n/a | 233.785 | `cross_method_suspicious`, `low_acceptance`, `path_disagrees_with_all_method_median` |
| `outdoor_hard_01` | `full` 0-5146 | `degen_sense_no_imu` | `investigate` | `diagnostic_watch` | `n/a` | `n/a` | 0.652 | 0.996 | 1344.483 | 1344.483 | `cross_method_suspicious`, `partial_acceptance`, `path_disagrees_with_all_method_median` |
| `outdoor_kidnap` | `full` 0-4016 | `degen_sense_imu` | `investigate` | `diagnostic_watch` | `n/a` | `n/a` | 0.688 | 0.724 | n/a | 2.837 | `cross_method_suspicious`, `partial_acceptance`, `path_disagrees_with_all_method_median` |
| `indoor_hard_01` | `full` 0-2378 | `ct_icp` | `watch` | `diagnostic_watch` | `n/a` | `n/a` | 0.664 | 0.228 | n/a | 0.794 | `low_convergence`, `partial_acceptance` |
| `outdoor_hard_01` | `full` 0-5146 | `ct_icp` | `watch` | `ok` | `n/a` | `n/a` | 0.986 | 0.002 | 1.067 | 1.067 | `low_convergence` |
| `outdoor_hard_01` | `full` 0-5146 | `kiss_keyframe` | `watch` | `diagnostic_watch` | `n/a` | `n/a` | 0.833 | 0.993 | 0.568 | 0.568 | `partial_acceptance` |
| `outdoor_hard_02` | `full` 0-5126 | `ct_icp` | `watch` | `ok` | `n/a` | `n/a` | 0.972 | 0.004 | 1.000 | 1.135 | `low_convergence` |
| `outdoor_hard_02` | `full` 0-5126 | `kiss_keyframe` | `watch` | `diagnostic_watch` | `n/a` | `n/a` | 0.857 | 0.992 | 0.688 | 0.781 | `partial_acceptance` |
| `outdoor_kidnap` | `full` 0-4016 | `ct_icp` | `watch` | `diagnostic_watch` | `n/a` | `n/a` | 0.720 | 0.011 | n/a | 1.101 | `low_convergence`, `partial_acceptance` |
| `outdoor_kidnap` | `full` 0-4016 | `kiss_keyframe` | `watch` | `diagnostic_watch` | `n/a` | `n/a` | 0.638 | 0.713 | n/a | 0.755 | `partial_acceptance` |
| `outdoor_kidnap` | `full` 0-4016 | `litamin2` | `watch` | `diagnostic_watch` | `n/a` | `n/a` | 0.719 | 0.720 | n/a | 0.899 | `partial_acceptance` |
| `outdoor_kidnap` | `full` 0-4016 | `xicp` | `watch` | `diagnostic_watch` | `n/a` | `n/a` | 0.715 | 0.679 | n/a | 0.839 | `partial_acceptance` |
| `outdoor_hard_01` | `full` 0-5146 | `degen_sense_imu` | `pass` | `ok` | `n/a` | `n/a` | 0.990 | 0.996 | 0.933 | 0.933 | `ok_no_risk` |
| `outdoor_hard_01` | `full` 0-5146 | `litamin2` | `pass` | `ok` | `n/a` | `n/a` | 0.995 | 0.995 | 0.899 | 0.899 | `ok_no_risk` |
| `outdoor_hard_01` | `full` 0-5146 | `xicp` | `pass` | `ok` | `n/a` | `n/a` | 0.993 | 0.930 | 1.132 | 1.132 | `ok_no_risk` |
| `outdoor_hard_02` | `full` 0-5126 | `degen_sense_imu` | `pass` | `ok` | `n/a` | `n/a` | 0.984 | 0.995 | 1.117 | 1.268 | `ok_no_risk` |
| `outdoor_hard_02` | `full` 0-5126 | `degen_sense_no_imu` | `pass` | `ok` | `n/a` | `n/a` | 0.982 | 0.995 | 1.055 | 1.197 | `ok_no_risk` |
| `outdoor_hard_02` | `full` 0-5126 | `litamin2` | `pass` | `ok` | `n/a` | `n/a` | 0.994 | 0.995 | 0.756 | 0.858 | `ok_no_risk` |
| `outdoor_hard_02` | `full` 0-5126 | `xicp` | `pass` | `ok` | `n/a` | `n/a` | 0.994 | 0.897 | 0.762 | 0.865 | `ok_no_risk` |

## Readout

- GT-backed rows were produced. Risk buckets and policy decisions remain runtime proxy labels; compare them directly with ATE/RPE rather than treating them as error labels.
- `fail` marks hard local failure, `investigate` unresolved cross-method disagreement, `watch` calibrated confidence downgrades, and `pass` no active proxy risk.
- Reason drilldown now shows which runtime signals coincide with measured trajectory error and which apparently healthy rows still have material GT error.
