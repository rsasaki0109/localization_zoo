# LiDAR Degeneracy Policy Gate Action Plan

Source: `experiments/results/fixed_map_ndt/policy_gate/policy_gate_report.json`
Policy: `lidar_degeneracy_triage_v4`

## Summary

- Action items: 6
- Offender rows: 23

## Recommended Order

| Priority | Decision | Category | Method | Rows | Top reasons | Action |
| ---: | --- | --- | --- | ---: | --- | --- |
| 1 | `fail` | `wrong_pose_acceptance` | `fixed_map_ndt:scan_context` | 14 | accepted_bad_localization | Treat high-acceptance/high-error rows as publish-blocking failures; add a second verifier before accepting these seed sources. |
| 2 | `fail` | `wrong_pose_acceptance` | `fixed_map_ndt:ct_icp` | 1 | accepted_bad_localization | Treat high-acceptance/high-error rows as publish-blocking failures; add a second verifier before accepting these seed sources. |
| 3 | `fail` | `global_hypothesis_selection` | `fixed_map_ndt:scan_context` | 11 | unfiltered_ndt_score_trap | Stop relying on NDT score alone for unfiltered global candidates; add a calibrated geometric verifier or sequential prior before retrying top-K. |
| 4 | `investigate` | `detectable_bad_seed` | `fixed_map_ndt:velocity` | 3 | rejected_bad_seed_detectable | Keep these rows out of pose publication, then add a recovery path that switches to relocalization or returns unknown instead of dead reckoning. |
| 5 | `investigate` | `detectable_bad_seed` | `fixed_map_ndt:ct_icp` | 1 | rejected_bad_seed_detectable | Keep these rows out of pose publication, then add a recovery path that switches to relocalization or returns unknown instead of dead reckoning. |
| 6 | `investigate` | `map_localization_gap` | `fixed_map_ndt:scan_context` | 4 | bad_localization | Compare seed quality, NDT basin limits, and fixed-map acceptance scores; use the result to decide whether the initializer or backend needs work. |

## Action Detail

### P1 Block accepted wrong-pose localization - `fixed_map_ndt:scan_context`

- Decision: `fail`
- Category: `wrong_pose_acceptance`
- Component: localization safety gate
- Rows: 14
- Reports: fixed_map_ndt_failure_audit=14
- Reasons: accepted_bad_localization=14
- Recommended action: Treat high-acceptance/high-error rows as publish-blocking failures; add a second verifier before accepting these seed sources.

| Sequence | Window | Frames | Rows |
| --- | --- | --- | ---: |
| `kitti_seq_02` | `seed_scan_context_stride_10` | `[0,108)` | 1 |
| `kitti_seq_02` | `seed_scan_context_stride_10_topk_10` | `[0,108)` | 1 |
| `kitti_seq_02` | `seed_scan_context_stride_10_topk_10_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_02` | `seed_scan_context_stride_10_topk_5` | `[0,108)` | 1 |
| `kitti_seq_02` | `seed_scan_context_stride_10_topk_5_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_02` | `seed_scan_context_stride_5_topk_10_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_02` | `seed_scan_context_stride_5_topk_5_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_05` | `seed_scan_context_stride_10_topk_10_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_05` | `seed_scan_context_stride_10_topk_5_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_05` | `seed_scan_context_stride_5_topk_10_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_05` | `seed_scan_context_stride_5_topk_5_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_08` | `seed_scan_context_stride_10_topk_10_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_08` | `seed_scan_context_stride_5_topk_10_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_08` | `seed_scan_context_stride_5_topk_5_unfiltered` | `[0,108)` | 1 |

### P2 Block accepted wrong-pose localization - `fixed_map_ndt:ct_icp`

- Decision: `fail`
- Category: `wrong_pose_acceptance`
- Component: localization safety gate
- Rows: 1
- Reports: fixed_map_ndt_failure_audit=1
- Reasons: accepted_bad_localization=1
- Recommended action: Treat high-acceptance/high-error rows as publish-blocking failures; add a second verifier before accepting these seed sources.

