# Protocol: which InTEn-LOAM feature class carries the KITTI 10 yaw bias?

Written and committed before any run of this study.

**Question.** The [gap triage](../gap_decomposition/README.md#inten-loam-heading-error-follow-up)
found a systematic per-frame yaw bias on InTEn-LOAM KITTI 10 (final -0.0024
deg/frame, S2S output -0.0048 deg/frame; yaw-only RTE 0.909 % against the
paper's 0.45 % total). Does removing one feature class remove the bias?

**Arms.** KITTI 10 only (development sequence). Each arm uses the unchanged
`paper_odometry_plane_check` arguments, plus at most one existing
`--inten-loam-paper-set` key:

| Arm | Extra argument |
|---|---|
| `reference` | none |
| `no_edge` | `edges_per_sector=0` |
| `no_facade` | `facades_per_sector=0` |
| `no_intensity` | `use_intensity=0` |
| `no_ground` | `max_ground=0` |

Every arm writes the passive trace (`--inten-loam-paper-trace-jsonl`) and
runs once from frame 0. These are recursive runs: removing a class changes
both S2S and S2M, and through them the map history. They are not fixed-history
interventions.

**Measures.** Official RTE and ATE, and the per-frame yaw-error mean (bias),
standard deviation and yaw-only official RTE at the S2S output and at the
final pose, computed as in `../gap_decomposition/inten_heading.py`.

**Checks.** `reference` must reproduce the committed `paper_odometry_plane_check`
result on 10 (official RTE 1.155652 %). Its yaw statistics must match the
committed natural trace.

**Reading rule, fixed now.** A class is implicated if removing it lowers the
magnitude of the final yaw bias by at least half, with official RTE no worse
than twice the reference. Arms that fail are reported with their numbers.

**Scope.** This is a diagnostic. No arm is proposed as a setting, defaults do
not change, and no result enters Table 6. All KITTI is development data.
Binary: `build/evaluation/pcd_dogfooding`, SHA-256
`7cbd8e0d737d6a7cb6d0e326a5ccb16ae02d11286d4d34543e228f50a7e50d2c`, built
from source identical to main `91d30e8a` for InTEn-LOAM and the runner.
