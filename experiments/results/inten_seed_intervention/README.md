# Fixed-pair S2S initialization interventions

Changing only the initial transform changes the S2S result on the same real
scan pair. A previous mapped-motion seed recovers few selected bad cases;
starting at the recorded GT motion recovers many more, but still leaves
failures, including the KITTI02 sustained-collapse entries. Initialization
alone does not explain those cases. Initial class systems show a backward
ground contribution in KITTI02 even at GT. This narrows the investigation;
it does not establish a complete correspondence-level cause or a working fix.

## What is intervened on

This follows the [complete natural traces](../inten_natural_traces/README.md)
in PR #152, at parent `68c23290`. The existing public forced-pose diagnostic
API is used in a separate, manually compiled program outside the harness.
For every selected pair, a fresh object indexes the previous scan, then solves
the current scan from each of the following seeds:

- `natural`: the passive full-run S2S input, as a replay control;
- `previous_mapped`: the previous accepted final-pose increment from that same
  baseline run, reused as a constant-motion prediction;
- `oracle_current_gt`: the current pair's saved GT increment, a diagnostic
  oracle unavailable to an estimator.

These are forced initial-condition interventions, **not natural full-sequence
accuracy runs**. Mapping is disabled identically in the probe to isolate S2S;
its equivalence to the natural S2S is checked below. The native solver, feature
extraction, gates, iteration budget, damping, range and elevation settings are
otherwise unchanged. No estimator/default/runner API or benchmark cell changes.
The original plane/elevation values were selected on KITTI07, with KITTI00
influencing the earlier diagnosis. The known 01/02 failures guide this
diagnosis; they are not held out.

[Protocol](protocol.json), native [probe](probe.cpp), verbatim pinned
[loader fragments](native_loader.h) and [driver](run.py) were committed before
data execution (`cd4185e6`). The probe links the immutable library from the
prior fresh Release build; resolved CMake compile/link metadata, library and
executable hashes, exact commands and exits are retained in [build/](build/)
and [binary.json](binary.json), committed at `356eaa8f`.

## Coverage and controls

Selection includes **every** natural S2S or final bad-length step and its
immediate neighbours, plus the first step and every inherited 100-step-block
endpoint on all three sequences. The bad-length criterion remains relative
error above 20% with GT translation at least 0.1 m. No successful subset is
selected. These retrospective diagnostic samples are not representative
full-sequence accuracy estimates.

| Sequence | Selected pairs | Native pair evaluations | Moving pairs |
|---|---:|---:|---:|
| 01 | 155 | 465 | 155 |
| 02 | 192 | 576 | 192 |
| 10 | 13 | 39 | 12 |

All native processes and replay controls exit 0. Across all 360 pairs, natural
S2S outputs agree **exactly** with the original continuous execution. First
class counts match; the maximum absolute differences after summing the class
systems are `1.382432e-10` for H and `2.842171e-13` for g (floating-point
reduction order). Thus the fresh-pair control reproduces the realized S2S
despite not reconstructing the map or a long trajectory.

The full-stream preparation also checks that every incoming S2S guess equals
the previous accepted S2S relative transform. S2M correction is not fed back
into that stored guess in the current implementation. This is a measured
data-flow observation, not proof that changing the recurrence fixes accuracy.

Official scans are fetched by the existing script, one full sequence at a
time. All scan/timestamp hashes match the committed natural-run inputs and
are rechecked before deleting only the owned scans. Every sequence closes
with a verified archive, cleanup record, commit and normal backup before the
next is fetched. Raw sequence evidence precedes this prose: 01 `e30917d1`,
02 `35d3f87b`, 10 `fd085cb5`. The terminal [collection exit](collection_exit.json)
is 0. No raw scan is committed.

## Results, including failures

Numbers below count outputs exceeding the inherited local-motion criteria
over all selected **moving** pairs. Each cell is **bad length / bad vector**;
these are local diagnostic counts, not KITTI RTE or full-trajectory results.

| Sequence | Natural seed | Previous mapped seed | Current GT oracle |
|---|---:|---:|---:|
| 01 | 139 / 139 | 136 / 136 | 1 / 1 |
| 02 | 136 / 137 | 134 / 135 | 33 / 34 |
| 10 | 0 / 0 | 0 / 0 | 0 / 0 |

The one selected nonmoving pair in 10 remains in the raw outputs and CSV but
has unavailable relative-error labels. The 02 vector-only failure is retained;
passing the length criterion alone does not imply correct motion.

Most baseline-bad cases remain bad with a one-shot previous-mapped seed. That
seed often comes from an already-collapsed baseline step: its median length/GT
ratio on the baseline-bad group is 0.117175 in 01 and 0.047187 in 02. These
tests **cannot reject or validate a recursive mapped-motion predictor** that
might prevent entry earlier. No such candidate is selected here.

The following table includes every S2S bad-length entry (zero-based destination
frame), including isolated overestimates. S2S entries differ from the final
S2M bad-step boundaries in the parent study. Ratios are output translation
length divided by GT length; the mapped-seed column also shows its input.

| Sequence / frame | Natural output | Previous mapped seed → output | GT seed → output |
|---|---:|---:|---:|
| 01 / 215 | 0.792821 | 0.931281 → 0.746403 | 1 → 0.820116 |
| 01 / 217 | 0.533110 | 0.858696 → 0.952078 | 1 → 0.954337 |
| 01 / 419 | 0.779435 | 1.007633 → 0.895524 | 1 → 0.895578 |
| 02 / 1789 | 1.213234 | 1.276295 → 1.213239 | 1 → 1.213213 |
| 02 / 1876 | 0.775259 | 1.037713 → 0.801285 | 1 → 0.785598 |
| 02 / 3308 | 1.276501 | 1.369083 → 1.276990 | 1 → 1.277238 |
| 02 / 4460 | 0.342471 | 1.021851 → 0.403473 | 1 → 0.404926 |

