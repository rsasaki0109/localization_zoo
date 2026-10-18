# Paper pipeline speed experiment

The full KITTI 07 comparison retains optional IMLS-SLAM and InTEn-LOAM
optimizations. Mesh-LOAM's candidate was slower and is excluded from the
implementation. **The roadmap's speed target remains open.**

The timing tables identify their measured source revisions. The subsequent
integration with merged shared components is a compatibility check; it does not
remeasure throughput on the current main revision or select registration values.

## Full paired comparison

[Committed comparison](comparisons/f4755573.json), measured implementation
`f47555735c94d43d6f8e1554ee7bf50be77ac159`; raw paired executions were committed
in `efec7cac` before deriving the comparison in `f8abcb1b`.
Each method runs the complete sequence with its selected registration arguments,
four OpenMP workers, one OpenBLAS worker and the profiler disabled. A fresh
frozen-source control immediately precedes each candidate run. Times are the
harness `time_ms`, including scan loading and preprocessing.

| Method | Fresh control [s] | Candidate [s] | Speedup | Control RTE [%] | Candidate RTE [%] | Disposition |
|---|---:|---:|---:|---:|---:|---|
| Mesh-LOAM | 1165.571 | 1397.595 | 0.83x | 0.341905 | 0.341905 | Rejected; original code retained |
| IMLS-SLAM | 1626.277 | 808.420 | 2.01x | 0.339406 | 0.339406 | Retained, opt-in/default off |
| InTEn-LOAM | 636.653 | 373.479 | 1.70x | 0.364131 | 0.364131 | Retained, opt-in/default off |

None of these attempts reaches the speed target. Official RTE is unchanged to
the third decimal for every method; all saved KITTI matrices match at their
recorded precision (nine significant digits). This does not measure differences
below that serialization precision. These are single pairs on a shared host,
not confidence intervals; the repeated development windows below show timing
variation. The earlier full baselines are preserved separately and do not define
the primary speed ratios.

## Retained code and fixed settings

The measured candidate above includes the rejected Mesh implementation. The
historical retained release is `1b51db694a137379151366b889087f1186bb08b5`: Mesh source,
header and original mechanism tests are restored to the frozen baseline, and
`--paper-pipeline-accelerated` applies only to the IMLS-SLAM and InTEn-LOAM paper
paths. The option is off by default and describes repository implementation
optimizations, not a processing step specified by either paper. That release's kernels
and mechanism tests are unchanged from the measured candidate. The harness
also fixes IMLS's reporting of effective paper-path settings.

The selected argument lists are copied unchanged from the existing fixed
variants in [the frozen plan](../../paper_pipeline_speed_kitti07_plan.json)
and `protocol.json`. Mesh's correction was selected using KITTI 00/07;
IMLS's coarse first registration was selected after the KITTI 04 failure;
InTEn's plane check/correction was selected on 07 windows with 00 influencing
the diagnosis. No registration setting was retuned for this speed experiment.
Implementation development and profiling use KITTI 07, so this full run is
not held out. Historical cross-sequence results continue to use their original
default implementations and evidence.

Old IMLS receipts round the actual bandwidth to `h=0.1` and print unused dense
startup defaults. The configured paper bandwidth is `0.06` and radius `0.20`;
the retained harness prints these effective settings accurately. This is a
reporting correction, not a registration-parameter change. The original logs
are preserved.

## Retained-code verification and final checks

The release verification was committed in `1c9adeee58f2b3b29ddaa1e611bc7382b8e160ff` before this account.
Its [commands](release_verification/1b51db69/commands.json),
[binary/library identities](release_verification/1b51db69/binary.json) and
[saved-pose comparisons](release_verification/1b51db69/trajectory_checks.json)
identify retained source `1b51db694a137379151366b889087f1186bb08b5` separately from the measured candidate.
The complete Release build and CTest pass with their actual exit codes.
Source checks confirm that Mesh matches the frozen baseline, and the retained
IMLS/InTEn implementation and test files match the measured candidate. Their
compiled static-library hashes are unchanged across the release rebuild.

The retained binary also replays the first development window for IMLS and
InTEn with the option off and on, and for Mesh with the global flag ignored.
All saved trajectories match their recorded references. These are verification
runs; their execution times are excluded from the speed ratios. Unit tests
also compare internal poses, feature order, queries and normal equations across
worker counts; serialization equality alone is not the only accuracy check.

Input cleanup and final Python/document checks retain commands and exit codes
in `cleanup_kitti07.json` and `checks/release_final_validation/`. Cleanup verifies
all scan hashes before deleting the raw sequence. The preservation check covers
historical aggregates, the paper reference data, Table 6 and published
leaderboards. Neither the speed experiment nor the retained opt-in flag
replaces historical paper-number results.

