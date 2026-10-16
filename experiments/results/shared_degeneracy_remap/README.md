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
