# Native paper odometry interfaces

Python and ROS 2 interfaces expose the existing SuMa frame-to-model,
IMLS-SLAM raw-scan model, DiLO range-image and LiTAMIN2 Gaussian-map pipelines.
They preserve native parameter defaults and causal state without ground-truth
seeding or wrapper-side pose accumulation. This is an interface check on the
committed MCD fixture; accuracy conclusions use the existing benchmark evidence.
The [main integration check](#integration-with-shared-components) below records
the interfaces after the shared robustness components were merged.

## Local native/Python verification

Implementation `98c2f56f16397f73c7fe93bc566ac0c3538747db`, based on
`134ead2f21a4611ec328a45aa543b31428d6b92c`. Raw executions were committed in
`502e5c8a` before rendering the table. [Commands and exit codes](local/98c2f56f/commands.json),
[fixture report](local/98c2f56f/paper_python_fixture.json),
[input/module/reference hashes](local/98c2f56f/metadata.json).

| Native paper path | Scans | Maximum Python/C++ pose-element difference | Native diagnostics |
|---|---:|---:|---|
| SuMa | 3 | 0.0 | equal |
| IMLS-SLAM | 3 | 0.0 | equal |
| DiLO | 3 | 0.0 | equal |
| LiTAMIN2 | 3 | 0.0 | equal |

The complete CTest run passes, including the new Python/native fixture test.
The existing Python example and each paper example pass. The binding tests also
exercise Fortran/non-contiguous arrays, independent pose copies, rejected empty
or nonfinite scans, enum fields and the existing disabled opt-in defaults.
The oracle receives the same raw XYZ values and calls the native implementations
directly, with no CLI sensor profile or ground-truth initialization.

DiLO's native default returns identity poses throughout this fixture. Its Python
interface returns the same poses and window state. This fixture provides an API
parity check; motion recovery uses the separate mechanism and KITTI evaluations.
No parameter was selected or retuned using these scans. A separate moving-room
fixture uses the same native DiLO defaults and the sensor beam layout from its
existing mechanism test. It produces non-identity poses and a maximum Python/C++
pose-element difference of 0.0, with equal native diagnostics.
[Committed moving-fixture report](local/cb6a2fb0_python_moving/paper_python_fixture.json),
commands and source guard were recorded in `9175ef7c`. The additional ROS case
checks these moving outputs too, using the same native configuration.


## ROS/native verification

Reliable-input execution at `1398455265a4bcaf3b7683ff77664d998f1754c3`
completed every real-fixture case. Its [report](ci/13984552_ros_reliable/report.json),
[command log](ci/13984552_ros_reliable/job.log) and
[receipt](ci/13984552_ros_reliable/receipt.json) were committed in `ef6115c5`
before the following observations. Both messages preserve the expected frame
names and scan timestamps. The reconstructed matrices include quaternion-to-matrix
rounding; the comparison tolerance is specified by the test.

| Native paper path | Scans | Maximum odometry/C++ matrix difference | Maximum TF/C++ matrix difference |
|---|---:|---:|---:|
| SuMa | 3 | 1.11e-15 | 1.11e-15 |
| IMLS-SLAM | 3 | 4.44e-16 | 4.44e-16 |
| DiLO | 3 | 0 | 0 |
| LiTAMIN2 | 3 | 4.44e-16 | 4.44e-16 |

These are interface parity measurements on one short fixture. DiLO's real-fixture
reference is identity as described above. The additional moving-room case guards
against returning a constant pose, with the same native defaults.

## ROS validation protocol and retained failure

The ROS test publishes the same scans through real node processes and compares
both odometry and TF against a separately built native oracle. It checks world
and sensor frame names, acquisition stamps, scalar float coordinates with either
byte order, and rejected invalid or repeated headers.

The first ROS execution at `98c2f56f` failed with exit code 1. Its
[full job log](ci/98c2f56f_ros_failed/job.log),
[partial report](ci/98c2f56f_ros_failed/partial_report.json) and
[receipt](ci/98c2f56f_ros_failed/receipt.json) remain committed. Before its
per-scan deadline it observed 0/3 odometry outputs for SuMa and 2/3 for IMLS;
DiLO and LiTAMIN2 completed. That attempt used best-effort input and waited only
for the point-cloud input connection. A missing output could reflect input
delivery or an output publisher not yet matched to the observer.

The reliable-input execution above completes every real-fixture case. The new
delivery option stays opt-in; normal sensor subscribers retain best-effort QoS.
Scans, native registration parameters and comparison tolerances are unchanged.
The final test also waits for input, odometry and TF endpoints before replay.
The intervening observer implementation at `021534d5` used a subscription API
absent from ROS 2 Humble and failed before any method comparison; its
[failed job and receipt](ci/021534d5_ros_humble_api_failed/) are retained. The
observer now uses Humble's supported node graph API. The final CI check covers
that observer and the additional moving-room case; the numerical table records
only the already committed execution identified above.

## Reproduction and scope

[Python examples](../../../python/README.md#stateful-paper-pipelines) and
[ROS 2 examples](../../../ros2/localization_zoo_ros/README.md#paper-odometry-nodes)
show each method. Native parameter fields remain available to configure another
sensor; published harness variants still require their recorded preprocessing
and arguments. Each test method uses one unchanged configuration for every scan.

Local commands retain their executed container/workspace prefixes. Replace those
prefixes when replaying; the repository CMake options build the same native
libraries. ROS CI runs the command recorded in `.github/workflows/ci.yml` after
sourcing the installed ROS workspace. Raw scan files and executable binaries are
not added to this results directory.

The original interface-only revision left core algorithm sources, common code,
runner loaders and stored benchmark aggregates unchanged relative to its stated
base. The main integration below incorporates the already-merged shared
components without further algorithm changes. Binding targets link those cores;
they do not change published paper numbers or enable robustness/sensor corrections.
The [final local checks](checks/final/commands.json) pass, including document
regeneration, Python regressions and showcase/leaderboard contracts. The
[preservation report](checks/final/preservation.json) confirms that historical
aggregates, paper reference values and published tables are unchanged; the root
README only adds the authorized binding description.
The NCLT real-data sensor-assistant check remains pending separately because its
approved download host is still absent from the enforced environment allowlist.

## Integration with shared components

Implementation `ff2fe7f1c8f38a0775329ac19333d65c3f200d61` merges main
`7505a2914328d53ac9b319d5f8042189b378d457` into the binding branch. Derived
documents are regenerated, while the roadmap retains both shared-component
results and the completed binding entry. Core algorithms, common code and runner
loaders match that main revision. Validation uses a separate worktree/build;
the ongoing KITTI replay's checkout, executable pins and sealed scripts remain
unchanged.

SuMa's newly merged `degenerate_ratio`, `coarse_frames`, `coarse_iterations` and
`coarse_distance_scale` fields are exposed in Python and ROS. Python also returns
`coarse_start` and the actual coarse iteration count. Remapping and coarse startup
retain their native disabled defaults.

The additional joint interface case explicitly uses remap ratio 0.03 and a single
coarse registration with budget 10 and distance multiplier 5.0. These transfer
settings come from the existing shared-remap/shared-coarse-start studies; none was
selected on this fixture. This combined case checks interface wiring and startup
expiry, without isolating either correction's accuracy effect. Each case uses
one unchanged configuration for every input scan.

Raw results were committed at `f8147ef3` before the following numbers:
[Python/native report](checks/main_refresh/native/paper_python_fixture.json),
[ROS/native report](checks/main_refresh/ci_ros/report.json),
[ROS job receipt](checks/main_refresh/ci_ros/receipt.json).

| Fixture case | Scans | Maximum Python/C++ pose-element difference | Maximum ROS odometry/C++ difference | Maximum TF/C++ difference |
|---|---:|---:|---:|---:|
| SuMa, native defaults | 3 | 0 | 1.11e-15 | 1.11e-15 |
| IMLS-SLAM, native defaults | 3 | 0 | 4.44e-16 | 4.44e-16 |
| DiLO, native defaults | 3 | 0 | 0 | 0 |
| LiTAMIN2, native defaults | 3 | 0 | 4.44e-16 | 4.44e-16 |
| DiLO, moving room / same defaults | 3 | 0 | 3.33e-16 | 3.33e-16 |
| SuMa, explicit shared options | 3 | 0 | 5.55e-16 | 5.55e-16 |

Python/native diagnostics match exactly. The explicit SuMa case reports coarse
startup `[false, true, false]` and coarse iterations `[0, 10, 0]`. The real DiLO
default remains identity; its moving-room case guards against constant output.
These short fixtures establish interface parity, with no new full-sequence
accuracy or speed claim. The earlier delivery/observer failures remain above.

The [native commands and actual exits](checks/main_refresh/native/commands.json)
cover a Release build of the affected targets, all 8 affected CTest entries,
the existing Python example and every paper example, all exiting 0. The
[document checks](checks/main_refresh/docs/commands.json) include all three
generators, 290 Python regressions and showcase/leaderboard/paper-ready checks,
also exiting 0. [Preservation](checks/main_refresh/docs/preservation.json) keeps
all 688 historical aggregate hashes, paper reference values, Table 6 and published
leaderboards unchanged relative to main. Native/ROS execution commands retain
their actual workspace paths; full ROS commands are in the losslessly compressed
[job transcript](checks/main_refresh/ci_ros/job.log.gz).

The initial full-branch whitespace check exits 2 on three previously committed
raw CI transcripts. [The failure](checks/main_refresh/prepare/initial_whitespace.json)
is retained, their original bytes are preserved, and only those exact log paths
receive whitespace attributes. The corrected check exits 0. New raw logs are
stored as gzip with [verified original/output hashes](checks/main_refresh/import.json).
The [check driver](refresh_main_checks.py) takes `--phase native` or `--phase docs`
and requires a fresh `--output-dir`; full-suite and ROS verification also run in CI.
The [post-import regeneration and checks](checks/main_refresh/post_import/commands.json)
and [preservation receipt](checks/main_refresh/post_import/preservation.json)
cover the imported evidence and this updated report.

## Integration with the merged acceleration and passive trace stack

Integration source `70c77efa80f6b7739ad7fd00122b3c18e6f9024e` merges main
`a597e4c95614e733fc20977321fe4d6aea0fba46`. Validation source
`9b6092e77d581279411bf580676b599653f8a42f` adds an explicit baseline argument
and recursive native-source guards to the existing check driver. The guards
confirm that algorithms, common code and evaluation sources/scripts match that
main. Python and ROS wrapper sources match the previously checked binding head
`5ee60bb6f05d38eceee006c5ff854bc37348570a`.

The IMLS-SLAM acceleration option remains disabled through these interfaces;
it is currently exposed only by the native API and runner. The passive InTEn-LOAM
API/runner changes do not add an InTEn-LOAM Python class or ROS node. No stored
registration setting is reselected. The separate validation worktree leaves the
ongoing full KITTI replay's source, executable pins and sealed scripts unchanged.

Native evidence was committed at `ac767dfc` before these results. The
[commands and actual exits](checks/merged_stack/native/commands.json) cover the
affected Release build, all 8 affected CTest entries and the existing/paper Python
examples, all exiting 0. In the [parity report](checks/merged_stack/native/paper_python_fixture.json),
all 6 cases have maximum Python/C++ pose-element difference 0 and identical native
diagnostics. The real DiLO default still returns identity; the moving-room case
still exercises non-identity output with the same native defaults. This is a
short-fixture compatibility check, without a new accuracy or speed claim.

Reproduce with a fresh output directory and a checkout of the recorded source:

```bash
python3 experiments/results/paper_pipeline_bindings/refresh_main_checks.py \
  --phase native --base-commit a597e4c95614e733fc20977321fe4d6aea0fba46 \
  --output-dir /absolute/path/to/fresh-native-checks
```

The driver records the actual Docker/build/example commands and checkout commit;
it does not provision its container. [Executable/input hashes](checks/merged_stack/native/hashes.json)
and [lossless import receipts](checks/merged_stack/native_import.json) retain
the provenance of this run. Earlier parity tables and negative ROS trials above
remain attached to their original source revisions.

ROS evidence was committed at `518fa5a8` before these numbers. The completed
[ROS CI job](https://github.com/rsasaki0109/localization_zoo/actions/runs/39126883285/job/117673763755)
checks all 6 cases with 3 input scans each, including real odometry and TF,
with maximum pose-element differences of 1.11e-15 for both outputs. The
[per-case report](checks/merged_stack/ci_ros/report.json),
[source/job/hash receipt](checks/merged_stack/ci_ros/receipt.json) and
[complete compressed transcript](checks/merged_stack/ci_ros/job.log.gz) retain
the execution commands, input/output checks and original values. This repeats
the existing fixture protocol and explicitly reliable input choice; it does
not reselect registration parameters or change the default subscriber QoS.

Document-check evidence was committed at `84466d64`: the
[recorded commands](checks/merged_stack/docs/commands.json) exit 0 for all
required generators, 299 Python regressions, showcase/leaderboard/paper-ready
contracts and full-branch whitespace. The [preservation receipt](checks/merged_stack/docs/preservation.json)
confirms that all 688 historical aggregate hashes, paper values, Table 6 and
published leaderboards match the integrated main baseline. The root README
diff is limited to the binding description. Use the driver with `--phase docs`
and the same explicit base to reproduce these checks in a fresh output directory.

[Post-import commands](checks/merged_stack/post_import/commands.json) and
[preservation](checks/merged_stack/post_import/preservation.json) cover the
imported ROS evidence and this report. Full native/Python and ROS builds also
run in CI for the final PR head.
