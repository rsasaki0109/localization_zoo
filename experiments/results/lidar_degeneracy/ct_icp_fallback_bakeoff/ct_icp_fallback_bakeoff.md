# CT-ICP Fallback Source Bakeoff

Comparison source: `experiments/results/lidar_degeneracy/method_health_comparison/method_health_comparison.json`

## Source Summary

| Source | Rows | Eligible | Reference consistent | Selected |
| --- | ---: | ---: | ---: | ---: |
| `ct_icp_original` | 7 | 0 | 1 | 0 |
| `geometry_icp` | 7 | 7 | 7 | 7 |
| `intensity_bev` | 7 | 0 | 0 | 0 |
| `kiss_keyframe` | 7 | 5 | 5 | 0 |
| `self_velocity` | 7 | 2 | 2 | 0 |

## Window Selection

| Sequence | Window | Guard | CT original | Self velocity | Selected source | Selected ref | Improves original? | Improves self? |
| --- | --- | --- | ---: | ---: | --- | ---: | --- | --- |
| `fog_200` | `baseline` 0-29 | `retry_optimizer` | 0.947 | 1.869 | `geometry_icp` | 1.000 | yes | yes |
| `fog_200` | `point_count_tail` 115-144 | `fallback_to_prior` | 2.761 | 2.100 | `geometry_icp` | 1.256 | yes | yes |
| `fog_200` | `degraded` 170-199 | `reject_or_retry` | 2.691 | 2.796 | `geometry_icp` | 1.391 | yes | yes |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `fallback_to_prior` | 3.411 | 16.078 | `geometry_icp` | 1.000 | yes | yes |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `fallback_to_prior` | 3.699 | 4.868 | `geometry_icp` | 1.270 | yes | yes |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `fallback_to_prior` | 3.745 | 1.301 | `geometry_icp` | 1.168 | yes | yes |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `fallback_to_prior` | 3.025 | 3.294 | `geometry_icp` | 0.853 | yes | yes |

## Candidate Detail

