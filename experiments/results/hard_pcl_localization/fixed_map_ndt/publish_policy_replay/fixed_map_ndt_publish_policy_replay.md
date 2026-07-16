# Fixed-Map NDT Publish Policy Replay

Trace glob: `experiments\results\hard_pcl_localization\fixed_map_ndt\traces\*_trace.json`

## Aggregate

| Traces | Frames | Raw wrong | Raw unsafe jumps | Gated pose outputs | Gated wrong outputs | Unknown frames | Block frames | Suppressed wrong | Suppressed unsafe jumps | Max published error [m] |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 3 | 5983 | 5690 | 2069 | 236 | 9 | 3677 | 2070 | 5690 | 2069 | 3.843 |

| Embedded runtime outputs | Embedded wrong outputs | Embedded unknown | Embedded relock candidates | Embedded max relock streak | Embedded suppressed wrong | Embedded suppressed unsafe |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 4015 | 3987 | 1968 | 0 | 0 | 1703 | 714 |

Replay decisions: fail_closed_suppressed:1, unsafe_publish:2

## Trace Reports

| Trace | Replay decision | Verifier decision | Raw wrong | Raw unsafe | Gated outputs | Gated wrong | Unknown | Block | Suppressed wrong | Embedded outputs | Embedded unknown | Relock candidates | Max relock streak | Embedded first publish | Replay first publish | Max published error [m] | Max unknown streak |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `indoor_kidnap_01` | `unsafe_publish` | `block_publish` | 1747 | 977 | 16 | 8 | 779 | 978 | 1747 | 1264 | 509 | 0 | 0 | 5 | 5 | 2.969 | 1686 |
| `indoor_kidnap_02` | `unsafe_publish` | `block_publish` | 1238 | 503 | 39 | 1 | 761 | 503 | 1238 | 919 | 384 | 0 | 0 | 49 | 2 | 3.843 | 1155 |
| `outdoor_kidnap` | `fail_closed_suppressed` | `block_publish` | 2705 | 589 | 181 | 0 | 2137 | 589 | 2705 | 1832 | 1075 | 0 | 0 | 515 | 3 | 0.819 | 2704 |

## Readout

- Raw fixed-map NDT is treated as publishing every frame's `final_pose`.
- Gated replay publishes only sequence-verifier publish candidates, can briefly hold the last safe pose, and otherwise returns unknown or blocks publish.
- Embedded runtime outputs are the GT-free publish actions written by `pcd_dogfooding` itself.
- `suppressed_wrong_pose_frames` measures false-pose suppression; `gated_wrong_pose_frames` must stay zero before this policy can be considered publish-safe.
- A `fail_closed_all_unknown` post-hoc replay is still fully closed; compare it with embedded relock outputs to measure recovery availability.
