# DALI-SLAM (from-paper reimplementation)

**Paper:** *DALI-SLAM: Degeneracy-aware LiDAR-inertial SLAM with novel distortion
correction and accurate multi-constraint pose graph optimization*, Wang et al.,
*ISPRS J. Photogrammetry and Remote Sensing* 221 (2025), DOI
10.1016/j.isprsjprs.2025.01.036. Author code is public (FAST-LIO2 + basalt
spline); this is an independent reimplementation on the repo's own point-to-plane
pipeline, porting DALI's two contributions.

## Core ideas

1. **DS-MDC (dual-spline motion distortion correction).** The original fits two
   SE3 cubic B-splines — `lf_spline` over the LiDAR scan-to-map poses, and
   `lf_imu_spline` over the IMU-integrated poses — and corrects the discrete IMU
   poses by

   ```
   T_correct(t) = T_lf(t) · T_lf_imu(t)⁻¹ · T_imu_integ(t)
   ```

   before deskewing. The continuous-time spline accounts for the accumulated
   error of IMU integration within the scan.

2. **Degeneracy-aware update (solution remapping).** The 6×6 coefficient matrix
   of the active point Jacobians is eigen-decomposed; directions whose eigenvalue
   falls below a threshold are removed from the state update so degenerate
   directions hold their prediction instead of drifting.

## Scope / approximations for KITTI (pure-LiDAR)

- **`lf_imu_spline` is replaced by a constant-velocity / quadratic extrapolated
  trajectory** (no IMU in KITTI). The dual-spline correction is realized as a
  spline (2nd-order, acceleration-aware) extrapolation of the recent scan-to-map
  pose deltas, used both for prediction and for deskew.
- **Per-point time is recovered from the azimuth** of a spinning LiDAR
  (`s = (atan2(y,x)+π)/2π ∈ [0,1)`), since the accumulated KITTI PCDs carry no
  per-point timestamp. Points are deskewed to the scan-end frame by
  `p_end = Exp((s−1)·twist)⊙p`.
- **Honest note:** under a constant-velocity model the LiDAR trajectory and the
  motion-model trajectory nearly coincide, so the DS-MDC correction term is much
  smaller than in the paper (whose numbers assume a MEMS IMU). On KITTI the
  **degeneracy-aware update is the primary contributor**, as in the paper's
  ablation. The degeneracy step is IMU-independent and ported faithfully.
- The threshold is **absolute** (paper default `degeneracy = 4.48`), not a
  condition-number ratio, so the rotation block's range²-scaled large eigenvalues
  do not cause false firing — only genuinely small (unconstrained) directions
  drop out.
- The multi-constraint pose-graph back end (MC-PGO) and loop closure are out of
  scope (front-end odometry only). ESKF/IKFoM is replaced by the repo's
  Gauss-Newton point-to-plane solve.

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `enable_deskew` | spline deskew on/off | true |
| `spline_quadratic` | 2nd-order (accel) extrapolation | true |
| `degeneracy_threshold` | min-eigenvalue gate | 4.48 (paper) |
| `enable_degeneracy` | solution remapping on/off | true |

## Tests

`test_dali_slam` covers: known-translation recovery (degeneracy on, well
conditioned → no false firing); corridor degeneracy detection and direction
removal vs a closed box; motion recovery with spline deskew enabled.
