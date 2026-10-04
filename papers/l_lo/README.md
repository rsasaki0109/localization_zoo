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

See `experiments/results/l_lo_kitti_seq_*_full_matrix.json` and the Table 6 rows in `docs/assets/paper/paper_ratio_table.csv`.
