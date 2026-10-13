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

The ATE vs. FPS scatter over **all elected defaults** in `docs/assets/paper/ready_defaults.csv` (one row per **ready** LiDAR/visual problem instance; IMU-only dead-reckoning rows are excluded) spans roughly **0.005 m to 292 m** ATE and **0.17 to 1717** FPS on the machine used for the stored aggregates. No single method family dominates the full front; fast scan-to-map variants (e.g., LiTAMIN2 `fast_*`, Small-GICP on KITTI) coexist with high-throughput multimodal OKVIS configurations and high-accuracy NDT configurations on other windows.

**Evidence:**
- `docs/assets/paper/kitti07_pareto.png` — KITTI 07 pure-odometry Pareto front (152 variants, 10 methods)
- `docs/assets/paper/manuscript_core_defaults.csv` — one **manuscript-facing** representative default per **core** method family (subset used for overview figures; full cloud is `ready_defaults.csv`).

### Sub-Claim 3: A stable CLI contract makes variant-first benchmarking practical

The stable `--summary-json` contract allows adding new variants and new benchmark windows without branching the evaluation runner. The current index tracks **434** ready problems, **1** blocked manifest, and **14** skipped manifests across **35** active selectors, all driven through `run_experiment_matrix.py` / `refresh_study_docs.py` with `pcd_dogfooding` and `multimodal_dogfooding` as sibling stable binaries.

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

### Sub-Claim 5: Paper-number fidelity is measurable, and it splits the reimplementations into two groups

For eight LiDAR odometry reimplementations, the paper-reported KITTI Odometry
translational error (read from the paper PDF, with table and row recorded) is
compared against the repository on the **same full sequences and the same
official KITTI RTE metric** (100-800 m segments, every 10th frame):

