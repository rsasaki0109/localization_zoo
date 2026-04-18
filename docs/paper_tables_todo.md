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

- [ ] **Table 5: Full Variant Results (Appendix)**
  - Description: All variants for all **258** index problems (**256** ready + **1** blocked + **1** skipped) with ATE, FPS, decision (adopt/keep/retire), and contract type.
  - Data source: Per-method `*_matrix.json` files under `experiments/results/`.
  - Status: Todo -- needs aggregation script across all matrix JSONs.

- [ ] **Table 6: Original Paper Comparison**
  - Description: For each method family, compare reported numbers from the original paper against our reproduced numbers on the same or comparable datasets.
  - Data source: Original papers (manual), `experiments/results/*_profile_matrix.json`.
  - Status: Todo -- original paper numbers not yet collected.

- [ ] **Table 7: CT-LIO Reference-Based Results (Appendix)**
  - Description: Separate table for HDL-400 reference/native-time-style evaluation versus public ROS1 synthetic-time evidence, explaining the blocked GT status and keeping exact-reproduction claims out of the synth-time rows.
  - Data source: `experiments/results/ct_lio_reference_profile_matrix.json`, `experiments/results/ct_lio_public_readiness_matrix.json`, `experiments/results/clins_hdl_400_public_ros1_synthtime_matrix.json`, `experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`.
  - Status: Todo -- data exists but not yet formatted into a manuscript-facing appendix table.

- [ ] **Table 8: Hardware and Environment Specification**
  - Description: CPU, GPU, RAM, OS, compiler, and library versions used for all benchmark runs.
  - Data source: Manual documentation of test machine.
  - Status: Todo.

## Figures

- [x] **Figure 1: Pareto Front (ATE vs. FPS)**
  - Description: Scatter plot of all **256** ready default variants (`ready_defaults.csv`). X-axis: ATE (m); Y-axis: FPS. Separate markers for GT-backed and reference-based. Re-annotate extremes after each export (current CSV spans roughly **0.005–292 m** ATE and **0.17–173** FPS).
  - Data source: `docs/assets/paper/ready_defaults_pareto.png`.
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
