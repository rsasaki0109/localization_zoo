# L-LO

## Paper
- Feiya Li, Chunyun Fu, Dongye Sun
- "L-LO: Enhancing Pose Estimation Precision via a Landmark-Based LiDAR Odometry", arXiv:2312.16787, 2023
- No public author code at the time of writing; reimplemented from the paper.

## What This Repository Implements

Frame-to-frame landmark odometry following Sec. III of the paper:

- **pre-processing**: line-fit ground segmentation (in the spirit of the paper's reference [8]), Euclidean clustering of non-ground points, per-cluster statistical outlier removal (eqs. 1-2), cluster centres (eq. 3)
- **initial landmark matching**: nearest cluster centre between consecutive frames, accepted when closer than the mean nearest-centre distance (eq. 4)
- **horizontal pose**: each matched pair is split into upper/lower layers at the shared mean height, each layer is projected to a 2D convex hull, and the layer whose hulls are most similar under the comprehensive index (turning-function area difference + Hausdorff distance, eqs. 5-9, alpha = beta = 0.5) is kept; (dx, dy, dyaw) maximises the summed hull-intersection area (eqs. 10-12) with the coordinate rotation method and the adaptive step of eq. 13
- **vertical pose**: roll assumed zero; pitch from fitted ground normals; z follows from composing the pitched motion (eq. 16). `pitch_mode=1` (opt-in) instead fits the same ground patch from both frames and takes the pitch change and the height change from the two planes; see [Vertical pose and the gap to the paper](#vertical-pose-and-the-gap-to-the-paper)

`pcd_dogfooding --methods l_lo` runs it.

## Reproduction notes and deviations

The paper gives **no numeric parameters** (clustering threshold, ground-segmentation thresholds, outlier-removal K, the step coefficients C, termination threshold). Default values in `LLOParams` are this repository's choices. The `seq07_tuned` overrides below were selected on KITTI 07 and are distinguished from those defaults.

Interpretations where the text is ambiguous or not physically consistent:

- **Pitch.** The paper sums, per frame, the angle between ground planes fitted *in front of and behind* the vehicle (eq. 15). Read literally this measures terrain curvature, would add the same slope change once per frame while the vehicle crosses it, and `arccos` has no sign. This implementation uses the signed change of the fitted ground normal *between consecutive frames* instead.
- **Prediction.** Initial centre matching, layer similarity, and the overlap search all start from a constant-velocity prediction; the paper does not say how they are initialised.
- **Turning function.** The "difference between the areas" under the two turning functions (eq. 5) is computed as the absolute difference of the two areas.
- **Hausdorff distance.** Computed after expressing both hulls in frame k-1 with the prediction, so it measures shape and size rather than raw motion.

## Results

See `experiments/results/l_lo_kitti_seq_*_full_matrix.json`, the parameter-sensitivity section below, and the Table 6 rows in `docs/assets/paper/paper_ratio_table.csv`.

## Parameter sensitivity

The paper gives no numeric parameters, so every value above is a conventional choice. To see how much of the gap is parameter choice, 30 settings were run on **seq 07 only** (`experiments/l_lo_kitti_seq_07_*_sweep_matrix.json`); `--l-lo-set key=value` overrides any `LLOParams` field.

- Only the clustering cell size matters much. The 100 m RPE on seq 07 is 1.18 % / 1.28 % / 1.33 % (default) / 1.83 % / 2.11 % for `cluster_cell` 0.25 / 0.35 / 0.5 / 0.7 / 1.0, and 1.32 % at 0.15. Step size, similarity weights, termination, pitch band, voxel size, and SOR all stay within 1.30-1.40 %.
- The best setting (`cluster_cell=0.25 ground_tolerance=0.15 max_range=80`, 1.17 %) was then run **unchanged** on 00/02/05/08 as the `seq07_tuned` variant:

| Seq | default | seq07_tuned | paper | ratio (tuned) |
|---|---:|---:|---:|---:|
| 00 (held out) | 1.653 % | 1.479 % | 0.91 % | 1.63x |
| 02 (held out) | 4.749 % | 4.478 % | 2.35 % | 1.91x |
| 05 (held out) | 1.323 % | 1.473 % | 0.82 % | 1.80x |
| 07 (chosen on) | 0.998 % | 1.004 % | 0.69 % | 1.46x |
| 08 (held out) | 1.778 % | 1.661 % | 1.37 % | 1.21x |

(Historical comparison on the five sequences above, official KITTI RTE.) The tuned setting lowers the 100 m RPE on every held-out sequence, by 6-8 %, but on the official metric it helps 00/02/08 and hurts 05; the geometric-mean ratio moves only from 1.62x to 1.58x. Absolute trajectory error is not consistently better either (seq 02: 58 m to 142 m; seq 00: 19 m to 16 m). Parameter choice therefore explains little of the gap to the paper; the remaining difference is more likely in method details the paper leaves open (see the deviations above) and in the vertical part of the pose (next section). Table 6 now uses `seq07_tuned` over every sequence with a paper value; the full-coverage mean is in the fixed-configuration section below (`repo_variants` in `evaluation/data/paper_reported_numbers.json`).

## Vertical pose and the gap to the paper

On seq 07, much of the measured gap appears in the vertical part of the pose: the `seq07_tuned` trajectory scores **0.577 %** when both it and the ground truth are reduced to (x, y, yaw). The full 6-DoF score is 1.004 %, and the paper reports 0.69 %. This comparison does not diagnose the cause on other sequences. The default `pitch_mode=0` never models height: each step's z is zero. Its pitch is the sensor's tilt against the local ground, measured against frame 0. Road grade is not part of either.

**Same-patch vertical estimate (opt-in, `pitch_mode=1`).** The ground patch around the new sensor position (`patch_half_length` x `patch_half_width`) is fitted as a plane in both frames. The change of the plane normal gives the relative pitch. The change of the plane offset gives the height change in frame k-1. `pitch_mode=2` keeps the mode-0 pitch and uses only the height change. A prototype took the horizontal motion from ground truth and used the same plane fit (seq 07, 1100 frames):

- The height change is good: error mean -1.5 mm, rms 6.8 mm per frame.
- The pitch change is not. Its error has rms 0.94 mrad and a systematic **+0.24 mrad per frame**, which adds up to 0.08 rad over 300 frames. The bias comes from the known elevation error of the KITTI HDL-64 scans: flat ground looks bowl-shaped, and the patch sits at different ranges in the two frames. `vertical_angle_correction_deg` raises every beam before processing. Over the first 400 frames the bias falls from 0.20 to 0.14, 0.08, 0.02 and -0.04 mrad per frame for 0.1°, 0.2°, 0.3°, 0.4° and 0.5°.
- Even unbiased, frame-to-frame pitch noise is integrated twice into position.

The full C++ pipeline on seq 07 (official RTE, `seq07_tuned` settings):

| vertical variant | RTE |
|---|---:|
| `pitch_mode=0` (default) | **1.004 %** |
| `pitch_mode=0`, correction 0.4° | 1.044 % |
| `pitch_mode=1` | 1.506 % |
| `pitch_mode=1`, correction 0.3° | 1.250 % |
| `pitch_mode=1`, correction 0.4° | 1.398 % |
| `pitch_mode=1`, correction 0.5° | 1.777 % |
| `pitch_mode=2` | 1.602 % |
| `pitch_mode=2`, correction 0.4° | 1.625 % |

None of them beats the default, so the defaults are unchanged.

**Tested GT-derived roll-free baselines.** The paper assumes a null roll angle. The trajectories below are built from the ground truth with roll removed and scored against the full ground truth (official RTE):

| Seq | x, y, yaw only (z = 0, no pitch) | per-step pitch + z, roll = 0 | GT positions, attitude without roll | paper |
|---|---:|---:|---:|---:|
| 00 | 0.976 % | 1.124 % | 1.055 % | 0.91 % |
| 02 | 2.802 % | 2.227 % | 0.955 % | 2.35 % |
| 05 | 0.902 % | 0.745 % | 0.685 % | 0.82 % |
| 07 | 0.759 % | 1.083 % | 0.909 % | 0.69 % |
| 08 | 1.098 % | 1.233 % | 1.419 % | 1.37 % |

On 00 and 07 the paper's numbers are below all three tested GT-derived roll-free baselines above. These finite comparisons show a discrepancy worth investigating; they do not establish a lower bound over all possible roll-free estimates or prove the paper values impossible under its motion model. On 07, the per-step estimate from the prototype (correction 0.4°, horizontal from ground truth) scores 0.979 %, between the planar 0.759 % and the ground-truth pitch and z at 1.083 %.

The 1.46x gap on 07 is supported by these observations:

- the horizontal module, which is at 0.577 % in 2D;
- the vertical part, which no frame-to-frame ground-plane estimate here improves;
- the tested GT-derived roll-free baselines also exceed the paper value; this does not prove every possible roll-free estimate must do so.

<!-- FIXED-KITTI-RESULTS:START -->

## Full KITTI comparison with one configuration

Fixed variant: `seq07_tuned`.

cluster_cell=0.25, ground_tolerance=0.15 and max_range=80 were selected by the existing 100 m RPE sweeps on sequence 07 and previously transferred unchanged to 00/02/05/08. Sequence 01 is NA in the paper and excluded.

This pipeline never uses per-frame GT seeding; GT is used only for the initial rigid evaluation anchor. Keep the original variant arguments verbatim, including an explicit --no-gt-seed where present.



No new estimator behavior or default change is introduced by this coverage run.

Exact fixed-setting CLI (substitute the sequence for NN):

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_NN_full experiments/reference_data/kitti_seq_NN_full_gt.csv 0 --methods l_lo --l-lo-set cluster_cell=0.25 --l-lo-set ground_tolerance=0.15 --l-lo-set max_range=80 --summary-json l_lo_seq_NN.json
```

Full sequences, official KITTI translational RTE (100-800 m), no GT seed.
Existing committed results are reused on 00, 02, 05, 07, 08.
The other rows are added full-sequence runs. Every row uses the same variant.
Sequence 01 is NA in the paper and is excluded from the comparison and mean.

| Seq | Paper RTE [%] | Fixed RTE [%] | Repo / paper | FPS |
|---|---:|---:|---:|---:|
| 00 (reused) | 0.910 | 1.479 | 1.63x | 4.374 |
| 02 (reused) | 2.350 | 4.478 | 1.91x | 3.578 |
| 03 | 2.920 | 3.066 | 1.05x | 2.824 |
| 04 | 2.280 | 2.135 | 0.94x | 0.877 |
| 05 (reused) | 0.820 | 1.473 | 1.80x | 4.382 |
| 06 | 0.660 | 1.315 | 1.99x | 3.259 |
| 07 (reused) | 0.690 | 1.004 | 1.46x | 4.450 |
| 08 (reused) | 1.370 | 1.661 | 1.21x | 3.999 |
| 09 | 1.890 | 4.122 | 2.18x | 1.162 |
| 10 | 1.900 | 4.105 | 2.16x | 1.309 |

Geometric mean over all sequences with paper values: **1.57x**
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

**09: 4.122 % vs 1.890 % (2.18x).**
The estimated path length is 0.999 of GT.
For GT steps of at least 0.1 m,
4 of 1590 steps differ in length by more than 20 %;
the local translation-vector error exceeds 20 % on 9 steps.
The longest runs of step length errors have destination-frame indices 6-6, 96-96, 118-118 (zero-based; inclusive).
All intervals are retained in the diagnostic JSON.

The largest relative-translation-error blocks locate where drift grows.
Each block contains up to 100 consecutive steps; these endpoint errors
are diagnostics, not official KITTI RTE.

Blocks spanning at least 1 m of GT travel are ranked below; the JSON
also retains blocks with less travel.

| Pose interval | GT distance [m] | Est / GT path | Position error start / end [m] | Block error [%] |
|---|---:|---:|---:|---:|
| 1500-1590 | 61.5 | 1.007 | 27.1 / 27.1 | 7.03 |
| 0-100 | 80.4 | 1.011 | 0.0 / 4.7 | 5.81 |
| 500-600 | 104.5 | 1.005 | 38.0 / 44.2 | 5.27 |

Diagnostic source: `experiments/results/runs/l_lo_kitti_seq_09_full_paper_comp_matrix/seq07_tuned/step_diagnostics.json`.

**10: 4.105 % vs 1.900 % (2.16x).**
The estimated path length is 1.000 of GT.
For GT steps of at least 0.1 m,
7 of 1153 steps differ in length by more than 20 %;
the local translation-vector error exceeds 20 % on 17 steps.
The longest runs of step length errors have destination-frame indices 3-4, 10-10, 51-51 (zero-based; inclusive).
All intervals are retained in the diagnostic JSON.

The largest relative-translation-error blocks locate where drift grows.
Each block contains up to 100 consecutive steps; these endpoint errors
are diagnostics, not official KITTI RTE.

Blocks spanning at least 1 m of GT travel are ranked below; the JSON
also retains blocks with less travel.

| Pose interval | GT distance [m] | Est / GT path | Position error start / end [m] | Block error [%] |
|---|---:|---:|---:|---:|
| 700-800 | 127.2 | 0.999 | 7.2 / 8.9 | 6.61 |
| 600-700 | 50.7 | 0.999 | 6.8 / 7.2 | 6.12 |
| 1100-1200 | 17.1 | 1.025 | 13.0 / 14.2 | 4.40 |

Diagnostic source: `experiments/results/runs/l_lo_kitti_seq_10_full_paper_comp_matrix/seq07_tuned/step_diagnostics.json`.

The total path lengths remain close to GT, so total travel distance
alone does not capture these failures. The local block errors and
absolute position-error endpoints measure different effects and need
not grow together. These saved-pose observations locate disagreements;
their association or solver causes remain open.


Saved-pose diagnostics are generated with
`evaluation/scripts/diagnose_kitti_steps.py --estimate <trajectory_kitti.txt>
--gt <evaluated_gt_kitti.txt> --summary <summary.json> --method "L-LO"
--output <step_diagnostics.json>`; each JSON records the exact command.
The script checks its full-sequence RTE against the original run summary.

<!-- FIXED-KITTI-RESULTS:END -->
