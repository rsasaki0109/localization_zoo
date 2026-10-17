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

Full KITTI 00–10 evaluation and affected-method compatibility checks will be
recorded here, with commands and actual exits. Experimental results are not
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
