# Fixed-Map NDT Publish Policy Replay

Trace glob: `experiments/results/fixed_map_ndt/traces/*_trace.json`

## Aggregate

| Traces | Frames | Raw wrong | Raw unsafe jumps | Gated pose outputs | Gated wrong outputs | Unknown frames | Block frames | Suppressed wrong | Suppressed unsafe jumps | Max published error [m] |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 108 | 73 | 2 | 0 | 0 | 106 | 2 | 73 | 2 | n/a |

Replay decisions: fail_closed_all_unknown:1

## Trace Reports

| Trace | Replay decision | Verifier decision | Raw wrong | Raw unsafe | Gated outputs | Gated wrong | Unknown | Block | Suppressed wrong | First publish | Max published error [m] | Max unknown streak |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `kitti_seq_02_108_seed_scan_context_stride_5_topk_5` | `fail_closed_all_unknown` | `block_publish` | 73 | 2 | 0 | 0 | 106 | 2 | 73 | none | n/a | 108 |

## Readout

- Raw fixed-map NDT is treated as publishing every frame's `final_pose`.
- Gated replay publishes only sequence-verifier publish candidates, can briefly hold the last safe pose, and otherwise returns unknown or blocks publish.
- `suppressed_wrong_pose_frames` measures false-pose suppression; `gated_wrong_pose_frames` must stay zero before this policy can be considered publish-safe.
- A `fail_closed_all_unknown` replay is safe but unavailable: the next engineering target is recovery/relocalization, not threshold relaxation.
