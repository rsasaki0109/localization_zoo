# KITTI07: committed historical differences and current error growth

Original native/cleanup commit `ed37e2fd` is imported byte-for-byte as
`9c9976c6`. Analysis source/protocol `59e69241` precedes execution;
raw checkpoint, diagnostics, actual commands and gate-test logs are committed
at `6db8522a` before this interpretation.

## Historical differences

All **20** selected cells complete **1,101** frames with native exits **0**;
**14** match stored RPE, ATE and frames. These **6** differ:

| Method | Stored 100 m RPE [%] | Replay RPE [%] | Stored ATE [m] | Replay ATE [m] |
|---|---:|---:|---:|---:|
| A-LOAM | 0.607458 | 0.605313 | 2.544355 | 2.505243 |
| CT-ICP | 0.455389 | 0.450827 | 1.256652 | 1.243500 |
| F-LOAM | 0.612719 | 0.590347 | 2.971748 | 3.173641 |
| LeGO-LOAM | 0.527758 | 0.526845 | 2.589775 | 2.557870 |
| Mesh-LOAM | 0.469625 | 0.469538 | 0.852913 | 0.843704 |
| VLOM | 0.607458 | 0.605313 | 2.544355 | 2.505243 |

RPE improves on every differing method. F-LOAM ATE worsens; the other ATEs
improve. The historical audit still exits **1**. The frozen pre-extraction
controls reproduce every pair with **0** recorded differences in RPE, ATE,
frames and official translation/rotation RTE, and byte-identical saved poses.
Their historical audit exits **1**; the comparison exits **0**. These differences
predate extraction; their original causes remain unresolved.

The [checkpoint CSV](../../checkpoints/07/comparison.csv) retains each selected
variant. These historical per-cell settings may differ by sequence. In
particular, this Mesh cell is `paper_odometry_elevation_keep_mesh`, rather than
the fixed variant used by Table 6. No setting is reselected and this replay
is not a new fixed-setting accuracy study.

## Where current error grows

Existing diagnostic parameters transfer unchanged from02/01: **100**-step
blocks, **0.1 m** moving-step threshold and **20%** relative-step threshold.
Every method's largest inherited block increase occurs at **800-900**:

| Method | Pose frames | Error at block start [m] | Error at block end [m] |
|---|---|---:|---:|
| A-LOAM | 800-900 | 1.728689 | 4.569446 |
| CT-ICP | 800-900 | 0.925644 | 2.294068 |
| F-LOAM | 800-900 | 1.697435 | 6.133360 |
| LeGO-LOAM | 800-900 | 1.143478 | 5.069158 |
| Mesh-LOAM | 800-900 | 0.456705 | 1.458130 |
| VLOM | 800-900 | 1.728689 | 4.569446 |

These are first-pose-normalized position errors against the same evaluated GT,
using zero-based indices, no fitted alignment and no rescaling. They locate
current error growth; they are distinct from official KITTI RTE or a gap between
historical and current estimates. Every block and every frame is retained in
per-method `current_steps.json` and `trace.csv` files.

All six exact referenced historical summaries/paths are absent in the inspected
tree, and their exact-path reachable-history probes find no commit. This is
local availability evidence, not proof of global absence. No paired historical
gap, historical runtime comparison or cause attribution is manufactured.
A-LOAM and VLOM have identical recorded current errors here; their shared
behavior is not an independent cause experiment.

## Commands, gates and scope

```bash
python3 experiments/results/shared_near_range/checkpoint_report.py \
  --output-dir /absolute/path/to/fresh-checkpoint07
python3 experiments/results/shared_near_range/diagnose_saved02.py \
  --sequence 07 --output-dir /absolute/path/to/fresh-saved07
```

[Actual diagnostic command and exit](analysis_command.json),
[checkpoint command and exit](../../checkpoints/07/analysis_command.json),
[input hashes and full results](summary.json),
[lossless native import](../../checks/saved07/import.json) and
[complete execution receipts](../../checks/saved07/commands.json) retain the run.
All **23** publication-gate tests pass with actual exit **0**, in normal and
optimized Python. The closed07 cohort includes VLOM without expanding earlier
02/05 cohorts; omitting its control is rejected before output. Committed native
receipts, matching frozen paths and verified input cleanup remain prerequisites.
Unclosed08 remains excluded.

[Repository check receipts](../../checks/saved07/docs/commands.json), committed
at `88c21dd9`, retain exact commands, actual exits and full lossless logs.
All required three generators, showcase, leaderboard, paper-ready and whitespace
checks exit **0**. Full Python runs **338** tests with **5** built-evaluator skips
in this separate checkout and exits **0**; native and ROS coverage run in CI.
The [preservation receipt](../../checks/saved07/docs/preservation.json) confirms
all **688** historical aggregate hashes, published table/paper-number bytes,
all **195** raw07 files, prior method README content and the primary's frozen
native source, sealed files and executable hashes. Timestamp-only generator
changes retain prior bytes with an explicit disposition receipt.

The initial private check driver failed on a mistyped native Git object name,
after its regeneration/regression commands had passed. Its source and exit
record are retained; correcting that reference and rerunning preservation alone
exits **0**. No estimator, result or publication value changed.

The [cleaned checkpoint](../../checkpoints/07/summary.json) now covers
**131/177** cells across 04/00/01/02/03/05/06/07: **103** historical matches and
**28** differences. All **30** frozen controls have zero recorded metric
differences and identical saved poses. All eight historical audits retain exit
**1**; checkpoint verification and offline diagnosis exit **0**. Remaining
08/09/10 cells are not counted until their own committed cleanup exists.

Official07 inputs were hash-verified before deleting the owned scans and alias.
Native source stays `0b079dbb`, with frozen executable hashes and historical
arguments. This offline analysis invokes no estimator or scan reader. Active08
inputs, binaries and sealed controller are untouched. Existing accuracy tables
and defaults are preserved; current-main acceleration/passive tracing and the
pending InTEn failure-cause investigation retain their separate scope.
