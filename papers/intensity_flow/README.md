# Intensity-Gradient-Flow Odometry (from-paper reimplementation)

**Paper:** *Intensity-enhanced LiDAR-inertial odometry with gradient-flow
sampling for degraded environments*, *Measurement* (Elsevier), 2026, PII
S0263224126007219. Author code not released and the publisher PDF is paywalled,
so the algorithm is reconstructed from its confirmed building blocks: **RMS**
(Petráček et al., RA-L 2024, gradient-flow sampling) and **COIN-LIO / IGE-LIO**
(intensity-augmented matching).

## Core ideas

1. **RMS geometric gradient-flow sampling.** For each point `p`, the displacement
   to its neighborhood centroid

   ```
   ∆p = (1/|N_p|) Σ_{j∈N_p} (j − p)
   ```

   is small inside flat surfaces (symmetric neighbors) and large at edges/corners.
   `‖∆p‖` is normalized, histogrammed into `K` bins, and points are drawn by
   cycling the bins from high gradient to low, picking the highest-`∆p` point per
   bin. This minimizes redundancy while preserving the spatial (entropy)
   distribution, keeping the informative points for localization.

2. **Intensity-geometry fused matching.** Each point-to-plane correspondence is
   weighted by intensity consistency

   ```
   w_int = exp(−(I_src − I_map)² / 2σ_I²)
   ```

   so correspondences whose source intensity disagrees with the map's local mean
   intensity are down-weighted (the "dynamic factor" / fused matching).

## Scope / approximations for KITTI (pure-LiDAR)

- KITTI `.bin` carries per-point intensity (`PointXYZI`), so the intensity term
  is fed directly; no IMU, so prediction is constant-velocity.
- The neighborhood radius `λp` is a **fixed `gf_radius`** rather than the
  range-adaptive `2·max(ν, ‖p‖·sinΔθ)` of RMS (simplification, documented).
- The entropy-rate stopping criterion of RMS is approximated by a fixed
  **`gf_keep_ratio`** with bin-cyclic high-gradient-first selection — same intent
  (distribution-preserving, redundancy-minimizing), simpler stop.
- The intensity term is realized as a **correspondence weight** (as in the repo's
  cube_lio), not a full image-space photometric residual with spherical
  projection + Sobel; the headline novelty here is the **gradient-flow sampling**.
  Geometric-consistency verification of pruned points is folded into the
  point-to-plane robust kernel.

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `gf_radius` | neighborhood radius λp | 1.0 m |
| `gf_num_bins` | histogram bins K | 10 (RMS) |
| `gf_keep_ratio` | sampling keep ratio | 0.5 |
| `intensity_sigma` | σ_I in intensity weight | 0.2 |
| `enable_gradient_flow` | RMS sampling on/off | true |
| `enable_intensity` | intensity weighting on/off | true |

## Tests

`test_intensity_flow` covers: gradient-flow sampling preferring boundary
(high-∆p) points over a flat interior; deterministic keep-ratio sampling;
known-translation recovery with sampling + intensity weighting enabled.