At 02 frame 4460, both the mapped seed and the saved-GT seed start within the
length-error criterion, yet the S2S solution loses most of the motion. At 01 frame
420 (inside the second S2S bad run), the GT-seeded output ratio is 0.712260.
These cases rule out a wrong initial motion as the **sole** explanation of
their fixed-pair failure. They do not identify which correspondence, surface,
intensity sample or GT imperfection causes it. The oracle is not deployable.

## First-system attribution at the oracle seed

The [report protocol](report_protocol.json) was committed at `012e9594` before
offline computation; results at `9b4051fd` precede this prose. For every stored
first system, it solves

`delta = -(sum(H_c) + 1e-6 I)^-1 sum(g_c)` and
`delta_c = -(sum(H_c) + 1e-6 I)^-1 g_c`.

The contributions share the **same total damped H** and sum to the joint step.
They are not independent class solves or class-removal ablations. The reported
translation is the first-order sensor-origin displacement
`delta_v + delta_omega × seed_translation`, projected along GT motion and
divided by its length. A negative value pushes backward along that motion.
This is neither the exact finite update nor the final repeated-association
trajectory.

| Oracle case | Edge pairs | Edge [%] | Ground [%] | Facade [%] | Intensity [%] | Joint [%] |
|---|---:|---:|---:|---:|---:|---:|
| 01 / 420 | 79 | -1.536647 | -0.100071 | -2.484957 | +0.174067 | -3.947608 |
| 02 / 1876 | 0 | 0 | -3.724821 | -1.489380 | -0.212141 | -5.426342 |
| 02 / 4460 | 0 | 0 | -4.308664 | +0.071834 | -0.540669 | -4.777499 |

Values are percentages of GT step length. All per-class vectors/counts and
additive controls are in the report. The zero edge support
at the two 02 collapse entries is observed **at the GT seed**, despite large
total support. Their initial backward ground contribution motivates inspecting
the actual ground-plane correspondences/normals next. It does not establish
that removing a class would help: changing a class also changes H and later
associations. KITTI01 has a different initial contribution pattern, so a single
ground-based explanation is not established across both sequences.

## Evidence and reproduction

Each `sequences/NN/archive.json` retains original sizes/hashes and the map to
stored files. `cases.json` retains every seed, GT transform and natural replay
target. `native_input.txt` and `native.stdout` retain the exact input/output of
the native process, including first systems by class. `*_command.json` records
actual argv/cwd/source/environment/exit. No native operation is inferred from
the end of a log. Cleanup is recorded separately after lossless archival.

The original native collection command is:

```bash
python3 experiments/results/inten_seed_intervention/run.py
```

It requires the pre-execution source `cd4185e6`, recorded environment, immutable
parent library/build metadata and fresh runtime/output paths. The current
result-bearing checkout intentionally refuses collection restart. This is a
study-specific reproduction record, not a portable dataset/runner interface.

The offline report needs only committed archives and locked Python dependencies:

```bash
OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 python3 experiments/results/inten_seed_intervention/report.py --output-dir /tmp/new-inten-seed-report
```

[Summary](report/summary.json), per-sequence case CSVs, boundary records and
first-step decompositions retain every result and unavailable marker. Alternate
stop reasons and last-iteration counts are not exposed by the existing forced
diagnostic API and are not inferred.

For angular reporting only, the offline report projects the rounded stored
rotation blocks to SO(3) and uses an `atan2` relative angle. No projected value
is fed into the solver or length/vector labels. The original collection CSV
retains its raw trace-based rotation proxy, which can show a small nonzero
floor even for an identical GT seed; use the corrected report for angles.

Initial Python parse, missing pkg-config metadata and linker-option attempts
each exit 1 **before scan acquisition/native evaluation**. They are retained
under [preflight_failures/](preflight_failures/). The successful build reuses
the pinned resolved CMake metadata, preserving linker options and redirecting
source includes to the checked worktree. These failures are not silently
relabelled as successful tests.

## Validation and preservation

Committed evidence `f14b1644` records Python discovery of 344 tests with
5 existing built-evaluator skips and exit 0. The three required generators,
showcase, leaderboard and paper-ready checks also exit 0. Outcome and entry
tables are checked against the committed machine report.

[Preservation checks](checks/final/preservation.json) verify all 688 historical
aggregate hashes, root README, HTML, Table 6, full-variant CSV and paper-number
JSON against parent `68c23290`. Native code, manifests, existing tools, parent
natural evidence and the pinned libraries/executables are unchanged. Generator
changes are timestamps only. Full native/ROS CI is tracked in the result PR.

## Remaining work

The fixed-pair intervention milestone is complete. The next causal question is
why the same solver leaves the GT neighbourhood on the affected pairs: inspect
the actual ground-plane associations/normals in 02 and the different edge/
facade contribution in 01, with controls that preserve source/target data.
The first-system attribution does not trace subsequent correspondence changes.
Any proposed modification still needs a separately declared, default-off
candidate and one fixed setting across full sequences; no new paper ratio,
trajectory benefit or general rejection rule is claimed here.
