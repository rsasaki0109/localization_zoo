# Paper Tables and Figures Checklist

## Tables

- [x] **Table 1: Dataset Characteristics**
  - Description: Sensor, frame count, environment type, and GT source for each dataset family (Istanbul, HDL-400, KITTI).
  - Data source: `experiments/results/index.json` (dataset paths), manual curation for sensor specs.
  - Status: Done (data available in index.json; needs manual formatting for KITTI expansion).

- [x] **Table 2: Method Families and Variant Counts**
  - Description: One row per method family showing the number of active variants and example variant names.
  - Data source: `experiments/results/index.json` (group by method prefix), `docs/decisions.md`.
  - Status: Done for prose table in `paper_draft_outline.md` (current artifact is broader than the manuscript core; see `docs/interfaces.md` for the active selector set and `experiments/` for per-manifest variant ids).

- [x] **Table 3: Cross-Dataset Default Variants**
  - Description: Matrix of (method family × dataset slug) with elected `current_default` per cell. Wide CSV for TeX import; long CSV for spreadsheets.
  - Data source: `experiments/results/index.json` + per-problem aggregates (`stable_interface.methods`, `dataset.pcd_dir` basename).
  - Status: Done — `evaluation/scripts/generate_default_variant_matrix.py` → `docs/assets/paper/default_variant_matrix.csv` (+ long form). Invoked from `export_paper_assets.py`.

- [x] **Table 4: Ready Defaults Summary (Core)**
  - Description: One representative default per method family with ATE, FPS, contract type, and dataset.
  - Data source: `docs/assets/paper/manuscript_core_defaults.csv`.
  - Status: Done (CSV exported by `export_paper_assets.py`).

- [x] **Table 5: Full Variant Results (Appendix)**
  - Description: All **1,313** variants across the **422** index problems (**407** ready + **14** skipped + **1** blocked with no variants) with ATE, RPE where recorded, FPS, run status, decision, and contract type.
  - Data source: Per-method `*_matrix.json` files under `experiments/results/`.
  - Status: Done — `evaluation/scripts/generate_full_variant_table.py` → `docs/assets/paper/full_variant_results.csv` (+ `full_variant_results.tex` longtable). Invoked from `export_paper_assets.py`.

- [ ] **Table 6: Original Paper Comparison**
  - Description: Per KITTI Odometry sequence, the paper-reported translational RTE next to the best non-GT-seeded repository variant on the same full sequence, its ratio, the pool median, and pool size.
  - Data source: `evaluation/data/paper_reported_numbers.json` (each value now carries `reported_source` with arXiv id, table, and row) and `experiments/results/*_matrix.json` on `kitti_seq_<NN>_full`.
  - Status: Partial — `evaluation/scripts/generate_paper_ratio_table.py` → `docs/assets/paper/paper_ratio_table.{csv,tex}`, invoked from `export_paper_assets.py`. Repo values use the official KITTI RTE (100-800 m) recorded by `rescore_kitti_rte.py` (`experiments/results/kitti_rte_rescore.json`) for the best 100 m-RPE variant of each sweep. Covers LiTAMIN2 (00/02/05/07/08, ~1.22x), CT-ICP (00/02/05/07/08, ~4.15x), KISS-ICP (00 only, 1.87x with the current default search), A-LOAM vs LOAM (00/02/05/07/08, ~1.01x, secondary-source paper values), MULLS (00/02/05/07/08, 7.62x), and SuMa (00/02/05/07/08, 2.21x; 00/02/05/08 use the seq 07 choice unchanged, so those rows are held-out). F-LOAM reports KITTI only as a bar chart and LeGO-LOAM has no KITTI table, so neither has verifiable per-sequence values. The other reimplementations have no verified paper numbers yet; add them only from the paper PDF with a `reported_source`.

- [x] **Table 7: CT-LIO Reference-Based Results (Appendix)**
  - Description: Three separated sections — (A) HDL-400 reference window with native per-point time, (B) public ROS1 HDL-400 window with synthesized per-point time, (C) the blocked GT-backed CT-LIO readiness problem. A and B are both scored against `hdl_400_public_reference.csv` (a reference trajectory, not GT), so neither is an exact-reproduction claim; CLINS rows are flagged as GT-seeded initialization.
  - Data source: `ct_lio_reference_profile_matrix.json`, `ct_icp_hdl_400_reference_matrix.json`, `ct_lio_hdl_400_public_ros1_synthtime_matrix.json`, `ct_icp_hdl_400_public_ros1_synthtime_matrix.json`, `clins_hdl_400_public_ros1_synthtime_matrix.json`, `ct_lio_public_readiness_matrix.json` under `experiments/results/`.
  - Status: Done — `evaluation/scripts/generate_ct_appendix_table.py` → `docs/assets/paper/ct_appendix.{csv,tex}`. Invoked from `export_paper_assets.py`.

