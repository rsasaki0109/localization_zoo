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

2. **Degeneracy-aware update (solution remapping).** The translation Hessian
   `H_tt` is eigen-decomposed; translation directions whose eigenvalue falls below
   a floor are removed from the state's translation update so degenerate
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
- **Honest note (deskew):** under a constant-velocity model the LiDAR and
  motion-model trajectories nearly coincide, so the DS-MDC correction is much
  smaller than in the paper (whose numbers assume a MEMS IMU). On the accumulated
  KITTI PCDs the azimuth-timed constant-velocity deskew actually *hurts* — it
  amplifies range error and diverges on the long seq 00 — so **`enable_deskew`
  defaults to off on KITTI** (re-enable with `--dali-slam-deskew`). The repo's
  other no-deskew methods already match KISS-ICP on this harness, indicating the
  accumulated clouds need no extra deskew here.
- **Degeneracy threshold:** the paper uses an *absolute* eigenvalue threshold
  (`degeneracy = 4.48`) calibrated to FAST-LIO's IMU-fused, normalized Hessian.
  That scale does not transfer to this Gauss-Newton point-to-plane pipeline —
  the absolute threshold false-fires on most KITTI frames, drops good directions
  and diverges on seq 00. It is replaced by the repo's standard **translation-block
  relative floor** (`λ_k < ratio·λmax_t`, as in damm_loam/lodestar): degeneracy is
  judged on the translation block only (the rotation Hessian scales with range²
  and its condition number is always large), and stays silent on well-conditioned
  KITTI urban scans.
- The multi-constraint pose-graph back end (MC-PGO) and loop closure are out of
  scope (front-end odometry only). ESKF/IKFoM is replaced by the repo's
  Gauss-Newton point-to-plane solve.

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `enable_deskew` | spline deskew on/off | false on KITTI (lib default true) |
| `spline_quadratic` | 2nd-order (accel) extrapolation | false |
| `degeneracy_ratio` | translation-block relative floor `λ_k<ratio·λmax_t` | 0.02 |
| `enable_degeneracy` | solution remapping on/off | true |

## Tests

`test_dali_slam` covers: known-translation recovery (degeneracy on, well
conditioned → no false firing); corridor degeneracy detection and direction
removal vs a closed box; motion recovery with spline deskew enabled.
