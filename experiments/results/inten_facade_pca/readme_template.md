# Full iterative S2S facade PCA intervention

The preceding ground-PCA study leaves the early KITTI01 collapse and10 drift.
Current01 onset controls are committed before selecting this intervention.
The same native S2S solver reproduces the saved current-natural initial-seed
output exactly; GT-oracle seeds are diagnostic controls only. Facade gradients
contribute backward initial updates while the existing weak-direction prior is
inactive. This motivates a facade-normal intervention, without proving surface
correctness or an isolated S2M cause.

Only `s2s_facade_pca` differs between full arms. The new option is outside the
paper and default off. For already accepted facade triangles, the existing
ground-PCA helper fits the same in-gate ten neighbours, keeps the original
nearest-point plane anchor, and falls back to the triangle on numerical
degeneracy. All native acceptance tests, rows, gates and iteration budgets stay
fixed. At a fixed pose all counts and non-facade data equations remain exact.
The unchanged data-adaptive prior rule can immediately change its information;
subsequent poses, support and mapping also depend on the realized trajectory.

Both arms retain ground PCA, rigid mapped-motion prediction, soft prior0.03,
acceleration, original120m range/.17deg correction/.05m S2M plane check, with row
completion off. Original profile chosen on07,00 informed diagnosis; the prior
fraction came from DiLO KITTI01 tuning. The PCA construction and numerical
fallback criterion are inherited unchanged. Current01 was acquired first for
onset diagnosis; this known failure and all prior KITTI outcomes are development
data. No independent held-out claim or per-sequence option adoption is made.
Every full sequence uses the same frozen setting, starts from an empty map,
and traverses all scans continuously without GT seeding.

Only one owned full raw sequence exists at a time. The official fetch script,
input/GT hashes, actual argv/environment/exits, native producer and binary hashes,
all trace systems and saved poses are retained. Each lossless archive is
round-tripped, committed and normally pushed before owned raw scans are removed;
cleanup is committed/pushed before the next acquisition. Initial01 reuses the
owned official diagnostic acquisition after full hash revalidation. Older data,
worktrees and binaries remain unchanged. Every negative outcome is reported.
Paired processes share CPUs, so raw timings are retained without standalone FPS.

The initial collector exited1 after both native01 calls and their analyses
exited0: its settings guard removed the old ground flag instead of the facade
flag. `checks/collection_initial_failure/` retains the invocation and code.
`collection_recovery.json` seals all completed bytes; an explicit resume
corrects only the guard, verifies hashes/argv/exits and reuses those calls
without changing or repeating their native execution. The native producer,
settings and input scans are unchanged.

These orchestration scripts record this preserved workspace and its pinned
parent build libraries. They do not modify or rebuild those parents. A fresh
machine can use the standard full CMake build at the native source commit in
`protocol.json` and run the committed manifest arguments on the official inputs;
the historical boundary control requires its matching older headers/library.

Commands (actual absolute paths and exits are in adjacent receipts):

```bash
python3 experiments/results/inten_facade_pca/boundary.py
python3 experiments/results/inten_facade_pca/build.py
python3 experiments/results/inten_facade_pca/collect.py
python3 experiments/results/inten_facade_pca/report.py --output-dir experiments/results/inten_facade_pca/report
python3 experiments/results/inten_facade_pca/import_results.py
python3 experiments/results/inten_facade_pca/finalize_docs.py
```

The boundary command is phase-specific: run its recorded source commit
`20c3314e` with the immutable PR158 headers/library; later estimator headers
add a new parameter and must not be mixed with that older binary ABI. The
receipts pin each historical source and producer. Full-run source is separately
pinned in `protocol.json`. Sources and protocol are frozen before their phase; all numerical
report/import artifacts are committed before the prose below. Boundary diagnosis
uses the immutable preceding native library, separately from the new producer.
The complete Native and ROS2 CI must pass at the final PR head before Ready.

@@RESULTS@@
