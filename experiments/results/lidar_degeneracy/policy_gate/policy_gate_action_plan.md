# LiDAR Degeneracy Policy Gate Action Plan

Source: `experiments/results/lidar_degeneracy/policy_gate/policy_gate_report.json`
Policy: `lidar_degeneracy_triage_v1`

## Summary

- Action items: 4
- Offender rows: 20

## Recommended Order

| Priority | Decision | Category | Method | Rows | Top reasons | Action |
| ---: | --- | --- | --- | ---: | --- | --- |
| 1 | `fail` | `local_matcher_failure` | `kiss_keyframe` | 6 | all_pairs_failed, low_acceptance, low_convergence | Inspect per-pair registration decisions, overlap, and acceptance gates; add fallback or retry behavior before relaxing the strict gate. |
| 2 | `fail` | `local_matcher_failure` | `geometry_icp` | 4 | low_acceptance, all_pairs_failed | Inspect per-pair registration decisions, overlap, and acceptance gates; add fallback or retry behavior before relaxing the strict gate. |
| 3 | `investigate` | `false_confidence_risk` | `intensity_bev` | 4 | motion_margin_dominant, overlap_tail | Tighten motion-margin and overlap-tail checks so confident-looking trajectories are downgraded when degeneracy evidence is strong. |
| 4 | `investigate` | `cross_method_disagreement` | `intensity_bev` | 6 | cross_method_suspicious, path_disagrees_with_all_method_median, path_disagrees_with_healthy_median, no_healthy_peer | Compare the suspect method against healthy peers, inspect path-length ratios, and require a second signal before accepting the pose stream. |

## Action Detail

### P1 Repair local matcher acceptance and convergence - `kiss_keyframe`

- Decision: `fail`
- Category: `local_matcher_failure`
- Component: local registration
- Rows: 6
- Reports: method_health_comparison=3, risk_gt_calibration=3
- Reasons: all_pairs_failed=6, low_acceptance=6, low_convergence=6
- Recommended action: Inspect per-pair registration decisions, overlap, and acceptance gates; add fallback or retry behavior before relaxing the strict gate.

| Sequence | Window | Frames | Rows |
| --- | --- | --- | ---: |
| `fog_200` | `baseline` | `[0,29)` | 2 |
| `fog_200` | `point_count_tail` | `[115,144)` | 2 |
| `fog_200` | `degraded` | `[170,199)` | 2 |

### P2 Repair local matcher acceptance and convergence - `geometry_icp`

- Decision: `fail`
- Category: `local_matcher_failure`
- Component: local registration
- Rows: 4
- Reports: method_health_comparison=2, risk_gt_calibration=2
- Reasons: low_acceptance=4, all_pairs_failed=2
- Recommended action: Inspect per-pair registration decisions, overlap, and acceptance gates; add fallback or retry behavior before relaxing the strict gate.

| Sequence | Window | Frames | Rows |
| --- | --- | --- | ---: |
| `fog_200` | `point_count_tail` | `[115,144)` | 2 |
| `fog_200` | `degraded` | `[170,199)` | 2 |

### P3 Calibrate false-confidence health signals - `intensity_bev`

- Decision: `investigate`
- Category: `false_confidence_risk`
- Component: failure awareness
- Rows: 4
- Reports: method_health_comparison=2, risk_gt_calibration=2
- Reasons: motion_margin_dominant=4, overlap_tail=2
- Recommended action: Tighten motion-margin and overlap-tail checks so confident-looking trajectories are downgraded when degeneracy evidence is strong.

| Sequence | Window | Frames | Rows |
| --- | --- | --- | ---: |
| `fog_200` | `point_count_tail` | `[115,144)` | 2 |
| `tunnel_geom_2700_200` | `degraded` | `[170,199)` | 2 |

### P4 Investigate cross-method trajectory disagreement - `intensity_bev`

- Decision: `investigate`
- Category: `cross_method_disagreement`
- Component: cross-method validation
- Rows: 6
- Reports: method_health_comparison=3, risk_gt_calibration=3
- Reasons: cross_method_suspicious=6, path_disagrees_with_all_method_median=4, path_disagrees_with_healthy_median=4, no_healthy_peer=2
- Recommended action: Compare the suspect method against healthy peers, inspect path-length ratios, and require a second signal before accepting the pose stream.

| Sequence | Window | Frames | Rows |
| --- | --- | --- | ---: |
| `fog_200` | `degraded` | `[170,199)` | 2 |
| `tunnel_geom_2700_200` | `point_count_tail` | `[80,109)` | 2 |
| `tunnel_geom_2700_200` | `geometry_degeneracy` | `[90,119)` | 2 |
