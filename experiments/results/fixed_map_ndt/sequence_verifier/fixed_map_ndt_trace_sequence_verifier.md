# Fixed-Map NDT Trace Sequence Verifier

Trace glob: `experiments/results/fixed_map_ndt/traces/*_trace.json`

## Aggregate

| Traces | Frames | Publishable traces | Return-unknown frames | Accepted wrong-pose frames | Recovery jumps | Unsafe jumps | Max final error [m] | Max step ratio |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 108 | 0 | 71 | 65 | 19 | 2 | 6.081 | 5.166 |

Trace decisions: block_publish:1

## Trace Reports

| Trace | Decision | State | Publishable frames | Unknown frames | Accepted wrong | Fallback streaks | Stable inlier intervals | Max stable run | Max error [m] | Max step ratio |
| --- | --- | --- | ---: | ---: | ---: | --- | --- | ---: | ---: | ---: |
| `kitti_seq_02_108_seed_scan_context_stride_5_topk_5` | `block_publish` | `unsafe_transition` | 0 | 71 | 65 | 1-4 (4), 6-9 (4), 11-14 (4), +12 more | 56-56 (1), 61-61 (1), 66-66 (1), +8 more | 2 | 6.081 | 5.166 |

## Readout

- This verifier consumes per-frame trace transitions, not only run-level ATE.
- Large pose steps are split into recovery jumps and unsafe jumps by checking whether GT error drops after the transition.
- Accurate single recovery frames are held until a stable inlier run reaches the configured publish length.
- `return_unknown` frames mark intervals where a live component should suppress pose publish and relocalize.
