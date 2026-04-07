# Paper Tracks

_Generated at 2026-04-07T13:50:19+00:00 by `evaluation/scripts/generate_publication_docs.py`._

This repository should not be pitched as "many implementations exist here".
The paper target has to be a claim about what this experiment-driven process reveals.

Current coverage: `80` ready problems and `1` blocked problems.

## Current State

| Problem | Status | Default | Best ATE [m] | Best FPS | Dataset |
|---------|--------|---------|--------------|----------|---------|
| A-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 6.105 | 5.8 | `dogfooding_results/kitti_raw_0009_full` |
| CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `balanced_window` | 1.659 | 54.8 | `dogfooding_results/kitti_raw_0009_200` |
| CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_window` | 1.475 | 56.9 | `dogfooding_results/kitti_raw_0061_200` |
| CT-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_window` | 6.115 | 59.8 | `dogfooding_results/mcd_kth_day_06_108` |
| CT-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_window` | 0.325 | 71.7 | `dogfooding_results/mcd_ntu_day_02_108` |
| CT-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_window` | 1.652 | 71.6 | `dogfooding_results/mcd_tuhh_night_09_108` |
| CT-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `fast_window` | 1.357 | 2.4 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| CT-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `fast_window` | 75.075 | 2.7 | `dogfooding_results/autoware_istanbul_open_108` |
| CT-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `fast_window` | 0.556 | 2.4 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| CT-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `balanced_window` | 6.820 | 3.1 | `dogfooding_results/autoware_istanbul_open_108_b` |
| CT-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `balanced_window` | 7.539 | 2.8 | `dogfooding_results/autoware_istanbul_open_108_c` |
| CT-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `balanced_window` | 1.659 | 71.4 | `dogfooding_results/kitti_raw_0009_200` |
| CT-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `balanced_window` | 4.673 | 44.9 | `dogfooding_results/kitti_raw_0009_full` |
| CT-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_window` | 6.972 | 37.6 | `dogfooding_results/kitti_raw_0061_full` |
| CT-LIO GT-backed public benchmark readiness on HDL-400 ROS2 data | `blocked` | `-` | - | - | `dogfooding_results/hdl_400_open_ct_lio_60` |
| CT-LIO reference-trajectory trade-off on the public HDL-400 120-frame window | `ready` | `seed_only_fast` | 0.412 | 0.5 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| DLIO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.026 | 7.3 | `dogfooding_results/kitti_raw_0009_full` |
| DLO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.026 | 7.3 | `dogfooding_results/kitti_raw_0009_full` |
| F-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 5.452 | 28.6 | `dogfooding_results/kitti_raw_0009_full` |
| GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_recent_map` | 1.177 | 25.8 | `dogfooding_results/kitti_raw_0009_200` |
| GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_recent_map` | 0.781 | 25.7 | `dogfooding_results/kitti_raw_0061_200` |
| GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 0.630 | 24.7 | `dogfooding_results/mcd_kth_day_06_108` |
| GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.017 | 28.7 | `dogfooding_results/mcd_ntu_day_02_108` |
| GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 0.317 | 31.2 | `dogfooding_results/mcd_tuhh_night_09_108` |
| GICP throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 0.091 | 1.7 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| GICP throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 0.994 | 6.3 | `dogfooding_results/autoware_istanbul_open_108` |
| GICP throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_recent_map` | 0.135 | 1.7 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| GICP throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 1.166 | 5.7 | `dogfooding_results/autoware_istanbul_open_108_b` |
| GICP throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 0.982 | 4.3 | `dogfooding_results/autoware_istanbul_open_108_c` |
| GICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `dense_recent_map` | 1.510 | 32.0 | `dogfooding_results/kitti_raw_0009_200` |
| GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 1.170 | 23.0 | `dogfooding_results/kitti_raw_0009_full` |
| GICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_recent_map` | 1.055 | 22.8 | `dogfooding_results/kitti_raw_0061_full` |
| KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_recent_map` | 2.412 | 24.4 | `dogfooding_results/kitti_raw_0009_200` |
| KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_recent_map` | 0.679 | 28.3 | `dogfooding_results/kitti_raw_0061_200` |
| KISS-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 5.568 | 11.3 | `dogfooding_results/mcd_kth_day_06_108` |
| KISS-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `fast_recent_map` | 0.017 | 66.7 | `dogfooding_results/mcd_ntu_day_02_108` |
| KISS-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 1.104 | 24.1 | `dogfooding_results/mcd_tuhh_night_09_108` |
| KISS-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `fast_recent_map` | 1.646 | 0.5 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| KISS-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 182.960 | 4.0 | `dogfooding_results/autoware_istanbul_open_108` |
| KISS-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `fast_recent_map` | 0.218 | 0.4 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| KISS-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `dense_local_map` | 143.921 | 3.6 | `dogfooding_results/autoware_istanbul_open_108_b` |
| KISS-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_recent_map` | 131.691 | 3.7 | `dogfooding_results/autoware_istanbul_open_108_c` |
| KISS-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_recent_map` | 2.412 | 28.2 | `dogfooding_results/kitti_raw_0009_200` |
| KISS-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 5.383 | 21.9 | `dogfooding_results/kitti_raw_0009_full` |
| KISS-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_recent_map` | 4.343 | 11.2 | `dogfooding_results/kitti_raw_0061_full` |
| LeGO-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 6.066 | 9.5 | `dogfooding_results/kitti_raw_0009_full` |
| LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `paper_icp_only_half_threads` | 1.053 | 43.7 | `dogfooding_results/kitti_raw_0009_200` |
| LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_cov_half_threads` | 0.511 | 68.6 | `dogfooding_results/kitti_raw_0061_200` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_cov_half_threads` | 0.401 | 64.2 | `dogfooding_results/mcd_kth_day_06_108` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `paper_icp_only_half_threads` | 0.045 | 105.6 | `dogfooding_results/mcd_ntu_day_02_108` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_cov_half_threads` | 0.194 | 106.5 | `dogfooding_results/mcd_tuhh_night_09_108` |
| LiTAMIN2 throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `paper_icp_only_half_threads` | 0.121 | 6.4 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| LiTAMIN2 throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_icp_only_half_threads` | 1.213 | 23.5 | `dogfooding_results/autoware_istanbul_open_108` |
| LiTAMIN2 throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_icp_only_half_threads` | 0.168 | 6.1 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| LiTAMIN2 throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_icp_only_half_threads` | 1.222 | 20.9 | `dogfooding_results/autoware_istanbul_open_108_b` |
| LiTAMIN2 throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `paper_icp_only_half_threads` | 0.741 | 21.2 | `dogfooding_results/autoware_istanbul_open_108_c` |
| LiTAMIN2 trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `paper_cov_half_threads` | 122.275 | 87.3 | `dogfooding_results/kitti_raw_0009_200` |
| LiTAMIN2 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_icp_only_half_threads` | 1.145 | 48.8 | `dogfooding_results/kitti_raw_0009_full` |
| LiTAMIN2 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_icp_only_half_threads` | 0.944 | 58.1 | `dogfooding_results/kitti_raw_0061_full` |
| MULLS trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast` | 4.610 | 3.3 | `dogfooding_results/kitti_raw_0009_full` |
| NDT throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `fast_coarse_map` | 0.279 | 36.1 | `dogfooding_results/kitti_raw_0009_200` |
| NDT throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `fast_coarse_map` | 0.319 | 41.2 | `dogfooding_results/kitti_raw_0061_200` |
| NDT throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_coarse_map` | 0.136 | 31.2 | `dogfooding_results/mcd_kth_day_06_108` |
| NDT throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `balanced_local_map` | 0.013 | 44.9 | `dogfooding_results/mcd_ntu_day_02_108` |
| NDT throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_coarse_map` | 0.063 | 40.8 | `dogfooding_results/mcd_tuhh_night_09_108` |
| NDT throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `fast_coarse_map` | 0.035 | 0.8 | `dogfooding_results/hdl_400_open_ct_lio_120` |
| NDT throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.070 | 2.0 | `dogfooding_results/autoware_istanbul_open_108` |
| NDT throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `fast_coarse_map` | 0.065 | 0.9 | `dogfooding_results/hdl_400_open_ct_lio_120_b` |
| NDT throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.007 | 2.1 | `dogfooding_results/autoware_istanbul_open_108_b` |
| NDT throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `fast_coarse_map` | 0.005 | 1.9 | `dogfooding_results/autoware_istanbul_open_108_c` |
| NDT trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `fast_coarse_map` | 121.733 | 25.0 | `dogfooding_results/kitti_raw_0009_200` |
| NDT trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_coarse_map` | 0.255 | 30.9 | `dogfooding_results/kitti_raw_0009_full` |
| NDT trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `fast_coarse_map` | 0.247 | 23.8 | `dogfooding_results/kitti_raw_0061_full` |
| Small-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `fast_recent_map` | 0.806 | 107.9 | `dogfooding_results/mcd_kth_day_06_108` |
| Small-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.031 | 113.8 | `dogfooding_results/mcd_ntu_day_02_108` |
| Small-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `fast_recent_map` | 0.250 | 107.2 | `dogfooding_results/mcd_tuhh_night_09_108` |
| Small-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `fast_recent_map` | 0.437 | 92.4 | `dogfooding_results/kitti_raw_0009_full` |
| Voxel-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `dense_recent_map` | 0.926 | 124.2 | `dogfooding_results/mcd_kth_day_06_108` |
| Voxel-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `dense_recent_map` | 0.121 | 117.2 | `dogfooding_results/mcd_ntu_day_02_108` |
| Voxel-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `dense_recent_map` | 0.286 | 116.4 | `dogfooding_results/mcd_tuhh_night_09_108` |
| Voxel-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `dense_recent_map` | 0.640 | 110.1 | `dogfooding_results/kitti_raw_0009_full` |

