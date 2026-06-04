# SVN-ICP (from-paper reimplementation)

**Paper:** *SVN-ICP: Uncertainty Estimation of ICP-based LiDAR Odometry using
Stein Variational Newton*, Shiping Ma, Haoming Zhang, Marc Toussaint, 2025
([arXiv:2509.08069](https://arxiv.org/abs/2509.08069)). Author code not released.

## Core idea

Instead of a single point estimate, SVN-ICP approximates the **pose posterior**
with `M` particles and refines them with **Stein Variational Newton (SVN)** on the
SE(3) manifold, giving a registration result *and* a covariance (uncertainty)
without explicit noise modelling or hand-tuned parameters.

Each particle `θ_i` is an SE(3) left-tangent vector around the prediction
`T_base` (`T_i = Exp(θ_i)·T_base`). Per SVN iteration:

```
g_i = ∇log p(θ_i)          # point-to-plane log-likelihood gradient + prior
H_i = −∇²log p(θ_i)        # Gauss-Newton Hessian (also the per-particle precision)
k(θ_j,θ_i) = exp(−‖θ_j−θ_i‖²/h)      # RBF kernel, h via the median heuristic
φ(θ_i)    = (1/M) Σ_j [ k(θ_j,θ_i)·g_j + ∇_{θ_j}k(θ_j,θ_i) ]
H_svn(θ_i)= (1/M) Σ_j [ k(θ_j,θ_i)²·H_j + ∇k·∇kᵀ ]
θ_i ← θ_i + α · H_svn(θ_i)⁻¹ φ(θ_i)
```

The kernel gradient term is **repulsive** (spreads particles to cover the
posterior), the likelihood gradient is **attractive** (pulls toward the optimum),
and the Newton preconditioner `H_svn⁻¹` gives second-order steps. The final pose
is the particle mean; the **particle covariance is the uncertainty estimate** — it
grows in unconstrained directions (e.g. along a corridor) and shrinks where the
geometry is well conditioned.

## Scope / approximations for KITTI (pure-LiDAR)

- **Constant-velocity** prediction is the prior centre `T_base` (no IMU in KITTI);
  the paper's loose ESKF coupling with an IMU is out of scope.
- The point-to-plane correspondences are found **once per SVN iteration at the
  particle mean** and reused for all particles' gradients/Hessians (the
  association is stable across nearby particles) — this keeps the cost close to a
  single ICP per iteration instead of `M`×.
- `Exp([w,v]) = [[expSO3(w), v],[0,1]]` (small-motion SE(3) chart), consistent
  with the rest of the repo's incremental updates.
- A weak Gaussian **prior** toward the prediction (`prior_precision`) bounds the
  posterior in fully unconstrained directions and stabilizes the particles.

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `num_particles` | particle count M | 12 |
| `svn_iterations` | SVN iterations / frame | 4 |
| `step_size` | Newton step α | 1.0 |
| `lidar_sigma` | point noise σ (likelihood 1/σ²) | 0.1 |
| `prior_precision` | prior toward prediction | 1.0 |
| `init_spread_rot/trans` | initial particle spread | 0.01 rad / 0.05 m |

## Tests

`test_svn_icp` covers: known-translation recovery by the particle mean; **higher
posterior uncertainty (particle covariance) in a degenerate corridor** than a
closed box (the headline uncertainty-estimation behaviour); deterministic output
under the fixed-seed particle initialization.
