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

Baselines (no degeneracy awareness):

| Method | Pair accepted | Pair converged | Path length (m) | Net displacement (m) | Accumulated yaw drift |
|---|---:|---:|---:|---:|---:|
| KISS keyframe | 3240/3240 (100%) | 3240/3240 (100%) | 106.2 | 24.0 | 56.3 deg |
| LiTAMIN2 | 3240/3240 (100%) | 3240/3240 (100%) | 273.0 | 63.0 | 27.1 deg |
| CT-ICP | 3240/3240 (100%) | 387/3240 (11.9%, refinement-gate 100%) | 474.7 | 79.0 | 15.5 deg |

Degeneracy-aware methods (new `xicp_window_odometry` and
`degen_sense_window_odometry` runners, same sequence, same gates):

| Method | Pair accepted | Frames flagged degenerate | Path length (m) | Net displacement (m) | Accumulated yaw drift | Runtime (s) |
|---|---:|---:|---:|---:|---:|---:|
| X-ICP | 3240/3240 (100%) | 3234/3240 (99.8%) | 332.1 | 33.1 | -22.9 deg | 45 |
| DegenSense (no IMU) | 3240/3240 (100%) | 497/3240 (15.3%) | 457.3 | 105.3 | 26.8 deg | 948 |
| DegenSense (+IMU blend) | 3240/3240 (100%) | 453/3240 (14.0%) | 372.7 | 115.4 | 26.9 deg | 1057 |

None of the six configurations reach anywhere close to the true ~500 m
straight path. The best net displacement is DegenSense+IMU at 115.4 m --
still under a quarter of the true traversal. CT-ICP's total path length is
the closest to 500 m, but its net displacement is only 79 m, meaning the
estimated path curls back on itself rather than continuing straight -- the
tunnel's weak longitudinal constraint is being aliased into spurious
yaw/lateral motion, a textbook degeneracy failure mode. KISS keyframe is
worst: 106 m of path for 24 m of net displacement with 56 degrees of
accumulated yaw error.

### Detection vs. mitigation

The two degeneracy-aware methods split cleanly along the
detection/mitigation axis:

- **X-ICP detects the degeneracy essentially perfectly**: 99.8% of frames
  carry 1-2 degenerate directions in its localizability classification
  (typically the third translation/rotation axis at PARTIAL or NONE),
  exactly matching the tunnel geometry. This is the runtime signal the
  baselines entirely lack -- every baseline health gate stayed green for
  all 3240 pairs. But constraining updates along the unobservable direction
  cannot recover true longitudinal motion (net 33.1 m): X-ICP prevents
  false motion, it does not restore missing information.
- **DegenSense's adaptive spike detector is blind to chronic degeneracy**:
  its median + k*MAD rule over the degeneracy-factor time series adapts to
  the tunnel's persistently elevated condition numbers (S_rot median ~420-620
  across the run) and only flags spikes above that baseline (14-15% of
  frames). The paper designed the detector for transient degradation; a
  whole-run degenerate environment moves the median itself, so most
  degenerate frames read as "normal". Absolute-threshold classification
  (X-ICP) and adaptive outlier detection (DegenSense) give opposite answers
  on the same data.
- **IMU blending helps net displacement modestly** (105.3 -> 115.4 m) but
  does not touch yaw drift (26.8 vs 26.9 deg): the VN100's accelerometer
  cannot observe near-constant-velocity longitudinal motion over ~5 minutes
  any better than the LiDAR can, so the missing constraint stays missing.

The practical conclusion matches the motivation of the dataset's own paper
(radar-inertial fusion) and the degenerate-environment fusion arguments in
the reference talks (wheel odometry / Doppler velocity / intensity
constraints): in a chronically degenerate corridor, detection is achievable
from LiDAR alone (X-ICP's classification), but recovery requires a sensor
that actually observes the degenerate direction.

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

- ~~Run X-ICP (degeneracy-aware) over the same full tunnel_full sequence~~
  Done above; RELEAD's constrained-ESIKF frontend (needs IMU wiring into a
  runner) remains untested.
- Promote X-ICP's per-frame localizability classification into the triage
  policy as a runtime degeneracy signal: it flags 99.8% of tunnel frames
  while every existing GT-free health gate stays green. DegenSense's
  median+MAD detector should NOT be used for chronic degeneracy (it adapts
  to the elevated baseline; see Detection vs. mitigation above).
- A GT-free per-pair health gate is not sufficient for degeneracy detection;
  consider a trajectory-level check (e.g. net displacement vs. path length
  ratio, or accumulated yaw-rate variance) as an additional triage signal.
- Map-localization false-lock behavior (fixed_map_ndt) on tunnel geometry is
  still untested here; the Koide Hard Point Cloud Localization dataset
  (zenodo 10122133, see SETUP_HARD_PCL_LOCALIZATION_BENCHMARK.md) ships
  reference maps + GT and is the designated venue for that track.
