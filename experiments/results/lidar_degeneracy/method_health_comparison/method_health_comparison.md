# LiDAR Degeneracy Method Health Comparison

## Method Aggregate

| Sequence | Method | Windows | Mean accepted | Min accepted | Mean converged | Failed windows | Risk windows | Max used path m |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `fog_200` | `geometry_icp` | 3 | 0.264 | 0.000 | 0.264 | 2 | 2 | 1.276 |
| `fog_200` | `intensity_bev` | 3 | 0.977 | 0.931 | 0.977 | 0 | 0 | 11.516 |
| `fog_200` | `kiss_keyframe` | 3 | 0.000 | 0.000 | 0.000 | 3 | 3 | 0.000 |
| `fog_200` | `ct_icp` | 3 | 0.885 | 0.655 | 0.046 | 0 | 3 | 2.703 |
| `tunnel_geom_2700_200` | `geometry_icp` | 4 | 1.000 | 1.000 | 1.000 | 0 | 0 | 1.867 |
| `tunnel_geom_2700_200` | `intensity_bev` | 4 | 1.000 | 1.000 | 1.000 | 0 | 0 | 12.642 |
| `tunnel_geom_2700_200` | `kiss_keyframe` | 4 | 1.000 | 1.000 | 1.000 | 0 | 0 | 1.186 |
| `tunnel_geom_2700_200` | `ct_icp` | 4 | 1.000 | 1.000 | 0.147 | 0 | 4 | 5.676 |

## Window Detail

