# Paper Tables and Figures Checklist

## Tables

- [x] **Table 1: Dataset Characteristics**
  - Description: Sensor, frame count, environment type, and GT source for each dataset family (Istanbul, HDL-400, KITTI).
  - Data source: `experiments/results/index.json` (dataset paths), manual curation for sensor specs.
  - Status: Done (data available in index.json; needs manual formatting for KITTI expansion).

- [x] **Table 2: Method Families and Variant Counts**
  - Description: One row per method family showing the number of active variants and example variant names.
  - Data source: `experiments/results/index.json` (group by method prefix), `docs/decisions.md`.
  - Status: Done (6 families, variant names extractable from index.json).

- [ ] **Table 3: Cross-Dataset Default Variants**
  - Description: Matrix of (method family x dataset) showing the elected default variant for each cell. Highlights cells where the default differs from the method's most common default.
  - Data source: `experiments/results/index.json` (`current_default` field per problem).
  - Status: Todo -- needs pivot table generation from index.json.

- [x] **Table 4: Ready Defaults Summary (Core)**
  - Description: One representative default per method family with ATE, FPS, contract type, and dataset.
  - Data source: `docs/assets/paper/manuscript_core_defaults.csv`.
  - Status: Done (CSV exported by `export_paper_assets.py`).

- [ ] **Table 5: Full Variant Results (Appendix)**
  - Description: All variants for all 27 problem instances with ATE, FPS, decision (adopt/keep/retire), and contract type.
  - Data source: Per-method `*_matrix.json` files under `experiments/results/`.
  - Status: Todo -- needs aggregation script across all matrix JSONs.

- [ ] **Table 6: Original Paper Comparison**
  - Description: For each method family, compare reported numbers from the original paper against our reproduced numbers on the same or comparable datasets.
  - Data source: Original papers (manual), `experiments/results/*_profile_matrix.json`.
  - Status: Todo -- original paper numbers not yet collected.

- [ ] **Table 7: CT-LIO Reference-Based Results (Appendix)**
  - Description: Separate table for CT-LIO reference-based evaluation, explaining the blocked GT status and showing reference-trajectory metrics.
  - Data source: `experiments/results/ct_lio_reference_profile_matrix.json`, `experiments/results/ct_lio_public_readiness_matrix.json`.
  - Status: Todo -- data exists but not formatted for paper.

- [ ] **Table 8: Hardware and Environment Specification**
  - Description: CPU, GPU, RAM, OS, compiler, and library versions used for all benchmark runs.
  - Data source: Manual documentation of test machine.
  - Status: Todo.

## Figures

- [x] **Figure 1: Pareto Front (ATE vs. FPS)**
  - Description: Scatter plot of all 26 ready default variants. X-axis: ATE (m, log scale). Y-axis: FPS. Separate markers for GT-backed and reference-based. Annotates fastest (LiTAMIN2, 23.5 FPS) and most accurate (NDT, 0.005 m).
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

- [ ] **Figure 4: Default Instability Heatmap**
  - Description: Heatmap or matrix plot with method families on rows, datasets on columns, colored by which variant is the default. Visually highlights that defaults are not stable.
  - Data source: `experiments/results/index.json`.
  - Status: Todo -- needs new visualization script.

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
| Tables | 2 | 6 | 8 |
| Figures | 3 | 4 | 7 |
| **Total** | **5** | **10** | **15** |
