# Shared local translation degeneracy remap

This experiment evaluates the first shared-robustness component in roadmap item
6: the existing DiLO opt-in remap, extracted into
[`common/degeneracy.h`](../../../common/include/common/degeneracy.h) and reused
by SuMa. Coarse initial registration, near-range filtering and NCLT evaluation
remain separate work. Both native defaults stay off.

[Protocol](protocol.json) records the fixed configuration, selection provenance,
affected-method audit scope, input lifecycle and publication boundary.
[Candidate](candidate.json) and [baseline](baseline.json) identify the frozen
native binaries. The native implementation is pinned independently of subsequent
evidence commits. New evidence stays in this directory and does not promote a
variant into the published leaderboard, Table 6 or paper reference metadata.

`run_all.py` records the exact commands and checked process exits. It uses the
official KITTI downloader for one sequence at a time, audits the shared header's
native consumers, evaluates the fixed SuMa variant, diagnoses saved trajectories
and verifies input hashes before deleting only its owned downloaded scans.
Completed sequence records are committed as they become available.
The controller uses fresh artifact directories; it is an execution record, not
a resume command. For a replay, retain the committed evidence and use a new
output directory with a binary built from the recorded implementation commit.
Per-cell command receipts specify the full native arguments and input sequence.

Independent runs use `native_wrapper.py` to isolate their output working
directories. It forwards native arguments unchanged and records the actual
native executable, argv, working directory, binary hash and exit code. Direct
versus wrapped fixture records are under `wrapper_validation/`. Concurrency and
host variation make these runs unsuitable for speed claims.

Full-sequence execution is in progress. The [committed intermediate snapshot](checks/reporting_00_02/report/summary.json)
contains paired full 00–02 runs and their affected-method audits. It remains
explicitly incomplete and contains no cohort paper ratio or speed claim.

SuMa 01 is negative: official RTE rises from 13.792690% to 95.257580%.
[Its diagnostics](sequences/01/candidate_drift.json) show startup failure.
[The full 02 comparison](sequences/02/comparison.json) is also negative:
RTE rises from 3.511588% to 21.423956%, ATE from 70.307811 m to 565.520007 m.
Length errors above 20% affect destination frames 1–912 continuously at startup
and 2040 moving steps overall. The same fixed configuration continues.

Existing audit cells match RPE, ATE and frame counts; the intermediate report
also verifies official translation and rotation RTE against committed selected
aggregates. The [frozen full 01 comparison](sequences/01/frozen_comparison.json)
has identical saved-pose bytes and zero printed-metric deltas for both methods.
Per-sequence cleanup records verify input hashes before removing owned scans.

[Release receipts](checks/commands.json) record all 123 CTest entries and all 290
Python regressions passing, along with source identity, showcase, leaderboard,
paper-ready and whitespace checks. Actual commands and process exits are kept
with the logs. Generated evidence is collapsed in GitHub diffs; the handwritten
controller, wrapper, protocol and native implementation remain reviewable.

`summarize_results.py --output-dir <new-directory>` requires committed coverage
of all requested sequences. It reads one immutable Git snapshot and verifies
native binary hashes, complete argument lists, thread settings, frame counts,
diagnostic definitions, audit status and owned-input cleanup. It writes JSON,
CSV and a method-README table without a paper ratio or a speed claim.
`--allow-partial` produces an explicitly incomplete interim report. The
[coverage-guard check receipts](checks/reporting/commands.json) exercise refusal
of incomplete coverage and explicit partial reporting against the committed
first sequence; the source files and execution commands are retained.
The report retains relative endpoint errors and the largest positive pointwise
position-error increases, each with GT travel distance and frame bounds. This
keeps low-motion relative errors in context. The
[partial error-growth report and checked execution](checks/reporting_error_growth/)
use committed inputs and remain explicitly incomplete.
The report additionally requires both official translation and rotation RTE
of the existing audit cells to match their committed selected aggregates,
supplementing the audit CLI's RPE, ATE and frame-count checks.
[The partial execution receipt](checks/reporting_official_metrics/command.json)
records this check against the completed sequences.
Both `table.md` and `diagnostics.md` are rendered from the report rows for use
in the method README. [The table-generation execution](checks/reporting_tables/command.json)
and its partial outputs retain the input snapshot, commands and checked exit.

[`plot_sequence.py`](plot_sequence.py) plots saved full-sequence poses from one
committed snapshot. The [01 trace](plots/01/trace.png),
[SVG](plots/01/trace.svg), [numeric trace](plots/01/trace.csv) and
[provenance](plots/01/provenance.json) show successive displacement and position
error in the saved evaluation frame. They do not fit or rescale either path;
the position-error trace is distinct from the official segment-based RTE.
[Command and checked exit](plots/01/command.json) accompany the figure.
The [02 trace](plots/02/trace.png), [SVG](plots/02/trace.svg),
[numeric trace](plots/02/trace.csv), [provenance](plots/02/provenance.json)
and [checked command](plots/02/command.json) use its committed paired full run.

`verify_preservation.py --output <receipt.json>` compares historical top-level
aggregate hashes and the published README, HTML, Table 6 and paper-reference
bytes with the recorded base. It also checks native source identity and the
same arguments and input-path pattern in every new manifest. The
[interim receipt](checks/preservation_interim.json) passes; this is independent
of benchmark completion and will be repeated after the final doc regeneration.
