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