| Sequence | Window | Frames | Obscurant | Method | Accepted | Converged | Score | Overlap | Used path m | Max step m | Keyframes | Flags |
| --- | --- | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |
| `fog_200` | `baseline` 0-29 | 30 | 0.006 | `geometry_icp` | 0.655 | n/a | 0.775 | 65.9 | 1.276 | 0.095 | n/a | ok |
| `fog_200` | `baseline` 0-29 | 30 | 0.006 | `intensity_bev` | 1.000 | 1.000 | 0.606 | 5852.6 | 7.250 | 0.250 | n/a | ok |
| `fog_200` | `baseline` 0-29 | 30 | 0.006 | `kiss_keyframe` | 0.000 | 0.000 | 0.844 | 309.0 | 0.000 | 0.000 | 0/2 | all_pairs_failed, low_acceptance, low_convergence |
| `fog_200` | `baseline` 0-29 | 30 | 0.006 | `ct_icp` | 1.000 | 0.138 | n/a | n/a | 1.283 | 0.126 | n/a | low_convergence |
| `fog_200` | `point_count_tail` 115-144 | 30 | 0.521 | `geometry_icp` | 0.138 | n/a | 0.790 | 11.3 | 0.326 | 0.057 | n/a | low_acceptance |
| `fog_200` | `point_count_tail` 115-144 | 30 | 0.521 | `intensity_bev` | 0.931 | 0.931 | 0.743 | 5159.7 | 11.516 | 1.521 | n/a | ok |
| `fog_200` | `point_count_tail` 115-144 | 30 | 0.521 | `kiss_keyframe` | 0.000 | 0.000 | 0.795 | 239.4 | 0.000 | 0.000 | 0/2 | all_pairs_failed, low_acceptance, low_convergence |
| `fog_200` | `point_count_tail` 115-144 | 30 | 0.521 | `ct_icp` | 1.000 | 0.000 | n/a | n/a | 2.703 | 0.323 | n/a | low_convergence |
| `fog_200` | `degraded` 170-199 | 30 | 0.911 | `geometry_icp` | 0.000 | n/a | n/a | 0.0 | 0.000 | 0.000 | n/a | all_pairs_failed, low_acceptance |
| `fog_200` | `degraded` 170-199 | 30 | 0.911 | `intensity_bev` | 1.000 | 1.000 | 0.801 | 5542.0 | 8.281 | 0.559 | n/a | ok |
| `fog_200` | `degraded` 170-199 | 30 | 0.911 | `kiss_keyframe` | 0.000 | 0.000 | 0.794 | 163.3 | 0.000 | 0.000 | 0/2 | all_pairs_failed, low_acceptance, low_convergence |
| `fog_200` | `degraded` 170-199 | 30 | 0.911 | `ct_icp` | 0.655 | 0.000 | n/a | n/a | 2.655 | 0.229 | n/a | low_convergence |
| `tunnel_geom_2700_200` | `baseline` 5-34 | 30 | 0.002 | `geometry_icp` | 1.000 | n/a | 0.704 | 472.6 | 1.519 | 0.194 | n/a | ok |
| `tunnel_geom_2700_200` | `baseline` 5-34 | 30 | 0.002 | `intensity_bev` | 1.000 | 1.000 | 0.535 | 5522.3 | 7.975 | 0.354 | n/a | ok |
| `tunnel_geom_2700_200` | `baseline` 5-34 | 30 | 0.002 | `kiss_keyframe` | 1.000 | 1.000 | 0.747 | 2255.8 | 1.186 | 0.100 | 2/2 | ok |
| `tunnel_geom_2700_200` | `baseline` 5-34 | 30 | 0.002 | `ct_icp` | 1.000 | 0.138 | n/a | n/a | 5.189 | 0.842 | n/a | low_convergence |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | 30 | 0.035 | `geometry_icp` | 1.000 | n/a | 0.706 | 525.0 | 1.867 | 0.213 | n/a | ok |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | 30 | 0.035 | `intensity_bev` | 1.000 | 1.000 | 0.540 | 5374.6 | 9.875 | 0.707 | n/a | ok |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | 30 | 0.035 | `kiss_keyframe` | 1.000 | 1.000 | 0.753 | 2643.2 | 0.988 | 0.102 | 2/2 | ok |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | 30 | 0.035 | `ct_icp` | 1.000 | 0.103 | n/a | n/a | 5.412 | 0.433 | n/a | low_convergence |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | 30 | 0.045 | `geometry_icp` | 1.000 | n/a | 0.709 | 536.1 | 1.766 | 0.213 | n/a | ok |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | 30 | 0.045 | `intensity_bev` | 1.000 | 1.000 | 0.521 | 5114.1 | 12.642 | 0.707 | n/a | ok |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | 30 | 0.045 | `kiss_keyframe` | 1.000 | 1.000 | 0.755 | 2675.4 | 0.920 | 0.102 | 2/2 | ok |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | 30 | 0.045 | `ct_icp` | 1.000 | 0.138 | n/a | n/a | 5.676 | 0.529 | n/a | low_convergence |
| `tunnel_geom_2700_200` | `degraded` 170-199 | 30 | 0.227 | `geometry_icp` | 1.000 | n/a | 0.714 | 656.2 | 1.178 | 0.153 | n/a | ok |
| `tunnel_geom_2700_200` | `degraded` 170-199 | 30 | 0.227 | `intensity_bev` | 1.000 | 1.000 | 0.648 | 5534.3 | 7.561 | 0.354 | n/a | ok |
| `tunnel_geom_2700_200` | `degraded` 170-199 | 30 | 0.227 | `kiss_keyframe` | 1.000 | 1.000 | 0.745 | 3184.1 | 1.088 | 0.100 | 2/2 | ok |
| `tunnel_geom_2700_200` | `degraded` 170-199 | 30 | 0.227 | `ct_icp` | 1.000 | 0.207 | n/a | n/a | 4.161 | 0.345 | n/a | low_convergence |

## Readout

- `fog_200`: intensity BEV keeps 93.1-100% acceptance on selected windows after zero-motion score-margin preference, including the strongest fog slice.
- `fog_200`: KISS keyframe rejects every selected window, geometry ICP collapses on the strongest fog window, and CT-ICP keeps baseline/tail healthy but drops on strongest fog.
- `tunnel_geom_2700_200`: the short-window checks stay accepted, so this slice is not yet a local-odometry failure case.
- CT-ICP convergence is reported separately from acceptance because this repo's CT-ICP dogfooding path uses gate-accepted refinements even when the internal stopping bit is low.