## Recommendation Order

1. Empirical study as the primary paper target.
2. Artifact / reproducibility submission in parallel.
3. Focused method paper only after a stronger multi-dataset delta exists.

## Track A: Empirical Study

- **Readiness**: Medium (65/100)
- **Primary claim**: Variant-first localization benchmarking reveals Pareto fronts that are hidden when repositories force one canonical implementation too early.
- **Decision**: Primary target. This is the most defensible full-paper narrative for the current repository.

### Strongest Evidence

- The stable benchmark contract now covers 80 ready problems under one CLI and one summary JSON interface.
- Each active problem keeps at least three concrete variants alive instead of collapsing immediately to a single abstraction.
- Current defaults already show non-trivial trade-offs, such as `LiTAMIN2=paper_icp_only_half_threads` at 43.7 FPS and `CT-LIO=seed_only_fast` at 0.412 m ATE on the public HDL-400 reference window.

### Gaps

- LiDAR-only methods now cover 12 repository-stored open sequences across 6 public dataset families, but external validity still rests on only 6 families.
- Reference-based and GT-backed results are now separated conceptually, but the GT-backed CT-LIO public benchmark is still blocked.
- There is no paper-ready comparison against originally reported results yet.
- Hardware-normalized reruns and confidence intervals are not exported yet.

