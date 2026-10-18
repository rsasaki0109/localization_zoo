# Shared initial coarse registration

The second shared-robustness component in roadmap item 6 shares the startup
stage policy between IMLS-SLAM and opt-in SuMa. Correspondences, residuals,
normal gates and numerical stopping remain specific to each method; this is
not a shared numerical aligner. Both native defaults remain off.

[Protocol](protocol.json) fixes the SuMa arguments before execution. The first
registration and coarse iteration budget derive from the existing IMLS-SLAM
04 choice. The distance multiplier derives from its coarse/fine radius ratio;
SuMa keeps its own Huber residual and projective associations. No SuMa sweep
or new held-out claim is made: its prior full KITTI results were already known.
The previous SuMa translation-remap experiment is not combined with this one.

Full KITTI 00–10 evaluation and the affected-method compatibility checks are
complete. [Accuracy](report/table.md), [diagnostics](report/diagnostics.md),
[CSV](report/comparison.csv) and the [immutable input snapshot](report/summary.json)
retain every sequence. RTE is higher on 05, 09, 10; ATE is higher on 03, 06, 07, 10.
The full table retains these tradeoffs without reselection. Commands and actual exits are retained. Experimental results are not
promoted to the published leaderboard, Table 6 or paper-reference metadata.
Near-range sharing and NCLT remain separate work. The NCLT host remains absent
from the enforced policy despite earlier approval; this work does not retry
that approval request or substitute synthetic scans for real validation.

Single-variant matrix aggregates retain the runner's automatic adoption label.
That label ranks only the experimental variant; the separately measured raw
control determines any transfer recommendation, and defaults stay off.

[PR #136](https://github.com/rsasaki0109/localization_zoo/pull/136) is stacked on
the previous remap PR. Native implementation and binary hashes are pinned in
[`candidate.json`](candidate.json); later commits retain evidence and docs.
The [preflight commands and actual exits](checks/preflight/commands.json) record
all 124 CTest entries and 290 Python regressions passing, plus source/publication
preservation and the expected refusal of incomplete full-coverage reporting.
[Fixture comparisons](wrapper_validation/comparison.json) retain frozen
pre-change versus candidate and direct/wrapped parity. Short-fixture RTE fields
without eligible segments do not establish full KITTI accuracy.
The initial nonzero build attempt was a cache-ownership failure, before source
compilation; its logs are retained under `checks/development_attempt_01_permissions/`.
Rebuilding as the existing cache owner succeeded without changing file ownership.

The [full study launch](attempts/initial_launch.json) records the checked source
and controller command. `run_all.py` uses fresh artifact directories; fetch-only
continuation validates committed previous sequences and refuses existing native
artifacts. It is not a general native-run resume. Exact per-cell native argv,
thread settings, binary hashes and process exits are retained by `native_wrapper.py`.

<!-- FETCH_RECOVERY:START -->
Official acquisition encountered truncated HTTP Range responses before native
artifacts. [Failure and launch receipts](attempts/) retain each attempt.
[Read-only continuation guards](checks/) first validated preceding committed
comparisons/cleanup, absence of native artifacts and frozen source/binary identity.
Failure and guard evidence was committed before each continuation launch.
Only the official downloader's CRC-checked complete files were reused, with
native settings unchanged. These transport failures are distinct from method
accuracy or native execution failures.
<!-- FETCH_RECOVERY:END -->

The full report was generated with the default all-sequence guard from committed
inputs before updating numeric documentation:

```bash
python experiments/results/shared_coarse_start/summarize_results.py --output-dir experiments/results/shared_coarse_start/report
```

[Exact command and checked exit](report/command.json) retain the Python path and
cwd. [Final controller completion](attempts/final_completion.json) records full
coverage and input cleanup. The report verifies complete native argv, executable
hashes, thread counts, full frames, GT CSV identity, saved-pose RTE agreement and
old official translation/rotation RTE. Official scan .bin hashes/bytes and GT CSV
match the prior study; all fresh raw controls match its accuracy/frame metrics.
Explicit historical partial reports do not replace the complete report.

All 22 selected existing IMLS-SLAM/SuMa cells have zero changes in RPE, ATE,
frame count and official translation/rotation RTE. The frozen pre-change full 04 comparison also has identical saved-pose bytes
for both methods.
[Archived documentation writer](checks/final/documentation_driver.py) records the
one-time derivation from the committed report; the final text includes formatting
review. [Final regeneration and checks](checks/final/commands.json) retain the required
three doc generators and actual exits. [Preservation](checks/final/preservation.json)
checks all historical aggregates, published README/HTML/Table 6/paper metadata,
fixed manifest settings, native source and the previous remap experiment.
[01 figure](plots/01/trace.png), [numeric trace](plots/01/trace.csv) and
[provenance](plots/01/provenance.json) preserve the remaining later drift, with
no path fitting or rescaling. Concurrent independent runs do not support speed
comparisons. Near-range sharing and NCLT remain open; this completes only the
second roadmap component.