- [ ] **Table 8: Hardware and Environment Specification**
  - Description: CPU, GPU, RAM, OS, compiler, and library versions used for benchmark runs.
  - Data source: `evaluation/scripts/capture_benchmark_environment.py` → `docs/assets/paper/benchmark_environment.{json,md}` (capture host plus host records in `evaluation/data/*.json`). Run it on the benchmark machine; it is deliberately not part of `export_paper_assets.py` because the output depends on the host.
  - Status: Partial — the capture host is documented. `run_experiment_matrix.py` now writes `host.json` beside each executed variant and a `host` field (CPU, cores, memory, OS, kernel; no hostname) into the aggregate, surfaced as `host_cpu` in Table 5 and as per-run coverage in Table 8. All existing 1,276 variants predate this (at least one evidence row ran on an i7-9750H WSL host), so FPS columns become attributable only as matrices are rerun.

## Figures

- [x] **Figure 1: Pareto Front (ATE vs. FPS)**
  - Description: `ready_defaults.csv` holds **403** ready default variants (407 ready problems minus 4 IMU-only `imu_dead_reckoning` rows, which read no point clouds); spans **0.005–292 m** ATE and **0.17–1717** FPS. The exported PNG plots the best-ATE default per method (32 methods). Contract type is now **360 GT-backed / 34 reference-based** (only the HDL-400 reference windows); before 2026-10-03 nearly every row was mislabelled reference-based because the rule matched the `experiments/reference_data/` directory.
  - Figure: `docs/assets/paper/kitti07_pareto.png` — every non-GT-seeded, non-diverged variant on full KITTI Odometry 07 (106 variants, 8 methods), 100 m RPE vs FPS on log axes with the Pareto front. Earlier versions plotted the best-ATE default per method across all windows, which let GT-seeded NDT (0.005 m seed adherence) look best; replaced on 2026-10-03.
  - Status: Done (exported by `export_paper_assets.py`).

- [x] **Figure 2: Variant Fronts by Method Family**
  - Description: Per-method subplots showing how current defaults, challengers, and reference variants distribute in the ATE/FPS plane. Shows default movement across datasets.
  - Data source: `docs/assets/paper/variant_fronts_by_selector.png`.
  - Status: Done (exported by `export_paper_assets.py`).

- [x] **Figure 3: Core Defaults Visualization**
  - Description: Manuscript-facing visualization of one representative default per method family, distinguishing reference-based from GT-backed.
  - Data source: `docs/assets/paper/manuscript_core_methods.png`.
  - Status: Done (exported by `export_paper_assets.py`).

- [x] **Figure 4: Default Instability Heatmap**
  - Description: Methods × datasets; **green** = matches row plurality default, **red** = differs, **gray** = no benchmark cell; variant id annotated in small type.
  - Data source: same as Table 3.
  - Status: Done — `docs/assets/paper/default_variant_instability.png` (generator script as above).

- [ ] **Figure 5: Per-Method Pareto Overlay**
  - Description: One combined plot showing the Pareto front for each method family as a separate curve/color, making it easy to see where families overlap and where they dominate.
  - Data source: Per-method `*_matrix.json` under `experiments/results/`.
  - Status: Todo -- needs new visualization script.

- [ ] **Figure 6: Framework Architecture Diagram**
  - Description: Block diagram showing the stable core contract (pcd_dogfooding CLI), variant layer, experiment matrix, and paper asset pipeline.
  - Data source: Manual (draw.io or TikZ).
  - Status: Todo.

- [ ] **Figure 7: Dataset Sample Point Clouds**
  - Description: Representative point cloud frames from Istanbul, HDL-400, and KITTI to show environmental diversity.
  - Data source: Raw dataset PCD files.
  - Status: Todo.

## Summary

| Category | Done | Todo | Total |
|----------|------|------|-------|
| Tables | 3 | 5 | 8 |
| Figures | 4 | 3 | 7 |
| **Total** | **7** | **8** | **15** |
