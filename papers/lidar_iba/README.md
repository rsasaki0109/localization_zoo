# Consistency-Improved LiDAR-IBA (from-paper reimplementation)

**Paper:** *A Consistency-Improved LiDAR(-Inertial) Bundle Adjustment*, Li et al.,
2026 ([arXiv:2602.06380](https://arxiv.org/abs/2602.06380)). Author code not
released.

## Core ideas

1. **Stereographic plane parameterization (Eq.9, 20).** A unit plane normal
   `n ∈ S²` is represented by 2 stereographic coordinates `u = [ux, uy]`:

   ```
   denom = 1 + ux² + uy²
   n(u)  = [ 2ux, 2uy, ux²+uy²−1 ] / denom        ⇒  ‖n(u)‖ = 1 (automatic)
   ```

   This avoids the over-parameterization and singular Jacobian of the
   closest-point representation, which **diverges for ground planes** that pass
   near the origin (the dominant feature in driving data). The analytic
   `∂n/∂u` (3×2) is implemented and bounded.

2. **Sliding-window plane bundle adjustment.** Recent keyframe poses and shared
   plane landmarks (stereographic) are jointly optimized by Gauss-Newton over
   point-to-plane residuals `r = n(u)·(R_k p + t_k) + OA`.

3. **FEJ (First-Estimates Jacobian).** To avoid injecting spurious information
   into unobservable directions, the marginalized (oldest) keyframe's
   linearization point is frozen.

## Scope / approximations for KITTI (pure-LiDAR)

- **Constant-velocity** prediction replaces IMU preintegration (no IMU in
  KITTI); the IMU residual `r_I` is dropped. The paper's headline contributions
  (stereographic parameterization + FEJ) are purely geometric and IMU-independent.
- **FEJ is realized as gauge-fixing the oldest keyframe** in the window and
  freezing its linearization point; a full Schur-complement marginalization with
  a persistent prior carried across windows is out of scope. With IMU removed,
  the unobservable subspace is the global 6-DoF, which the oldest-keyframe gauge
  fix anchors.
- Plane landmarks are re-extracted per window from a world-frame plane-voxel
  grid (PCA normal + planarity gate), with observation/landmark caps to keep the
  dense BA tractable. Long-term cross-window data association and the loop-closure
  back end are out of scope.
- Front-end is point-to-plane against a voxel-hash map; LOAM-style edge features
  are not separately modeled.

The paper itself (v1) contains the theory (observability analysis, proofs) but
no experiment section, parameter tables, or dataset numbers, so absolute
parameters here are chosen for KITTI, not taken from the paper.

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `keyframe_interval` | frames per keyframe | 2 |
| `window_size` | keyframes in window W | 5 |
| `ba_iterations` | Gauss-Newton iters | 4 |
| `plane_voxel` | landmark extraction voxel | 2.0 m |
| `plane_min_planarity` | planarity gate | 0.6 |
| `max_landmarks` | dense-solve size cap | 150 |
| `stereo_radius_margin` | δ in ‖u‖≤1+δ | 0.2 |
| `lidar_sigma` | point noise σ (weight 1/σ) | 0.05 |

## KITTI benchmark profile

The public KITTI full-sequence artifacts use `--lidar-iba-no-ba`. On this
IMU-free protocol, the sliding-window BA lowers whole-run ATE in some runs but
hurts the primary translational RPE metric and costs about 3x throughput. The
committed no-BA profile keeps the stereographic plane front-end and FEJ-tested
BA implementation available, but reports the stronger local-drift setting.

| Sequence | Frames | ATE [m] | RPE [%/100 m] | RPE rot [deg/m] | FPS | Artifact |
|---|---:|---:|---:|---:|---:|---|
| seq00 | 4541 | 11.342 | 0.841 | 0.007 | 3.09 | [`seq00_lidar_iba.json`](../../docs/benchmarks/kitti_full_new_methods/seq00_lidar_iba.json) |
| seq07 | 1101 | 1.339 | 0.633 | 0.006 | 3.09 | [`seq07_lidar_iba.json`](../../docs/benchmarks/kitti_full_new_methods/seq07_lidar_iba.json) |

### BA on/off ablation (IMU-free KITTI)

| Sequence | no-BA RPE | BA-on RPE | Δ RPE | no-BA ATE | BA-on ATE | Artifacts |
|---|---:|---:|---:|---:|---:|---|
| seq00 | 0.841% | 2.280% | +1.44 pt | 11.34 m | 10.93 m | [`no-BA`](../../docs/benchmarks/kitti_full_new_methods/seq00_lidar_iba.json) / [`BA-on`](../../docs/benchmarks/kitti_full_new_methods/seq00_lidar_iba_ba_on.json) |
| seq07 | 0.633% | 2.530% | +1.90 pt | 1.34 m | 1.15 m | [`no-BA`](../../docs/benchmarks/kitti_full_new_methods/seq07_lidar_iba.json) / [`BA-on`](../../docs/benchmarks/kitti_full_new_methods/seq07_lidar_iba_ba_on.json) |

Paired summary: [`lidar_iba_ba_ablation.json`](../../docs/benchmarks/kitti_full_new_methods/lidar_iba_ba_ablation.json).
Sliding-window BA slightly lowers whole-run ATE but worsens translational RPE and
throughput on this constant-velocity protocol; the frozen bundle keeps no-BA as
the main KITTI row.

Reproduction command shape:

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_00_full \
  experiments/reference_data/kitti_seq_00_full_gt.csv 0 \
  --methods lidar_iba --no-gt-seed --lidar-iba-no-ba \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq00_lidar_iba.json
```

## Tests

`test_lidar_iba` covers: stereographic round-trip with unit-norm guarantee
(including the north-pole sign flip); analytic `∂n/∂u` vs finite differences;
known-translation recovery with the windowed BA enabled.
