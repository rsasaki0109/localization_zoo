# Fixed-Map NDT Trace Sequence Verifier

Trace glob: `experiments\results\hard_pcl_localization\fixed_map_ndt\traces\*_trace.json`

## Aggregate

| Traces | Frames | Publishable traces | Return-unknown frames | Accepted wrong-pose frames | Recovery jumps | Unsafe jumps | Max final error [m] | Max step ratio |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 3 | 5983 | 0 | 3673 | 739 | 0 | 2069 | 190.510 | 3331.489 |

Trace decisions: block_publish:3

## Trace Reports

| Trace | Decision | State | Publishable frames | Unknown frames | Accepted wrong | Fallback streaks | Stable inlier intervals | Max stable run | Max error [m] | Max step ratio |
| --- | --- | --- | ---: | ---: | ---: | --- | --- | ---: | ---: | ---: |
| `indoor_kidnap_01` | `block_publish` | `unsafe_transition` | 8 | 777 | 537 | 1815-2153 (339) | 1-16 (16), 68-71 (4), 73-74 (2), +2 more | 16 | 17.258 | 862.673 |
| `indoor_kidnap_02` | `block_publish` | `unsafe_transition` | 37 | 760 | 45 | 1304-1608 (305) | 1-57 (57), 59-59 (1), 102-102 (1), +1 more | 57 | 27.061 | 535.934 |
| `outdoor_kidnap` | `block_publish` | `unsafe_transition` | 181 | 2136 | 157 | 2916-3153 (238), 3159-3161 (3), 3331-3674 (344), +1 more | 1-114 (114), 116-202 (87) | 114 | 190.510 | 3331.489 |

## Readout

- This verifier consumes per-frame trace transitions, not only run-level ATE.
- Large pose steps are split into recovery jumps and unsafe jumps by checking whether GT error drops after the transition.
- Accurate single recovery frames are held until a stable inlier run reaches the configured publish length.
- `return_unknown` frames mark intervals where a live component should suppress pose publish and relocalize.
