# CT-ICP Guarded Composite

Comparison source: `experiments/results/lidar_degeneracy/method_health_comparison/method_health_comparison.json`
Fallback bakeoff: `experiments/results/lidar_degeneracy/ct_icp_fallback_bakeoff/ct_icp_fallback_bakeoff.json`

## Aggregate

| Rows | Composite statuses | Selected sources | Improves original | Improves self velocity | Mean fallback rate |
| ---: | --- | --- | ---: | ---: | ---: |
| 7 | reference_consistent:7 | geometry_icp:7 | 7 | 7 | 1.000 |

## Rows

| Sequence | Window | Guard | Selected | Original ref | Self ref | Selected ref | Composite ref | Composite status | Fallback rate | Switches | Missing pairs |
| --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- | ---: | ---: | ---: |
| `fog_200` | `baseline` 0-29 | `retry_optimizer` | `geometry_icp` | 0.947 | 1.869 | 1.000 | 1.000 | `reference_consistent` | 1.000 | 0 | 0 |
| `fog_200` | `point_count_tail` 115-144 | `fallback_to_prior` | `geometry_icp` | 2.761 | 2.100 | 1.256 | 1.256 | `reference_consistent` | 1.000 | 0 | 0 |
| `fog_200` | `degraded` 170-199 | `reject_or_retry` | `geometry_icp` | 2.691 | 2.796 | 1.391 | 1.391 | `reference_consistent` | 1.000 | 0 | 0 |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `fallback_to_prior` | `geometry_icp` | 3.411 | 16.078 | 1.000 | 1.000 | `reference_consistent` | 1.000 | 0 | 0 |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `fallback_to_prior` | `geometry_icp` | 3.699 | 4.868 | 1.270 | 1.270 | `reference_consistent` | 1.000 | 0 | 0 |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `fallback_to_prior` | `geometry_icp` | 3.745 | 1.301 | 1.168 | 1.168 | `reference_consistent` | 1.000 | 0 | 0 |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `fallback_to_prior` | `geometry_icp` | 3.025 | 3.294 | 0.853 | 0.853 | `reference_consistent` | 1.000 | 0 | 0 |

## Source Counts

| Sequence | Window | Composite source counts | Result |
| --- | --- | --- | --- |
| `fog_200` | `baseline` 0-29 | geometry_icp:29 | `experiments/results/lidar_degeneracy/ct_icp_guarded_composite/results/fog_200_baseline_0000_0029_composite.json` |
| `fog_200` | `point_count_tail` 115-144 | geometry_icp:29 | `experiments/results/lidar_degeneracy/ct_icp_guarded_composite/results/fog_200_point_count_tail_0115_0144_composite.json` |
| `fog_200` | `degraded` 170-199 | geometry_icp:29 | `experiments/results/lidar_degeneracy/ct_icp_guarded_composite/results/fog_200_degraded_0170_0199_composite.json` |
| `tunnel_geom_2700_200` | `baseline` 5-34 | geometry_icp:29 | `experiments/results/lidar_degeneracy/ct_icp_guarded_composite/results/tunnel_geom_2700_200_baseline_0005_0034_composite.json` |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | geometry_icp:29 | `experiments/results/lidar_degeneracy/ct_icp_guarded_composite/results/tunnel_geom_2700_200_point_count_tail_0080_0109_composite.json` |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | geometry_icp:29 | `experiments/results/lidar_degeneracy/ct_icp_guarded_composite/results/tunnel_geom_2700_200_geometry_degeneracy_0090_0119_composite.json` |
| `tunnel_geom_2700_200` | `degraded` 170-199 | geometry_icp:29 | `experiments/results/lidar_degeneracy/ct_icp_guarded_composite/results/tunnel_geom_2700_200_degraded_0170_0199_composite.json` |

## Readout

- The composite uses CT-ICP refined steps only for `use_refined`; otherwise it uses the selected fallback source from the fallback bakeoff, falling back to self velocity only if the selected source is missing a pair.
- This is still GT-free and path-reference based. It validates the wiring contract and continuity, not final localization accuracy.
- A useful next check is to replay this composite contract on GT-backed KITTI 108 windows and compare ATE/RPE against CT-ICP, geometry-only, and self-velocity fallback.
