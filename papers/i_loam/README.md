# I-LOAM: Intensity Enhanced LiDAR Odometry and Mapping (from-paper reimplementation)

**Paper:** Yeong-Sang Park, Hyesu Jang, Ayoung Kim, *"I-LOAM: Intensity
Enhanced LiDAR Odometry and Mapping"*, 17th International Conference on
Ubiquitous Robots (UR), 2020. No author code released (verified: not on the
authors' RPM Lab repositories; the GitHub "I-LOAM" results are unrelated LOAM
forks). Reconstructed from the paper.

## Core idea

I-LOAM keeps LOAM's geometric edge/plane feature pipeline but injects the LiDAR
**reflectance (intensity)** channel into the scan-to-scan association in two
places:

1. **Intensity-enhanced correspondence search.** When searching the previous
   frame for the second/third point that defines the line (edge) or plane, the
   candidate cost is augmented with a reflectance term

   ```
   cost(j) = ‖p_sel − q_j‖²  +  λ · (I_curr − I_j)²
   ```

   so that among geometrically comparable candidates the one with matching
   reflectance is preferred — this disambiguates repeated geometry (corridors,
   parallel façades).

2. **Intensity-similarity residual weighting.** Each accepted correspondence's
   residual is scaled by

   ```
   w = exp(−(I_curr − I_closest)² / 2σ_I²)        w ∈ (0, 1]
   ```

   down-weighting matches whose reflectance disagrees (likely wrong
   correspondences) in the Ceres least-squares problem.

The geometric feature extraction is reused from the shared `aloam`
`ScanRegistration`. Because that pipeline repurposes the point `intensity` field
to encode the scan-line id, the **true reflectance is recovered per feature
point by a nearest-neighbour lookup into the raw input cloud** (which still
carries `PointXYZI` reflectance).

## Scope / approximations for KITTI

- **Odometry only** (scan-to-scan); the paper's mapping refinement is not
  reproduced here — consistent with the repo's odometry-focused benchmarking.
- KITTI's intensity is **uncalibrated and coarse** (8-bit, range/incidence
  dependent). I-LOAM uses intensity only as a *correspondence cost term and a
  residual weight* (never as the primary signal), so it degrades gracefully
  rather than catastrophically when reflectance is noisy.
- The reflectance lookup uses the nearest raw point (no incidence/range
  normalisation), documented as a simplification.

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `use_intensity_correspondence` | intensity term in association search | true |
| `use_intensity_weight` | intensity residual weighting | true |
| `intensity_sigma` | σ_I in the residual weight | 0.15 |
| `intensity_corr_weight` | λ for the correspondence cost [m²] | 1.0 |
| `distance_sq_threshold` | max correspondence distance² | 25 m² |
| `num_optimization_iters` / `ceres_max_iterations` | outer / inner iters | 2 / 4 |

Ablation: `--i-loam-no-intensity` disables both intensity paths, reducing the
method to a plain geometric LOAM scan-to-scan baseline (used to measure what the
intensity channel actually contributes — see Result).

## Result (KITTI Odometry, `--no-gt-seed`, scan-to-scan only)

The headline is an **ablation**: the same pipeline run with the intensity paths
on vs. off (`--i-loam-no-intensity`). RPE is translational drift %/100 m; ATE in
parentheses. Mapping refinement is *not* implemented here, so absolute drift is
at plain-LOAM scan-to-scan level — the point is the **intensity delta**.

| Sequence | Geometric baseline (intensity off) | **I-LOAM (intensity on)** | Δ drift |
|---|---:|---:|---:|
| Seq 00 _(4541 fr)_ | 3.186% <sub>(76.0 m)</sub> | **2.606%** <sub>(49.4 m)</sub> | **−18.2%** |
| Seq 07 _(1101 fr)_ | 3.806% <sub>(18.5 m)</sub> | **3.055%** <sub>(15.1 m)</sub> | **−19.7%** |

The reflectance channel consistently cuts drift by ~18–20% and ATE by ~18–35%
versus the identical geometric pipeline, at ~12–15 FPS. mean intensity weight
≈ 0.76 (intensity is actively engaged). So I-LOAM's central claim — that LiDAR
reflectance disambiguates correspondences and improves odometry — **reproduces
on KITTI**, even though KITTI's intensity is uncalibrated and coarse. Honest
caveat: as scan-to-scan-only odometry the absolute accuracy is well below the
mapping-based entries on the main leaderboard; the contribution here is the
demonstrated, reproducible intensity improvement, not SOTA absolute drift.

Reproduce:

```sh
# intensity on (I-LOAM)
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv --methods i_loam --no-gt-seed
# intensity off (geometric baseline)
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv --methods i_loam \
  --no-gt-seed --i-loam-no-intensity
```

## Tests

`test_i_loam` covers: the intensity-weight function (bounded in (0,1], monotone
in |ΔI|, symmetric, disabled when σ≤0); the augmented correspondence distance
(reduces to geometry when λ=0 or ΔI=0, increases with |ΔI|); short-sequence
tracking with reflectance-bearing scans; that both intensity-on and
intensity-off variants track; and state reset on `clear()`.
