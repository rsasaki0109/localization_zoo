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

The paper gives **no numeric parameters** (clustering threshold, ground-segmentation thresholds, outlier-removal K, the step coefficients C, termination threshold). All values in `LLOParams` are this repository's choices and were not tuned on KITTI.

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

(Official KITTI RTE.) The tuned setting lowers the 100 m RPE on every held-out sequence, by 6-8 %, but on the official metric it helps 00/02/08 and hurts 05; the geometric-mean ratio moves only from 1.62x to 1.58x. Absolute trajectory error is not consistently better either (seq 02: 58 m to 142 m; seq 00: 19 m to 16 m). Parameter choice therefore explains little of the gap to the paper; the remaining difference is more likely in method details the paper leaves open (see the deviations above) and in the vertical part of the pose (next section). Table 6 uses `seq07_tuned` (`repo_variants` in `evaluation/data/paper_reported_numbers.json`).

## Vertical pose and the gap to the paper

The horizontal module is not where the gap is. On seq 07, the `seq07_tuned` trajectory scores **0.577 %** when both it and the ground truth are reduced to (x, y, yaw). The full 6-DoF score is 1.004 %, and the paper reports 0.69 %. The excess is in the vertical part. The default `pitch_mode=0` never models height: each step's z is zero. Its pitch is the sensor's tilt against the local ground, measured against frame 0. Road grade is not part of either.

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

**What a roll-free model can score at all.** The paper assumes a null roll angle. The trajectories below are built from the ground truth with roll removed and scored against the full ground truth (official RTE):

| Seq | x, y, yaw only (z = 0, no pitch) | per-step pitch + z, roll = 0 | GT positions, attitude without roll | paper |
|---|---:|---:|---:|---:|
| 00 | 0.976 % | 1.124 % | 1.055 % | 0.91 % |
| 02 | 2.802 % | 2.227 % | 0.955 % | 2.35 % |
| 05 | 0.902 % | 0.745 % | 0.685 % | 0.82 % |
| 07 | 0.759 % | 1.083 % | 0.909 % | 0.69 % |
| 08 | 1.098 % | 1.233 % | 1.419 % | 1.37 % |

On 00 and 07 the paper's numbers are below every roll-free trajectory built from the ground truth. So, under the official metric, these two numbers cannot come from the paper's stated motion model with any horizontal and vertical estimates. On 07, the per-step estimate from the prototype (correction 0.4°, horizontal from ground truth) scores 0.979 %, between the planar 0.759 % and the ground-truth pitch and z at 1.083 %.

The 1.46x gap on 07 therefore has three parts:

- the horizontal module, which is at 0.577 % in 2D;
- the vertical part, which no frame-to-frame ground-plane estimate here improves;
- a paper value that is out of reach for a roll-free pose.

