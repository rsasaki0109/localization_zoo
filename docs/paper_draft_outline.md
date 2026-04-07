# Paper Draft Outline

**Working title**: Variant-First Localization Benchmarking Reveals Pareto Fronts Hidden by Single-Canonical Repositories

**Thesis**: Variant-first localization benchmarking reveals Pareto fronts hidden when repositories force one canonical implementation too early.

---

## 1. Introduction

**Problem.** Most open-source LiDAR localization repositories ship a single canonical configuration per method. Users must accept the upstream default or hand-tune parameters without visibility into the accuracy/throughput trade-off space. This hides useful Pareto fronts and makes cross-method comparison misleading.

**Contribution.**
- A _variant-first_ benchmarking framework that keeps 3+ concrete variants alive per method family under a shared CLI contract (`pcd_dogfooding --summary-json`).
- An empirical study across **13** integrated method families — LiTAMIN2, GICP, Small-GICP, NDT, KISS-ICP, DLO, DLIO, CT-ICP, CT-LIO, A-LOAM, F-LOAM, LeGO-LOAM, MULLS — and **four** public dataset families (**Istanbul**, **HDL-400**, **MCD**, **KITTI Raw**), with **73** ready search problems in `experiments/results/index.json` (plus one blocked CT-LIO GT-backed manifest).
- Evidence that, for every family with **broad** cross-window coverage today (LiTAMIN2, GICP, NDT, KISS-ICP, CT-ICP on twelve shared windows), the elected default **is not unique** across datasets — premature canonicalization discards real trade-offs.

---

## 2. Related Work

### 2.1 LiDAR Localization Methods
- Point-to-point ICP and variants (GICP, LiTAMIN2).
- Distribution-to-distribution (NDT).
- Adaptive-threshold odometry (KISS-ICP).
- Continuous-time methods (CT-ICP, CT-LIO).
- Relationship to the **13** families currently benchmarked under `pcd_dogfooding` (see `PLAN.md`).

### 2.2 Benchmarking Methodology
- Existing LiDAR benchmarks (KITTI odometry leaderboard, Hilti Challenge, FusionPortable).
- Limitations of single-number leaderboards.
- Multi-objective and Pareto-aware evaluation in robotics.
- Reproducibility and artifact-based evaluation culture.

---

## 3. Methodology

### 3.1 Stable Core Contract
- The `pcd_dogfooding` CLI as the shared entry point for all methods.
- Contract: every method variant must produce `--summary-json` output with ATE and FPS fields.
- Separation of stable interface (core) from exploratory profiles (experiments/).

### 3.2 Variant Design
- At least 3 concrete variants per method family (see `docs/decisions.md` rules).
- Variant axes: voxel resolution, thread count, ICP iterations, map strategy, window size.
- Promotion/demotion rules: adopt as default, keep as reference, or retire.

### 3.3 Dataset Selection
- **Istanbul**: Autoware-class urban Velodyne windows (3 × ~108 frames); GT CSV.
- **HDL-400**: public Velodyne HDL-32E; 120-frame reference windows (2); LiDAR+IMU available for CT-LIO-style runs (**CT-LIO GT-backed** manifest remains blocked pending curated open GT).
- **MCD**: three Ouster OS1 windows (KTH / NTU / TUHH), 108 frames each; GT CSV.
- **KITTI Raw**: Velodyne HDL-64E; **200-frame** and **full-sequence** slices for drives **0009** and **0061**; GT from OXTS-exported poses (see `evaluation/scripts/kitti_raw_to_benchmark.py`); optional `imu.csv` from `kitti_oxts_imu_for_dogfooding.py` for DLIO/CT-LIO.
- Table 1: dataset characteristics (sensor, frames, environment, GT source).

### 3.4 Metrics
- **ATE (Absolute Trajectory Error)**: meters, computed against GT or reference trajectory.
- **FPS (Frames Per Second)**: wall-clock throughput on shared hardware.
- GT-backed vs. reference-based evaluation separated throughout.
- Contract type recorded per problem instance.

---

## 4. Experimental Setup

### Table 1: Dataset Characteristics

| Dataset | Sensor | Frames | Environment | GT Source |
|---------|--------|--------|-------------|-----------|
| Istanbul (a/b/c) | Velodyne | ~108 | Urban | GT CSV |
| HDL-400 (a/b) | Velodyne HDL-32E | 120 | Indoor / outdoor | GT / reference CSV |
| MCD (KTH / NTU / TUHH) | Ouster OS1 | ~108 | Various | GT CSV |
| KITTI Raw (0009 / 0061) | Velodyne HDL-64E | 200 or full (~443 / ~703) | Urban / highway | OXTS-derived pose CSV |

### Table 2: Method Families and Variant Counts

Each integrated family ships **≥3** CLI profiles in manifests (see `experiments/*_matrix.json`). Core examples:

