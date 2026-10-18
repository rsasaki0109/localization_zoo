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

## Native fixture replay

[Committed native results](real_fixture/comparison.json) exercise the actual
Release `pcd_dogfooding` executable with KISS-ICP and `--no-gt-seed` on the
committed three-frame MCD fixture. A fresh native run supplies the reference;
the negative cases deliberately alter only that isolated reference or omit the
input link. No method parameter is tuned.

| Audit case | Expected exit | Observed exit | Result |
|---|---:|---:|---|
| Unchanged reference | 0 | 0 | matches |
| Reference ATE increased by 1 m | 1 | 1 | ATE drift |
| Reference frame count increased by 1 | 1 | 1 | frame-count drift |
| Reference RPE increased by 1 percentage point | 1 | 1 | RPE drift |
| Missing input directory | 2 | 2 | preflight error; no replay process |

[Commands and process exits](real_fixture/commands.json),
[input/binary hashes](real_fixture/inputs.json), baseline summary, per-case
reference snapshots, reports and replay logs are retained. The isolated input
directories have KITTI-shaped names only to exercise the existing selector;
they contain the MCD fixture. These are not KITTI measurements. Null official
RTE in the short fixture means there are no valid official-length segments.

The [validation driver](validate_real_fixture.py) is the exact executed script.
It uses the recorded `/workspace` paths and requires a fresh
`/workspace/localization-env/kitti-audit/real_fixture` scratch directory. The
launch command and environment choices are in [the launcher receipt](real_fixture/launcher.json).
The replay paths in the raw reports name that original scratch directory; their
imported summary/log files are the corresponding `*_replay/` directories here.

## Release and regression checks

[Committed check receipts](checks/commands.json) record a successful Release
configure/build, all 122 CTest entries, all 290 Python regressions, showcase,
leaderboard, paper-ready, syntax and whitespace checks. Every process exit was
checked directly. The implementation source files remain identical to the
protocol's pinned commit.

[Preservation checks](checks/preservation.json) confirm that all 688 historical
matrix aggregates, `paper_reported_numbers.json`, Table 6 CSV, the root README,
`docs/index.html` and the historical full drift audit are unchanged. No new KITTI
scans were fetched for this tooling change. Shared robustness extraction and its
full-sequence evaluation remain separate work; this PR does not close them.

[Final regeneration receipts](final_checks/commands.json) cover
`refresh_study_docs.py`, `generate_full_variant_table.py` and
`generate_paper_ratio_table.py`, followed by showcase, leaderboard and paper-ready
checks. Regeneration updates inventory timestamps; the
[final preservation report](final_checks/preservation.json) confirms that the
historical aggregates and published numerical tables remain unchanged.

[User procedure and exit-code contract](../../../docs/kitti_drift_audit.md).
The historical full audit and published result aggregates are preserved.
