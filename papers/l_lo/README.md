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
- **vertical pose**: roll assumed zero; pitch from fitted ground normals; z follows from composing the pitched motion (eq. 16)

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

(Official KITTI RTE.) The tuned setting lowers the 100 m RPE on every held-out sequence, by 6-8 %, but on the official metric it helps 00/02/08 and hurts 05; the geometric-mean ratio moves only from 1.62x to 1.58x. Absolute trajectory error is not consistently better either (seq 02: 58 m to 142 m; seq 00: 19 m to 16 m). Parameter choice therefore explains little of the gap to the paper; the remaining difference is more likely in method details the paper leaves open (see the deviations above). Table 6 uses `seq07_tuned` (`repo_variants` in `evaluation/data/paper_reported_numbers.json`).
