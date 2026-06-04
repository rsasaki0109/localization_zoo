# Adaptive ICP (Reliable Initial Pose)

## Paper
- "An Adaptive ICP LiDAR Odometry Based on Reliable Initial Pose", IEEE 2025
- arXiv: https://arxiv.org/abs/2509.22058
- **No public reference implementation** (no code released by the authors as of
  this writing) — this is a from-paper reimplementation.

## What This Repository Implements

A pure-LiDAR scan-to-local-map odometry built on the same voxel-hash loop as the
KISS-ICP / GenZ-ICP ports, with the paper's three core ideas added:

1. **Distributed coarse registration via density filtering** — per-point local
   density is counted on a voxel grid; only points at or above the
   `density_percentile` (α) density are kept for a short coarse alignment
   `T_align`. This makes the coarse stage rely on well-supported surfaces and
   reduces sensitivity to sparse/noisy returns.
2. **Reliable initial-pose selection** — the constant-velocity motion prediction
   `T_pred` is compared against the coarse solution `T_align` by their
   translation difference. If `‖trans(T_pred⁻¹·T_align)‖ ≤ τ` the coarse pose is
   trusted; otherwise `T_pred` is used to avoid converging to a local optimum.
3. **Adaptive threshold `σ_th` from current + historical errors** — a
   motion-state weight `γ_i = exp(-‖Δaccel‖ / σ_decay)` discounts frames with
   abrupt acceleration changes, and `σ_th` is the (weighted) RMS of per-frame
   errors `e_i = σ_max·tanh(β·θ_i) + ‖t_i‖` combining rotation angle and
   translation. `σ_th` drives both the correspondence gate and the robust
   point-to-plane weight `β_k = σ_th² / (σ_th² + e_k²)`.

## Current Scope

- self-contained registration loop (no ROS dependency in the core library)
- shares the project's voxel-hash map with per-correspondence normal / planarity
  estimation; the fine stage is point-to-plane on planar correspondences and a
  1-D point-to-point fallback elsewhere
- coarse stage uses the same solver with fewer iterations on density-filtered
  points

## Deviations / Not Included Yet

- the paper leaves several constants unspecified (search radius, `k`, `τ`,
  `σ_max`, `β`, `λ`); these are exposed as parameters with repo-consistent
  defaults rather than copied from the paper
- density and neighbor search use the project's voxel-grid approximation instead
  of a KD-tree
- the coarse stage uses point-to-plane ICP rather than a full GICP
  (joint-covariance) cost