| Method Family | Example variant ids |
|---------------|---------------------|
| LiTAMIN2 | `fast_icp_only_half_threads`, `paper_icp_only_half_threads`, `fast_cov_half_threads`, … |
| GICP / Small-GICP | `fast_recent_map`, `balanced_local_map`, `dense_recent_map`, … |
| NDT | `fast_coarse_map`, `balanced_local_map`, `dense_local_map`, … |
| KISS-ICP | `fast_recent_map`, `balanced_local_map`, `dense_local_map`, … |
| CT-ICP | `fast_window`, `balanced_window`, `dense_window` |
| DLO / DLIO | `kitti_default`, `fast`, `dense` (+ profile stacks) |
| A-LOAM / F-LOAM / LeGO-LOAM | `kitti_default`, `fast`, `dense` |
| MULLS | `kitti_default`, `fast`, `dense` |
| CT-LIO | e.g. `seed_only_fast` (reference-based window; GT-backed blocked) |

---

## 5. Results

### Table 3: Cross-Dataset Default Variants
- One row per (method family, benchmark window) showing the elected default (`current_default` in aggregates).
- Key observation: among LiTAMIN2, GICP, NDT, KISS-ICP, and CT-ICP, **each** shows **2–3** unique defaults across the **twelve** Istanbul+HDL+KITTI+MCD windows summarized in `docs/variant_analysis.md` §2.
- Data source: `experiments/results/index.json`, `docs/variant_analysis.md`, `docs/assets/paper/manuscript_core_defaults.csv` (overview slice only).

### Figure 1: Pareto Fronts (ATE vs. FPS)
- Scatter plot of **all 73** ready-problem defaults (`docs/assets/paper/ready_defaults.csv`): ATE (m) vs. FPS.
- Separate markers (or faceting) for GT-backed vs. reference-based contracts.
- Annotate extremes from the current CSV (e.g., NDT **~0.005 m** ATE on an Istanbul window; **~106 FPS** peak on high-speed MCD/LiTAMIN2 rows — exact pairings depend on export date).
- Source: `docs/assets/paper/ready_defaults_pareto.png`.

### Figure 2: Default Instability Across Datasets
- Per-method subplots showing how the elected default moves in ATE/FPS space across dataset windows.
- Source: `docs/assets/paper/variant_fronts_by_selector.png`.

### Figure 4 (new): Default mismatch heatmap
- Matrix view: rows = `pcd_dogfooding` method selectors, columns = dataset directory basenames (twelve-window grid where populated). Row-plurality agreement encoded as green vs red (see script header).
- Source: `docs/assets/paper/default_variant_instability.png`, table export `docs/assets/paper/default_variant_matrix.csv`.

### Additional Results
- Per-method accuracy breakdown tables (appendix).
- CT-LIO reference-based results shown separately due to blocked GT status.

---

## 6. Discussion

### Finding 1: No Universal Default (under broad coverage)
- For **all five** widely covered scan-to-map / odometry families in the twelve-window grid (LiTAMIN2, GICP, NDT, KISS-ICP, CT-ICP), the promoted default **varies** with the benchmark window.
- Implication: a single repo-wide default cannot summarize the empirical Pareto front.

### Finding 2: Fast Profiles Are Competitive
- Several "fast" variants achieve accuracy within 2x of the "paper" variant while running 3-5x faster.
- The Pareto front is denser than expected near the speed end.

### Finding 3: Dataset Dependency Is the Norm
- Istanbul (urban, GPS-grade GT) and HDL-400 (shorter, higher-precision GT) produce different rankings.
- This is not a flaw; it is the information that variant-first benchmarking is designed to surface.

### Limitations
- Several newly integrated families (LOAM variants, DLO/DLIO, MULLS, Small-GICP) appear on **fewer** windows than the historical five-way grid; expanding their manifests closes the circle on cross-dataset claims.
- CT-LIO **GT-backed** evaluation remains blocked pending aligned open GT for the HDL-400 LIO window.
- KITTI DLIO experiments currently mirror LiDAR-only behavior unless `imu.csv` is present (`--write-imu-csv` path in README / `PLAN.md`).
- Hardware profile is single-machine; cross-machine scaling not yet characterized.

---

## 7. Conclusion

- Variant-first benchmarking with a stable CLI contract is practical and reveals trade-offs hidden by canonical repos.
- The **73-problem**, **13-family** artifact demonstrates that default instability is **measurable wherever we grant equal window coverage**, not a corner case.
- Artifacts (experiment matrices, generated decision tables, Pareto exports) are fully reproducible via `run_experiment_matrix.py --reuse-existing`.

---

## Appendix

### A. Full Variant Results
- Complete tables for **74** manifest lines in the index (**73** ready + **1** blocked) and per-variant rows inside each `experiments/results/*_matrix.json`.

### B. CT-LIO Reference-Based Evaluation
- Separate treatment of `ct_lio_reference_tradeoff` results.
- Explanation of why GT-backed CT-LIO is blocked and how reference-based comparison differs.

### C. Reproduction Instructions
- Step 1: Install dependencies and build via CMake.
- Step 2: Run `python3 evaluation/scripts/run_experiment_matrix.py --reuse-existing`.
- Step 3: Run `python3 evaluation/scripts/export_paper_assets.py` to regenerate all tables and figures.
- Step 4: Verify against `experiments/results/index.json` counts.
