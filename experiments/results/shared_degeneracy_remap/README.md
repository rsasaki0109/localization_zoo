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
