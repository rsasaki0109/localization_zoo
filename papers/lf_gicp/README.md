# LF-GICP

## Paper
- Eunsoo Im
- "LF-GICP: Parameter-Free Degeneracy Handling for LiDAR Odometry via a Voxel-Normal Localizability Field", arXiv:2608.19522, 2026
- No public author code at the time of writing; reimplemented from the paper (CC BY-NC-SA 4.0 paper; the code here is an independent implementation).

## What This Repository Implements

A GICP scan-to-map odometry (paper Sec. III-A) with the paper's degeneracy handling:

- **backend**: age-windowed voxel map of per-voxel Gaussians (1.0 m voxels, >= 3 points, last 500 scans), 0.3 m source voxels, 1-80 m range, constant-velocity prediction, KISS-style adaptive correspondence threshold, Huber (delta = 1), covariance regularization `C + beta I` with beta = 2.0 (field planarity uses beta_f = 0.25, see below), up to 12 Gauss-Newton iterations with re-association each iteration
- **localizability field** (Eq. 5): `M = sum_v rho_v n_v n_v^T` over at most 4096 stride-sampled map voxels, `f0 = lambda_min(M) / tr(M)`
- **absence statistic** (Eq. 6): `lambda0 = lambda_min(M) / |V|`
- **gate** (Sec. III-F, Alg. S1): 20-frame trailing medians, enter when `med f0 < 0.165` and `med lambda0 < 0.0143`, exit when `med f0 > 0.185` or the lambda0 condition fails
- **soft Fisher weighting** (Eq. 7): when the gate is active, each correspondence is reweighted by its information along the weakest eigenvector of the Gauss-Newton Hessian, normalized to mean 1

