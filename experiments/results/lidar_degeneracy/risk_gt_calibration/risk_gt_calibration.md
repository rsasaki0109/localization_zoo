# LiDAR Degeneracy Risk GT Calibration

Policy: `lidar_degeneracy_triage_v4` (`evaluation/config/lidar_degeneracy_triage_policy.json`)

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
| `diagnostic_watch` | 4 | 4.497 | 3.307 | 3.745 | 3.307 | 3.745 |
| `local_risk` | 6 | 1.275 | 1.219 | 4.128 | 0.639 | 2.022 |
| `ok` | 10 | 1.271 | 1.000 | 1.391 | 0.898 | 1.270 |

## Policy Decisions

Stress-window rows only.

| Decision | Rows | Methods | Mean accepted | Mean converged | Mean path/healthy | Mean path/all |
| --- | ---: | --- | ---: | ---: | ---: | ---: |
| `pass` | 9 | `geometry_icp`, `kiss_keyframe` | 1.000 | 1.000 | 1.043 | 0.965 |
| `watch` | 11 | `ct_icp`, `intensity_bev`, `kiss_keyframe` | 0.969 | 0.586 | 1.923 | 1.579 |
| `investigate` | 0 | n/a | n/a | n/a | n/a | n/a |
| `fail` | 0 | n/a | n/a | n/a | n/a | n/a |

## Reason Drilldown

Stress-window rows only. Rows with multiple active signals appear under each reason.

| Reason | Policy | Rows | Methods | Risk buckets | Mean accepted | Mean converged | Mean path m | Mean path/healthy | Max path/healthy | Mean path/all | Max path/all |
| --- | --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `ct_icp_internal_convergence_low` | `watch` | 5 | `ct_icp` | `diagnostic_watch`, `local_risk` | 0.931 | 0.090 | 4.142 | 3.184 | 3.745 | 2.910 | 3.745 |
| `partial_acceptance` | `watch` | 1 | `ct_icp` | `local_risk` | 0.655 | 0.000 | 2.723 | 2.691 | 2.691 | 1.319 | 1.319 |
| `ok_no_risk` | `pass` | 9 | `geometry_icp`, `kiss_keyframe` | `ok` | 1.000 | 1.000 | 1.344 | 1.043 | 1.391 | 0.965 | 1.270 |
| `low_motion_margin_dominant` | `watch` | 5 | `intensity_bev` | `local_risk` | 1.000 | 1.000 | 0.985 | 0.925 | 4.128 | 0.504 | 2.022 |
| `path_disagrees_with_all_method_median` | `watch` | 1 | `kiss_keyframe` | `ok` | 1.000 | 1.000 | 0.617 | 0.609 | 0.609 | 0.299 | 0.299 |
| `low_used_path` | `watch` | 3 | `intensity_bev` | `local_risk` | 1.000 | 1.000 | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 |

## Policy Detail

Stress-window rows only.

