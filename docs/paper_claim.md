# Paper Claim

## Core Claim

Variant-first localization benchmarking reveals Pareto fronts hidden when repositories force one canonical implementation too early. By keeping 3+ variants alive per method family under a shared CLI contract, we expose accuracy/throughput trade-offs that single-default repositories suppress, and we show empirically that the "best" default is dataset-dependent for the majority of tested method families.

## Supporting Sub-Claims

### Sub-Claim 1: Default instability is the norm (where cross-dataset coverage exists)

Across the **five** method families that currently share the **same twelve benchmark windows** (Istanbul × 3, HDL-400 × 2, KITTI Raw × 4 slices, MCD × 3 — see `docs/variant_analysis.md` §2), **every** family elects **more than one** default variant: LiTAMIN2 and CT-ICP each show **three** distinct defaults; GICP, NDT, and KISS-ICP each show **two**. Several additional families (A-LOAM, DLO, DLIO, F-LOAM, LeGO-LOAM, MULLS, Small-GICP, CT-LIO) are integrated with **smaller window coverage** today, so their cross-dataset stability is **not yet** comparable to the five above.

**Evidence:**
- `docs/variant_analysis.md` §2 — per-method default tables and **Stability:** lines (generated from current aggregates).
- `docs/decisions.md` — per-problem promotion/demotion reasoning.
- `docs/assets/paper/variant_fronts_by_selector.png` — visual default movement across selectors/datasets.

### Sub-Claim 2: The Pareto front is informative and non-trivial

The ATE vs. FPS scatter over **all elected defaults** in `docs/assets/paper/ready_defaults.csv` (one row per **ready** problem instance) spans roughly **0.005 m to 183 m** ATE and **0.4 to 106** FPS on the machine used for the stored aggregates. No single method family dominates the full front; fast scan-to-map variants (e.g., LiTAMIN2 `fast_*`, Small-GICP on KITTI) coexist with high-accuracy NDT configurations on other windows.

**Evidence:**
- `docs/assets/paper/ready_defaults_pareto.png`
- `docs/assets/paper/manuscript_core_defaults.csv` — one **manuscript-facing** representative default per **core** method family (subset used for overview figures; full cloud is `ready_defaults.csv`).

### Sub-Claim 3: A stable CLI contract makes variant-first benchmarking practical

The `pcd_dogfooding --summary-json` contract allows adding new variants datasets without branching the evaluation runner. **73** ready problems plus **1** blocked manifest (CT-LIO GT-backed readiness) are tracked in `experiments/results/index.json`, all driven through `run_experiment_matrix.py` / `refresh_study_docs.py`.

**Evidence:**
- `docs/interfaces.md` — stable core contract.
- `experiments/results/index.json` — problem list, `current_default`, and `status`.
- `PLAN.md` — current method-family count (**13** wired into `pcd_dogfooding`).

### Sub-Claim 4: Reference-based evaluation extends coverage where GT is unavailable

CT-LIO GT-backed evaluation is blocked due to missing repository-aligned GT CSV for the public HDL-400 LiDAR+IMU window, but **reference-based** comparison against other methods' trajectories still provides ranking signal, showing graceful degradation of the framework.

**Evidence:**
- `experiments/results/ct_lio_public_readiness_matrix.json` — `blocked` + documented `blocker`.
- `experiments/results/ct_lio_reference_profile_matrix.json` — reference-based CT-LIO results.
- `docs/assets/paper/manuscript_core_defaults.csv` — CT-LIO row marked `reference-based`.

## Evidence Summary Table

| Evidence File | What It Shows |
|---------------|---------------|
| `experiments/results/index.json` | **73** ready + **1** blocked problems; per-problem defaults |
| `docs/variant_analysis.md` | GT-seed ablation, cross-dataset default stability, profile impact |
| `docs/decisions.md` | Variant lifecycle and adoption rules |
| `docs/assets/paper/ready_defaults.csv` | All ready-problem defaults — ATE, FPS, dataset tag |
| `docs/assets/paper/ready_defaults_pareto.png` | ATE vs. FPS Pareto scatter |
| `docs/assets/paper/variant_fronts_by_selector.png` | Default movement across datasets |
| `docs/assets/paper/default_variant_matrix.csv` | Wide Table 3 — method × dataset slug → default variant |
| `docs/assets/paper/default_variant_instability.png` | Green/red heatmap vs row plurality (Figure 4) |
| `docs/assets/paper/manuscript_core_defaults.csv` | One representative default per **overview** method family |
| `docs/paper_tracks.md` / `docs/paper_comparison.md` | Full tables derived from aggregates |