`pcd_dogfooding --methods lf_gicp` runs it; `--lf-gicp-no-mitigation` disables the gate (the paper's vanilla VGICP backend ablation), `--lf-gicp-beta X` changes beta, and `--lf-gicp-field-beta X` changes beta_f.

## Reproduction notes and deviations

- **Planarity definition.** Sec. III-C says the field is built "before regularization", but Table IV shows `f0` changing with beta, which only happens if `rho_v = (lambda3 - lambda2) / lambda3` is computed from a regularized information `(C + beta_f I)^-1`. The field therefore has its own `field_regularization` (beta_f), separate from the GICP beta = 2.0.
- **lambda0 scale and beta_f.** The gate thresholds only make sense on the paper's `lambda0` scale, where open-road "dilution" scenes span 0.0168-0.0334 (Fig. 3). On the paper's own calibration trace (KITTI 00, first 500 frames) the median `lambda0` is 0.191 / 0.0201 / 0.0110 / 0.0058 / 0.0030 for beta_f = 0 / 0.25 / 0.5 / 1.0 / 2.0. The first run used beta_f = 2.0; the gate then stayed on for 82-100 % of KITTI 02/08 frames and hurt accuracy, which is the dilution failure the paper's `lambda0` condition exists to prevent. After seeing that, beta_f was re-chosen by a rule that uses only the calibration trace and the paper's published range, not any evaluated error: the smallest beta_f whose median falls inside the dilution range, so **beta_f = 0.25**. GEODE tunnels are not available here, so the gate has not been checked on genuine absence.
- **Unspecified details** filled with conventional choices: minimum range 1 m, voxel eviction by last-update frame, first-order SE(3) increment, 27-voxel correspondence search.
- No deskewing: KITTI Odometry scans have no per-point times (the paper also evaluates KITTI without deskewing and without the mounting-angle correction).

## Results

See `experiments/results/lf_gicp_kitti_seq_*_full_matrix.json` and the Table 6 rows in `docs/assets/paper/paper_ratio_table.csv`.

<!-- FIXED-KITTI-RESULTS:START -->

## Full KITTI comparison with one configuration

Fixed variant: `paper_default`.

field beta 0.25 was calibrated on sequence 00 first 500 frames against the paper dilution range, after earlier 02/08 runs exposed excessive gating. Those sequences influenced the decision to recalibrate; no added sequence influences the frozen setting.

This pipeline never uses per-frame GT seeding; GT is used only for the initial rigid evaluation anchor. Keep the original variant arguments verbatim, including an explicit --no-gt-seed where present.



No new estimator behavior or default change is introduced by this coverage run.

Exact fixed-setting CLI (substitute the sequence for NN):

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_NN_full experiments/reference_data/kitti_seq_NN_full_gt.csv 0 --methods lf_gicp --summary-json lf_gicp_seq_NN.json
```

Full sequences, official KITTI translational RTE (100-800 m), no GT seed.
Existing committed results are reused on 00, 02, 05, 07, 08.
The other rows are added full-sequence runs. Every row uses the same variant.


| Seq | Paper RTE [%] | Fixed RTE [%] | Repo / paper | FPS |
|---|---:|---:|---:|---:|
| 00 (reused) | 0.692 | 0.664 | 0.96x | 3.566 |
| 01 | 1.822 | 1.816 | 1.00x | 1.997 |
| 02 (reused) | 1.084 | 1.075 | 0.99x | 3.271 |
| 03 | 1.207 | 0.785 | 0.65x | 3.385 |
| 04 | 0.814 | 0.629 | 0.77x | 2.316 |
| 05 (reused) | 0.577 | 0.512 | 0.89x | 3.549 |
| 06 | 0.546 | 0.543 | 0.99x | 2.267 |
| 07 (reused) | 0.496 | 0.411 | 0.83x | 4.125 |
| 08 (reused) | 0.878 | 1.005 | 1.14x | 2.574 |
| 09 | 0.566 | 0.725 | 1.28x | 1.632 |
| 10 | 0.837 | 5.616 | 6.71x | 2.030 |

Geometric mean over all sequences with paper values: **1.12x**
with the fixed setting. Table 6 is restricted to this same variant by
`repo_variants` in `evaluation/data/paper_reported_numbers.json`.
Historical ablations remain available in their original
result aggregates.

FPS describes the recorded execution: the new runs use one OpenMP
thread per method and may overlap on the same sequence. Reused FPS
comes from historical runs on their recorded hosts.

Source aggregates and all unrounded rows are recorded in
`experiments/results/remaining_kitti_coverage/summary.json`.
New per-run summaries, host records, logs, estimated poses, evaluated GT
and step diagnostics are committed under the corresponding
`experiments/results/runs/` directory. Exact execution commands and the
isolated-output launcher used for overlapping methods are recorded
in `experiments/results/remaining_kitti_coverage/`.

### Sequences above twice the paper value

**10: 5.616 % vs 0.837 % (6.71x).**
The estimated path length is 1.010 of GT.
For GT steps of at least 0.1 m,
26 of 1153 steps differ in length by more than 20 %;
the local translation-vector error exceeds 20 % on 34 steps.
The longest runs of step length errors have destination-frame indices 1-8, 209-211, 383-386 (zero-based; inclusive).
All intervals are retained in the diagnostic JSON.

The largest relative-translation-error blocks locate where drift grows.
Each block contains up to 100 consecutive steps; these endpoint errors
are diagnostics, not official KITTI RTE.

Blocks spanning at least 1 m of GT travel are ranked below; the JSON
also retains blocks with less travel.

| Pose interval | GT distance [m] | Est / GT path | Position error start / end [m] | Block error [%] |
|---|---:|---:|---:|---:|
| 300-400 | 81.2 | 1.069 | 5.4 / 15.0 | 8.59 |
| 200-300 | 70.4 | 1.048 | 1.5 / 5.4 | 6.42 |
| 400-500 | 97.1 | 0.999 | 15.0 / 29.9 | 1.68 |

Diagnostic source: `experiments/results/runs/lf_gicp_kitti_seq_10_full_paper_comp_matrix/paper_default/step_diagnostics.json`.

The total path lengths remain close to GT, so total travel distance
alone does not capture these failures. The local block errors and
absolute position-error endpoints measure different effects and need
not grow together. These saved-pose observations locate disagreements;
their association or solver causes remain open.


Saved-pose diagnostics are generated with
`evaluation/scripts/diagnose_kitti_steps.py --estimate <trajectory_kitti.txt>
--gt <evaluated_gt_kitti.txt> --summary <summary.json> --method "LF-GICP"
--output <step_diagnostics.json>`; each JSON records the exact command.
The script checks its full-sequence RTE against the original run summary.

<!-- FIXED-KITTI-RESULTS:END -->

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader full KITTI replay

All 11 selected stored LF-GICP full-sequence cells complete with native exits 0. 11 match historical 100 m RPE, ATE and frame count; 0 differ. Each cell uses its original recorded arguments, which may differ by sequence. This audit does not select a new fixed setting or paper value. The optional near-range cut still defaults to zero/off.

[Complete report and commands](../../experiments/results/shared_near_range/completion/03/report/summary.json) retain all original audit exits, summaries, logs, saved poses, frozen comparisons and verified input cleanup. KITTI replay does not establish fresh NCLT accuracy.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