The final Python/document checks pass, and the preservation report confirms
that these historical artifacts are unchanged. The first final-check attempt
stopped before document generation because its background process could not
access the Docker socket. Its failure is retained in
`checks/release_final_validation_failed_socket/`; the same validation script
passed when launched with Docker access. No numerical result or test criterion
was changed to recover from that execution restriction.

## Profiles and development history

The rejected normal-bound candidate's [final Mesh CPU profile](runs/f4755573_profile_accelerated_mesh_loam/cpu_profile.txt)
was captured only after the unprofiled full measurements and committed in
`82da76c0`. Its sampled CPU remains concentrated in facet association; the
method README records the attribution and limits. This profile describes the
rejected candidate, not the retained Mesh implementation.

The frozen baseline is source `134ead2f21a4611ec328a45aa543b31428d6b92c`.
Its binary SHA-256, input hashes, build command and environment are recorded here.
The three completed CPU profiles were committed in `97cf50af` before describing
these observations or adding the opt-in acceleration candidates.

The KITTI 07 profiles use the first 120 frames, with four OpenMP workers and
one OpenBLAS worker. The selected Mesh-LOAM, IMLS-SLAM and InTEn-LOAM argument
lists are copied from their existing fixed variants; see `protocol.json` and
[`paper_pipeline_speed_kitti07_plan.json`](../../paper_pipeline_speed_kitti07_plan.json).
KITTI 07 is development data for this work, not held out.

The sampling profiles point to facet association in Mesh-LOAM, model projection,
scan normals and list sorting in IMLS-SLAM, and KD-tree construction/query
work in InTEn-LOAM.
`runs/baseline_profile_*/` contains the profiler input, text report, summary,
poses, exact invocation, exit codes and profile-report command. Profiled execution
times are not used to claim speedups.

The unprofiled full-sequence baselines were committed in `b66122bd`. Times
include scan loading and preprocessing (`time_ms`), with the profiler disabled:

| Method | Frames | Time [s] | FPS | Official RTE [%] | ATE [m] |
|---|---:|---:|---:|---:|---:|
| Mesh-LOAM | 1101 | 1417.883 | 0.777 | 0.341905 | 0.855800 |
| IMLS-SLAM | 1101 | 1760.844 | 0.625 | 0.339406 | 0.693085 |
| InTEn-LOAM | 1101 | 602.591 | 1.827 | 0.364131 | 1.461444 |

The first acceleration candidate, source `fe607199`, has completed build,
CTest and short-window screening. The raw screening was committed in
`8479040a` and `8fe732a7`; see
[`candidates/fe607199/screening.json`](candidates/fe607199/screening.json).

| First 120 frames | Frozen control [s] | Changed default [s] | Opt-in [s] | Control / opt-in |
|---|---:|---:|---:|---:|
| Mesh-LOAM | 117.877 | 105.840 | 168.128 | 0.70x |
| IMLS-SLAM | 160.513 | 194.906 | 79.561 | 2.02x |
| InTEn-LOAM | 45.685 | 34.105 | 18.661 | 2.45x |

All saved poses match the control exactly in this window. Timing varies even
between the two default paths; these single window runs guide development and
do not establish a full-sequence speedup. The Mesh tree candidate is a negative
result; subsequent packet and normal-bound candidates are recorded below.
The full paired comparison above determines the target status.

The first packet-cache build (`3541f773`) completed, but full CTest exited 8:
the freeze-memory assertion compared a cached frozen pipeline with an uncached
unfrozen one. The analytic gates, copied-cache behavior and exact-trajectory
tests passed. The failed test and commands were committed in `6235f89c`;
no speed runs were started for this candidate. Equal cache settings corrected
the memory comparison without weakening the assertion.

The revised packet candidate (`fe03ed88`) also proved slower: 76.264 s against
66.421 s for its frozen control (**0.87x**), with identical saved poses. Its
IMLS cell cache gave **2.46x** in the same development window. InTEn soft-pause
scheduling gave **1.33x**, below the earlier candidate's **2.45x** screening
result. The unchanged InTEn implementation in `67867d0b` later gave **4.81x**
with a much slower control (`f3c6b9f3`). Scheduling benefit is uncertain; the
final candidate excludes soft-pause calls. All windows remain in the method
READMEs rather than selecting the most favorable one. The complete raw runs were committed in
`21935329`. Sampling profiles are in `97cf50af` and `d8fed75a`; CPU attribution
is not a stage wall-time percentage.

Source `67867d0b` replaces Mesh's per-facet packet cache with a reference normal
and a conservative bound per bin. It keeps the original scalar facet loop for
survivors. Its build and full CTest pass; receipts were committed in `0d4f37ad`.
Measured source `f4755573` removes InTEn soft-pause scheduling. Its build,
full CTest, default/accelerated screening and paired full-sequence validation
all have successful command receipts under `candidates/f4755573/`.
The normal-bound Mesh candidate is excluded from the retained implementation
after its full-sequence slowdown.

