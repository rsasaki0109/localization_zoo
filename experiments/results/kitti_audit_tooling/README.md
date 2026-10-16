# KITTI audit tooling validation

These records validate audit selection, replay receipts and exit codes. They do
not constitute a full KITTI audit or a new method accuracy/speed experiment.
Native algorithms, `common/`, loaders and registration settings are unchanged.

The [implementation protocol](protocol.json) pins the implementation commit and
source hashes. [Targeted commands](targeted/commands.json) record actual commands
and checked exit codes; their stdout/stderr logs are retained alongside them.
Those checks ran immediately before the implementation commit, and their tested
source files match the committed hashes.

The CLI regressions run isolated, deliberately artificial result cells with a
controlled replay executable. They exercise selection, command reconstruction,
completed drift, failed processes, invalid summaries, missing inputs and stale
work directories. Artificial metrics are test inputs, not method measurements.

[User procedure and exit-code contract](../../../docs/kitti_drift_audit.md).
The historical full audit and published result aggregates are preserved.
