# Fixed-Map NDT Publish Policy Replay

Trace glob: `experiments\results\hard_pcl_localization\fixed_map_ndt\traces\*_trace.json`

## Aggregate

| Traces | Frames | Raw wrong | Raw unsafe jumps | Gated pose outputs | Gated wrong outputs | Unknown frames | Block frames | Suppressed wrong | Suppressed unsafe jumps | Max published error [m] |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 2 | 3076 | 2985 | 1480 | 45 | 0 | 1550 | 1481 | 2985 | 1480 | 0.945 |

| Embedded runtime outputs | Embedded wrong outputs | Embedded unknown | Embedded relock candidates | Embedded max relock streak | Embedded suppressed wrong | Embedded suppressed unsafe |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 2183 | 2155 | 893 | 0 | 0 | 830 | 455 |

Replay decisions: fail_closed_suppressed:2

## Trace Reports

| Trace | Replay decision | Verifier decision | Raw wrong | Raw unsafe | Gated outputs | Gated wrong | Unknown | Block | Suppressed wrong | Embedded outputs | Embedded unknown | Relock candidates | Max relock streak | Embedded first publish | Replay first publish | Max published error [m] | Max unknown streak |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `indoor_kidnap_01` | `fail_closed_suppressed` | `block_publish` | 1747 | 977 | 8 | 0 | 787 | 978 | 1747 | 1264 | 509 | 0 | 0 | 5 | 5 | 0.945 | 1758 |
| `indoor_kidnap_02` | `fail_closed_suppressed` | `block_publish` | 1238 | 503 | 37 | 0 | 763 | 503 | 1238 | 919 | 384 | 0 | 0 | 49 | 2 | 0.835 | 1155 |

## Readout

- Raw fixed-map NDT is treated as publishing every frame's `final_pose`.
- Gated replay publishes only sequence-verifier publish candidates, can briefly hold the last safe pose, and otherwise returns unknown or blocks publish.
- Embedded runtime outputs are the GT-free publish actions written by `pcd_dogfooding` itself.
- `suppressed_wrong_pose_frames` measures false-pose suppression; `gated_wrong_pose_frames` must stay zero before this policy can be considered publish-safe.
- A `fail_closed_all_unknown` post-hoc replay is still fully closed; compare it with embedded relock outputs to measure recovery availability.
