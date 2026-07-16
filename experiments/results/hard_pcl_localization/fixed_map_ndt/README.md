# Fixed-map NDT kidnap false-lock evaluation

The two indoor kidnap sequences and `outdoor_kidnap` were localized against
their dataset-provided maps. This required adding
`--fixed-map-ndt-map-ply`; the old implementation silently built its target map
from the evaluated sequence's GT poses and therefore was not a provided-map
localization test.

## Localizer result

CT-ICP provides the deployable odometry seed. NDT uses the fixed PLY without
updating it:

| Sequence | Input frames | Trace frames | NDT accepted | CT prior ATE (m) | Fixed-map ATE (m) |
|---|---:|---:|---:|---:|---:|
| indoor_kidnap_01 | 2,154 | 1,773 | 549 / 1,772 | 9.546 | 8.839 |
| indoor_kidnap_02 | 1,609 | 1,303 | 59 / 1,302 | 15.100 | 16.966 |
| outdoor_kidnap | 4,017 | 2,907 | 171 / 2,906 | 107.730 | 103.823 |

Trace frames are fewer than bag frames because fully occluded/invalid depth
clouds become empty after range filtering. The verifier now supports these
sparse source frame indices.

Velocity-only seed was fail-closed but unusable: NDT accepted 0 frames in both
sequences and emitted no poses. A 100-frame GT-seed probe accepted 32
refinements, confirming that the provided map and GT share a usable coordinate
frame; the practical failure is the narrow NDT convergence basin.

## Embedded runtime publish guard

The GT-free guard inside `pcd_dogfooding` is unsafe on all three traces:

| Sequence | Pose/hold outputs | Wrong outputs (>1 m) | Maximum published error (m) |
|---|---:|---:|---:|
| indoor_kidnap_01 | 1,264 | 1,252 | 16.065 |
| indoor_kidnap_02 | 919 | 911 | 13.384 |
| outdoor_kidnap | 1,832 | 1,832 | 183.805 |
| **Total** | **4,015** | **3,995** | **183.805** |

The generated replay report counts 3,987 embedded wrong outputs because it
recomputes pose error from the serialized matrices and applies its own finite
frame filters. Either count reaches the same conclusion: the runtime
stable-refinement/hold signals are not evidence of a correct global lock.

## Sequence verifier and replay

The post-hoc GT sequence verifier classifies all three traces as
`block_publish`: 5,690/5,983 raw frames are wrong and 2,069 transitions are
unsafe jumps.

The default replay (`max_hold_frames=3`) still emits 9 wrong poses among 236
outputs, up to 3.843 m error. Setting `max_hold_frames=0` produces 226 outputs
with zero wrong poses (maximum 0.945 m) and suppresses all 5,690 wrong frames.
This no-hold replay is a GT-informed safety upper bound, not yet a deployable
runtime policy: the runtime system still needs a GT-free global-lock proof or
must return unknown.

Artifacts:

- `traces/*_trace.json`: per-frame seed/refinement/final/publish/GT labels
- `sequence_verifier/`: sparse-trace-aware block/unknown analysis
- `publish_policy_replay/`: existing three-frame hold policy
- `publish_policy_replay_no_hold/`: fail-closed no-hold result

## Reproduction

```bash
build/evaluation/pcd_dogfooding \
  dogfooding_results/hard_pcl_localization/indoor_kidnap_01 \
  experiments/reference_data/hard_pcl_indoor_kidnap_01_gt.csv 2154 \
  --methods fixed_map_ndt \
  --fixed-map-ndt-map-ply data/hard_pcl_localization/map_indoor_hard.ply \
  --fixed-map-ndt-seed-source ct_icp \
  --fixed-map-ndt-trace-json <sequence_trace.json> \
  --summary-json <summary.json>
```
