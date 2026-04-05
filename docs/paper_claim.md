# Paper Claim

## Core Claim

Variant-first localization benchmarking reveals Pareto fronts hidden when repositories force one canonical implementation too early. By keeping 3+ variants alive per method family under a shared CLI contract, we expose accuracy/throughput trade-offs that single-default repositories suppress, and we show empirically that the "best" default is dataset-dependent for the majority of tested method families.

## Supporting Sub-Claims

### Sub-Claim 1: Default instability is the norm

For 4 out of 6 method families, the elected default variant changes when the evaluation dataset changes. This is not noise; it reflects genuine shifts in the accuracy/throughput Pareto front across environments.

**Evidence:**
- `experiments/results/index.json` -- 26 ready problems show different `current_default` values for the same method across datasets (e.g., LiTAMIN2 elects `fast_icp_only_half_threads` on Istanbul but `paper_icp_only_half_threads` on HDL-400 window a; CT-ICP elects `fast_window` on HDL-400 but `balanced_window` on Istanbul b/c; KISS-ICP elects `dense_local_map` on Istanbul-b but `fast_recent_map` elsewhere).
- `docs/decisions.md` -- Generated decision tables show per-problem promotion/demotion reasoning.
- `docs/assets/paper/variant_fronts_by_selector.png` -- Visual evidence of default movement across datasets.

### Sub-Claim 2: The Pareto front is informative and non-trivial

The ATE vs. FPS Pareto front across all 26 ready defaults spans a wide range (0.005 m to 182.9 m ATE; 0.4 to 23.5 FPS) and is not dominated by a single method family. Fast profiles from LiTAMIN2 and GICP coexist on the Pareto front with high-accuracy NDT configurations.

**Evidence:**
- `docs/assets/paper/ready_defaults_pareto.png` -- Pareto scatter plot of all ready defaults.
- `docs/assets/paper/manuscript_core_defaults.csv` -- One representative default per method family with ATE and FPS columns.

### Sub-Claim 3: A stable CLI contract makes variant-first benchmarking practical

The `pcd_dogfooding --summary-json` contract allows adding new variants and datasets without changing the evaluation pipeline. The 26 ready problems were generated and evaluated through a single `run_experiment_matrix.py` entry point.

**Evidence:**
- `docs/interfaces.md` -- Stable core contract specification.
- `experiments/results/index.json` -- 26 ready + 1 blocked problems all conforming to the same contract.
- `docs/decisions.md` -- Rules 1-4 governing variant lifecycle.

### Sub-Claim 4: Reference-based evaluation extends coverage where GT is unavailable

CT-LIO GT-backed evaluation is blocked due to missing aligned GT CSV, but reference-based comparison against other methods' trajectories still provides useful ranking signal, demonstrating that the framework degrades gracefully.

**Evidence:**
- `experiments/results/index.json` -- `ct_lio_public_gt_readiness` has status `blocked` with documented blocker; `ct_lio_reference_tradeoff` has status `ready`.
- `experiments/results/ct_lio_reference_profile_matrix.json` -- Reference-based CT-LIO results.
- `docs/assets/paper/manuscript_core_defaults.csv` -- CT-LIO included via reference-based path.

## Evidence Summary Table

| Evidence File | What It Shows |
|---------------|---------------|
| `experiments/results/index.json` | 26 ready + 1 blocked problem instances with per-problem defaults |
| `docs/decisions.md` | Per-problem variant promotion/demotion decisions and lifecycle rules |
| `docs/assets/paper/ready_defaults_pareto.png` | ATE vs. FPS Pareto scatter of all ready defaults |
| `docs/assets/paper/variant_fronts_by_selector.png` | Per-method default movement across datasets |
| `docs/assets/paper/manuscript_core_defaults.csv` | One representative default per method family |
| `docs/assets/paper/manuscript_core_methods.png` | Core methods visualization |
| `docs/paper_tracks.md` | Full results table with ATE, FPS, dataset, and status per problem |
