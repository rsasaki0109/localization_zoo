# Paper Draft Outline

**Working title**: Variant-First Localization Benchmarking Reveals Pareto Fronts Hidden by Single-Canonical Repositories

**Thesis**: Variant-first localization benchmarking reveals Pareto fronts hidden when repositories force one canonical implementation too early.

---

## 1. Introduction

**Problem.** Most open-source LiDAR localization repositories ship a single canonical configuration per method. Users must accept the upstream default or hand-tune parameters without visibility into the accuracy/throughput trade-off space. This hides useful Pareto fronts and makes cross-method comparison misleading.

**Contribution.**
- A _variant-first_ benchmarking framework that keeps 3+ concrete variants alive per method family under a shared CLI contract (`pcd_dogfooding --summary-json`).
- An empirical study across 6 method families (LiTAMIN2, GICP, NDT, KISS-ICP, CT-ICP, CT-LIO) and 3 public dataset families (Istanbul, HDL-400, KITTI), covering 26 ready problem instances.
- Evidence that the "best" default variant shifts across datasets, proving that premature canonicalization loses information.

---

## 2. Related Work

### 2.1 LiDAR Localization Methods
- Point-to-point ICP and variants (GICP, LiTAMIN2).
- Distribution-to-distribution (NDT).
- Adaptive-threshold odometry (KISS-ICP).
- Continuous-time methods (CT-ICP, CT-LIO).
- Relationship to the 6 families implemented in this repository.

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
- **Istanbul**: repository-stored urban sequences (3 windows: a, b, c). ~108 frames each.
- **HDL-400**: public Velodyne dataset, 120-frame reference windows (2 windows: a, b). Includes LiDAR+IMU for CT-LIO.
- **KITTI**: standard odometry sequences (planned expansion for camera-era generality).
- Table 1 placeholder: dataset characteristics (sensor, frames, environment, GT source).

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
| HDL-400 (a/b) | Velodyne | 120 | Indoor/outdoor | GT CSV |
| KITTI | Velodyne HDL-64E | varies | Urban/highway | GPS/INS |

### Table 2: Method Families and Variant Counts

| Method Family | Variants | Example Variant Names |
|---------------|----------|----------------------|
| LiTAMIN2 | 3+ | `fast_icp_only_half_threads`, `paper_icp_only_half_threads`, ... |
| GICP | 3+ | `fast_recent_map`, `balanced_recent_map`, `dense_recent_map`, ... |
| NDT | 3+ | `fast_coarse_map`, `balanced_coarse_map`, `dense_coarse_map`, ... |
| KISS-ICP | 3+ | `fast_recent_map`, `dense_local_map`, ... |
| CT-ICP | 3+ | `fast_window`, `balanced_window`, `dense_window` |
| CT-LIO | 3+ | `seed_only_fast`, ... (reference-based only) |

---

## 5. Results

### Table 3: Cross-Dataset Default Variants
- One row per (method, dataset) pair showing the elected default.
- Key observation: defaults are _not_ stable across datasets.
  - LiTAMIN2: `fast_icp_only_half_threads` on Istanbul, `paper_icp_only_half_threads` on HDL-400 (window a).
  - CT-ICP: `fast_window` on HDL-400, `balanced_window` on Istanbul (b, c).
  - KISS-ICP: `fast_recent_map` mostly, but `dense_local_map` on Istanbul-b.
- Data source: `experiments/results/index.json`, `docs/assets/paper/manuscript_core_defaults.csv`.

### Figure 1: Pareto Fronts (ATE vs. FPS)
- Scatter plot of all 26 ready defaults on ATE (x) vs. FPS (y).
- Separate markers for GT-backed and reference-based.
- Fastest default: LiTAMIN2 `fast_icp_only_half_threads` at 23.5 FPS.
- Lowest ATE: NDT `fast_coarse_map` at 0.005 m.
- Source: `docs/assets/paper/ready_defaults_pareto.png`.

### Figure 2: Default Instability Across Datasets
- Per-method subplots showing how the elected default moves in ATE/FPS space across dataset windows.
- Source: `docs/assets/paper/variant_fronts_by_selector.png`.

### Additional Results
- Per-method accuracy breakdown tables (appendix).
- CT-LIO reference-based results shown separately due to blocked GT status.

---

## 6. Discussion

### Finding 1: No Universal Default
- For 4 out of 6 method families, the elected default changes when switching datasets.
- Implication: repos that freeze a single default lose information about the trade-off.

### Finding 2: Fast Profiles Are Competitive
- Several "fast" variants achieve accuracy within 2x of the "paper" variant while running 3-5x faster.
- The Pareto front is denser than expected near the speed end.

### Finding 3: Dataset Dependency Is the Norm
- Istanbul (urban, GPS-grade GT) and HDL-400 (shorter, higher-precision GT) produce different rankings.
- This is not a flaw; it is the information that variant-first benchmarking is designed to surface.

### Limitations
- Current dataset coverage is 2 families (3 planned); KITTI expansion pending.
- CT-LIO GT-backed evaluation blocked pending aligned GT CSV for HDL-400 LiDAR+IMU.
- Hardware profile is single-machine; cross-platform scaling not yet tested.

---

## 7. Conclusion

- Variant-first benchmarking with a stable CLI contract is practical and reveals trade-offs hidden by canonical repos.
- The 26-problem, 6-method study demonstrates that default instability is the norm, not the exception.
- Artifacts (experiment matrices, generated decision tables, Pareto exports) are fully reproducible via `run_experiment_matrix.py --reuse-existing`.

---

## Appendix

### A. Full Variant Results
- Complete tables for all 27 problem instances (26 ready + 1 blocked).
- Source: per-method `*_matrix.json` files under `experiments/results/`.

### B. CT-LIO Reference-Based Evaluation
- Separate treatment of `ct_lio_reference_tradeoff` results.
- Explanation of why GT-backed CT-LIO is blocked and how reference-based comparison differs.

### C. Reproduction Instructions
- Step 1: Install dependencies and build via CMake.
- Step 2: Run `python3 evaluation/scripts/run_experiment_matrix.py --reuse-existing`.
- Step 3: Run `python3 evaluation/scripts/export_paper_assets.py` to regenerate all tables and figures.
- Step 4: Verify against `experiments/results/index.json` counts.
