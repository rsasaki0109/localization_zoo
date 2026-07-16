# Tunnel Full-Trajectory Drift (3241 frames, ~324 s, ntnu-arl tunnel.bag)

Prior short-window health checks (tunnel_200, tunnel_geom_2700_200, 200 frames
each) reported 100% pair acceptance for geometry ICP, KISS keyframe, LiTAMIN2,
and CT-ICP, and concluded this was a negative result for local odometry on the
tunnel sequence. This run extracts and processes the full tunnel.bag (3241
LiDAR frames, ~5.4 minutes) continuously through the same three window-runner
binaries in GT-free mode, to test the doc's own follow-up hypothesis: that a
longer trajectory would surface degeneracy drift invisible to short-window
per-pair health gates.

The upstream README describes this as an 8 m wide, ~500 m straight tunnel
section, so a healthy odometry estimate should look close to a straight
~500 m line with near-zero net yaw drift.

## Results

| Method | Pair accepted | Pair converged | Path length (m) | Net displacement (m) | Accumulated yaw drift |
|---|---:|---:|---:|---:|---:|
| KISS keyframe | 3240/3240 (100%) | 3240/3240 (100%) | 106.2 | 24.0 | 56.3 deg |
| LiTAMIN2 | 3240/3240 (100%) | 3240/3240 (100%) | 273.0 | 63.0 | 27.1 deg |
| CT-ICP | 3240/3240 (100%) | 387/3240 (11.9%, refinement-gate 100%) | 474.7 | 79.0 | 15.5 deg |

None of the three methods reach anywhere close to the true ~500 m straight
path. CT-ICP's total path length is the closest to 500 m, but its net
displacement is still only 79 m, meaning the estimated path curls back on
itself rather than continuing straight -- the tunnel's weak longitudinal
constraint is being aliased into spurious yaw/lateral motion, a textbook
degeneracy failure mode. KISS keyframe is worst: it travels 106 m of path but
nets only 24 m of displacement with 56 degrees of accumulated yaw error.

## Interpretation

This confirms the doc's own prediction: short-window (200-frame, ~20 s) pair
health gates are blind to this failure. Each individual pairwise registration
step stays within the accepted step-size gate (2 m translation / 20 deg yaw)
and is locally "converged"/"accepted", so the health-check layer never
flags a problem -- but the accumulated trajectory over the full tunnel is
wrong by hundreds of meters of unrecovered net displacement. The prior
negative result (tunnel does not visibly break local odometry) was an
artifact of the 200-frame window being too short to let degeneracy drift
accumulate into something a per-pair gate would notice.

This is exactly the kind of failure degeneracy-aware methods (RELEAD's
Hessian-SVD degeneracy detection, X-ICP) target: none of the three baselines
tested here detect or constrain the degenerate direction, so it silently
accumulates into large drift while every local health signal stays green.

## Follow-up

- Run RELEAD / X-ICP (degeneracy-aware) over the same full tunnel_full
  sequence and compare drift against these three baselines.
- A GT-free per-pair health gate is not sufficient for degeneracy detection;
  consider a trajectory-level check (e.g. net displacement vs. path length
  ratio, or accumulated yaw-rate variance) as an additional triage signal.
- Map-localization false-lock behavior (fixed_map_ndt) on tunnel geometry is
  still untested; it requires building a reference map first and was not
  attempted in this pass.