| Sequence | Window | Source | Kind | Health | Risk | Ref | Status | Eligible | Blockers |
| --- | --- | --- | --- | --- | --- | ---: | --- | --- | --- |
| `fog_200` | `baseline` 0-29 | `geometry_icp` | `external_peer` | `ok` | `ok` | 1.000 | `reference_consistent` | yes | none |
| `fog_200` | `baseline` 0-29 | `kiss_keyframe` | `external_peer` | `ok` | `ok` | 0.271 | `path_collapse` | no | path_not_reference_consistent |
| `fog_200` | `baseline` 0-29 | `self_velocity` | `internal_prior` | `diagnostic_watch` | `diagnostic_watch` | 1.869 | `reference_consistent` | yes | none |
| `fog_200` | `baseline` 0-29 | `intensity_bev` | `external_peer` | `ok` | `ok` | 5.319 | `path_high` | no | path_not_reference_consistent |
| `fog_200` | `baseline` 0-29 | `ct_icp_original` | `ct_icp_refined` | `diagnostic_watch` | `diagnostic_watch` | 0.947 | `reference_consistent` | no | guard_rejects_refined |
| `fog_200` | `point_count_tail` 115-144 | `geometry_icp` | `external_peer` | `ok` | `ok` | 1.256 | `reference_consistent` | yes | none |
| `fog_200` | `point_count_tail` 115-144 | `kiss_keyframe` | `external_peer` | `ok` | `ok` | 0.756 | `reference_consistent` | yes | none |
| `fog_200` | `point_count_tail` 115-144 | `self_velocity` | `internal_prior` | `diagnostic_watch` | `diagnostic_watch` | 2.100 | `path_high` | no | path_not_reference_consistent |
| `fog_200` | `point_count_tail` 115-144 | `intensity_bev` | `external_peer` | `suspicious` | `suspicious` | 0.000 | `path_collapse` | no | source_health_not_ok, source_risk_not_ok, path_not_reference_consistent, reference_error_missing |
| `fog_200` | `point_count_tail` 115-144 | `ct_icp_original` | `ct_icp_refined` | `diagnostic_watch` | `diagnostic_watch` | 2.761 | `path_high` | no | guard_rejects_refined, path_not_reference_consistent |
| `fog_200` | `degraded` 170-199 | `geometry_icp` | `external_peer` | `ok` | `ok` | 1.391 | `reference_consistent` | yes | none |
| `fog_200` | `degraded` 170-199 | `kiss_keyframe` | `external_peer` | `ok` | `ok` | 0.613 | `path_collapse` | no | path_not_reference_consistent |
| `fog_200` | `degraded` 170-199 | `self_velocity` | `internal_prior` | `suspicious` | `suspicious` | 2.796 | `path_high` | no | path_not_reference_consistent |
| `fog_200` | `degraded` 170-199 | `intensity_bev` | `external_peer` | `suspicious` | `suspicious` | 4.128 | `path_high` | no | source_health_not_ok, source_risk_not_ok, path_not_reference_consistent |
| `fog_200` | `degraded` 170-199 | `ct_icp_original` | `ct_icp_refined` | `suspicious` | `suspicious` | 2.691 | `path_high` | no | guard_rejects_refined, path_not_reference_consistent |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `geometry_icp` | `external_peer` | `ok` | `ok` | 1.000 | `reference_consistent` | yes | none |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `kiss_keyframe` | `external_peer` | `ok` | `ok` | 0.780 | `reference_consistent` | yes | none |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `self_velocity` | `internal_prior` | `diagnostic_watch` | `diagnostic_watch` | 16.078 | `path_high` | no | path_not_reference_consistent |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `intensity_bev` | `external_peer` | `ok` | `ok` | 5.249 | `path_high` | no | path_not_reference_consistent |
| `tunnel_geom_2700_200` | `baseline` 5-34 | `ct_icp_original` | `ct_icp_refined` | `diagnostic_watch` | `diagnostic_watch` | 3.411 | `path_high` | no | guard_rejects_refined, path_not_reference_consistent |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `geometry_icp` | `external_peer` | `ok` | `ok` | 1.270 | `reference_consistent` | yes | none |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `kiss_keyframe` | `external_peer` | `ok` | `ok` | 0.672 | `reference_consistent` | yes | none |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `self_velocity` | `internal_prior` | `diagnostic_watch` | `diagnostic_watch` | 4.868 | `path_high` | no | path_not_reference_consistent |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `intensity_bev` | `external_peer` | `suspicious` | `suspicious` | 0.000 | `path_collapse` | no | source_health_not_ok, source_risk_not_ok, path_not_reference_consistent, reference_error_missing |
| `tunnel_geom_2700_200` | `point_count_tail` 80-109 | `ct_icp_original` | `ct_icp_refined` | `diagnostic_watch` | `diagnostic_watch` | 3.699 | `path_high` | no | guard_rejects_refined, path_not_reference_consistent |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `geometry_icp` | `external_peer` | `ok` | `ok` | 1.168 | `reference_consistent` | yes | none |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `kiss_keyframe` | `external_peer` | `ok` | `ok` | 0.608 | `reference_consistent` | yes | none |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `self_velocity` | `internal_prior` | `diagnostic_watch` | `diagnostic_watch` | 1.301 | `reference_consistent` | yes | none |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `intensity_bev` | `external_peer` | `suspicious` | `suspicious` | 0.496 | `path_collapse` | no | source_health_not_ok, source_risk_not_ok, path_not_reference_consistent |
| `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | `ct_icp_original` | `ct_icp_refined` | `diagnostic_watch` | `diagnostic_watch` | 3.745 | `path_high` | no | guard_rejects_refined, path_not_reference_consistent |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `geometry_icp` | `external_peer` | `ok` | `ok` | 0.853 | `reference_consistent` | yes | none |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `kiss_keyframe` | `external_peer` | `ok` | `ok` | 0.788 | `reference_consistent` | yes | none |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `self_velocity` | `internal_prior` | `diagnostic_watch` | `diagnostic_watch` | 3.294 | `path_high` | no | path_not_reference_consistent |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `intensity_bev` | `external_peer` | `suspicious` | `suspicious` | 0.000 | `path_collapse` | no | source_health_not_ok, source_risk_not_ok, path_not_reference_consistent, reference_error_missing |
| `tunnel_geom_2700_200` | `degraded` 170-199 | `ct_icp_original` | `ct_icp_refined` | `diagnostic_watch` | `diagnostic_watch` | 3.025 | `path_high` | no | guard_rejects_refined, path_not_reference_consistent |

## Readout

- This bakeoff does not rerun localization. It compares already-computed candidate trajectories under the same GT-free path-reference contract.
- A source is eligible only when it is healthy, risk-free, sufficiently accepted, and path-reference consistent.
- `ct_icp_original` is shown as the rejected refined trajectory baseline; it is never eligible when the guard rejects refined CT-ICP.
- External healthy peers are fallback-source candidates, not final production truth. GT-backed evaluation is still required before wiring a real component.