Identical frozen Mesh controls took 117.877 s in the first screening and
66.421 s in the later screening. To account for execution-time variation, the
primary full comparison runs a fresh frozen-source full control immediately
before the accelerated run for each method. The earlier full baselines above
remain separate evidence. Final screening may reuse the recorded frozen short
controls, with their real run names in `screening_runs.json`; those reused
controls do not define the primary full speed ratio.

Acceleration code is opt-in and default off. Historical benchmark aggregates
and paper reference metadata are not changed by this experiment. CLI changes
parse/forward the option and report effective settings; neither `common/` nor the
runner loaders change, so the shared-code leaderboard drift audit is outside
this change's scope. CI's existing fixture drift check still applies.

Replay the frozen baseline/profile commands in `baseline_commands.json` and
candidate commands in each run's `command.json`. Their `exit_code` fields and
`candidates/<source>/checks/commands.json` record success directly. The initial
screening driver lost its stdout connection after a successful control run;
its nonzero driver receipt is retained. The resume script verified the completed
run's command/source/binary hash, reused it, and wrote subsequent logs to files.
This transport failure did not turn a failed measurement into a pass.

`helpers/` preserves the executed preparation, capture and profiling scripts.
Commands retain their original workspace paths; replace only workspace/output
prefixes when replaying. The downloader is the repository's
`evaluation/scripts/fetch_kitti_odometry_sequences.py`. Raw scans and executable
binaries are not committed. The first download attempt ended mid-stream; the
successful retry reused completed files. Profiler installation initially used an
unreachable proxy address saved in the existing container; the successful command
forwards the current session proxy for that command only. Both failures and the
successful exits are preserved.

## Integration with merged shared components

Compatibility source `82678117fd7491b15f5c0c91f215c60c3e147d11` includes main
`7505a2914328d53ac9b319d5f8042189b378d457`. Native main sources match the frozen
pre-acceleration executable source `0b079dbbcb7c6f58c1d8367ade881c15f003e525`.
IMLS links both OpenMP and the shared startup policy; its numerical stages
and the retained acceleration code preserve their order. InTEn native code is
unchanged from the previous speed branch, and Mesh native code matches main.

Raw compatibility evidence was committed in `7619f198` before these numbers:
[commands and actual exits](checks/main_refresh/native/commands.json),
[binary/input hashes and comparison](checks/main_refresh/native/comparison.json),
[source merge review](checks/main_refresh/prepare/source_merge_review.json).

Each of the 9 native runs processes the same 3 committed MCD scans, using the
old fixed argument lists without reselection. IMLS retains the coarse startup
chosen after its KITTI 04 failure; this fixture is not used to tune it. The
comparison uses four OpenMP workers and one OpenBLAS worker. All native exits
are 0. The two merged modes are compared with the frozen main executable:

| Method | Default saved poses equal main | Flag saved poses equal main | Default ATE delta [m] | Flag ATE delta [m] |
|---|---|---|---:|---:|
| IMLS-SLAM | yes | yes | 0.000000 | 0.000000 |
| InTEn-LOAM | yes | yes | 0.000000 | 0.000000 |
| Mesh-LOAM | yes | yes | 0.000000 | 0.000000 |

Every available recorded accuracy/frame field has zero difference. Mesh
ignores the global acceleration flag. Saved-pose equality is limited to its
serialization precision; the mechanism tests also compare internal poses,
feature ordering and queries across worker counts. The 3-frame fixture cannot
establish official long-segment KITTI RTE. These are compatibility executions;
their times are excluded from the speed ratios, and no full speed measurement
on the newly merged source is claimed. The historical target remains open.

The affected Release build and all four affected CTest entries pass with
actual exit 0. [Document/preservation checks](checks/main_refresh/docs/commands.json)
include the required generators, 290 Python regressions and public-document
contracts. [Preservation](checks/main_refresh/docs/preservation.json) keeps all
688 historical aggregate hashes, published README/HTML, Table 6 and paper
references unchanged. Earlier shared-component evidence is also unchanged.

The [initial whitespace failure](checks/main_refresh/prepare/initial_whitespace.json)
is retained. Historical native/profiler logs keep their original bytes through
narrow log attributes; all new raw logs are losslessly compressed with
[original/output hashes](checks/main_refresh/import.json). Native provenance
comes from executable hashes and checked source trees; `evaluator_git_commit`
is write-time metadata and does not identify a frozen executable.

Reproduce the compatibility build and runs with
`python3 experiments/results/paper_pipeline_speed/helpers/revalidate_main.py --output-dir <fresh-dir>`.
The recorded commands preserve their actual container/workspace paths.

[Post-import regeneration/checks](checks/main_refresh/post_import/commands.json) and
[preservation](checks/main_refresh/post_import/preservation.json) cover this report
and the committed compatibility evidence.