| Sequence | Window | Method | Decision | Risk bucket | Watch action | Guard decision | Accepted | Converged | Path/healthy | Path/all | Reasons |
| --- | --- | --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| `fog_200` | `point_count_tail` 115-144 | `ct_icp` | `watch` | `diagnostic_watch` | `fallback_required` | `fallback_to_prior` | 1.000 | 0.000 | 2.761 | 2.761 | `ct_icp_internal_convergence_low` |
| `fog_200` | `point_count_tail` 115-144 | `intensity_bev` | `watch` | `local_risk` | `n/a` | `n/a` | 1.000 | 1.000 | 0.000 | 0.000 | `low_motion_margin_dominant`, `low_used_path` |
| `fog_200` | `degraded` 170-199 | `ct_icp` | `watch` | `local_risk` | `n/a` | `reject_or_retry` | 0.655 | 0.000 | 2.691 | 1.319 | `ct_icp_internal_convergence_low`, `partial_acceptance` |
| `fog_200` | `degraded` 170-199 | `intensity_bev` | `watch` | `local_risk` | `n/a` | `n/a` | 1.000 | 1.000 | 4.128 | 2.022 | `low_motion_margin_dominant` |
| `fog_200` | `degraded` 170-199 | `kiss_keyframe` | `watch` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 0.609 | 0.299 | `path_disagrees_with_all_method_median` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `ct_icp` | `watch` | `diagnostic_watch` | `fallback_required` | `fallback_to_prior` | 1.000 | 0.103 | 3.699 | 3.699 | `ct_icp_internal_convergence_low` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `intensity_bev` | `watch` | `local_risk` | `n/a` | `n/a` | 1.000 | 1.000 | 0.000 | 0.000 | `low_motion_margin_dominant`, `low_used_path` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `ct_icp` | `watch` | `diagnostic_watch` | `fallback_required` | `fallback_to_prior` | 1.000 | 0.138 | 3.745 | 3.745 | `ct_icp_internal_convergence_low` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `intensity_bev` | `watch` | `local_risk` | `n/a` | `n/a` | 1.000 | 1.000 | 0.496 | 0.496 | `low_motion_margin_dominant` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `ct_icp` | `watch` | `diagnostic_watch` | `fallback_required` | `fallback_to_prior` | 1.000 | 0.207 | 3.025 | 3.025 | `ct_icp_internal_convergence_low` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `intensity_bev` | `watch` | `local_risk` | `n/a` | `n/a` | 1.000 | 1.000 | 0.000 | 0.000 | `low_motion_margin_dominant`, `low_used_path` |
| `fog_200` | `point_count_tail` 115-144 | `geometry_icp` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 1.256 | 1.256 | `ok_no_risk` |
| `fog_200` | `point_count_tail` 115-144 | `kiss_keyframe` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 0.744 | 0.744 | `ok_no_risk` |
| `fog_200` | `degraded` 170-199 | `geometry_icp` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 1.391 | 0.681 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `geometry_icp` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | n/a | 1.270 | 1.270 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `kiss_keyframe` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 0.730 | 0.730 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_icp` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | n/a | 1.168 | 1.168 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `kiss_keyframe` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 0.832 | 0.832 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `geometry_icp` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | n/a | 0.853 | 0.853 | `ok_no_risk` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `kiss_keyframe` | `pass` | `ok` | `n/a` | `n/a` | 1.000 | 1.000 | 1.147 | 1.147 | `ok_no_risk` |

## Readout

- GT calibration is currently blocked by missing public pose/odom/tf for the local NTNU LiDAR degeneracy extraction.
- The comparison remains GT-free for now: local risk, diagnostic watch, and cross-method risk should be treated as triage signals, not error labels.
- Policy decisions are pre-GT triage labels: `fail` for hard local failure, `investigate` for unresolved cross-method disagreement, `watch` for calibrated local confidence downgrades and medium risk, and `pass` for rows with no active risk reason.
- Reason drilldown separates local failure signals from cross-method disagreement so the strongest triage signals can be checked first when GT arrives.
- The script is ready to rerun with external GT via `--gt-csv fog_200=... --gt-csv tunnel_geom_2700_200=...`.
- This policy has since been GT-calibrated for real on the Koide Hard PCL
  dataset: `experiments/results/hard_pcl_localization/risk_gt_calibration/risk_gt_calibration.md`.
  That GT-calibrated policy has in turn been applied unmodified to the NTNU
  `tunnel_full` / `fog_full` full-trajectory runs as a transferability
  check (not an accuracy claim, since NTNU still has no GT): see
  `ntnu_cross_dataset.md` in this directory. Verdict: it transfers to
  fog-type (sensing-degradation) stress but not to tunnel-type (geometric
  self-similarity) stress, because the policy's reason vocabulary has no
  trajectory-shape signal and the tunnel's failure mode only shows up at
  the trajectory level.
