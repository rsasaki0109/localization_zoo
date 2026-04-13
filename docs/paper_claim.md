# Paper Claim

## Core Claim

Variant-first localization benchmarking reveals Pareto fronts hidden when repositories force one canonical implementation too early. By keeping 3+ variants alive per method family under a shared CLI contract, we expose accuracy/throughput trade-offs that single-default repositories suppress, and we show empirically that the "best" default is dataset-dependent for the majority of tested method families.

## Supporting Sub-Claims

### Sub-Claim 1: Default instability is the norm (where cross-dataset coverage exists)

Across the **five** method families that currently share the **same twelve benchmark windows** (Istanbul × 3, HDL-400 reference × 2, KITTI Raw × 4 slices, MCD × 3 — see `docs/variant_analysis.md` §2), **every** family elects **more than one** default variant: LiTAMIN2 and CT-ICP each show **three** distinct defaults; GICP, NDT, and KISS-ICP each show **two**. Several additional families are integrated with **smaller or uneven window coverage** today, so their cross-dataset stability is **not yet** comparable to the five above.

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

The `pcd_dogfooding --summary-json` contract allows adding new variants and new benchmark windows without branching the evaluation runner. The current index tracks **166** ready problems, **1** blocked manifest, and **1** skipped manifest across **27** active selectors, all driven through `run_experiment_matrix.py` / `refresh_study_docs.py`.

**Evidence:**
- `docs/interfaces.md` — stable core contract.
- `experiments/results/index.json` — problem list, `current_default`, and `status`.
- `docs/interfaces.md` / `PLAN.md` — current selector set and public handoff notes.

### Sub-Claim 4: Reference-based evaluation extends coverage where GT is unavailable

CT-LIO GT-backed evaluation is blocked due to missing repository-aligned GT CSV for the public HDL-400 LiDAR+IMU window, but **reference-based** and **public ROS1 synthetic-time** comparisons still provide ranking signal, showing graceful degradation of the framework. Those public ROS1 synthetic-time results for CT-ICP, CT-LIO, and CLINS should be treated as separate public-only evidence, not as exact native-time reproduction.

**Evidence:**
- `experiments/results/ct_lio_public_readiness_matrix.json` — `blocked` + documented `blocker`.
- `experiments/results/ct_lio_reference_profile_matrix.json` — reference-based CT-LIO results.
- `experiments/results/clins_hdl_400_public_ros1_synthtime_matrix.json` — public ROS1 synthetic-time CLINS evidence.
- `docs/assets/paper/manuscript_core_defaults.csv` — CT-LIO row marked `reference-based`.

## Evidence Summary Table

| Evidence File | What It Shows |
|---------------|---------------|
| `experiments/results/index.json` | **166** ready + **1** blocked + **1** skipped problems; per-problem defaults |
| `docs/variant_analysis.md` | GT-seed ablation, cross-dataset default stability, profile impact |
| `docs/decisions.md` | Variant lifecycle and adoption rules |
| `docs/assets/paper/ready_defaults.csv` | All ready-problem defaults — ATE, FPS, dataset tag |
| `docs/assets/paper/ready_defaults_pareto.png` | ATE vs. FPS Pareto scatter |
| `docs/assets/paper/variant_fronts_by_selector.png` | Default movement across datasets |
| `docs/assets/paper/default_variant_matrix.csv` | Wide Table 3 — method × dataset slug → default variant |
| `docs/assets/paper/default_variant_instability.png` | Green/red heatmap vs row plurality (Figure 4) |
| `docs/assets/paper/manuscript_core_defaults.csv` | One representative default per **overview** method family |
| `docs/paper_tracks.md` / `docs/paper_comparison.md` | Full tables derived from aggregates |