| Group | Method | Sequences | Repo / paper (geometric mean) |
|---|---|---|---:|
| Near paper | LiTAMIN2 (paper odometry pipeline, `--litamin2-paper-odometry`; ICP+Cov and ICP rows, no loop closure; one fixed setting gives 1.05x on 00-10, see `papers/litamin2/README.md`) | 00-10 | 1.00x |
| Near paper | A-LOAM (vs LOAM, secondary-source values) | 00/02/05/07/08 | 1.01x |
| Near paper | LF-GICP (no author code; gate calibrated on the paper's KITTI 00 trace) | 00/02/05/07/08 | 0.96x |
| Near paper | KISS-ICP (upstream threshold, kernel, and map gate on corrected scans as upstream; 00-10 average equals the official code's; see `papers/kiss_icp/README.md`) | 00/04 | 1.04x |
| Near paper | MULLS (MULLS-LO reimplementation, `--mulls-lo`, highway profile on 01, 0.195 deg correction as the official profile; 00-10 average equals the official code's; see `papers/mulls/README.md`) | 00-10 | 1.12x |
| Near paper | CT-ICP (upstream pipeline, `--ct-icp-upstream-mechanics`, on corrected scans as upstream; 00-10 average equals the official code's; see `papers/ct_icp/README.md`) | 00-10 | 1.15x |
| Gap remains | L-LO (no author code; parameters chosen on seq 07; horizontal 0.577 % in 2D on 07; the paper's 00/07 values are below every roll-free trajectory built from the ground truth, see `papers/l_lo/README.md`) | 00/02/05/07/08 | 1.58x |
| Near paper | SuMa (Frame-to-Model; paper pipeline `--suma-paper`, best of two variants per sequence; 01 at 2.09x; one setting for all: 1.25x; see `papers/suma/README.md`) | 00-10 | 0.97x |
| Near paper | Quadric-LO (vs the paper's scan-to-submap Table II; no author code) | 00/07 | 0.80x |
| Near paper | Mesh-LOAM (no author code; paper path, `--mesh-loam-paper-odometry`: nearest-facet association, 0.1 m input, 0.22 deg correction; 1.80x before, see `papers/mesh_loam/README.md`) | 00/07 | 1.08x |
| Gap remains | ELO (no author code; 0.195 deg correction, the paper normal patch and forward-bias fixes chosen on 07 windows; 2.60x before, see `papers/elo/README.md`) | 00/07 | 1.17x |
| Near paper | IMLS-SLAM (paper pipeline, `--imls-slam-paper-odometry`, 0.22 deg correction; 2.66x before, see `papers/imls_slam/README.md`) | 00/07 | 1.04x |
| Near paper | DiLO (plane-only paper pipeline `--dilo-paper-odometry` with spread planar key points, slope-aware vertical fill and the 0.18 deg correction, chosen on 07 windows with some 00 windows in view; edge key points do not help on KITTI; 4.12x before, see `papers/dilo/README.md`) | 00/07 | 0.87x |
| Gap remains | InTEn-LOAM (no author code; paper pipeline, `--inten-loam-paper-odometry`, 0.2 deg correction; plane map voxel and edge threshold chosen on 07; an S2M plane check with a 0.17 deg correction brings 00 from 1.48x to 1.34x (07 1.07x, one setting 1.20x); the earlier pipeline failed at 42-48 %, see `papers/inten_loam/README.md`) | 00/07 | 1.18x |

What may be claimed: A-LOAM, LF-GICP, KISS-ICP, MULLS, CT-ICP, LiTAMIN2, Quadric-LO, IMLS-SLAM and Mesh-LOAM (seq 00/07 only for the last three) reach paper-level odometry accuracy
on KITTI (KISS-ICP on seq 00 and 04, and only with the upstream mechanics and the
KITTI elevation correction; its 00-10 average, 0.564 %, equals the official
code's on the same pipeline, both 1.13x the paper's 0.50 %; CT-ICP only with
`--ct-icp-upstream-mechanics` and the correction, whose 00-10 average, 0.587 %,
equals the official code's 0.585 % on the same scans, 1.10x the paper's per-sequence
values; MULLS only with `--mulls-lo`, whose 00-10 average, 0.572 %, equals the
official code's 0.576 %, 1.06x the paper's per-sequence values); L-LO, ELO, and InTEn-LOAM run on the same metric but do **not** reproduce the
paper numbers. What may not be claimed: faithful reproduction for
L-LO, ELO, or InTEn-LOAM, a KISS-ICP, CT-ICP, or MULLS claim for the repository default, or any ratio as unbiased. The repository value is the
best variant of a sweep **selected on the evaluated sequence**, so every ratio
is an optimistic bound, except the L-LO and SuMa rows on 00/02/05/08, whose variants were chosen on seq 07 and transferred unchanged; SuMa's paper values have one decimal, and A-LOAM is
compared with LOAM values cited by later papers because LOAM has no
per-sequence table.

**Evidence:**
- `docs/assets/paper/paper_ratio_table.csv` / `.tex` — Table 6, per sequence, with sweep size.
- `evaluation/data/paper_reported_numbers.json` — paper values with `reported_source` (arXiv id, table, row), pinned by `tests/test_paper_ratio_table.py`.
- `experiments/results/kitti_rte_rescore.json` — official-RTE re-runs and the stored-vs-rerun determinism check.

Papers checked for a per-sequence KITTI Odometry table (2026-10-12), beyond the rows above:

- **Compared above** (official translational error in %): ELO (Table II), IMLS-SLAM (Table I), InTEn-LOAM (Table 1), Mesh-LOAM (Table I), Quadric-LO (Table II with scan-to-submap; Table I scan-to-scan only), DiLO (Table 1).
- **Not compared**: VLOM (Table II, 00/02/05/08; visual-LiDAR with the camera, and its A-LOAM baseline, 1.66 % on 00, disagrees with the usual 0.78 %), PL-LOAM (Table I, camera + LiDAR with loop closure), SROM / Spectral-LO (Table I, "% error per 100m" on 01-05 only, averaging not stated).
- **KITTI 00-10 only as ATE in metres**: M-GCLO, R-VoxelMap, Small-Mighty, Adaptive-ICP.
- **No KITTI Odometry table**: CT-VoxelMap, DAMM-LOAM, Degen-Sense, GMM-LO (HGMR), GNC-LO, KC-LO, LiDAR-IBA, LODESTAR, MCC-LO, Student-t-LO, SVN-ICP, TC-LVGF, RF-LIO, ID-LIO, Terrain-RBF-LIO, Vibration-LIO, D2LIO, BIEVR-LIO, VGICP, small_gicp, GICP, NDT, TrICP-LO (the last few predate KITTI).
- **Not checked (no accessible PDF)**: DALI-SLAM, I-LOAM, ICPSC, Intensity-Flow, MCGICP, PCR-DAT, UA-LIO, CUBE-LIO.

### Sub-Claim 6: Paper-number audits catch silent errors

The same audit found two classes of error that a benchmark repository can carry
unnoticed, which motivates pinning both inputs and code:

- **Wrong reference values.** Before 2026-10-03, every stored paper value for
  LiTAMIN2, CT-ICP, and KISS-ICP disagreed with the paper (for example KISS-ICP
  was recorded as per-sequence ATE in metres, which the paper does not report).
  This had inflated LiTAMIN2's apparent gap (1.35x claimed vs 1.22x measured)
  and overstated CT-ICP's (~4.5x claimed vs 4.03x measured on the official metric).
- **Silent code drift.** A later change to the shared KISS-ICP voxel search
  (27 voxels → all voxels within the correspondence distance) moved KITTI 00
  from 0.857 % to 1.069 % 100 m RPE without any aggregate being re-run.
  `--kiss-legacy-27-neighborhood` reproduces the stored result bit for bit.
  CT-ICP and A-LOAM re-runs drift by up to 0.11 and 0.017 points; LiTAMIN2,
  MULLS, and SuMa reproduce exactly.

**Evidence:**
- `docs/dogfooding_methodology.md` — errata on the earlier paper columns.
- `papers/kiss_icp/README.md` — correspondence-search change and opt-in flag.
- `experiments/results/kitti_rte_rescore.json` — `rerun_rpe_abs_delta` per row.

## Evidence Summary Table

| Evidence File | What It Shows |
|---------------|---------------|
| `experiments/results/index.json` | **434** ready + **1** blocked + **14** skipped problems; per-problem defaults |
| `docs/variant_analysis.md` | GT-seed ablation, cross-dataset default stability, profile impact |
| `docs/decisions.md` | Variant lifecycle and adoption rules |
| `docs/assets/paper/ready_defaults.csv` | All ready-problem defaults — ATE, FPS, dataset tag |
| `docs/assets/paper/kitti07_pareto.png` | KITTI 07 RPE vs. FPS Pareto front (pure odometry) |
| `docs/assets/paper/variant_fronts_by_selector.png` | Default movement across datasets |
| `docs/assets/paper/default_variant_matrix.csv` | Wide Table 3 — method × dataset slug → default variant |
| `docs/assets/paper/default_variant_instability.png` | Green/red heatmap vs row plurality (Figure 4) |
| `docs/assets/paper/manuscript_core_defaults.csv` | One representative default per **overview** method family |
| `evaluation/scripts/SETUP_MULTIMODAL_BENCHMARK.md` | Camera-aware benchmark contract and KITTI multimodal workflow |
| `docs/paper_tracks.md` / `docs/paper_comparison.md` | Full tables derived from aggregates |
| `docs/assets/paper/paper_ratio_table.csv` | Table 6 — paper vs repo on the official KITTI RTE |
| `experiments/results/kitti_rte_rescore.json` | Official-RTE re-runs with determinism check |