| Sequence | Window | Frames | Rows |
| --- | --- | --- | ---: |
| `kitti_seq_02` | `seed_ct_icp` | `[0,108)` | 1 |

### P3 Replace unsafe global hypothesis selection - `fixed_map_ndt:scan_context`

- Decision: `fail`
- Category: `global_hypothesis_selection`
- Component: global initializer
- Rows: 11
- Reports: fixed_map_ndt_failure_audit=11
- Reasons: unfiltered_ndt_score_trap=11
- Recommended action: Stop relying on NDT score alone for unfiltered global candidates; add a calibrated geometric verifier or sequential prior before retrying top-K.

| Sequence | Window | Frames | Rows |
| --- | --- | --- | ---: |
| `kitti_seq_02` | `seed_scan_context_stride_10_topk_10_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_02` | `seed_scan_context_stride_10_topk_5_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_02` | `seed_scan_context_stride_5_topk_10_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_02` | `seed_scan_context_stride_5_topk_5_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_05` | `seed_scan_context_stride_10_topk_10_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_05` | `seed_scan_context_stride_10_topk_5_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_05` | `seed_scan_context_stride_5_topk_10_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_05` | `seed_scan_context_stride_5_topk_5_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_08` | `seed_scan_context_stride_10_topk_10_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_08` | `seed_scan_context_stride_5_topk_10_unfiltered` | `[0,108)` | 1 |
| `kitti_seq_08` | `seed_scan_context_stride_5_topk_5_unfiltered` | `[0,108)` | 1 |

### P4 Handle rejected bad seeds explicitly - `fixed_map_ndt:velocity`

- Decision: `investigate`
- Category: `detectable_bad_seed`
- Component: seed fallback
- Rows: 3
- Reports: fixed_map_ndt_failure_audit=3
- Reasons: rejected_bad_seed_detectable=3
- Recommended action: Keep these rows out of pose publication, then add a recovery path that switches to relocalization or returns unknown instead of dead reckoning.

| Sequence | Window | Frames | Rows |
| --- | --- | --- | ---: |
| `kitti_seq_02` | `seed_velocity` | `[0,108)` | 1 |
| `kitti_seq_05` | `seed_velocity` | `[0,108)` | 1 |
| `kitti_seq_08` | `seed_velocity` | `[0,108)` | 1 |

### P5 Handle rejected bad seeds explicitly - `fixed_map_ndt:ct_icp`

- Decision: `investigate`
- Category: `detectable_bad_seed`
- Component: seed fallback
- Rows: 1
- Reports: fixed_map_ndt_failure_audit=1
- Reasons: rejected_bad_seed_detectable=1
- Recommended action: Keep these rows out of pose publication, then add a recovery path that switches to relocalization or returns unknown instead of dead reckoning.

| Sequence | Window | Frames | Rows |
| --- | --- | --- | ---: |
| `kitti_seq_08` | `seed_ct_icp` | `[0,108)` | 1 |

### P6 Close map-localization accuracy gap - `fixed_map_ndt:scan_context`

- Decision: `investigate`
- Category: `map_localization_gap`
- Component: map localization backend
- Rows: 4
- Reports: fixed_map_ndt_failure_audit=4
- Reasons: bad_localization=4
- Recommended action: Compare seed quality, NDT basin limits, and fixed-map acceptance scores; use the result to decide whether the initializer or backend needs work.

| Sequence | Window | Frames | Rows |
| --- | --- | --- | ---: |
| `kitti_seq_08` | `seed_scan_context_stride_10` | `[0,108)` | 1 |
| `kitti_seq_08` | `seed_scan_context_stride_10_topk_10` | `[0,108)` | 1 |
| `kitti_seq_08` | `seed_scan_context_stride_10_topk_5` | `[0,108)` | 1 |
| `kitti_seq_08` | `seed_scan_context_stride_10_topk_5_unfiltered` | `[0,108)` | 1 |