### Next Experiments

- Add a third public dataset family, or expand the current two-family evidence with longer and less curated windows.
- Unblock or explicitly exclude GT-backed CT-LIO from the main study before writing.
- Generate a method-by-method table comparing repository defaults, challengers, and original-paper numbers.
- Export paper-ready Pareto figures from `experiments/results/*.json`.

## Track B: Artifact / Reproducibility

- **Readiness**: High (87/100)
- **Primary claim**: Localization research tooling is more reproducible when the benchmark contract is stable and variant search remains explicit, comparable, and discardable.
- **Decision**: Parallel target. This is the fastest submission path if the goal is near-term publication evidence.

### Strongest Evidence

- The repository already separates stable core (`pcd_dogfooding --summary-json`) from discardable experimental manifests.
- Comparison state is externalized into generated docs instead of being trapped in code comments or ad-hoc notebooks.
- The runner now supports `--reuse-existing`, which makes expensive comparisons reproducible without rerunning every variant.
- Experiment-facing and publication-facing docs can now be refreshed in one command via `python3 evaluation/scripts/refresh_study_docs.py`.
- Paper-ready tables and Pareto figures are now exported from aggregate JSON via `python3 evaluation/scripts/export_paper_assets.py`.

### Gaps

- Container or pinned environment instructions are still missing.
- The public Pages site does not yet expose the experiment docs alongside the benchmark snapshot.
- Dataset bootstrap is still manual, so full artifact replay is not yet one-command from a clean machine.

### Next Experiments

- Ship a pinned environment definition or container recipe.
- Publish `docs/paper_tracks.md` and `docs/paper_roadmap.md` through Pages so the artifact narrative is visible externally.
- Add dataset bootstrap helpers so a clean machine can reproduce the study without manual path setup.

## Track C: Focused Method Paper

- **Readiness**: Low (20/100)
- **Primary claim**: One concrete localization method variant materially improves the accuracy / throughput frontier beyond the current repository baselines.
- **Decision**: Hold. Keep collecting evidence, but do not make this the main paper narrative yet.

### Strongest Evidence

- CT-LIO now has a public reference-based trade-off problem with three bounded variants under one interface.
- LiTAMIN2 already has a measurable throughput-oriented variant family and a paper-profile family under the same evaluator.
- The repository can now keep weaker and stronger method variants alive without deleting the evidence trail.

### Gaps

- No single method currently shows a strong enough multi-dataset improvement claim for a focused algorithm paper.
- Current CT-LIO evidence is reference-based and slow; it is not yet enough for a strong method contribution.
- LiTAMIN2 speedups are real inside this repository, but they are not yet benchmarked broadly enough to support a standalone claim.

### Next Experiments

- Pick one method only after it shows repeatable improvement on at least two open sequences.
- Add ablations that isolate exactly one new algorithmic idea instead of a profile bundle.
- Treat the focused-method path as a by-product of Track A, not the current main plan.
