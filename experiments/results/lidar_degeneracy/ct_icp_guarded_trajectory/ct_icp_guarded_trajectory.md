# CT-ICP Guarded Trajectory Simulation

Comparison source: `experiments/results/lidar_degeneracy/method_health_comparison/method_health_comparison.json`

## Aggregate

| Guard decision | Rows | Improved | Guarded path status |
| --- | ---: | ---: | --- |
| `fallback_to_prior` | 5 | 2 | path_high:4, reference_consistent:1 |
| `reject_or_retry` | 1 | 0 | path_high:1 |
| `retry_optimizer` | 1 | 0 | reference_consistent:1 |

## Rows

| Sequence | Window | Guard | Original path | Guarded path | Original ref | Guarded ref | Status | Prior rate | Hold rate | Step delta max | Yaw delta max |
| --- | --- | --- | ---: | ---: | ---: | ---: | --- | ---: | ---: | ---: | ---: |
| `fog_200` | `baseline` 0-29 | `retry_optimizer` | 1.283 | 2.548 | 0.947 | 1.869 | `reference_consistent` | 1.000 | 0.000 | 0.000 | 0.000 |
| `fog_200` | `point_count_tail` 115-144 | `fallback_to_prior` | 2.703 | 2.065 | 2.761 | 2.100 | `path_high` | 1.000 | 0.000 | 0.000 | 0.000 |
| `fog_200` | `degraded` 170-199 | `reject_or_retry` | 2.655 | 2.829 | 2.691 | 2.796 | `path_high` | 1.000 | 0.000 | 0.000 | 0.000 |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `fallback_to_prior` | 5.189 | 24.430 | 3.411 | 16.078 | `path_high` | 1.000 | 0.000 | 0.000 | 0.000 |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `fallback_to_prior` | 5.412 | 7.153 | 3.699 | 4.868 | `path_high` | 1.000 | 0.000 | 0.000 | 0.000 |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `fallback_to_prior` | 5.676 | 1.969 | 3.745 | 1.301 | `reference_consistent` | 1.000 | 0.000 | 0.000 | 0.000 |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `fallback_to_prior` | 4.161 | 4.546 | 3.025 | 3.294 | `path_high` | 1.000 | 0.000 | 0.000 | 0.000 |

## Readout

- This is an offline simulation of the CT-ICP production guard; it does not rerun scan matching.
- `fallback_to_prior`, `retry_optimizer`, and `reject_or_retry` use a bounded constant-velocity prior instead of trusted refined CT-ICP steps.
- `velocity_bootstrap` pairs are counted in the per-window JSON so a path that only works by bootstrapping from suspect refined motion remains visible.
- `path_collapse` means the guard avoided the original path excursion but produced too little motion relative to healthy/all-method references.
