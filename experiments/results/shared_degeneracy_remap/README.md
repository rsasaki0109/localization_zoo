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

Full-sequence execution is in progress. Sequence results, preservation checks,
negative findings and completion status will be summarized after the full fixed
configuration and affected-method audit finish.

The [committed full 01 comparison](sequences/01/comparison.json) is negative for
SuMa: official RTE rises from 13.792690% to 95.257580% with the transferred
setting. Its [saved-pose diagnostics](sequences/01/candidate_drift.json) show
short steps from startup; the other sequences continue with the same setting.
The [frozen baseline comparison](sequences/01/frozen_comparison.json) has zero
printed-metric differences and identical saved-pose bytes for both existing
audit variants. [Input cleanup](sequences/01/cleanup.json) verifies hashes before
the downloaded sequence is removed.

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

[`plot_sequence.py`](plot_sequence.py) plots saved full-sequence poses from one
committed snapshot. The [01 trace](plots/01/trace.png),
[SVG](plots/01/trace.svg), [numeric trace](plots/01/trace.csv) and
[provenance](plots/01/provenance.json) show successive displacement and position
error in the saved evaluation frame. They do not fit or rescale either path;
the position-error trace is distinct from the official segment-based RTE.
[Command and checked exit](plots/01/command.json) accompany the figure.

`verify_preservation.py --output <receipt.json>` compares historical top-level
aggregate hashes and the published README, HTML, Table 6 and paper-reference
bytes with the recorded base. It also checks native source identity and the
same arguments and input-path pattern in every new manifest. The
[interim receipt](checks/preservation_interim.json) passes; this is independent
of benchmark completion and will be repeated after the final doc regeneration.
