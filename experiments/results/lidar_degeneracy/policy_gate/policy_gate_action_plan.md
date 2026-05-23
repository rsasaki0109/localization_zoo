# LiDAR Degeneracy Policy Gate Action Plan

Source: `experiments/results/lidar_degeneracy/policy_gate/policy_gate_report.json`
Policy: `lidar_degeneracy_triage_v2`

## Summary

- Action items: 2
- Offender rows: 8

## Recommended Order

| Priority | Decision | Category | Method | Rows | Top reasons | Action |
| ---: | --- | --- | --- | ---: | --- | --- |
| 1 | `investigate` | `cross_method_disagreement` | `intensity_bev` | 6 | cross_method_suspicious, path_disagrees_with_healthy_median, path_disagrees_with_all_method_median | Compare the suspect method against healthy peers, inspect path-length ratios, and require a second signal before accepting the pose stream. |
| 2 | `investigate` | `cross_method_disagreement` | `kiss_keyframe` | 2 | cross_method_suspicious, path_disagrees_with_all_method_median, path_disagrees_with_healthy_median | Compare the suspect method against healthy peers, inspect path-length ratios, and require a second signal before accepting the pose stream. |

## Action Detail

### P1 Investigate cross-method trajectory disagreement - `intensity_bev`

- Decision: `investigate`
- Category: `cross_method_disagreement`
- Component: cross-method validation
- Rows: 6
- Reports: method_health_comparison=3, risk_gt_calibration=3
- Reasons: cross_method_suspicious=6, path_disagrees_with_healthy_median=6, path_disagrees_with_all_method_median=4
- Recommended action: Compare the suspect method against healthy peers, inspect path-length ratios, and require a second signal before accepting the pose stream.

| Sequence | Window | Frames | Rows |
| --- | --- | --- | ---: |
| `fog_200` | `degraded` | `[170,199)` | 2 |
| `tunnel_geom_2700_200` | `point_count_tail` | `[80,109)` | 2 |
| `tunnel_geom_2700_200` | `geometry_degeneracy` | `[90,119)` | 2 |

### P2 Investigate cross-method trajectory disagreement - `kiss_keyframe`

- Decision: `investigate`
- Category: `cross_method_disagreement`
- Component: cross-method validation
- Rows: 2
- Reports: method_health_comparison=1, risk_gt_calibration=1
- Reasons: cross_method_suspicious=2, path_disagrees_with_all_method_median=2, path_disagrees_with_healthy_median=2
- Recommended action: Compare the suspect method against healthy peers, inspect path-length ratios, and require a second signal before accepting the pose stream.

| Sequence | Window | Frames | Rows |
| --- | --- | --- | ---: |
| `fog_200` | `degraded` | `[170,199)` | 2 |
